#pragma once

uint32_t get_string_chars2(std::string str, int start_index) {
  uint32_t result = 0;

  if (start_index < 0 || start_index >= str.length()) {
    return result;
  }

  size_t num_chars = std::min(4, static_cast<int>(str.length()) - start_index);

  for (size_t i = 0; i < num_chars; ++i) {
    unsigned char ch = static_cast<unsigned char>(str[start_index + i]);
    if (ch > 127) {
      ch = 0;
    }
    result |= (static_cast<uint32_t>(ch) << (i * 8));
  }

  return result;
}

uint32_t get_string_chars(const std::string &input, size_t index) {
  uint32_t data = 0x00000000; // Initialize the data to 0

  // Combine up to four characters into a single uint32_t
  if (index < input.length()) {
    data |= static_cast<uint32_t>(input[index]) << 24;  // First char in the highest byte
  }
  if (index + 1 < input.length()) {
    data |= static_cast<uint32_t>(input[index + 1]) << 16; // Second char in the second highest byte
  }
  if (index + 2 < input.length()) {
    data |= static_cast<uint32_t>(input[index + 2]) << 8;  // Third char in the second lowest byte
  }
  if (index + 3 < input.length()) {
    data |= static_cast<uint32_t>(input[index + 3]);       // Fourth char in the lowest byte
  }

  return data;
}