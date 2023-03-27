#include "esphome.h"
#include <ws2812_i2s.h>

class WS2812CustomLight : public Component, public LightOutput {
 private:
  WS2812 led_;
  Pixel_t pixels_[1];

 public:
  void setup() override {
    this->led_.init(1);
  }

  LightTraits get_traits() override {
    auto traits = LightTraits();
    traits.set_supported_color_modes({ColorMode::RGB, ColorMode::BRIGHTNESS});
    return traits;
  }

  void write_state(LightState *state) override {
    float red, green, blue;
    state->current_values_as_rgb(&red, &green, &blue);
    this->pixels_[0].R = red;
    this->pixels_[0].G = green;
    this->pixels_[0].B = blue;
    this->led_.show(this->pixels_);
  }
};
