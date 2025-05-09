// Params.hpp
#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <utility>
#include <opencv2/core.hpp>
#include <nlohmann/json.hpp>

struct Params {
    int framesize = 65536;
    int windowFunc = 9;
    int lineType = 0;
    int graphMode = 1;
    int previousFrames = 20;
    int colormap = 1;
    int fillType = 2;
    int colormapCoef = 127;
    int normalizeMultiplier = 20;

    bool border = false;
    bool grid = false;
    bool onlyAudio = false;
    bool debug = false;

    float volume = 0.8f;

    std::pair<int, int> windowSize = {1000, 1800};
    std::pair<int, int> amplitudeRange = {-90, 50};

    cv::Scalar lineColor        = {255, 255, 255};
    cv::Scalar underlineColor   = {127, 127, 127};
    cv::Scalar gridLineColor    = {79,  73,  80};
    cv::Scalar gridTextColor    = {51, 186, 243};

    void loadFromJson(const nlohmann::json& j);
    nlohmann::json toJson() const;
};

#endif // PARAMS_HPP
