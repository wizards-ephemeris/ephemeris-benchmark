import { Ephemeris, type Body, type EclipticPos, type EquatorialPos, type Houses } from '../src/index.js'

interface Fixtures {
  jd: number
  deltaT: number
  lat: number
  lonEast: number
  sunTropical: EclipticPos
  sunTropicalEquatorial: EquatorialPos
  sunSiderealLahiri: EclipticPos
  lilithInclined: EclipticPos
  lilithEcliptic: EclipticPos
  placidus: Houses
  wholeSignSidereal: Houses
}

export interface Report {
  ok: boolean
  failures: string[]
}

const arcsec = 0.05
const speedTol = arcsec / 3600
const distTol = 1e-9

async function loadFixtures(): Promise<Fixtures> {
  const url = new URL('./fixtures.json', import.meta.url)
  if (typeof process !== 'undefined' && process.versions?.node) {
    const { readFile } = await import('node:fs/promises')
    const { fileURLToPath } = await import('node:url')
    return JSON.parse(await readFile(fileURLToPath(url), 'utf8')) as Fixtures
  }
  const response = await fetch(url)
  if (!response.ok) throw new Error(`fixtures ${response.status}`)
  return response.json() as Promise<Fixtures>
}

function boundaryArcsec(deg: number): number {
  const mod = ((deg % 30) + 30) % 30
  return Math.min(mod, 30 - mod) * 3600
}

export async function runCases(): Promise<Report> {
  const failures: string[] = []
  const fixtures = await loadFixtures()
  const eph = await Ephemeris.load()
  const jd = fixtures.jd
  const place = { lat: fixtures.lat, lon: fixtures.lonEast }
  const sidereal = {
    zodiac: 'sidereal' as const,
    ayanamsa: 'lahiri' as const,
    nutation: 'true' as const,
    wholeSign: 'shift-then-snap' as const,
    lilith: 'inclined' as const
  }

  const fail = (label: string, detail: string) => {
    failures.push(`${label}: ${detail}`)
  }

  const nearAngle = (actual: number, expected: number, label: string) => {
    const diff = eph.sep(actual, expected) * 3600
    if (diff > arcsec) fail(label, `${diff} arcsec`)
  }

  const nearLinear = (actual: number, expected: number, label: string, tol: number) => {
    const diff = Math.abs(actual - expected)
    if (diff > tol) fail(label, `${diff}`)
  }

  const checkEcliptic = (actual: EclipticPos, expected: EclipticPos, label: string) => {
    nearAngle(actual.lon, expected.lon, `${label} lon`)
    nearLinear(actual.lat, expected.lat, `${label} lat`, arcsec / 3600)
    nearLinear(actual.lonSpeed, expected.lonSpeed, `${label} speed`, speedTol)
    nearLinear(actual.distAu, expected.distAu, `${label} distance`, distTol)
  }

  const calendar = eph.julianDay({ year: 2000, month: 1, day: 1, hour: 12 })
  if (calendar !== 2451545) fail('julian day', String(calendar))
  const fromDate = eph.julianDay(new Date(Date.UTC(2000, 0, 1, 12, 0, 0)))
  if (fromDate !== 2451545) fail('julian day from Date', String(fromDate))

  const back = eph.revJul(2451545)
  if (back.year !== 2000 || back.month !== 1 || back.day !== 1) {
    fail('revJul', JSON.stringify(back))
  }
  nearLinear(back.hour, 12, 'revJul hour', 1e-6)

  nearLinear(eph.deltaT(jd), fixtures.deltaT, 'delta T', 1e-6)
  nearLinear(eph.sep(10, 350), 20, 'sep', 1e-9)

  checkEcliptic(eph.position('sun', jd), fixtures.sunTropical, 'sun tropical')
  const eq = eph.equatorial('sun', jd)
  nearAngle(eq.ra, fixtures.sunTropicalEquatorial.ra, 'sun ra')
  nearLinear(eq.dec, fixtures.sunTropicalEquatorial.dec, 'sun dec', arcsec / 3600)
  nearLinear(eq.raSpeed, fixtures.sunTropicalEquatorial.raSpeed, 'sun ra speed', speedTol)

  checkEcliptic(eph.position('sun', jd, sidereal), fixtures.sunSiderealLahiri, 'sun sidereal')
  const tropical = eph.position('sun', jd)
  const siderealSun = eph.position('sun', jd, sidereal)
  const ayanamsa = eph.ayanamsa('lahiri', jd, 'true')
  nearAngle(siderealSun.lon, tropical.lon - ayanamsa, 'sidereal identity')

  checkEcliptic(
    eph.position('mean-lilith', jd, { lilith: 'inclined' }),
    fixtures.lilithInclined,
    'lilith inclined'
  )
  const eclipticLilith = eph.position('mean-lilith', jd, { lilith: 'ecliptic' })
  checkEcliptic(eclipticLilith, fixtures.lilithEcliptic, 'lilith ecliptic')
  if (Math.abs(eclipticLilith.lat) * 3600 >= 0.01) {
    fail('ecliptic lilith latitude', `${eclipticLilith.lat} deg`)
  }

  const checkHouses = (actual: Houses, expected: Houses, label: string) => {
    if (actual.cusps.length !== 12) fail(label, `cusp count ${actual.cusps.length}`)
    for (let i = 0; i < 12; i += 1) nearAngle(actual.cusps[i], expected.cusps[i], `${label} cusp ${i}`)
    nearAngle(actual.asc, expected.asc, `${label} asc`)
    nearAngle(actual.mc, expected.mc, `${label} mc`)
    nearAngle(actual.armc, expected.armc, `${label} armc`)
    nearAngle(actual.vertex, expected.vertex, `${label} vertex`)
    if (actual.fallback !== expected.fallback) fail(`${label} fallback`, String(actual.fallback))
  }

  checkHouses(eph.houses(jd, place, 'placidus'), fixtures.placidus, 'placidus')
  const whole = eph.houses(jd, place, 'whole-sign', sidereal)
  checkHouses(whole, fixtures.wholeSignSidereal, 'whole sign')
  for (let i = 0; i < whole.cusps.length; i += 1) {
    const off = boundaryArcsec(whole.cusps[i])
    if (off > arcsec) fail(`whole sign cusp ${i} boundary`, `${off} arcsec`)
  }

  try {
    eph.position('earth' as Body, jd)
    fail('unknown body', 'did not throw')
  } catch (err) {
    if (!(err instanceof TypeError)) fail('unknown body', String(err))
  }

  return { ok: failures.length === 0, failures }
}
