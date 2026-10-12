import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function snapThenShift() {
  // doc:begin
  const eph = await Ephemeris.load()
  const houses = eph.houses(
    { year: 2000, month: 1, day: 1, hour: 12 },
    { lat: 50.22, lon: 18.67 },
    'whole-sign',
    {
      zodiac: 'sidereal',
      ayanamsa: 'lahiri',
      nutation: 'true',
      wholeSign: 'snap-then-shift'
    }
  )
  // doc:end
  return houses
}
