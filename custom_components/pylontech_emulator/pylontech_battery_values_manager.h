#pragma once

#include <tuple>
#include <limits>
#include <vector>
#include "pylontech_types.h"

namespace esphome {
namespace pylontech {

  class PylontechBatteryValuesManager {
    public:
      PylontechBatteryValuesManager(std::vector<BatteryConfig> batteries) {
        this->batteries_ = batteries;
        this->battery_states_ = {};
        for (auto &battery : batteries) {
          this->load_battery_state(battery);
        }
      }

      float get_pack_voltage() {
        return this->battery_states_.front().voltage;
      }

      uint8_t get_pack_soc() {
        uint8_t max_value = std::numeric_limits<uint8_t>::lowest();

        for (const auto &battery : this->battery_states_) {
          uint8_t value = battery.soc;
          if (value > max_value) {
            max_value = value;
          }
        }
        return max_value;
      }

      float get_summary_current() {
        float current = 0;
        for (const auto &battery : this->battery_states_) {
          current += battery.current;
        }
        return current;
      }

      std::tuple<uint8_t, float, size_t> get_min_cell_voltage() {
        float min_value = std::numeric_limits<float>::max();
        size_t min_index = 0;
        uint8_t min_address = 0;

        for (const auto &battery : this->battery_states_) {
          for (size_t i = 0; i < battery.cell_voltages.size(); ++i) {
            float value = battery.cell_voltages[i];
            if (value < min_value) {
              min_value = value;
              min_index = i;
              min_address = battery.address;
            }
          }
        }
        return std::make_tuple(min_address, min_value, min_index + 1);
      }

      std::tuple<uint8_t, float, size_t> get_max_cell_voltage() {
        float max_value = std::numeric_limits<float>::lowest();
        size_t max_index = 0;
        uint8_t max_address = 0;

        for (const auto &battery : this->battery_states_) {
          for (size_t i = 0; i < battery.cell_voltages.size(); ++i) {
            float value = battery.cell_voltages[i];
            if (value > max_value) {
              max_value = value;
              max_index = i;
              max_address = battery.address;
            }
          }
        }
        return std::make_tuple(max_address, max_value, max_index + 1);
      }

      std::tuple<uint8_t, float, size_t> get_min_cell_temperature() {
        float min_value = std::numeric_limits<float>::max();
        size_t min_index = 0;
        uint8_t min_address = 0;

        for (const auto &battery : this->battery_states_) {
          for (size_t i = 0; i < battery.cell_temps.size(); ++i) {
            float value = battery.cell_temps[i];
            if (value < min_value) {
              min_value = value;
              min_index = i;
              min_address = battery.address;
            }
          }
        }
        return std::make_tuple(min_address, min_value, min_index + 1);
      }

      std::tuple<uint8_t, float, size_t> get_max_cell_temperature() {
        float max_value = 0;
        size_t max_index = 0;
        uint8_t max_address = 0;

        for (const auto &battery : this->battery_states_) {
          for (size_t i = 0; i < battery.cell_temps.size(); ++i) {
            float value = battery.cell_temps[i];
            if (value > max_value) {
              max_value = value;
              max_index = i;
              max_address = battery.address;
            }
          }
        }
        return std::make_tuple(max_address, max_value, max_index + 1);
      }

      std::tuple<uint8_t, uint8_t> get_avg_min_soh() {
        uint8_t min_value = std::numeric_limits<uint8_t>::max();
        uint64_t value_acc = 0;
        size_t count = 0;

        for (const auto &battery : this->battery_states_) {
          uint8_t value = battery.soh;
          value_acc += value;
          count ++;
          if (value < min_value) {
            min_value = value;
          }
        }
        return std::make_tuple((uint8_t)(value_acc / count), min_value);
      }

      std::tuple<uint16_t, uint16_t> get_avg_max_nr_of_cycles() {
        uint16_t max_value = 0;
        uint64_t value_acc = 0;
        size_t count = 0;

        for (const auto &battery : this->battery_states_) {
          uint16_t value = battery.number_of_cycles;
          value_acc += value;
          count ++;
          if (value > max_value) {
            max_value = value;
          }
        }
        return std::make_tuple((uint16_t)(value_acc / count), max_value);
      }

      float get_avg_cell_temperature() {
        float value_acc = 0;
        size_t count = 0;

        for (const auto &battery : this->battery_states_) {
          for (size_t i = 0; i < battery.cell_temps.size(); ++i) {
              value_acc += battery.cell_temps[i];
              count ++;
          }
        }
        return value_acc / count;
      }

      float get_avg_mos_temperature() {
        float value_acc = 0;
        size_t count = 0;

        for (const auto &battery : this->battery_states_) {
          value_acc += battery.mos_temp;
          count ++;
        }
        return value_acc / count;
      }

      std::tuple<uint8_t, float> get_max_mos_temperature() {
        float max_value = 0;
        uint8_t max_address = 0;

        for (const auto &battery : this->battery_states_) {
          float value = battery.mos_temp;
          if (value > max_value) {
            max_value = value;
            max_address = battery.address;
          }
        }
        return std::make_tuple(max_address, max_value);
      }

      std::tuple<uint8_t, float> get_min_mos_temperature() {
        float min_value = std::numeric_limits<float>::max();
        uint8_t min_address = 0;

        for (const auto &battery : this->battery_states_) {
          float value = battery.mos_temp;
          if (value < min_value) {
            min_value = value;
            min_address = battery.address;
          }
        }
        return std::make_tuple(min_address, min_value);
      }

      float get_avg_bms_temperature() {
        float value_acc = 0;
        size_t count = 0;

        for (const auto &battery : this->battery_states_) {
          value_acc += battery.bms_temp;
          count ++;
        }
        return value_acc / count;
      }

      std::tuple<uint8_t, float> get_max_bms_temperature() {
        float max_value = 0;
        uint8_t max_address = 0;

        for (const auto &battery : this->battery_states_) {
          float value = battery.bms_temp;
          if (value > max_value) {
            max_value = value;
            max_address = battery.address;
          }
        }
        return std::make_tuple(max_address, max_value);
      }

      std::tuple<uint8_t, float> get_min_bms_temperature() {
        float min_value = std::numeric_limits<float>::max();
        uint8_t min_address = 0;

        for (const auto &battery : this->battery_states_) {
          float value = battery.bms_temp;
          if (value < min_value) {
            min_value = value;
            min_address = battery.address;
          }
        }
        return std::make_tuple(min_address, min_value);
      }

    private:
      struct BatteryState {
        uint8_t address;
        float voltage;
        float current;
        uint8_t soc;
        uint16_t number_of_cycles;
        uint8_t soh;
        std::vector<float> cell_voltages;
        std::vector<float> cell_temps;
        float mos_temp;
        float bms_temp;
      };

      void load_battery_state(BatteryConfig battery) {
        auto battery_state = PylontechBatteryValuesManager::BatteryState{
          .address = battery.address,
          .voltage = battery.voltage_sensor->state,
          .current = battery.current_sensor->state,
          .soc = (uint8_t)battery.soc_sensor->state,
          .number_of_cycles = (uint16_t)battery.number_of_cycles_sensor->state,
          .soh = (uint8_t)battery.soh_sensor->state,
          .cell_voltages = {},
          .cell_temps = {},
          .mos_temp = battery.mos_temp_sensor->state,
          .bms_temp = battery.bms_temp_sensor->state
        };
        for (auto *sensor : battery.cell_voltage_sensors) {
          battery_state.cell_voltages.push_back(sensor->state);
        }
        for (auto *sensor : battery.cell_temp_sensors) {
          battery_state.cell_temps.push_back(sensor->state);
        }
        this->battery_states_.push_back(battery_state);
      }

      std::vector<BatteryState> battery_states_;
      std::vector<BatteryConfig> batteries_;
  };

}
}