#include "binary_display_buffer.h"

namespace esphome {
namespace binary_display_buffer {

BinaryDisplayBuffer::BinaryDisplayBuffer(uint16_t width, uint16_t height)
    : height_(height), width_(width) {
  size_t length = (width * height +7u) / 8u;
  this->init_internal_(length);
  memset(this->buffer_, 0x00, length);
}

void BinaryDisplayBuffer::write_data(Display *display, Color color) {
  
}

int BinaryDisplayBuffer::get_width_internal() {
  return width_;
}

int BinaryDisplayBuffer::get_height_internal() {
  return height_;
}

bool BinaryDisplayBuffer::get_pixel_at_(uint16_t x, uint16_t y) {
  if (x < width_ && y < height_) {
    uint32_t index = y * ((width_ + 7u) / 8u) + x / 8u;
    uint8_t bit = 1 << (x % 8u);
    return (this->buffer_[index] & bit) != 0;
  }
  return false;
}

}
}