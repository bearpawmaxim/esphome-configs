#pragma once

#include "esphome.h"
#include "pylontech_rs485_low_voltage.h"

using namespace esphome;
using namespace pylontech;

class PylontechEmulatorComponent : public esphome::Component, public esphome::uart::UARTDevice {
  public:
    PylontechEmulatorComponent(esphome::uart::UARTComponent *parent): UARTDevice(parent) {}

    void setup() override {
      this->protocol_ = new PylontechLowVoltageProtocol();
      this->protocol_->set_read_byte_callback([this]() {
        uint8_t data;
        if (!this->available()) {
          return (uint8_t)0xFF;
        }
        if (!this->read_byte(&data)) {
          return (uint8_t)0xFF;
        }
        return data;
      });
      this->protocol_->set_write_byte_callback([this](uint8_t byte) {
        this->write_byte(byte);
      });
    }

    void loop() override {
      this->protocol_->loop();
    }

  private:
    PylontechLowVoltageProtocol *protocol_;
};
