#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

TEST_CASE("sun through neptune versus reference") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  const eph::Body bodies[] = {
    eph::Body::Sun, eph::Body::Mercury, eph::Body::Venus, eph::Body::Mars,
    eph::Body::Jupiter, eph::Body::Saturn, eph::Body::Uranus, eph::Body::Neptune
  };
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    const emph::SkyPlace sun = subject->position(eph::Body::Sun, 2451545.0);
    const emph::SkyPlace refSun = ref.position(eph::Body::Sun, 2451545.0);
    REQUIRE(arcsec(sun.lonDeg, refSun.lonDeg) < 5.0);
    Worst inner;
    Worst uranus;
    Worst neptune;
    for (double jd : monthlyJds()) {
      for (eph::Body body : bodies) {
        const emph::SkyPlace pos = subject->position(body, jd);
        const emph::SkyPlace got = ref.position(body, jd);
        Worst& slot = body == eph::Body::Neptune ? neptune
          : body == eph::Body::Uranus ? uranus : inner;
        slot.consider(arcsec(pos.lonDeg, got.lonDeg), jd, "lon");
        slot.consider(arcsec(pos.latDeg, got.latDeg), jd, "lat");
        slot.consider(speedArcsec(pos.speedDegPerDay, got.speedDegPerDay), jd, "speed");
      }
    }
    INFO("inner " << inner.sep << "\" jd " << inner.jd
      << " uranus " << uranus.sep << "\" jd " << uranus.jd
      << " neptune " << neptune.sep << "\" jd " << neptune.jd);
    CHECK(inner.sep < 1.0);
    CHECK(uranus.sep < 2.0);
    CHECK(neptune.sep < 3.0);
  }
}
