import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function julianDay() {
  // doc:begin
  const eph = await Ephemeris.load()
  const jd = eph.julianDay({ year: 2000, month: 1, day: 1, hour: 12 })
  const back = eph.revJul(jd)
  // doc:end
  return { jd, back }
}
