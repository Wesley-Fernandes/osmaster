#pragma once

#include <cmath>

namespace utils {

constexpr double PI = 3.14159265358979323846;

inline double haversine(double lat1, double lon1, double lat2, double lon2) {
  constexpr double R = 6371000.0; // Earth radius in meters
  const double dLat = (lat2 - lat1) * PI / 180.0;
  const double dLon = (lon2 - lon1) * PI / 180.0;

  const double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
                   std::cos(lat1 * PI / 180.0) * std::cos(lat2 * PI / 180.0) *
                       std::sin(dLon / 2) * std::sin(dLon / 2);

  const double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
  return R * c;
}

} // namespace utils
