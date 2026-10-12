import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function utcDate() {
  // doc:begin
  const eph = await Ephemeris.load()
  const fromFields = eph.julianDay({ year: 2000, month: 1, day: 1, hour: 12 })
  const fromDate = eph.julianDay(new Date(Date.UTC(2000, 0, 1, 12, 0, 0)))
  // doc:end
  return { fromFields, fromDate }
}
