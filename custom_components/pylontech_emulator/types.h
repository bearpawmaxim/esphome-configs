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
  };

}
}