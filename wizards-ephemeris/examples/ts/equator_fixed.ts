import { Ephemeris } from 'wizards-ephemeris'

export async function equatorFixed() {
  // doc:begin
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
  // doc:end
  return { tropical, tropicalSky, sidereal, siderealSky }
}
