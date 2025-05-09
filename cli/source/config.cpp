// Config.cpp
#include "config.hpp"
#include "version.hpp"

#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

Config::Config() {
    // всё, что нужно для defaults, уже задано в Params::Params()
}

bool Config::execute() const {
    if (command == "play") {
        executePlayFromAudiofile();
        return true;
    } else if (command == "mic") {
        executePlayFromMic();
        return true;
    } else if (command == "config") {
        executeConfig();
        return false;
    } else {
        std::cout << "Unknown command\n";
        return false;
    }
}

void Config::executePlayFromAudiofile() const {
    std::cout << "play\n";
}

void Config::executePlayFromMic() const {
    std::cout << "mic\n";
}

void Config::executeConfig() const {
    saveToFile(getConfigFilePath(), "default");
    std::cout << "Default configuration successfully saved in "
                 "~/.config/a2i/config.json\n";
}

bool Config::saveToFile(const std::string& filename,
                        const std::string& configname) const {
    mkdir((filename.substr(0, filename.find_last_of('/'))).c_str(), 0755);
    std::ofstream f(filename);
    if (!f.is_open()) return false;

    nlohmann::json out = {
        {configname, params.toJson()}
    };
    f << out.dump(4);
    return true;
}

std::string Config::getConfigFilePath() const {
    const char* h = getenv("HOME");
    std::string dir = std::string(h ? h : ".") + "/.config/a2i";
    // mkdir(dir.c_str(), 0755); // уже делаем в saveToFile
    return dir + "/config.json";
}

bool Config::validate() const {
    return params.framesize >= 512
        && (params.framesize & (params.framesize - 1)) == 0
        && params.windowFunc >= 0 && params.windowFunc <= 9
        && params.lineType >= 0 && params.lineType <= 2
        && params.graphMode >= 0 && params.graphMode <= 1
        && params.previousFrames >= 0
        && params.colormap >= 0 && params.colormap <= 21
        && params.fillType >= 0 && params.fillType <= 2
        && params.colormapCoef >= 0 && params.colormapCoef <= 255
        && params.volume >= 0.0f && params.volume <= 1.0f;
}

void Config::set(const std::string& key, const nlohmann::json& v) {
    // здесь мапим ключи на поля params (и на command, audiofile...)
    if (key == "framesize")         params.framesize     = v;
    else if (key == "window-func")  params.windowFunc    = v;
    else if (key == "line-type")    params.lineType      = v;
    else if (key == "graph-mode")   params.graphMode     = v;
    else if (key == "previous-frames") params.previousFrames = v;
    else if (key == "colormap")     params.colormap      = v;
    else if (key == "fill-type")    params.fillType      = v;
    else if (key == "colormap-coef")  params.colormapCoef = v;
    else if (key == "volume")       params.volume        = v;
    else if (key == "border")       params.border        = v;
    else if (key == "grid")         params.grid          = v;
    else if (key == "onlyaudio")    params.onlyAudio     = v;
    else if (key == "debug")        params.debug         = v;
    else if (key == "command")      command              = v;
    else if (key == "subcommand")   subcommand           = v;
    else if (key == "audiofile")    audiofile            = v;
    // для массивов и цветов можно либо расширить, либо закинуть через Params::loadFromJson
}

void Config::push_back(const std::string& key, const std::string& value) {
    if (key == "command_arguments")
        command_arguments.push_back(value);
    else if (key == "subcommand_arguments")
        subcommand_arguments.push_back(value);
}
