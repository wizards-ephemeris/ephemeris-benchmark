#pragma once

#include "eph/Math.hpp"

namespace eph::detail {

// Geocentric moon, mean ecliptic and equinox of J2000, AU.
Vec3 elpGeocentricAu(double jdTt);

// Mean lunar perigee and ascending node, radians, mean ecliptic of date.
double meanLunarPerigee(double jdTt);
double meanLunarNode(double jdTt);

} // namespace eph::detail
