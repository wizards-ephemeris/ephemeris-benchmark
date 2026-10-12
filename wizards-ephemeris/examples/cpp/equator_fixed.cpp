#include "eph/eph.hpp"
#include "examples.hpp"

EquatorFixedEx ex_equator_fixed() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::EclipticPos tropical = chart.position(eph::Body::Sun, jd);
  eph::EquatorialPos tropicalSky = chart.equatorial(eph::Body::Sun, jd);

  chart.setSidereal(true, eph::SiderealMethod{
    eph::Ayanamsa::Lahiri,
    eph::SiderealNutation::True,
    eph::WholeSignSnap::ShiftThenSnap
  });
  eph::EclipticPos sidereal = chart.position(eph::Body::Sun, jd);
  eph::EquatorialPos siderealSky = chart.equatorial(eph::Body::Sun, jd);
  // doc:end
  return {
    tropical.lon.deg(), sidereal.lon.deg(),
    tropicalSky.ra.deg(), siderealSky.ra.deg()
  };
}
