#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

TEST_CASE("pluto chiron and asteroids versus reference") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  const eph::Body bodies[] = {
    eph::Body::Pluto, eph::Body::Chiron, eph::Body::Ceres,
    eph::Body::Pallas, eph::Body::Juno, eph::Body::Vesta
  };
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    const emph::SkyPlace pluto = subject->position(eph::Body::Pluto, 2451545.0);
    const emph::SkyPlace refPluto = ref.position(eph::Body::Pluto, 2451545.0);
    REQUIRE(arcsec(pluto.lonDeg, refPluto.lonDeg) < 30.0);
    Worst worst;
    for (double jd = 2415021.0; jd <= 2488070.0; jd += 60.0) {
      for (eph::Body body : bodies) {
        const emph::SkyPlace pos = subject->position(body, jd);
        const emph::SkyPlace got = ref.position(body, jd);
        const double place = placeBoundArcsec(body);
        const double rate = speedBoundArcsec(body);
        worst.consider(arcsec(pos.lonDeg, got.lonDeg) / place, jd, "scaled lon");
        worst.consider(arcsec(pos.latDeg, got.latDeg) / place, jd, "scaled lat");
        worst.consider(speedArcsec(pos.speedDegPerDay, got.speedDegPerDay) / rate, jd, "scaled speed");
      }
    }
    INFO("worst ratio " << worst.sep << " " << worst.note << " jd " << worst.jd);
    CHECK(worst.sep < 1.0);
  }
}
