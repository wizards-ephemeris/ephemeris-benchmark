#include "Catalog.hpp"
#include "Compare.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char* bodyName(eph::Body body) {
  switch (body) {
  case eph::Body::Sun: return "Sun";
  case eph::Body::Moon: return "Moon";
  case eph::Body::Mercury: return "Mercury";
  case eph::Body::Venus: return "Venus";
  case eph::Body::Mars: return "Mars";
  case eph::Body::Jupiter: return "Jupiter";
  case eph::Body::Saturn: return "Saturn";
  case eph::Body::Uranus: return "Uranus";
  case eph::Body::Neptune: return "Neptune";
  case eph::Body::Pluto: return "Pluto";
  case eph::Body::TrueNode: return "TrueNode";
  case eph::Body::MeanLilith: return "MeanLilith";
  case eph::Body::Chiron: return "Chiron";
  case eph::Body::Ceres: return "Ceres";
  case eph::Body::Pallas: return "Pallas";
  case eph::Body::Juno: return "Juno";
  case eph::Body::Vesta: return "Vesta";
  }
  return "?";
}

const eph::Body kBodies[] = {
  eph::Body::Sun, eph::Body::Moon, eph::Body::Mercury, eph::Body::Venus, eph::Body::Mars,
  eph::Body::Jupiter, eph::Body::Saturn, eph::Body::Uranus, eph::Body::Neptune, eph::Body::Pluto,
  eph::Body::TrueNode, eph::Body::MeanLilith, eph::Body::Chiron, eph::Body::Ceres,
  eph::Body::Pallas, eph::Body::Juno, eph::Body::Vesta
};

std::vector<double> grid() {
  std::vector<double> jds;
  const double span = 2488070.0 - 2415021.0;
  for (int i = 0; i < 1000; ++i) jds.push_back(2415021.0 + span * i / 999.0);
  return jds;
}

} // namespace

int main(int argc, char** argv) {
  const char* path = argc > 1 ? argv[1] : "bench_position.json";
  const std::vector<double> jds = grid();
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  for (emph::Adapter* lib : emph::libraries()) lib->setSidereal(false, eph::Ayanamsa::Lahiri);

  const int nBody = static_cast<int>(sizeof(kBodies) / sizeof(kBodies[0]));
  std::vector<std::vector<double>> ns(emph::libraries().size(), std::vector<double>(nBody, 0.0));
  std::vector<std::vector<double>> err(emph::libraries().size(), std::vector<double>(nBody, 0.0));

  for (std::size_t a = 0; a < emph::libraries().size(); ++a) {
    emph::Adapter* lib = emph::libraries()[a];
    for (int b = 0; b < nBody; ++b) {
      const auto t0 = std::chrono::steady_clock::now();
      for (double jd : jds) {
        volatile double lon = lib->position(kBodies[b], jd).lonDeg;
        (void)lon;
      }
      const auto t1 = std::chrono::steady_clock::now();
      ns[a][b] = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(jds.size());
      if (lib == &ref) continue;
      for (double jd : jds) {
        const double d = arcsec(lib->position(kBodies[b], jd).lonDeg, ref.position(kBodies[b], jd).lonDeg);
        if (d > err[a][b]) err[a][b] = d;
      }
    }
  }

  std::ofstream out(path);
  out << "{\n  \"bodies\": [";
  for (int b = 0; b < nBody; ++b) out << (b ? ", " : "") << "\"" << bodyName(kBodies[b]) << "\"";
  out << "],\n  \"libraries\": [\n";
  for (std::size_t a = 0; a < emph::libraries().size(); ++a) {
    out << "    {\"name\": \"" << emph::libraries()[a]->name() << "\", \"nsPerCall\": [";
    for (int b = 0; b < nBody; ++b) out << (b ? ", " : "") << ns[a][b];
    out << "], \"worstArcsec\": [";
    for (int b = 0; b < nBody; ++b) out << (b ? ", " : "") << err[a][b];
    out << "]}" << (a + 1 == emph::libraries().size() ? "\n" : ",\n");
  }
  out << "  ]\n}\n";
  std::cout << "wrote " << path << "\n";
  return out ? 0 : 1;
}
