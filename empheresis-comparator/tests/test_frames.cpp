#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

#include <string>

TEST_CASE("equatorial place versus reference") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  const std::vector<double> jds = quarterlyJds();
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    Worst place;
    Worst speed;
    for (eph::Body body : allBodies()) {
      const double placeLimit = placeBoundArcsec(body);
      const double speedLimit = speedBoundArcsec(body);
      for (double jd : jds) {
        const emph::SkyPlace eq = subject->equatorial(body, jd);
        const emph::SkyPlace got = ref.equatorial(body, jd);
        const std::string name = bodyName(body);
        place.consider(arcsec(eq.raDeg, got.raDeg) / placeLimit, jd, name + " ra");
        place.consider(arcsec(eq.decDeg, got.decDeg) / placeLimit, jd, name + " dec");
        speed.consider(speedArcsec(eq.raSpeedDegPerDay, got.raSpeedDegPerDay) / speedLimit, jd, name + " ra speed");
      }
    }
    INFO("place ratio " << place.sep << " " << place.note << " jd " << place.jd
      << " speed ratio " << speed.sep << " " << speed.note << " jd " << speed.jd);
    CHECK(place.sep < 1.0);
    CHECK(speed.sep < 1.0);
  }
}
