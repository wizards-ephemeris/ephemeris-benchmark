import { Ephemeris } from 'wizards-ephemeris'

export async function citraGalactic() {
  // doc:begin
  const eph = await Ephemeris.load()
  const when = { year: 2000, month: 1, day: 1, hour: 12 }
  const lahiri = eph.ayanamsa('lahiri', when, 'true')
  const citra = eph.ayanamsa('citra', when, 'true')
  const galactic = eph.ayanamsa('galactic-center-mula', when, 'true')
  // doc:end
  return { lahiri, citra, galactic }
}
