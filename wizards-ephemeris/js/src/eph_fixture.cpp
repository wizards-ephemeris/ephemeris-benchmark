#include "eph/eph.hpp"

#include <fstream>
#include <iostream>
#include <string>

namespace {

constexpr double kJd = 2451545.0;
constexpr double kLat = 50.22;
constexpr double kLon = 18.67;

void writePos(std::ostream& out, const eph::EclipticPos& pos) {
  out << "{\"lon\":" << pos.lon.deg()
      << ",\"lat\":" << pos.lat.deg()
      << ",\"lonSpeed\":" << pos.lonSpeed
      << ",\"distAu\":" << pos.distAu << "}";
}

void writeEq(std::ostream& out, const eph::EquatorialPos& pos) {
  out << "{\"ra\":" << pos.ra.deg()
      << ",\"dec\":" << pos.dec.deg()
      << ",\"raSpeed\":" << pos.raSpeed << "}";
}

void writeHouses(std::ostream& out, const eph::Houses& h) {
  out << "{\"cusps\":[";
  for (int i = 0; i < 12; ++i) {
    if (i) out << ',';
    out << h.cusp[i].deg();
  }
  out << "],\"asc\":" << h.asc.deg()
      << ",\"mc\":" << h.mc.deg()
      << ",\"armc\":" << h.armc.deg()
      << ",\"vertex\":" << h.vertex.deg()
      << ",\"fallback\":" << (h.fallback ? "true" : "false") << "}";
}

} // namespace

int main(int argc, char** argv) {
  std::ofstream file;
  std::ostream* out = &std::cout;
  if (argc > 1) {
    file.open(argv[1]);
    if (!file) {
      std::cerr << "could not write " << argv[1] << '\n';
      return 1;
    }
    out = &file;
  }
  out->precision(17);

  eph::Ephemeris chart;
  const eph::JulianDay jd{kJd};

  *out << "{\"jd\":" << kJd
       << ",\"deltaT\":" << eph::deltaTSeconds(jd)
       << ",\"lat\":" << kLat << ",\"lonEast\":" << kLon;

  const eph::EclipticPos sun = chart.position(eph::Body::Sun, jd);
  const eph::EquatorialPos sunEq = chart.equatorial(eph::Body::Sun, jd);
  *out << ",\"sunTropical\":";
  writePos(*out, sun);
  *out << ",\"sunTropicalEquatorial\":";
  writeEq(*out, sunEq);

  chart.setSidereal(true, eph::SiderealMethod{
    eph::Ayanamsa::Lahiri,
    eph::SiderealNutation::True,
    eph::WholeSignSnap::ShiftThenSnap
  });
  *out << ",\"sunSiderealLahiri\":";
  writePos(*out, chart.position(eph::Body::Sun, jd));

  chart.setSidereal(false);
  chart.setLilithMode(eph::LilithMode::Inclined);
  *out << ",\"lilithInclined\":";
  writePos(*out, chart.position(eph::Body::MeanLilith, jd));

  chart.setLilithMode(eph::LilithMode::Ecliptic);
  *out << ",\"lilithEcliptic\":";
  writePos(*out, chart.position(eph::Body::MeanLilith, jd));

  chart.setLilithMode(eph::LilithMode::Inclined);
  chart.setSidereal(false);
  *out << ",\"placidus\":";
  writeHouses(*out, chart.houses(jd, kLat, kLon, eph::HouseSystem::Placidus));

  chart.setSidereal(true, eph::SiderealMethod{
    eph::Ayanamsa::Lahiri,
    eph::SiderealNutation::True,
    eph::WholeSignSnap::ShiftThenSnap
  });
  *out << ",\"wholeSignSidereal\":";
  writeHouses(*out, chart.houses(jd, kLat, kLon, eph::HouseSystem::WholeSign));

  *out << "}\n";
  return 0;
}
