import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function equatorialSun() {
  // doc:begin
  const eph = await Ephemeris.load()
  const sun = eph.equatorial('sun', { year: 2000, month: 1, day: 1, hour: 12 })
  // doc:end
  return sun
}
