#pragma once

#include <string>
#include <cstdio>

inline std::string value_accuracy_to_string(float value, int accuracy) {
  if (std::isnan(value)) {
    return "—";
  }

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%.*f", accuracy, value);
  return std::string(buffer);
}