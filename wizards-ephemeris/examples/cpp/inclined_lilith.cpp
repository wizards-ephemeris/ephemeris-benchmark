#include "eph/eph.hpp"
#include "examples.hpp"

LilithEx ex_inclined_lilith() {
  // doc:begin
  eph::Ephemeris chart;
  chart.setLilithMode(eph::LilithMode::Inclined);
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::EclipticPos lilith = chart.position(eph::Body::MeanLilith, jd);
  // doc:end
  return {lilith.lon.deg(), lilith.lat.deg()};
}
