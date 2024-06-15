#pragma once

#include "soc/rtc.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"

class HoldingGpio
{
  private:
    gpio_num_t _gpio;

    bool _get_gpio_state() {
      gpio_hold_dis(_gpio);
      bool state = gpio_get_level(_gpio) == 1;
      gpio_hold_en(_gpio);
      return state;
    }

  public:
    HoldingGpio(gpio_num_t gpio) {
      gpio_set_direction(gpio, GPIO_MODE_INPUT_OUTPUT);
      _gpio = gpio;
      gpio_hold_en(gpio);
    }

    void set_state(bool state) {
      gpio_hold_dis(_gpio);
      gpio_set_level(_gpio, state);
      gpio_hold_en(_gpio);
    }

    bool get_state() {
      return _get_gpio_state();
    }


};

HoldingGpio *holding_gpio;