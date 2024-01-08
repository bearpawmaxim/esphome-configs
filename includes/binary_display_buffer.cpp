#include "binary_display_buffer.h"

namespace esphome {
namespace binary_display_buffer {

BinaryDisplayBuffer::BinaryDisplayBuffer(uint16_t width, uint16_t height)
    : height_(height), width_(width) {
  size_t length = (width * height + 7u) / 8u;
  this->init_internal_(length);
  memset(this->buffer_, 0x00, length);
}

void BinaryDisplayBuffer::draw_to(Display *display, Color color) {
  for (uint16_t x = 0; x < width_; ++x) {
    for (uint16_t y = 0; y < height_; ++y) {
      if (get_pixel_at_(x, y)) {
        display->draw_pixel_at(x, y, color);
      }
    }
  }
}

void HOT BinaryDisplayBuffer::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  if (x > width_ || y > height_) {
    return;
  }
  uint32_t index = y * ((width_ + 7u) / 8u) + x / 8u;
  uint8_t bit = 1 << (x % 8u);
  this->buffer_[index] |= bit;
}

bool HOT BinaryDisplayBuffer::get_pixel_at_(uint16_t x, uint16_t y) {
  if (x > width_ && y > height_) {
    return false;
  }
  uint32_t index = y * ((width_ + 7u) / 8u) + x / 8u;
  uint8_t bit = 1 << (x % 8u);
  return (this->buffer_[index] & bit) != 0;
}

}
}