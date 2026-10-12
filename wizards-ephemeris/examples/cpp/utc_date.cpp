#include "eph/eph.hpp"
#include "examples.hpp"

UtcDateEx ex_utc_date() {
  // doc:begin
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  // doc:end
  return {jd.value};
}
