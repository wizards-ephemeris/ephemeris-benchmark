#pragma once

#include "Adapter.hpp"

#include <string>
#include <vector>

namespace emph {

// Swiss Ephemeris is the reference column. Every other registered library is a subject.
Adapter& reference();
const std::vector<Adapter*>& subjects();
const std::vector<Adapter*>& libraries();

} // namespace emph
