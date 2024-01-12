//
// Created by X-ray on 2/9/2022.
//

#pragma once

#ifndef TEST_INIREADER_CONVERSION_HPP
#define TEST_INIREADER_CONVERSION_HPP
#include <string>
#include <algorithm>

#ifdef min
#ifdef max
#undef min
#undef max
#endif
#endif

namespace ini::conversion {
  namespace utility {
    inline bool IsHex(const std::string& str) {
      if (str.size() < 3) {
        return false;
      }

      if (str[0] != '0' || str[1] != 'x') {
        return false;
      }

      for (size_t i = 2; i < str.size(); i++) {
        if (!isxdigit(str[i])) {
          return false;
        }
      }

      return true;
    }

    inline std::int64_t HexToInt64(const std::string& str) {
      return std::stoll(str, nullptr, 16);
    }

    inline std::uint64_t HexToUInt64(const std::string& str) {
      return std::stoull(str, nullptr, 16);
    }

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

  template <>
  struct AsImpl<bool> {
    static bool is(std::string val) {
      std::transform(val.begin(), val.end(), val.begin(), [](const char c) {
        return static_cast<char>(::toupper(c));
      });

      return val == "TRUE" || val == "YES" || val == "ON" || val == "FALSE" || val == "NO" || val == "OFF" || val == "1" || val == "0";
    }

    static void get(std::string val, bool& out) {
      if (!is(val)) {
        return;
      }

      std::transform(val.begin(), val.end(), val.begin(), [](const char c) {
        return static_cast<char>(::toupper(c));
      });

      if (val == "TRUE" || val == "YES" || val == "ON" || val == "1") {
        out = true;
      } else if (val == "FALSE" || val == "NO" || val == "OFF" || val == "0") {
        out = false;
      }
    }

    static void set(bool val, std::string& out) {
      out = val ? "true" : "false";
    }
  };

  template <>
  struct AsImpl<std::int8_t> {
    static bool is(const std::string& val) {
      try {
        const std::int64_t num = utility::IsHex(val) ? utility::HexToInt64(val) : std::stoi(val);
        return num >= std::numeric_limits<std::int8_t>::min() && num <= std::numeric_limits<std::int8_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, std::int8_t& out) {
      out = utility::IsHex(val) ? static_cast<std::int8_t>(utility::HexToInt64(val)) : static_cast<std::int8_t>(std::stoi(val));
    }

    static void set(std::int8_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<std::uint8_t> {
    static bool is(const std::string& val) {
      try {
        const std::int64_t num = utility::IsHex(val) ? utility::HexToInt64(val) : std::stoi(val);
        return num >= 0 && num <= std::numeric_limits<std::uint8_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, int& out) {
      out = utility::IsHex(val) ? static_cast<std::uint8_t>(utility::HexToInt64(val)) : static_cast<std::uint8_t>(std::stoi(val));
    }

    static void set(std::uint8_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<std::int16_t> {
    static bool is(const std::string& val) {
      try {
        const std::int64_t num = utility::IsHex(val) ? utility::HexToInt64(val) : std::stoi(val);
        return num >= std::numeric_limits<std::int16_t>::min() && num <= std::numeric_limits<std::int16_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, std::int16_t& out) {
      out = utility::IsHex(val) ? static_cast<std::int16_t>(utility::HexToInt64(val)) : static_cast<std::int16_t>(std::stoi(val));
    }

    static void set(std::int16_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<std::uint16_t> {
    static bool is(const std::string& val) {
      try {
        const std::int64_t num = utility::IsHex(val) ? utility::HexToInt64(val) : std::stoi(val);
        return num >= std::numeric_limits<std::uint16_t>::min() && num <= std::numeric_limits<std::uint16_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, std::uint16_t& out) {
      out = utility::IsHex(val) ? static_cast<std::uint16_t>(utility::HexToInt64(val)) : static_cast<std::uint16_t>(std::stoi(val));
    }

    static void set(std::uint16_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<std::int32_t> {
    static bool is(const std::string& val) {
      try {
        const std::int64_t num = utility::IsHex(val) ? utility::HexToInt64(val) : std::stoi(val);
        return num >= std::numeric_limits<std::int32_t>::min() && num <= std::numeric_limits<std::int32_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, std::int32_t& out) {
      out = utility::IsHex(val) ? static_cast<std::int32_t>(utility::HexToInt64(val)) : std::stoi(val);
    }

    static void set(std::int32_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<std::uint32_t> {
    static bool is(const std::string& val) {
      try {
        const std::int64_t num = utility::IsHex(val) ? utility::HexToInt64(val) : std::stoi(val);
        return num >= std::numeric_limits<std::uint32_t>::min() && num <= std::numeric_limits<std::uint32_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, std::uint32_t& out) {
      out = utility::IsHex(val) ? static_cast<std::uint32_t>(utility::HexToInt64(val)) : static_cast<std::uint32_t>(std::stoi(val));
    }

    static void set(std::uint32_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<std::int64_t> {
    static bool is(const std::string& val) {
      try {
        const std::int64_t num = utility::IsHex(val) ? utility::HexToInt64(val) : std::stoi(val);
        return num >= std::numeric_limits<std::int64_t>::min() && num <= std::numeric_limits<std::int64_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, std::int64_t& out) {
      out = utility::IsHex(val) ? utility::HexToInt64(val) : static_cast<std::int64_t>(std::stoi(val));
    }

    static void set(std::int64_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<std::uint64_t> {
    static bool is(const std::string& val) {
      try {
        const std::uint64_t num = utility::IsHex(val) ? utility::HexToUInt64(val) : std::stoull(val);
        return num <= std::numeric_limits<std::uint64_t>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, std::uint64_t& out) {
      out = utility::IsHex(val) ? utility::HexToUInt64(val) : std::stoull(val);
    }

    static void set(std::uint64_t val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<float> {
    static bool is(const std::string& val) {
      try {
        return std::stof(val) >= -std::numeric_limits<float>::max() && std::stof(val) <= std::numeric_limits<float>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, float& out) {
      out = std::stof(val);
    }

    static void set(float val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<double> {
    static bool is(const std::string& val) {
      try {
        return std::stod(val) >= -std::numeric_limits<double>::max() && std::stod(val) <= std::numeric_limits<double>::max();
      } catch (...) {
        return false;
      }
    }

    static void get(const std::string& val, double& out) {
      out = std::stod(val);
    }

    static void set(double val, std::string& out) {
      out = std::to_string(val);
    }
  };

  template <>
  struct AsImpl<const char*> {
    static bool is(const std::string& val) {
      return true;
    }

    static void get(const std::string& val, const char*& out) {
      out = val.c_str();
    }

    static void set(const char* val, std::string& out) {
      out = val;
    }
  };

  template <>
  struct AsImpl<char*> {
    static bool is(const std::string& val) {
      return true;
    }

    static void get(const std::string& val, char* out) {
      out = const_cast<char*>(val.c_str());
    }

    static void set(char* val, std::string& out) {
      out = val;
    }
  };

  template <size_t N>
  struct AsImpl<char[N]> {
    static void set(const char* val, std::string& out) {
      out = val;
    }
  };
}
#endif //TEST_INIREADER_CONVERSION_HPP
