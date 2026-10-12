# Empheresis Comparator

Copyright (c) 2026 Magowie Sp. z o.o.

A harness that compares astrology libraries on the same dates, the same bodies, and the same house systems. The aim is a growing table of what each library can compute for a natal chart, how far it sits from a reference, and what it costs to call.

Two libraries are wired in today:

- [Wizards Ephemeris](../wizards-ephemeris/) — MIT, series in the source, no data files required.
- [Swiss Ephemeris](https://github.com/aloistr/swisseph) — AGPL or a professional license, the reference column. It is fetched at build time and is not vendored here.

Further libraries belong in this repository as additional adapters. See [docs/adding-a-library.md](docs/adding-a-library.md).

## What a natal chart needs

| Capability | Wizards Ephemeris | Swiss Ephemeris |
|------------|-------------------|-----------------|
| Sun through Pluto, true node, mean Lilith, Chiron, Ceres, Pallas, Juno, Vesta | yes | yes, plus many more bodies |
| Longitude, latitude, speed | yes | yes |
| Declination (parallel aspects) | yes | yes |
| Light-time and aberration | yes | yes |
| Gravitational deflection | no | yes (the comparison turns it off) |
| Ten house systems (Placidus, Whole Sign, Koch, Equal, Regiomontanus, Campanus, Porphyry, Alcabitius, Sripati, Polich-Page) | yes | yes, plus further systems |
| Polar fallback | Porphyry | Porphyry |
| Tropical and sidereal | yes | yes |
| Ayanamsa | Lahiri, Raman, Fagan-Bradley, Krishnamurti, True Citra, Galactic center (Mula) | those and many more |
| True and mean sidereal nutation, two whole-sign orders, ecliptic and inclined Lilith | yes | yes, as flags on the same calls |
| Sidereal house cusps | yes | yes, via `swe_houses_ex` |
| Julian day, calendar, delta T | yes | yes |
| Optional JPL SPK kernels | yes | its own `.se1` files |
| Runs with no external ephemeris files | yes | no, not at full accuracy |
| License of the library itself | MIT | AGPL or Professional |

## Accuracy

Bounds enforced by `empheresis_tests` for Wizards Ephemeris against Swiss Ephemeris, 1900–2100 unless noted. Swiss is called with deflection disabled, because Wizards does not apply it.

| Quantity | Bound |
|----------|-------|
| Sun–Saturn longitude, latitude, speed | 1" (1"/day) |
| Uranus | 2" |
| Neptune | 3" |
| Moon | 10" (speed 30"/day) |
| Pluto (built-in series, every 60 days) | 1" |
| Chiron and the four asteroids | 5" |
| True node longitude and latitude | 10" (speed 120"/day) |
| Mean Lilith longitude, latitude, and speed (inclined) | 2" |
| Ecliptic Lilith latitude | 0.01" (longitude still 2") |
| Ayanamsa, six modes, monthly | 1" |
| Sidereal longitude, true nutation, every 90 days | tropical bound + 1" |
| Sidereal longitude, mean nutation, every 90 days | tropical bound + 20" |
| Equatorial RA/Dec, every 90 days, including inclined Lilith | same as the ecliptic place |
| Ascendant, MC, ARMC, vertex, through 2050 | 5" |
| Cusps, ten systems, through 2050, latitudes ±80° | 10" |
| Sidereal angles and cusps, true ayanamsa, shift-then-snap | 5" angles, 10" cusps, 1" whole sign |
| Sidereal angles and non-whole-sign cusps, mean ayanamsa | 20" |
| Delta T | 15 s |

The default Lilith is the inclined mean apogee, and its latitude and equator of date are compared with the longitude. Ecliptic mode is a separate case: latitude stays under 0.01". True sidereal places are the tropical place minus the true ayanamsa, within 0.05" of that identity and within the tropical bound plus 1" of Swiss. Mean sidereal leaves nutation in the longitude, which is the extra 20". Shift-then-snap Whole Sign cusps match Swiss within 1". Snap-then-shift cusps match the tropical sign minus the mean ayanamsa within 0.05", and can sit almost 30° from the Swiss sidereal snap. At ±89° Placidus and Koch fall back to Porphyry and still match Swiss within 10". The tradition each switch belongs to is in the Wizards Ephemeris [traditions](../wizards-ephemeris/docs/traditions.md) note.

A 1000-date scan of longitude error is in [charts/accuracy.png](charts/accuracy.png). The largest outer-planet figures on that scan are about 1.4" for Uranus and 2.4" for Neptune. Sun through Saturn stay under 0.6".

## Performance

Swiss Ephemeris reads Chebyshev files. Wizards Ephemeris sums series. On the machine that produced these charts, a Swiss position call is on the order of 20 microseconds and a Wizards call is on the order of 0.1–0.6 milliseconds, depending on the body. The Moon is the slow series. Mean Lilith and the built-in minor-body polynomials are among the faster Wizards calls.

- [charts/performance_position.png](charts/performance_position.png)
- [charts/performance_houses.png](charts/performance_houses.png)

How to read them: [docs/charts.md](docs/charts.md).

## Build

C++20. From this directory, with Wizards Ephemeris as a sibling:

```
cmake -S . -B build
cmake --build build
./build/empheresis_tests
./build/bench_position bench_position.json
./build/bench_houses bench_houses.json
python3 tools/make_charts.py bench_position.json bench_houses.json
```

Inside the monorepo the same target is `empheresis_tests`. `EMPH_BUILD_COMPARATOR` defaults to ON for a native build and OFF under Emscripten. Turning it off skips the Swiss download.

## License

GNU Affero General Public License v3.0. See `LICENSE`. The comparison code links Swiss Ephemeris and is distributed under the AGPL for that reason. Wizards Ephemeris itself stays MIT and is not part of this license. Attribution is in `NOTICE`.
