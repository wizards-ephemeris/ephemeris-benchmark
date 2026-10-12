#include "Catalog.hpp"

#include "SwissRef.hpp"

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

namespace emph {
namespace {

class WizardsAdapter : public Adapter {
public:
  const char* name() const override { return "Wizards Ephemeris"; }
  bool available() const override { return true; }

  SkyPlace position(eph::Body body, double jdUt) const override {
    const eph::EclipticPos pos = eph_.position(body, eph::JulianDay{jdUt});
    SkyPlace sky;
    sky.lonDeg = pos.lon.deg();
    sky.latDeg = pos.lat.deg();
    sky.speedDegPerDay = pos.lonSpeed;
    return sky;
  }

  SkyPlace equatorial(eph::Body body, double jdUt) const override {
    const eph::EquatorialPos eq = eph_.equatorial(body, eph::JulianDay{jdUt});
    SkyPlace sky = position(body, jdUt);
    sky.raDeg = eq.ra.deg();
    sky.decDeg = eq.dec.deg();
    sky.raSpeedDegPerDay = eq.raSpeed;
    return sky;
  }

  HousePlace houses(double jdUt, double latDeg, double lonEast, eph::HouseSystem sys) const override {
    const eph::Houses h = eph_.houses(eph::JulianDay{jdUt}, latDeg, lonEast, sys);
    HousePlace out;
    out.ascDeg = h.asc.deg();
    out.mcDeg = h.mc.deg();
    out.armcDeg = h.armc.deg();
    out.vertexDeg = h.vertex.deg();
    out.fallback = h.fallback;
    for (int i = 0; i < 12; ++i) out.cuspDeg[i] = h.cusp[i].deg();
    return out;
  }

  double ayanamsaDeg(eph::Ayanamsa mode, double jdUt) const override {
    return eph::ayanamsa(mode, eph::JulianDay{jdUt}).deg();
  }

  void setSidereal(bool on, eph::Ayanamsa mode) override {
    eph_.setSidereal(on, mode);
  }

  void setSidereal(bool on, eph::SiderealMethod method) override {
    eph_.setSidereal(on, method);
  }

  void setLilithMode(eph::LilithMode mode) override {
    eph_.setLilithMode(mode);
  }

private:
  mutable eph::Ephemeris eph_;
};

class SwissAdapter : public Adapter {
public:
  const char* name() const override { return "Swiss Ephemeris"; }
  bool available() const override { return true; }

  SkyPlace position(eph::Body body, double jdUt) const override {
    double lon = 0.0;
    double lat = 0.0;
    double speed = 0.0;
    swiss_.calc(sweBody(body), jdUt, lon, lat, speed, siderealFlags());
    SkyPlace sky;
    sky.lonDeg = lon;
    sky.latDeg = lat;
    sky.speedDegPerDay = speed;
    if (body == eph::Body::MeanLilith && lilith_ == eph::LilithMode::Ecliptic) sky.latDeg = 0.0;
    return sky;
  }

  SkyPlace equatorial(eph::Body body, double jdUt) const override {
    if (body == eph::Body::MeanLilith && lilith_ == eph::LilithMode::Ecliptic) {
      SkyPlace sky = position(body, jdUt);
      equatorOf(jdUt, sky.lonDeg, 0.0, sky.raDeg, sky.decDeg);
      double lon2 = 0.0;
      double lat2 = 0.0;
      double speed2 = 0.0;
      swiss_.calc(sweBody(body), jdUt + 1.0e-4, lon2, lat2, speed2, siderealFlags());
      double ra2 = 0.0;
      double dec2 = 0.0;
      equatorOf(jdUt + 1.0e-4, lon2, 0.0, ra2, dec2);
      sky.raSpeedDegPerDay = wrapDeg(ra2 - sky.raDeg) / 1.0e-4;
      return sky;
    }
    double lon = 0.0;
    double lat = 0.0;
    double speed = 0.0;
    int extra = SEFLG_EQUATORIAL;
    if (sidereal_) extra |= SEFLG_SIDEREAL;
    if (sidereal_ && method_.nutation == eph::SiderealNutation::Mean) extra |= SEFLG_NONUT;
    swiss_.calc(sweBody(body), jdUt, lon, lat, speed, extra);
    SkyPlace sky;
    sky.raDeg = lon;
    sky.decDeg = lat;
    sky.speedDegPerDay = speed;
    sky.raSpeedDegPerDay = speed;
    return sky;
  }

  HousePlace houses(double jdUt, double latDeg, double lonEast, eph::HouseSystem sys) const override {
    double cusp[13] {};
    double ascmc[10] {};
    swe_houses_ex(jdUt, siderealFlags(), latDeg, lonEast, sweHouse(sys), cusp, ascmc);
    HousePlace out;
    out.ascDeg = ascmc[0];
    out.mcDeg = ascmc[1];
    out.armcDeg = ascmc[2];
    out.vertexDeg = ascmc[3];
    for (int i = 0; i < 12; ++i) out.cuspDeg[i] = cusp[i + 1];
    return out;
  }

  double ayanamsaDeg(eph::Ayanamsa mode, double jdUt) const override {
    swe_set_sid_mode(sweAyanamsa(mode), 0.0, 0.0);
    return swe_get_ayanamsa_ut(jdUt);
  }

  void setSidereal(bool on, eph::Ayanamsa mode) override {
    setSidereal(on, eph::SiderealMethod{mode});
  }

  void setSidereal(bool on, eph::SiderealMethod method) override {
    sidereal_ = on;
    method_ = method;
    swe_set_sid_mode(sweAyanamsa(method.ayanamsa), 0.0, 0.0);
  }

  void setLilithMode(eph::LilithMode mode) override {
    lilith_ = mode;
  }

private:
  int siderealFlags() const {
    if (!sidereal_) return 0;
    int flags = SEFLG_SIDEREAL;
    if (method_.nutation == eph::SiderealNutation::Mean) flags |= SEFLG_NONUT;
    return flags;
  }

  static double wrapDeg(double deg) {
    while (deg > 180.0) deg -= 360.0;
    while (deg < -180.0) deg += 360.0;
    return deg;
  }

  // Ecliptic Lilith is the Swiss longitude laid on the ecliptic. Right
  // ascension and declination are that point, not the inclined apogee.
  static void equatorOf(double jdUt, double lonDeg, double latDeg, double& raDeg, double& decDeg) {
    double xx[6] {};
    char err[256] {};
    if (swe_calc_ut(jdUt, SE_ECL_NUT, 0, xx, err) < 0) {
      throw std::runtime_error(err);
    }
    const double eps = xx[0] * eph::kDeg;
    const double lon = lonDeg * eph::kDeg;
    const double lat = latDeg * eph::kDeg;
    const double cl = std::cos(lat);
    const double sl = std::sin(lat);
    const double ce = std::cos(eps);
    const double se = std::sin(eps);
    const double x = cl * std::cos(lon);
    const double y = ce * cl * std::sin(lon) - se * sl;
    const double z = se * cl * std::sin(lon) + ce * sl;
    raDeg = std::atan2(y, x) / eph::kDeg;
    if (raDeg < 0.0) raDeg += 360.0;
    decDeg = std::atan2(z, std::hypot(x, y)) / eph::kDeg;
  }

  Swiss swiss_;
  bool sidereal_ = false;
  eph::SiderealMethod method_ {};
  eph::LilithMode lilith_ = eph::LilithMode::Inclined;
};

WizardsAdapter& wizardsImpl() {
  static WizardsAdapter adapter;
  return adapter;
}

SwissAdapter& swissImpl() {
  static SwissAdapter adapter;
  return adapter;
}

} // namespace

Adapter& reference() { return swissImpl(); }

const std::vector<Adapter*>& libraries() {
  static const std::vector<Adapter*> all {&wizardsImpl(), &swissImpl()};
  return all;
}

const std::vector<Adapter*>& subjects() {
  static const std::vector<Adapter*> subs {&wizardsImpl()};
  return subs;
}

} // namespace emph
