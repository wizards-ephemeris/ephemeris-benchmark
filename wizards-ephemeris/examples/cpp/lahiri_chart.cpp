#include "eph/eph.hpp"
#include "examples.hpp"

LahiriEx ex_lahiri_chart() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::EclipticPos tropical = chart.position(eph::Body::Sun, jd);
  double ayanamsa = eph::ayanamsaTrue(eph::Ayanamsa::Lahiri, jd).deg();

  chart.setSidereal(true, eph::SiderealMethod{
    eph::Ayanamsa::Lahiri,
    eph::SiderealNutation::True,
    eph::WholeSignSnap::ShiftThenSnap
  });
  eph::EclipticPos sidereal = chart.position(eph::Body::Sun, jd);
  // doc:end
  return {tropical.lon.deg(), sidereal.lon.deg(), ayanamsa};
}
