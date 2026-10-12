#include "eph/eph.hpp"
#include "examples.hpp"

PolichEx ex_polich_page() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::PolichPage);
  // doc:end
  return {h.cusp[0].deg(), h.cusp[9].deg(), h.asc.deg(), h.mc.deg()};
}
