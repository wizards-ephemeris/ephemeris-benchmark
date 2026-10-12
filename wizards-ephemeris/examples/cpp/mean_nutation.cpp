#include "eph/eph.hpp"
#include "examples.hpp"

MeanNutationEx ex_mean_nutation() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  chart.setSidereal(true, eph::SiderealMethod{
    eph::Ayanamsa::Lahiri,
    eph::SiderealNutation::True,
    eph::WholeSignSnap::ShiftThenSnap
  });
  eph::EclipticPos asTrue = chart.position(eph::Body::Sun, jd);

  chart.setSidereal(true, eph::SiderealMethod{
    eph::Ayanamsa::Lahiri,
    eph::SiderealNutation::Mean,
    eph::WholeSignSnap::ShiftThenSnap
  });
  eph::EclipticPos asMean = chart.position(eph::Body::Sun, jd);
  double ayTrue = eph::ayanamsaTrue(eph::Ayanamsa::Lahiri, jd).deg();
  double ayMean = eph::ayanamsa(eph::Ayanamsa::Lahiri, jd).deg();
  // doc:end
  return {asTrue.lon.deg(), asMean.lon.deg(), ayTrue, ayMean};
}
