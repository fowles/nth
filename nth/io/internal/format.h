#ifndef NTH_IO_INTERNAL_FORMAT_H
#define NTH_IO_INTERNAL_FORMAT_H

#include <bit>
#include <cstdlib>
#include <optional>
#include <string_view>

namespace nth::internal_format {

inline constexpr bool FollowingCodeUnit(char c) {
  return (c & 0b1100'0000) == 0b1000'0000;
}

inline constexpr std::optional<uint32_t> ExtractNextUtf8Codepoint(
    std::string_view& text) {
  uint32_t value;
  if constexpr (std::is_signed_v<char>) {
    if (text[0]>= 0) {
      value = text[0];
    } else {
      value = static_cast<uint32_t>(static_cast<int32_t>(text[0]) + 128);
    }
  } else {
    value = text[0];
  }
  auto bytes     = std::countl_one(value);
  if (text.size() < static_cast<size_t>(bytes)) { return std::nullopt; }
  switch (bytes) {
    case 0: text.remove_prefix(1); break;
    case 2:
      if (not FollowingCodeUnit(text[1])) { std::abort(); }
      value = ((value & 0b0001'1111) << 6) | static_cast<uint32_t>(text[1]);
      break;
    case 3:
      if (not FollowingCodeUnit(text[1])) { std::abort(); }
      if (not FollowingCodeUnit(text[2])) { std::abort(); }
      value = ((value & 0b0000'1111) << 12) |
              ((0b0011'1111 & static_cast<uint32_t>(text[1])) << 6) |
              (0b0011'1111 & static_cast<uint32_t>(text[2]));
      break;
    case 4:
      if (not FollowingCodeUnit(text[1])) { std::abort(); }
      if (not FollowingCodeUnit(text[2])) { std::abort(); }
      if (not FollowingCodeUnit(text[3])) { std::abort(); }
      value = ((value & 0b0000'0111) << 18) |
              ((0b0011'1111 & static_cast<uint32_t>(text[1])) << 12) |
              ((0b0011'1111 & static_cast<uint32_t>(text[2])) << 6) |
              (0b0011'1111 & static_cast<uint32_t>(text[3]));
      break;
    default: return std::nullopt;
  }
  text.remove_prefix(bytes);
  return value;
}

inline constexpr void ValidateFormatString(std::string_view format_string) {
  int brace_depth = 0;
  while (not format_string.empty()) {
    if (auto codepoint = ExtractNextUtf8Codepoint(format_string)) {
      switch (*codepoint) {
        case '{': {
          if (brace_depth == 1) {
            std::abort();  // Nested braces.
          }
          ++brace_depth;
        } break;
        case '}': {
          if (brace_depth == 0) {
            std::abort();  // `}` without matching `{`.
          }
          --brace_depth;
        } break;
        default: {
          if (brace_depth == 1) {
            std::abort();  // `{` must be followed by `}`.
          }
        } break;
      }
    } else {
      std::abort();
    }
  }
  if (brace_depth != 0) {
    std::abort();  // Mismatched braces.
  }
}

// Represents the range of code units to be replaced.
struct PlaceholderRange {
  int start  = -1;
  int length = -1;
};

template <auto Fmt>
constexpr auto Replacements() {
  std::array<PlaceholderRange, Fmt.placeholders()> array;

  auto* array_ptr = array.data();
  char const* p   = Fmt.NthInternalFormatStringDataMember;
  for (size_t i = 0; i < Fmt.size(); ++i) {
    switch (p[i]) {
      case '{': array_ptr->start = i; break;
      case '}': {
        array_ptr->length = i - array_ptr->start + 1;
        ++array_ptr;
      } break;
    }
  }
  return array;
}

}  // namespace nth::internal_format

#endif  // NTH_IO_INTERNAL_FORMAT_H
