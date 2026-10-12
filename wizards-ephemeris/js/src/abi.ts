// Heap glue over the Emscripten ES module. Output records are packed doubles.

export interface Glue {
  _malloc(bytes: number): number
  _free(ptr: number): void
  _eph_create(): number
  _eph_destroy(ptr: number): void
  _eph_position(
    chart: number,
    body: number,
    jdUt: number,
    zodiac: number,
    ayanamsa: number,
    nutation: number,
    wholeSign: number,
    lilith: number,
    out: number
  ): void
  _eph_equatorial(
    chart: number,
    body: number,
    jdUt: number,
    zodiac: number,
    ayanamsa: number,
    nutation: number,
    wholeSign: number,
    lilith: number,
    out: number
  ): void
  _eph_houses(
    chart: number,
    jdUt: number,
    latDeg: number,
    lonDegEast: number,
    system: number,
    zodiac: number,
    ayanamsa: number,
    nutation: number,
    wholeSign: number,
    lilith: number,
    out: number
  ): void
  _eph_julian_day(year: number, month: number, day: number, hour: number): number
  _eph_rev_jul(jdUt: number, out: number): void
  _eph_delta_t(jdUt: number): number
  _eph_ayanamsa(mode: number, jdUt: number, trueNutation: number): number
  _eph_sep_deg(aDeg: number, bDeg: number): number
  HEAPF64: Float64Array
  wasmMemory: WebAssembly.Memory
}

export interface LoadOptions {
  wasmUrl?: string
}

type Factory = (options?: { locateFile?: (path: string) => string }) => Promise<Glue>

function glueHref(): string {
  const path = new URL(import.meta.url).pathname
  const rel = path.includes('/src/') ? '../dist/eph.js' : './eph.js'
  return new URL(rel, import.meta.url).href
}

export async function loadGlue(options: LoadOptions = {}): Promise<Glue> {
  const imported = await import(/* @vite-ignore */ glueHref())
  const create = imported.default as Factory
  const wasmUrl = options.wasmUrl
  return create(wasmUrl
    ? { locateFile: (path: string) => path.endsWith('.wasm') ? wasmUrl : path }
    : {})
}

const doubles = 28
const eclipticAt = 0
const equatorialAt = 4
const housesAt = 7
const calendarAt = 24

export class Heap {
  readonly chart: number
  private readonly scratch: number

  private constructor(private readonly glue: Glue, scratch: number, chart: number) {
    this.scratch = scratch
    this.chart = chart
  }

  static create(glue: Glue): Heap {
    const scratch = glue._malloc(doubles * 8)
    if (scratch === 0 || (scratch & 7) !== 0) {
      throw new Error('wasm heap did not give an aligned scratch buffer')
    }
    return new Heap(glue, scratch, glue._eph_create())
  }

  private view(): Float64Array {
    const buffer = this.glue.wasmMemory.buffer
    if (this.glue.HEAPF64.buffer === buffer) return this.glue.HEAPF64
    return new Float64Array(buffer)
  }

  private read(index: number, count: number): number[] {
    const start = (this.scratch >> 3) + index
    return Array.from(this.view().subarray(start, start + count))
  }

  position(
    body: number,
    jdUt: number,
    zodiac: number,
    ayanamsa: number,
    nutation: number,
    wholeSign: number,
    lilith: number
  ): number[] {
    this.glue._eph_position(
      this.chart, body, jdUt, zodiac, ayanamsa, nutation, wholeSign, lilith,
      this.scratch + eclipticAt * 8
    )
    return this.read(eclipticAt, 4)
  }

  equatorial(
    body: number,
    jdUt: number,
    zodiac: number,
    ayanamsa: number,
    nutation: number,
    wholeSign: number,
    lilith: number
  ): number[] {
    this.glue._eph_equatorial(
      this.chart, body, jdUt, zodiac, ayanamsa, nutation, wholeSign, lilith,
      this.scratch + equatorialAt * 8
    )
    return this.read(equatorialAt, 3)
  }

  houses(
    jdUt: number,
    latDeg: number,
    lonDegEast: number,
    system: number,
    zodiac: number,
    ayanamsa: number,
    nutation: number,
    wholeSign: number,
    lilith: number
  ): number[] {
    this.glue._eph_houses(
      this.chart, jdUt, latDeg, lonDegEast, system,
      zodiac, ayanamsa, nutation, wholeSign, lilith,
      this.scratch + housesAt * 8
    )
    return this.read(housesAt, 17)
  }

  julianDay(year: number, month: number, day: number, hour: number): number {
    return this.glue._eph_julian_day(year, month, day, hour)
  }

  revJul(jdUt: number): number[] {
    this.glue._eph_rev_jul(jdUt, this.scratch + calendarAt * 8)
    return this.read(calendarAt, 4)
  }

  deltaT(jdUt: number): number {
    return this.glue._eph_delta_t(jdUt)
  }

  ayanamsa(mode: number, jdUt: number, trueNutation: number): number {
    return this.glue._eph_ayanamsa(mode, jdUt, trueNutation)
  }

  sep(aDeg: number, bDeg: number): number {
    return this.glue._eph_sep_deg(aDeg, bDeg)
  }
}
