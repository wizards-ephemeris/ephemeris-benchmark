#include <doctest/doctest.h>

#include "eph/Math.hpp"

#include <cmath>

using namespace eph;

TEST_CASE("vectors and rotations") {
  const Vec3 a{1.0, 2.0, 3.0};
  const Vec3 b{4.0, -1.0, 2.0};
  CHECK(dot(a, b) == doctest::Approx(1.0 * 4.0 + 2.0 * -1.0 + 3.0 * 2.0));
  const Vec3 c = cross(a, b);
  CHECK(dot(c, a) == doctest::Approx(0.0).epsilon(1e-12));
  CHECK(dot(c, b) == doctest::Approx(0.0).epsilon(1e-12));
  CHECK((a + b).x == doctest::Approx(5.0));
  CHECK((a * 2.0).y == doctest::Approx(4.0));
  CHECK(a.normalized().length() == doctest::Approx(1.0));

  const Vec3 x{1.0, 0.0, 0.0};
  const Vec3 y = Mat3::rotZ(kPi / 2.0) * x;
  CHECK(y.x == doctest::Approx(0.0).epsilon(1e-12));
  CHECK(y.y == doctest::Approx(1.0));

  const Mat3 there = Mat3::rotX(0.3);
  const Mat3 back = Mat3::rotX(-0.3);
  const Vec3 v{0.2, -0.4, 0.7};
  const Vec3 round = back * (there * v);
  CHECK(round.x == doctest::Approx(v.x));
  CHECK(round.y == doctest::Approx(v.y));
  CHECK(round.z == doctest::Approx(v.z));

  const Angle lon = Angle::fromDeg(370.0);
  CHECK(lon.wrapped().deg() == doctest::Approx(10.0));
  CHECK(sepDeg(350.0, 10.0) == doctest::Approx(20.0));
}
