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

      this->protocol_->set_get_analog_info_callback([this](PylonAnalogInfo *info) {
        info->pack_voltage = 11.859;
        info->current = 25.0;
        info->soc = 98;
        info->avg_nr_of_cycles = 2516;
        info->max_nr_of_cycles = 2932;
        info->avg_soh = 98;
        info->min_soh = 97;
        info->max_cell_voltage = 3.512;
        info->max_cell_voltage_pack_num = 3;
        info->max_cell_voltage_num = 4;
        info->min_cell_voltage = 3.259;
        info->min_cell_voltage_pack_num = 1;
        info->min_cell_voltage_num = 4;
        info->avg_cell_temp_k = 25.5 + 273.15;
        info->max_cell_temp_k = 26.8 + 273.15;
        info->max_cell_temp_pack_num = 3;
        info->max_cell_temp_num = 4;
        info->min_cell_temp_k = 24.2 + 273.15;
        info->min_cell_temp_pack_num = 1;
        info->min_cell_temp_num = 5;
        info->avg_mos_temp_k = 25.5 + 273.15;
        info->max_mos_temp_k = 26.9 + 273.15;
        info->max_mos_temp_pack_num = 3;
        info->max_mos_temp_num = 6;
        info->min_mos_temp_k = 24.1 + 273.15;
        info->min_mos_temp_pack_num = 1;
        info->min_mos_temp_num = 6;
        info->avg_bms_temp_k = 25.5 + 273.15;
        info->max_bms_temp_k = 26.7 + 273.15;
        info->max_bms_temp_pack_num = 3;
        info->max_bms_temp_num = 7;
        info->min_bms_temp_k = 24.3 + 273.15;
        info->min_bms_temp_pack_num = 1;
        info->min_bms_temp_num = 7;
      });
    }

    void loop() override {
      this->protocol_->loop();
    }

  private:
    PylontechLowVoltageProtocol *protocol_;
};
