#pragma once

#include <cstdint>
#include <functional>
#include "esphome.h"

namespace pylontech {
  using namespace esphome;

  static const uint8_t SOI                     = 0x7E;
  static const uint8_t EOI                     = 0x0D;
  static const uint8_t RTN_CODE_OK             = 0x00;
  static const uint8_t RTN_CODE_VER_ERROR      = 0x01;
  static const uint8_t RTN_CODE_CHKSUM_ERROR   = 0x02;
  static const uint8_t RTN_CODE_LCHKSUM_ERROR  = 0x03;
  static const uint8_t RTN_CODE_CID2_ERROR     = 0x04;
  static const uint8_t RTN_CODE_CMD_FMT_ERROR  = 0x05;
  static const uint8_t RTN_CODE_DATA_ERROR     = 0x06;

  // 7E 32 30 30 32 34 36 34 32 45 30 30 32 30 32 46 44 33 33 0D
  typedef struct PylonFrame {
    uint8_t   ver;
    uint8_t   addr;
    uint8_t   cid1;
    uint8_t   cid2;
    uint8_t   len_chksum;
    uint16_t  len_id;
    uint8_t   *info;
    uint16_t  chksum;
    uint16_t  _ascii_sum;
  } PylonFrame_t;

  typedef struct PylonAnalogInfo {
    float     pack_voltage; // uint16, volts * 1000.
    float     current; // uint16, amps * 1000.
    uint8_t   soc;
    uint16_t  avg_nr_of_cycles;
    uint16_t  max_nr_of_cycles;
    uint8_t   avg_soh;
    uint8_t   min_soh;
    float     max_cell_voltage; // uint16, volts * 1000.
    uint8_t   max_cell_voltage_pack_num;
    uint8_t   max_cell_voltage_num;
    float     min_cell_voltage; // uint16, volts * 1000.
    uint8_t   min_cell_voltage_pack_num;
    uint8_t   min_cell_voltage_num;
    float     avg_cell_temp_k; // uint16, kelvins * 1000.
    float     max_cell_temp_k; // uint16, kelvins * 1000.
    uint8_t   max_cell_temp_pack_num;
    uint8_t   max_cell_temp_num;
    float     min_cell_temp_k; // uint16, kelvins * 1000.
    uint8_t   min_cell_temp_pack_num;
    uint8_t   min_cell_temp_num;
    float     avg_mos_temp_k; // uint16, kelvins * 1000.
    float     max_mos_temp_k; // uint16, kelvins * 1000.
    uint8_t   max_mos_temp_pack_num;
    uint8_t   max_mos_temp_num;
    float     min_mos_temp_k; // uint16, kelvins * 1000.
    uint8_t   min_mos_temp_pack_num;
    uint8_t   min_mos_temp_num;
    float     avg_bms_temp_k; // uint16, kelvins * 1000.
    float     max_bms_temp_k; // uint16, kelvins * 1000.
    uint8_t   max_bms_temp_pack_num;
    uint8_t   max_bms_temp_num;
    float     min_bms_temp_k; // uint16, kelvins * 1000.
    uint8_t   min_bms_temp_pack_num;
    uint8_t   min_bms_temp_num;
  } PylonAnalogInfo_t;

  class PylontechLowVoltageProtocol {
    public:
      void set_read_byte_callback(std::function<uint8_t()> &&callback) {
        this->read_byte_cb_ = std::move(callback);
      }

      void set_get_analog_info_callback(std::function<void(PylonAnalogInfo*)> &&callback) {
        this->get_analog_info_cb_ = std::move(callback);
      }

      void set_write_byte_callback(std::function<void(uint8_t)> &&callback) {
        this->send_byte_cb_ = std::move(callback);
      }

      void loop() {
        uint8_t byte = this->read_byte_cb_();
        if (byte == 0xFF) {
          return;
        }
        if (byte == SOI) {
          uint8_t *header_bytes = new uint8_t[6];
          uint16_t ascii_sum = 0;
          this->read_ascii_bytes_(header_bytes, 6, &ascii_sum);
          PylonFrame *header = this->construct_frame_(header_bytes, 6, ascii_sum);
          delete[] header_bytes;
          this->handle_frame_(header);
        }
      }

    private:
      static inline void write_uint8_(uint8_t value, std::vector<uint8_t> &data) {
        data.push_back(static_cast<uint8_t>(value));
      }

      static inline void write_uint16_(uint16_t value, std::vector<uint8_t> &data) {
        data.push_back(static_cast<uint8_t>(value >> 8));
        data.push_back(static_cast<uint8_t>(value & 0xFF));
      }

      static inline void write_float_(float value, std::vector<uint8_t> &data) {
        uint16_t scaled_value = static_cast<uint16_t>(round(value));
        PylontechLowVoltageProtocol::write_uint16_(scaled_value, data);
      }

      bool read_ascii_bytes_(uint8_t *destination, uint8_t len,
          uint16_t *ascii_sum = nullptr) {
        uint16_t double_len = len * 2;
        uint8_t *raw_bytes = new uint8_t[double_len];
        uint8_t byte = 0;
        for (uint8_t i = 0; i < double_len; i ++) {
          byte = this->read_byte_cb_();
          if (i < double_len && (byte == EOI || byte == 0xFF)) {
            return false;
          }
          if (ascii_sum != nullptr) {
            *ascii_sum += byte;
          }
          raw_bytes[i] = byte;
        }
        this->ascii_to_uint8_array_(raw_bytes, destination, double_len);
        delete[] raw_bytes;
        return true;
      }

      static uint16_t get_frame_checksum_(uint16_t ascii_sum) {
        uint32_t sum = ascii_sum;
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
          uint8_t high_nibble = (ascii_bytes[i] <= '9')
            ? (ascii_bytes[i] - '0')
            : (ascii_bytes[i] - 'A' + 10);
          uint8_t low_nibble  = (ascii_bytes[i + 1] <= '9')
            ? (ascii_bytes[i + 1] - '0')
            : (ascii_bytes[i + 1] - 'A' + 10);
          output[i / 2] = (high_nibble << 4) | low_nibble;
        }
      }

      static void uint8_array_to_ascii_(uint8_t* bytes, uint8_t* ascii_bytes,
          size_t len, uint16_t *ascii_sum = nullptr) {
        for (size_t i = 0; i < len; ++i) {
          uint8_t high_nibble = (bytes[i] >> 4) & 0x0F;
          uint8_t low_nibble  = bytes[i] & 0x0F;
          uint8_t first_nibble_byte = (high_nibble < 10)
            ? ('0' + high_nibble)
            : ('A' + high_nibble - 10);
          uint8_t second_nibble_byte = (low_nibble < 10)
            ? ('0' + low_nibble)
            : ('A' + low_nibble - 10);
          ascii_bytes[i * 2]     = first_nibble_byte;
          ascii_bytes[i * 2 + 1] = second_nibble_byte;
          if (ascii_sum != nullptr) {
            *ascii_sum += first_nibble_byte;
            *ascii_sum += second_nibble_byte;
          }
        }
      }

      bool static verify_len_chksum_(uint16_t len, uint8_t len_chksum) {
        uint8_t computed_len_chksum = get_len_chksum(len);
        return computed_len_chksum == len_chksum;
      }

      PylonFrame* construct_frame_(uint8_t *bytes, uint8_t len, uint16_t ascii_sum) {
        PylonFrame *frame = new PylonFrame;
        frame->ver = bytes[0];
        frame->addr = bytes[1];
        frame->cid1 = bytes[2];
        frame->cid2 = bytes[3];
        frame->len_chksum = (bytes[4] & 0xF0) >> 4;
        frame->len_id = (bytes[4] & 0x0F) << 8 | bytes[5]; 
        frame->_ascii_sum = ascii_sum;
        return frame;
      }

      bool verify_frame_checksum_(PylonFrame *frame) {
        uint16_t chksum = this->get_frame_checksum_(frame->_ascii_sum);
        if (chksum != frame->chksum) {
          ESP_LOGE("PYL", "Error in checksum verification(expected: %04X, actual: %04X)", chksum, frame->chksum);
          return false;
        }
        return true;
      }

      void handle_frame_(PylonFrame *frame) {
        esphome::ESP_LOGE("PYL", "Got frame. Version = %02X, Addr = %02X, Cid1 = %02X, Cid2 = %02X, Lch = %02X, Lid = %04X",
          frame->ver, frame->addr, frame->cid1, frame->cid2, frame->len_chksum, frame->len_id);
        if (frame->len_id > 0) {
          if (!verify_len_chksum_(frame->len_id, frame->len_chksum)) {
            this->send_response_(frame, RTN_CODE_LCHKSUM_ERROR, {});
            return;
          }
          frame->info = new uint8_t[frame->len_id / 2];
          this->read_ascii_bytes_(frame->info, frame->len_id / 2, &frame->_ascii_sum);
        } else {
          frame->info = nullptr;
        }
        uint8_t *chksum_bytes = new uint8_t[2];
        this->read_ascii_bytes_(chksum_bytes, 2, nullptr);
        frame->chksum = chksum_bytes[0] << 8 | chksum_bytes[1];
        delete[] chksum_bytes;
        if (!this->verify_frame_checksum_(frame)) {
          this->send_response_(frame, RTN_CODE_CHKSUM_ERROR, {});
          return;
        }
        uint8_t last_byte = this->read_byte_cb_();
        if (last_byte != EOI) {
          ESP_LOGE("PYL", "Last byte is not 0x0D (but %02X)", last_byte);
          this->send_response_(frame, RTN_CODE_VER_ERROR, {});
          return;
        }
        this->handle_command_(frame);
      }

      void handle_command_(PylonFrame *frame) {
        if (frame->cid1 != 0x46) {
          ESP_LOGE("PYL", "Invalid CID1 value %02X", frame->cid1);
        }
        std::vector<uint8_t> info_bytes = {};
        switch (frame->cid2) {
          case 0x61:
            ESP_LOGE("PYL", "Received 'Get analog info' command");
            info_bytes = this->get_analog_info_command_bytes_();
            break;
          default:
            ESP_LOGE("PYL", "Received unknown command %02X", frame->cid2);
            this->send_response_(frame, RTN_CODE_CID2_ERROR, {});
            return;
        }
        if (frame->info != nullptr) {
          delete[] frame->info;
        }
        this->send_response_(frame, RTN_CODE_OK, info_bytes);
        delete frame;
      }

      void send_response_(PylonFrame *frame, uint8_t status, std::vector<uint8_t> info_bytes) {
        uint16_t info_len = this->get_info_length_(info_bytes.size() * 2);
        info_bytes.insert(info_bytes.begin(), info_len & 0x00FF);
        info_bytes.insert(info_bytes.begin(), info_len >> 8);
        info_bytes.insert(info_bytes.begin(), status);
        info_bytes.insert(info_bytes.begin(), frame->cid1);
        info_bytes.insert(info_bytes.begin(), frame->addr);
        info_bytes.insert(info_bytes.begin(), frame->ver);
        this->send_byte_cb_(SOI);
        uint16_t ascii_sum = 0;
        uint8_t *bytes = new uint8_t[2];
        for (uint16_t i = 0; i < info_bytes.size(); i ++) {
          this->uint8_array_to_ascii_(&info_bytes[i], bytes, 1, &ascii_sum);
          this->send_byte_cb_(bytes[0]);
          this->send_byte_cb_(bytes[1]);
        }
        delete[] bytes;
        uint16_t chksum = this->get_frame_checksum_(ascii_sum);
        uint16_t lsb_chksum = (chksum << 8) | (chksum >> 8);
        uint8_t *chksum_ascii_bytes = new uint8_t[4];
        this->uint8_array_to_ascii_((uint8_t*)&lsb_chksum, chksum_ascii_bytes, 2);
        for (uint8_t i = 0; i < 4; i ++) {
          this->send_byte_cb_(chksum_ascii_bytes[i]);
        }
        delete[] chksum_ascii_bytes;
        this->send_byte_cb_(EOI);
      }

      std::vector<uint8_t> get_analog_info_command_bytes_() {
        PylonAnalogInfo *info = new PylonAnalogInfo();
        this->get_analog_info_cb_(info);
        std::vector<uint8_t> result = {};
        PylontechLowVoltageProtocol::write_float_(info->pack_voltage * 1000, result);
        PylontechLowVoltageProtocol::write_float_(info->current * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->soc, result);
        PylontechLowVoltageProtocol::write_uint16_(info->avg_nr_of_cycles, result);
        PylontechLowVoltageProtocol::write_uint16_(info->max_nr_of_cycles, result);
        PylontechLowVoltageProtocol::write_uint8_(info->avg_soh, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_soh, result);
        PylontechLowVoltageProtocol::write_float_(info->max_cell_voltage * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_cell_voltage_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_cell_voltage_num, result);
        PylontechLowVoltageProtocol::write_float_(info->min_cell_voltage * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_cell_voltage_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_cell_voltage_num, result);
        PylontechLowVoltageProtocol::write_float_(info->avg_cell_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_float_(info->max_cell_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_cell_temp_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_cell_temp_num, result);
        PylontechLowVoltageProtocol::write_float_(info->min_cell_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_cell_temp_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_cell_temp_num, result);
        PylontechLowVoltageProtocol::write_float_(info->avg_mos_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_float_(info->max_mos_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_mos_temp_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_mos_temp_num, result);
        PylontechLowVoltageProtocol::write_float_(info->min_mos_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_mos_temp_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_mos_temp_num, result);
        PylontechLowVoltageProtocol::write_float_(info->avg_bms_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_float_(info->max_bms_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_bms_temp_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->max_bms_temp_num, result);
        PylontechLowVoltageProtocol::write_float_(info->min_bms_temp_k * 1000, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_bms_temp_pack_num, result);
        PylontechLowVoltageProtocol::write_uint8_(info->min_bms_temp_num, result);
        delete info;
        return result;
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
      std::function<void(PylonAnalogInfo*)> get_analog_info_cb_;
      std::function<void(uint8_t)> send_byte_cb_;
  };

}