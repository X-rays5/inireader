#pragma once

#ifndef INIREADER_NUMBER_CONVERSION_HPP
#define INIREADER_NUMBER_CONVERSION_HPP

#include <string>
#include <algorithm>
#include <cctype>
#include <cassert>
#include <limits>

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
  }

  template <typename T>
  struct AsImpl {};

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
}

#endif // INIREADER_NUMBER_CONVERSION_HPP
