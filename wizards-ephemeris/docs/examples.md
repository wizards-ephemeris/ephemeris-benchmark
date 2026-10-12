# Examples

These programs are the calls a chart makes.
The date, unless a story says otherwise, is 1 January 2000 at 12:00 UT.
Houses use 50.22° N and 18.67° E.
On the documentation site the language control switches every sample together, and the choice is remembered.

## Time

### Julian day

1 January 2000 at 12:00 UT is Julian day 2451545. revJul turns that day back into the calendar date.

::: example Julian day
```cpp
#include "eph/eph.hpp"

eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
int year = 0;
int month = 0;
int day = 0;
double hour = 0.0;
eph::revJul(jd, year, month, day, hour);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const jd = eph.julianDay({ year: 2000, month: 1, day: 1, hour: 12 })
const back = eph.revJul(jd)
```
:::

### A Date in UTC

Pass the date as year, month, day, and hours from midnight UT. TypeScript also accepts a Date, read in UTC. Date.UTC by itself is a count of milliseconds, so wrap it in new Date.

::: example A Date in UTC
```cpp
#include "eph/eph.hpp"

eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const fromFields = eph.julianDay({ year: 2000, month: 1, day: 1, hour: 12 })
const fromDate = eph.julianDay(new Date(Date.UTC(2000, 0, 1, 12, 0, 0)))
```
:::

### Delta T

Delta T is TT minus UT, in seconds. Around the year 2000 it is about a minute.

::: example Delta T
```cpp
#include "eph/eph.hpp"

eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
double seconds = eph::deltaTSeconds(jd);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const seconds = eph.deltaT({ year: 2000, month: 1, day: 1, hour: 12 })
```
:::

## A tropical chart

### Tropical Sun

On this date the Sun is in Capricorn, within a degree of the ecliptic, and moves about one degree a day. Longitude, latitude, and speed are degrees. Distance is astronomical units.

::: example Tropical Sun
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::EclipticPos sun = chart.position(eph::Body::Sun, jd);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const sun = eph.position('sun', { year: 2000, month: 1, day: 1, hour: 12 })
```
:::

### Equatorial Sun

Right ascension and declination are the equatorial place of date. A parallel aspect uses these. In January the Sun's declination is about 23° south.

::: example Equatorial Sun
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::EquatorialPos sun = chart.equatorial(eph::Body::Sun, jd);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const sun = eph.equatorial('sun', { year: 2000, month: 1, day: 1, hour: 12 })
```
:::

### Separation on the circle

sep is the shortest arc on the circle, in degrees, from 0 to 180. From 10° to 350° that arc is 20°, not 340°. A longitude separated from itself is 0.

::: example Separation on the circle
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::EclipticPos sun = chart.position(eph::Body::Sun, jd);
double quarter = eph::sepDeg(10.0, 350.0);
double same = eph::sepDeg(sun.lon.deg(), sun.lon.deg());
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const sun = eph.position('sun', { year: 2000, month: 1, day: 1, hour: 12 })
const quarter = eph.sep(10, 350)
const same = eph.sep(sun.lon, sun.lon)
```
:::

## The same Sun, sidereal

### Lahiri sidereal Sun

A sidereal longitude is the tropical longitude minus the ayanamsa. Lahiri with true nutation is the offset a Swiss-compatible chart subtracts. The whole-sign order is set on the same call. It does not move the Sun.

::: example Lahiri sidereal Sun
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::EclipticPos tropical = chart.position(eph::Body::Sun, jd);
double ayanamsa = eph::ayanamsaTrue(eph::Ayanamsa::Lahiri, jd).deg();

chart.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::True,
  eph::WholeSignSnap::ShiftThenSnap
});
eph::EclipticPos sidereal = chart.position(eph::Body::Sun, jd);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const when = { year: 2000, month: 1, day: 1, hour: 12 }
const tropical = eph.position('sun', when)
const ayanamsa = eph.ayanamsa('lahiri', when, 'true')
const sidereal = eph.position('sun', when, {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'true',
  wholeSign: 'shift-then-snap'
})
```
:::

### Mean nutation

Mean nutation leaves the nutation in longitude inside the sidereal place. The two Suns differ by that nutation, a fraction of an arcminute, not by a new zodiac.

::: example Mean nutation
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
chart.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::True,
  eph::WholeSignSnap::ShiftThenSnap
});
eph::EclipticPos asTrue = chart.position(eph::Body::Sun, jd);

chart.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::Mean,
  eph::WholeSignSnap::ShiftThenSnap
});
eph::EclipticPos asMean = chart.position(eph::Body::Sun, jd);
double ayTrue = eph::ayanamsaTrue(eph::Ayanamsa::Lahiri, jd).deg();
double ayMean = eph::ayanamsa(eph::Ayanamsa::Lahiri, jd).deg();
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const when = { year: 2000, month: 1, day: 1, hour: 12 }
const asTrue = eph.position('sun', when, {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'true',
  wholeSign: 'shift-then-snap'
})
const asMean = eph.position('sun', when, {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'mean',
  wholeSign: 'shift-then-snap'
})
const trueAyanamsa = eph.ayanamsa('lahiri', when, 'true')
const meanAyanamsa = eph.ayanamsa('lahiri', when, 'mean')
```
:::

### Citra and Galactic center

True Citra and Lahiri both put Spica at the sidereal zero, so on this date they sit about an arcminute apart. Galactic center (Mula) is a different zero: the projected right ascension of Sgr A* at the middle of Mula, several degrees from Lahiri. All three values include nutation in longitude.

::: example Citra and Galactic center
```cpp
#include "eph/eph.hpp"

eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
double lahiri = eph::ayanamsaTrue(eph::Ayanamsa::Lahiri, jd).deg();
double citra = eph::ayanamsaTrue(eph::Ayanamsa::Citra, jd).deg();
double galactic = eph::ayanamsaTrue(eph::Ayanamsa::GalacticCenterMula, jd).deg();
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const when = { year: 2000, month: 1, day: 1, hour: 12 }
const lahiri = eph.ayanamsa('lahiri', when, 'true')
const citra = eph.ayanamsa('citra', when, 'true')
const galactic = eph.ayanamsa('galactic-center-mula', when, 'true')
```
:::

## Lilith

### Inclined Lilith

The default mean Lilith sits on the mean lunar orbit. Its latitude reaches about 5.15°.

::: example Inclined Lilith
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
chart.setLilithMode(eph::LilithMode::Inclined);
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::EclipticPos lilith = chart.position(eph::Body::MeanLilith, jd);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const lilith = eph.position('mean-lilith', { year: 2000, month: 1, day: 1, hour: 12 }, {
  lilith: 'inclined'
})
```
:::

### Ecliptic Lilith

Ecliptic Lilith keeps the same longitude and sets the latitude to 0. A declination taken from that point is not the latitude of the orbit.

::: example Ecliptic Lilith
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);

chart.setLilithMode(eph::LilithMode::Inclined);
eph::EclipticPos inclined = chart.position(eph::Body::MeanLilith, jd);

chart.setLilithMode(eph::LilithMode::Ecliptic);
eph::EclipticPos ecliptic = chart.position(eph::Body::MeanLilith, jd);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const when = { year: 2000, month: 1, day: 1, hour: 12 }
const inclined = eph.position('mean-lilith', when, { lilith: 'inclined' })
const ecliptic = eph.position('mean-lilith', when, { lilith: 'ecliptic' })
```
:::

## Houses

The houses below are for 1 January 2000, 12:00 UT, at 50.22° N and 18.67° E, unless the story names another latitude.

### Placidus

For Placidus, cusp 1 is the ascendant and cusp 10 is the midheaven. The cusp array starts at 0, so cusp 1 is index 0. At an ordinary latitude fallback stays false.

::: example Placidus
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Placidus);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const houses = eph.houses(
  { year: 2000, month: 1, day: 1, hour: 12 },
  { lat: 50.22, lon: 18.67 },
  'placidus'
)
```
:::

### Equal houses

Equal houses step 30° from the ascendant. They are not snapped to the start of a sign.

::: example Equal houses
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Equal);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const houses = eph.houses(
  { year: 2000, month: 1, day: 1, hour: 12 },
  { lat: 50.22, lon: 18.67 },
  'equal'
)
```
:::

### Whole sign, shift then snap

Subtract the ayanamsa from the ascendant, then snap to a sign. The cusps fall on 0° of the sidereal signs. This chart uses Lahiri and true nutation.

::: example Whole sign, shift then snap
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
chart.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::True,
  eph::WholeSignSnap::ShiftThenSnap
});
eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::WholeSign);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const houses = eph.houses(
  { year: 2000, month: 1, day: 1, hour: 12 },
  { lat: 50.22, lon: 18.67 },
  'whole-sign',
  {
    zodiac: 'sidereal',
    ayanamsa: 'lahiri',
    nutation: 'true',
    wholeSign: 'shift-then-snap'
  }
)
```
:::

### Whole sign, snap then shift

Snap the tropical ascendant to a sign, then subtract the ayanamsa. The cusp leaves the sign boundary by the remainder of the ayanamsa.

::: example Whole sign, snap then shift
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
chart.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::True,
  eph::WholeSignSnap::SnapThenShift
});
eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::WholeSign);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const houses = eph.houses(
  { year: 2000, month: 1, day: 1, hour: 12 },
  { lat: 50.22, lon: 18.67 },
  'whole-sign',
  {
    zodiac: 'sidereal',
    ayanamsa: 'lahiri',
    nutation: 'true',
    wholeSign: 'snap-then-shift'
  }
)
```
:::

### ARMC stays put

ARMC is the right ascension of the meridian, a time angle. Sidereal mode does not shift it. The ascendant does move.

::: example ARMC stays put
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::Houses tropical = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Placidus);

chart.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::True,
  eph::WholeSignSnap::ShiftThenSnap
});
eph::Houses sidereal = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Placidus);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const when = { year: 2000, month: 1, day: 1, hour: 12 }
const place = { lat: 50.22, lon: 18.67 }
const tropical = eph.houses(when, place, 'placidus')
const sidereal = eph.houses(when, place, 'placidus', {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'true',
  wholeSign: 'shift-then-snap'
})
```
:::

### Right ascension stays put

Sidereal mode subtracts the ayanamsa from ecliptic longitude. Right ascension stays the apparent place of date.

::: example Right ascension stays put
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::EclipticPos tropical = chart.position(eph::Body::Sun, jd);
eph::EquatorialPos tropicalSky = chart.equatorial(eph::Body::Sun, jd);

chart.setSidereal(true, eph::SiderealMethod{
  eph::Ayanamsa::Lahiri,
  eph::SiderealNutation::True,
  eph::WholeSignSnap::ShiftThenSnap
});
eph::EclipticPos sidereal = chart.position(eph::Body::Sun, jd);
eph::EquatorialPos siderealSky = chart.equatorial(eph::Body::Sun, jd);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const when = { year: 2000, month: 1, day: 1, hour: 12 }
const tropical = eph.position('sun', when)
const tropicalSky = eph.equatorial('sun', when)
const sidereal = eph.position('sun', when, {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'true',
  wholeSign: 'shift-then-snap'
})
const siderealSky = eph.equatorial('sun', when, {
  zodiac: 'sidereal',
  ayanamsa: 'lahiri',
  nutation: 'true',
  wholeSign: 'shift-then-snap'
})
```
:::

## Less common house systems

### Sripati

Sripati puts each cusp in the middle of a Porphyry sector. The ascendant is still an angle of the chart, and it is not a cusp.

::: example Sripati
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::Sripati);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const houses = eph.houses(
  { year: 2000, month: 1, day: 1, hour: 12 },
  { lat: 50.22, lon: 18.67 },
  'sripati'
)
```
:::

### Polich-Page

In Polich-Page the ascendant is cusp 1 and the midheaven is cusp 10.

::: example Polich-Page
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::Houses h = chart.houses(jd, 50.22, 18.67, eph::HouseSystem::PolichPage);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const houses = eph.houses(
  { year: 2000, month: 1, day: 1, hour: 12 },
  { lat: 50.22, lon: 18.67 },
  'polich-page'
)
```
:::

### Inside the polar circle

At latitude 80° a quadrant house cannot always be formed. Placidus and Koch then use Porphyry and set fallback.

::: example Inside the polar circle
```cpp
#include "eph/eph.hpp"

eph::Ephemeris chart;
eph::JulianDay jd = eph::julianDayUt(2000, 1, 1, 12.0);
eph::Houses placidus = chart.houses(jd, 80.0, 18.67, eph::HouseSystem::Placidus);
eph::Houses koch = chart.houses(jd, 80.0, 18.67, eph::HouseSystem::Koch);
```

```ts
import { Ephemeris } from '@magowie/wizards-ephemeris'

const eph = await Ephemeris.load()
const when = { year: 2000, month: 1, day: 1, hour: 12 }
const place = { lat: 80, lon: 18.67 }
const placidus = eph.houses(when, place, 'placidus')
const koch = eph.houses(when, place, 'koch')
```
:::
