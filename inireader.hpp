//
// Created by X-ray on 5/18/2021.
//
#pragma once

#ifndef INIREADER_HPP
#define INIREADER_HPP
#include <string>
#include <regex>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <memory>
#include <utility>
namespace fs = std::filesystem;

namespace ini {
  class Parser {
  public:
    /**
     * @param wipe_on_parse wipe the ini file root_ when parsing a new document
     */
    explicit Parser(bool wipe_on_parse = true) {
      wipe_on_parse_ = wipe_on_parse;
      root_ = std::make_unique<IniRoot>();
    }

    /**
     * @param file path/contents of ini file
     * @param is_path is the file a path or contents of a ini file
     */
    inline void Parse(const std::string& file, bool is_path) {
      if (is_path) {
        if (!fs::exists(file)) {
          assert(!fs::exists(file));
          throw std::runtime_error("File not found");
        } else {
          std::ifstream ini_file(file);
          Parse(ini_file);
        }
      } else {
        auto lines = Split(file, '\n');
        ImplParse(lines);
      }
    }

    inline void Parse(fs::path& file) {
      if (!fs::exists(file)) {
        assert(!fs::exists(file));
        throw std::runtime_error("File not found");
      } else {
        std::ifstream ini_file(file);
        Parse(ini_file);
      }
    }

    inline void Parse(std::fstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    inline void Parse(std::ifstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    struct IniValue {
    public:
      template<typename T>
      T as() const {
        AsImpl<T> as;
        T res;
        if (as.is(value_)) {
          as.get(value_, res);
        } else {
          assert(as.is(value_));
        }
        return res;
      }

      template<typename T>
      [[nodiscard]] bool is() const {
        AsImpl<T> as;
        return as.is(value_);
      }

      template<typename T>
      IniValue& operator=(const T& value) {
        AsImpl<T> as;
        as.set(value, value_);
        return *this;
      }
    private:
      std::string value_;
    };

    struct IniSection {
      template<typename T>
      void Add(const std::string& key, const T& value) {
        AsImpl<T> as;
        std::string tmp;
        as.set(value, tmp);
        items_[key] = tmp;
      }

      bool Remove(const std::string& key) {
        if (items_.find(key) != items_.end()) {
          items_.erase(key);
          return true;
        } else {
          assert(items_.find(key) != items_.end());
          return false;
        }
      }

      void RemoveAll() {
        items_.clear();
      }

      [[nodiscard]] bool HasValue(const std::string& key) const {
        return items_.find(key) != items_.end();
      }

      [[nodiscard]] inline std::string Stringify() const {
        std::string res;
        for (auto& item : items_) {
          res += item.first + "=" + item.second.as<std::string>() + "\n";
        }
        return res;
      }

      IniValue& operator[](const std::string& key) {
        auto entry = items_.find(key);

        if (entry != items_.end()) {
          return entry->second;
        } else {
          assert(entry != items_.end());
          throw std::runtime_error("Section does not have a value with the key: "+key);
        }
      }

    private:
      std::unordered_map<std::string, IniValue> items_;
    };

    IniSection& AddSection(const std::string& section) {
      root_->sections[section] = IniSection();
      return root_->sections[section];
    }

    [[nodiscard]] bool HasSection(const std::string& section) const {
      return root_->sections.find(section) != root_->sections.end();
    }

    [[nodiscard]] std::uint32_t GetSectionCount() const {
      return root_->sections.size();
    }

    bool RemoveSection(const std::string& section) {
      if (HasSection(section)) {
        root_->sections.erase(section);
        return true;
      } else {
        assert(HasSection(section));
        return false;
      }
    }

    IniSection& GetRootSection() {
      return root_->root_section;
    }

    IniSection& operator[](const std::string& section) {
      auto entry = root_->sections.find(section);

      if (entry != root_->sections.end()) {
        return entry->second;
      } else {
        assert(entry != root_->sections.end());
        throw std::runtime_error("Section: "+section+" does not exist");
      }
    }

    [[nodiscard]] inline std::string Stringify() const {
      std::stringstream ss;

      ss << root_->root_section.Stringify();
      for (const auto& section : root_->sections) {
        ss << "[" << section.first << "]" << std::endl;
        ss << section.second.Stringify();
      }
      return ss.str();
    }

  private:
    struct IniRoot {
      IniSection root_section;
      std::unordered_map<std::string, IniSection> sections;
    };

    std::string current_section_;
    std::unique_ptr<IniRoot> root_;
    bool wipe_on_parse_;

  private:
#define TRIM_STR(str, c) TrimR(Trim(str, c), c)
    void ImplParse(std::vector<std::string>& lines) {
      if (wipe_on_parse_) {
        current_section_.clear();
        root_ = std::make_unique<IniRoot>();
      }

      if (lines.empty()) {return;}

      for (auto&& line : lines) {
        if (line.empty()) continue;

        RemoveComment(line);
        if (line.empty()) continue;

        auto item = GetItem(line);
        if (!item.first.empty() && !item.second.empty()) {
          if (current_section_.empty()) {
            GetRootSection().Add(item.first, item.second);
          } else if (HasSection(current_section_)) {
            (*this)[current_section_].Add(item.first, item.second);
          } else {
            assert(HasSection(current_section_));
            throw std::runtime_error("Section does not have a value with the key: "+current_section_);
          }
        }

        if (auto section = GetSection(line); !section.empty()) {
          current_section_ = section;
          AddSection(current_section_);
          continue;
        }

        // if it gets to here it's an empty line
      }
    }

    static inline void RemoveComment(std::string& line) {
      std::smatch match;
      if (std::regex_search(line, match, std::regex(R"((^\;.*)|(^\#.*)|(.*[^\\];)|(.*[^\\]#))"))) {
        // full string match
        if (match[0].str() == line) {
          line.clear();
        } else {
          line.erase(match[0].str().size() - 1, line.back());
        }
      }
    }

    static inline std::pair<std::string, std::string> GetItem(std::string& line) {
      std::smatch match;
      if (std::regex_match(line, match, std::regex(R"((.*)= ?(.*))"))) {
        return std::make_pair(TRIM_STR(match[1].str(), ' '), TRIM_STR(match[2].str(), ' '));
      }
      return {};
    }

    static inline std::string GetSection(std::string& line) {
      std::smatch match;
      if (std::regex_match(line, match, std::regex(R"(\[(.*)\])"))) {
        return TRIM_STR(match[1].str(), ' ');
      }
      return {};
    }

    template<typename T>
    static inline std::vector<std::string> ReadFile(T& stream) {
      std::string tmp;
      std::vector<std::string> lines;
      while (std::getline(stream, tmp)) {
        lines.emplace_back(tmp);
      }
      return lines;
    }

    static inline std::vector<std::string> Split(const std::string& str, char c) {
      std::vector<std::string> res;
      std::string tmp;
      for (auto& ch : str) {
        if (ch == c) {
          res.emplace_back(tmp);
          tmp.clear();
        } else {
          tmp.push_back(ch);
        }
      }
      if (!tmp.empty()) {
        res.emplace_back(tmp);
      }
      return res;
    }

    static inline std::string Trim(std::string str, char trim_c) {
      while(str.front() == trim_c) {
        str.erase(0, 1);
      }
      return str;
    }

    static inline std::string TrimR(std::string str, char trim_c) {
      while (str.back() == trim_c) {
        str.pop_back();
      }
      return str;
    }

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
    struct AsImpl<std::float_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,19}(\.[0-9]{1,19})?$)"));
      }

      static inline void get(const std::string& val, std::float_t& out) {
        out = std::stof(val);
      }

      static inline void set(std::float_t val, std::string& out) {
        out = std::to_string(val);
      }
    };

    template<>
    struct AsImpl<std::double_t> {
      static inline bool is(const std::string& val) {
        return std::regex_match(val, std::regex(R"(^-?[0-9]{1,19}(\.[0-9]{1,19})?$)"));
      }

      static inline void get(const std::string& val, std::double_t& out) {
        out = std::stod(val);
      }

      static inline void set(std::double_t val, std::string& out) {
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
  };
}
#ifdef TRIM_STR
#undef TRIM_STR
#endif

#endif // INIREADER_HPP
