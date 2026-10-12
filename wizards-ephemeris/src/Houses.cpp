#include "HouseCalc.hpp"

#include "Reduce.hpp"
#include "eph/Time.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace eph {
namespace {

double raOf(double lon, double eps) {
  return std::atan2(std::sin(lon) * std::cos(eps), std::cos(lon));
}

double decOf(double lon, double eps) {
  return std::asin(std::clamp(std::sin(eps) * std::sin(lon), -1.0, 1.0));
}

double ascensionalDiff(double dec, double lat) {
  const double t = std::tan(lat) * std::tan(dec);
  if (std::fabs(t) > 1.0) return std::numeric_limits<double>::quiet_NaN();
  return std::asin(t);
}

bool newton(double guess, const auto& residual, double& lon) {
  double lam = guess;
  for (int n = 0; n < 30; ++n) {
    const double r0 = residual(lam);
    if (!std::isfinite(r0)) return false;
    const double r1 = residual(lam + 1e-5);
    if (!std::isfinite(r1)) return false;
    const double der = (r1 - r0) / 1e-5;
    if (std::fabs(der) < 1e-12) return false;
    double step = r0 / der;
    if (step > 0.5) step = 0.5;
    if (step < -0.5) step = -0.5;
    lam = wrapTwoPi(lam - step);
    if (std::fabs(step) < 1e-13) {
      lon = lam;
      return true;
    }
  }
  return false;
}

void angles(double jdUt, double lat, double lonEast, double& ramc, double& eps, double& asc, double& mc, double& vertex) {
  const double jdTt = utToTt(JulianDay{jdUt}).value;
  eps = detail::meanObliquityRad(jdTt) ;
  double dpsi = 0.0;
  double deps = 0.0;
  detail::nutationRad(jdTt, dpsi, deps);
  eps += deps;
  ramc = wrapTwoPi(detail::gastRad(jdUt, jdTt) + lonEast);
  mc = wrapTwoPi(std::atan2(std::sin(ramc), std::cos(ramc) * std::cos(eps)));
  asc = wrapTwoPi(std::atan2(
    std::cos(ramc),
    -(std::sin(ramc) * std::cos(eps) + std::tan(lat) * std::sin(eps))));
  // The two-argument form can return the setting point when the ecliptic is
  // nearly horizontal. The ascendant is the eastern intersection.
  if (wrapPi(ramc - raOf(asc, eps)) > 0.0) asc = wrapTwoPi(asc + kPi);

  const double cl = std::cos(lat);
  const double sl = std::sin(lat);
  const Vec3 zenith{cl * std::cos(ramc), cl * std::sin(ramc), sl};
  const Vec3 pole{0.0, 0.0, 1.0};
  Vec3 east = cross(pole, zenith);
  if (east.length() < 1e-10) {
    vertex = wrapTwoPi(asc + kPi);
    return;
  }
  east = east.normalized();
  const Vec3 normal = cross(east, zenith);
  const Vec3 eclPole{0.0, -std::sin(eps), std::cos(eps)};
  Vec3 d = cross(normal, eclPole);
  if (dot(d, east) > 0.0) d = -d;
  const Vec3 ecl = detail::equatorialToEcliptic(d, eps);
  vertex = wrapTwoPi(std::atan2(ecl.y, ecl.x));
}

void porphyry(double asc, double mc, double cusp[12]) {
  const double arcMa = wrapTwoPi(asc - mc);
  const double ic = wrapTwoPi(mc + kPi);
  const double arcAi = wrapTwoPi(ic - asc);
  cusp[0] = asc;
  cusp[9] = mc;
  cusp[10] = wrapTwoPi(mc + arcMa / 3.0);
  cusp[11] = wrapTwoPi(mc + 2.0 * arcMa / 3.0);
  cusp[1] = wrapTwoPi(asc + arcAi / 3.0);
  cusp[2] = wrapTwoPi(asc + 2.0 * arcAi / 3.0);
  cusp[3] = ic;
  cusp[4] = wrapTwoPi(cusp[10] + kPi);
  cusp[5] = wrapTwoPi(cusp[11] + kPi);
  cusp[6] = wrapTwoPi(asc + kPi);
  cusp[7] = wrapTwoPi(cusp[1] + kPi);
  cusp[8] = wrapTwoPi(cusp[2] + kPi);
}

struct Horizon {
  Vec3 zenith;
  Vec3 east;
  Vec3 north;
  bool ok = false;
};

Horizon horizon(double ramc, double lat) {
  Horizon h;
  const double cl = std::cos(lat);
  const double sl = std::sin(lat);
  h.zenith = {cl * std::cos(ramc), cl * std::sin(ramc), sl};
  const Vec3 pole{0.0, 0.0, 1.0};
  h.east = cross(pole, h.zenith);
  if (h.east.length() < 1e-10) return h;
  h.east = h.east.normalized();
  h.north = cross(h.zenith, h.east).normalized();
  h.ok = true;
  return h;
}

double eclipticCut(Vec3 axis, Vec3 point, double eps) {
  const Vec3 normal = cross(axis, point);
  const Vec3 eclPole{0.0, -std::sin(eps), std::cos(eps)};
  Vec3 d = cross(normal, eclPole);
  if (dot(d, point) < 0.0) d = -d;
  const Vec3 ecl = detail::equatorialToEcliptic(d, eps);
  return wrapTwoPi(std::atan2(ecl.y, ecl.x));
}

void quadrantFromPoles(const Horizon& h, double eps, const auto& pointAt, double cusp[12]) {
  for (int step = 0; step < 12; ++step) {
    const double f = step * (kPi / 6.0);
    const int house = (12 - step) % 12;
    cusp[house] = eclipticCut(h.north, pointAt(f), eps);
  }
}

bool placidusOne(double ramc, double eps, double lat, double guess, bool nocturnal, double fraction, double& lon) {
  auto residual = [&](double lam) {
    const double dec = decOf(lam, eps);
    const double ad = ascensionalDiff(dec, lat);
    if (!std::isfinite(ad)) return std::numeric_limits<double>::quiet_NaN();
    const double dsa = kPi / 2.0 + ad;
    const double nsa = kPi / 2.0 - ad;
    const double semi = nocturnal ? nsa : dsa;
    const double raTarget = nocturnal
      ? ramc + dsa + fraction * semi
      : ramc + fraction * semi;
    return wrapPi(raOf(lam, eps) - raTarget);
  };
  return newton(guess, residual, lon);
}

bool placidus(double ramc, double eps, double lat, double asc, double mc, double cusp[12]) {
  const double ic = wrapTwoPi(mc + kPi);
  const double arcMa = wrapTwoPi(asc - mc);
  const double arcAi = wrapTwoPi(ic - asc);
  double c11 = 0.0;
  double c12 = 0.0;
  double c2 = 0.0;
  double c3 = 0.0;
  const bool ok =
    placidusOne(ramc, eps, lat, mc + arcMa / 3.0, false, 1.0 / 3.0, c11) &&
    placidusOne(ramc, eps, lat, mc + 2.0 * arcMa / 3.0, false, 2.0 / 3.0, c12) &&
    placidusOne(ramc, eps, lat, asc + arcAi / 3.0, true, 1.0 / 3.0, c2) &&
    placidusOne(ramc, eps, lat, asc + 2.0 * arcAi / 3.0, true, 2.0 / 3.0, c3);
  if (!ok) return false;
  cusp[0] = asc;
  cusp[1] = c2;
  cusp[2] = c3;
  cusp[3] = ic;
  cusp[9] = mc;
  cusp[10] = c11;
  cusp[11] = c12;
  cusp[4] = wrapTwoPi(c11 + kPi);
  cusp[5] = wrapTwoPi(c12 + kPi);
  cusp[6] = wrapTwoPi(asc + kPi);
  cusp[7] = wrapTwoPi(c2 + kPi);
  cusp[8] = wrapTwoPi(c3 + kPi);
  return true;
}

double ascAt(double ramc, double eps, double lat) {
  double asc = wrapTwoPi(std::atan2(
    std::cos(ramc),
    -(std::sin(ramc) * std::cos(eps) + std::tan(lat) * std::sin(eps))));
  if (wrapPi(ramc - raOf(asc, eps)) > 0.0) asc = wrapTwoPi(asc + kPi);
  return asc;
}

// Koch: the cusps are the ascendants at sidereal times stepped by thirds of
// the semi-arc of the MC's declination. Undefined once any ecliptic degree
// is circumpolar.
bool koch(double ramc, double eps, double lat, double asc, double mc, double cusp[12]) {
  if (std::fabs(lat) + eps >= kPi / 2.0) return false;
  const double dec = decOf(mc, eps);
  const double arg = std::tan(lat) * std::tan(dec);
  if (!(std::fabs(arg) < 1.0)) return false;
  const double dsa = std::acos(-arg);
  const double ic = wrapTwoPi(mc + kPi);
  cusp[0] = asc;
  cusp[9] = mc;
  cusp[3] = ic;
  cusp[6] = wrapTwoPi(asc + kPi);
  cusp[10] = ascAt(wrapTwoPi(ramc - 2.0 * dsa / 3.0), eps, lat);
  cusp[11] = ascAt(wrapTwoPi(ramc - dsa / 3.0), eps, lat);
  cusp[1] = ascAt(wrapTwoPi(ramc + dsa / 3.0), eps, lat);
  cusp[2] = ascAt(wrapTwoPi(ramc + 2.0 * dsa / 3.0), eps, lat);
  cusp[4] = wrapTwoPi(cusp[10] + kPi);
  cusp[5] = wrapTwoPi(cusp[11] + kPi);
  cusp[7] = wrapTwoPi(cusp[1] + kPi);
  cusp[8] = wrapTwoPi(cusp[2] + kPi);
  return true;
}

double regioCusp(double ramc, double eps, double lat, double hour, bool belowHorizon) {
  const double F = ramc + hour;
  const double pole = std::atan(std::tan(lat) * std::sin(hour));
  const double M = std::atan2(std::tan(pole), std::cos(F));
  double lon = std::atan2(std::sin(F) * std::cos(M), std::cos(F) * std::cos(M + eps));
  if (std::cos(F) < 0.0) lon += kPi;
  if (belowHorizon) lon += kPi;
  return wrapTwoPi(lon);
}

bool meridianBelow(double mc, double eps, double lat) {
  const double dec = std::asin(std::clamp(std::sin(eps) * std::sin(mc), -1.0, 1.0));
  const double salt = std::sin(lat) * std::sin(dec) + std::cos(lat) * std::cos(dec);
  return salt < 0.0;
}

double lonFromRa(double ra, double eps) {
  return wrapTwoPi(std::atan2(std::sin(ra), std::cos(ra) * std::cos(eps)));
}

// Alcabitius trisects the right-ascension arc from MC to ASC and from ASC to IC.
void alcabitius(double ramc, double eps, double asc, double mc, double cusp[12]) {
  const double raAsc = raOf(asc, eps);
  const double toAsc = wrapTwoPi(raAsc - ramc);
  const double raIc = wrapTwoPi(ramc + kPi);
  const double toIc = wrapTwoPi(raIc - raAsc);
  const double ic = wrapTwoPi(mc + kPi);
  cusp[0] = asc;
  cusp[9] = mc;
  cusp[3] = ic;
  cusp[6] = wrapTwoPi(asc + kPi);
  cusp[10] = lonFromRa(ramc + toAsc / 3.0, eps);
  cusp[11] = lonFromRa(ramc + 2.0 * toAsc / 3.0, eps);
  cusp[1] = lonFromRa(raAsc + toIc / 3.0, eps);
  cusp[2] = lonFromRa(raAsc + 2.0 * toIc / 3.0, eps);
  cusp[4] = wrapTwoPi(cusp[10] + kPi);
  cusp[5] = wrapTwoPi(cusp[11] + kPi);
  cusp[7] = wrapTwoPi(cusp[1] + kPi);
  cusp[8] = wrapTwoPi(cusp[2] + kPi);
}

// Sripati puts each cusp on the middle of a Porphyry sector, so the
// ascendant and midheaven are not themselves cusps. Near the pole, when
// the ascendant lies on the western side of the midheaven, Swiss adds 180°.
void sripati(double& asc, double mc, double cusp[12]) {
  double acmc = wrapPi(asc - mc);
  if (acmc < 0.0) {
    asc = wrapTwoPi(asc + kPi);
    acmc = wrapPi(asc - mc);
  }
  const double q1 = kPi - acmc;
  const double s1 = q1 / 3.0;
  const double s4 = acmc / 3.0;
  cusp[0] = wrapTwoPi(asc - 0.5 * s4);
  cusp[1] = wrapTwoPi(asc + 0.5 * s1);
  cusp[2] = wrapTwoPi(asc + 1.5 * s1);
  cusp[9] = wrapTwoPi(mc - 0.5 * s1);
  cusp[10] = wrapTwoPi(mc + 0.5 * s4);
  cusp[11] = wrapTwoPi(mc + 1.5 * s4);
  cusp[3] = wrapTwoPi(cusp[9] + kPi);
  cusp[4] = wrapTwoPi(cusp[10] + kPi);
  cusp[5] = wrapTwoPi(cusp[11] + kPi);
  cusp[6] = wrapTwoPi(cusp[0] + kPi);
  cusp[7] = wrapTwoPi(cusp[1] + kPi);
  cusp[8] = wrapTwoPi(cusp[2] + kPi);
}

// Ascendant of a great circle whose pole height is f, at equatorial angle x.
// Quadrant split follows the Swiss Asc1/Asc2 pair so the polar branch agrees.
double asc2(double x, double f, double eps) {
  double ass = -std::tan(f) * std::sin(eps) + std::cos(eps) * std::cos(x);
  double sinx = std::sin(x);
  if (std::fabs(ass) < 1e-12) ass = 0.0;
  if (std::fabs(sinx) < 1e-12) sinx = 0.0;
  if (sinx == 0.0) {
    ass = ass < 0.0 ? -1e-12 : 1e-12;
    ass = std::atan(sinx / ass);
  } else if (ass == 0.0) {
    ass = sinx < 0.0 ? -kPi / 2.0 : kPi / 2.0;
  } else {
    ass = std::atan(sinx / ass);
  }
  if (ass < 0.0) ass += kPi;
  return ass;
}

double asc1(double x1, double f, double eps) {
  x1 = wrapTwoPi(x1);
  if (std::fabs(kPi / 2.0 - f) < 1e-12) return kPi;
  if (std::fabs(-kPi / 2.0 - f) < 1e-12) return 0.0;
  const int n = static_cast<int>(x1 / (kPi / 2.0) + 1.0);
  double ass = 0.0;
  if (n == 1) ass = asc2(x1, f, eps);
  else if (n == 2) ass = kPi - asc2(kPi - x1, -f, eps);
  else if (n == 3) ass = kPi + asc2(x1 - kPi, -f, eps);
  else ass = kTwoPi - asc2(kTwoPi - x1, f, eps);
  return wrapTwoPi(ass);
}

// Polich/Page (topocentric): ascendants at pole heights atan(tan φ / 3)
// and atan(2 tan φ / 3). Inside the polar circle, when the raw ascendant
// lies west of the midheaven, every angle is moved by 180°.
void polichPage(double ramc, double eps, double lat, double& asc, double& mc, double cusp[12]) {
  double useLat = lat;
  if (useLat > kPi / 2.0 - 1e-8) useLat = kPi / 2.0 - 1e-8;
  if (useLat < -kPi / 2.0 + 1e-8) useLat = -kPi / 2.0 + 1e-8;
  const double tanLat = std::tan(useLat);
  const double fh1 = std::atan(tanLat / 3.0);
  const double fh2 = std::atan(tanLat * 2.0 / 3.0);
  const double rawAsc = asc1(ramc + kPi / 2.0, useLat, eps);
  cusp[0] = rawAsc;
  cusp[9] = mc;
  cusp[10] = asc1(ramc + kPi / 6.0, fh1, eps);
  cusp[11] = asc1(ramc + kPi / 3.0, fh2, eps);
  cusp[1] = asc1(ramc + 2.0 * kPi / 3.0, fh2, eps);
  cusp[2] = asc1(ramc + 5.0 * kPi / 6.0, fh1, eps);
  cusp[3] = wrapTwoPi(cusp[9] + kPi);
  cusp[4] = wrapTwoPi(cusp[10] + kPi);
  cusp[5] = wrapTwoPi(cusp[11] + kPi);
  cusp[6] = wrapTwoPi(cusp[0] + kPi);
  cusp[7] = wrapTwoPi(cusp[1] + kPi);
  cusp[8] = wrapTwoPi(cusp[2] + kPi);
  if (std::fabs(lat) + eps >= kPi / 2.0 && wrapPi(rawAsc - mc) < 0.0) {
    asc = wrapTwoPi(rawAsc + kPi);
    mc = wrapTwoPi(mc + kPi);
    for (int i = 0; i < 12; ++i) cusp[i] = wrapTwoPi(cusp[i] + kPi);
  } else {
    asc = rawAsc;
  }
}

Houses pack(double asc, double mc, double ramc, double vertex, const double cusp[12], bool fallback) {
  Houses h;
  h.asc = Angle{asc};
  h.mc = Angle{mc};
  h.armc = Angle{ramc};
  h.vertex = Angle{vertex};
  h.fallback = fallback;
  for (int i = 0; i < 12; ++i) h.cusp[i] = Angle{cusp[i]};
  return h;
}

} // namespace

Houses computeHouses(double jdUt, double latDeg, double lonDegEast, HouseSystem sys) {
  const double lat = latDeg * kDeg;
  const double lonEast = lonDegEast * kDeg;
  double ramc = 0.0;
  double eps = 0.0;
  double asc = 0.0;
  double mc = 0.0;
  double vertex = 0.0;
  angles(jdUt, lat, lonEast, ramc, eps, asc, mc, vertex);

  double cusp[12] {};
  bool fallback = false;
  const Horizon hor = horizon(ramc, lat);

  switch (sys) {
  case HouseSystem::Equal:
    for (int i = 0; i < 12; ++i) cusp[i] = wrapTwoPi(asc + i * (kPi / 6.0));
    break;
  case HouseSystem::WholeSign: {
    const double start = std::floor(wrap360(asc / kDeg) / 30.0) * 30.0 * kDeg;
    for (int i = 0; i < 12; ++i) cusp[i] = wrapTwoPi(start + i * (kPi / 6.0));
    break;
  }
  case HouseSystem::Porphyrius:
    porphyry(asc, mc, cusp);
    break;
  case HouseSystem::Placidus:
    if (!placidus(ramc, eps, lat, asc, mc, cusp)) {
      porphyry(asc, mc, cusp);
      fallback = true;
    }
    break;
  case HouseSystem::Koch:
    if (!koch(ramc, eps, lat, asc, mc, cusp)) {
      porphyry(asc, mc, cusp);
      fallback = true;
    }
    break;
  case HouseSystem::Campanus:
    if (!hor.ok) {
      porphyry(asc, mc, cusp);
      fallback = true;
    } else {
      quadrantFromPoles(hor, eps, [&](double f) {
        return hor.east * std::cos(f) + hor.zenith * std::sin(f);
      }, cusp);
    }
    break;
  case HouseSystem::Regiomontanus: {
    const bool below = meridianBelow(mc, eps, lat);
    const double mcCusp = below ? wrapTwoPi(mc + kPi) : mc;
    cusp[0] = asc;
    cusp[6] = wrapTwoPi(asc + kPi);
    cusp[9] = mcCusp;
    cusp[3] = wrapTwoPi(mcCusp + kPi);
    cusp[10] = regioCusp(ramc, eps, lat, kPi / 6.0, below);
    cusp[11] = regioCusp(ramc, eps, lat, kPi / 3.0, below);
    cusp[1] = regioCusp(ramc, eps, lat, 2.0 * kPi / 3.0, below);
    cusp[2] = regioCusp(ramc, eps, lat, 5.0 * kPi / 6.0, below);
    cusp[4] = wrapTwoPi(cusp[10] + kPi);
    cusp[5] = wrapTwoPi(cusp[11] + kPi);
    cusp[7] = wrapTwoPi(cusp[1] + kPi);
    cusp[8] = wrapTwoPi(cusp[2] + kPi);
    break;
  }
  case HouseSystem::Alcabitius:
    alcabitius(ramc, eps, asc, mc, cusp);
    break;
  case HouseSystem::Sripati:
    sripati(asc, mc, cusp);
    break;
  case HouseSystem::PolichPage:
    polichPage(ramc, eps, lat, asc, mc, cusp);
    break;
  }
  if (sys == HouseSystem::Campanus || sys == HouseSystem::Regiomontanus) mc = cusp[9];
  return pack(asc, mc, ramc, vertex, cusp, fallback);
}

} // namespace eph
