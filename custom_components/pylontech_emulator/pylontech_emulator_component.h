#include <set>
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "pylontech_rs485_low_voltage.h"
#include "pylontech_types.h"
#include "pylontech_battery_values_manager.h"

namespace esphome {
namespace pylontech {
  class PylontechEmulatorComponent : public Component, public uart::UARTDevice,
      public pylontech_lv::PylontechTransport {
    public:
      void set_address(uint8_t address) { this->address_ = address; }
      void set_unknown_commands_sensor(text_sensor::TextSensor *sensor) { this->unknown_commands_sensor_ = sensor; }
      void add_battery(const BatteryConfig &battery) { this->batteries_.push_back(battery); }

      void setup() override;
      void write_uint8(uint8_t byte);
      uint8_t read_uint8();
      void loop() override;

    private:
      void handle_get_analog_info_(pylontech_lv::PylonAnalogInfo *info);
      void register_callbacks_();
      PylontechBatteryValuesManager get_batt_values_manager() { return PylontechBatteryValuesManager(this->batteries_); };

      uint8_t address_;
      std::vector<BatteryConfig> batteries_ = {};
      pylontech_lv::PylontechLowVoltageProtocol *protocol_;
      std::set<uint8_t> unknown_commands_ = {};
      text_sensor::TextSensor *unknown_commands_sensor_{nullptr};
  };
}
}
