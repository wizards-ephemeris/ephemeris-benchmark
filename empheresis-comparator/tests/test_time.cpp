#include <doctest/doctest.h>

#include "SwissRef.hpp"

#include <cmath>

TEST_CASE("julian day matches Swiss Ephemeris") {
  Swiss swiss;
  CHECK(eph::julianDayUt(2000, 1, 1, 12.0).value == doctest::Approx(2451545.0));
  const int years[] = {1900, 1950, 1988, 2000, 2024, 2100};
  for (int year : years) {
    for (int month = 1; month <= 12; month += 5) {
      const double hour = 15.0 + 10.0 / 60.0;
      const double ours = eph::julianDayUt(year, month, 30, hour).value;
      const double ref = swe_julday(year, month, 30, hour, SE_GREG_CAL);
      CHECK(ours == doctest::Approx(ref).epsilon(1e-12));
      int y = 0;
      int m = 0;
      int d = 0;
      double h = 0.0;
      eph::revJul(eph::JulianDay{ours}, y, m, d, h);
      int ry = 0;
      int rm = 0;
      int rd = 0;
      double rh = 0.0;
      swe_revjul(ref, SE_GREG_CAL, &ry, &rm, &rd, &rh);
      CHECK(y == ry);
      CHECK(m == rm);
      CHECK(d == rd);
      CHECK(h == doctest::Approx(rh).epsilon(1e-8));
    }
  }
}

TEST_CASE("delta T stays near Swiss") {
  Swiss swiss;
  double worst = 0.0;
  double at = 0.0;
  for (double jd = 2415021.0; jd <= 2488070.0; jd += 100.0) {
    const double ours = eph::deltaTSeconds(eph::JulianDay{jd});
    const double ref = swe_deltat(jd) * 86400.0;
    const double d = std::fabs(ours - ref);
    if (d > worst) {
      worst = d;
      at = jd;
    }
  }
  INFO("worst delta T " << worst << " s at jd " << at);
  // Espenak's measured polynomials and Swiss diverge by a few seconds after 2050.
  CHECK(worst < 15.0);
}
