#pragma once

#include "Adapter.hpp"

#include <array>
#include <cmath>
#include <string>
#include <vector>

inline double arcsec(double aDeg, double bDeg) {
  return eph::sepDeg(aDeg, bDeg) * 3600.0;
}

// Speeds are not circle positions. A negative rate must not wrap through 360.
inline double speedArcsec(double aDegPerDay, double bDegPerDay) {
  return std::fabs(aDegPerDay - bDegPerDay) * 3600.0;
}

inline std::vector<double> monthlyJds() {
  std::vector<double> jds;
  for (double jd = 2415021.0; jd <= 2488070.0; jd += 30.0) jds.push_back(jd);
  return jds;
}

inline std::vector<double> quarterlyJds() {
  std::vector<double> jds;
  for (double jd = 2415021.0; jd <= 2488070.0; jd += 90.0) jds.push_back(jd);
  return jds;
}

// House geometry is compared through 2050. Later delta-T drift moves the RAMC.
inline std::vector<double> monthlyHouseJds() {
  std::vector<double> jds;
  for (double jd = 2415021.0; jd <= 2469807.5; jd += 30.0) jds.push_back(jd);
  return jds;
}

inline std::vector<double> yearlyHouseJds() {
  std::vector<double> jds;
  for (double jd = 2415021.0; jd <= 2469807.5; jd += 365.25) jds.push_back(jd);
  return jds;
}

inline std::array<eph::Body, 17> allBodies() {
  return {
    eph::Body::Sun, eph::Body::Moon, eph::Body::Mercury, eph::Body::Venus, eph::Body::Mars,
    eph::Body::Jupiter, eph::Body::Saturn, eph::Body::Uranus, eph::Body::Neptune, eph::Body::Pluto,
    eph::Body::TrueNode, eph::Body::MeanLilith, eph::Body::Chiron, eph::Body::Ceres,
    eph::Body::Pallas, eph::Body::Juno, eph::Body::Vesta
  };
}

inline std::array<eph::Ayanamsa, 6> allAyanamsas() {
  return {
    eph::Ayanamsa::Lahiri, eph::Ayanamsa::Raman,
    eph::Ayanamsa::FaganBradley, eph::Ayanamsa::Krishnamurti,
    eph::Ayanamsa::Citra, eph::Ayanamsa::GalacticCenterMula
  };
}

inline std::array<eph::HouseSystem, 10> allHouseSystems() {
  return {
    eph::HouseSystem::Placidus, eph::HouseSystem::WholeSign, eph::HouseSystem::Koch,
    eph::HouseSystem::Equal, eph::HouseSystem::Regiomontanus, eph::HouseSystem::Campanus,
    eph::HouseSystem::Porphyrius, eph::HouseSystem::Alcabitius,
    eph::HouseSystem::Sripati, eph::HouseSystem::PolichPage
  };
}

inline const char* bodyName(eph::Body body) {
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

// Longitude, latitude, right ascension, and declination, in arcseconds.
inline double placeBoundArcsec(eph::Body body) {
  switch (body) {
  case eph::Body::Uranus: return 2.0;
  case eph::Body::Neptune: return 3.0;
  case eph::Body::Moon: return 10.0;
  case eph::Body::TrueNode: return 10.0;
  case eph::Body::MeanLilith: return 2.0;
  case eph::Body::Chiron:
  case eph::Body::Ceres:
  case eph::Body::Pallas:
  case eph::Body::Juno:
  case eph::Body::Vesta:
    return 5.0;
  default:
    return 1.0;
  }
}

// Ecliptic longitude speed and right-ascension speed, in arcseconds per day.
inline double speedBoundArcsec(eph::Body body) {
  if (body == eph::Body::Moon) return 30.0;
  if (body == eph::Body::TrueNode) return 120.0;
  return placeBoundArcsec(body);
}

// Sidereal longitude against Swiss. True subtracts nutation in longitude,
// the same offset Swiss uses, so the bound is the tropical place plus 1".
// Mean leaves that nutation in the longitude, about ±18".
inline double siderealSwissArcsec(eph::Body body, eph::SiderealNutation nutation) {
  const double slack = nutation == eph::SiderealNutation::True ? 1.0 : 20.0;
  return placeBoundArcsec(body) + slack;
}

struct Worst {
  double sep = 0.0;
  double jd = 0.0;
  std::string note;
  void consider(double separation, double at, const std::string& text) {
    if (separation > sep) {
      sep = separation;
      jd = at;
      note = text;
    }
  }
};

inline void considerHouses(Worst& angles, Worst& cusps, const emph::HousePlace& got, const emph::HousePlace& expect, double at) {
  angles.consider(arcsec(got.ascDeg, expect.ascDeg), at, "asc");
  angles.consider(arcsec(got.mcDeg, expect.mcDeg), at, "mc");
  angles.consider(arcsec(got.armcDeg, expect.armcDeg), at, "armc");
  angles.consider(arcsec(got.vertexDeg, expect.vertexDeg), at, "vertex");
  for (int i = 0; i < 12; ++i) {
    cusps.consider(arcsec(got.cuspDeg[i], expect.cuspDeg[i]), at, "cusp");
  }
}
