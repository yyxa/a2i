#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <map>
#include "config.hpp"
#include "nlohmann/json.hpp"

class Parser {
public:
    Parser(int argc, char** argv, Config& c);
    void loadConfigFromFile(Config& c, std::string configName);
    void loadDefault(Config& c);
    void parseArgs(int argc, char** argv, Config& config);
    void printUsage() const;
    void printControls() const;

private:
    nlohmann::json arguments = {
        {"flags", {
            {"a", {
              {"default", {-90,50}}, 
              {"about", "Amplitude range"},
              {"needValue", true}
            }},
            {"w", {
              {"default", 9}, 
              {"about", "Window function (0-9)"},
              {"needValue", true}
            }},
            {"l", {
              {"default", 0}, 
              {"about", "Line type (0-2)"},
              {"needValue", true}
            }},
            {"g", {
              {"default", 1}, 
              {"about", "Graph mode (0-1)"},
              {"needValue", true}
            }},
            {"m", {
              {"default", 20}, 
              {"about", "Normalize multiplier"},
              {"needValue", true}
            }},
            {"f", {
              {"default", 65536}, 
              {"about", "Frame size (>=512)"},
              {"needValue", true}
            }},
            {"n", {
              {"default", 20}, 
              {"about", "Number of previous frames (>0)"},
              {"needValue", true}
            }},
            {"s", {
              {"default", {400,2100}}, 
              {"about", "Window size (height,width)"},
              {"needValue", true}
            }},
            {"grad", {
              {"default", true}, 
              {"about", "Colormap (0-21)"},
              {"needValue", false}
            }},
            {"fill", {
              {"default", 2}, 
              {"about", "Fill type (0-2)"},
              {"needValue", true}
            }},
            {"b", {
              {"default", false}, 
              {"about", "Border line"},
              {"needValue", false}
            }},
            {"lc", {
              {"default", "255,255,255"}, 
              {"about", "Line color"},
              {"needValue", true}
            }},
            {"uc", {
              {"default", "127,127,127"}, 
              {"about", "Underline color"},
              {"needValue", true}
            }},
            {"grid", {
              {"default", true}, 
              {"about", "Grid"},
              {"needValue", false}
            }},
            {"gc", {
              {"default", 127}, 
              {"about", "Gradient coefficient (0-255)"},
              {"needValue", true}
            }},
            {"glc", {
              {"default", "79,73,80"}, 
              {"about", "Grid line color"},
              {"needValue", true}
            }},
            {"gtc", {
              {"default", "51,186,243"}, 
              {"about", "Grid text color"},
              {"needValue", true}
            }},
            {"v", {
              {"default", 0.8}, 
              {"about", "Set volume level (0.0-1.0)"},
              {"needValue", true}
            }},
            {"onlyaudio", {
              {"default", false}, 
              {"about", "Play only audio"},
              {"needValue", false}
            }},
            {"debug", {
              {"default", false}, 
              {"about", "Enable debug mode"},
              {"needValue", false}
            }},
            {"config", {
              {"about", "Read configuration with name"},
              {"needValue", true}
            }},
        }},
        {"commands", {
            {"mic", {
              {"about", "Use microphone"},
              {"needFlags", true},
              {"needValue", 0}
            }},
            {"help", { 
              {"about", "Show help message"},
              {"needFlags", false},
              {"needValue", 0}
            }},
            {"write-default-config", {
              {"about", "Write default configuration"},
              {"needFlags", false},
              {"needValue", 0}
            }},
            {"write-config", {
              {"about", "Write configuration with name"},
              {"needFlags", true},
              {"needValue", 1}
            }},
            {"rename-config", {
              {"about", "Rename configuration name"},
              {"needFlags", false},
              {"needValue", 2}
            }},
            {"version", {
              {"about", "Show version"},
              {"needFlags", false},
              {"needValue", 0}
            }}
        }},
        {"controls", {
            {"Space", {
              {"about", "Pause/Resume"}
            }},
            {"Left arrow/a", { 
              {"about", "Rewind 5 seconds"}
            }},
            {"Right arrow/d", {
              {"about", "Fast Forward 5 seconds"}
            }},
            {"Esc/q", {
              {"about", "Exit"}
            }}
        }}
    };
};

#endif // PARSER_HPP