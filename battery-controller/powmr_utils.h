#pragma once

// #include <bitset>
// #include <string>
// #include <iostream>
#include "esphome.h"

uint16_t swap_bytes(uint16_t value) {
  return (value << 8) | (value >> 8);
}

uint16_t swap_bytes(std::string value) {
  uint16_t intValue = std::stoi(value);
  return (intValue << 8) | (intValue >> 8);
}

void update_select_value(uint16_t idx, esphome::modbus_controller::ModbusSelect* select_comp) {
  if (!select_comp->active_index().has_value() || idx != select_comp->active_index().value()) {
    auto call = select_comp->make_call();
    call.set_index(idx);
    call.perform();
  }
}
