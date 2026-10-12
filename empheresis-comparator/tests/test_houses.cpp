#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

namespace {

void porphyryAgrees(emph::Adapter& subject, const emph::HousePlace& got, double date, double lat, double lon, Worst& worst) {
  if (!got.fallback) return;
  const emph::HousePlace porphyry = subject.houses(date, lat, lon, eph::HouseSystem::Porphyrius);
  for (int i = 0; i < 12; ++i) {
    worst.consider(arcsec(got.cuspDeg[i], porphyry.cuspDeg[i]), date, "fallback cusp");
  }
}

} // namespace

TEST_CASE("houses versus reference") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  const double jd = eph::julianDayUt(1988, 1, 30, 15.0 + 10.0 / 60.0).value;
  const double lats[] = {-80.0, -60.0, -30.0, 0.0, 30.0, 50.22, 66.0, 80.0};
  const double lons[] = {-120.0, 0.0, 18.67, 140.0};
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    const emph::HousePlace h = subject->houses(jd, 50.2197, 18.6742, eph::HouseSystem::Placidus);
    const emph::HousePlace refH = ref.houses(jd, 50.2197, 18.6742, eph::HouseSystem::Placidus);
    REQUIRE(arcsec(h.ascDeg, refH.ascDeg) < 30.0);

    Worst angles;
    Worst cusps;
    Worst fallback;
    // Past 2050 the clock-correction polynomials diverge by several seconds.
    // Near the polar circle that shifts a cusp by an arcminute, which is the
    // time scale and not the house geometry.
    for (double date = 2415021.0; date <= 2469807.5; date += 180.0) {
      for (double lat : lats) {
        for (double lon : lons) {
          for (eph::HouseSystem sys : allHouseSystems()) {
            const emph::HousePlace got = subject->houses(date, lat, lon, sys);
            const emph::HousePlace expect = ref.houses(date, lat, lon, sys);
            considerHouses(angles, cusps, got, expect, date);
            porphyryAgrees(*subject, got, date, lat, lon, fallback);
          }
        }
      }
    }
    INFO("angles " << angles.sep << "\" " << angles.note << " jd " << angles.jd
      << " cusps " << cusps.sep << "\" jd " << cusps.jd
      << " fallback " << fallback.sep << "\" jd " << fallback.jd);
    CHECK(angles.sep < 5.0);
    CHECK(cusps.sep < 10.0);
    CHECK(fallback.sep < 0.01);
  }
}

TEST_CASE("houses on a monthly mid-latitude slice") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  const std::vector<double> dates = monthlyHouseJds();
  const double lons[] = {-120.0, 0.0, 18.67, 140.0};
  const double lat = 50.22;
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    Worst angles;
    Worst cusps;
    for (double date : dates) {
      for (double lon : lons) {
        for (eph::HouseSystem sys : allHouseSystems()) {
          const emph::HousePlace got = subject->houses(date, lat, lon, sys);
          const emph::HousePlace expect = ref.houses(date, lat, lon, sys);
          considerHouses(angles, cusps, got, expect, date);
        }
      }
    }
    INFO("angles " << angles.sep << "\" " << angles.note << " jd " << angles.jd
      << " cusps " << cusps.sep << "\" jd " << cusps.jd);
    CHECK(angles.sep < 5.0);
    CHECK(cusps.sep < 10.0);
  }
}

TEST_CASE("polar placidus and koch fall back to porphyry") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  const double lats[] = {-89.0, 89.0};
  const double lons[] = {-120.0, 0.0, 18.67, 140.0};
  const eph::HouseSystem systems[] = {eph::HouseSystem::Placidus, eph::HouseSystem::Koch};
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    Worst cusps;
    int samples = 0;
    int fallen = 0;
    for (double date = 2415021.0; date <= 2469807.5; date += 180.0) {
      for (double lat : lats) {
        for (double lon : lons) {
          for (eph::HouseSystem sys : systems) {
            const emph::HousePlace got = subject->houses(date, lat, lon, sys);
            const emph::HousePlace expect = ref.houses(date, lat, lon, sys);
            ++samples;
            if (got.fallback) ++fallen;
            for (int i = 0; i < 12; ++i) {
              cusps.consider(arcsec(got.cuspDeg[i], expect.cuspDeg[i]), date, "polar cusp");
            }
          }
        }
      }
    }
    INFO("cusps " << cusps.sep << "\" jd " << cusps.jd << " fallback " << fallen << "/" << samples);
    CHECK(cusps.sep < 10.0);
    CHECK(fallen == samples);
  }
}

TEST_CASE("sripati cusps sit on porphyry sector midpoints") {
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    const double jd = 2451545.0;
    const emph::HousePlace sri = subject->houses(jd, 50.22, 18.67, eph::HouseSystem::Sripati);
    const emph::HousePlace por = subject->houses(jd, 50.22, 18.67, eph::HouseSystem::Porphyrius);
    // House 1 is the middle of the Porphyry sector from cusp 12 to the ascendant.
    const double forward = eph::wrap360(por.ascDeg - por.cuspDeg[11]);
    const double expect = eph::wrap360(por.cuspDeg[11] + 0.5 * forward);
    CHECK(arcsec(sri.cuspDeg[0], expect) < 0.01);
    CHECK(arcsec(sri.ascDeg, por.ascDeg) < 0.01);
    CHECK(arcsec(sri.cuspDeg[0], sri.ascDeg) > 1.0);
    const emph::HousePlace page = subject->houses(jd, 50.22, 18.67, eph::HouseSystem::PolichPage);
    CHECK(arcsec(page.cuspDeg[0], page.ascDeg) < 0.01);
    CHECK(arcsec(page.cuspDeg[9], page.mcDeg) < 0.01);
  }
}
