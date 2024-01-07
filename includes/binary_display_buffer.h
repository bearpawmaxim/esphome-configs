#pragma once

#include "esphome.h"
#include "esphome/core/color.h"
#include "esphome/components/display/display.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace binary_display_buffer {

class BinaryDisplayBuffer: public display::DisplayBuffer {
  public:
    BinaryDisplayBuffer(uint16_t width, uint16_t height);
    void write_data(Display *display, Color color);

  protected:
    int get_width_internal() override;
    int get_height_internal() override;

  private:
    uint16_t width_ = 240;
    uint16_t height_ = 240;
    bool get_pixel_at_(uint16_t x, uint16_t y);

};

}
}