//
// Created by X-ray on 5/18/2021.
//
#pragma once

#ifndef INIREADER_HPP
#define INIREADER_HPP
#include <string>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <regex>
#include <filesystem>
namespace fs = std::filesystem;

namespace ini {
  class Parser {
  public:
    Parser()
    {
      parsed_ = std::make_unique<Parsed>(); // avoid nullptr exceptions when parse hasn't been called
    }

    /// Opens file at given path and parses it
    /**
     * @param file_name Path to ini file to parse
     * @return Opened file successfully
     * @note Calling this function will reset the parsed data
     */
    inline bool Parse(const fs::path& file_name) {
      std::fstream reader(file_name);
      return Parse(reader);
    }

    /// Reads file and parsed it
    /**
     * @param file Open file stream with file to parse
     * @return Opened file successfully
     * @note Calling this function will reset the parsed data
     */
    bool Parse(std::fstream& file) {
      // reset previous parse data if present
      parsed_ = std::make_unique<Parsed>();
      last_parse_error_.clear();

      if (!file.is_open()) {
        assert(file.is_open());
        last_parse_error_ = "Couldn't open file";
        return false;
      }

      std::vector<std::string> lines;
      std::string buffer;
      while (std::getline(file, buffer)) {
        lines.emplace_back(buffer);
      }
      lines.shrink_to_fit(); // since the amount of lines will stay the same save some memory

      for (auto&& line : lines)
        ParseLine(line);

      return true;
    }

    /**
     * @return Stringified version of parsed data
     */
    std::string Stringify() {
      std::stringstream stringified;

      // root kv
      for (auto&& kv : parsed_->root)
        stringified << kv.first << '=' << kv.second << "\n";

      stringified << "\n"; // line between sections
      // normal kv
      for (auto&& section : parsed_->sections) {
        stringified << '[' << section.first << ']' << "\n";
        for (auto&& kv: section.second.entries)
          stringified << kv.first << '=' << kv.second << "\n";
        stringified << "\n"; // line between sections
      }

      return stringified.str();
    }

    /// Get a kv which is located before the first section
    /**
     * @param key
     * @return Value of kv
     */
    inline std::string GetDefault(const std::string& key) {
      auto entry = parsed_->root.find(key);

      if (entry != parsed_->root.end()) {
        return entry->second;
      }
      return {};
    }

    /**
     * @param key
     * @param val
     * @note Overwrites value with key if already exists
     */
    inline void AddKVDefault(const std::string& key, const std::string& val) {
      parsed_->root[key] = val;
    }

    /**
     *
     * @param section
     * @param key
     * @param val
     * @note Overwrites value with key if already exists
     */
    void AddKV(const std::string& section, const std::string& key, const std::string& val) {
      if (!section.empty()) {
        auto entry = parsed_->sections.find(section);

        if (entry != parsed_->sections.end()) {
          entry->second.entries[key] = val;
        } else {
          Entries new_section;
          new_section.entries[key] = val;
          parsed_->sections[section] = new_section;
        }
      } else {
        AddKVDefault(key, val);
      }
    }

    /// Remove all key,values from the default section
    inline void RemoveDefault() {
      parsed_->root.clear();
    }

    /// Remove a kv from the default section
    /**
     * @param key
     */
    inline void RemoveKVDefault(const std::string& key) {
      auto entry = parsed_->root.find(key);

      if (entry != parsed_->root.end())
        parsed_->root.erase(entry);
    }

    /// Remove a specific kv from a section
    /**
     * @param section
     * @param key
     */
    inline void RemoveKV(const std::string& section, const std::string& key) {
      auto entry = parsed_->sections.find(section);

      if (entry != parsed_->sections.end()) {
        auto entry_kv = entry->second.entries.find(key);

        if (entry_kv != entry->second.entries.end())
          entry->second.entries.erase(entry_kv);
      }
    }

    /// Remove a section and all it's key,values
    /**
     * @param section
     */
    inline void RemoveSection(const std::string& section) {
      auto entry = parsed_->sections.find(section);

      if (entry != parsed_->sections.end())
        parsed_->sections.erase(entry);
    }

    /**
     * @return Has a parse error occurred
     */
    inline bool HasParseError() {
      return !last_parse_error_.empty();
    }

    /**
     * @return The last parse error
     */
    inline std::string GetParseError() {
      return last_parse_error_;
    }

    using Entries_t = std::unordered_map<std::string, std::string>;

    // has to be in a struct so that the [] operator can be implemented
    struct Entries {
      Entries_t entries;

      /**
       * @param key
       * @return Value of key in section
       */
      std::string operator[](const std::string& key) {
        auto entry = entries.find(key);

        if (entry != entries.end()) {
          return entry->second;
        }
        return {};
      }
    };
    using Sections_t = std::unordered_map<std::string, Entries>;

    /**
     * @param section
     * @return Section with all it's entries
     */
    Parser::Entries operator[](const std::string& section) {
      auto entry = parsed_->sections.find(section);

      if (entry != parsed_->sections.end()) {
        return entry->second;
      }
      return {};
    }

  private:
    struct Entry {
      const std::string key;
      const std::string value;
    };

    struct Parsed {
      std::string current_section;
      Sections_t sections;
      Entries_t root;
    };

    std::string last_parse_error_;
    std::unique_ptr<Parsed> parsed_;

  private:
#define TRIM_STR(str, c) trim_r(trim(str, c), c)

    void ParseLine(std::string& line) {
      if (line.empty())
        return;

      RemoveComment(line);
      if (line.empty())
        return; // don't process the line any further when empty

      auto entry = GetEntry(line);
      if (!entry.key.empty() && !entry.value.empty()) {
        if (!parsed_->current_section.empty()) {
          AddKV(parsed_->current_section, entry.key, entry.value);
        } else {
          // no section has been found yet so write to root
          AddKVDefault(entry.key, entry.value);
        }
        return;
      }

      std::string section = GetSection(line);
      if (!section.empty()) {
        parsed_->current_section = std::move(section);
        return;
      }
    }

    static inline void RemoveComment(std::string& line) {
      std::regex pattern(R"((^\;.*)|(^\#.*)|(.*[^\\];)|(.*[^\\]#))");
      std::smatch match;

      if (std::regex_search(line, match, pattern)) {
        // full string match
        if (match[0].str() == line)
          line.clear();
        else
          line.erase(match[0].str().size() - 1, line.back());
      }
    }

    static inline Entry GetEntry(const std::string& line) {
      std::regex pattern(R"((.*)= ?(.*))");
      std::smatch match;

      if (std::regex_search(line, match, pattern)) {
        return {
          TRIM_STR(match[1].str(), ' '),
          TRIM_STR(match[2].str(), ' ')
        };
      }
      return {"",""};
    }

    static inline std::string GetSection(const std::string& line) {
      std::regex pattern(R"(\[(.*)\])");
      std::smatch match;

      if (std::regex_search(line, match, pattern)) {
        return TRIM_STR(match[1].str(), ' ');
      }

      return {};
    }

    static inline std::string trim(std::string str, char trim_c) {
      while(str.front() == trim_c) {
        str.erase(0, 1);
      }
      return str;
    }

    static inline std::string trim_r(std::string str, char trim_c) {
      while (str.back() == trim_c) {
        str.pop_back();
      }
      return str;
    }
  };
}
#endif // INIREADER_HPP
