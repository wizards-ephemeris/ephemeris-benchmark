#include "eph/Time.hpp"

#include <cmath>

namespace eph {

JulianDay julianDayUt(int year, int month, int day, double hour) {
  int y = year;
  int m = month;
  if (m <= 2) {
    y -= 1;
    m += 12;
  }
  const int a = y / 100;
  const int b = 2 - a + a / 4;
  const double jd = std::floor(365.25 * (y + 4716))
    + std::floor(30.6001 * (m + 1))
    + day + b - 1524.5
    + hour / 24.0;
  return JulianDay{jd};
}

void revJul(JulianDay jd, int& year, int& month, int& day, double& hour) {
  const double zf = std::floor(jd.value + 0.5);
  const double f = (jd.value + 0.5) - zf;
  const int z = static_cast<int>(zf);
  int a = z;
  if (z >= 2299161) {
    const int alpha = static_cast<int>(std::floor((z - 1867216.25) / 36524.25));
    a = z + 1 + alpha - alpha / 4;
  }
  const int b = a + 1524;
  const int c = static_cast<int>(std::floor((b - 122.1) / 365.25));
  const int d = static_cast<int>(std::floor(365.25 * c));
  const int e = static_cast<int>(std::floor((b - d) / 30.6001));
  day = b - d - static_cast<int>(std::floor(30.6001 * e));
  month = e < 14 ? e - 1 : e - 13;
  year = month > 2 ? c - 4716 : c - 4715;
  hour = f * 24.0;
}

namespace {

double poly(double t, const double* c, int n) {
  double s = 0.0;
  for (int i = n - 1; i >= 0; --i) s = s * t + c[i];
  return s;
}

} // namespace

double deltaTSeconds(JulianDay jdUt) {
  int year = 0;
  int month = 0;
  int day = 0;
  double hour = 0.0;
  revJul(jdUt, year, month, day, hour);
  const double y = year + (month - 0.5) / 12.0;

  if (y < -500.0) {
    const double u = (y - 1820.0) / 100.0;
    return -20.0 + 32.0 * u * u;
  }
  if (y < 500.0) {
    const double u = y / 100.0;
    const double c[] = {10583.6, -1014.41, 33.78311, -5.952053, -0.1798452, 0.022174192, 0.0090316521};
    return poly(u, c, 7);
  }
  if (y < 1600.0) {
    const double u = (y - 1000.0) / 100.0;
    const double c[] = {1574.2, -556.01, 71.23472, 0.319781, -0.8503463, -0.005050998, 0.0083572073};
    return poly(u, c, 7);
  }
  if (y < 1700.0) {
    const double t = y - 1600.0;
    return 120.0 - 0.9808 * t - 0.01532 * t * t + t * t * t / 7129.0;
  }
  if (y < 1800.0) {
    const double t = y - 1700.0;
    const double c[] = {8.83, 0.1603, -0.0059285, 0.00013336, -1.0 / 1174000.0};
    return poly(t, c, 5);
  }
  if (y < 1860.0) {
    const double t = y - 1800.0;
    const double c[] = {
      13.72, -0.332447, 0.0068612, 0.0041116, -0.00037436, 0.0000121272, -0.0000001699, 0.000000000875
    };
    return poly(t, c, 8);
  }
  if (y < 1900.0) {
    const double t = y - 1860.0;
    const double c[] = {7.62, 0.5737, -0.251754, 0.01680668, -0.0004473624, 1.0 / 233174.0};
    return poly(t, c, 6);
  }
  if (y < 1920.0) {
    const double t = y - 1900.0;
    const double c[] = {-2.79, 1.494119, -0.0598939, 0.0061966, -0.000197};
    return poly(t, c, 5);
  }
  if (y < 1941.0) {
    const double t = y - 1920.0;
    const double c[] = {21.20, 0.84493, -0.076100, 0.0020936};
    return poly(t, c, 4);
  }
  if (y < 1961.0) {
    const double t = y - 1950.0;
    return 29.07 + 0.407 * t - t * t / 233.0 + t * t * t / 2547.0;
  }
  if (y < 1986.0) {
    const double t = y - 1975.0;
    return 45.45 + 1.067 * t - t * t / 260.0 - t * t * t / 718.0;
  }
  if (y < 2005.0) {
    const double t = y - 2000.0;
    const double c[] = {63.86, 0.3345, -0.060374, 0.0017275, 0.000651814, 0.00002373599};
    return poly(t, c, 6);
  }
  // The 2004 parabola past 2005 climbs far above the measured clock error
  // (about 69 s in the early 2020s). Hold a slowing rise from the 2005 value.
  if (y < 2150.0) {
    const double t = y - 2005.0;
    return 64.70 + 0.285 * t - 0.00055 * t * t;
  }
  const double u = (y - 1820.0) / 100.0;
  return -20.0 + 32.0 * u * u;
}

JulianDay utToTt(JulianDay jdUt) {
  return JulianDay{jdUt.value + deltaTSeconds(jdUt) / 86400.0};
}

} // namespace eph
