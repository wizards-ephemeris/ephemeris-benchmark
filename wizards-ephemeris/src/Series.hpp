#pragma once

#include <cstdint>

namespace eph::detail {

double vsopComponent(int planet, int var, double millennia);
void iauNutation2000b(double tCenturies, double& dpsi, double& deps);
bool minorChebyshev(int body, double jd, double& x, double& y, double& z);

struct ElpMain {
  int16_t i[4];
  double c[7];
};

struct ElpFig {
  int16_t iz;
  int16_t i[4];
  double pha;
  double amp;
};

struct ElpPla {
  int16_t ip[11];
  double pha;
  double amp;
};

extern const ElpMain kMainLon[];
extern const int kMainLonN;
extern const ElpMain kMainLat[];
extern const int kMainLatN;
extern const ElpMain kMainDist[];
extern const int kMainDistN;

extern const ElpFig kFigLon0[];
extern const int kFigLon0N;
extern const ElpFig kFigLon1[];
extern const int kFigLon1N;
extern const ElpFig kFigLon2[];
extern const int kFigLon2N;
extern const ElpFig kFigLat0[];
extern const int kFigLat0N;
extern const ElpFig kFigLat1[];
extern const int kFigLat1N;
extern const ElpFig kFigLat2[];
extern const int kFigLat2N;
extern const ElpFig kFigDist0[];
extern const int kFigDist0N;
extern const ElpFig kFigDist1[];
extern const int kFigDist1N;
extern const ElpFig kFigDist2[];
extern const int kFigDist2N;

extern const ElpPla kPlaLon01[];
extern const int kPlaLon01N;
extern const ElpPla kPlaLon02[];
extern const int kPlaLon02N;
extern const ElpPla kPlaLon11[];
extern const int kPlaLon11N;
extern const ElpPla kPlaLon12[];
extern const int kPlaLon12N;
extern const ElpPla kPlaLat01[];
extern const int kPlaLat01N;
extern const ElpPla kPlaLat02[];
extern const int kPlaLat02N;
extern const ElpPla kPlaLat11[];
extern const int kPlaLat11N;
extern const ElpPla kPlaLat12[];
extern const int kPlaLat12N;
extern const ElpPla kPlaDist01[];
extern const int kPlaDist01N;
extern const ElpPla kPlaDist02[];
extern const int kPlaDist02N;
extern const ElpPla kPlaDist11[];
extern const int kPlaDist11N;
extern const ElpPla kPlaDist12[];
extern const int kPlaDist12N;

} // namespace eph::detail
