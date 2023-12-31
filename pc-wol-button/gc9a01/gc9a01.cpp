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
0x0, 0, 
0xef, 0, 
0xeb, 1, 0x14,
0xfe, 0, 
0xef, 0, 
0xeb, 1, 0x14,
0x84, 1, 0x40,
0x85, 1, 0xff,
0x86, 1, 0xff,
0x87, 1, 0xff,
0x88, 1, 0xa,
0x89, 1, 0x21,
0x8a, 1, 0x0,
0x8b, 1, 0x80,
0x8c, 1, 0x1,
0x8d, 1, 0x1,
0x8e, 1, 0xff,
0x8f, 1, 0xff,
0xb6, 2, 0x0, 0x20,
0x36, 1, 0x8,
0x3a, 1, 0x5,
0x90, 4, 0x8, 0x8, 0x8, 0x8,
0xbd, 1, 0x6,
0xbc, 1, 0x0,
0xff, 3, 0x60, 0x1, 0x4,
0xc3, 1, 0x13,
0xc4, 1, 0x13,
0xc9, 1, 0x22,
0xbe, 1, 0x11,
0xe1, 2, 0x10, 0xe,
0xdf, 3, 0x21, 0xc, 0x2,
0xf0, 6, 0x45, 0x9, 0x8, 0x8, 0x26, 0x2a,
0xf1, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6f,
0xf2, 6, 0x45, 0x9, 0x8, 0x8, 0x26, 0x2a,
0xf3, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6f,
0xed, 2, 0x1b, 0xb,
0xae, 1, 0x77,
0xcd, 1, 0x63,
0x70, 9, 0x7, 0x7, 0x4, 0xe, 0xf, 0x9, 0x7, 0x8, 0x3,
0xe8, 1, 0x34,
0x62, 12, 0x18, 0xd, 0x71, 0xed, 0x70, 0x70, 0x18, 0xf, 0x71, 0xef, 0x70, 0x70,
0x63, 12, 0x18, 0x11, 0x71, 0xf1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xf3, 0x70, 0x70,
0x64, 7, 0x28, 0x29, 0xf1, 0x1, 0xf1, 0x0, 0x7,
0x66, 10, 0x3c, 0x0, 0xcd, 0x67, 0x45, 0x45, 0x10, 0x0, 0x0, 0x0,
0x67, 10, 0x0, 0x3c, 0x0, 0x0, 0x0, 0x1, 0x54, 0x10, 0x32, 0x98,
0x74, 7, 0x10, 0x85, 0x80, 0x0, 0x0, 0x4e, 0x0,
0x98, 2, 0x3e, 0x7,
0x35, 0, 
0x21, 0, 
0x11, 0, 
0x29, 0, 
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
  return size_t(this->get_width_internal()) * size_t(this->get_height_internal()) * 2;
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

  if (this->eightbitcolor_) {
    for (size_t line = 0; line < this->get_buffer_length(); line = line + this->get_width_internal()) {
      for (int index = 0; index < this->get_width_internal(); ++index) {
        auto color332 = display::ColorUtil::to_color(this->buffer_[index + line], display::ColorOrder::COLOR_ORDER_BGR,
          display::ColorBitness::COLOR_BITNESS_332, true);

        auto color = display::ColorUtil::color_to_565(color332);

        this->write_byte((color >> 8) & 0xff);
        this->write_byte(color & 0xff);
      }
    }
  } else {
    this->write_array(this->buffer_, this->get_buffer_length());
  }
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
