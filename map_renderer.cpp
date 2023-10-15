#include "map_renderer.h"
#include <algorithm>

namespace renderer {

bool IsZeroTest(double value) {
    const double EPSILON = 1e-6;
            return std::abs(value) < EPSILON;
}

} // namespace end