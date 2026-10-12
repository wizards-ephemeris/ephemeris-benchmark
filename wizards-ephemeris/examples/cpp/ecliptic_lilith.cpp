#include "eph/eph.hpp"
#include "examples.hpp"

LilithPairEx ex_ecliptic_lilith() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);

  chart.setLilithMode(eph::LilithMode::Inclined);
  eph::EclipticPos inclined = chart.position(eph::Body::MeanLilith, jd);

  chart.setLilithMode(eph::LilithMode::Ecliptic);
  eph::EclipticPos ecliptic = chart.position(eph::Body::MeanLilith, jd);
  // doc:end
  return {
    inclined.lon.deg(), inclined.lat.deg(),
    ecliptic.lon.deg(), ecliptic.lat.deg()
  };
}
