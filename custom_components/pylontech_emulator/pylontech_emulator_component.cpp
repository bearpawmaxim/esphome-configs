#include "pylontech_emulator_component.h"

namespace esphome {
  namespace pylontech {
    using namespace pylontech_lv;
    using esphome::esp_log_printf_;

    void PylontechEmulatorComponent::register_callbacks_() {
      this->protocol_->set_get_analog_info_callback([this](PylonAnalogInfo *info) {
        this->handle_get_analog_info_(info);
      });
    }

    void PylontechEmulatorComponent::set_enabled(bool enabled) {
      if (enabled && !this->enabled_) {
        bool batteries_ready = true;
        for (auto battery : this->batteries_) {
          if (!battery.get_is_ready()) {
            batteries_ready = false;
            ESP_LOGW("PYL", "Battery %02X not ready", battery.address);
            break;
          }
        }
        this->enabled_ = batteries_ready;
        return;
      }
      this->enabled_ = false;
    }

    void PylontechEmulatorComponent::handle_get_analog_info_(PylonAnalogInfo *info) {
      auto values_manager = this->get_batt_values_manager_();

      info->pack_voltage = values_manager.get_pack_voltage();
      info->current = values_manager.get_summary_current();
      info->soc = values_manager.get_pack_soc();

      auto avg_max_nr_of_cycles = values_manager.get_avg_max_nr_of_cycles();
      info->avg_nr_of_cycles = std::get<0>(avg_max_nr_of_cycles);
      info->max_nr_of_cycles = std::get<1>(avg_max_nr_of_cycles);

      auto avg_min_soh = values_manager.get_avg_min_soh();
      info->avg_soh = std::get<0>(avg_min_soh);
      info->min_soh = std::get<1>(avg_min_soh);

      auto max_cell_voltage = values_manager.get_max_cell_voltage();
      info->max_cell_voltage_pack_addr = std::get<0>(max_cell_voltage);
      info->max_cell_voltage = std::get<1>(max_cell_voltage);
      info->max_cell_voltage_num = std::get<2>(max_cell_voltage);

      auto min_cell_voltage = values_manager.get_min_cell_voltage();
      info->min_cell_voltage_pack_addr = std::get<0>(min_cell_voltage);
      info->min_cell_voltage = std::get<1>(min_cell_voltage);
      info->min_cell_voltage_num = std::get<2>(min_cell_voltage);

      info->avg_cell_temp_k = 273.15
        + values_manager.get_avg_cell_temperature();

      auto max_cell_temperature = values_manager.get_max_cell_temperature();
      info->max_cell_temp_pack_addr = std::get<0>(max_cell_temperature);
      info->max_cell_temp_k = 273.15 + std::get<1>(max_cell_temperature);
      info->max_cell_temp_num = std::get<2>(max_cell_temperature);

      auto min_cell_temperature = values_manager.get_min_cell_temperature();
      info->min_cell_temp_pack_addr = std::get<0>(min_cell_temperature);
      info->min_cell_temp_k = 273.15 + std::get<1>(min_cell_temperature);
      info->min_cell_temp_num = std::get<2>(min_cell_temperature);

      info->avg_mos_temp_k = values_manager.get_avg_mos_temperature() + 273.15;

      auto max_mos_temperature = values_manager.get_max_mos_temperature();
      info->max_mos_temp_pack_addr = std::get<0>(max_mos_temperature);
      info->max_mos_temp_k = 273.15 + std::get<1>(max_mos_temperature);
      info->max_mos_temp_num = 1;

      auto min_mos_temperature = values_manager.get_min_mos_temperature();
      info->min_mos_temp_pack_addr = std::get<0>(min_mos_temperature);
      info->min_mos_temp_k = 273.15 + std::get<1>(min_mos_temperature);
      info->min_mos_temp_num = 1;

      info->avg_bms_temp_k = values_manager.get_avg_bms_temperature() + 273.15;

      auto max_bms_temperature = values_manager.get_max_bms_temperature();
      info->max_bms_temp_pack_addr = std::get<0>(max_bms_temperature);
      info->max_bms_temp_k = 273.15 + std::get<1>(max_bms_temperature);
      info->max_bms_temp_num = 1;

      auto min_bms_temperature = values_manager.get_min_bms_temperature();
      info->min_bms_temp_pack_addr = std::get<0>(min_bms_temperature);
      info->min_bms_temp_k = 273.15 + std::get<1>(min_bms_temperature);
      info->min_bms_temp_num = 1;
    }

    void PylontechEmulatorComponent::add_on_connection_state_changed_callback(std::function<void(bool)> &&callback) {
      this->connection_state_changed_callback_ = std::move(callback);
    }

    void PylontechEmulatorComponent::setup() {
      this->protocol_ = new PylontechLowVoltageProtocol(this, this->address_);
      this->protocol_->set_log_callback([](PylonLogLevel level, const char *format, ...) {
        va_list args;
        va_start(args, format);
        switch (level) {
          case PylonLogLevel::ERROR:
            esp_log_printf_(ESPHOME_LOG_LEVEL_ERROR, "PYL", __LINE__, format, args);
            break;
          case PylonLogLevel::WARN:
            esp_log_printf_(ESPHOME_LOG_LEVEL_WARN, "PYL", __LINE__, format, args);
            break;
          case PylonLogLevel::INFO:
            esp_log_printf_(ESPHOME_LOG_LEVEL_INFO, "PYL", __LINE__, format, args);
            break;
          default:
            esp_log_printf_(ESPHOME_LOG_LEVEL_DEBUG, "PYL", __LINE__, format, args);
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
      if (!this->enabled_) {
        while (this->available() > 0) {
          this->read();
        }
        return;
      }
      this->protocol_->loop();
      bool is_online = this->protocol_->is_online();
      if (this->is_online_ != is_online) {
        this->is_online_ = is_online;
        if (this->connection_state_changed_callback_ != nullptr) {
          this->connection_state_changed_callback_(is_online);
        }
      }
    }
  }
}