#pragma once

#include "eph/Math.hpp"

namespace eph::detail {

struct SkyVec {
  Vec3 ecl; // apparent, true ecliptic of date
  Vec3 eq;  // apparent, true equator of date
};

double meanObliquityRad(double jdTt);
void nutationRad(double jdTt, double& dpsi, double& deps);
double gastRad(double jdUt, double jdTt);

// IAU 2006 precession only. Nutation stays out, so a mean place can be formed.
Vec3 precessEquator(Vec3 eqJ2000, double jdTt);

// Geometric J2000 vector plus the observer velocity in that same frame (AU/day).
SkyVec reduceEquatorial(Vec3 geoEqJ2000, Vec3 velEqAuPerDay, double jdTt);
SkyVec reduceEcliptic(Vec3 geoEclJ2000, Vec3 velEclAuPerDay, double jdTt);

Vec3 eclipticToEquatorial(Vec3 ecl, double eps);
Vec3 equatorialToEcliptic(Vec3 eq, double eps);

} // namespace eph::detail
