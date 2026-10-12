#include "VSOP87.hpp"

#include "Series.hpp"

namespace eph::detail {

Vec3 vsopHeliocentric(int planet, double jdTt) {
  const double t = (jdTt - kJ2000) / 365250.0;
  const double lon = wrapTwoPi(vsopComponent(planet, 0, t));
  const double lat = vsopComponent(planet, 1, t);
  const double rad = vsopComponent(planet, 2, t);
  const double cb = std::cos(lat);
  return {rad * cb * std::cos(lon), rad * cb * std::sin(lon), rad * std::sin(lat)};
}

} // namespace eph::detail
