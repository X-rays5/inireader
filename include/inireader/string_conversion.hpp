#pragma once

#ifndef INIREADER_STRING_CONVERSION_HPP
#define INIREADER_STRING_CONVERSION_HPP

#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <cassert>
#include <limits>

namespace ini::conversion {
  namespace utility {
    // Reference: https://github.com/hermanzdosilovic/petiteutf8
    template <typename CharType = char16_t>
    std::string EncodeUTF(const std::basic_string<CharType>& s) {
      std::size_t capacity{0};
      for (const CharType& c : s) {
        if (c < 0x80) {
          capacity += 1;
        } else if (c < 0x800) {
          capacity += 2;
        } else if (c < 0x10000) {
          capacity += 3;
        } else {
          capacity += 4;
        }
      }

      std::string utf8;
      utf8.reserve(capacity);

      for (const CharType& c : s) {
        if (c < 0x80) {
          utf8 += static_cast<char>(c);
        } else if (c < 0x800) {
          utf8 += static_cast<char>(0xC0 | ((c >> 6) & 0x1F));
          utf8 += static_cast<char>(0x80 | (c & 0x3F));
        } else if (c < 0x10000) {
          utf8 += static_cast<char>(0xE0 | ((c >> 12) & 0xF));
          utf8 += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
          utf8 += static_cast<char>(0x80 | (c & 0x3F));
        } else {
          utf8 += static_cast<char>(0xF0 | ((c >> 18) & 0x7));
          utf8 += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
          utf8 += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
          utf8 += static_cast<char>(0x80 | (c & 0x3F));
        }
      }

      return utf8;
    }

    // Reference: https://github.com/hermanzdosilovic/petiteutf8
    template <typename CharType = char16_t>
    std::basic_string<CharType> DecodeUTF(const std::string& s) {
      std::size_t capacity{0};
      for (std::size_t i{0}; i < s.length(); ++capacity) {
        auto c{static_cast<CharType>(s[i])};
        if ((c & 0x80) == 0x0) {
          i += 1;
        } else if ((c & 0xE0) == 0xC0) {
          i += 2;
        } else if ((c & 0xF0) == 0xE0) {
          i += 3;
        } else {
          i += 4;
        }
      }

      std::basic_string<CharType> decoded;
      decoded.reserve(capacity);

      for (std::size_t i{0}; i < s.length();) {
        auto c{static_cast<CharType>(s[i])};
        if ((c & 0x80) == 0x0) {
          decoded += c;
          i += 1;
        } else if ((c & 0xE0) == 0xC0) {
          decoded += ((c & 0x1F) << 6) |
            (static_cast<CharType>(s[i + 1]) & 0x3F);
          i += 2;
        } else if ((c & 0xF0) == 0xE0) {
          decoded += ((c & 0xF) << 12) |
            ((static_cast<CharType>(s[i + 1]) & 0x3F) << 6) |
            ((static_cast<CharType>(s[i + 2]) & 0x3F));
          i += 3;
        } else {
          decoded += ((c & 0x7) << 18) |
            ((static_cast<CharType>(s[i + 1]) & 0x3F) << 12) |
            ((static_cast<CharType>(s[i + 2]) & 0x3F) << 6) |
            ((static_cast<CharType>(s[i + 3]) & 0x3F));
          i += 4;
        }
      }

      return decoded;
    }
  }

  // Conversion implementations a new as type can be added here
  template <typename T>
  struct AsImpl {};

  template <>
  struct AsImpl<std::string> {
    static bool is(const std::string& val) {
      return true;
    }

    static void get(const std::string& val, std::string& out) {
      out = val;
    }

    static void set(const std::string& val, std::string& out) {
      out = val;
    }
  };

  template <>
  struct AsImpl<std::string_view> {
    static bool is(const std::string& val) {
      return true;
    }

    static void get(const std::string& val, std::string_view& out) {
      out = std::string_view(val.data(), val.size());
    }

    static void set(const std::string_view& val, std::string& out) {
      out = val;
    }
  };

  template <>
  struct AsImpl<std::u16string> {
    static bool is(const std::string& val) {
      return true;
    }

    static void get(const std::string& val, std::u16string& out) {
      out = utility::DecodeUTF(val);
    }

    static void set(const std::u16string& val, std::string& out) {
      out = utility::EncodeUTF(val);
    }
  };

  template <>
  struct AsImpl<std::u32string> {
    static bool is(const std::string& val) {
      return true;
    }

    static void get(const std::string& val, std::u32string& out) {
      out = utility::DecodeUTF<char32_t>(val);
    }

    static void set(const std::u32string& val, std::string& out) {
      out = utility::EncodeUTF<char32_t>(val);
    }
  };
}

#endif // INIREADER_STRING_CONVERSION_HPP
