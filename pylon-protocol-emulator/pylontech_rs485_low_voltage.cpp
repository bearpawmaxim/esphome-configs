#pragma once
#include "pylontech_rs485_low_voltage.h"

namespace pylontech {

  uint16_t PylontechLowVoltageProtocol::_get_frame_checksum(const uint8_t* frame, uint16_t length) {
    uint32_t sum = 0;

    for (uint16_t i = 0; i < length; i++) {
      sum += frame[i];
    }

    sum %= 0x10000;
    sum = ~sum;
    sum += 1;

    return static_cast<uint16_t>(sum);
  }

  uint16_t PylontechLowVoltageProtocol::_get_info_length(const uint8_t* info, uint16_t length) {
    if (length == 0) {
      return 0;
    }

    int length_sum = (length & 0xF) + ((length >> 4) & 0xF) + ((length >> 8) & 0xF);
    int length_modulo = length_sum % 16;
    int length_invert_plus_one = 0b1111 - length_modulo + 1;

    return (length_invert_plus_one << 12) + length;
  }

  void PylontechLowVoltageProtocol::_handle_frame(const uint8_t* frame, uint16_t length) {
    if (length <= 7) {
      return;
    }
    PylonFrameHeader header = &frame;
    
  }

  void PylontechLowVoltageProtocol::on_data(const uint8_t* data, uint16_t length) {
    this._handle_frame(data, length);
  }

}