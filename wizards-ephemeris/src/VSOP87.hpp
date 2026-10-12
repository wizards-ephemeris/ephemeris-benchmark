#pragma once

#include "eph/Math.hpp"

namespace eph::detail {

// Heliocentric dynamical ecliptic and equinox of J2000, AU.
// planet: 0 Mercury ... 2 Earth-Moon barycentre ... 7 Neptune.
Vec3 vsopHeliocentric(int planet, double jdTt);

} // namespace eph::detail
