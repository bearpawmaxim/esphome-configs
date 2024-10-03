#pragma once

#include <cstdarg>
#include <cstdint>
#include <functional>
#include <vector>
#include <cmath>

namespace pylontech_lv {
  using namespace std;

  static const uint8_t SOI                     = 0x7E;
  static const uint8_t EOI                     = 0x0D;
  static const uint8_t RTN_CODE_OK             = 0x00;
  static const uint8_t RTN_CODE_VER_ERROR      = 0x01;
  static const uint8_t RTN_CODE_CHKSUM_ERROR   = 0x02;
  static const uint8_t RTN_CODE_LCHKSUM_ERROR  = 0x03;
  static const uint8_t RTN_CODE_CID2_ERROR     = 0x04;
  static const uint8_t RTN_CODE_CMD_FMT_ERROR  = 0x05;
  static const uint8_t RTN_CODE_DATA_ERROR     = 0x06;

  enum PylonLogLevel {
    ERROR = 4,
    WARN = 3,
    INFO = 2,
    DEBUG = 1
  };

  // 7E 32 30 30 32 34 36 34 32 45 30 30 32 30 32 46 44 33 33 0D
  struct PylonFrame {
    uint8_t   ver;
    uint8_t   addr;
    uint8_t   cid1;
    uint8_t   cid2;
    uint8_t   len_chksum;
    uint16_t  len_id;
    uint8_t   *info;
    uint16_t  chksum;
  };

  struct PylonAnalogInfo {
    float     pack_voltage; // uint16, volts * 1000.
    float     current; // uint16, amps * 1000.
    uint8_t   soc;
    uint16_t  avg_nr_of_cycles;
    uint16_t  max_nr_of_cycles;
    uint8_t   avg_soh;
    uint8_t   min_soh;
    float     max_cell_voltage; // uint16, volts * 1000.
    uint8_t   max_cell_voltage_pack_addr;
    uint8_t   max_cell_voltage_num;
    float     min_cell_voltage; // uint16, volts * 1000.
    uint8_t   min_cell_voltage_pack_addr;
    uint8_t   min_cell_voltage_num;
    float     avg_cell_temp_k; // uint16, kelvins * 1000.
    float     max_cell_temp_k; // uint16, kelvins * 1000.
    uint8_t   max_cell_temp_pack_addr;
    uint8_t   max_cell_temp_num;
    float     min_cell_temp_k; // uint16, kelvins * 1000.
    uint8_t   min_cell_temp_pack_addr;
    uint8_t   min_cell_temp_num;
    float     avg_mos_temp_k; // uint16, kelvins * 1000.
    float     max_mos_temp_k; // uint16, kelvins * 1000.
    uint8_t   max_mos_temp_pack_addr;
    uint8_t   max_mos_temp_num;
    float     min_mos_temp_k; // uint16, kelvins * 1000.
    uint8_t   min_mos_temp_pack_addr;
    uint8_t   min_mos_temp_num;
    float     avg_bms_temp_k; // uint16, kelvins * 1000.
    float     max_bms_temp_k; // uint16, kelvins * 1000.
    uint8_t   max_bms_temp_pack_addr;
    uint8_t   max_bms_temp_num;
    float     min_bms_temp_k; // uint16, kelvins * 1000.
    uint8_t   min_bms_temp_pack_addr;
    uint8_t   min_bms_temp_num;
  };

  using WriteLogCallback = std::function<void(PylonLogLevel, const char*, va_list)>;
  using GetAnalogInfoCallback = std::function<void(PylonAnalogInfo *info)>;

  class PylontechTransport {
    public:
      virtual uint8_t read_uint8();
      virtual void write_uint8(uint8_t byte);
  };

  class PylontechLowVoltageProtocol {
    public:
      PylontechLowVoltageProtocol(PylontechTransport *transport, uint8_t addr);
      void set_log_callback(WriteLogCallback callback) { this->write_log_cb_ = std::move(callback); }
      void set_get_analog_info_callback(GetAnalogInfoCallback callback) { this->get_analog_info_cb_ = std::move(callback); }
      void loop();
      bool is_online() { return this->is_online_; }

    private:
      inline void log_(PylonLogLevel level, const char *format, ...);
      inline void write_uint8_(uint8_t value, std::vector<uint8_t> &data);
      inline void write_uint16_(uint16_t value, std::vector<uint8_t> &data);
      inline void write_current_(float value, std::vector<uint8_t> &data);
      inline void write_voltage_(float value, std::vector<uint8_t> &data);
      inline void write_temperature_(float value, std::vector<uint8_t> &data);
      static uint16_t get_frame_checksum_(uint16_t ascii_sum);
      static uint8_t get_len_chksum(uint16_t len);
      static uint16_t get_info_length_(uint16_t len);
      static void ascii_to_uint8_array_(uint8_t* ascii_bytes, uint8_t* output,
          size_t length);
      static void uint8_array_to_ascii_(uint8_t* bytes, uint8_t* ascii_bytes,
          size_t len, uint16_t *ascii_sum = nullptr);
      bool static verify_len_chksum_(uint16_t len, uint8_t len_chksum);
      bool read_ascii_bytes_(uint8_t *destination, uint8_t len,
          uint16_t *ascii_sum = nullptr);
      PylonFrame* construct_frame_(uint8_t *bytes, uint8_t len);
      bool verify_frame_checksum_(uint16_t frame_chksum, uint16_t ascii_sum);
      void read_info_(PylonFrame *frame, uint16_t *ascii_sum);
      void handle_frame_(PylonFrame *frame, uint16_t ascii_sum);
      void handle_command_(PylonFrame *frame);
      void send_response_(PylonFrame *frame, uint8_t status,
          std::vector<uint8_t> info_bytes);
      std::vector<uint8_t> get_analog_info_command_bytes_(PylonFrame *frame);

      PylontechTransport *transport_;
      uint8_t addr_;

      bool is_online_;
      WriteLogCallback write_log_cb_{nullptr};
      GetAnalogInfoCallback get_analog_info_cb_{nullptr};
  };

}
