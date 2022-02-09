//
// Created by X-ray on 2/9/2022.
//

#pragma once

#ifndef TEST_INIREADER_CONVERSION_HPP
#define TEST_INIREADER_CONVERSION_HPP
#include <string>
#include <algorithm>
#include <regex>

namespace ini {
  namespace conversion {
    // Conversion implementations a new as type can be added here
    template<typename T>
    struct AsImpl{};

    template<>
    struct AsImpl<std::string> {
      static inline bool is(const std::string& val) {
        return true;
      }

      static inline void get(const std::string& val, std::string& out) {
        out = val;
      }

      static inline void set(const std::string& val, std::string& out) {
        out = val;
      }
    };

    template<>
    struct AsImpl<bool> {
      static inline bool is(const std::string& val) {
        return val == "TRUE" || val == "YES" || val == "ON" || val == "FALSE" || val == "NO" || val == "OFF";
      }

      static inline void get(std::string val, bool& out) {
        std::transform(val.begin(), val.end(), val.begin(), [](const char c){
          return static_cast<char>(::toupper(c));
        });

        if (val == "TRUE" || val == "YES" || val == "ON") {
          out = true;
        } else if (val == "FALSE" || val == "NO" || val == "OFF") {
          out = false;
        } else {
          throw std::runtime_error("Invalid boolean value: " + val);
        }
      }

      static inline void set(bool val, std::string& out) {
        out = val ? "true" : "false";
      }
    };

    template<>
    struct AsImpl<std::int8_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,3}$)"));
      }

      static inline void get(const std::string& val, std::int8_t& out) {
        if (val.size() != 1) {
          throw std::runtime_error("Invalid char value: " + val);
        }
        out = val[0];
      }

      static inline void set(std::int8_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::uint8_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^[0-9]{1,3}$)"));
      }

      static inline void get(const std::string& val, int& out) {
        if (val.size() != 1) {
          throw std::runtime_error("Invalid char value: " + val);
        }
        out = val[0];
      }

      static inline void set(std::uint8_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::int16_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,5}$)"));
      }

      static inline void get(const std::string& val, std::int16_t& out) {
        std::int32_t tmp = std::stoi(val);
        if (tmp < std::numeric_limits<std::int16_t>::min() || tmp > std::numeric_limits<std::int16_t>::max()) {
          throw std::runtime_error("Invalid int16_t value: " + val);
        }
        out = static_cast<std::int16_t>(tmp);
      }

      static inline void set(std::int16_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::uint16_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^[0-9]{1,5}$)"));
      }

      static inline void get(const std::string& val, std::uint16_t& out) {
        std::uint32_t tmp = std::stoi(val);
        if (tmp > std::numeric_limits<std::uint16_t>::max()) {
          throw std::runtime_error("Invalid uint16_t value: " + val);
        }
        out = static_cast<std::uint16_t>(tmp);
      }

      static inline void set(std::uint16_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::int32_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,10}$)"));
      }

      static inline void get(const std::string& val, std::int32_t& out) {
        out = std::stoi(val);
      }

      static inline void set(std::int32_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::uint32_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^[0-9]{1,10}$)"));
      }

      static inline void get(const std::string& val, std::uint32_t& out) {
        out = std::stoi(val);
      }

      static inline void set(std::uint32_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::int64_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,19}$)"));
      }

      static inline void get(const std::string& val, std::int64_t& out) {
        out = std::stoll(val);
      }

      static inline void set(std::int64_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::uint64_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^[0-9]{1,19}$)"));
      }

      static inline void get(const std::string& val, std::uint64_t& out) {
        out = std::stoll(val);
      }

      static inline void set(std::uint64_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<float> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,19}(\.[0-9]{1,19})?$)"));
      }

      static inline void get(const std::string& val, float& out) {
        out = std::stof(val);
      }

      static inline void set(float val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<double> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,19}(\.[0-9]{1,19})?$)"));
      }

      static inline void get(const std::string& val, double& out) {
        out = std::stod(val);
      }

      static inline void set(double val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<const char*> {
      static inline bool is(const std::string& val) {
        return true;
      }

      static inline void get(const std::string& val, const char*& out) {
        out = val.c_str();
      }

      static inline void set(const char* val, std::string& out) {
        out = val;
      }
    };

    template<>
    struct AsImpl<char*> {
      static inline bool is(const std::string& val) {
        return true;
      }

      static inline void get(const std::string& val, char* out) {
        out = (char*)val.c_str();
      }

      static inline void set(char* val, std::string& out) {
        out = val;
      }
    };

    template<size_t N>
    struct AsImpl<char[N]> {
      static inline void set(const char* val, std::string& out) {
        out = val;
      }
    };
  }
}
#endif //TEST_INIREADER_CONVERSION_HPP
