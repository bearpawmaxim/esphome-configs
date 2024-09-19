#pragma once

#include "esphome.h"
#include "pylontech_rs485_low_voltage.h"

using namespace esphome;
using namespace pylontech;

class PylontechEmulatorComponent : public esphome::Component,
    public esphome::uart::UARTDevice,
    public pylontech::PylontechTransport {
  public:
    PylontechEmulatorComponent(esphome::uart::UARTComponent *parent): UARTDevice(parent) {}

    void set_get_analog_info_callback(GetAnalogInfoCallback callback) {
      this->protocol_->set_get_analog_info_callback(callback);
    }

    void setup() override {
      this->protocol_ = new PylontechLowVoltageProtocol(this, 0x02);
    }

    void write_uint8(uint8_t byte) override {
      this->write_byte(byte);
    }

    uint8_t read_uint8() override {
      uint8_t data;
      if (!this->available()) {
        return (uint8_t)0xFF;
      }
      if (!this->read_byte(&data)) {
        return (uint8_t)0xFF;
      }
      return data;
    }

    void loop() override {
      this->protocol_->loop();
    }

  private:
    PylontechLowVoltageProtocol *protocol_;
};
