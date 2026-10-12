#pragma once

namespace eph {

struct JulianDay {
  double value = 0.0;

  explicit constexpr JulianDay(double v = 0.0) : value(v) {}
  constexpr operator double() const { return value; }
};

// Proleptic Gregorian calendar, matching Swiss Ephemeris SE_GREG_CAL.
// `hour` is hours from midnight UT, including minutes and seconds as a fraction.
JulianDay julianDayUt(int year, int month, int day, double hour);
void revJul(JulianDay jd, int& year, int& month, int& day, double& hour);

// TT - UT in seconds (Espenak / Meeus polynomials).
double deltaTSeconds(JulianDay jdUt);
JulianDay utToTt(JulianDay jdUt);

} // namespace eph
