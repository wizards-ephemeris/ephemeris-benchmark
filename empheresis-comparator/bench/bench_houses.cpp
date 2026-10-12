#include "Catalog.hpp"
#include "Compare.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

const char* systemName(eph::HouseSystem sys) {
  switch (sys) {
  case eph::HouseSystem::Placidus: return "Placidus";
  case eph::HouseSystem::WholeSign: return "WholeSign";
  case eph::HouseSystem::Koch: return "Koch";
  case eph::HouseSystem::Equal: return "Equal";
  case eph::HouseSystem::Regiomontanus: return "Regiomontanus";
  case eph::HouseSystem::Campanus: return "Campanus";
  case eph::HouseSystem::Porphyrius: return "Porphyrius";
  case eph::HouseSystem::Alcabitius: return "Alcabitius";
  case eph::HouseSystem::Sripati: return "Sripati";
  case eph::HouseSystem::PolichPage: return "PolichPage";
  }
  return "?";
}

const eph::HouseSystem kSystems[] = {
  eph::HouseSystem::Placidus, eph::HouseSystem::WholeSign, eph::HouseSystem::Koch,
  eph::HouseSystem::Equal, eph::HouseSystem::Regiomontanus, eph::HouseSystem::Campanus,
  eph::HouseSystem::Porphyrius, eph::HouseSystem::Alcabitius
};

} // namespace

int main(int argc, char** argv) {
  const char* path = argc > 1 ? argv[1] : "bench_houses.json";
  const double lats[] = {-60.0, -30.0, 0.0, 30.0, 50.22, 66.0};
  const double lons[] = {-120.0, 0.0, 18.67, 140.0};
  std::vector<double> dates;
  for (double jd = 2415021.0; jd <= 2469807.5; jd += 400.0) dates.push_back(jd);
  const int nSys = static_cast<int>(sizeof(kSystems) / sizeof(kSystems[0]));
  int calls = 0;
  for (double date : dates) {
    (void)date;
    calls += static_cast<int>(sizeof(lats) / sizeof(lats[0]) * sizeof(lons) / sizeof(lons[0]));
  }

  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  for (emph::Adapter* lib : emph::libraries()) lib->setSidereal(false, eph::Ayanamsa::Lahiri);

  std::vector<std::vector<double>> ns(emph::libraries().size(), std::vector<double>(nSys, 0.0));
  std::vector<std::vector<double>> err(emph::libraries().size(), std::vector<double>(nSys, 0.0));

  for (std::size_t a = 0; a < emph::libraries().size(); ++a) {
    emph::Adapter* lib = emph::libraries()[a];
    for (int s = 0; s < nSys; ++s) {
      const auto t0 = std::chrono::steady_clock::now();
      for (double date : dates) {
        for (double lat : lats) {
          for (double lon : lons) {
            volatile double asc = lib->houses(date, lat, lon, kSystems[s]).ascDeg;
            (void)asc;
          }
        }
      }
      const auto t1 = std::chrono::steady_clock::now();
      ns[a][s] = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(calls);
      if (lib == &ref) continue;
      for (double date : dates) {
        for (double lat : lats) {
          for (double lon : lons) {
            const emph::HousePlace got = lib->houses(date, lat, lon, kSystems[s]);
            const emph::HousePlace expect = ref.houses(date, lat, lon, kSystems[s]);
            const double d = arcsec(got.ascDeg, expect.ascDeg);
            if (d > err[a][s]) err[a][s] = d;
          }
        }
      }
    }
  }

  std::ofstream out(path);
  out << "{\n  \"systems\": [";
  for (int s = 0; s < nSys; ++s) out << (s ? ", " : "") << "\"" << systemName(kSystems[s]) << "\"";
  out << "],\n  \"libraries\": [\n";
  for (std::size_t a = 0; a < emph::libraries().size(); ++a) {
    out << "    {\"name\": \"" << emph::libraries()[a]->name() << "\", \"nsPerCall\": [";
    for (int s = 0; s < nSys; ++s) out << (s ? ", " : "") << ns[a][s];
    out << "], \"worstArcsec\": [";
    for (int s = 0; s < nSys; ++s) out << (s ? ", " : "") << err[a][s];
    out << "]}" << (a + 1 == emph::libraries().size() ? "\n" : ",\n");
  }
  out << "  ]\n}\n";
  std::cout << "wrote " << path << "\n";
  return out ? 0 : 1;
}
