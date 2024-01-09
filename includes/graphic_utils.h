#pragma once

#include "esphome/core/color.h"
#include "esphome/components/display/display.h"

class GraphicUtils {
  public:

    static void triangle(Display *display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2,
        uint16_t y2, Color color = display::COLOR_ON) {
      display->line(x0, y0, x1, y1, color);
      display->line(x1, y1, x2, y2, color);
      display->line(x2, y2, x0, y0, color);
    }

    static void filled_triangle(Display* display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2,
        uint16_t y2, Color color = display::COLOR_ON) {
      triangle(display, x0, y0, x1, y1, x2, y2, color);
      int16_t a, b, y, last;
      if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
      }
      if (y1 > y2) {
        std::swap(y2, y1);
        std::swap(x2, x1);
      }
      if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
      }
      if (y0 == y2) {
        a = b = x0;
        if (x1 < a) {
          a = x1;
        } else if (x1 > b) {
          b = x1;
        }
        if (x2 < a) {
          a = x2;
        } else if (x2 > b) {
          b = x2;
        }
        display->horizontal_line(a, y0, b - a + 1, color);
        return;
      }
      int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
        dx12 = x2 - x1, dy12 = y2 - y1;
      int32_t sa = 0, sb = 0;

      if (y1 == y2) {
        last = y1;
      } else {
        last = y1 - 1;
      }

      for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) {
          std::swap(a, b);
        }
        display->horizontal_line(a, y, b - a + 1, color);
      }

      sa = (int32_t)dx12 * (y - y1);
      sb = (int32_t)dx02 * (y - y0);
      for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) {
          std::swap(a, b);
        }
        display->horizontal_line(a, y, b - a + 1, color);
      }
    }

};