#include "Spk.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>

namespace eph::detail {
namespace {

bool hostLittle() {
  const uint16_t x = 1;
  return *reinterpret_cast<const uint8_t*>(&x) == 1;
}

double readF64(const uint8_t* p, bool fileLittle) {
  uint8_t b[8];
  if (fileLittle == hostLittle()) {
    std::memcpy(b, p, 8);
  } else {
    for (int i = 0; i < 8; ++i) b[i] = p[7 - i];
  }
  double d = 0.0;
  std::memcpy(&d, b, 8);
  return d;
}

int32_t readI32(const uint8_t* p, bool fileLittle) {
  uint8_t b[4];
  if (fileLittle == hostLittle()) {
    std::memcpy(b, p, 4);
  } else {
    for (int i = 0; i < 4; ++i) b[i] = p[3 - i];
  }
  int32_t v = 0;
  std::memcpy(&v, b, 4);
  return v;
}

double chebyshev(const double* lowFirst, int n, double s) {
  double b2 = 0.0;
  double b1 = 0.0;
  const double two = 2.0 * s;
  for (int i = n - 1; i >= 1; --i) {
    const double bk = two * b1 - b2 + lowFirst[i];
    b2 = b1;
    b1 = bk;
  }
  return s * b1 - b2 + lowFirst[0];
}

bool sameBody(int target, int want) {
  if (target == want) return true;
  if (want == 999 && target == 9) return true;
  if (want == 2002060 && target == 2060) return true;
  if (want >= 2000001 && want <= 2000004 && target == want - 2000000) return true;
  return false;
}

} // namespace

void SpkBook::loadDirectory(const std::string& dir) {
  namespace fs = std::filesystem;
  std::error_code ec;
  if (dir.empty() || !fs::is_directory(dir, ec)) return;
  for (const fs::directory_entry& ent : fs::directory_iterator(dir, ec)) {
    if (ec) break;
    if (!ent.is_regular_file()) continue;
    if (ent.path().extension() != ".bsp") continue;
    std::ifstream in(ent.path(), std::ios::binary);
    if (!in) continue;
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (bytes.size() < 1024) continue;
    if (std::memcmp(bytes.data(), "DAF/SPK", 7) != 0 && std::memcmp(bytes.data(), "NAIF/DAF", 8) != 0) {
      continue;
    }
    const uint8_t* loc = bytes.data() + 88;
    bool little = std::memcmp(loc, "LTL-IEEE", 8) == 0;
    if (!little && std::memcmp(loc, "BIG-IEEE", 8) != 0) {
      // Older NAIF/DAF files omit the format tag. ND == 2 picks the endianness.
      const int ndLittle = readI32(bytes.data() + 8, true);
      little = ndLittle == 2;
    }
    const int nd = readI32(bytes.data() + 8, little);
    const int ni = readI32(bytes.data() + 12, little);
    if (nd != 2 || ni != 6) continue;
    uint32_t rec = static_cast<uint32_t>(readI32(bytes.data() + 76, little));
    auto record = [&](uint32_t n) -> const uint8_t* {
      const size_t off = static_cast<size_t>(n - 1) * 1024;
      if (n == 0 || off + 1024 > bytes.size()) return nullptr;
      return bytes.data() + off;
    };
    auto word = [&](int index1) -> double {
      const size_t off = static_cast<size_t>(index1 - 1) * 8;
      if (index1 < 1 || off + 8 > bytes.size()) return 0.0;
      return readF64(bytes.data() + off, little);
    };

    int guard = 0;
    while (rec != 0 && guard++ < 100000) {
      const uint8_t* data = record(rec);
      if (!data) break;
      const double next = readF64(data, little);
      const int nsum = static_cast<int>(readF64(data + 16, little));
      for (int i = 0; i < nsum; ++i) {
        const int off = 24 + i * 40;
        if (off + 40 > 1024) break;
        Segment seg;
        const double startSec = readF64(data + off, little);
        const double endSec = readF64(data + off + 8, little);
        seg.target = readI32(data + off + 16, little);
        seg.center = readI32(data + off + 20, little);
        seg.frame = readI32(data + off + 24, little);
        const int type = readI32(data + off + 28, little);
        const int startI = readI32(data + off + 32, little);
        const int endI = readI32(data + off + 36, little);
        if ((type != 2 && type != 3) || endI < startI + 4) continue;
        seg.components = type == 2 ? 3 : 6;
        seg.initSec = word(endI - 3);
        seg.intlenSec = word(endI - 2);
        const double rsize = word(endI - 1);
        const double nrec = word(endI);
        if (seg.intlenSec <= 0.0 || rsize < 2.0 || nrec < 1.0) continue;
        seg.coeffCount = static_cast<int>(rsize - 2.0) / seg.components;
        seg.nRecords = static_cast<int>(nrec);
        if (seg.coeffCount < 1 || seg.nRecords < 1) continue;
        seg.startJd = kJ2000 + startSec / 86400.0;
        seg.endJd = kJ2000 + endSec / 86400.0;
        seg.records.resize(static_cast<size_t>(seg.nRecords));
        const int wordsPer = 2 + seg.components * seg.coeffCount;
        bool ok = true;
        for (int r = 0; r < seg.nRecords; ++r) {
          const int base = startI + r * wordsPer;
          if (static_cast<size_t>(base + wordsPer - 1) * 8 > bytes.size()) {
            ok = false;
            break;
          }
          Record& recOut = seg.records[static_cast<size_t>(r)];
          recOut.mid = word(base);
          recOut.radius = word(base + 1);
          recOut.coef.resize(static_cast<size_t>(seg.components * seg.coeffCount));
          for (int k = 0; k < seg.components * seg.coeffCount; ++k) {
            recOut.coef[static_cast<size_t>(k)] = word(base + 2 + k);
          }
        }
        if (ok) segments_.push_back(std::move(seg));
      }
      rec = static_cast<uint32_t>(next);
    }
  }
}

bool SpkBook::state(int center, int target, double jd, Vec3& out, int& frame) const {
  const Segment* found = nullptr;
  for (const Segment& seg : segments_) {
    if (seg.center != center || !sameBody(seg.target, target)) continue;
    if (jd < seg.startJd - 1e-8 || jd > seg.endJd + 1e-8) continue;
    found = &seg;
  }
  if (!found) return false;
  const double seconds = (jd - kJ2000) * 86400.0;
  double indexReal = (seconds - found->initSec) / found->intlenSec;
  int index = static_cast<int>(std::floor(indexReal));
  double offset = seconds - found->initSec - index * found->intlenSec;
  if (index == found->nRecords) {
    index -= 1;
    offset += found->intlenSec;
  }
  if (index < 0 || index >= found->nRecords) return false;
  const Record& rec = found->records[static_cast<size_t>(index)];
  const double s = 2.0 * offset / found->intlenSec - 1.0;
  double xyz[3];
  for (int c = 0; c < 3; ++c) {
    const double* coef = rec.coef.data() + static_cast<size_t>(c * found->coeffCount);
    xyz[c] = chebyshev(coef, found->coeffCount, s);
  }
  // SPK type 2/3 positions are kilometres.
  out = Vec3{xyz[0], xyz[1], xyz[2]} / kAuKm;
  frame = found->frame;
  return true;
}

bool SpkBook::heliocentric(int naifId, double jd, Vec3& out, bool& equatorial) const {
  int frame = 1;
  if (state(10, naifId, jd, out, frame)) {
    equatorial = frame != 17;
    return true;
  }
  Vec3 body;
  Vec3 sun;
  int frameBody = 1;
  int frameSun = 1;
  if (state(0, naifId, jd, body, frameBody) && state(0, 10, jd, sun, frameSun)) {
    out = body - sun;
    equatorial = frameBody != 17;
    return true;
  }
  return false;
}

} // namespace eph::detail
