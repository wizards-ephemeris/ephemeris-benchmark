#include "eph/eph.hpp"
#include "examples.hpp"

CitraGalacticEx ex_citra_galactic() {
  // doc:begin
  eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
  double lahiri = eph::ayanamsaTrue(eph::Ayanamsa::Lahiri, jd).deg();
  double citra = eph::ayanamsaTrue(eph::Ayanamsa::Citra, jd).deg();
  double galactic = eph::ayanamsaTrue(eph::Ayanamsa::GalacticCenterMula, jd).deg();
  // doc:end
  return {lahiri, citra, galactic};
}
