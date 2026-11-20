// model.cpp  -  implementations for model.hpp
#include "careroute/model.hpp"

#include <cmath>

namespace careroute {

bool Carer::has_skill(int s) const {
    for (int k : skills)
        if (k == s) return true;
    return false;
}

double Instance::travel_min(double x1, double y1, double x2, double y2) const {
    const double dx = x1 - x2;
    const double dy = y1 - y2;
    const double dist_km = std::sqrt(dx * dx + dy * dy);
    // minutes = km / (km per hour) * 60
    return dist_km / speed_kmh * 60.0;
}

double Instance::travel_min(const Carer& c, const Visit& v) const {
    return travel_min(c.x, c.y, v.x, v.y);
}

double Instance::travel_min(const Visit& a, const Visit& b) const {
    return travel_min(a.x, a.y, b.x, b.y);
}

}  // namespace careroute
