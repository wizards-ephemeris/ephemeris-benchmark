#pragma once

#include "Time.hpp"
#include "Math.hpp"

namespace eph {

enum class Ayanamsa {
  Lahiri,
  Raman,
  FaganBradley,
  Krishnamurti,
  // True Citra: Spica at 180° sidereal. Mean of that star ayanamsa.
  Citra,
  // Galactic centre at the middle of Mula (Wilhelm). Mean of that ayanamsa.
  GalacticCenterMula
};

// Mean ayanamsa. Nutation is not included.
Angle ayanamsa(Ayanamsa mode, JulianDay jdUt);

// Mean ayanamsa plus nutation in longitude. This is the offset Swiss
// Ephemeris subtracts for a sidereal place.
Angle ayanamsaTrue(Ayanamsa mode, JulianDay jdUt);

} // namespace eph
