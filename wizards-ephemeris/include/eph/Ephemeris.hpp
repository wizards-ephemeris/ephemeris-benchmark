#pragma once

#include "Ayanamsa.hpp"
#include "Body.hpp"
#include "Houses.hpp"
#include "Method.hpp"
#include "Position.hpp"

#include <memory>
#include <string>

namespace eph {

class Ephemeris {
public:
  // `spkDir` is an optional directory of heliocentric .bsp kernels (type 2/3).
  // Pluto, Chiron and the four asteroids use a built-in series when no kernel covers the date.
  explicit Ephemeris(std::string spkDir = {});
  ~Ephemeris();
  Ephemeris(Ephemeris&&) noexcept;
  Ephemeris& operator=(Ephemeris&&) noexcept;
  Ephemeris(const Ephemeris&) = delete;
  Ephemeris& operator=(const Ephemeris&) = delete;

  EclipticPos position(Body body, JulianDay jdUt) const;
  EquatorialPos equatorial(Body body, JulianDay jdUt) const;

  // Tropical Lilith is affected too. Default is the inclined apogee.
  void setLilithMode(LilithMode mode);

  // Sidereal longitudes and houses. The Ayanamsa overload uses true
  // nutation and shift-then-snap, matching Swiss Ephemeris.
  void setSidereal(bool on, SiderealMethod method);
  void setSidereal(bool on, Ayanamsa mode = Ayanamsa::Lahiri);

  // `latDeg` positive north, `lonDegEast` positive east.
  Houses houses(JulianDay jdUt, double latDeg, double lonDegEast, HouseSystem sys) const;

  bool sidereal() const { return sidereal_; }
  Ayanamsa ayanamsaMode() const { return ayanamsa_; }
  SiderealNutation siderealNutation() const { return nutation_; }
  WholeSignSnap wholeSignSnap() const { return wholeSignSnap_; }
  LilithMode lilithMode() const { return lilith_; }

private:
  struct Sky {
    double lon = 0.0;
    double lat = 0.0;
    double dist = 0.0;
    double ra = 0.0;
    double dec = 0.0;
  };
  Sky sky(Body body, double jdUt) const;
  double siderealShift(double jdUt) const;

  struct Impl;
  std::unique_ptr<Impl> impl_;
  bool sidereal_ = false;
  Ayanamsa ayanamsa_ = Ayanamsa::Lahiri;
  SiderealNutation nutation_ = SiderealNutation::True;
  WholeSignSnap wholeSignSnap_ = WholeSignSnap::ShiftThenSnap;
  LilithMode lilith_ = LilithMode::Inclined;
};

} // namespace eph
