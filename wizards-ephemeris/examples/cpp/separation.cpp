#include "eph/eph.hpp"
#include "examples.hpp"

SeparationEx ex_separation() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::EclipticPos sun = chart.position(eph::Body::Sun, jd);
  double quarter = eph::sepDeg(10.0, 350.0);
  double same = eph::sepDeg(sun.lon.deg(), sun.lon.deg());
  // doc:end
  return {quarter, same};
}
