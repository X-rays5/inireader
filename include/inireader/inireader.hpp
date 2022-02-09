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
#include "conversion.hpp"

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
        if (!std::filesystem::exists(file)) {
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

    inline void Parse(std::filesystem::path& file) {
      if (!std::filesystem::exists(file)) {
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
      [[nodiscard]] T as() const {
        conversion::AsImpl<T> as;
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
        conversion::AsImpl<T> as;
        return as.is(value_);
      }

      template<typename T>
      IniValue& operator=(const T& value) {
        conversion::AsImpl<T> as;
        as.set(value, value_);
        return *this;
      }
    private:
      std::string value_;
    };

    struct IniSection {
      template<typename T>
      void Add(const std::string& key, const T& value) {
        conversion::AsImpl<T> as;
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
  };
}
#ifdef TRIM_STR
#undef TRIM_STR
#endif

#endif // INIREADER_HPP
