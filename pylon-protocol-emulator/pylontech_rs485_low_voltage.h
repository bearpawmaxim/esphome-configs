#include <cstdint>

namespace pylontech {

  // 7E 32 30 30 32 34 36 34 32 45 30 30 32 30 32 46 44 33 33 0D
  struct {
    // uint8_t soi = 0x7E;
    uint8_t ver = 0x32;
    uint8_t addr;
    uint8_t cid1;
    uint8_t cid2;
    uint16_t length;
    const char* info;
    uint16_t checksum;
    uint8_t eoi = 0x0D;
  } PylonFrameHeader;

  struct {
    const uint8_t* info;
    uint16_t checksum;
    // uint8_t eoi = 0x0D;
  } PylonFrameData;

  class PylontechLowVoltageProtocol {
    public:
      void set_read_byte_callback(std::function<uint8_t()> read_byte);
      void set_get_
      void on_data(const uint8_t* data, uint16_t length);

    private:
      static uint16_t _get_frame_checksum(const uint8_t* frame, uint16_t length);
      static uint16_t _get_info_length(const uint8_t* info, uint16_t length);
      void _handle_frame(const uint8_t* frame, uint16_t length);
  };

}