// Config.hpp
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "params.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class Config {
public:
    Config();

    // возвращает false, если была команда "config"
    bool execute() const;
    void executePlayFromAudiofile() const;
    void executePlayFromMic() const;
    void executeConfig() const;

    bool saveToFile(const std::string& filename, const std::string& configname) const;
    std::string getConfigFilePath() const;
    bool validate() const;

    // вызывается из Parser: напрямую выставляет поля params или command
    void set(const std::string& key, const nlohmann::json& value);
    void push_back(const std::string& key, const std::string& value);

    // runtime-параметры
    Params params;
    std::string command;
    std::string audiofile;
    std::string subcommand;
    std::vector<std::string> command_arguments;
    std::vector<std::string> subcommand_arguments;
    bool end = false;
};

#endif // CONFIG_HPP
