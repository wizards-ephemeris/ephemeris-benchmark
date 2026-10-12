import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function lahiriChart() {
  // doc:begin
  const eph = await Ephemeris.load()
  const when = { year: 2000, month: 1, day: 1, hour: 12 }
  const tropical = eph.position('sun', when)
  const ayanamsa = eph.ayanamsa('lahiri', when, 'true')
  const sidereal = eph.position('sun', when, {
    zodiac: 'sidereal',
    ayanamsa: 'lahiri',
    nutation: 'true',
    wholeSign: 'shift-then-snap'
  })
  // doc:end
  return { tropicalLon: tropical.lon, siderealLon: sidereal.lon, ayanamsa }
}
