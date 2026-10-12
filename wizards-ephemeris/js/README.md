# wizards-ephemeris

JavaScript bindings for [Wizards Ephemeris](../README.md). Stock Emscripten builds the wasm. The TypeScript API returns degrees and takes one options object per call.

```ts
import { Ephemeris } from 'wizards-ephemeris'

const eph = await Ephemeris.load()
const sun = eph.position('sun', new Date(Date.UTC(1988, 0, 30, 15, 10)), {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'true',
  wholeSign: 'shift-then-snap',
  lilith: 'inclined'
})
```

`sun.lon`, `sun.lat`, and `sun.lonSpeed` are degrees. `sun.distAu` is astronomical units. `Date.UTC` returns milliseconds, so wrap it in `new Date`. A number is a Julian day. `{ year, month, day, hour }` uses hours from midnight UT. A `Date` is read in UTC.

Defaults, when a field is omitted: tropical zodiac, Lahiri, true nutation, shift-then-snap, inclined Lilith. Lilith mode applies to tropical charts too. Sidereal mode reduces ecliptic longitude and house angles. Right ascension, declination, and ARMC stay the place of date.

House systems use the name `porphyry`. The C++ enumerator is `Porphyrius`.

This build does not take a `.bsp` directory. Pluto, Chiron, and the four asteroids use the built-in series. The calculation notes are in [../docs/javascript.md](../docs/javascript.md).

## Build

From the library directory, with system `emcmake` (not a p2996 toolchain):

```
emcmake cmake -S . -B build-js -DEPH_JS=ON
cmake --build build-js --target eph_js
```

That writes `js/dist/eph.js` and `js/dist/eph.wasm`. Then, in this directory:

```
npm install
npm run build
```

`npm run build` runs `tsc` and emits `dist/index.js` next to the wasm glue. `EPH_JS` stays off unless you pass it, so a normal C++ build and the monorepo chart build do not produce this target.

## Tests

Node and Chromium run the same cases against `test/fixtures.json`. Angles may differ from the native fixture by 0.05 arcsecond.

```
npm test
npm run test:browser
```

Regenerate the fixture file from a native build of the library (the `eph_fixture` target, top-level cmake, no Emscripten):

```
cmake -S . -B build
cmake --build build --target eph_fixture
./build/eph_fixture js/test/fixtures.json
```

## License

MIT. See [../LICENSE](../LICENSE).
