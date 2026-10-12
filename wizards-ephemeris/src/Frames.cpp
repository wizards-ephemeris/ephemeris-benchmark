#include "Reduce.hpp"

#include "Series.hpp"
#include "eph/Time.hpp"

#include <cmath>

namespace eph::detail {
namespace {

constexpr double kCentury = 36525.0;

double centuries(double jdTt) {
  return (jdTt - kJ2000) / kCentury;
}

double earthRotation(double jdUt) {
  const double day = std::floor(jdUt);
  const double frac = jdUt - day;
  const double t = jdUt - kJ2000;
  return wrapTwoPi(kTwoPi * (frac + 0.7790572732640 + 0.00273781191135448 * t));
}

double gmstRad(double jdUt, double jdTt) {
  const double t = centuries(jdTt);
  const double extra = (0.014506 + (4612.156534 + (1.3915817 + (-0.00000044
    + (-0.000029956 + (-0.0000000368) * t) * t) * t) * t) * t) * kArcsec;
  return wrapTwoPi(earthRotation(jdUt) + extra);
}

// IAU 2006 Fukushima-Williams angles, radians, without nutation.
void fwAngles(double t, double& gam, double& phi, double& psi, double& epsA) {
  constexpr double kEps0 = 84381.406 * kArcsec;
  gam = (10.556403 + (0.4932044 + (-0.00031238 + (-0.000002788 + 0.0000000260 * t) * t) * t) * t) * t * kArcsec;
  phi = kEps0 + (-46.811015 + (0.0511269 + (0.00053289 + (-0.000000440 - 0.0000000176 * t) * t) * t) * t) * t * kArcsec;
  psi = (5038.481507 + (1.5584176 + (-0.00018522 + (-0.000026452 - 0.0000000148 * t) * t) * t) * t) * t * kArcsec;
  epsA = (84381.406 + (-46.836769 + (-0.0001831 + (0.00200340 + (-0.000000576
    + (-0.0000000434) * t) * t) * t) * t) * t) * kArcsec;
}

Vec3 precessNutate(Vec3 eqJ2000, double jdTt) {
  const double t = centuries(jdTt);
  double gam = 0.0;
  double phi = 0.0;
  double psi = 0.0;
  double epsA = 0.0;
  fwAngles(t, gam, phi, psi, epsA);
  double dpsi = 0.0;
  double deps = 0.0;
  iauNutation2000b(t, dpsi, deps);
  // NxPxB = R1(-eps) R3(-psi) R1(phi) R3(gamma), applied to a GCRS vector.
  Vec3 v = Mat3::sofaRz(gam) * eqJ2000;
  v = Mat3::sofaRx(phi) * v;
  v = Mat3::sofaRz(-(psi + dpsi)) * v;
  v = Mat3::sofaRx(-(epsA + deps)) * v;
  return v;
}

} // namespace

Vec3 precessEquator(Vec3 eqJ2000, double jdTt) {
  const double t = centuries(jdTt);
  double gam = 0.0;
  double phi = 0.0;
  double psi = 0.0;
  double epsA = 0.0;
  fwAngles(t, gam, phi, psi, epsA);
  Vec3 v = Mat3::sofaRz(gam) * eqJ2000;
  v = Mat3::sofaRx(phi) * v;
  v = Mat3::sofaRz(-psi) * v;
  v = Mat3::sofaRx(-epsA) * v;
  return v;
}

Vec3 eclipticToEquatorial(Vec3 ecl, double eps) {
  const double c = std::cos(eps);
  const double s = std::sin(eps);
  return {ecl.x, c * ecl.y - s * ecl.z, s * ecl.y + c * ecl.z};
}

Vec3 equatorialToEcliptic(Vec3 eq, double eps) {
  const double c = std::cos(eps);
  const double s = std::sin(eps);
  return {eq.x, c * eq.y + s * eq.z, -s * eq.y + c * eq.z};
}

double meanObliquityRad(double jdTt) {
  const double t = centuries(jdTt);
  return (84381.406 + (-46.836769 + (-0.0001831 + (0.00200340 + (-0.000000576
    + (-0.0000000434) * t) * t) * t) * t) * t) * kArcsec;
}

void nutationRad(double jdTt, double& dpsi, double& deps) {
  iauNutation2000b(centuries(jdTt), dpsi, deps);
}

double gastRad(double jdUt, double jdTt) {
  double dpsi = 0.0;
  double deps = 0.0;
  nutationRad(jdTt, dpsi, deps);
  const double eps = meanObliquityRad(jdTt);
  return wrapTwoPi(gmstRad(jdUt, jdTt) + dpsi * std::cos(eps));
}

SkyVec reduceEquatorial(Vec3 geoEqJ2000, Vec3 velEqAuPerDay, double jdTt) {
  const double dist = geoEqJ2000.length();
  Vec3 u = geoEqJ2000.normalized();
  u = (u + velEqAuPerDay / kAuPerDay).normalized();
  const Vec3 eq = precessNutate(u * dist, jdTt);
  double dpsi = 0.0;
  double deps = 0.0;
  nutationRad(jdTt, dpsi, deps);
  const double epsTrue = meanObliquityRad(jdTt) + deps;
  SkyVec out;
  out.eq = eq;
  out.ecl = equatorialToEcliptic(eq, epsTrue);
  return out;
}

SkyVec reduceEcliptic(Vec3 geoEclJ2000, Vec3 velEclAuPerDay, double jdTt) {
  constexpr double kEps0 = 84381.406 * kArcsec;
  return reduceEquatorial(
    eclipticToEquatorial(geoEclJ2000, kEps0),
    eclipticToEquatorial(velEclAuPerDay, kEps0),
    jdTt);
}

} // namespace eph::detail
