# JavaScript

Install the package, then load one ephemeris:

```
npm install wizards-ephemeris
```

```ts
import { Ephemeris } from 'wizards-ephemeris'

const eph = await Ephemeris.load()
```

Node 20 or newer. The wasm ships inside the package. Positions come back in degrees. The package does not use the chart kernel and does not include Swiss Ephemeris.

```ts
import { Ephemeris } from 'wizards-ephemeris'

const eph = await Ephemeris.load()
const when = new Date(Date.UTC(1988, 0, 30, 15, 10))

const sun = eph.position('sun', when, {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'true',
  wholeSign: 'shift-then-snap',
  lilith: 'inclined'
})
// sun.lon, sun.lat, sun.lonSpeed are degrees. sun.distAu is astronomical units.

const sky = eph.equatorial('sun', when)
const houses = eph.houses(when, { lat: 50.2197, lon: 18.6742 }, 'placidus')
```

`Date.UTC` returns milliseconds. Wrap it in `new Date`, or pass a Julian day, or pass `{ year, month, day, hour }` with `hour` as hours from midnight UT. A `Date` is read in UTC, so the host timezone does not move the chart.

## Options

Each call takes an optional method object. Omitted fields use the Swiss-compatible defaults.

- `zodiac`: `tropical` (default) or `sidereal`
- `ayanamsa`: `lahiri` (default), `raman`, `fagan-bradley`, `krishnamurti`, `citra`, `galactic-center-mula`. Used when the zodiac is sidereal
- `nutation`: `true` (default) or `mean`
- `wholeSign`: `shift-then-snap` (default) or `snap-then-shift`. Used for sidereal whole-sign cusps
- `lilith`: `inclined` (default) or `ecliptic`. This applies to a tropical chart as well

Unknown names throw `TypeError` before the calculation.

Bodies: `sun`, `moon`, `mercury`, `venus`, `mars`, `jupiter`, `saturn`, `uranus`, `neptune`, `pluto`, `true-node`, `mean-lilith`, `chiron`, `ceres`, `pallas`, `juno`, `vesta`.

House systems: `placidus`, `whole-sign`, `koch`, `equal`, `regiomontanus`, `campanus`, `porphyry`, `alcabitius`, `sripati`, `polich-page`. `porphyry` is the C++ enumerator `Porphyrius`.

`houses` returns `cusps` (twelve numbers, index 0 is house 1), `asc`, `mc`, `armc`, `vertex`, and `fallback`. Latitude is positive north, longitude positive east.

`julianDay`, `revJul`, `deltaT` (seconds), `ayanamsa(mode, time, nutation?)` (degrees), and `sep(aDeg, bDeg)` are on the same instance.

## What a sidereal chart changes

Sidereal mode subtracts the ayanamsa from ecliptic longitude and from the house angles. Right ascension, declination, and ARMC stay the apparent place of date. Lilith latitude follows `lilith` before that subtraction. The combinations are in [traditions.md](traditions.md). The C++ surface is in [api.md](api.md).

This build calls `Ephemeris` with an empty kernel directory. Pluto, Chiron, and the four asteroids use the built-in series. There is no `.bsp` argument.

One instance is sequential. The wrapper sets the method on that instance before each call.

Paired programs, the same calculation in C++ and TypeScript, are in [examples.md](examples.md).

`npm install wizards-ephemeris` is the usual way in. Building the wasm yourself, on Linux or in Docker, is in [../js/README.md](../js/README.md).
