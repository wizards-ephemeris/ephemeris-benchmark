import { Ephemeris } from 'wizards-ephemeris'

export async function armcFixed() {
  // doc:begin
  const eph = await Ephemeris.load()
  const when = { year: 2000, month: 1, day: 1, hour: 12 }
  const place = { lat: 50.22, lon: 18.67 }
  const tropical = eph.houses(when, place, 'placidus')
  const sidereal = eph.houses(when, place, 'placidus', {
    zodiac: 'sidereal',
    ayanamsa: 'lahiri',
    nutation: 'true',
    wholeSign: 'shift-then-snap'
  })
  // doc:end
  return { tropical, sidereal }
}
