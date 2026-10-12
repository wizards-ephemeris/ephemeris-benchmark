import { Ephemeris } from 'wizards-ephemeris'

export async function eclipticLilith() {
  // doc:begin
  const eph = await Ephemeris.load()
  const when = { year: 2000, month: 1, day: 1, hour: 12 }
  const inclined = eph.position('mean-lilith', when, { lilith: 'inclined' })
  const ecliptic = eph.position('mean-lilith', when, { lilith: 'ecliptic' })
  // doc:end
  return { inclined, ecliptic }
}
