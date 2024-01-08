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
    void draw_to(Display *display, Color color);

  protected:
    int get_width_internal() override { return width_; }
    int get_height_internal() override { return height_; }
    display::DisplayType get_display_type() override {
      return display::DisplayType::DISPLAY_TYPE_BINARY;
    }
    void draw_absolute_pixel_internal(int x, int y, esphome::Color color) override;
    void update() override {};

  private:
    uint16_t width_ = 240;
    uint16_t height_ = 240;
    bool get_pixel_at_(uint16_t x, uint16_t y);

};

}
}