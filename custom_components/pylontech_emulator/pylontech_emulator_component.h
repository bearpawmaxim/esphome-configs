#pragma once

#include "esphome/components/uart/uart.h"
#include "pylontech_rs485_low_voltage.h"
#include "pylontech_types.h"
#include "pylontech_battery_values_manager.h"

namespace esphome {
namespace pylontech {
  class PylontechEmulatorComponent : public Component, public uart::UARTDevice,
      public pylontech_lv::PylontechTransport {
    public:
      void set_address(uint8_t address) { this->address_ = address; }
      void add_battery(const BatteryConfig &battery) { this->batteries_.push_back(battery); }
      void add_on_connection_state_changed_callback(std::function<void(bool)> &&callback);
      void set_enabled(bool enabled);

      void setup() override;
      void write_uint8(uint8_t byte);
      uint8_t read_uint8();
      void loop() override;

      bool get_is_enabled() { return this->enabled_; }

    private:
      void handle_get_analog_info_(pylontech_lv::PylonAnalogInfo *info);
      void register_callbacks_();
      PylontechBatteryValuesManager get_batt_values_manager_() { return PylontechBatteryValuesManager(this->batteries_); };

      uint8_t address_;
      bool enabled_;
      bool is_online_;
      std::vector<BatteryConfig> batteries_ = {};
      std::function<void(bool)> connection_state_changed_callback_;
      pylontech_lv::PylontechLowVoltageProtocol *protocol_;
  };
}
}
