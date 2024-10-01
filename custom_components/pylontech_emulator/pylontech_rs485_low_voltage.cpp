#include "pylontech_rs485_low_voltage.h"

namespace pylontech_lv {
  PylontechLowVoltageProtocol::PylontechLowVoltageProtocol(PylontechTransport *transport, uint8_t addr) {
    this->transport_ = transport;
    this->addr_ = addr;
  }

  void PylontechLowVoltageProtocol::loop() {
    uint8_t byte = this->transport_->read_uint8();
    if (byte == 0xFF) {
      return;
    }
    if (byte == SOI) {
      uint8_t *header_bytes = new uint8_t[6];
      uint16_t ascii_sum = 0;
      this->read_ascii_bytes_(header_bytes, 6, &ascii_sum);
      PylonFrame *frame = this->construct_frame_(header_bytes, 6);
      delete[] header_bytes;
      this->handle_frame_(frame, ascii_sum);
    }
  }

  void PylontechLowVoltageProtocol::log_(PylonLogLevel level, const char *format, ...) {
    if (this->write_log_cb_ != nullptr) {
      va_list args;
      va_start(args, format);
      this->write_log_cb_(level, format, args);
      va_end(args);
    }
  }

  void PylontechLowVoltageProtocol::write_uint8_(uint8_t value, std::vector<uint8_t> &data) {
    data.push_back(static_cast<uint8_t>(value));
  }

  void PylontechLowVoltageProtocol::write_uint16_(uint16_t value, std::vector<uint8_t> &data) {
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value & 0xFF));
  }

  void PylontechLowVoltageProtocol::write_float_(float value, std::vector<uint8_t> &data) {
    uint16_t scaled_value = static_cast<uint16_t>(round(value));
    PylontechLowVoltageProtocol::write_uint16_(scaled_value, data);
  }

  bool PylontechLowVoltageProtocol::read_ascii_bytes_(uint8_t *destination, uint8_t len,
      uint16_t *ascii_sum) {
    uint16_t double_len = len * 2;
    uint8_t *raw_bytes = new uint8_t[double_len];
    uint8_t byte = 0;
    for (uint8_t i = 0; i < double_len; i ++) {
      byte = this->transport_->read_uint8();
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

  uint16_t PylontechLowVoltageProtocol::get_frame_checksum_(uint16_t ascii_sum) {
    uint32_t sum = ascii_sum;
    sum %= 0x10000;
    sum = ~sum;
    sum += 1;
    return static_cast<uint16_t>(sum);
  }

  uint8_t PylontechLowVoltageProtocol::get_len_chksum(uint16_t len) {
    int len_sum = (len & 0xF) + ((len >> 4) & 0xF) + ((len >> 8) & 0xF);
    int len_modulo = len_sum & 0xF;
    int len_invert_plus_one = (0xF - len_modulo) + 1;
    return len_invert_plus_one;
  }

  uint16_t PylontechLowVoltageProtocol::get_info_length_(uint16_t len) {
    if (len == 0) {
      return 0;
    }
    int len_chksum = get_len_chksum(len);
    return (len_chksum << 12) + len;
  }

  void PylontechLowVoltageProtocol::ascii_to_uint8_array_(uint8_t* ascii_bytes, uint8_t* output, size_t length) {
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

  void PylontechLowVoltageProtocol::uint8_array_to_ascii_(uint8_t* bytes, uint8_t* ascii_bytes,
      size_t len, uint16_t *ascii_sum) {
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

  bool PylontechLowVoltageProtocol::verify_len_chksum_(uint16_t len, uint8_t len_chksum) {
    uint8_t computed_len_chksum = get_len_chksum(len);
    return computed_len_chksum == len_chksum;
  }

  PylonFrame* PylontechLowVoltageProtocol::construct_frame_(uint8_t *bytes, uint8_t len) {
    PylonFrame *frame = new PylonFrame;
    frame->ver = bytes[0];
    frame->addr = bytes[1];
    frame->cid1 = bytes[2];
    frame->cid2 = bytes[3];
    frame->len_chksum = (bytes[4] & 0xF0) >> 4;
    frame->len_id = (bytes[4] & 0x0F) << 8 | bytes[5]; 
    return frame;
  }

  bool PylontechLowVoltageProtocol::verify_frame_checksum_(uint16_t frame_chksum, uint16_t ascii_sum) {
    uint16_t chksum = this->get_frame_checksum_(ascii_sum);
    if (chksum != frame_chksum) {
      this->log_(PylonLogLevel::ERROR, "Error in checksum verification(expected: %04X, actual: %04X)", chksum, frame_chksum);
      return false;
    }
    return true;
  }

  void PylontechLowVoltageProtocol::read_info_(PylonFrame *frame, uint16_t *ascii_sum) {
    if (frame->len_id > 0) {
      if (!verify_len_chksum_(frame->len_id, frame->len_chksum)) {
        this->send_response_(frame, RTN_CODE_LCHKSUM_ERROR, {});
        return;
      }
      frame->info = new uint8_t[frame->len_id / 2];
      this->read_ascii_bytes_(frame->info, frame->len_id / 2, ascii_sum);
    } else {
      frame->info = nullptr;
    }
  }

  void PylontechLowVoltageProtocol::handle_frame_(PylonFrame *frame, uint16_t ascii_sum) {
    this->log_(PylonLogLevel::DEBUG, "Got frame. Version = %02X, Addr = %02X, Cid1 = %02X, Cid2 = %02X, Lch = %02X, Lid = %04X",
      frame->ver, frame->addr, frame->cid1, frame->cid2, frame->len_chksum, frame->len_id);
    if (frame->addr != this->addr_) {
      this->log_(PylonLogLevel::WARN, "Got frame for non-matching address %d, skipping.", frame->addr);
      delete frame;
      return;
    }
    this->read_info_(frame, &ascii_sum);
    uint8_t *chksum_bytes = new uint8_t[2];
    this->read_ascii_bytes_(chksum_bytes, 2);
    frame->chksum = chksum_bytes[0] << 8 | chksum_bytes[1];
    delete[] chksum_bytes;
    if (!this->verify_frame_checksum_(frame->chksum, ascii_sum)) {
      this->send_response_(frame, RTN_CODE_CHKSUM_ERROR, {});
      return;
    }
    uint8_t last_byte = this->transport_->read_uint8();
    if (last_byte != EOI) {
      this->log_(PylonLogLevel::ERROR, "Last byte is not 0x0D (but %02X)", last_byte);
      this->send_response_(frame, RTN_CODE_VER_ERROR, {});
      return;
    }
    this->handle_command_(frame);
  }

  void PylontechLowVoltageProtocol::handle_command_(PylonFrame *frame) {
    if (frame->cid1 != 0x46) {
      this->log_(PylonLogLevel::ERROR, "Invalid CID1 value %02X", frame->cid1);
    }
    std::vector<uint8_t> info_bytes = {};
    switch (frame->cid2) {
      case 0x61:
        this->log_(PylonLogLevel::INFO, "Received 'Get analog info' command");
        info_bytes = this->get_analog_info_command_bytes_(frame);
        break;
      default:
        this->log_(PylonLogLevel::ERROR, "Received unknown command %02X", frame->cid2);
        this->send_response_(frame, RTN_CODE_CID2_ERROR, {});
        return;
    }
    if (frame->info != nullptr) {
      delete[] frame->info;
    }
    this->send_response_(frame, RTN_CODE_OK, info_bytes);
    delete frame;
  }

  void PylontechLowVoltageProtocol::send_response_(PylonFrame *frame, uint8_t status, std::vector<uint8_t> info_bytes) {
    // TODO: write the array. Try to get rid of prepending the vector.
    uint16_t info_len = this->get_info_length_(info_bytes.size() * 2);
    info_bytes.insert(info_bytes.begin(), info_len & 0x00FF);
    info_bytes.insert(info_bytes.begin(), info_len >> 8);
    info_bytes.insert(info_bytes.begin(), status);
    info_bytes.insert(info_bytes.begin(), frame->cid1);
    info_bytes.insert(info_bytes.begin(), frame->addr);
    info_bytes.insert(info_bytes.begin(), frame->ver);
    this->transport_->write_uint8(SOI);
    uint16_t ascii_sum = 0;
    uint8_t *bytes = new uint8_t[2];
    for (uint16_t i = 0; i < info_bytes.size(); i ++) {
      this->uint8_array_to_ascii_(&info_bytes[i], bytes, 1, &ascii_sum);
      this->transport_->write_uint8(bytes[0]);
      this->transport_->write_uint8(bytes[1]);
    }
    delete[] bytes;
    uint16_t chksum = this->get_frame_checksum_(ascii_sum);
    uint16_t lsb_chksum = (chksum << 8) | (chksum >> 8);
    uint8_t *chksum_ascii_bytes = new uint8_t[4];
    this->uint8_array_to_ascii_((uint8_t*)&lsb_chksum, chksum_ascii_bytes, 2);
    for (uint8_t i = 0; i < 4; i ++) {
      this->transport_->write_uint8(chksum_ascii_bytes[i]);
    }
    delete[] chksum_ascii_bytes;
    this->transport_->write_uint8(EOI);
    this->is_online_ = true;
  }

  std::vector<uint8_t> PylontechLowVoltageProtocol::get_analog_info_command_bytes_(PylonFrame *frame) {
    PylonAnalogInfo info;
    if (this->get_analog_info_cb_ != nullptr) {
      this->get_analog_info_cb_(&info);
    }
    std::vector<uint8_t> result = {};
    PylontechLowVoltageProtocol::write_float_(info.pack_voltage * 1000, result);
    PylontechLowVoltageProtocol::write_float_(info.current, result);
    PylontechLowVoltageProtocol::write_uint8_(info.soc, result);
    PylontechLowVoltageProtocol::write_uint16_(info.avg_nr_of_cycles, result);
    PylontechLowVoltageProtocol::write_uint16_(info.max_nr_of_cycles, result);
    PylontechLowVoltageProtocol::write_uint8_ (info.avg_soh, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_soh, result);
    PylontechLowVoltageProtocol::write_float_(info.max_cell_voltage * 1000, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_cell_voltage_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_cell_voltage_num, result);
    PylontechLowVoltageProtocol::write_float_(info.min_cell_voltage * 1000, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_cell_voltage_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_cell_voltage_num, result);
    PylontechLowVoltageProtocol::write_float_(info.avg_cell_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_float_(info.max_cell_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_cell_temp_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_cell_temp_num, result);
    PylontechLowVoltageProtocol::write_float_(info.min_cell_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_cell_temp_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_cell_temp_num, result);
    PylontechLowVoltageProtocol::write_float_(info.avg_mos_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_float_(info.max_mos_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_mos_temp_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_mos_temp_num, result);
    PylontechLowVoltageProtocol::write_float_(info.min_mos_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_mos_temp_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_mos_temp_num, result);
    PylontechLowVoltageProtocol::write_float_(info.avg_bms_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_float_(info.max_bms_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_bms_temp_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.max_bms_temp_num, result);
    PylontechLowVoltageProtocol::write_float_(info.min_bms_temp_k * 10, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_bms_temp_pack_addr, result);
    PylontechLowVoltageProtocol::write_uint8_(info.min_bms_temp_num, result);

    return result;
  }

}
