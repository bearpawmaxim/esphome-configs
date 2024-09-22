#include "pylontech_emulator_component.h"

namespace esphome {
  namespace pylontech {
    using namespace pylontech_lv;

    void PylontechEmulatorComponent::register_callbacks_() {
      this->protocol_->set_get_analog_info_callback([this](PylonAnalogInfo *info) {
        this->handle_get_analog_info_(info);
      });
      this->protocol_->set_unknown_command_callback([this](uint8_t cmd) {
        this->unknown_commands_.insert(cmd);
        if (this->unknown_commands_sensor_ != nullptr) {
          std::string res = "";
          char buf[5];
          for (const uint8_t cmd : this->unknown_commands_) {
            sprintf(buf, "%02X, ", cmd);
            res += buf;
          }
          this->unknown_commands_sensor_->publish_state(res);
        }
      });
    }

    void PylontechEmulatorComponent::handle_get_analog_info_(PylonAnalogInfo *info) {
      BatteryConfig first_battery = this->batteries_.front();
      auto values_manager = this->get_batt_values_manager();

      info->pack_voltage = values_manager.get_pack_voltage();
      info->current = values_manager.get_summary_current();
      info->soc = values_manager.get_pack_soc();

      auto [avg_nr_of_cycles, max_nr_of_cycles]
        = values_manager.get_avg_max_nr_of_cycles();
      info->avg_nr_of_cycles = avg_nr_of_cycles;
      info->max_nr_of_cycles = max_nr_of_cycles;

      auto [avg_soh, min_soh] = values_manager.get_avg_min_soh();
      info->avg_soh = avg_soh;
      info->min_soh = min_soh;

      auto [max_cell_v_address, max_cell_v, max_cell_v_idx]
        = values_manager.get_max_cell_voltage();
      info->max_cell_voltage = max_cell_v;
      info->max_cell_voltage_pack_num = max_cell_v_address;
      info->max_cell_voltage_num = max_cell_v_idx;
      auto [min_cell_v_address, min_cell_v, min_cell_v_idx]
        = values_manager.get_min_cell_voltage();
      info->min_cell_voltage = min_cell_v;
      info->min_cell_voltage_pack_num = min_cell_v_address;
      info->min_cell_voltage_num = min_cell_v_idx;
      
      info->avg_cell_temp_k = 273.15
        + values_manager.get_average_cell_temperature();
      auto [max_cell_t_address, max_cell_t, max_cell_t_idx]
        = values_manager.get_min_cell_temperature();
      info->max_cell_temp_k = 273.15 + max_cell_t;
      info->max_cell_temp_pack_num = max_cell_t_address;
      info->max_cell_temp_num = max_cell_t_idx;
      auto [min_cell_t_address, min_cell_t, min_cell_t_idx]
        = values_manager.get_min_cell_temperature();
      info->min_cell_temp_k = 273.15 + min_cell_t;
      info->min_cell_temp_pack_num = min_cell_t_address;
      info->min_cell_temp_num = min_cell_t_idx;

      info->avg_mos_temp_k = values_manager.get_avg_mos_temperature() + 273.15;
      auto [max_mos_t_address, max_mos_t]
        = values_manager.get_max_mos_temperature();
      info->max_mos_temp_k = max_mos_t + 273.15;
      info->max_mos_temp_pack_num = max_mos_t_address;
      info->max_mos_temp_num = 1;
      auto [min_mos_t_address, min_mos_t]
        = values_manager.get_max_mos_temperature();
      info->min_mos_temp_k = min_mos_t + 273.15;
      info->min_mos_temp_pack_num = min_mos_t_address;
      info->min_mos_temp_num = 1;

      info->avg_bms_temp_k = values_manager.get_avg_bms_temperature() + 273.15;
      auto [max_bms_t_address, max_bms_t]
        = values_manager.get_max_bms_temperature();
      info->max_bms_temp_k = max_bms_t + 273.15;
      info->max_bms_temp_pack_num = max_bms_t_address;
      info->max_bms_temp_num = 1;
      auto [min_bms_t_address, min_bms_t]
        = values_manager.get_max_bms_temperature();
      info->min_bms_temp_k = min_bms_t + 273.15;
      info->min_bms_temp_pack_num = min_bms_t_address;
      info->min_bms_temp_num = 1;
    }

    void PylontechEmulatorComponent::setup() {
      this->protocol_ = new PylontechLowVoltageProtocol(this, this->address_);
      this->protocol_->set_log_callback([](PylonLogLevel level, const char *format, ...) {
        va_list args;
        va_start(args, format);
        switch (level) {
          case PylonLogLevel::ERROR:
            esphome::ESP_LOGE("PYL", format, args);
            break;
          case PylonLogLevel::WARN:
            esphome::ESP_LOGW("PYL", format, args);
            break;
          case PylonLogLevel::INFO:
            //esphome::ESP_LOGI("PYL", format, args);
            break;
          default:
            //esphome::ESP_LOGD("PYL", format, args);
            break;
        }
        va_end(args);
      });
      this->register_callbacks_();
    }

    void PylontechEmulatorComponent::write_uint8(uint8_t byte) {
      this->write_byte(byte);
    }

    uint8_t PylontechEmulatorComponent::read_uint8() {
      uint8_t data;
      if (!this->available()) {
        return (uint8_t)0xFF;
      }
      if (!this->read_byte(&data)) {
        return (uint8_t)0xFF;
      }
      return data;
    }

    void PylontechEmulatorComponent::loop() {
      this->protocol_->loop();
    }
  }
}