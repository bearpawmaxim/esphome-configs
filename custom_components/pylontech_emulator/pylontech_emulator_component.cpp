#include "pylontech_emulator_component.h"

namespace esphome {
namespace pylontech_lv {

void PylontechEmulatorComponent::register_callbacks() {
  this->protocol_->set_get_analog_info_callback(&this->handle_get_analog_info);
}

void PylontechEmulatorComponent::handle_get_analog_info_(PylonAnalogInfo *info) {
  info->pack_voltage = this->voltage_sensor_->state;
  info->current = this->current_sensor_->state;
  info->soc = this->soc_sensor_->state;
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
}

void PylontechEmulatorComponent::setup() {
  this->protocol_ = new PylontechLowVoltageProtocol(this, 0x02);
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
