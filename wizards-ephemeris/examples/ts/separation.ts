import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function separation() {
  // doc:begin
  const eph = await Ephemeris.load()
  const sun = eph.position('sun', { year: 2000, month: 1, day: 1, hour: 12 })
  const quarter = eph.sep(10, 350)
  const same = eph.sep(sun.lon, sun.lon)
  // doc:end
  return { quarter, same }
}
