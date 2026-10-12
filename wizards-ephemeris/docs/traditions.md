# Traditions

The same sky can be drawn as more than one chart. The switches below pick the reading. The defaults match Swiss Ephemeris, so a sidereal chart, a whole-sign chart, and the mean apogee line up with that ephemeris. The other settings stay available.

| Setting | Values | What it changes |
|---------|--------|-----------------|
| Zodiac | Tropical, Sidereal | Sidereal subtracts an ayanamsa from ecliptic longitude and from house angles |
| Ayanamsa | Lahiri, Raman, Fagan-Bradley, Krishnamurti, True Citra, Galactic center (Mula) | Which sidereal zero is used |
| Sidereal nutation | True, Mean | True also subtracts nutation in longitude. Mean leaves that nutation in the place |
| Whole-sign order | Shift then snap, Snap then shift | Only for a sidereal whole-sign chart. Shift then snap puts cusps on 0° of the sidereal signs |
| Lilith | Inclined, Ecliptic | Inclined gives the mean apogee the latitude of the mean lunar orbit. Ecliptic keeps latitude 0 |

Right ascension of the meridian (ARMC) is a time angle and is not shifted in sidereal mode. Declination stays a coordinate of the sky.

## Western tropical, Hellenistic, medieval

Use the tropical zodiac. The apparent place already includes nutation, which is the equinox of date. Whole sign snaps the tropical ascendant, so the sidereal snap switch does not enter the chart. Lahiri, Raman, and the other ayanamsas are unused.

Mean Lilith is a twentieth-century point. Its ecliptic longitude is the same in both Lilith modes, to the longitude bound in [accuracy.md](accuracy.md). A declination parallel to that point uses the inclined apogee.

## Western sidereal

Fagan-Bradley is the usual ayanamsa. Nutation is True, so the place is the tropical longitude minus the true ayanamsa. A whole-sign chart uses shift then snap: subtract the ayanamsa, then snap the cusps onto the sidereal signs. Inclined Lilith is the apogee Swiss Ephemeris publishes, including its latitude.

## Jyotish

Lahiri (Chitrapaksha, the Indian Calendar Reform ayanamsa) is the usual zero. Nutation is True. Rasi charts are whole sign on the sidereal signs, which is shift then snap. True Citra is Spica's ecliptic longitude minus 180°. Galactic center (Mula) puts the projected right ascension of Sgr A* at the middle of Mula (minus 246°40').

Mean nutation is the printed precessional ayanamsa. It differs from the chart place by the nutation in longitude, about ±18". That moves a sign or a nakshatra only when the planet is already within those 18" of the boundary.

## Older reading kept as a setting

Mean nutation, snap-then-shift whole sign, and ecliptic Lilith are the previous behaviour of this library. Mean nutation leaves nutation in the sidereal longitude. Snap then shift snaps the tropical sign and then subtracts the ayanamsa, so a cusp can sit almost 30° away from the sidereal-sign cusp. Ecliptic Lilith is the longitude alone, with latitude 0, so its declination is not the inclined apogee.

## House systems beyond the first eight

Sripati puts each cusp on the middle of a Porphyry sector. The ascendant and the midheaven are angles of the chart and are not themselves cusps.

Polich-Page (topocentric) divides the latitude. The ascendant is cusp 1 and the midheaven is cusp 10. Inside the polar circle, when the raw ascendant lies west of the midheaven, the angles and cusps are moved by 180° so the houses stay in the same order as the reference.
