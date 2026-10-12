#pragma once

#include "Spk.hpp"

namespace eph::detail {

// index: 0 Pluto, 1 Chiron, 2 Ceres, 3 Pallas, 4 Juno, 5 Vesta.
// equatorial is true when `helio` is J2000 equatorial rather than ecliptic.
bool minorHeliocentric(int index, double jdTt, const SpkBook& spk, Vec3& helio, bool& equatorial);

} // namespace eph::detail
