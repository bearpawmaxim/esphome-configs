#include "binary_display_buffer.h"
#include "graphic_utils.h"

using namespace esphome::binary_display_buffer;

class Clock {

  private:
    static constexpr double shift_angle_ = radians(90);
    static constexpr double angle_step_ = M_PI / 30;
    static BinaryDisplayBuffer *buffer_;
    static uint8_t tick_values_[60][4];
    static bool stopped_;

    static void compute_tick_(uint8_t number, uint16_t center_x, uint16_t center_y, uint16_t outer_radius,
        uint16_t inner_radius) {
      tick_values_[number][0] =
        static_cast<uint8_t>(center_x + outer_radius * cos(number * angle_step_ - shift_angle_));
      tick_values_[number][1] =
        static_cast<uint8_t>(center_y + outer_radius * sin(number * angle_step_ - shift_angle_));
      tick_values_[number][2] =
        static_cast<uint8_t>(center_x + inner_radius * cos(number * angle_step_ - shift_angle_));
      tick_values_[number][3] =
        static_cast<uint8_t>(center_y + inner_radius * sin(number * angle_step_ - shift_angle_));
    }

    static void draw_bezel_(Display *display, uint16_t center_x, uint16_t center_y, uint16_t outer_radius,
        uint16_t inner_radius) {
      display->circle(center_x, center_y, outer_radius);
        for (int i = 0; i < 60; ++i) {
          uint16_t outer_radius_adj = i % 5 == 0 ? outer_radius + 5 : outer_radius;
          if (tick_values_[i][0] == 0 && tick_values_[i][1] == 0) {
            compute_tick_(i, center_x, center_y, outer_radius_adj, inner_radius);
          }
          auto tick_values = tick_values_[i];
          if (i == 0) {
            GraphicUtils::filled_triangle(display, tick_values[0] - 5,
              tick_values[1], tick_values[0] + 5, tick_values[1],
              tick_values[2], tick_values[3]);
            continue;
          }
          display->line(tick_values[0], tick_values[1],
            tick_values[2], tick_values[3]);
      }
    }

    static void draw_minutes_hand_(Display *display, uint16_t x, uint16_t y, uint16_t radius, ESPTime time,
        Color color) {
      const double angle_step = M_PI / 30;
      uint8_t i = time.minute;
      uint8_t x1 = static_cast<uint8_t>(x + (radius - 7) * cos(i * angle_step - angle_step - shift_angle_ + 0.04));
      uint8_t y1 = static_cast<uint8_t>(y + (radius - 7) * sin(i * angle_step - angle_step - shift_angle_ + 0.04));
      uint8_t x2 = static_cast<uint8_t>(x + (radius - 7) * cos(i * angle_step + angle_step - shift_angle_ - 0.04));
      uint8_t y2 = static_cast<uint8_t>(y + (radius - 7) * sin(i * angle_step + angle_step - shift_angle_ - 0.04));
      auto tick_values = tick_values_[i];
      uint8_t x3 = tick_values[2];
      uint8_t y3 = tick_values[3];
      GraphicUtils::filled_triangle(display, x1, y1, x2, y2, x3, y3, color);
    }

    static void draw_hours_hand_(Display *display, uint16_t x, uint16_t y, uint16_t radius, ESPTime time, Color color) {
      const double angle_step = M_PI / 6;
      const double sixth_step = angle_step / 6;
      uint8_t i = time.hour > 12 ? time.hour - 12 : time.hour;
      uint8_t x1 = static_cast<uint8_t>(x + (radius + 7) * cos(i * angle_step - sixth_step - shift_angle_ + 0.04));
      uint8_t y1 = static_cast<uint8_t>(y + (radius + 7) * sin(i * angle_step - sixth_step - shift_angle_ + 0.04));
      uint8_t x2 = static_cast<uint8_t>(x + (radius + 7) * cos(i * angle_step + sixth_step - shift_angle_ - 0.04));
      uint8_t y2 = static_cast<uint8_t>(y + (radius + 7) * sin(i * angle_step + sixth_step - shift_angle_ - 0.04));
      uint8_t x3 = static_cast<uint8_t>(x + (radius) * cos(i * angle_step - shift_angle_));
      uint8_t y3 = static_cast<uint8_t>(y + (radius) * sin(i * angle_step - shift_angle_));
      GraphicUtils::filled_triangle(display, x1, y1, x2, y2, x3, y3, color);
    }

    static void draw_seconds_hand_(Display *display, ESPTime time, Color color) {
      auto tick_values = tick_values_[time.second];
      display->line(tick_values[0], tick_values[1], tick_values[2], tick_values[3], color);
    }

  public:

    static void draw_watch_face(Display *display, uint16_t center_x,  uint16_t center_y, uint16_t outer_radius,
        uint16_t inner_radius, ESPTime time, Color face_color, Color hours_color, Color minutes_color,
        Color seconds_color) {
      if (stopped_) {
        return;
      }
      if (!buffer_) {
        buffer_ = new BinaryDisplayBuffer(display->get_width(), display->get_height());
        draw_bezel_(buffer_, center_x, center_y, outer_radius, inner_radius);
      }
      buffer_->draw_to(display, face_color);
      if (time.is_valid()) {
        draw_hours_hand_(display, center_x, center_y, outer_radius, time, hours_color);
        draw_minutes_hand_(display, center_x, center_y, inner_radius, time, minutes_color);
        draw_seconds_hand_(display, time, seconds_color);
      }
    }

    static void stop() {
      buffer_->~BinaryDisplayBuffer();
      free(buffer_);
      stopped_ = true;
    }

};

BinaryDisplayBuffer *Clock::buffer_ = NULL;
uint8_t Clock::tick_values_[60][4];
bool Clock::stopped_ = false;