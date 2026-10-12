import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function deltaT() {
  // doc:begin
  const eph = await Ephemeris.load()
  const seconds = eph.deltaT({ year: 2000, month: 1, day: 1, hour: 12 })
  // doc:end
  return seconds
}
