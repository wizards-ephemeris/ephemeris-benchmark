import { Ephemeris } from '@magowie/wizards-ephemeris'
import { citraGalactic } from '../../examples/ts/citra_galactic.ts'
import { deltaT } from '../../examples/ts/delta_t.ts'
import { eclipticLilith } from '../../examples/ts/ecliptic_lilith.ts'
import { equalHouses } from '../../examples/ts/equal_houses.ts'
import { equatorFixed } from '../../examples/ts/equator_fixed.ts'
import { equatorialSun } from '../../examples/ts/equatorial_sun.ts'
import { armcFixed } from '../../examples/ts/armc_fixed.ts'
import { inclinedLilith } from '../../examples/ts/inclined_lilith.ts'
import { julianDay } from '../../examples/ts/julian_day.ts'
import { lahiriChart } from '../../examples/ts/lahiri_chart.ts'
import { meanNutation } from '../../examples/ts/mean_nutation.ts'
import { placidus } from '../../examples/ts/placidus.ts'
import { polarFallback } from '../../examples/ts/polar_fallback.ts'
import { polichPage } from '../../examples/ts/polich_page.ts'
import { separation } from '../../examples/ts/separation.ts'
import { snapThenShift } from '../../examples/ts/snap_then_shift.ts'
import { sripati } from '../../examples/ts/sripati.ts'
import { tropicalSun } from '../../examples/ts/tropical_sun.ts'
import { utcDate } from '../../examples/ts/utc_date.ts'
import { wholeSign } from '../../examples/ts/whole_sign.ts'

function boundaryArcsec(deg: number): number {
  const mod = ((deg % 30) + 30) % 30
  return Math.min(mod, 30 - mod) * 3600
}

export async function runExampleTests(): Promise<string[]> {
  const failures: string[] = []
  const fail = (label: string) => {
    failures.push(label)
  }
  const eph = await Ephemeris.load()
  const near = (actual: number, expected: number, arcsec: number, label: string) => {
    if (eph.sep(actual, expected) * 3600 > arcsec) fail(label)
  }

  const day = await julianDay()
  if (day.jd !== 2451545) fail('julian day value')
  if (day.back.year !== 2000 || day.back.month !== 1 || day.back.day !== 1) {
    fail('julian day calendar')
  }
  if (Math.abs(day.back.hour - 12) > 1e-6) fail('julian day hour')

  const utc = await utcDate()
  if (utc.fromFields !== 2451545) fail('utc fields')
  if (utc.fromDate !== utc.fromFields) fail('utc date')

  const seconds = await deltaT()
  if (!(seconds > 0 && seconds < 120)) fail('delta T')

  const sun = await tropicalSun()
  if (!(sun.lon >= 270 && sun.lon < 300)) fail('sun sign')
  if (Math.abs(sun.lat) > 1) fail('sun latitude')
  if (!(sun.lonSpeed >= 0.9 && sun.lonSpeed <= 1.1)) fail('sun speed')

  const sky = await equatorialSun()
  if (!Number.isFinite(sky.ra)) fail('sun ra')
  if (!(sky.dec >= -24 && sky.dec <= -22)) fail('sun declination')

  const arcs = await separation()
  if (Math.abs(arcs.quarter - 20) > 1e-6) fail('separation 10 to 350')
  if (arcs.same !== 0) fail('separation with itself')

  const lahiri = await lahiriChart()
  near(lahiri.siderealLon, lahiri.tropicalLon - lahiri.ayanamsa, 0.05, 'lahiri identity')

  const nut = await meanNutation()
  const sunGap = eph.sep(nut.trueLon, nut.meanLon)
  const ayGap = eph.sep(nut.trueAyanamsa, nut.meanAyanamsa)
  if (Math.abs(sunGap - ayGap) * 3600 > 0.05) fail('nutation gaps differ')
  const nutArcsec = sunGap * 3600
  if (!(nutArcsec > 0.001 && nutArcsec < 20)) fail('nutation size')

  const zeros = await citraGalactic()
  for (const value of [zeros.lahiri, zeros.citra, zeros.galactic]) {
    if (!(value >= 0 && value < 360)) fail('ayanamsa range')
  }
  const citraSep = eph.sep(zeros.citra, zeros.lahiri)
  if (!(citraSep > 0.01 && citraSep < 0.05)) fail('citra vs lahiri')
  if (eph.sep(zeros.galactic, zeros.lahiri) <= 0.5) fail('galactic vs lahiri')

  const inclined = await inclinedLilith()
  if (!Number.isFinite(inclined.lon)) fail('inclined longitude')
  if (!(inclined.lat >= 0.5 && inclined.lat <= 5.2)) fail('inclined latitude')

  const lilith = await eclipticLilith()
  if (Math.abs(lilith.ecliptic.lat) * 3600 >= 0.01) fail('ecliptic latitude')
  near(lilith.inclined.lon, lilith.ecliptic.lon, 0.05, 'lilith longitude')

  const plac = await placidus()
  near(plac.cusps[0], plac.asc, 0.05, 'placidus cusp 1')
  near(plac.cusps[9], plac.mc, 0.05, 'placidus cusp 10')
  if (plac.fallback) fail('placidus fallback')

  const equal = await equalHouses()
  for (let i = 0; i < 12; i += 1) {
    near(equal.cusps[i], equal.asc + i * 30, 0.05, 'equal cusp')
  }

  const whole = await wholeSign()
  for (const cusp of whole.cusps) {
    if (boundaryArcsec(cusp) > 0.05) fail('whole sign boundary')
  }

  const snapped = await snapThenShift()
  const nearest = Math.round(snapped.cusps[0] / 30) * 30
  if (eph.sep(snapped.cusps[0], nearest) <= 1) fail('snap then shift still on a boundary')

  const armc = await armcFixed()
  near(armc.tropical.armc, armc.sidereal.armc, 0.05, 'armc shifted')
  if (eph.sep(armc.tropical.asc, armc.sidereal.asc) <= 1) fail('ascendant did not move')

  const equator = await equatorFixed()
  near(equator.tropicalSky.ra, equator.siderealSky.ra, 0.05, 'right ascension shifted')
  if (eph.sep(equator.tropical.lon, equator.sidereal.lon) <= 1) fail('longitude did not move')

  const sri = await sripati()
  if (!Number.isFinite(sri.asc) || !Number.isFinite(sri.mc)) fail('sripati angles')
  if (eph.sep(sri.cusps[0], sri.asc) <= 1) fail('sripati cusp is the ascendant')

  const polich = await polichPage()
  near(polich.cusps[0], polich.asc, 0.05, 'polich cusp 1')
  near(polich.cusps[9], polich.mc, 0.05, 'polich cusp 10')

  const polar = await polarFallback()
  if (!polar.placidus.fallback) fail('polar placidus')
  if (!polar.koch.fallback) fail('polar koch')

  return failures
}
