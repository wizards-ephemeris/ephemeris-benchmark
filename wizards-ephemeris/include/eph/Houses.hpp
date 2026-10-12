#pragma once

#include "Math.hpp"
#include "Time.hpp"

namespace eph {

enum class HouseSystem {
  Placidus,
  WholeSign,
  Koch,
  Equal,
  Regiomontanus,
  Campanus,
  Porphyrius,
  Alcabitius,
  // Porphyry sectors, with each cusp on the middle of a sector.
  Sripati,
  // Polich/Page, also called topocentric.
  PolichPage
};

struct Houses {
  Angle cusp[12]; // index 0 is house 1
  Angle asc;
  Angle mc;
  Angle armc;
  Angle vertex;
  // True when a quadrant system did not converge and Porphyry was used.
  // Swiss Ephemeris falls back to Porphyry near the poles; that is matched here.
  bool fallback = false;
};

} // namespace eph
