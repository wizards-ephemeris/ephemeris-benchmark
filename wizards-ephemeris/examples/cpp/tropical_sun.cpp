#include "eph/eph.hpp"
#include "examples.hpp"

EclipticEx ex_tropical_sun() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::EclipticPos sun = chart.position(eph::Body::Sun, jd);
  // doc:end
  return {sun.lon.deg(), sun.lat.deg(), sun.lonSpeed, sun.distAu};
}
