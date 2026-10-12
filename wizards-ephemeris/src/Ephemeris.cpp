#include "eph/Ephemeris.hpp"

#include "HouseCalc.hpp"
#include "Minor.hpp"
#include "Moon.hpp"
#include "Reduce.hpp"
#include "Spk.hpp"
#include "VSOP87.hpp"
#include "eph/Time.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace eph {

struct Slot {
  double jd = -1.0e300;
  Vec3 v {};
};

namespace {

constexpr double kEps0 = 84381.406 * kArcsec;
constexpr double kSpeedDt = 1.0e-4;

Vec3 remember(Slot* slots, int& clock, double jd, const Vec3& value) {
  Slot& slot = slots[clock++ & 31];
  slot.jd = jd;
  slot.v = value;
  return value;
}

Vec3 findSlot(Slot* slots, double jd, bool& hit) {
  for (int i = 0; i < 32; ++i) {
    if (slots[i].jd == jd) {
      hit = true;
      return slots[i].v;
    }
  }
  hit = false;
  return {};
}

int vsopOf(Body body) {
  switch (body) {
  case Body::Mercury: return 0;
  case Body::Venus: return 1;
  case Body::Mars: return 3;
  case Body::Jupiter: return 4;
  case Body::Saturn: return 5;
  case Body::Uranus: return 6;
  case Body::Neptune: return 7;
  default: return -1;
  }
}

int minorOf(Body body) {
  switch (body) {
  case Body::Pluto: return 0;
  case Body::Chiron: return 1;
  case Body::Ceres: return 2;
  case Body::Pallas: return 3;
  case Body::Juno: return 4;
  case Body::Vesta: return 5;
  default: return -1;
  }
}

double degPerDay(double rad0, double rad1) {
  return wrapPi(rad1 - rad0) / kDeg / kSpeedDt;
}

} // namespace

struct Ephemeris::Impl {
  detail::SpkBook spk;
  mutable Slot moon[32];
  mutable int moonClock = 0;
  mutable Slot earth[32];
  mutable int earthClock = 0;
  mutable double velJd = -1.0e300;
  mutable Vec3 vel {};

  Vec3 moonAu(double jdTt) const {
    bool hit = false;
    Vec3 found = findSlot(moon, jdTt, hit);
    if (hit) return found;
    return remember(moon, moonClock, jdTt, detail::elpGeocentricAu(jdTt));
  }

  Vec3 earthAu(double jdTt) const {
    bool hit = false;
    Vec3 found = findSlot(earth, jdTt, hit);
    if (hit) return found;
    // VSOP87's Earth series matches Swiss geocentric coordinates directly.
    // Subtracting the lunar offset adds a 6" monthly term that Swiss does not show.
    return remember(earth, earthClock, jdTt, detail::vsopHeliocentric(2, jdTt));
  }

  Vec3 earthVel(double jdTt) const {
    if (velJd == jdTt) return vel;
    constexpr double h = 0.5;
    vel = (earthAu(jdTt + h) - earthAu(jdTt - h)) / (2.0 * h);
    velJd = jdTt;
    return vel;
  }
};

Ephemeris::Ephemeris(std::string spkDir) : impl_(std::make_unique<Impl>()) {
  impl_->spk.loadDirectory(std::move(spkDir));
}

Ephemeris::~Ephemeris() = default;
Ephemeris::Ephemeris(Ephemeris&&) noexcept = default;
Ephemeris& Ephemeris::operator=(Ephemeris&&) noexcept = default;

void Ephemeris::setLilithMode(LilithMode mode) {
  lilith_ = mode;
}

void Ephemeris::setSidereal(bool on, SiderealMethod method) {
  sidereal_ = on;
  ayanamsa_ = method.ayanamsa;
  nutation_ = method.nutation;
  wholeSignSnap_ = method.wholeSignSnap;
}

void Ephemeris::setSidereal(bool on, Ayanamsa mode) {
  setSidereal(on, SiderealMethod{mode, SiderealNutation::True, WholeSignSnap::ShiftThenSnap});
}

double Ephemeris::siderealShift(double jdUt) const {
  if (nutation_ == SiderealNutation::True) {
    return eph::ayanamsaTrue(ayanamsa_, JulianDay{jdUt}).rad;
  }
  return eph::ayanamsa(ayanamsa_, JulianDay{jdUt}).rad;
}

Ephemeris::Sky Ephemeris::sky(Body body, double jdUt) const {
  const double jdTt = utToTt(JulianDay{jdUt}).value;
  detail::SkyVec place;
  const Vec3 vel = impl_->earthVel(jdTt);

  if (body == Body::Sun) {
    place = detail::reduceEcliptic(-impl_->earthAu(jdTt), vel, jdTt);
  } else if (body == Body::Moon) {
    // ELP is geocentric at one instant. Light leaves the moon at t-tau and
    // arrives at the earth at t, so the inertial direction is moon(t-tau) - earth(t).
    Vec3 geo = impl_->moonAu(jdTt);
    const double tau = geo.length() / kAuPerDay;
    geo = impl_->moonAu(jdTt - tau);
    geo += impl_->earthAu(jdTt - tau) - impl_->earthAu(jdTt);
    place = detail::reduceEcliptic(geo, vel, jdTt);
  } else if (const int vsop = vsopOf(body); vsop >= 0) {
    const Vec3 earth = impl_->earthAu(jdTt);
    Vec3 p = detail::vsopHeliocentric(vsop, jdTt);
    double tau = (p - earth).length() / kAuPerDay;
    for (int i = 0; i < 2; ++i) {
      p = detail::vsopHeliocentric(vsop, jdTt - tau);
      tau = (p - earth).length() / kAuPerDay;
    }
    place = detail::reduceEcliptic(p - earth, vel, jdTt);
  } else if (const int minor = minorOf(body); minor >= 0) {
    Vec3 helio;
    bool equatorial = false;
    detail::minorHeliocentric(minor, jdTt, impl_->spk, helio, equatorial);
    if (equatorial) {
      Vec3 earth = detail::eclipticToEquatorial(impl_->earthAu(jdTt), kEps0);
      double tau = (helio - earth).length() / kAuPerDay;
      for (int i = 0; i < 2; ++i) {
        detail::minorHeliocentric(minor, jdTt - tau, impl_->spk, helio, equatorial);
        tau = (helio - earth).length() / kAuPerDay;
      }
      const Vec3 velEq = detail::eclipticToEquatorial(vel, kEps0);
      place = detail::reduceEquatorial(helio - earth, velEq, jdTt);
    } else {
      const Vec3 earth = impl_->earthAu(jdTt);
      double tau = (helio - earth).length() / kAuPerDay;
      for (int i = 0; i < 2; ++i) {
        detail::minorHeliocentric(minor, jdTt - tau, impl_->spk, helio, equatorial);
        if (equatorial) break;
        tau = (helio - earth).length() / kAuPerDay;
      }
      place = detail::reduceEcliptic(helio - earth, vel, jdTt);
    }
  } else if (body == Body::MeanLilith || body == Body::TrueNode) {
    double dpsi = 0.0;
    double deps = 0.0;
    detail::nutationRad(jdTt, dpsi, deps);
    double lon = 0.0;
    double precession = 0.0;
    if (body == Body::MeanLilith) {
      const double t = (jdTt - kJ2000) / 36525.0;
      precession = (5028.796195 + (1.1054348 + (0.00007964
        + (-0.000023857 + (-0.0000000383) * t) * t) * t) * t) * t * kArcsec;
      // The mean apogee is the mean perigee plus one periodic term in the
      // argument 2(node - perigee), about 416".
      const double nodeArg = detail::meanLunarNode(jdTt);
      const double beat = 2.0 * (nodeArg - detail::meanLunarPerigee(jdTt));
      lon = detail::meanLunarPerigee(jdTt) + kPi + precession + dpsi + 416.43 * kArcsec * std::sin(beat);
    } else {
      auto moonDate = [&](double t) {
        Vec3 geo = impl_->moonAu(t);
        const double tau = geo.length() / kAuPerDay;
        geo = impl_->moonAu(t - tau);
        return detail::reduceEcliptic(geo, Vec3{}, t).ecl;
      };
      const Vec3 r0 = moonDate(jdTt);
      const Vec3 r1 = moonDate(jdTt + 1.0e-3);
      const Vec3 h = cross(r0, r1 - r0);
      lon = std::atan2(h.x, -h.y);
    }
    Sky node;
    node.lon = wrapTwoPi(lon);
    node.lat = 0.0;
    node.dist = 0.0;
    if (body == Body::MeanLilith && lilith_ == LilithMode::Inclined) {
      // The published longitude already matches the apogee projected onto
      // the ecliptic. Swiss then rotates that point by the mean lunar
      // inclination about the node (5.1453964°). Latitude is recovered
      // from the argument relative to the mean node, before precession.
      constexpr double kIncl = 5.1453964 * kDeg;
      const double rel = wrapPi(node.lon - precession - dpsi - detail::meanLunarNode(jdTt));
      const double arg = std::atan2(std::sin(rel), std::cos(rel) * std::cos(kIncl));
      node.lat = std::asin(std::clamp(std::sin(arg) * std::sin(kIncl), -1.0, 1.0));
    }
    const double epsTrue = detail::meanObliquityRad(jdTt) + deps;
    const double clat = std::cos(node.lat);
    const Vec3 eq = detail::eclipticToEquatorial(
      Vec3{clat * std::cos(node.lon), clat * std::sin(node.lon), std::sin(node.lat)}, epsTrue);
    node.ra = wrapTwoPi(std::atan2(eq.y, eq.x));
    node.dec = std::atan2(eq.z, std::hypot(eq.x, eq.y));
    if (sidereal_) node.lon = wrapTwoPi(node.lon - siderealShift(jdUt));
    return node;
  }

  Sky out;
  out.lon = wrapTwoPi(std::atan2(place.ecl.y, place.ecl.x));
  out.lat = std::atan2(place.ecl.z, std::hypot(place.ecl.x, place.ecl.y));
  out.dist = place.ecl.length();
  out.ra = wrapTwoPi(std::atan2(place.eq.y, place.eq.x));
  out.dec = std::atan2(place.eq.z, std::hypot(place.eq.x, place.eq.y));
  if (sidereal_) out.lon = wrapTwoPi(out.lon - siderealShift(jdUt));
  return out;
}

EclipticPos Ephemeris::position(Body body, JulianDay jdUt) const {
  const Sky a = sky(body, jdUt.value);
  const Sky b = sky(body, jdUt.value + kSpeedDt);
  EclipticPos pos;
  pos.lon = Angle{a.lon};
  pos.lat = Angle{a.lat};
  pos.distAu = a.dist;
  pos.lonSpeed = degPerDay(a.lon, b.lon);
  return pos;
}

EquatorialPos Ephemeris::equatorial(Body body, JulianDay jdUt) const {
  const Sky a = sky(body, jdUt.value);
  const Sky b = sky(body, jdUt.value + kSpeedDt);
  EquatorialPos pos;
  pos.ra = Angle{a.ra};
  pos.dec = Angle{a.dec};
  pos.raSpeed = degPerDay(a.ra, b.ra);
  return pos;
}

Houses Ephemeris::houses(JulianDay jdUt, double latDeg, double lonDegEast, HouseSystem sys) const {
  Houses h = computeHouses(jdUt.value, latDeg, lonDegEast, sys);
  if (!sidereal_) return h;
  const double shift = siderealShift(jdUt.value);
  auto sub = [shift](Angle a) { return Angle{wrapTwoPi(a.rad - shift)}; };
  h.asc = sub(h.asc);
  h.mc = sub(h.mc);
  h.vertex = sub(h.vertex);
  if (sys == HouseSystem::WholeSign && wholeSignSnap_ == WholeSignSnap::ShiftThenSnap) {
    // Snap the shifted ascendant onto a sidereal sign, then step by 30°.
    const double start = std::floor(wrap360(h.asc.deg()) / 30.0) * 30.0 * kDeg;
    for (int i = 0; i < 12; ++i) h.cusp[i] = Angle{wrapTwoPi(start + i * (kPi / 6.0))};
  } else {
    for (Angle& cusp : h.cusp) cusp = sub(cusp);
  }
  return h;
}

} // namespace eph
