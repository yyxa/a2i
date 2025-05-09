// parser.hpp
#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include "config.hpp"
#include "nlohmann/json.hpp"

class Parser {
public:
    Parser(int argc, char** argv, Config& c);
    void loadConfigFromFile(Config& c, std::string configName);
    void parseArgs(int argc, char** argv, Config& config);
    void printUsage() const;
    void printManPage(const std::string& command) const;
    void printVersion() const;

    static nlohmann::json arguments;
};

#endif // PARSER_HPP
