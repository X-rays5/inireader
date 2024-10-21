#pragma once

#ifndef INIREADER_HPP
#define INIREADER_HPP

#include <string>
#include <string_view>
#include <regex>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include "conversion.hpp"

namespace ini {
  class Parser {
  public:
    /**
     * @brief Constructor for the Parser class.
     * @param wipe_on_parse If true, the ini file root will be wiped when parsing a new document.
     */
    explicit Parser(const bool wipe_on_parse = true) {
      wipe_on_parse_ = wipe_on_parse;
      root_ = std::make_unique<IniRoot>();
    }

    /**
     * @brief Parses an ini file.
     * @param file Path or contents of the ini file.
     * @param is_path If true, the file parameter is treated as a path; otherwise, it is treated as the contents of an ini file.
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
     * @brief Parses an ini file from a given path.
     * @param file Path to the ini file.
     */
    void Parse(const std::filesystem::path& file) {
      CheckValidFile(file);

      std::ifstream ini_file(file);
      Parse(ini_file);
    }

    /**
     * @brief Parses an ini file from an open stream.
     * @param file Open stream of the ini file.
     */
    void Parse(std::fstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    /**
     * @brief Parses an ini file from an open stream.
     * @param file Open stream of the ini file.
     */
    void Parse(std::ifstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    struct IniValue {
    public:
      /**
       * @brief Gets the value as a specified type.
       * @tparam T Return type of the value.
       * @return Value as type T.
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
       * @brief Checks if the value is of a specified type.
       * @tparam T Type to check.
       * @return True if the value is of type T, false otherwise.
       */
      template <typename T>
      [[nodiscard]] bool is() const {
        conversion::AsImpl<T> as;
        return as.is(value_);
      }

      /**
       * @brief Assigns a value of a specified type.
       * @tparam T Type of the value to assign.
       * @param value Value to assign.
       * @return Reference to the IniValue object.
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
       * @brief Adds a key-value pair to the section.
       * @tparam T Type of the value to add.
       * @param key Key of the value.
       * @param value Value to add.
       */
      template <typename T>
      void Add(const std::string& key, const T& value) {
        conversion::AsImpl<T> as;
        std::string tmp;
        as.set(value, tmp);
        items_[key] = tmp;
      }

      /**
       * @brief Removes a key-value pair from the section.
       * @param key Key of the value to remove.
       * @return True if the key-value pair was removed, false otherwise.
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
       * @brief Removes all key-value pairs from the section.
       */
      void RemoveAll() {
        items_.clear();
      }

      /**
       * @brief Checks if a key exists in the section.
       * @param key Key to check.
       * @return True if the key exists, false otherwise.
       */
      [[nodiscard]] bool HasValue(const std::string& key) const {
        return items_.find(key) != items_.end();
      }

      /**
       * @brief Converts the section to a string representation.
       * @return String representation of the section.
       */
      [[nodiscard]] std::string Stringify() const {
        std::stringstream res;
        for (auto& item : items_) {
          res << item.first << "=" << item.second.as<std::string>() << "\n";
        }

        return res.str();
      }

      /**
       * @brief Gets the number of key-value pairs in the section.
       * @return Number of key-value pairs in the section.
       */
      [[nodiscard]] size_t Size() const {
        return items_.size();
      }

      /**
       * @brief Gets a reference to a value by key.
       * @param key Key of the value to get.
       * @return Reference to the value.
       * @throws std::runtime_error if the key does not exist.
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
     * @brief Adds a section to the ini file.
     * @param section Name of the section to add.
     * @return Reference to the added section.
     */
    IniSection& AddSection(const std::string& section) const {
      root_->sections[section] = IniSection();
      return root_->sections[section];
    }

    /**
     * @brief Checks if a section exists in the ini file.
     * @param section Name of the section to check.
     * @return True if the section exists, false otherwise.
     */
    [[nodiscard]] bool HasSection(const std::string& section) const {
      return root_->sections.find(section) != root_->sections.end();
    }

    /**
     * @brief Gets the number of sections in the ini file.
     * @return Number of sections in the ini file.
     */
    [[nodiscard]] std::size_t GetSectionCount() const {
      return root_->sections.size();
    }

    /**
     * @brief Removes a section from the ini file.
     * @param section Name of the section to remove.
     * @return True if the section was removed, false otherwise.
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
     * @brief Gets a reference to the root section.
     * @return Reference to the root section.
     */
    [[nodiscard]] IniSection& GetRootSection() const {
      return root_->root_section;
    }

    /**
     * @brief Gets a reference to a section by name.
     * @param section Name of the section to get.
     * @return Reference to the section.
     * @throws std::runtime_error if the section does not exist.
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
     * @brief Gets a reference to all the sections in the ini file.
     * @return Reference to all the sections.
     * @throws std::runtime_error if no sections are found.
     */
    [[nodiscard]] IniSections& GetSections() const {
      if (!root_->sections.empty()) {
        return root_->sections;
      }

      assert(root_->sections.size() == 0);
      throw std::runtime_error("No sections found");
    }

    /**
     * @brief Gets a reference to a section by name.
     * @param section Name of the section to get.
     * @return Reference to the section.
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
     * @brief Converts the ini file to a string representation.
     * @return String representation of the ini file.
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
     * @brief Parses the lines of an ini file.
     * @param lines Vector of lines to parse.
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
     * @brief Removes a comment from a line.
     * @param line Line to remove the comment from.
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
     * @brief Gets a key-value pair from a line.
     * @param line Line to check for a valid ini item.
     * @return Key-value pair if it is a valid item, empty pair otherwise.
     */
    static std::pair<std::string, std::string> GetItem(const std::string& line) {
      std::smatch match;
      if (std::regex_match(line, match, std::regex(R"((.*?)\s*=\s*(.*))"))) {
        return std::make_pair(TRIM_STR(match[1].str(), ' '), TRIM_STR(TRIM_STR(match[2].str(), '"'), ' '));
      }
      return {};
    }

    /**
     * @brief Gets the name of a section from a line.
     * @param line Line to check for a valid section.
     * @return Name of the section if it is a valid section, empty string otherwise.
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
     * @brief Reads the contents of a file into a vector of lines.
     * @tparam T Stream type.
     * @param stream Open stream to read from.
     * @return Vector of lines.
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
     * @brief Splits a string into a vector of lines.
     * @param str String to split.
     * @return Vector of lines.
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

    /**
     * @brief Trims the front of a string by a given character.
     * @param str String to trim.
     * @param trim_c Character to trim.
     * @return Trimmed string.
     */
    static std::string Trim(std::string str, char trim_c) {
      while (str.front() == trim_c) {
        str.erase(0, 1);
      }
      return str;
    }

    /**
     * @brief Trims the back of a string by a given character.
     * @param str String to trim.
     * @param trim_c Character to trim.
     * @return Trimmed string.
     */
    static std::string TrimR(std::string str, char trim_c) {
      while (str.back() == trim_c) {
        str.pop_back();
      }

      return str;
    }

    /**
     * @brief Checks if a given path is a valid file.
     * @param file Path to check.
     * @throws std::runtime_error if the file is not found or is not a regular file.
     */
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
