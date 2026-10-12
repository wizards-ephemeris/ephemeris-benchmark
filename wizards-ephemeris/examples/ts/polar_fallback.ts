import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function polarFallback() {
  // doc:begin
  const eph = await Ephemeris.load()
  const when = { year: 2000, month: 1, day: 1, hour: 12 }
  const place = { lat: 80, lon: 18.67 }
  const placidus = eph.houses(when, place, 'placidus')
  const koch = eph.houses(when, place, 'koch')
  // doc:end
  return { placidus, koch }
}
