#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

TEST_CASE("Knurów demo sun is in Aquarius") {
  const double jd = eph::julianDayUt(1988, 1, 30, 15.0 + 10.0 / 60.0).value;
  emph::reference().setSidereal(false, eph::Ayanamsa::Lahiri);
  for (emph::Adapter* subject : emph::subjects()) {
    subject->setSidereal(false, eph::Ayanamsa::Lahiri);
    const emph::SkyPlace sun = subject->position(eph::Body::Sun, jd);
    CHECK(sun.lonDeg > 309.0);
    CHECK(sun.lonDeg < 311.0);
    const emph::SkyPlace ref = emph::reference().position(eph::Body::Sun, jd);
    CHECK(arcsec(sun.lonDeg, ref.lonDeg) < 1.0);
  }
}
