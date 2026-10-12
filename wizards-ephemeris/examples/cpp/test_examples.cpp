#include "eph/eph.hpp"
#include "examples.hpp"

#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void fail(const std::string& label) {
  std::cerr << label << '\n';
  failures += 1;
}

bool nearAngle(double a, double b, double arcsec) {
  return eph::sepDeg(a, b) * 3600.0 <= arcsec;
}

double boundaryArcsec(double deg) {
  double mod = std::fmod(std::fmod(deg, 30.0) + 30.0, 30.0);
  return std::min(mod, 30.0 - mod) * 3600.0;
}

void checkJulianDay() {
  const JulianDayEx got = ex_julian_day();
  if (got.jd != 2451545.0) fail("julian day value");
  if (got.year != 2000 || got.month != 1 || got.day != 1) fail("julian day calendar");
  if (std::fabs(got.hour - 12.0) > 1e-6) fail("julian day hour");
}

void checkUtcDate() {
  if (ex_utc_date().jd != 2451545.0) fail("utc date");
}

void checkDeltaT() {
  const double seconds = ex_delta_t().seconds;
  if (!(seconds > 0.0 && seconds < 120.0)) fail("delta T");
}

void checkTropicalSun() {
  const EclipticEx sun = ex_tropical_sun();
  if (!(sun.lon >= 270.0 && sun.lon < 300.0)) fail("sun sign");
  if (std::fabs(sun.lat) > 1.0) fail("sun latitude");
  if (!(sun.lonSpeed >= 0.9 && sun.lonSpeed <= 1.1)) fail("sun speed");
}

void checkEquatorialSun() {
  const EquatorialEx sun = ex_equatorial_sun();
  if (!std::isfinite(sun.ra)) fail("sun ra");
  if (!(sun.dec >= -24.0 && sun.dec <= -22.0)) fail("sun declination");
}

void checkSeparation() {
  const SeparationEx got = ex_separation();
  if (std::fabs(got.quarter - 20.0) > 1e-6) fail("separation 10 to 350");
  if (got.same != 0.0) fail("separation with itself");
}

void checkLahiri() {
  const LahiriEx got = ex_lahiri_chart();
  if (!nearAngle(got.siderealLon, got.tropicalLon - got.ayanamsa, 0.05)) {
    fail("lahiri identity");
  }
}

void checkMeanNutation() {
  const MeanNutationEx got = ex_mean_nutation();
  const double sunGap = eph::sepDeg(got.trueLon, got.meanLon);
  const double ayGap = eph::sepDeg(got.trueAyanamsa, got.meanAyanamsa);
  if (std::fabs(sunGap - ayGap) * 3600.0 > 0.05) fail("nutation gaps differ");
  const double arcsec = sunGap * 3600.0;
  if (!(arcsec > 0.001 && arcsec < 20.0)) fail("nutation size");
}

void checkCitra() {
  const CitraGalacticEx got = ex_citra_galactic();
  const double values[] = {got.lahiri, got.citra, got.galactic};
  for (double v : values) {
    if (!(v >= 0.0 && v < 360.0)) fail("ayanamsa range");
  }
  const double citraSep = eph::sepDeg(got.citra, got.lahiri);
  if (!(citraSep > 0.01 && citraSep < 0.05)) fail("citra vs lahiri");
  if (eph::sepDeg(got.galactic, got.lahiri) <= 0.5) fail("galactic vs lahiri");
}

void checkInclined() {
  const LilithEx got = ex_inclined_lilith();
  if (!std::isfinite(got.lon)) fail("inclined longitude");
  if (!(got.lat >= 0.5 && got.lat <= 5.2)) fail("inclined latitude");
}

void checkEclipticLilith() {
  const LilithPairEx got = ex_ecliptic_lilith();
  if (std::fabs(got.eclipticLat) * 3600.0 >= 0.01) fail("ecliptic latitude");
  if (!nearAngle(got.inclinedLon, got.eclipticLon, 0.05)) fail("lilith longitude");
}

void checkPlacidus() {
  const HousesEx h = ex_placidus();
  if (!nearAngle(h.cusps[0], h.asc, 0.05)) fail("placidus cusp 1");
  if (!nearAngle(h.cusps[9], h.mc, 0.05)) fail("placidus cusp 10");
  if (h.fallback) fail("placidus fallback");
}

void checkEqual() {
  const HousesEx h = ex_equal_houses();
  for (int i = 0; i < 12; ++i) {
    if (!nearAngle(h.cusps[static_cast<std::size_t>(i)], h.asc + i * 30.0, 0.05)) {
      fail("equal cusp");
    }
  }
}

void checkWholeSign() {
  const HousesEx h = ex_whole_sign();
  for (double cusp : h.cusps) {
    if (boundaryArcsec(cusp) > 0.05) fail("whole sign boundary");
  }
}

void checkSnap() {
  const HousesEx h = ex_snap_then_shift();
  if (eph::sepDeg(h.cusps[0], std::round(h.cusps[0] / 30.0) * 30.0) <= 1.0) {
    fail("snap then shift still on a boundary");
  }
}

void checkArmc() {
  const AnglesEx got = ex_armc_fixed();
  if (!nearAngle(got.tropicalArmc, got.siderealArmc, 0.05)) fail("armc shifted");
  if (eph::sepDeg(got.tropicalAsc, got.siderealAsc) <= 1.0) fail("ascendant did not move");
}

void checkEquator() {
  const EquatorFixedEx got = ex_equator_fixed();
  if (!nearAngle(got.tropicalRa, got.siderealRa, 0.05)) fail("right ascension shifted");
  if (eph::sepDeg(got.tropicalLon, got.siderealLon) <= 1.0) fail("longitude did not move");
}

void checkSripati() {
  const SripatiEx got = ex_sripati();
  if (!std::isfinite(got.asc) || !std::isfinite(got.mc)) fail("sripati angles");
  if (eph::sepDeg(got.cusp1, got.asc) <= 1.0) fail("sripati cusp is the ascendant");
}

void checkPolich() {
  const PolichEx got = ex_polich_page();
  if (!nearAngle(got.cusp1, got.asc, 0.05)) fail("polich cusp 1");
  if (!nearAngle(got.cusp10, got.mc, 0.05)) fail("polich cusp 10");
}

void checkPolar() {
  const PolarEx got = ex_polar_fallback();
  if (!got.placidusFallback) fail("polar placidus");
  if (!got.kochFallback) fail("polar koch");
}

} // namespace

int main() {
  checkJulianDay();
  checkUtcDate();
  checkDeltaT();
  checkTropicalSun();
  checkEquatorialSun();
  checkSeparation();
  checkLahiri();
  checkMeanNutation();
  checkCitra();
  checkInclined();
  checkEclipticLilith();
  checkPlacidus();
  checkEqual();
  checkWholeSign();
  checkSnap();
  checkArmc();
  checkEquator();
  checkSripati();
  checkPolich();
  checkPolar();
  return failures == 0 ? 0 : 1;
}
