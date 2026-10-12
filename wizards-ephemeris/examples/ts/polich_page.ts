import { Ephemeris } from 'wizards-ephemeris'

export async function polichPage() {
  // doc:begin
  const eph = await Ephemeris.load()
  const houses = eph.houses(
    { year: 2000, month: 1, day: 1, hour: 12 },
    { lat: 50.22, lon: 18.67 },
    'polich-page'
  )
  // doc:end
  return houses
}
