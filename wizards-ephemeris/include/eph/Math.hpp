#pragma once

#include <cmath>

namespace eph {

inline constexpr double kPi = 3.141592653589793238462643383279502884;
inline constexpr double kTwoPi = 2.0 * kPi;
inline constexpr double kDeg = kPi / 180.0;
inline constexpr double kArcsec = kDeg / 3600.0;
inline constexpr double kJ2000 = 2451545.0;
inline constexpr double kAuKm = 149597870.7;
inline constexpr double kAuPerDay = 299792.458 * 86400.0 / kAuKm;

inline double wrapTwoPi(double rad) {
  double v = std::fmod(rad, kTwoPi);
  if (v < 0.0) v += kTwoPi;
  return v;
}

inline double wrapPi(double rad) {
  double v = std::fmod(rad + kPi, kTwoPi);
  if (v < 0.0) v += kTwoPi;
  return v - kPi;
}

inline double wrap360(double deg) {
  double v = std::fmod(deg, 360.0);
  if (v < 0.0) v += 360.0;
  return v;
}

// Shortest separation on a circle, in degrees, in [0, 180].
inline double sepDeg(double aDeg, double bDeg) {
  double d = std::fabs(wrap360(aDeg) - wrap360(bDeg));
  if (d > 180.0) d = 360.0 - d;
  return d;
}

struct Angle {
  double rad = 0.0;

  static constexpr Angle fromDeg(double deg) { return Angle{deg * kDeg}; }
  static constexpr Angle fromRad(double rad) { return Angle{rad}; }

  constexpr double deg() const { return rad / kDeg; }
  Angle wrapped() const { return Angle{wrapTwoPi(rad)}; }

  friend Angle operator+(Angle a, Angle b) { return Angle{a.rad + b.rad}; }
  friend Angle operator-(Angle a, Angle b) { return Angle{a.rad - b.rad}; }
  friend Angle operator*(Angle a, double s) { return Angle{a.rad * s}; }
  friend Angle operator*(double s, Angle a) { return Angle{a.rad * s}; }
  friend Angle operator/(Angle a, double s) { return Angle{a.rad / s}; }
  Angle operator-() const { return Angle{-rad}; }
  Angle& operator+=(Angle o) { rad += o.rad; return *this; }
  Angle& operator-=(Angle o) { rad -= o.rad; return *this; }
};

struct Vec3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  friend Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
  friend Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
  friend Vec3 operator*(Vec3 a, double s) { return {a.x * s, a.y * s, a.z * s}; }
  friend Vec3 operator*(double s, Vec3 a) { return a * s; }
  friend Vec3 operator/(Vec3 a, double s) { return {a.x / s, a.y / s, a.z / s}; }
  Vec3 operator-() const { return {-x, -y, -z}; }
  Vec3& operator+=(Vec3 o) { x += o.x; y += o.y; z += o.z; return *this; }
  Vec3& operator-=(Vec3 o) { x -= o.x; y -= o.y; z -= o.z; return *this; }

  double length() const { return std::sqrt(x * x + y * y + z * z); }

  Vec3 normalized() const {
    double n = length();
    return n > 0.0 ? (*this) / n : Vec3{};
  }
};

inline double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(Vec3 a, Vec3 b) {
  return {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}

// Row-major 3x3. v' = M * v.
struct Mat3 {
  double m[9] {};

  static Mat3 id() {
    Mat3 r;
    r.m[0] = r.m[4] = r.m[8] = 1.0;
    return r;
  }

  // Textbook right-handed rotations.
  static Mat3 rotX(double a) {
    double c = std::cos(a);
    double s = std::sin(a);
    Mat3 r = id();
    r.m[4] = c;
    r.m[5] = -s;
    r.m[7] = s;
    r.m[8] = c;
    return r;
  }

  static Mat3 rotZ(double a) {
    double c = std::cos(a);
    double s = std::sin(a);
    Mat3 r = id();
    r.m[0] = c;
    r.m[1] = -s;
    r.m[3] = s;
    r.m[4] = c;
    return r;
  }

  // SOFA/ERFA sense: R1(phi) and R3(phi) as used by the IAU Fukushima-Williams product.
  static Mat3 sofaRx(double a) {
    double c = std::cos(a);
    double s = std::sin(a);
    Mat3 r = id();
    r.m[4] = c;
    r.m[5] = s;
    r.m[7] = -s;
    r.m[8] = c;
    return r;
  }

  static Mat3 sofaRz(double a) {
    double c = std::cos(a);
    double s = std::sin(a);
    Mat3 r = id();
    r.m[0] = c;
    r.m[1] = s;
    r.m[3] = -s;
    r.m[4] = c;
    return r;
  }

  Mat3 operator*(Mat3 o) const {
    Mat3 r;
    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        r.m[row * 3 + col] =
          m[row * 3 + 0] * o.m[0 * 3 + col] +
          m[row * 3 + 1] * o.m[1 * 3 + col] +
          m[row * 3 + 2] * o.m[2 * 3 + col];
      }
    }
    return r;
  }

  Vec3 operator*(Vec3 v) const {
    return {
      m[0] * v.x + m[1] * v.y + m[2] * v.z,
      m[3] * v.x + m[4] * v.y + m[5] * v.z,
      m[6] * v.x + m[7] * v.y + m[8] * v.z
    };
  }
};

} // namespace eph
