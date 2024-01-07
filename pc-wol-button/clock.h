class Clock {

  private:
    struct BinaryImageBuffer {
      private:
        uint8_t *buffer;
        uint16_t width;
        uint16_t height;
        void set_pixel_at_(uint16_t x, uint16_t y) {
          if (x < width && y < height) {
            uint32_t index = y * ((width + 7u) / 8u) + x / 8u;
            uint8_t bit = 1 << (x % 8u);
            buffer[index] |= bit;
          }
        }

        bool get_pixel_at_(uint16_t x, uint16_t y) {
          if (x < width && y < height) {
            uint32_t index = y * ((width + 7u) / 8u) + x / 8u;
            uint8_t bit = 1 << (x % 8u);
            return (buffer[index] & bit) != 0;
          }
          return false;
        }

      public:
        BinaryImageBuffer(uint16_t w, uint16_t h): height(h), width(w) {
          uint16_t length = (width * height +7u) / 8u;
          buffer = static_cast<uint8_t*>(malloc(length));
          memset(buffer, 0x00, length);
        }

        void line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
          const int32_t dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
          const int32_t dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
          int32_t err = dx + dy;

          while (true) {
            set_pixel_at_(x1, y1);
            if (x1 == x2 && y1 == y2) {
              break;
            }
            int32_t e2 = 2 * err;
            if (e2 >= dy) {
              err += dy;
              x1 += sx;
            }
            if (e2 <= dx) {
              err += dx;
              y1 += sy;
            }
          }
        }

        void circle(uint16_t center_x, uint16_t center_y, uint16_t radius) {
          int dx = -radius;
          int dy = 0;
          int err = 2 - 2 * radius;
          int e2;

          do {
            set_pixel_at_(center_x - dx, center_y + dy);
            set_pixel_at_(center_x + dx, center_y + dy);
            set_pixel_at_(center_x + dx, center_y - dy);
            set_pixel_at_(center_x - dx, center_y - dy);
            e2 = err;
            if (e2 < dy) {
              err += ++dy * 2 + 1;
              if (-dx == dy && e2 <= dx) {
                e2 = 0;
              }
            }
            if (e2 > dx) {
              err += ++dx * 2 + 1;
            }
          } while (dx <= 0);
        }

        void draw_buffer(Display *it, Color color) {
          for (uint16_t x = 0; x < it->get_width(); ++x) {
            for (uint16_t y = 0; y < it->get_height(); ++y) {
              if (get_pixel_at_(x, y)) {
                it->draw_pixel_at(x, y, color);
              }
            }
          }
        }
    };

    static constexpr double shift_angle_ = radians(90);
    static BinaryImageBuffer *buffer_;
    static uint8_t tick_values_[60][4];

    static void draw_bezel_(BinaryImageBuffer *buffer, uint16_t center_x,
        uint16_t center_y, uint16_t outer_radius, uint16_t inner_radius) {
      buffer->circle(center_x, center_y, outer_radius);
      const double angle_step = M_PI / 30;
      if (tick_values_[0][0] == 0) {
        for (int i = 0; i < 60; ++i) {
          uint16_t outer_radius_adj = i % 5 == 0 ? outer_radius + 5 : outer_radius;
          tick_values_[i][0] = static_cast<uint8_t>(center_x + outer_radius_adj * cos(i * angle_step - shift_angle_));
          tick_values_[i][1] = static_cast<uint8_t>(center_y + outer_radius_adj * sin(i * angle_step - shift_angle_));
          tick_values_[i][2] = static_cast<uint8_t>(center_x + inner_radius * cos(i * angle_step - shift_angle_));
          tick_values_[i][3] = static_cast<uint8_t>(center_y + inner_radius * sin(i * angle_step - shift_angle_));
        }
      }
      for (int i = 0; i < 60; ++i) {
        buffer->line(tick_values_[i][0], tick_values_[i][1],
          tick_values_[i][2], tick_values_[i][3]);
      }
    }

    static void triangle_(Display* it, uint16_t x0, uint16_t y0, uint16_t x1,
        uint16_t y1, uint16_t x2, uint16_t y2, Color color) {
      it->line(x0, y0, x1, y1, color);
      it->line(x1, y1, x2, y2, color);
      it->line(x2, y2, x0, y0, color);
    }

    static void filled_triangle_(Display* it, uint16_t x0, uint16_t y0, uint16_t x1,
        uint16_t y1, uint16_t x2, uint16_t y2, Color color) {
      triangle_(it, x0, y0, x1, y1, x2, y2, color);
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
        it->horizontal_line(a, y0, b - a + 1, color);
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
        it->horizontal_line(a, y, b - a + 1, color);
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
        it->horizontal_line(a, y, b - a + 1, color);
      }
    }

    static void draw_minutes_hand_(Display *it, uint16_t x, uint16_t y,
        uint16_t radius, ESPTime time, Color color) {
      const double angle_step = M_PI / 30;
      uint8_t i = time.minute;
      // uint8_t i_prev = i == 0 ? 59 : i - 1;
      // uint8_t i_next = i == 59 ? 0 : i + 1;
      uint8_t x1 = static_cast<uint8_t>(x + (radius - 7) * cos(i * angle_step - angle_step - shift_angle_ + 0.04));
      uint8_t y1 = static_cast<uint8_t>(y + (radius - 7) * sin(i * angle_step - angle_step - shift_angle_ + 0.04));
      uint8_t x2 = static_cast<uint8_t>(x + (radius - 7) * cos(i * angle_step + angle_step - shift_angle_ - 0.04));
      uint8_t y2 = static_cast<uint8_t>(y + (radius - 7) * sin(i * angle_step + angle_step - shift_angle_ - 0.04));
      auto tick_values = tick_values_[i];
      uint8_t x3 = tick_values[2];
      uint8_t y3 = tick_values[3];
      filled_triangle_(it, x1, y1, x2, y2, x3, y3, color);
    }

    static void draw_hours_hand_(Display *it, uint16_t x, uint16_t y,
        uint16_t radius, ESPTime time, Color color) {
      const double angle_step = M_PI / 6;
      const double sixth_step = angle_step / 6;
      uint8_t i = time.hour > 12 ? time.hour - 12 : time.hour;
      uint8_t x1 = static_cast<uint8_t>(x + (radius + 7) * cos(i * angle_step - sixth_step - shift_angle_ + 0.04));
      uint8_t y1 = static_cast<uint8_t>(y + (radius + 7) * sin(i * angle_step - sixth_step - shift_angle_ + 0.04));
      uint8_t x2 = static_cast<uint8_t>(x + (radius + 7) * cos(i * angle_step + sixth_step - shift_angle_ - 0.04));
      uint8_t y2 = static_cast<uint8_t>(y + (radius + 7) * sin(i * angle_step + sixth_step - shift_angle_ - 0.04));
      uint8_t x3 = static_cast<uint8_t>(x + (radius) * cos(i * angle_step - shift_angle_));
      uint8_t y3 = static_cast<uint8_t>(y + (radius) * sin(i * angle_step - shift_angle_));
      filled_triangle_(it, x1, y1, x2, y2, x3, y3, color);
    }

    static void draw_seconds_hand_(Display *it, ESPTime time, Color color) {
      auto tick_values = tick_values_[time.second];
      it->line(tick_values[0], tick_values[1], tick_values[2], tick_values[3], color);
    }

  public:

    static void draw_watch_face(Display *it, uint16_t center_x,  uint16_t center_y,
        uint16_t outer_radius, uint16_t inner_radius, ESPTime time, Color face_color,
        Color hours_color, Color minutes_color, Color seconds_color) {
      if (!buffer_) {
        buffer_ = new BinaryImageBuffer(it->get_width(), it->get_height());
        draw_bezel_(buffer_, center_x, center_y, outer_radius, inner_radius);
      }
      buffer_->draw_buffer(it, face_color);
      if (time.is_valid()) {
        draw_hours_hand_(it, center_x, center_y, outer_radius, time, hours_color);
        draw_minutes_hand_(it, center_x, center_y, inner_radius, time, minutes_color);
        draw_seconds_hand_(it, time, seconds_color);
      }
    }

};

Clock::BinaryImageBuffer *Clock::buffer_ = NULL;
uint8_t Clock::tick_values_[60][4];