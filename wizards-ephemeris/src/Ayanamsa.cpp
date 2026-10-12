#include "eph/Ayanamsa.hpp"

#include "Reduce.hpp"
#include "VSOP87.hpp"
#include "eph/Math.hpp"
#include "eph/Time.hpp"

namespace eph {
namespace {

struct Ay {
  double a0Deg;
  double extraArcsecPerCy;
};

// Mean ayanamsa at J2000. The rate is IAU 2006 general precession; the
// constant is the mode's offset so the value matches the mean equinox used
// by the comparison ephemeris. Nutation is not included.
Ay coeff(Ayanamsa mode) {
  switch (mode) {
  case Ayanamsa::Lahiri: return {23.85709235370882, 0.0};
  case Ayanamsa::Raman: return {22.41079104032650, 0.0};
  case Ayanamsa::FaganBradley: return {24.74029999443496, 0.0};
  case Ayanamsa::Krishnamurti: return {23.76024004032649, 0.0};
  case Ayanamsa::Citra:
  case Ayanamsa::GalacticCenterMula:
    break;
  }
  return {23.85709235370882, 0.0};
}

// Catalog places are ICRS, as in the comparison star file. Proper motion in
// RA is stored there already multiplied by cos(dec); the unit is 0.001"/year.
struct StarCat {
  double ra0;
  double dec0;
  double pmRaCosDec;
  double pmDec;
};

StarCat spica() {
  const double ra = 15.0 * (13.0 + 25.0 / 60.0 + 11.57937 / 3600.0) * kDeg;
  const double dec = -(11.0 + 9.0 / 60.0 + 40.7501 / 3600.0) * kDeg;
  return {ra, dec, -42.35e-3 * kArcsec, -30.67e-3 * kArcsec};
}

StarCat sgrA() {
  const double ra = 15.0 * (17.0 + 45.0 / 60.0 + 40.03599 / 3600.0) * kDeg;
  const double dec = -(29.0 + 28.1699 / 3600.0) * kDeg;
  return {ra, dec, -2.755718425e-3 * kArcsec, -5.547e-3 * kArcsec};
}

Vec3 starJ2000(const StarCat& star, double jdTt) {
  const double years = (jdTt - kJ2000) / 365.25;
  const double ra = star.ra0 + years * star.pmRaCosDec / std::cos(star.dec0);
  const double dec = star.dec0 + years * star.pmDec;
  const double c = std::cos(dec);
  return {c * std::cos(ra), c * std::sin(ra), std::sin(dec)};
}

// Annual aberration from the Earth-Moon barycentre's heliocentric velocity.
// Light deflection is omitted: these two stars stay more than a degree from
// the Sun, where the deflection is a fraction of an arcsecond.
Vec3 aberrate(Vec3 direction, double jdTt) {
  constexpr double h = 1e-3;
  const Vec3 vEcl = (detail::vsopHeliocentric(2, jdTt + h) - detail::vsopHeliocentric(2, jdTt - h)) / (2.0 * h);
  constexpr double kEps0 = 84381.406 * kArcsec;
  const Vec3 vEq = detail::eclipticToEquatorial(vEcl, kEps0);
  return (direction + vEq / kAuPerDay).normalized();
}

Vec3 starOfDate(const StarCat& star, double jdTt) {
  return detail::precessEquator(aberrate(starJ2000(star, jdTt), jdTt), jdTt);
}

// True Citra: ecliptic longitude of Spica, minus 180°. No nutation.
Angle citraMean(double jdTt) {
  const Vec3 eq = starOfDate(spica(), jdTt);
  const Vec3 ecl = detail::equatorialToEcliptic(eq, detail::meanObliquityRad(jdTt));
  return Angle::fromRad(wrapTwoPi(std::atan2(ecl.y, ecl.x) - kPi));
}

// Galactic center at the middle of Mula: right ascension projected onto the
// ecliptic the way a midheaven is, then minus 246°40'.
Angle galacticMean(double jdTt) {
  const Vec3 eq = starOfDate(sgrA(), jdTt);
  const double ra = std::atan2(eq.y, eq.x);
  const double eps = detail::meanObliquityRad(jdTt);
  const double mc = std::atan2(std::sin(ra), std::cos(ra) * std::cos(eps));
  return Angle::fromRad(wrapTwoPi(mc - 246.6666666667 * kDeg));
}

double generalPrecession(double t) {
  return (5028.796195 + (1.1054348 + (0.00007964 + (-0.000023857 + (-0.0000000383) * t) * t) * t) * t) * t * kArcsec;
}

} // namespace

Angle ayanamsa(Ayanamsa mode, JulianDay jdUt) {
  const double jdTt = utToTt(jdUt).value;
  if (mode == Ayanamsa::Citra) return citraMean(jdTt);
  if (mode == Ayanamsa::GalacticCenterMula) return galacticMean(jdTt);
  const double t = (jdTt - kJ2000) / 36525.0;
  const Ay ay = coeff(mode);
  const double rad = ay.a0Deg * kDeg + generalPrecession(t) + ay.extraArcsecPerCy * t * kArcsec;
  return Angle::fromRad(rad);
}

Angle ayanamsaTrue(Ayanamsa mode, JulianDay jdUt) {
  double dpsi = 0.0;
  double deps = 0.0;
  detail::nutationRad(utToTt(jdUt).value, dpsi, deps);
  return ayanamsa(mode, jdUt) + Angle::fromRad(dpsi);
}

} // namespace eph
