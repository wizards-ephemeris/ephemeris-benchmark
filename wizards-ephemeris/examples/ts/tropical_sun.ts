import { Ephemeris } from 'wizards-ephemeris'

export async function tropicalSun() {
  // doc:begin
  const eph = await Ephemeris.load()
  const sun = eph.position('sun', { year: 2000, month: 1, day: 1, hour: 12 })
  // doc:end
  return sun
}
