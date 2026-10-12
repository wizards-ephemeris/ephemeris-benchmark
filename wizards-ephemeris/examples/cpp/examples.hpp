#pragma once

#include <array>

struct JulianDayEx {
  double jd = 0;
  int year = 0;
  int month = 0;
  int day = 0;
  double hour = 0;
};

struct UtcDateEx {
  double jd = 0;
};

struct DeltaTEx {
  double seconds = 0;
};

struct EclipticEx {
  double lon = 0;
  double lat = 0;
  double lonSpeed = 0;
  double distAu = 0;
};

struct EquatorialEx {
  double ra = 0;
  double dec = 0;
  double raSpeed = 0;
};

struct SeparationEx {
  double quarter = 0;
  double same = 0;
};

struct LahiriEx {
  double tropicalLon = 0;
  double siderealLon = 0;
  double ayanamsa = 0;
};

struct MeanNutationEx {
  double trueLon = 0;
  double meanLon = 0;
  double trueAyanamsa = 0;
  double meanAyanamsa = 0;
};

struct CitraGalacticEx {
  double lahiri = 0;
  double citra = 0;
  double galactic = 0;
};

struct LilithEx {
  double lon = 0;
  double lat = 0;
};

struct LilithPairEx {
  double inclinedLon = 0;
  double inclinedLat = 0;
  double eclipticLon = 0;
  double eclipticLat = 0;
};

struct HousesEx {
  std::array<double, 12> cusps {};
  double asc = 0;
  double mc = 0;
  double armc = 0;
  double vertex = 0;
  bool fallback = false;
};

struct AnglesEx {
  double tropicalArmc = 0;
  double siderealArmc = 0;
  double tropicalAsc = 0;
  double siderealAsc = 0;
};

struct EquatorFixedEx {
  double tropicalLon = 0;
  double siderealLon = 0;
  double tropicalRa = 0;
  double siderealRa = 0;
};

struct SripatiEx {
  double cusp1 = 0;
  double asc = 0;
  double mc = 0;
};

struct PolichEx {
  double cusp1 = 0;
  double cusp10 = 0;
  double asc = 0;
  double mc = 0;
};

struct PolarEx {
  bool placidusFallback = false;
  bool kochFallback = false;
};

JulianDayEx ex_julian_day();
UtcDateEx ex_utc_date();
DeltaTEx ex_delta_t();
EclipticEx ex_tropical_sun();
EquatorialEx ex_equatorial_sun();
SeparationEx ex_separation();
LahiriEx ex_lahiri_chart();
MeanNutationEx ex_mean_nutation();
CitraGalacticEx ex_citra_galactic();
LilithEx ex_inclined_lilith();
LilithPairEx ex_ecliptic_lilith();
HousesEx ex_placidus();
HousesEx ex_equal_houses();
HousesEx ex_whole_sign();
HousesEx ex_snap_then_shift();
AnglesEx ex_armc_fixed();
EquatorFixedEx ex_equator_fixed();
SripatiEx ex_sripati();
PolichEx ex_polich_page();
PolarEx ex_polar_fallback();
