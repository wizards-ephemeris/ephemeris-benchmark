# Accuracy

The comparison lives in Empheresis Comparator (`empheresis_tests`). It calls Swiss Ephemeris with deflection disabled. This library does not apply gravitational deflection, which reaches a few arcseconds when a planet is within about 0.2° of the Sun and stays under 0.1" otherwise. The bounds below are what that suite enforces on a monthly grid from JD 2415021 (1900) to JD 2488070 (2100), unless a row says otherwise. Longitude speed uses the same numbers in arcseconds per day, except the Moon (30"/day, a forward difference over 10⁻⁴ day) and the true node (120"/day, because that rate jumps).

| Quantity | Bound |
|----------|-------|
| Sun–Saturn longitude, latitude, and speed | 1" (speed: 1"/day) |
| Uranus | 2" |
| Neptune | 3" |
| Moon longitude and latitude | 10" (speed 30"/day) |
| Pluto | 1" (`full` series, every 60 days) |
| Chiron, Ceres, Pallas, Juno, Vesta | 5" (`full` series, every 60 days) |
| True node longitude and latitude | 10" (speed 120"/day) |
| Mean Lilith longitude, latitude, and speed | 2" |
| Ayanamsa, six modes, monthly through 2100 | 1" |
| Equatorial RA, Dec, and RA speed, every 90 days, all bodies | same as the ecliptic place |
| Ascendant, MC, ARMC, vertex | 5" |
| House cusps, all ten systems, through 2050 | 10" |
| ΔT versus the comparison ephemeris, 1900–2100 | 15 s |

The Pluto and asteroid rows are the `full` preset, 16-day Chebyshev segments. The default preset is `year`: one polynomial of order 11 per 365 days. Against that 16-day table it adds about 0.3" (Pluto is the worst of the six). The year polynomial replaces about 1.8 MB of 16-day coefficients. `cmake -DEPH_MINOR_SERIES=full` restores the 1" and 5" bounds. The wasm build takes the same switch: `EPH_MINOR_SERIES=full ./scripts/build-js.sh`.

Right ascension and declination use the ecliptic bounds above. The grid is every 90 days and covers every body, including mean Lilith in the inclined mode.

Mean Lilith's longitude is the ELP mean perigee plus 180°, general precession, nutation in longitude, and one periodic term `416.43" sin 2(node − perigee)`. Without that term the longitude sits about 7' off the reference. The default mode is the inclined apogee: the same longitude, with latitude from the mean lunar inclination (5.1453964°). Latitude, right ascension, and declination then stay inside 2". Ecliptic mode forces latitude to 0 (under 0.01") and keeps the longitude bound. Which chart uses which mode is in [traditions.md](traditions.md).

Sidereal longitudes default to the tropical place minus the true ayanamsa (the mean ayanamsa plus nutation in longitude). That identity is enforced to 0.05". Against Swiss Ephemeris the true mode allows the tropical bound plus 1", for every body and all six ayanamsas, every 90 days. Mean mode subtracts only the precessional ayanamsa, so nutation (about ±18") stays in the longitude, and the comparison allows the tropical bound plus 20". The Sun's sidereal speed matches its tropical speed minus the ayanamsa rate of the selected mode to 0.05"/day. Against Swiss's analytic rate the forward difference is allowed 0.5"/day.

House tests stop at JD 2469807.5 (2050). Past that date the delta-T polynomials diverge by several seconds (about 6 s at the worst sample through 2100). Six seconds of Earth rotation is 90" of RAMC, and near the polar circle a Koch cusp amplifies that. Inside the window the geometry matches the arcsecond bounds above, on latitudes from −80° to +80°, including a monthly slice at 50.22°. At ±89° Placidus and Koch fall back to Porphyry; those cusps still match Swiss within 10", and a fallback result matches a Porphyry call here to 0.01".

Sidereal houses default to the true ayanamsa and, for Whole Sign, to shift-then-snap. Ascendant, midheaven, and vertex stay within 5" of Swiss. Cusps of the other nine systems stay within 10". Whole Sign cusps, snapped after the shift, stay within 1". ARMC is not shifted. It stays within 5" of Swiss and within 0.05" of this library's tropical ARMC.

Mean nutation leaves about ±18" in the angles, so those angles and the non-whole-sign cusps are allowed 20" against the Swiss true place. Snap-then-shift Whole Sign cusps are the tropical signs moved by the mean ayanamsa. That identity is enforced to 0.05". The Swiss whole-sign cusp is the sidereal sign, and the two can differ by almost 30°.

Uranus and Neptune are the VSOP87 drift against a modern numerical ephemeris. The worst points are late in the century (about 1.5" and 2.5"), not a truncation bug in the inner planets. Sun through Saturn stay under 0.6" on the same grid.

Keplerian fallback for Pluto and the minor bodies is not covered by those arcsecond bounds. A `.bsp` kernel, when present and covering the date, replaces the built-in series; that path is implemented and not part of the default comparison grid.
