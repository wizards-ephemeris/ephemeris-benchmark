#include "eph/eph.hpp"
#include "examples.hpp"

AnglesEx ex_armc_fixed() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::Houses tropical = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Placidus);

  chart.setSidereal(true, eph::SiderealMethod{
    eph::Ayanamsa::Lahiri,
    eph::SiderealNutation::True,
    eph::WholeSignSnap::ShiftThenSnap
  });
  eph::Houses sidereal = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Placidus);
  // doc:end
  return {
    tropical.armc.deg(), sidereal.armc.deg(),
    tropical.asc.deg(), sidereal.asc.deg()
  };
}
