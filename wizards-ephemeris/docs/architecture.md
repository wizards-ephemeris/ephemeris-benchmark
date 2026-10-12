# Architecture

Julian days on the public API are Universal Time. Internally, dynamical time is UT plus ΔT from the Espenak / Meeus polynomials. After 2005 the polynomial is a slowing rise from 64.70 s, because the older parabola runs away from the measured clock error.

## Apparent place

Sun through Neptune start as VSOP87B heliocentric vectors on the dynamical ecliptic and equinox of J2000. The Earth series is used as the observer. Subtracting the lunar offset from the Earth-Moon barycentre adds a monthly term of about 6" that the reference geocentric places do not have, so that correction is not applied.

A planet is evaluated at the light-time epoch, iterated twice, with `c ≈ 173.1446` AU/day. The Sun is the origin, so its direction is the observer vector at reception and only aberration moves it. The result is rotated from the mean ecliptic of J2000 to the true equator of date with IAU 2006 Fukushima-Williams precession and IAU 2000B nutation, then to the ecliptic of date. Stellar aberration is the first-order `normalize(u + v/c)` using the heliocentric Earth velocity. Frame bias and gravitational light deflection are omitted. Longitude speed is a one-sided difference of the final longitude over `1e-4` day.

The Moon is ELP2000-82B, geocentric at one instant. Light leaves at `t − τ` and arrives at the Earth at `t`, so the inertial direction is `moon(t−τ) + earth(t−τ) − earth(t)` before the same aberration. Those two shifts are each about 20" and cancel to about 1".

## Pluto, Chiron, asteroids

`Ephemeris` scans `spkDir` for DAF/SPK type 2 and type 3 (Chebyshev) segments. A segment that covers the date replaces the built-in series for that body. Otherwise a Chebyshev fit of public Horizons states is used (order 7, 16-day segments, 1900–2100). Outside that span, or if the series does not cover the date, Keplerian elements from JPL SBDB are the last resort. Those elements will not hold an arcminute over two centuries.

## Node and Lilith

The true node is the osculating ascending node of the geometric lunar orbit, with light time and no Earth-reception correction. Mean Lilith's longitude is the ELP mean perigee plus 180°, general precession, nutation in longitude, and one periodic term `416.43" sin 2(node − perigee)`. Inclined mode then places that longitude on the mean lunar orbit (inclination 5.1453964°) and rebuilds right ascension and declination. Ecliptic mode leaves the latitude at 0.

## Houses

Ten systems: Placidus, Whole Sign, Koch, Equal, Regiomontanus, Campanus, Porphyry, Alcabitius, Sripati, and Polich-Page. Ascendant, MC, ARMC, and vertex come from the true obliquity of date and Greenwich apparent sidereal time.

Placidus and Koch are undefined once any ecliptic degree is circumpolar (`|latitude| + ε ≥ 90°`). They then use Porphyry and set `Houses::fallback`. Campanus does the same if the horizon basis cannot be built. This follows the reference ephemeris, which fills Porphyry in that case rather than Whole Sign.

When the formula midheaven is below the horizon, Campanus and Regiomontanus report the opposite meridian point as MC, matching their 10th cusp. Equal and Whole Sign keep the formula MC. Alcabitius trisects right ascension from the MC to the ascendant. Koch steps the ascendant by thirds of the semi-diurnal arc of the MC. Sripati puts cusps on the midpoints of the Porphyry sectors, so the ascendant is not a cusp. Polich-Page uses ascendants at `atan(tan φ / 3)` and `atan(2 tan φ / 3)`. Inside the polar circle, when the raw ascendant lies west of the midheaven, Polich-Page adds 180° to the angles and the cusps.

## Sidereal

`setSidereal(true, SiderealMethod)` subtracts an ayanamsa from ecliptic longitudes and from ascendant, MC, vertex, and cusps. True nutation subtracts the mean ayanamsa plus nutation in longitude. Mean nutation subtracts only the mean. ARMC is a time angle and is not reduced. Right ascension and declination of the planets stay true of date. For Whole Sign, shift-then-snap subtracts the ayanamsa from the ascendant and then snaps the cusps to 30°; snap-then-shift snaps the tropical ascendant first.

Lahiri, Raman, Fagan-Bradley, and Krishnamurti are polynomials: a J2000 constant plus IAU 2006 general precession. True Citra is the mean ecliptic longitude of Spica minus 180°, with proper motion and annual aberration. Galactic center (Mula) projects the mean right ascension of Sgr A* onto the ecliptic the way a midheaven is computed, then subtracts 246°40'. `ayanamsa` returns the mean value. `ayanamsaTrue` adds nutation in longitude.
