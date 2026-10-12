#include "eph/eph.hpp"
#include "examples.hpp"

PolarEx ex_polar_fallback() {
  // doc:begin
  eph::Ephemeris chart;
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  eph::Houses placidus = chart.houses(jd, 80.0, 18.67, eph::HouseSystem::Placidus);
  eph::Houses koch = chart.houses(jd, 80.0, 18.67, eph::HouseSystem::Koch);
  // doc:end
  return {placidus.fallback, koch.fallback};
}
