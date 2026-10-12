#include <doctest/doctest.h>

#include "Catalog.hpp"
#include "Compare.hpp"

#include <string>

namespace {

void resetTropical(emph::Adapter& subject, emph::Adapter& ref) {
  subject.setSidereal(false, eph::Ayanamsa::Lahiri);
  ref.setSidereal(false, eph::Ayanamsa::Lahiri);
  subject.setLilithMode(eph::LilithMode::Inclined);
  ref.setLilithMode(eph::LilithMode::Inclined);
}

double shiftDeg(eph::Ayanamsa mode, eph::SiderealNutation nutation, double jd) {
  const eph::JulianDay when{jd};
  if (nutation == eph::SiderealNutation::True) return eph::ayanamsaTrue(mode, when).deg();
  return eph::ayanamsa(mode, when).deg();
}

const char* ayanamsaName(eph::Ayanamsa mode) {
  switch (mode) {
  case eph::Ayanamsa::Lahiri: return "Lahiri";
  case eph::Ayanamsa::Raman: return "Raman";
  case eph::Ayanamsa::FaganBradley: return "FaganBradley";
  case eph::Ayanamsa::Krishnamurti: return "Krishnamurti";
  case eph::Ayanamsa::Citra: return "Citra";
  case eph::Ayanamsa::GalacticCenterMula: return "GalacticCenterMula";
  }
  return "?";
}

const char* houseName(eph::HouseSystem sys) {
  switch (sys) {
  case eph::HouseSystem::Placidus: return "Placidus";
  case eph::HouseSystem::WholeSign: return "WholeSign";
  case eph::HouseSystem::Koch: return "Koch";
  case eph::HouseSystem::Equal: return "Equal";
  case eph::HouseSystem::Regiomontanus: return "Regiomontanus";
  case eph::HouseSystem::Campanus: return "Campanus";
  case eph::HouseSystem::Porphyrius: return "Porphyrius";
  case eph::HouseSystem::Alcabitius: return "Alcabitius";
  case eph::HouseSystem::Sripati: return "Sripati";
  case eph::HouseSystem::PolichPage: return "PolichPage";
  }
  return "?";
}

} // namespace

TEST_CASE("ayanamsa versus reference") {
  emph::Adapter& ref = emph::reference();
  const std::vector<double> jds = monthlyJds();
  for (emph::Adapter* subject : emph::subjects()) {
    INFO("subject " << subject->name());
    Worst worst;
    for (eph::Ayanamsa mode : allAyanamsas()) {
      for (double jd : jds) {
        const double ours = subject->ayanamsaDeg(mode, jd);
        const double got = ref.ayanamsaDeg(mode, jd);
        worst.consider(arcsec(ours, got), jd, "ayanamsa");
      }
    }
    INFO("worst " << worst.sep << "\" jd " << worst.jd);
    CHECK(worst.sep < 1.0);
  }
}

TEST_CASE("sidereal longitudes versus reference") {
  emph::Adapter& ref = emph::reference();
  const std::vector<double> jds = quarterlyJds();
  for (emph::Adapter* subject : emph::subjects()) {
    INFO("subject " << subject->name());
    Worst place;
    Worst speed;
    Worst identity;
    Worst sunSpeed;
    Worst sunDerived;
    resetTropical(*subject, ref);
    for (double jd : jds) {
      resetTropical(*subject, ref);
      emph::SkyPlace tropical[17];
      int n = 0;
      for (eph::Body body : allBodies()) tropical[n++] = subject->position(body, jd);
      const eph::SiderealNutation nutations[] = {
        eph::SiderealNutation::True, eph::SiderealNutation::Mean
      };
      for (eph::Ayanamsa mode : allAyanamsas()) {
        for (eph::SiderealNutation nutation : nutations) {
        const eph::SiderealMethod method{mode, nutation, eph::WholeSignSnap::ShiftThenSnap};
        subject->setSidereal(true, method);
        ref.setSidereal(true, method);
        n = 0;
        for (eph::Body body : allBodies()) {
          const emph::SkyPlace pos = subject->position(body, jd);
          const emph::SkyPlace got = ref.position(body, jd);
          const std::string name = std::string(bodyName(body)) + " " + ayanamsaName(mode)
            + (nutation == eph::SiderealNutation::True ? " true" : " mean");
          const double placeLimit = siderealSwissArcsec(body, nutation);
          const double lonErr = arcsec(pos.lonDeg, got.lonDeg);
          place.consider(lonErr / placeLimit, jd, name);
          const double shifted = eph::wrap360(tropical[n].lonDeg - shiftDeg(mode, nutation, jd));
          identity.consider(arcsec(pos.lonDeg, shifted), jd, name);
          const double ayanamsaRate = shiftDeg(mode, nutation, jd + 0.5) - shiftDeg(mode, nutation, jd - 0.5);
          const double derived = tropical[n].speedDegPerDay - ayanamsaRate;
          if (body == eph::Body::Sun) {
            sunDerived.consider(speedArcsec(pos.speedDegPerDay, derived), jd, "sun derived");
          }
          const double rate = speedArcsec(pos.speedDegPerDay, got.speedDegPerDay);
          if (body == eph::Body::Sun) {
            sunSpeed.consider(rate, jd, "sun speed");
          } else {
            speed.consider(rate / speedBoundArcsec(body), jd, name);
          }
          ++n;
        }
        }
      }
    }
    INFO("place ratio " << place.sep << " " << place.note << " jd " << place.jd
      << " identity " << identity.sep << "\" " << identity.note << " jd " << identity.jd
      << " speed ratio " << speed.sep << " " << speed.note << " jd " << speed.jd
      << " sun speed " << sunSpeed.sep << "\"/day jd " << sunSpeed.jd
      << " sun derived " << sunDerived.sep << "\"/day jd " << sunDerived.jd);
    CHECK(place.sep < 1.0);
    CHECK(identity.sep < 0.05);
    CHECK(speed.sep < 1.0);
    // Swiss differentiates its analytic sidereal longitude. This library uses a
    // forward difference, which sits about 0.22"/day off that rate for the Sun.
    CHECK(sunSpeed.sep < 0.5);
    CHECK(sunDerived.sep < 0.05);
    resetTropical(*subject, ref);
  }
}

TEST_CASE("sidereal houses versus reference") {
  emph::Adapter& ref = emph::reference();
  const std::vector<double> dates = yearlyHouseJds();
  const double lats[] = {-80.0, -60.0, 0.0, 50.0, 80.0};
  const double lons[] = {-120.0, 18.67, 140.0};
  for (emph::Adapter* subject : emph::subjects()) {
    INFO("subject " << subject->name());
    Worst anglesTrue;
    Worst anglesMean;
    Worst armc;
    Worst cuspsTrue;
    Worst cuspsMean;
    Worst wholeSwiss;
    Worst wholeIdentity;
    Worst armcShift;
    resetTropical(*subject, ref);
    for (double date : dates) {
      for (double lat : lats) {
        for (double lon : lons) {
          for (eph::HouseSystem sys : allHouseSystems()) {
            resetTropical(*subject, ref);
            const emph::HousePlace tropical = subject->houses(date, lat, lon, sys);
            for (eph::Ayanamsa mode : allAyanamsas()) {
              const std::string where = std::string(houseName(sys)) + " " + ayanamsaName(mode);
              const eph::SiderealMethod truth{
                mode, eph::SiderealNutation::True, eph::WholeSignSnap::ShiftThenSnap};
              subject->setSidereal(true, truth);
              ref.setSidereal(true, truth);
              const emph::HousePlace got = subject->houses(date, lat, lon, sys);
              const emph::HousePlace expect = ref.houses(date, lat, lon, sys);
              anglesTrue.consider(arcsec(got.ascDeg, expect.ascDeg), date, where + " asc");
              anglesTrue.consider(arcsec(got.mcDeg, expect.mcDeg), date, where + " mc");
              anglesTrue.consider(arcsec(got.vertexDeg, expect.vertexDeg), date, where + " vertex");
              armc.consider(arcsec(got.armcDeg, expect.armcDeg), date, where + " armc");
              armcShift.consider(arcsec(got.armcDeg, tropical.armcDeg), date, where);
              for (int i = 0; i < 12; ++i) {
                if (sys == eph::HouseSystem::WholeSign) {
                  wholeSwiss.consider(arcsec(got.cuspDeg[i], expect.cuspDeg[i]), date, where);
                } else {
                  cuspsTrue.consider(arcsec(got.cuspDeg[i], expect.cuspDeg[i]), date, where);
                }
              }

              // Mean ayanamsa, snap the tropical sign first. Swiss still uses
              // the true ayanamsa and snaps after the shift.
              const eph::SiderealMethod mean{
                mode, eph::SiderealNutation::Mean, eph::WholeSignSnap::SnapThenShift};
              subject->setSidereal(true, mean);
              ref.setSidereal(true, truth);
              const emph::HousePlace meanGot = subject->houses(date, lat, lon, sys);
              const emph::HousePlace swiss = ref.houses(date, lat, lon, sys);
              const double meanShift = eph::ayanamsa(mode, eph::JulianDay{date}).deg();
              anglesMean.consider(arcsec(meanGot.ascDeg, swiss.ascDeg), date, where + " mean asc");
              anglesMean.consider(arcsec(meanGot.mcDeg, swiss.mcDeg), date, where + " mean mc");
              anglesMean.consider(arcsec(meanGot.vertexDeg, swiss.vertexDeg), date, where + " mean vertex");
              for (int i = 0; i < 12; ++i) {
                if (sys == eph::HouseSystem::WholeSign) {
                  const double shifted = eph::wrap360(tropical.cuspDeg[i] - meanShift);
                  wholeIdentity.consider(arcsec(meanGot.cuspDeg[i], shifted), date, where);
                } else {
                  cuspsMean.consider(arcsec(meanGot.cuspDeg[i], swiss.cuspDeg[i]), date, where);
                }
              }
            }
          }
        }
      }
    }
    INFO("true angles " << anglesTrue.sep << "\" " << anglesTrue.note << " jd " << anglesTrue.jd
      << " true cusps " << cuspsTrue.sep << "\" " << cuspsTrue.note << " jd " << cuspsTrue.jd
      << " whole swiss " << wholeSwiss.sep << "\" " << wholeSwiss.note << " jd " << wholeSwiss.jd
      << " mean angles " << anglesMean.sep << "\" " << anglesMean.note << " jd " << anglesMean.jd
      << " mean cusps " << cuspsMean.sep << "\" " << cuspsMean.note << " jd " << cuspsMean.jd
      << " whole identity " << wholeIdentity.sep << "\" jd " << wholeIdentity.jd
      << " armc " << armc.sep << "\" " << armc.note << " jd " << armc.jd
      << " armc shift " << armcShift.sep << "\" jd " << armcShift.jd);
    // True ayanamsa and shift-then-snap match Swiss. Mean ayanamsa leaves
    // nutation in the angle, about ±18". Snap-then-shift whole sign is the
    // tropical sign moved by the mean ayanamsa, not Swiss's sidereal snap.
    CHECK(anglesTrue.sep < 5.0);
    CHECK(cuspsTrue.sep < 10.0);
    CHECK(wholeSwiss.sep < 1.0);
    CHECK(anglesMean.sep < 20.0);
    CHECK(cuspsMean.sep < 20.0);
    CHECK(wholeIdentity.sep < 0.05);
    CHECK(armc.sep < 5.0);
    CHECK(armcShift.sep < 0.05);
    resetTropical(*subject, ref);
  }
}
