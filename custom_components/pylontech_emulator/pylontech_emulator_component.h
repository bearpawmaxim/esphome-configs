#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "pylontech_rs485_low_voltage.h"

namespace esphome {
  namespace pylontech {

    class PylontechEmulatorComponent : public Component, public uart::UARTDevice,
        public pylontech_lv::PylontechTransport {
      public:
        void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
        void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
        void set_soc_sensor(sensor::Sensor *sensor) { this->soc_sensor_ = sensor; }

        void setup() override;
        void write_uint8(uint8_t byte);
        uint8_t read_uint8();
        void loop() override;

      private:
        void handle_get_analog_info_(pylontech_lv::PylonAnalogInfo *info);
        void register_callbacks_();

        sensor::Sensor *voltage_sensor_;
        sensor::Sensor *current_sensor_;
        sensor::Sensor *soc_sensor_;
        pylontech_lv::PylontechLowVoltageProtocol *protocol_;

    };
  }
}
