# @magowie/wizards-ephemeris

JavaScript bindings for [Wizards Ephemeris](../README.md). Stock Emscripten builds the wasm. The TypeScript API returns degrees and takes one options object per call.

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

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

The package name is `@magowie/wizards-ephemeris`. It is not on the npm registry, and it does not have its own git repository yet. `private` stays set, so `npm publish` is refused. A local build writes `dist/eph.js`, `dist/eph.wasm`, and the TypeScript glue, then `npm pack` puts them in `pack/`.

Install that tarball where you want to use it:

```
npm install ./pack/magowie-wizards-ephemeris-0.1.0.tgz
```

Node loads the wasm from the same directory as `dist/index.js`. In a bundler, pass the wasm URL to `Ephemeris.load({ wasmUrl })`.

Emscripten 6.0.11 and Node 20 or newer. This is stock `emcmake`, not the p2996 toolchain. `EPH_JS` stays off unless the script passes it, so a normal C++ build does not produce this target.

### Linux

From a clean directory:

```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install 6.0.11
./emsdk activate 6.0.11
source ./emsdk_env.sh
```

Then, from the library directory:

```
./scripts/build-js.sh
```

The script configures `build-js`, builds `eph_js`, runs the Node tests, and packs `js/pack/magowie-wizards-ephemeris-0.1.0.tgz`.

### Docker

From the library directory, with Docker BuildKit:

```
docker build -f Dockerfile -o js/pack .
```

The first stage is `emscripten/emsdk:6.0.11`. The second stage is `node:20` and runs the Node tests before packing. The tarball is the only file written to `js/pack`. The image does not publish the package. The Chromium test stays on the host: `npm run test:browser` from `js/`, after the wasm exists in `js/dist`.

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
