//
// Created by X-ray on 5/18/2021.
//
#pragma once

#ifndef INIREADER_HPP
#define INIREADER_HPP
#include <string>
#include <string_view>
#include <regex>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <memory>
#include <utility>
#include <sstream>
#include <unordered_map>
#include "conversion.hpp"

namespace ini {
  class Parser {
  public:
    /**
     * @param wipe_on_parse wipe the ini file root_ when parsing a new document
     */
    explicit Parser(const bool wipe_on_parse = true) {
      wipe_on_parse_ = wipe_on_parse;
      root_ = std::make_unique<IniRoot>();
    }

    /**
     * @param file path/contents of ini file
     * @param is_path is the file a path or contents of a ini file
     */
    void Parse(const std::string& file, const bool is_path) {
      if (is_path) {
        CheckValidFile(file);

        std::ifstream ini_file(file);
        Parse(ini_file);
      } else {
        auto lines = Split(file);
        ImplParse(lines);
      }
    }

    /**
     * @param file path to a ini file
     */
    void Parse(const std::filesystem::path& file) {
      CheckValidFile(file);

      std::ifstream ini_file(file);
      Parse(ini_file);
    }

    /**
     * @param file a open stream of a ini file
     */
    void Parse(std::fstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    /**
     * @param file a open stream of a ini file
     */
    void Parse(std::ifstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    struct IniValue {
    public:
      /**
       * @tparam T return type of the value
       * @return get value as T
       */
      template <typename T>
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

      /**
       * @tparam T type of the value
       * @return check if value is of type T
       */
      template <typename T>
      [[nodiscard]] bool is() const {
        conversion::AsImpl<T> as;
        return as.is(value_);
      }

      /**
       * @tparam T type of value to assign
       */
      template <typename T>
      IniValue& operator=(const T& value) {
        conversion::AsImpl<T> as;
        as.set(value, value_);
        return *this;
      }

    private:
      std::string value_;
    };

    struct IniSection {
      /**
       * @tparam T type of the value to add
       * @param key key of the value
       * @param value value to add
       */
      template <typename T>
      void Add(const std::string& key, const T& value) {
        conversion::AsImpl<T> as;
        std::string tmp;
        as.set(value, tmp);
        items_[key] = tmp;
      }

      /**
       * @param key key of value to remove
       * @return success
       */
      bool Remove(const std::string& key) {
        if (items_.find(key) != items_.end()) {
          items_.erase(key);
          return true;
        }

        assert(items_.find(key) != items_.end());
        return false;
      }

      /**
       * @note This will remove all the values in the section
       */
      void RemoveAll() {
        items_.clear();
      }

      /**
       * @param key check if the key exists in the section
       * @return true if the key exists
       */
      [[nodiscard]] bool HasValue(const std::string& key) const {
        return items_.find(key) != items_.end();
      }

      /**
       * @return a stringified version of the section
       */
      [[nodiscard]] std::string Stringify() const {
        std::stringstream res;
        for (auto& item : items_) {
          res << item.first << "=" << item.second.as<std::string>() << "\n";
        }

        return res.str();
      }

      /**
       * @return Amount of members in the section
       */
      [[nodiscard]] size_t Size() const {
        return items_.size();
      }

      /**
       * @param key key of the value to get
       * @return a reference to the key
       */
      [[nodiscard]] IniValue& operator[](const std::string& key) {
        const auto entry = items_.find(key);

        if (entry != items_.end()) {
          return entry->second;
        }

        assert(entry != items_.end());
        throw std::runtime_error("Section does not have a value with the key: " + key);
      }

      [[nodiscard]] std::unordered_map<std::string, IniValue>::iterator begin() noexcept {
        return items_.begin();
      }

      [[nodiscard]] std::unordered_map<std::string, IniValue>::const_iterator cbegin() const noexcept {
        return items_.cbegin();
      }

      [[nodiscard]] std::unordered_map<std::string, IniValue>::iterator end() noexcept {
        return items_.end();
      }

      [[nodiscard]] std::unordered_map<std::string, IniValue>::const_iterator cend() const noexcept {
        return items_.cend();
      }

    private:
      std::unordered_map<std::string, IniValue> items_;
    };

    using IniSections = std::unordered_map<std::string, IniSection>;

    /**
     * @param section name of the section to add
     * @return a reference to the section
     */
    IniSection& AddSection(const std::string& section) const {
      root_->sections[section] = IniSection();
      return root_->sections[section];
    }

    /**
     * @param section name of the section to check for
     * @return returns true if the section exists
     */
    [[nodiscard]] bool HasSection(const std::string& section) const {
      return root_->sections.find(section) != root_->sections.end();
    }

    /**
     * @return count of non root sections
     */
    [[nodiscard]] std::size_t GetSectionCount() const {
      return root_->sections.size();
    }

    /**
     * @param section name of the section to remove
     * @return returns true if the section is removed
     */
    bool RemoveSection(const std::string& section) const {
      if (HasSection(section)) {
        root_->sections.erase(section);
        return true;
      }

      assert(HasSection(section));
      return false;
    }

    /**
     * @return a reference to the root section
     */
    [[nodiscard]] IniSection& GetRootSection() const {
      return root_->root_section;
    }

    /**
     * @param section name of the section to get
     * @return a reference to the section
     */
    [[nodiscard]] IniSection& GetSection(const std::string& section) const {
      const auto entry = root_->sections.find(section);

      if (entry != root_->sections.end()) {
        return entry->second;
      }

      assert(entry != root_->sections.end());
      throw std::runtime_error("Section: " + section + " does not exist");
    }

    /**
     * @return a reference to all the available sections
     */
    [[nodiscard]] IniSections& GetSections() const {
      if (!root_->sections.empty()) {
        return root_->sections;
      }

      assert(root_->sections.size() == 0);
      throw std::runtime_error("No sections found");
    }

    /**
     * @param section name of the section to get
     * @return a reference to the section
     */
    [[nodiscard]] IniSection& operator[](const std::string& section) const {
      return GetSection(section);
    }

    [[nodiscard]] std::unordered_map<std::string, IniSection>::iterator begin() noexcept {
      return root_->sections.begin();
    }

    [[nodiscard]] std::unordered_map<std::string, IniSection>::const_iterator cbegin() const noexcept {
      return root_->sections.cbegin();
    }

    [[nodiscard]] std::unordered_map<std::string, IniSection>::iterator end() noexcept {
      return root_->sections.end();
    }

    [[nodiscard]] std::unordered_map<std::string, IniSection>::const_iterator cend() const noexcept {
      return root_->sections.cend();
    }

    /**
     * @return a string representation of the ini file
     */
    [[nodiscard]] std::string Stringify() const {
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

    /**
     * @param lines a vector of lines to parse
     */
    void ImplParse(std::vector<std::string>& lines) {
      if (wipe_on_parse_) {
        current_section_.clear();
        root_ = std::make_unique<IniRoot>();
      }

      if (lines.empty()) {
        return;
      }

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
            throw std::runtime_error("Section does not have a value with the key: " + current_section_);
          }
          continue;
        }

        if (auto section = GetSection(line); !section.empty()) {
          current_section_ = section;
          AddSection(current_section_);
          continue;
        }

        // if it gets to here it's an empty line
      }
    }

    /**
     * @param line removes a ini comment from the given string
     */
    static void RemoveComment(std::string& line) {
      for (auto&& c : line) {
        if (c == ';' || c == '#') {
          line.clear();
          return;
        } else if (c != ' ') {
          break;
        }
      }

      std::size_t pos{};
      if (pos = line.find(';'); pos != std::string::npos) {
        if (line[pos - 1] == ' ')
          line.erase(pos);
      } else if (pos = line.find('#'); pos != std::string::npos) {
        if (line[pos - 1] == ' ')
          line.erase(pos);
      }
    }

    /**
     * @param line to check for a valid ini item
     * @return if it is a valid item a kv
     */
    static std::pair<std::string, std::string> GetItem(const std::string& line) {
      std::smatch match;
      if (std::regex_match(line, match, std::regex(R"((.*)= ?(.*))"))) {
        return std::make_pair(TRIM_STR(match[1].str(), ' '), TRIM_STR(TRIM_STR(match[2].str(), '"'), ' '));
      }
      return {};
    }

    /**
     * @param line to check for a valid section
     * @return the name of the section
     */
    static std::string GetSection(std::string& line) {
      Trim(line, ' ');
      if (line.empty()) return {};

      if (line[0] == '[') {
        std::size_t search_pos = 1;
        auto close_pos = line.find(']', search_pos);
        while (close_pos != std::string::npos && line[close_pos - 1] == '\\') {
          search_pos = close_pos + 1;
          close_pos = line.find(']', search_pos);
        }
        return line.substr(1, close_pos - 1);
      }

      return {};
    }

    /**
     * @tparam T the stream type
     * @param stream a open stream to read from
     * @return a vector of with the content seperated by new lines
     */
    template <typename T>
    static std::vector<std::string> ReadFile(T& stream) {
      std::string tmp;
      std::vector<std::string> lines;
      while (std::getline(stream, tmp)) {
        std::smatch match;

        // Check if the line contains a line-break.
        // Split it into two lines accordingly.
        if (std::regex_match(tmp, match, std::regex(R"((.*)\r(.*))"))) {
            lines.emplace_back(match[1].str());
            lines.emplace_back(match[2].str());
        } else
          lines.emplace_back(tmp);
      }
      return lines;
    }

    /**
     * @param str to split
     * @return a split string inside a vector
     */
    static std::vector<std::string> Split(const std::string& str) {
      std::vector<std::string> res;
      std::string tmp;
      for (auto& ch : str) {
        if (ch == '\n' || ch == '\r') {
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

    /// trim the front of the string by given character
    static std::string Trim(std::string str, char trim_c) {
      while (str.front() == trim_c) {
        str.erase(0, 1);
      }
      return str;
    }

    /// trim the back of the string by given character
    static std::string TrimR(std::string str, char trim_c) {
      while (str.back() == trim_c) {
        str.pop_back();
      }

      return str;
    }

    /// Check if given path is a file that can be parsed
    static void CheckValidFile(const std::filesystem::path& file) {
      if (!std::filesystem::exists(file)) {
        assert(!std::filesystem::exists(file));
        throw std::runtime_error("File not found");
      }

      if (!std::filesystem::is_regular_file(file)) {
        assert(!std::filesystem::is_regular_file(file));
        throw std::runtime_error("Not a regular file");
      }
    }
  };
}
#ifdef TRIM_STR
#undef TRIM_STR
#endif

#endif // INIREADER_HPP
