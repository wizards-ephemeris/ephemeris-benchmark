# API

```cpp
#include "eph/eph.hpp"

eph::JulianDay jd = eph::julianDayUt(1988, 1, 30, 15.0 + 10.0 / 60.0);
int year, month, day;
double hour;
eph::revJul(jd, year, month, day, hour);

eph::Ephemeris eph;                  // or eph::Ephemeris("/path/to/bsp")
eph.setLilithMode(eph::LilithMode::Inclined);
eph.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::True,
  eph::WholeSignSnap::ShiftThenSnap
});
auto sun = eph.position(eph::Body::Sun, jd);
auto eq = eph.equatorial(eph::Body::Sun, jd);
double sep = eph::sepDeg(sun.lon.deg(), eq.ra.deg());

eph::Houses h = eph.houses(jd, 50.2197, 18.6742, eph::HouseSystem::Placidus);
double asc = h.asc.deg();
double cusp1 = h.cusp[0].deg();      // index 0 is house 1
bool porphyry = h.fallback;          // polar Placidus or Koch
```

`position` returns ecliptic longitude, latitude, longitude speed in degrees per day, and distance in AU. `equatorial` returns right ascension, declination, and RA speed. Both are apparent of date. In sidereal mode only the ecliptic longitude is reduced. Right ascension and declination stay the apparent place of date. Mean Lilith's equator follows the Lilith mode, before that reduction.

`houses` takes latitude positive north and longitude positive east. `armc` is not reduced in sidereal mode. `fallback` is true when Placidus, Koch, or Campanus could not be formed and Porphyry was substituted.

`ayanamsa(mode, jd)` is the mean ayanamsa in an `Angle` (nutation left out). `ayanamsaTrue` adds nutation in longitude. `setSidereal(true, mode)` uses the true ayanamsa and shift-then-snap. `setSidereal(true, SiderealMethod)` selects the ayanamsa, the nutation switch, and the whole-sign order. `setLilithMode` selects the inclined apogee or the ecliptic point, including in a tropical chart. The object is movable and not copyable. `position` and `houses` are const; the setters are not.

`Vec3`, `Angle`, and `Mat3` support `+`, `-`, and scalar `*`. `Angle::deg` / `fromDeg` convert. `sepDeg` is the shortest separation on a circle, in degrees, in `[0, 180]`.

Bodies: `Sun`, `Moon`, `Mercury`, `Venus`, `Mars`, `Jupiter`, `Saturn`, `Uranus`, `Neptune`, `Pluto`, `TrueNode`, `MeanLilith`, `Chiron`, `Ceres`, `Pallas`, `Juno`, `Vesta`.

House systems: `Placidus`, `WholeSign`, `Koch`, `Equal`, `Regiomontanus`, `Campanus`, `Porphyrius`, `Alcabitius`, `Sripati`, `PolichPage`.

Ayanamsas: `Lahiri`, `Raman`, `FaganBradley`, `Krishnamurti`, `Citra`, `GalacticCenterMula`.

Sidereal nutation: `Mean`, `True`. Whole-sign order: `SnapThenShift`, `ShiftThenSnap`. Lilith: `Ecliptic`, `Inclined`. See [traditions.md](traditions.md).
