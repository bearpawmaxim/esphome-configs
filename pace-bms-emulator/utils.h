#pragma once

using namespace esphome;

uint32_t get_ascii_char_(const std::string &input, size_t offset) {
  uint32_t chr = static_cast<uint32_t>(input[offset]);
  return chr > 127 ? 0x5F : chr;
}

template <typename T>
  T pack_chars_impl(const std::string &input, size_t offset, size_t num_chars) {
    T data = 0;
    uint8_t len = input.length();
    for (size_t i = 0; i < num_chars; ++i) {
      if (offset + i < len) {
        data |= get_ascii_char_(input, offset + i) << (8 * (num_chars - 1 - i));
      }
    }
    return data;
  }

// Packs 2 characters into a 16-bit integer
uint16_t pack_chars16(const std::string &input, size_t offset) {
  return pack_chars_impl<uint16_t>(input, offset, 2);
}

// Packs 4 characters into a 32-bit integer
uint32_t pack_chars32(const std::string &input, size_t offset) {
  return pack_chars_impl<uint32_t>(input, offset, 4);
}

// Packs 16 switch values into a 16-bit integer
uint16_t pack_flags16(const Switch* flags[], size_t num_flags) {
  uint16_t result = 0;
  for (int i = 0; i < 16; i++) {
    if (i >= num_flags) {
      return result;
    }
    result |= (flags[i]->state ? 1 : 0) << i;
  }
  return result;
}
