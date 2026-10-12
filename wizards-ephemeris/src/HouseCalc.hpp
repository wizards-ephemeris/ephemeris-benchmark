#pragma once

#include "eph/Houses.hpp"

namespace eph {

Houses computeHouses(double jdUt, double latDeg, double lonDegEast, HouseSystem sys);

} // namespace eph
