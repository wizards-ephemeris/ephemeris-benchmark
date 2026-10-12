#pragma once

#include "eph/eph.hpp"

#include <stdexcept>
#include <string>

extern "C" {
#include "swephexp.h"
}

struct Swiss {
  Swiss() {
    swe_set_ephe_path(const_cast<char*>(EPH_SWISS_EPHE));
    double xx[6] {};
    char err[256] {};
    if (swe_calc_ut(2451545.0, SE_SUN, SEFLG_SWIEPH | SEFLG_SPEED, xx, err) < 0) {
      throw std::runtime_error(std::string("Swiss Ephemeris: ") + err);
    }
  }
  ~Swiss() { swe_close(); }
  Swiss(const Swiss&) = delete;
  Swiss& operator=(const Swiss&) = delete;

  void calc(int body, double jd, double& lon, double& lat, double& speed, int extra = 0) const {
    double xx[6] {};
    char err[256] {};
    // NOGDEFL: this library does not apply solar light deflection, so the
    // reference is asked for the same place.
    if (swe_calc_ut(jd, body, SEFLG_SWIEPH | SEFLG_SPEED | SEFLG_NOGDEFL | extra, xx, err) < 0) {
      throw std::runtime_error(err);
    }
    lon = xx[0];
    lat = xx[1];
    speed = xx[3];
  }

  void houses(double jd, double lat, double lonEast, int system, double cusp[13], double ascmc[10]) const {
    for (int i = 0; i < 13; ++i) cusp[i] = 0.0;
    for (int i = 0; i < 10; ++i) ascmc[i] = 0.0;
    // A negative return means the system is undefined there (polar Placidus).
    // The cusp arrays are still filled, usually with the Porphyry fallback.
    swe_houses(jd, lat, lonEast, system, cusp, ascmc);
  }
};

inline int sweBody(eph::Body body) {
  switch (body) {
  case eph::Body::Sun: return SE_SUN;
  case eph::Body::Moon: return SE_MOON;
  case eph::Body::Mercury: return SE_MERCURY;
  case eph::Body::Venus: return SE_VENUS;
  case eph::Body::Mars: return SE_MARS;
  case eph::Body::Jupiter: return SE_JUPITER;
  case eph::Body::Saturn: return SE_SATURN;
  case eph::Body::Uranus: return SE_URANUS;
  case eph::Body::Neptune: return SE_NEPTUNE;
  case eph::Body::Pluto: return SE_PLUTO;
  case eph::Body::TrueNode: return SE_TRUE_NODE;
  case eph::Body::MeanLilith: return SE_MEAN_APOG;
  case eph::Body::Chiron: return SE_CHIRON;
  case eph::Body::Ceres: return SE_CERES;
  case eph::Body::Pallas: return SE_PALLAS;
  case eph::Body::Juno: return SE_JUNO;
  case eph::Body::Vesta: return SE_VESTA;
  }
  return SE_SUN;
}

inline int sweHouse(eph::HouseSystem sys) {
  switch (sys) {
  case eph::HouseSystem::Placidus: return 'P';
  case eph::HouseSystem::WholeSign: return 'W';
  case eph::HouseSystem::Koch: return 'K';
  case eph::HouseSystem::Equal: return 'E';
  case eph::HouseSystem::Regiomontanus: return 'R';
  case eph::HouseSystem::Campanus: return 'C';
  case eph::HouseSystem::Porphyrius: return 'O';
  case eph::HouseSystem::Alcabitius: return 'B';
  case eph::HouseSystem::Sripati: return 'S';
  case eph::HouseSystem::PolichPage: return 'T';
  }
  return 'P';
}

inline int sweAyanamsa(eph::Ayanamsa mode) {
  switch (mode) {
  case eph::Ayanamsa::Lahiri: return SE_SIDM_LAHIRI;
  case eph::Ayanamsa::Raman: return SE_SIDM_RAMAN;
  case eph::Ayanamsa::FaganBradley: return SE_SIDM_FAGAN_BRADLEY;
  case eph::Ayanamsa::Krishnamurti: return SE_SIDM_KRISHNAMURTI;
  case eph::Ayanamsa::Citra: return SE_SIDM_TRUE_CITRA;
  case eph::Ayanamsa::GalacticCenterMula: return SE_SIDM_GALCENT_MULA_WILHELM;
  }
  return SE_SIDM_LAHIRI;
}
