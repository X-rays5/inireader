//
// Created by X-ray on 5/18/2021.
//
#pragma once
#include <string>
#include <map>
#include <fstream>
#include <utility>
#include <vector>
#include <algorithm>
#include <cassert>

// this class is meant for internal library use only
class ini_exception {
public:
    ini_exception(std::string error) : what_(std::move(error)) {}

    std::string What() {
        return what_;
    }
private:
    std::string what_;
};

class ini {
public:
    ini() {
        std::map<std::string, std::string> assign;
        sections_.insert({"global", assign}); // make the global section
    }

    void Parse(std::string filename) {
        std::ifstream reader(filename);
        if (!reader.is_open()) {
            ParseError("Failed to open supplied file for reading");
            return;
        }

        std::vector<std::string> lines;
        std::string buffer;
        while (std::getline(reader, buffer)) {
            if (buffer.size() > 0 && buffer != " ") { // avoid putting empty lines into the lines vector
                lines.emplace_back(buffer);
            }
        }
        if (lines.empty()) {
            ParseError("Suppliad file is empty");
            return;
        }

        ParseFile(lines);
    }

    std::string GetString(std::string section, std::string key) {
        return Get(section, key);
    }

    bool GetBool(std::string section, std::string key) {
        std::string convert = Get(section, key);

        if (convert == "false" || convert == "FALSE" || convert == "0")
            return false;
        return true;
    }

    int GetInt(std::string section, std::string key) {
        try {
            return std::stoi(Get(section, key));
        } catch (std::exception &e) {
            assert(false); // if in debug mode raise a error to notify of error
        }
        return NULL;
    }

    int64_t GetInt64(std::string section, std::string key) {
        try {
            return std::stoll(Get(section, key));
        } catch (std::exception &e) {
            assert(false); // if in debug mode raise a error to notify of error
        }
        return NULL;
    }

    float GetFloat(std::string section, std::string key) {
        try {
            return std::stof(Get(section, key));
        } catch (std::exception &e) {
            assert(false); // if in debug mode raise a error to notify of error
        }
        return NULL;
    }

    double GetDouble(std::string section, std::string key) {
        try {
            return std::stod(Get(section, key));
        } catch (std::exception &e) {
            assert(false); // if in debug mode raise a error to notify of error
        }
        return NULL;
    }

    void AddSection(std::string section) {
        std::map<std::string, std::string> assign;
        sections_[section] = assign;
    }

    void SetValue(std::string section, std::string key, std::string value) {
        auto section_ = sections_.find(section);

        if (section_ != sections_.end()) {
            section_->second[key] = value;
        } else {
            // since the section doesn't exist add it and call the function again
            AddSection(section);
            SetValue(std::move(section), std::move(key), std::move(value));
        }
    }

    void AddKv(std::string section, std::string key, std::string value) {
        SetValue(std::move(section), std::move(key), std::move(value));
    }

    void RemoveSection(std::string section) {
        sections_.erase(section);
    }

    void RemoveKv(std::string section, std::string key) {
        auto section_ = sections_.find(section);

        if (section_ != sections_.end()) {
            section_->second.erase(key);
        }
    }

    std::string Stringify() {
        std::string stringified;

        // globals should be at the top
        if (!sections_["global"].empty()) {
            for (auto&& kv : sections_["global"]) {
                stringified.append(kv.first);
                stringified.append(" = ");
                stringified.append(kv.second);
                stringified.append("\n");
            }
        }

        for (auto&& section : sections_) {
            if (section.first == "global") {
                continue;
            }
            stringified.append("[");
            stringified.append(section.first);
            stringified.append("]\n");
            for (auto&& kv : section.second) {
                stringified.append(kv.first);
                stringified.append(" = ");
                stringified.append(kv.second);
                stringified.append("\n");
            }
        }
        return stringified;
    }

    std::vector<std::string> GetSections() {
        std::vector<std::string> sections;
        for (auto&& section : sections_) {
            sections.emplace_back(section.first);
        }
        return sections;
    }

    std::map<std::string, std::map<std::string, std::string>> GetSectionsRaw() {
        return sections_;
    }

    std::vector<std::string> GetKeys(std::string section) {
        std::vector<std::string> keys;

        auto section_ = sections_.find(section);

        if (section_ != sections_.end()) {
            for (auto&& kv : section_->second) {
                keys.emplace_back(kv.first);
            }
        }
        return keys;
    }

    std::map<std::string, std::string> GetKeysRaw(std::string section) {
        auto section_ = sections_.find(section);

        if (section_ != sections_.end()) {
            return section_->second;
        } else {
            std::map<std::string, std::string> rtn;
            return rtn;
        }
    }

    bool HasParseError() {
        return hasparseerror_;
    }

    std::string GetParseError() {
        if (hasparseerror_)
            return parseerror;
        return "";
    }

private:
    std::map<std::string, std::map<std::string, std::string>> sections_;

    bool hasparseerror_ = false;
    std::string parseerror;
private:
    std::string Get(std::string& section, std::string& key) {
        auto section_ = sections_.find(section);

        if (section_ != sections_.end()) {
            return  section_->second[key];
        }
        return NULL;
    }

    void ParseError(std::string error) {
        hasparseerror_ = true;
        parseerror = std::move(error);
    }

    void ParseFile(std::vector<std::string>& lines) {
        std::string currentsection = "global";
        std::string lastkey = "NULL";
        for (auto&& line : lines) {
            StringToLower(line); // like the windows implementation this won't be case sensitive
            RemoveSpacesFront(line); // remove useless spaces

            try {
                if (HandleComments(line)) { // if this returns true this line should be skipped
                    continue;
                }
                if (IsSection(line, currentsection)) {
                    continue;
                }
                else if (IsKv(line, currentsection, lastkey)) {
                    continue;
                }
                else {
                    // if we get to here we assume it's a multi line value
                    if (lastkey != "NULL") {
                        std::string setvalue = GetString(currentsection, lastkey);
                        SetValue(currentsection, lastkey, setvalue + "\n" + line);
                    }
                }
            } catch (ini_exception &e) {
                ParseError(e.What() + "\n" + line);
                return;
            }
        }
    }


    static bool HandleComments(std::string& line) {
        // this fixes a issue if both a # and a ; where present
        int semicolonpos = line.find(';') != std::string::npos ? line.find(';') : INT_MAX;
        int hashtagpos = line.find('#') != std::string::npos ? line.find('#') : INT_MAX;

        if (semicolonpos < hashtagpos) {
            if (semicolonpos != std::string::npos) {
                int semicolonpos = line.find(';');
                if (semicolonpos > 0) {
                    std::string compare;
                    compare += line.at(semicolonpos - 1);
                    compare += line.at(semicolonpos);
                    if (compare != R"(\;)") { // comment is not escaped so it's a comment
                        line.erase(semicolonpos);
                        RemoveSpaceBack(line);
                    } else { // comment escaped so remove escape character
                        line.erase(semicolonpos - 1, 1);
                    }
                    return false;
                }
                return true;
            }
        } else {
            if (line.find('#') != std::string::npos) {
                int hashtagpos = line.find('#');
                if (hashtagpos > 0) {
                    std::string compare;
                    compare += line.at(hashtagpos - 1);
                    compare += line.at(hashtagpos);
                    if (compare != R"(\#)") { // comment is not escaped so it's a comment
                        line.erase(hashtagpos);
                        RemoveSpaceBack(line);
                    } else { // comment escaped so remove escape character
                        line.erase(hashtagpos - 1, 1);
                    }
                    return false;
                }
                return true; // cause of returning true here the parser will skip this line
            }
        }
        return false;
    }

    bool IsSection(std::string& line, std::string& currentsection) {
        if (line.find('[') != std::string::npos && line.find(']') != std::string::npos) {
            int sectionstartpos = line.find('[');
            if (sectionstartpos == 0) { // since spaces at the front are removed sections should start at 0
                currentsection = line.substr(1, (line.find(']') - 1));
                AddSection(currentsection);
                return true;
            }
        }
        return false;
    }

    bool IsKv(std::string& line, std::string& currentsection, std::string& lastkey) {
        if (line.find('=') != std::string::npos) {
            int ispos = line.find('=');
            if (ispos > 0) { // if this isn't true it misses a key for the value
                RemoveEscapes(line);

                lastkey = line.substr(0, (ispos));
                RemoveSpaceBack(lastkey);
                std::string value = line.substr((ispos + 1), line.back());
                RemoveSpacesFront(value);

                if (lastkey.size() > 0 && value.size() > 0) {
                    SetValue(currentsection, lastkey, value);
                    return true;
                } else {
                    throw (ini_exception("Value doesn't have a key"));
                }
            } else {
                throw (ini_exception("Value doesn't have a key"));
            }
        }
        return false;
    }

    static void RemoveEscapes(std::string& remove) {
        for (int i = 0; i < remove.size(); i++) {
            if (remove.at(i) == '\\') {
                if (i > 0) {
                    if (remove.at(i + 1) == '\\') {
                        // escape character is escaping a escape character so erase one of them and skip the next character
                        remove.erase(i, 1);
                        i++;
                    } else {
                        // if we are here it's not a escape character escaping a escape character
                        remove.erase(i);
                    }
                } else {
                    // if we are here it's not a escape character escaping a escape character
                    remove.erase(i);
                }
            }
        }
    }

    static std::string StringToLower(std::string& convert) {
        std::transform(convert.begin(), convert.end(), convert.begin(), ::tolower);
        return convert;
    }

    static std::string RemoveSpacesFront(std::string& remove) {
        while (!remove.empty() && remove.at(0) == ' ') {
            remove.erase(0, 1);
        }
        return remove;
    }

    static std::string RemoveSpaceBack(std::string& remove) {
        while (!remove.empty() && remove.at(remove.size() - 1) == ' ') {
            remove.pop_back();
        }
        return remove;
    }
};
