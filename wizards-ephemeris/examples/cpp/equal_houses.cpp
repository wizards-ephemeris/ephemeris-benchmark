#include "eph/eph.hpp"
#include "examples.hpp"

namespace {

HousesEx pack(const eph::Houses& h) {
  HousesEx out;
  for (int i = 0; i < 12; ++i) out.cusps[static_cast<std::size_t>(i)] = h.cusp[i].deg();
  out.asc = h.asc.deg();
  out.mc = h.mc.deg();
  out.armc = h.armc.deg();
  out.vertex = h.vertex.deg();
  out.fallback = h.fallback;
  return out;
}

} // namespace

HousesEx ex_equal_houses() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Equal);
  // doc:end
  return pack(h);
}
