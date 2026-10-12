#pragma once

#include "Ayanamsa.hpp"

namespace eph {

// Mean subtracts only the precessional ayanamsa, so nutation stays in the
// sidereal longitude. True also subtracts nutation in longitude, which is
// what Swiss Ephemeris does for a sidereal place.
enum class SiderealNutation {
  Mean,
  True
};

// SnapThenShift snaps the tropical ascendant, then subtracts the ayanamsa,
// so sidereal cusps are not on sign boundaries. ShiftThenSnap subtracts
// first and then snaps, which is the Swiss Ephemeris whole-sign order.
enum class WholeSignSnap {
  SnapThenShift,
  ShiftThenSnap
};

// Ecliptic keeps mean Lilith on the ecliptic (latitude 0). Inclined is the
// mean apogee on the lunar orbit, latitude up to about 5.145°, which is
// the Swiss Ephemeris mean apogee.
enum class LilithMode {
  Ecliptic,
  Inclined
};

struct SiderealMethod {
  Ayanamsa ayanamsa = Ayanamsa::Lahiri;
  SiderealNutation nutation = SiderealNutation::True;
  WholeSignSnap wholeSignSnap = WholeSignSnap::ShiftThenSnap;
};

} // namespace eph
