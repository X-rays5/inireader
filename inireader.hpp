//
// Created by X-ray on 5/18/2021.
//
#include <map>
#include <string>
#include <fstream>
#include <vector>

class inireader {
public:
    void Parse(std::string& filename) {
        hasparseerror_ = false;
        std::ifstream reader(filename);
        if (!reader.is_open()) {
            hasparseerror_ = true;
            parseerrors_.emplace_back("Failed to open file for reading");
            return;
        }

        std::string buffer;
        std::vector<std:string> lines;
        while (std::getline(reader, buffer)) {
            lines.emplace_back(buffer);
        }
        if (lines.empty()) {
            hasparseerror_ = true;
            parseerrors_.emplace_back("Supplied file was empty");
            return;
        }

        ParseFile(lines);
    }

    bool HasParseError() {
        return hasparseerror_;
    }

    std::string GetLastError() {
        if (!errors.empty())
            return errors.back();
        hasparseerror_ = false;
        return "";
    }

    std::string GetErrorAtIndex(int index) {
        try {
            return errors[index];
        } catch (std::exception &e) {
            return e.what();
        }
    }
private:
    std::map<std::string, std::map<std::string, std::string>> sections_;
    std::vector<std::string> parseerrors_;
    bool hasparseerror_ = false;

private:
    void ParseFile(std::vector<std::string>& lines) {
        std::string currentsection = "NULL";
        for (auto&& line : lines) {
            RemoveSpacesAtFront(line); // remove all the spaces before a line so it easier to avoid seeing something as a section which isn't
            StringToLower(line); // make it so that the ini reader isn't case sensitive like the windows implementation

            // if a comment is present nothing else can be there for now
            if (IsComment(line)) {
                if (hasparseerror_)
                    return;
                continue;
            }
            if (IsSection(line, currentsection)) {
                if (hasparseerror_)
                    return; // return here since we don't want keys assigned to wrong sections
                continue;
            }
        }
    }

    bool IsComment(std::string line) {
        if (line.find(';') != std::string::npos) {
            int semicolonpos = line.find(';');
            if (semicolonpos != 0) {
                if (line.at(semicolonpos - 1) == '\\') { // semicolon is escaped
                    return false;
                } else {
                    return true;
                }
            } else {
                return true;
            }
        }
        return false;
    }

    bool IsSection(std::string& line, std::string& currentsection) {
        if (line.find('[') == 0 && line.find(']') != std::string::npos) {
            int sectionendpos = line.find(']')
            if (sectionendpos != 1) {
                currentsection = line.substr(1, sectionendpos);
                sections.insert({currentsection, std::map<std::string, std::string>});
                return true;
            } else {
                hasparseerror = true;
                parseerrors_.emplace_back("Line is missing section name\n"+line);
                // return here since if we continue keys can get assigned to wrong sections
                return true; // return true so it willhandle the error
            }
        }
        return false;
    }

    bool IsKey(std::string& line, std::string currentsection) {
        if (line.find('=') != std::string::npos) {
            int ispos = line.find('=');
            if (ispos != 0) {
                if (line.at(ispos - 1) != '\\') {
                    if (ispos - 2 != 0) {

                    } else {
                        // if we get here the name of the section is /
                    }
                } else {
                    return false; // is is escaped
                }
            } else {
                hasparseerror_ = true;
                parseerrors_.emplace_back("Key name was not given\n"+line);
                return true; // returning true so it will handle the error
            }
        }
        return false;
    }

    std::string RemoveSpacesAtFront(std::string& remove) {
        for (int i = 0; i < remove.size(); i++) {
            if (remove.at(i) == ' ') {
                remove.erase(0);
            } else {
                break;
            }
        }
        return remove;
    }

    std::string StringToLower(std::string& convert) {
        std::transform(convert.begin(), convert.end(), convert.begin(), ::tolower);
        return convert;
    }
};
