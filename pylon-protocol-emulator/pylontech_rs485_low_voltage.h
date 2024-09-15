#pragma once

#include <cstdint>
#include <functional>
#include "esphome.h"

namespace pylontech {
  using namespace esphome;

  static const uint8_t SOI = 0x7E;
  static const uint8_t EOI = 0x0D;

  // 7E 32 30 30 32 34 36 34 32 45 30 30 32 30 32 46 44 33 33 0D
  typedef struct PylonFrame {
    uint8_t ver;
    uint8_t addr;
    uint8_t cid1;
    uint8_t cid2;
    uint8_t len_chksum;
    uint16_t len_id;
    uint8_t *info;
    uint16_t chksum;
  } PylonFrame_t;

  // typedef struct PylonFrameFooter {
  //   uint16_t checksum;
  // } PylonFrameFooter_t;

  class PylontechLowVoltageProtocol {
    public:
      void set_read_byte_callback(std::function<uint8_t()> &&callback) {
        this->read_byte_cb_ = std::move(callback);
      }

      void set_get_analog_info_callback(std::function<void()> &&callback) {
        this->get_analog_info_cb_ = std::move(callback);
      }

      bool read_ascii_bytes(uint8_t *destination, uint8_t len) {
        uint16_t double_len = len * 2;
        uint8_t *raw_bytes = new uint8_t[double_len];
        uint8_t byte = 0;
        for (uint8_t i = 0; i < double_len; i ++) {
          byte = this->read_byte_cb_();
          if (i < double_len && (byte == EOI || byte == 0xFF)) {
            return false;
          }
          raw_bytes[i] = byte;
        }
        this->ascii_to_uint8_array_(raw_bytes, destination, double_len);
        return true;
      }

      void loop() {
        uint8_t byte = this->read_byte_cb_();
        if (byte == 0xFF) {
          return;
        }
        if (byte == SOI) {
          uint8_t *header_bytes = new uint8_t[6];
          this->read_ascii_bytes(header_bytes, 6);
          PylonFrame *header = this->construct_frame_(header_bytes, 6);
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

      static uint8_t get_len_chksum(uint16_t len) {
        int len_sum = (len & 0xF) + ((len >> 4) & 0xF) + ((len >> 8) & 0xF);
        int len_modulo = len_sum & 0xF;
        int len_invert_plus_one = (0xF - len_modulo) + 1;
        return len_invert_plus_one;
      }

      static uint16_t get_info_length_(uint16_t len) {
        if (len == 0) {
          return 0;
        }
        int len_chksum = get_len_chksum(len);
        return (len_chksum << 12) + len;
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
          ESP_LOGE("A2H", "h: %02X, l: %02X", high_nibble, low_nibble);
          output[i / 2] = (high_nibble << 4) | low_nibble;
        }
      }

      bool static verify_len_chksum_(uint16_t len, uint8_t len_chksum) {
        uint8_t computed_len_chksum = get_len_chksum(len);
        ESP_LOGE("PYL", "Length checksum %d, computed %d", len_chksum, computed_len_chksum);
        return computed_len_chksum == len_chksum;
      }

      PylonFrame* construct_frame_(uint8_t *bytes, uint8_t len) {
        PylonFrame *header = new PylonFrame;
        header->ver = bytes[0];
        header->addr = bytes[1];
        header->cid1 = bytes[2];
        header->cid2 = bytes[3];
        header->len_chksum = (bytes[4] & 0xF0) >> 4;
        header->len_id = (bytes[4] & 0x0F) << 8 | bytes[5]; 
        return header;
      }

      void handle_frame_(PylonFrame *frame) {
        esphome::ESP_LOGE("PYL", "Got frame. Version = %02X, Addr = %02X, Cid1 = %02X, Cid2 = %02X, Lch = %02X, Lid = %04X",
          frame->ver, frame->addr, frame->cid1, frame->cid2, frame->len_chksum, frame->len_id);
        if (!verify_len_chksum_(frame->len_id, frame->len_chksum)) {
          return;
        }
        uint8_t *info_bytes = new uint8_t[frame->len_id / 2];
        this->read_ascii_bytes(info_bytes, frame->len_id / 2);
        log_hex_("Info", info_bytes, frame->len_id / 2);
        uint8_t *chksum_bytes = new uint8_t[2];
        this->read_ascii_bytes(chksum_bytes, 2);
        frame->chksum = chksum_bytes[0] << 8 | chksum_bytes[1];
        ESP_LOGE("PYL", "Frame checksum %04X", frame->chksum);

        // TODO: Verify checksum.
        uint8_t last_byte = this->read_byte_cb_();
        if (last_byte != EOI) {
          ESP_LOGE("PYL", "Last byte is not 0x0D (but %02X)", last_byte);
          return;
        }
        this->handle_command_(frame);
      }

      void handle_command_(PylonFrame *frame) {
        if (frame->cid1 != 0x46) {
          ESP_LOGE("PYL", "Invalid CID1 value %02X", frame->cid1);
        }
        switch (frame->cid2) {
          case 0x61:
            ESP_LOGE("PYL", "Received 'Get analog info' command");
            break;
          default:
            ESP_LOGE("PYL", "Received unknown command %02X", frame->cid2);
            break;
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

      void log_hex_(std::string name, uint8_t *bytes, uint8_t len) {
        std::string res = "";
        char buf[5];
        for (size_t i = 0; i < len; i++) {
          sprintf(buf, "%02X", bytes[i]);
          res += buf;
        }
        esphome::ESP_LOGE("PYL", "%s: 0x%s", name.c_str(), res.c_str());
      }

      std::function<uint8_t()> read_byte_cb_;
      std::function<void()> get_analog_info_cb_;
  };

}