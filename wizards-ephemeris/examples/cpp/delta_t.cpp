#include "eph/eph.hpp"
#include "examples.hpp"

DeltaTEx ex_delta_t() {
  // doc:begin
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  double seconds = eph::deltaTSeconds(jd);
  // doc:end
  return {seconds};
}
