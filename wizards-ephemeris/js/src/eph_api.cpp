#include "eph/eph.hpp"

#include <cstddef>

namespace {

struct eph_ephemeris {
  eph::Ephemeris impl;
};

// Integer order matches the C++ enumerators. Zodiac is 0 tropical, 1 sidereal.
void apply(
  eph::Ephemeris& chart,
  int zodiac,
  int ayanamsa,
  int nutation,
  int wholeSign,
  int lilith
) {
  chart.setLilithMode(static_cast<eph::LilithMode>(lilith));
  if (zodiac != 0) {
    chart.setSidereal(true, eph::SiderealMethod{
      static_cast<eph::Ayanamsa>(ayanamsa),
      static_cast<eph::SiderealNutation>(nutation),
      static_cast<eph::WholeSignSnap>(wholeSign)
    });
  } else {
    chart.setSidereal(false);
  }
}

} // namespace

extern "C" {

struct eph_ecliptic {
  double lon;
  double lat;
  double lon_speed;
  double dist_au;
};

struct eph_equatorial {
  double ra;
  double dec;
  double ra_speed;
};

struct eph_houses {
  double cusp[12];
  double asc;
  double mc;
  double armc;
  double vertex;
  double fallback;
};

struct eph_calendar {
  double year;
  double month;
  double day;
  double hour;
};

static_assert(sizeof(eph_ecliptic) == 4 * sizeof(double));
static_assert(sizeof(eph_equatorial) == 3 * sizeof(double));
static_assert(sizeof(eph_houses) == 17 * sizeof(double));
static_assert(sizeof(eph_calendar) == 4 * sizeof(double));
static_assert(offsetof(eph_houses, cusp) == 0);
static_assert(offsetof(eph_houses, asc) == 12 * sizeof(double));
static_assert(offsetof(eph_houses, fallback) == 16 * sizeof(double));

eph_ephemeris* eph_create(void) {
  return new eph_ephemeris;
}

void eph_destroy(eph_ephemeris* chart) {
  delete chart;
}

void eph_position(
  eph_ephemeris* chart,
  int body,
  double jdUt,
  int zodiac,
  int ayanamsa,
  int nutation,
  int wholeSign,
  int lilith,
  eph_ecliptic* out
) {
  apply(chart->impl, zodiac, ayanamsa, nutation, wholeSign, lilith);
  const eph::EclipticPos pos = chart->impl.position(
    static_cast<eph::Body>(body), eph::JulianDay{jdUt});
  out->lon = pos.lon.deg();
  out->lat = pos.lat.deg();
  out->lon_speed = pos.lonSpeed;
  out->dist_au = pos.distAu;
}

void eph_equatorial(
  eph_ephemeris* chart,
  int body,
  double jdUt,
  int zodiac,
  int ayanamsa,
  int nutation,
  int wholeSign,
  int lilith,
  eph_equatorial* out
) {
  apply(chart->impl, zodiac, ayanamsa, nutation, wholeSign, lilith);
  const eph::EquatorialPos pos = chart->impl.equatorial(
    static_cast<eph::Body>(body), eph::JulianDay{jdUt});
  out->ra = pos.ra.deg();
  out->dec = pos.dec.deg();
  out->ra_speed = pos.raSpeed;
}

void eph_houses(
  eph_ephemeris* chart,
  double jdUt,
  double latDeg,
  double lonDegEast,
  int system,
  int zodiac,
  int ayanamsa,
  int nutation,
  int wholeSign,
  int lilith,
  eph_houses* out
) {
  apply(chart->impl, zodiac, ayanamsa, nutation, wholeSign, lilith);
  const eph::Houses h = chart->impl.houses(
    eph::JulianDay{jdUt}, latDeg, lonDegEast, static_cast<eph::HouseSystem>(system));
  for (int i = 0; i < 12; ++i) out->cusp[i] = h.cusp[i].deg();
  out->asc = h.asc.deg();
  out->mc = h.mc.deg();
  out->armc = h.armc.deg();
  out->vertex = h.vertex.deg();
  out->fallback = h.fallback ? 1.0 : 0.0;
}

double eph_julian_day(int year, int month, int day, double hour) {
  return eph::julianDayUt(year, month, day, hour).value;
}

void eph_rev_jul(double jdUt, eph_calendar* out) {
  int year = 0;
  int month = 0;
  int day = 0;
  double hour = 0.0;
  eph::revJul(eph::JulianDay{jdUt}, year, month, day, hour);
  out->year = year;
  out->month = month;
  out->day = day;
  out->hour = hour;
}

double eph_delta_t(double jdUt) {
  return eph::deltaTSeconds(eph::JulianDay{jdUt});
}

double eph_ayanamsa(int mode, double jdUt, int trueNutation) {
  const eph::Ayanamsa which = static_cast<eph::Ayanamsa>(mode);
  const eph::JulianDay jd{jdUt};
  if (trueNutation) return eph::ayanamsaTrue(which, jd).deg();
  return eph::ayanamsa(which, jd).deg();
}

double eph_sep_deg(double aDeg, double bDeg) {
  return eph::sepDeg(aDeg, bDeg);
}

} // extern "C"
