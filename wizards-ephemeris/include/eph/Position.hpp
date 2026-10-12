#pragma once

#include "Math.hpp"

namespace eph {

struct EclipticPos {
  Angle lon;
  Angle lat;
  double lonSpeed = 0.0; // degrees per day
  double distAu = 0.0;
};

struct EquatorialPos {
  Angle ra;
  Angle dec;
  double raSpeed = 0.0; // degrees per day
};

} // namespace eph
