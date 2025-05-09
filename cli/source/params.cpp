// Params.cpp
#include "params.hpp"

void Params::loadFromJson(const nlohmann::json& j) {
    if (j.contains("framesize"))            framesize           = j["framesize"];
    if (j.contains("window-func"))          windowFunc          = j["window-func"];
    if (j.contains("line-type"))            lineType            = j["line-type"];
    if (j.contains("graph-mode"))           graphMode           = j["graph-mode"];
    if (j.contains("previous-frames"))      previousFrames      = j["previous-frames"];
    if (j.contains("colormap"))             colormap            = j["colormap"];
    if (j.contains("fill-type"))            fillType            = j["fill-type"];
    if (j.contains("colormap-coef"))        colormapCoef        = j["colormap-coef"];
    if (j.contains("normalize-multiplier")) normalizeMultiplier  = j["normalize-multiplier"];

    if (j.contains("volume"))               volume              = j["volume"];
    if (j.contains("border"))               border              = j["border"];
    if (j.contains("grid"))                 grid                = j["grid"];
    if (j.contains("onlyaudio"))            onlyAudio           = j["onlyaudio"];
    if (j.contains("debug"))                debug               = j["debug"];

    if (j.contains("window-size") && j["window-size"].is_array()) {
        windowSize = {
            j["window-size"][0].get<int>(),
            j["window-size"][1].get<int>()
        };
    }

    if (j.contains("amplitude-range") && j["amplitude-range"].is_array()) {
        amplitudeRange = {
            j["amplitude-range"][0].get<int>(),
            j["amplitude-range"][1].get<int>()
        };
    }

    if (j.contains("line-color") && j["line-color"].is_array()) {
        auto &c = j["line-color"];
        lineColor = { c[2], c[1], c[0] };
    }
    if (j.contains("underline-color") && j["underline-color"].is_array()) {
        auto &c = j["underline-color"];
        underlineColor = { c[2], c[1], c[0] };
    }
    if (j.contains("grid-line-color") && j["grid-line-color"].is_array()) {
        auto &c = j["grid-line-color"];
        gridLineColor = { c[2], c[1], c[0] };
    }
    if (j.contains("grid-text-color") && j["grid-text-color"].is_array()) {
        auto &c = j["grid-text-color"];
        gridTextColor = { c[2], c[1], c[0] };
    }
}

nlohmann::json Params::toJson() const {
    return {
        {"framesize",             framesize},
        {"window-func",           windowFunc},
        {"line-type",             lineType},
        {"graph-mode",            graphMode},
        {"previous-frames",       previousFrames},
        {"colormap",              colormap},
        {"fill-type",             fillType},
        {"colormap-coef",         colormapCoef},
        {"normalize-multiplier",  normalizeMultiplier},

        {"volume",                volume},
        {"border",                border},
        {"grid",                  grid},
        {"onlyaudio",             onlyAudio},
        {"debug",                 debug},

        {"window-size",   { windowSize.first, windowSize.second }},
        {"amplitude-range",{ amplitudeRange.first, amplitudeRange.second }},

        {"line-color",         { (int)lineColor[2],       (int)lineColor[1],       (int)lineColor[0] }},
        {"underline-color",    { (int)underlineColor[2],  (int)underlineColor[1],  (int)underlineColor[0] }},
        {"grid-line-color",    { (int)gridLineColor[2],   (int)gridLineColor[1],   (int)gridLineColor[0] }},
        {"grid-text-color",    { (int)gridTextColor[2],   (int)gridTextColor[1],   (int)gridTextColor[0] }}
    };
}
