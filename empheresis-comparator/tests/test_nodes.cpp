#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

#include <cmath>

TEST_CASE("true node and mean lilith versus reference") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    INFO("subject " << subject->name());
    Worst node;
    Worst lilith;
    Worst lilithLat;
    Worst nodeSpeed;
    Worst lilithSpeed;
    for (double jd : monthlyJds()) {
      const emph::SkyPlace ours = subject->position(eph::Body::TrueNode, jd);
      const emph::SkyPlace got = ref.position(eph::Body::TrueNode, jd);
      node.consider(arcsec(ours.lonDeg, got.lonDeg), jd, "node lon");
      node.consider(arcsec(ours.latDeg, got.latDeg), jd, "node lat");
      nodeSpeed.consider(speedArcsec(ours.speedDegPerDay, got.speedDegPerDay), jd, "node speed");
      const emph::SkyPlace mean = subject->position(eph::Body::MeanLilith, jd);
      const emph::SkyPlace refMean = ref.position(eph::Body::MeanLilith, jd);
      lilith.consider(arcsec(mean.lonDeg, refMean.lonDeg), jd, "lilith lon");
      lilithLat.consider(arcsec(mean.latDeg, refMean.latDeg), jd, "lilith lat");
      lilithSpeed.consider(speedArcsec(mean.speedDegPerDay, refMean.speedDegPerDay), jd, "lilith speed");
    }
    INFO("node " << node.sep << "\" " << node.note << " jd " << node.jd
      << " node speed " << nodeSpeed.sep << "\"/day jd " << nodeSpeed.jd
      << " lilith " << lilith.sep << "\" " << lilith.note << " jd " << lilith.jd
      << " lilith lat " << lilithLat.sep << "\" jd " << lilithLat.jd
      << " lilith speed " << lilithSpeed.sep << "\"/day jd " << lilithSpeed.jd);
    CHECK(node.sep < 10.0);
    CHECK(nodeSpeed.sep < 120.0);
    CHECK(lilith.sep < 2.0);
    CHECK(lilithLat.sep < 2.0);
    CHECK(lilithSpeed.sep < 2.0);
  }
}

TEST_CASE("ecliptic lilith stays on the ecliptic") {
  emph::Adapter& ref = emph::reference();
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  ref.setLilithMode(eph::LilithMode::Ecliptic);
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    subject->setLilithMode(eph::LilithMode::Ecliptic);
    Worst lon;
    Worst lat;
    for (double jd : monthlyJds()) {
      const emph::SkyPlace mean = subject->position(eph::Body::MeanLilith, jd);
      const emph::SkyPlace got = ref.position(eph::Body::MeanLilith, jd);
      lon.consider(arcsec(mean.lonDeg, got.lonDeg), jd, "lon");
      lat.consider(std::fabs(mean.latDeg) * 3600.0, jd, "lat");
    }
    INFO("lon " << lon.sep << "\" jd " << lon.jd << " lat " << lat.sep << "\" jd " << lat.jd);
    CHECK(lon.sep < 2.0);
    CHECK(lat.sep < 0.01);
    subject->setLilithMode(eph::LilithMode::Inclined);
    ref.setLilithMode(eph::LilithMode::Inclined);
  }
}
