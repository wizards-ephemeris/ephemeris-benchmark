#include "Moon.hpp"

#include "Series.hpp"

#include <algorithm>
#include <cmath>

namespace eph::detail {
namespace {

struct Arg {
  double c[5] {};
};

double evalArg(const Arg& a, double t) {
  return a.c[0] + t * (a.c[1] + t * (a.c[2] + t * (a.c[3] + t * a.c[4])));
}

double dms(double d, double m, double s) {
  return (d + m / 60.0 + s / 3600.0) * kDeg;
}

double as2rad(double arcsec) {
  return arcsec / (648000.0 / kPi);
}

struct ElpConst {
  Arg w1;
  Arg w2;
  Arg w3;
  Arg del[4];
  Arg planet[8];
  double zeta0 = 0.0;
  double zeta1 = 0.0;
  double delnu = 0.0;
  double dele = 0.0;
  double delg = 0.0;
  double delnp = 0.0;
  double delep = 0.0;
  double dtasm = 0.0;
};

ElpConst makeConst() {
  ElpConst e;
  e.w1.c[0] = dms(218, 18, 59.95571);
  e.w1.c[1] = as2rad(1732559343.73604);
  e.w1.c[2] = as2rad(-5.8883);
  e.w1.c[3] = as2rad(0.6604e-2);
  e.w1.c[4] = as2rad(-0.3169e-4);

  e.w2.c[0] = dms(83, 21, 11.67475);
  e.w2.c[1] = as2rad(14643420.2632);
  e.w2.c[2] = as2rad(-38.2776);
  e.w2.c[3] = as2rad(-0.45047e-1);
  e.w2.c[4] = as2rad(0.21301e-3);

  e.w3.c[0] = dms(125, 2, 40.39816);
  e.w3.c[1] = as2rad(-6967919.3622);
  e.w3.c[2] = as2rad(6.3622);
  e.w3.c[3] = as2rad(0.7625e-2);
  e.w3.c[4] = as2rad(-0.3586e-4);

  Arg eart;
  eart.c[0] = dms(100, 27, 59.22059);
  eart.c[1] = as2rad(129597742.2758);
  eart.c[2] = as2rad(-0.0202);
  eart.c[3] = as2rad(0.9e-5);
  eart.c[4] = as2rad(0.15e-6);

  Arg peri;
  peri.c[0] = dms(102, 56, 14.42753);
  peri.c[1] = as2rad(1161.2283);
  peri.c[2] = as2rad(0.5327);
  peri.c[3] = as2rad(-0.138e-3);

  for (int k = 0; k < 5; ++k) {
    e.del[0].c[k] = e.w1.c[k] - eart.c[k];
    e.del[1].c[k] = eart.c[k] - peri.c[k];
    e.del[2].c[k] = e.w1.c[k] - e.w2.c[k];
    e.del[3].c[k] = e.w1.c[k] - e.w3.c[k];
  }
  e.del[0].c[0] += kPi;

  e.planet[0].c[0] = dms(252, 15, 3.25986);
  e.planet[0].c[1] = as2rad(538101628.68898);
  e.planet[1].c[0] = dms(181, 58, 47.28305);
  e.planet[1].c[1] = as2rad(210664136.43355);
  e.planet[2] = eart;
  e.planet[3].c[0] = dms(355, 25, 59.78866);
  e.planet[3].c[1] = as2rad(68905077.59284);
  e.planet[4].c[0] = dms(34, 21, 5.34212);
  e.planet[4].c[1] = as2rad(10925660.42861);
  e.planet[5].c[0] = dms(50, 4, 38.89694);
  e.planet[5].c[1] = as2rad(4399609.65932);
  e.planet[6].c[0] = dms(314, 3, 18.01841);
  e.planet[6].c[1] = as2rad(1542481.19393);
  e.planet[7].c[0] = dms(304, 20, 55.19575);
  e.planet[7].c[1] = as2rad(786550.32074);

  e.zeta0 = e.w1.c[0];
  e.zeta1 = e.w1.c[1] + as2rad(5029.0966);

  constexpr double kAm = 0.074801329518;
  constexpr double kAlfa = 0.002571881335;
  e.dtasm = 2.0 * kAlfa / (3.0 * kAm);
  e.delnu = as2rad(0.55604) / e.w1.c[1];
  e.dele = as2rad(0.01789);
  e.delg = as2rad(-0.08066);
  e.delnp = as2rad(-0.06424) / e.w1.c[1];
  e.delep = as2rad(-0.12879);
  return e;
}

const ElpConst& consts() {
  static const ElpConst c = makeConst();
  return c;
}

void addMain(const ElpMain* terms, int n, const ElpConst& e, const double tp[5], bool distance, double& sum) {
  for (int nterm = 0; nterm < n; ++nterm) {
    const ElpMain& term = terms[nterm];
    const double* c = term.c;
    double c0 = c[0];
    if (distance) c0 *= (1.0 - 2.0 * e.delnu / 3.0);
    const double tgv = c[1] + e.dtasm * c[5];
    const double x = c0
      + tgv * (e.delnp - 0.074801329518 * e.delnu)
      + c[2] * e.delg + c[3] * e.dele + c[4] * e.delep;
    double y = 0.0;
    for (int k = 0; k < 5; ++k) {
      double yk = 0.0;
      for (int j = 0; j < 4; ++j) yk += term.i[j] * e.del[j].c[k];
      y += yk * tp[k];
    }
    if (distance) y += kPi / 2.0;
    sum += x * std::sin(y);
  }
}

void addFig(const ElpFig* terms, int n, const ElpConst& e, double t, double power, double& sum) {
  const double scale = power == 0.0 ? 1.0 : (power == 1.0 ? t : t * t);
  for (int nterm = 0; nterm < n; ++nterm) {
    const ElpFig& term = terms[nterm];
    double y = term.pha * kDeg;
    y += term.iz * e.zeta0;
    y += term.iz * e.zeta1 * t;
    for (int j = 0; j < 4; ++j) {
      y += term.i[j] * e.del[j].c[0];
      y += term.i[j] * e.del[j].c[1] * t;
    }
    sum += term.amp * scale * std::sin(y);
  }
}

void addPla(const ElpPla* terms, int n, const ElpConst& e, double t, double power, bool table1, double& sum) {
  const double scale = power == 0.0 ? 1.0 : t;
  for (int nterm = 0; nterm < n; ++nterm) {
    const ElpPla& term = terms[nterm];
    double y = term.pha * kDeg;
    for (int k = 0; k < 2; ++k) {
      const double tk = k == 0 ? 1.0 : t;
      if (table1) {
        const double z = term.ip[8] * e.del[0].c[k]
          + term.ip[9] * e.del[2].c[k]
          + term.ip[10] * e.del[3].c[k];
        y += z * tk;
        for (int i = 0; i < 8; ++i) y += term.ip[i] * e.planet[i].c[k] * tk;
      } else {
        for (int i = 0; i < 4; ++i) y += term.ip[i + 7] * e.del[i].c[k] * tk;
        for (int i = 0; i < 7; ++i) y += term.ip[i] * e.planet[i].c[k] * tk;
      }
    }
    sum += term.amp * scale * std::sin(y);
  }
}

} // namespace

Vec3 elpGeocentricAu(double jdTt) {
  const ElpConst& e = consts();
  const double t = (jdTt - kJ2000) / 36525.0;
  double tp[5] = {1.0, t, t * t, t * t * t, t * t * t * t};

  double lon = 0.0;
  double lat = 0.0;
  double dist = 0.0;
  addMain(kMainLon, kMainLonN, e, tp, false, lon);
  addMain(kMainLat, kMainLatN, e, tp, false, lat);
  addMain(kMainDist, kMainDistN, e, tp, true, dist);

  addFig(kFigLon0, kFigLon0N, e, t, 0, lon);
  addFig(kFigLon1, kFigLon1N, e, t, 1, lon);
  addFig(kFigLon2, kFigLon2N, e, t, 2, lon);
  addFig(kFigLat0, kFigLat0N, e, t, 0, lat);
  addFig(kFigLat1, kFigLat1N, e, t, 1, lat);
  addFig(kFigLat2, kFigLat2N, e, t, 2, lat);
  addFig(kFigDist0, kFigDist0N, e, t, 0, dist);
  addFig(kFigDist1, kFigDist1N, e, t, 1, dist);
  addFig(kFigDist2, kFigDist2N, e, t, 2, dist);

  addPla(kPlaLon01, kPlaLon01N, e, t, 0, true, lon);
  addPla(kPlaLon02, kPlaLon02N, e, t, 0, false, lon);
  addPla(kPlaLon11, kPlaLon11N, e, t, 1, true, lon);
  addPla(kPlaLon12, kPlaLon12N, e, t, 1, false, lon);
  addPla(kPlaLat01, kPlaLat01N, e, t, 0, true, lat);
  addPla(kPlaLat02, kPlaLat02N, e, t, 0, false, lat);
  addPla(kPlaLat11, kPlaLat11N, e, t, 1, true, lat);
  addPla(kPlaLat12, kPlaLat12N, e, t, 1, false, lat);
  addPla(kPlaDist01, kPlaDist01N, e, t, 0, true, dist);
  addPla(kPlaDist02, kPlaDist02N, e, t, 0, false, dist);
  addPla(kPlaDist11, kPlaDist11N, e, t, 1, true, dist);
  addPla(kPlaDist12, kPlaDist12N, e, t, 1, false, dist);

  constexpr double kRad = 648000.0 / kPi;
  constexpr double kAth = 384747.9806743165;
  constexpr double kA0 = 384747.9806448954;
  lon = lon / kRad + evalArg(e.w1, t);
  lat = lat / kRad;
  dist = dist * kA0 / kAth;

  const double cl = std::cos(lat);
  double x1 = dist * cl * std::cos(lon);
  double x2 = dist * cl * std::sin(lon);
  double x3 = dist * std::sin(lat);

  const double p1 = 0.10180391e-4;
  const double p2 = 0.47020439e-6;
  const double p3 = -0.5417367e-9;
  const double p4 = -0.2507948e-11;
  const double p5 = 0.463486e-14;
  const double q1 = -0.113469002e-3;
  const double q2 = 0.12372674e-6;
  const double q3 = 0.1265417e-8;
  const double q4 = -0.1371808e-11;
  const double q5 = -0.320334e-14;
  double pw = (p1 + t * (p2 + t * (p3 + t * (p4 + t * p5)))) * t;
  double qw = (q1 + t * (q2 + t * (q3 + t * (q4 + t * q5)))) * t;
  const double ra = 2.0 * std::sqrt(std::max(0.0, 1.0 - pw * pw - qw * qw));
  const double pwqw = 2.0 * pw * qw;
  const double pw2 = 1.0 - 2.0 * pw * pw;
  const double qw2 = 1.0 - 2.0 * qw * qw;
  pw *= ra;
  qw *= ra;

  Vec3 km {
    pw2 * x1 + pwqw * x2 + pw * x3,
    pwqw * x1 + qw2 * x2 - qw * x3,
    -pw * x1 + qw * x2 + (pw2 + qw2 - 1.0) * x3
  };
  return km / kAuKm;
}

double meanLunarPerigee(double jdTt) {
  const double t = (jdTt - kJ2000) / 36525.0;
  return evalArg(consts().w2, t);
}

double meanLunarNode(double jdTt) {
  const double t = (jdTt - kJ2000) / 36525.0;
  return evalArg(consts().w3, t);
}

} // namespace eph::detail
