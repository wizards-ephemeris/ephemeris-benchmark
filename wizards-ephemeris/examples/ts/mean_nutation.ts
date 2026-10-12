import { Ephemeris } from '@magowie/wizards-ephemeris'

export async function meanNutation() {
  // doc:begin
  const eph = await Ephemeris.load()
  const when = { year: 2000, month: 1, day: 1, hour: 12 }
  const asTrue = eph.position('sun', when, {
    zodiac: 'sidereal',
    ayanamsa: 'lahiri',
    nutation: 'true',
    wholeSign: 'shift-then-snap'
  })
  const asMean = eph.position('sun', when, {
    zodiac: 'sidereal',
    ayanamsa: 'lahiri',
    nutation: 'mean',
    wholeSign: 'shift-then-snap'
  })
  const trueAyanamsa = eph.ayanamsa('lahiri', when, 'true')
  const meanAyanamsa = eph.ayanamsa('lahiri', when, 'mean')
  // doc:end
  return {
    trueLon: asTrue.lon,
    meanLon: asMean.lon,
    trueAyanamsa,
    meanAyanamsa
  }
}
