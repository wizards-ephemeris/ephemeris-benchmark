#include "Minor.hpp"

#include "Series.hpp"

#include <cmath>

namespace eph::detail {
namespace {

struct Kep {
  double epoch;
  double a;
  double e;
  double inc;
  double om;
  double w;
  double ma;
  double n;
};

// Osculating elements from JPL SBDB, ecliptic J2000. Fallback outside the built-in series.
constexpr Kep kKep[] = {
  {2457588.5, 39.58862938517124, 0.2518378778576892, 17.14771140999114, 110.2923840543057, 113.7090015158565, 38.68366347318184, 0.003956838955553025},
  {2461200.5, 13.68426760850124, 0.3797656311453571, 6.930574468846328, 209.2961258613147, 339.2878326589729, 216.7198966018106, 0.0194702593257484},
  {2461200.5, 2.765552595034094, 0.07969229514816586, 10.58802780183462, 80.24862682043221, 73.29421453021587, 274.4193463761342, 0.21430445064843},
  {2461200.5, 2.769559010737709, 0.2307000995648547, 34.93279321851542, 172.8866193357694, 310.9699161652136, 254.2496521742734, 0.2138396029251949},
  {2461200.5, 2.670989527103278, 0.2556999836681878, 12.98659236598085, 169.8115953492418, 247.8950743075613, 262.7322944883855, 0.2257853690721904},
  {2461200.5, 2.361365965127599, 0.09020374382834395, 7.143925545058711, 103.701293265032, 151.4686478221564, 81.19015607686903, 0.2716183613599909},
};

constexpr int kNaif[] = {999, 2002060, 2000001, 2000002, 2000003, 2000004};

Vec3 kepler(const Kep& k, double jd) {
  const double mean = wrapTwoPi((k.ma + k.n * (jd - k.epoch)) * kDeg);
  double ecc = mean;
  for (int i = 0; i < 25; ++i) {
    const double f = ecc - k.e * std::sin(ecc) - mean;
    const double fp = 1.0 - k.e * std::cos(ecc);
    ecc -= f / fp;
  }
  const double xp = k.a * (std::cos(ecc) - k.e);
  const double yp = k.a * std::sqrt(1.0 - k.e * k.e) * std::sin(ecc);
  const double cw = std::cos(k.w * kDeg);
  const double sw = std::sin(k.w * kDeg);
  const double co = std::cos(k.om * kDeg);
  const double so = std::sin(k.om * kDeg);
  const double ci = std::cos(k.inc * kDeg);
  const double si = std::sin(k.inc * kDeg);
  return {
    (cw * co - sw * so * ci) * xp + (-sw * co - cw * so * ci) * yp,
    (cw * so + sw * co * ci) * xp + (-sw * so + cw * co * ci) * yp,
    (sw * si) * xp + (cw * si) * yp
  };
}

} // namespace

bool minorHeliocentric(int index, double jdTt, const SpkBook& spk, Vec3& helio, bool& equatorial) {
  if (index < 0 || index > 5) return false;
  if (!spk.empty() && spk.heliocentric(kNaif[index], jdTt, helio, equatorial)) return true;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  if (minorChebyshev(index, jdTt, x, y, z)) {
    helio = {x, y, z};
    equatorial = false;
    return true;
  }
  helio = kepler(kKep[index], jdTt);
  equatorial = false;
  return true;
}

} // namespace eph::detail
