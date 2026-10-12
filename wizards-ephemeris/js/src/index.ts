import { Heap, loadGlue, type LoadOptions } from './abi.js'

export type { LoadOptions }

export type Body =
  | 'sun'
  | 'moon'
  | 'mercury'
  | 'venus'
  | 'mars'
  | 'jupiter'
  | 'saturn'
  | 'uranus'
  | 'neptune'
  | 'pluto'
  | 'true-node'
  | 'mean-lilith'
  | 'chiron'
  | 'ceres'
  | 'pallas'
  | 'juno'
  | 'vesta'

export type HouseSystem =
  | 'placidus'
  | 'whole-sign'
  | 'koch'
  | 'equal'
  | 'regiomontanus'
  | 'campanus'
  | 'porphyry'
  | 'alcabitius'
  | 'sripati'
  | 'polich-page'

export type Ayanamsa =
  | 'lahiri'
  | 'raman'
  | 'fagan-bradley'
  | 'krishnamurti'
  | 'citra'
  | 'galactic-center-mula'

export type Zodiac = 'tropical' | 'sidereal'
export type Nutation = 'mean' | 'true'
export type WholeSign = 'snap-then-shift' | 'shift-then-snap'
export type Lilith = 'ecliptic' | 'inclined'

export interface Method {
  zodiac?: Zodiac
  ayanamsa?: Ayanamsa
  nutation?: Nutation
  wholeSign?: WholeSign
  lilith?: Lilith
}

export interface Instant {
  year: number
  month: number
  day: number
  hour: number
}

// A Julian day, a Date read in UTC, or a calendar instant.
export type Time = number | Date | Instant

export interface EclipticPos {
  lon: number
  lat: number
  lonSpeed: number
  distAu: number
}

export interface EquatorialPos {
  ra: number
  dec: number
  raSpeed: number
}

export interface Place {
  lat: number
  lon: number
}

export interface Houses {
  cusps: number[]
  asc: number
  mc: number
  armc: number
  vertex: number
  fallback: boolean
}

const bodies: Record<Body, number> = {
  sun: 0,
  moon: 1,
  mercury: 2,
  venus: 3,
  mars: 4,
  jupiter: 5,
  saturn: 6,
  uranus: 7,
  neptune: 8,
  pluto: 9,
  'true-node': 10,
  'mean-lilith': 11,
  chiron: 12,
  ceres: 13,
  pallas: 14,
  juno: 15,
  vesta: 16
}

const houses: Record<HouseSystem, number> = {
  placidus: 0,
  'whole-sign': 1,
  koch: 2,
  equal: 3,
  regiomontanus: 4,
  campanus: 5,
  porphyry: 6,
  alcabitius: 7,
  sripati: 8,
  'polich-page': 9
}

const ayanamsas: Record<Ayanamsa, number> = {
  lahiri: 0,
  raman: 1,
  'fagan-bradley': 2,
  krishnamurti: 3,
  citra: 4,
  'galactic-center-mula': 5
}

const nutations: Record<Nutation, number> = { mean: 0, true: 1 }
const snaps: Record<WholeSign, number> = {
  'snap-then-shift': 0,
  'shift-then-snap': 1
}
const liliths: Record<Lilith, number> = { ecliptic: 0, inclined: 1 }
const zodiacs: Record<Zodiac, number> = { tropical: 0, sidereal: 1 }

function ordinal<T extends string>(table: Record<T, number>, name: string, label: string): number {
  if (Object.prototype.hasOwnProperty.call(table, name)) return table[name as T]
  const names = Object.keys(table).join(', ')
  throw new TypeError(`unknown ${label} "${name}". Expected one of: ${names}`)
}

interface Flags {
  zodiac: number
  ayanamsa: number
  nutation: number
  wholeSign: number
  lilith: number
}

function flagsOf(method: Method = {}): Flags {
  return {
    zodiac: ordinal(zodiacs, method.zodiac ?? 'tropical', 'zodiac'),
    ayanamsa: ordinal(ayanamsas, method.ayanamsa ?? 'lahiri', 'ayanamsa'),
    nutation: ordinal(nutations, method.nutation ?? 'true', 'nutation'),
    wholeSign: ordinal(snaps, method.wholeSign ?? 'shift-then-snap', 'whole sign order'),
    lilith: ordinal(liliths, method.lilith ?? 'inclined', 'lilith mode')
  }
}

function instantOf(time: Date | Instant): Instant {
  if (time instanceof Date) {
    if (Number.isNaN(time.getTime())) throw new TypeError('invalid Date')
    return {
      year: time.getUTCFullYear(),
      month: time.getUTCMonth() + 1,
      day: time.getUTCDate(),
      hour: time.getUTCHours()
        + time.getUTCMinutes() / 60
        + time.getUTCSeconds() / 3600
        + time.getUTCMilliseconds() / 3_600_000
    }
  }
  if (!Number.isFinite(time.year) || !Number.isFinite(time.month)
    || !Number.isFinite(time.day) || !Number.isFinite(time.hour)) {
    throw new TypeError('calendar fields must be finite numbers')
  }
  return time
}

export class Ephemeris {
  private constructor(private readonly heap: Heap) {}

  static async load(options: LoadOptions = {}): Promise<Ephemeris> {
    const glue = await loadGlue(options)
    return new Ephemeris(Heap.create(glue))
  }

  julianDay(time: Time): number {
    if (typeof time === 'number') {
      if (!Number.isFinite(time)) throw new TypeError('Julian day must be finite')
      return time
    }
    const instant = instantOf(time)
    return this.heap.julianDay(instant.year, instant.month, instant.day, instant.hour)
  }

  revJul(time: Time): Instant {
    const [year, month, day, hour] = this.heap.revJul(this.julianDay(time))
    return { year, month, day, hour }
  }

  deltaT(time: Time): number {
    return this.heap.deltaT(this.julianDay(time))
  }

  ayanamsa(mode: Ayanamsa, time: Time, nutation: Nutation = 'true'): number {
    return this.heap.ayanamsa(
      ordinal(ayanamsas, mode, 'ayanamsa'),
      this.julianDay(time),
      ordinal(nutations, nutation, 'nutation')
    )
  }

  position(body: Body, time: Time, method?: Method): EclipticPos {
    const flags = flagsOf(method)
    const [lon, lat, lonSpeed, distAu] = this.heap.position(
      ordinal(bodies, body, 'body'),
      this.julianDay(time),
      flags.zodiac,
      flags.ayanamsa,
      flags.nutation,
      flags.wholeSign,
      flags.lilith
    )
    return { lon, lat, lonSpeed, distAu }
  }

  equatorial(body: Body, time: Time, method?: Method): EquatorialPos {
    const flags = flagsOf(method)
    const [ra, dec, raSpeed] = this.heap.equatorial(
      ordinal(bodies, body, 'body'),
      this.julianDay(time),
      flags.zodiac,
      flags.ayanamsa,
      flags.nutation,
      flags.wholeSign,
      flags.lilith
    )
    return { ra, dec, raSpeed }
  }

  houses(time: Time, place: Place, system: HouseSystem, method?: Method): Houses {
    if (!Number.isFinite(place.lat) || !Number.isFinite(place.lon)) {
      throw new TypeError('latitude and longitude must be finite numbers')
    }
    const flags = flagsOf(method)
    const raw = this.heap.houses(
      this.julianDay(time),
      place.lat,
      place.lon,
      ordinal(houses, system, 'house system'),
      flags.zodiac,
      flags.ayanamsa,
      flags.nutation,
      flags.wholeSign,
      flags.lilith
    )
    return {
      cusps: raw.slice(0, 12),
      asc: raw[12],
      mc: raw[13],
      armc: raw[14],
      vertex: raw[15],
      fallback: raw[16] !== 0
    }
  }

  sep(aDeg: number, bDeg: number): number {
    if (!Number.isFinite(aDeg) || !Number.isFinite(bDeg)) {
      throw new TypeError('separation arguments must be finite numbers')
    }
    return this.heap.sep(aDeg, bDeg)
  }
}
