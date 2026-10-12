#include "eph/eph.hpp"
#include "examples.hpp"

JulianDayEx ex_julian_day() {
  // doc:begin
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  int year = 0;
  int month = 0;
  int day = 0;
  double hour = 0.0;
  eph::revJul(jd, year, month, day, hour);
  // doc:end
  return {jd.value, year, month, day, hour};
}
