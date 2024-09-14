#pragma once

#include <cstdint>
#include <functional>
#include "esphome.h"

namespace pylontech {

  static const uint8_t SOI = 0x7E;
  static const uint8_t EOI = 0x0D;

  // 7E 32 30 30 32 34 36 34 32 45 30 30 32 30 32 46 44 33 33 0D
  typedef struct PylonFrameHeader {
    uint8_t ver = 0x32;
    uint8_t addr;
    uint8_t cid1;
    uint8_t cid2;
    uint16_t length;
  } PylonFrameHeader_t;

  // struct PylonFrameFooter {
  //   uint16_t checksum;
  // } PylonFrameFooter_t;

  class PylontechLowVoltageProtocol {
    public:
      void set_read_byte_callback(std::function<uint8_t()> callback) {
        this->read_byte_cb_ = read_byte_cb_;
      }
      void set_get_analog_info_callback(std::function<void()> callback) {
        this->get_analog_info_cb_ = std::move(callback);
      }
      void loop() {
        uint8_t byte = this->read_byte_cb_();
        if (byte == 0xFF) {
          return;
        }
        PylonFrameHeader *header = {};
        if (byte == SOI) {
          uint8_t *header_bytes = new uint8_t[12];
          for (uint8_t i = 0; i < 12; i ++) {
            byte = this->read_byte_cb_();
            if (i < 12 && (byte == EOI || byte == 0xFF)) {
              return;
            }
            header_bytes[i] = byte;
          }
          this->ascii_to_uint8_array_((uint8_t*)header, header_bytes, 12);
          log_hex_(header_bytes, 6);
          this->handle_frame_(header);
        }
      }

    private:
      static uint16_t get_frame_checksum_(uint8_t* frame, uint16_t length) {
        uint32_t sum = 0;

        for (uint16_t i = 0; i < length; i++) {
          sum += frame[i];
        }

        sum %= 0x10000;
        sum = ~sum;
        sum += 1;

        return static_cast<uint16_t>(sum);
      }

      static uint16_t get_info_length_(uint8_t* info, uint16_t length) {
        if (length == 0) {
          return 0;
        }

        int length_sum = (length & 0xF) + ((length >> 4) & 0xF) + ((length >> 8) & 0xF);
        int length_modulo = length_sum % 16;
        int length_invert_plus_one = 0b1111 - length_modulo + 1;

        return (length_invert_plus_one << 12) + length;
      }

      static void ascii_to_uint8_array_(uint8_t* ascii_bytes, uint8_t* output, size_t length) {
        for (size_t i = 0; i < length; i += 2) {
          // Convert the two ASCII characters to a single uint8_t value
          uint8_t high_nibble = (ascii_bytes[i] <= '9')
            ? (ascii_bytes[i] - '0')
            : (ascii_bytes[i] - 'A' + 10);
          uint8_t low_nibble  = (ascii_bytes[i + 1] <= '9')
            ? (ascii_bytes[i + 1] - '0')
            : (ascii_bytes[i + 1] - 'A' + 10);
          output[i / 2] = (high_nibble << 4) | low_nibble;
        }
      }

      static void uint8_array_to_ascii_(uint8_t* bytes, uint8_t* ascii_bytes, size_t length) {
        for (size_t i = 0; i < length; ++i) {
          uint8_t high_nibble = (bytes[i] >> 4) & 0x0F;
          uint8_t low_nibble  = bytes[i] & 0x0F;
          ascii_bytes[i * 2]     = (high_nibble < 10) ? ('0' + high_nibble) : ('A' + high_nibble - 10);
          ascii_bytes[i * 2 + 1] = (low_nibble < 10) ? ('0' + low_nibble) : ('A' + low_nibble - 10);
        }
        ascii_bytes[length * 2] = '\0';
      }

      void handle_frame_(PylonFrameHeader* frame) {
        esphome::ESP_LOGE("PYL", "Got frame. Version = %d, Addr = %d, Cid1 = %d, Cid2 = %d, Len = %d",
          frame->ver, frame->addr, frame->cid1, frame->cid2, (frame->length << 4) >> 4);
        
      }

      void log_hex_(uint8_t *bytes, uint8_t len) {
        std::string res = "";
        char buf[5];
        for (size_t i = 0; i < len; i++) {
          sprintf(buf, "%02X", bytes[i]);
          res += buf;
        }
        esphome::ESP_LOGE("PYL", "Got frame header: 0x%s", res.c_str());
      }
      std::function<uint8_t()> read_byte_cb_;
      std::function<void()> get_analog_info_cb_;
  };

}