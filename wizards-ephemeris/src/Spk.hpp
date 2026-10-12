#pragma once

#include "eph/Math.hpp"

#include <string>
#include <vector>

namespace eph::detail {

// SPK type 2/3 reader. Positions are returned in the segment frame:
// equatorial == true means J2000 equatorial, otherwise ecliptic J2000.
class SpkBook {
public:
  void loadDirectory(const std::string& dir);
  bool heliocentric(int naifId, double jd, Vec3& out, bool& equatorial) const;
  bool empty() const { return segments_.empty(); }

private:
  struct Record {
    double mid = 0.0;
    double radius = 0.0;
    std::vector<double> coef; // component-major, low degree first
  };
  struct Segment {
    int center = 0;
    int target = 0;
    int frame = 0;
    int components = 3;
    int coeffCount = 0;
    double initSec = 0.0;
    double intlenSec = 0.0;
    int nRecords = 0;
    double startJd = 0.0;
    double endJd = 0.0;
    std::vector<Record> records;
  };

  bool state(int center, int target, double jd, Vec3& out, int& frame) const;
  std::vector<Segment> segments_;
};

} // namespace eph::detail
