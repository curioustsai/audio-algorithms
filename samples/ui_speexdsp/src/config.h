#pragma once

#include "nlohmann/json.hpp"
#include <string>
#include <vector>

struct ModuleParam {
    bool aec{false};
    bool denoise{true};
    bool agc{false};
    bool equalizer{false};
    bool highpass{false};
    bool lowpass{false};
};

struct AecParam {
    int tailLength{1024};
};

struct DenoiseParam {
    int nsLevel{10};
};

struct AgcParam {
    int agcTarget{16000};
};

struct EqParam {
    int f0{2000};
    float gain{0};
    float Q{1};
};

struct EqParamSet {
    int numEQ{0};
    std::vector<EqParam> eqParamVec;
};

struct HpfParam {
    int f0{100};
};
struct LpfParam {
    int f0{8000};
};

struct DrcParam {
    float pregain{0};
    float postgain{0};
    float knee{1};
    float threshold{-12.0};
    float ratio{3.0};
    float threshold_agg{-6};
    float ratio_agg{12};
    float threshold_expander{-96};
    float ratio_expander{1.0f};
    float attack{0.003};
    float release{0.250};
};

struct Config {
    int frameSize;
    ModuleParam moduleParam;
    AecParam aecParam;
    DenoiseParam denoiseParam;
    AgcParam agcParam;
    EqParamSet eqParamSet;
    HpfParam hpfParam;
    LpfParam lpfParam;
    DrcParam drcParam;
};

void ParseConfig(nlohmann::json &jsonFile, Config &config);
