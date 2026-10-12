#pragma once

#include "eph/eph.hpp"

namespace emph {

struct SkyPlace {
  double lonDeg = 0.0;
  double latDeg = 0.0;
  double speedDegPerDay = 0.0;
  double raDeg = 0.0;
  double decDeg = 0.0;
  double raSpeedDegPerDay = 0.0;
};

struct HousePlace {
  double cuspDeg[12] {};
  double ascDeg = 0.0;
  double mcDeg = 0.0;
  double armcDeg = 0.0;
  double vertexDeg = 0.0;
  // True when a quadrant system fell back to Porphyry.
  bool fallback = false;
};

// One astrology library. A new backend is a subclass plus one line in the catalog.
class Adapter {
public:
  virtual ~Adapter() = default;
  virtual const char* name() const = 0;
  virtual bool available() const = 0;
  virtual SkyPlace position(eph::Body body, double jdUt) const = 0;
  virtual SkyPlace equatorial(eph::Body body, double jdUt) const = 0;
  virtual HousePlace houses(double jdUt, double latDeg, double lonEast, eph::HouseSystem sys) const = 0;
  virtual double ayanamsaDeg(eph::Ayanamsa mode, double jdUt) const = 0;
  virtual void setSidereal(bool on, eph::Ayanamsa mode) = 0;
  virtual void setSidereal(bool on, eph::SiderealMethod method) = 0;
  virtual void setLilithMode(eph::LilithMode mode) = 0;
};

} // namespace emph
