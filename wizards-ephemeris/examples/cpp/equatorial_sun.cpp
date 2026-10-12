#include "eph/eph.hpp"
#include "examples.hpp"

EquatorialEx ex_equatorial_sun() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::EquatorialPos sun = chart.equatorial(eph::Body::Sun, jd);
  // doc:end
  return {sun.ra.deg(), sun.dec.deg(), sun.raSpeed};
}
