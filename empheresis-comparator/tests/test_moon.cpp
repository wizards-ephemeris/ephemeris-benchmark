#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

TEST_CASE("moon versus reference") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    const emph::SkyPlace moon = subject->position(eph::Body::Moon, 2451545.0);
    const emph::SkyPlace refMoon = ref.position(eph::Body::Moon, 2451545.0);
    REQUIRE(arcsec(moon.lonDeg, refMoon.lonDeg) < 30.0);
    Worst worst;
    Worst speed;
    for (double jd : monthlyJds()) {
      const emph::SkyPlace pos = subject->position(eph::Body::Moon, jd);
      const emph::SkyPlace got = ref.position(eph::Body::Moon, jd);
      worst.consider(arcsec(pos.lonDeg, got.lonDeg), jd, "lon");
      worst.consider(arcsec(pos.latDeg, got.latDeg), jd, "lat");
      speed.consider(speedArcsec(pos.speedDegPerDay, got.speedDegPerDay), jd, "speed");
    }
    INFO("worst " << worst.sep << "\" " << worst.note << " jd " << worst.jd
      << " speed " << speed.sep << "\"/day jd " << speed.jd);
    CHECK(worst.sep < 10.0);
    CHECK(speed.sep < 30.0);
  }
}
