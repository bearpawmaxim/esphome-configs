#include "gc9a01.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

#include "gc9a01_defines.h"

namespace esphome {
namespace gc9a01 {

static const uint8_t MADCTL_MY = 0x80;   ///< Bottom to top
static const uint8_t MADCTL_MX = 0x40;   ///< Right to left
static const uint8_t MADCTL_MV = 0x20;   ///< Reverse Mode
static const uint8_t MADCTL_ML = 0x10;   ///< LCD refresh Bottom to top
static const uint8_t MADCTL_RGB = 0x00;  ///< Red-Green-Blue pixel order
static const uint8_t MADCTL_BGR = 0x08;  ///< Blue-Green-Red pixel order
static const uint8_t MADCTL_MH = 0x04;   ///< LCD refresh right to left

// clang-format off
static const uint8_t PROGMEM INITCMD[] = {
  0xEF, 0x0,
  0xEB, 0x1, 0x14,
  0xFE, 0x0,
  0xEF, 0x0,
  0xEB, 0x1, 0x14,
  0x84, 0x1, 0x40,
  0x85, 0x1, 0xFF,
  0x86, 0x1, 0xFF,
  0x87, 0x1, 0xFF,
  0x88, 0x1, 0x0A,
  0x89, 0x1, 0x21,
  0x8A, 0x1, 0x00,
  0x8B, 0x1, 0x80,
  0x8C, 0x1, 0x01,
  0x8D, 0x1, 0x01,
  0x8E, 0x1, 0xFF,
  0x8F, 0x1, 0xFF,
  0xB6, 0x2, 0x00, 0x20,
  0x3A, 0x1, 0x05,
  0x90, 0x4, 0x08, 0x08, 0x08, 0x08,
  0xBD, 0x1, 0x06,
  0xBC, 0x1, 0x00,
  0xFF, 0x3, 0x60, 0x01, 0x04,
  0xC3, 0x1, 0x13,
  0xC4, 0x1, 0x13,
  0xC9, 0x1, 0x22,
  0xBE, 0x1, 0x11,
  0xE1, 0x2, 0x10, 0x0E,
  0xDF, 0x3, 0x21, 0x0c, 0x02,
  0xF0, 0x6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
  0xF1, 0x6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
  0xF2, 0x6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
  0xF3, 0x6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
  0xED, 0x2, 0x1B, 0x0B,
  0xAE, 0x1, 0x77,
  0xCD, 0x1, 0x63,
  0x70, 0x9, 0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03,
  0xE8, 0x1, 0x34,
  0x62, 0x0C, 0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70,
  0x63, 0x0C, 0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70,
  0x64, 0x7, 0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07,
  0x66, 0x0A, 0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00,
  0x67, 0x0A, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98,
  0x74, 0x7, 0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00,
  0x98, 0x2, 0x3e, 0x07,
  0x35, 0x0,
  0x21, 0x0,
  0x11, 0x80,
  0x29, 0x14,
  0x00                  // End of list
};
// clang-format on

static const char *const TAG = "gc9a01";

GC9A01::GC9A01(int width, int height, int colstart, int rowstart, bool eightbitcolor)
    : colstart_(colstart), rowstart_(rowstart), eightbitcolor_(eightbitcolor), width_(width), height_(height) {}

void GC9A01::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GC9A01...");
  this->spi_setup();

  this->dc_pin_->setup();  // OUTPUT
  this->cs_->setup();      // OUTPUT

  this->dc_pin_->digital_write(true);
  this->cs_->digital_write(true);

  this->init_reset_();
  delay(100);  // NOLINT

  ESP_LOGD(TAG, "  START");
  dump_config();
  ESP_LOGD(TAG, "  END");

  display_init_(INITCMD);

  this->init_internal_(this->get_buffer_length());
  memset(this->buffer_, 0x00, this->get_buffer_length());
}

void GC9A01::update() {
  this->do_update_();
  this->write_display_data_();
}

int GC9A01::get_height_internal() { return height_; }

int GC9A01::get_width_internal() { return width_; }

size_t GC9A01::get_buffer_length() {
  if (this->eightbitcolor_) {
    return size_t(this->get_width_internal()) * size_t(this->get_height_internal());
  }
  return size_t(this->get_width_internal() * this->get_height_internal() * 2);
}

void HOT GC9A01::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0)
    return;

  if (this->eightbitcolor_) {
    const uint8_t color332 = display::ColorUtil::color_to_332(color);
    uint16_t pos = (x + y * this->get_width_internal());
    this->buffer_[pos] = color332;
  } else {
    const uint16_t color565 = display::ColorUtil::color_to_565(color);
    uint16_t pos = (x + y * this->get_width_internal()) * 2;
    this->buffer_[pos++] = (color565 >> 8) & 0xff;
    this->buffer_[pos] = color565 & 0xff;
  }
}

void GC9A01::init_reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
    delay(1);
    // Trigger Reset
    this->reset_pin_->digital_write(false);
    delay(10);
    // Wake up
    this->reset_pin_->digital_write(true);
  }
}

void GC9A01::display_init_(const uint8_t *addr) {
  uint8_t cmd, x, num_args;
  while ((cmd = progmem_read_byte(addr++)) > 0) {
    x = progmem_read_byte(addr++);
    num_args = x & 0x7F;
    this->sendcommand_(cmd, addr, num_args);
    addr += num_args;
    if (x & 0x80) {
      delay(150);  // NOLINT
    }
  }
}

void GC9A01::dump_config() {
  LOG_DISPLAY("", "GC9A01", this);
  LOG_PIN("  CS Pin: ", this->cs_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGD(TAG, "  Buffer Size: %zu", this->get_buffer_length());
  ESP_LOGD(TAG, "  Height: %d", this->height_);
  ESP_LOGD(TAG, "  Width: %d", this->width_);
  ESP_LOGD(TAG, "  OffsetX: %d", this->colstart_);
  ESP_LOGD(TAG, "  OffsetY: %d", this->rowstart_);
  LOG_UPDATE_INTERVAL(this);
}

void HOT GC9A01::writecommand_(uint8_t value) {
  this->enable();
  this->dc_pin_->digital_write(false);
  this->write_byte(value);
  this->dc_pin_->digital_write(true);
  this->disable();
}

void HOT GC9A01::writedata_(uint8_t value) {
  this->dc_pin_->digital_write(true);
  this->enable();
  this->write_byte(value);
  this->disable();
}

void HOT GC9A01::sendcommand_(uint8_t cmd, const uint8_t *data_bytes, uint8_t num_data_bytes) {
  this->writecommand_(cmd);
  this->senddata_(data_bytes, num_data_bytes);
}

void HOT GC9A01::senddata_(const uint8_t *data_bytes, uint8_t num_data_bytes) {
  this->dc_pin_->digital_write(true);  // pull DC high to indicate data
  this->cs_->digital_write(false);
  this->enable();
  for (uint8_t i = 0; i < num_data_bytes; i++) {
    this->write_byte(progmem_read_byte(data_bytes++));  // write byte - SPI library
  }
  this->cs_->digital_write(true);
  this->disable();
}

void HOT GC9A01::write_display_data_() {
  uint16_t offsetx = colstart_;
  uint16_t offsety = rowstart_;

  uint16_t x1 = offsetx;
  uint16_t x2 = x1 + get_width_internal() - 1;
  uint16_t y1 = offsety;
  uint16_t y2 = y1 + get_height_internal() - 1;

  this->enable();

  // set column(x) address
  this->dc_pin_->digital_write(false);
  this->write_byte(GC9A01_CASET);
  this->dc_pin_->digital_write(true);
  this->spi_master_write_addr_(x1, x2);

  // set Page(y) address
  this->dc_pin_->digital_write(false);
  this->write_byte(GC9A01_PASET);
  this->dc_pin_->digital_write(true);
  this->spi_master_write_addr_(y1, y2);

  //  Memory Write
  this->dc_pin_->digital_write(false);
  this->write_byte(GC9A01_RAMWR);
  this->dc_pin_->digital_write(true);

  uint16_t b_written = 0;

  if (this->eightbitcolor_) {
    for (size_t line = 0; line < this->get_buffer_length(); line = line + this->get_width_internal()) {
      for (int index = 0; index < this->get_width_internal(); ++index) {
        auto color332 = display::ColorUtil::to_color(this->buffer_[index + line], display::ColorOrder::COLOR_ORDER_BGR,
          display::ColorBitness::COLOR_BITNESS_332, true);

        auto color = display::ColorUtil::color_to_565(color332);

        this->write_byte((color >> 8) & 0xff);
        this->write_byte(color & 0xff);
        b_written += 2;
      }
    }
  } else {
    this->write_array(this->buffer_, this->get_buffer_length());
    b_written = this->get_buffer_length();
  }

  ESP_LOGW(TAG, "bytes written %d", b_written);

  this->disable();
}

void GC9A01::spi_master_write_addr_(uint16_t addr1, uint16_t addr2) {
  static uint8_t byte[4];
  byte[0] = addr1 >> 8;
  byte[1] = addr1 & 0xFF;
  byte[2] = addr2 >> 8;
  byte[3] = addr2 & 0xFF;

  this->dc_pin_->digital_write(true);
  this->write_array(byte, 4);
}

void GC9A01::spi_master_write_color_(uint16_t color, uint16_t size) {
  static uint8_t byte[1024];
  int index = 0;
  for (int i = 0; i < size; i++) {
    byte[index++] = (color >> 8) & 0xFF;
    byte[index++] = color & 0xFF;
  }

  this->dc_pin_->digital_write(true);
  return write_array(byte, size * 2);
}

}  // namespace gc9a01
}  // namespace esphome
