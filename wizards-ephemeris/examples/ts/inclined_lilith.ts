import { Ephemeris } from 'wizards-ephemeris'

export async function inclinedLilith() {
  // doc:begin
  const eph = await Ephemeris.load()
  const lilith = eph.position('mean-lilith', { year: 2000, month: 1, day: 1, hour: 12 }, {
    lilith: 'inclined'
  })
  // doc:end
  return lilith
}
