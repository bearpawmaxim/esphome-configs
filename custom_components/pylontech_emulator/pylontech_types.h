#pragma once
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace pylontech {
  struct BatteryConfig {
    uint8_t address;
    sensor::Sensor *voltage_sensor;
    sensor::Sensor *current_sensor;
    sensor::Sensor *soc_sensor;
    sensor::Sensor *number_of_cycles_sensor;
    sensor::Sensor *soh_sensor;
    std::vector<sensor::Sensor*> cell_voltage_sensors;
    std::vector<sensor::Sensor*> cell_temp_sensors;
    sensor::Sensor *mos_temp_sensor;
    sensor::Sensor *bms_temp_sensor;

    bool get_is_ready() {
      bool sensors_has_state = voltage_sensor->has_state() &&
        current_sensor->has_state() &&
        soc_sensor->has_state() &&
        number_of_cycles_sensor->has_state() &&
        soh_sensor->has_state() &&
        mos_temp_sensor->has_state() &&
        bms_temp_sensor->has_state();

      for (auto *sensor : cell_voltage_sensors) {
        if (!sensor->has_state()) {
          sensors_has_state = false;
          break;
        }
      }

      for (auto *sensor : cell_temp_sensors) {
        if (!sensor->has_state()) {
          sensors_has_state = false;
          break;
        }
      }

      return sensors_has_state;
    }
  };

}
}