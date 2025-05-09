#include "parser.hpp"
#include "version.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <nlohmann/json.hpp>

nlohmann::json Parser::arguments = {
  { "commands", {
      { "config", {
          { "needValue", 0 },
          { "needFlags", 0 },
          { "subcommands", {
              { "write",         { { "needValue", 1 }, { "needFlags", 0 } } },
              { "rename",        { { "needValue", 2 }, { "needFlags", 0 } } },
              { "reset-default", { { "needValue", 0 }, { "needFlags", 0 } } }
          }}
      }},
      { "mic", {
          { "needValue", 0 },
          { "needFlags", 1 }
      }}
  }},
  { "flags", {
      // вот примеры ключей, добавьте по аналогии остальные из defaultFlags
      { "framesize",      { { "dash", 2 }, { "needValue", 1 } } },
      { "window-func",    { { "dash", 2 }, { "needValue", 1 } } },
      { "line-type",      { { "dash", 2 }, { "needValue", 1 } } },
      { "graph-mode",     { { "dash", 2 }, { "needValue", 1 } } },
      { "previous-frames",{ { "dash", 2 }, { "needValue", 1 } } },
      { "colormap",       { { "dash", 2 }, { "needValue", 1 } } },
      { "fill-type",      { { "dash", 2 }, { "needValue", 1 } } },
      { "colormap-coef",  { { "dash", 2 }, { "needValue", 1 } } },
      { "normalize-multiplier", { { "dash", 2 }, { "needValue", 1 } } },
      { "volume",         { { "dash", 2 }, { "needValue", 1 } } },
      { "window-size",         { { "dash", 2 }, { "needValue", 1 } } },
      { "border",         { { "dash", 2 } } },
      { "grid",           { { "dash", 2 } } },
      { "onlyaudio",      { { "dash", 2 } } },
      { "debug",          { { "dash", 2 } } },
      // глобальные флаги
      { "help",           { { "dash", 2 } } },
      { "version",        { { "dash", 2 } } }
  }},
  { "controls", {
      { "space", { { "about", "Play/Pause" } } },
      { "a",     { { "about", "Seek backward" } } },
      { "d",     { { "about", "Seek forward" } } },
      { "q",     { { "about", "Quit" } } }
  }}
};

Parser::Parser(int argc, char** argv, Config& c) {
    if(argc == 1) {
      printUsage();
      c.end = 1;
      return;
    }
    loadConfigFromFile(c, "default");
    parseArgs(argc, argv, c);
}

void Parser::loadConfigFromFile(Config& c, std::string configName) {
  auto filename = c.getConfigFilePath();
  std::ifstream configFile(filename);
  nlohmann::json flags;
  if (configFile.is_open()) {
      configFile >> flags;
      if (flags.contains(configName)) {
          auto defaultConfig = flags[configName];
             for (auto& item : defaultConfig.items()) {
                 c.set(item.key(), item.value());
             }
      } else {
          std::cout << "Configuration '" << configName << "' not found. Loading default settings." << std::endl;
      }
  } else {
      std::cerr << "Failed to open config file: " << filename << std::endl;
  }
}

void Parser::parseArgs(int argc, char** argv, Config& c) {

    std::string command = argv[1];
    int counter = 2;
    bool containsCommand = arguments["commands"].contains(command);
    bool containsSubcommands = false;
    bool needFlags = true;

    if (containsCommand) {
        if(argc > 2 && strcmp(argv[2], "--help") == 0) {
          if(command == "config") {
            printManPage(command);
            c.end = true;
            return;
          }
          else if(command == "mic") {
            printManPage(command);
            c.end = true;
            return;
          }
        }
        containsSubcommands = arguments["commands"][command].contains("subcommands");
    }

    if (containsSubcommands) {
        if (argc < 3) {
            throw std::runtime_error("No subcommand provided");
        }
        std::string subcommand = argv[2];

        if (!arguments["commands"][command]["subcommands"].contains(subcommand)) {
            throw std::runtime_error("Unknown subcommand: " + subcommand);
        }

        int needValue = arguments["commands"][command]["subcommands"][subcommand].value("needValue", 0);
        counter++;
        for (int i = 3; i < 3 + needValue; ++i, ++counter) {
            c.push_back("subcommand_arguments", argv[i]);
        }
        c.set("command", command);
        c.set("subcommand", subcommand);

        needFlags = arguments["commands"][command]["subcommands"][subcommand].value("needFlags", 0);
    } 
    else if (containsCommand) {
        int needValue = arguments["commands"][command].value("needValue", 0);
        for (int i = 2; i < 2 + needValue; ++i, ++counter) {
            c.push_back("command_arguments", argv[i]);
        }
        c.set("command", command);

        needFlags = arguments["commands"][command].value("needFlags", 0);
    } 
    else if (command.substr(0, 2) == "--" && arguments["flags"].contains(command.substr(2, command.length()))) {
        std::string flag = command.substr(2, command.length());
        if (arguments["flags"].contains(flag) && arguments["flags"][flag].contains("dash") && arguments["flags"][flag]["dash"] == 2) {
            if (flag == "help") {
                printManPage("help");
                c.end = true;
                return;
            } else if (flag == "version") {
                printVersion();
                c.end = true;
                return;
            } else {
                throw std::runtime_error("Unknown flag --" + flag);
            }
        } else {
            throw std::runtime_error("Unknown flag --" + flag);
        }
    }
    else {
      std::string format = command.substr(command.length()-4, command.length());
      if(format == ".mp3" || format == ".wav") {
        c.set("command", "play");
        c.set("audiofile", command);
      }
      else {
        throw std::runtime_error("Unknown command " + command);
      }
    }

    for (int i = counter; i < argc && needFlags; ++i) {
        std::string arg = argv[i];

        if (arg.rfind("--", 0) == 0) {
          // --flag или --flag=value
          size_t eq = arg.find('=');
          std::string flag = (eq != std::string::npos)
              ? arg.substr(2, eq - 2)
              : arg.substr(2);
          if (!arguments["flags"].contains(flag))
              throw std::runtime_error("Unknown flag --" + flag);

          bool needsValue = arguments["flags"][flag].value("needValue", 0);
          if (needsValue) {
              std::string val;
              if (eq != std::string::npos) {
                  val = arg.substr(eq + 1);
              } else if (i + 1 < argc) {
                  val = argv[++i];  // читаем следующий токен
              } else {
                  throw std::runtime_error("Expected value for flag --" + flag);
              }
              nlohmann::json jv;
              jv = nlohmann::json::parse(val);
              c.set(flag, jv);
          } else {
              c.set(flag, true);
          }
      }
      else {
          throw std::runtime_error("Unknown argument: " + arg);
      }
    }
    
    c.end = false;
}

void Parser::printUsage() const {
    std::cout << "Usage: a2i [--version] [--help] <command|audiofile> [<args>]\n\n"
              << "These are common a2i commands used in various situations:\n\n"
              << "start a working area:\n"
              << "   mic       Use microphone\n"
              << "   config    Configure settings\n\n"

              << "manage configurations:\n"
              << "   config write <name>       Write configuration with name\n"
              << "   config rename <old> <new> Rename configuration name\n"
              << "   config reset-default      Write default configuration\n\n"

              << "Options:\n";

    for (const auto& flag : arguments["flags"].items()) {
        if (flag.value().contains("dash") && flag.value()["dash"] == 1) {
            std::cout << "  -" << flag.key();
            if (flag.value().contains("needValue") && flag.value()["needValue"]) {
                std::cout << "=<value>";
            }
            std::cout << "\t" << (flag.value().contains("about") ? flag.value()["about"].get<std::string>() : "") << "\n";
        }
    }

    std::cout << "\nControls:\n";
    for (const auto& control : arguments["controls"].items()) {
        std::cout << "  " << control.key() << "\t" << (control.value().contains("about") ? control.value()["about"].get<std::string>() : "") << "\n";
    }

    std::cout << "\nSee 'a2i --help' or 'a2i --version' for more information.\n";
}

void Parser::printManPage(const std::string& command) const {
    if (command == "config") {
        system("man a2i-config");
    } else if (command == "mic") {
        system("man a2i-mic");
    } else if (command == "help"){
        system("man a2i");
    }
}

void Parser::printVersion() const {
    std::cout << "a2i version " << A2I_VERSION << std::endl;
}