#include "config.h"

void ParseConfig(nlohmann::json &jsonFile, Config &config) {
    config.frameSize = jsonFile["frameSize"];
    config.enable.aec = jsonFile["enable"]["aec"];
    config.enable.denoise = jsonFile["enable"]["denoise"];
    config.enable.agc = jsonFile["enable"]["agc"];
    config.enable.equalizer = jsonFile["enable"]["equalizer"];
    config.enable.highpass = jsonFile["enable"]["highpass"];
    config.enable.lowpass = jsonFile["enable"]["lowpass"];
    config.enable.drc = jsonFile["enable"]["drc"];

    config.aecParam.tailLength = jsonFile["aec"]["tailLength"];

    config.denoiseParam.nsLevel = jsonFile["denoise"]["nsLevel"];
    config.agcParam.agcTarget = jsonFile["agc"]["agcTarget"];

    // // eq
    config.eqParamSet.numEQ = jsonFile["equalizer"]["num_eq"];
    for (int i = 0; i < config.eqParamSet.numEQ; ++i) {
        EqParam eqParam;
        std::string eqIndex = "eq" + std::to_string(i);
        eqParam.f0 = jsonFile["equalizer"][eqIndex]["f0"];
        eqParam.gain = jsonFile["equalizer"][eqIndex]["gain"];
        eqParam.Q = jsonFile["equalizer"][eqIndex]["Q"];

        config.eqParamSet.eqParamVec.push_back(eqParam);
        
    }

    config.hpfParam.f0 = jsonFile["highpass"]["f0"];
    config.lpfParam.f0 = jsonFile["lowpass"]["f0"];

    //drc
    config.drcParam.pregain = jsonFile["drc"]["pregain"];
    config.drcParam.postgain = jsonFile["drc"]["postgain"];
    config.drcParam.knee = jsonFile["drc"]["knee"];
    config.drcParam.threshold = jsonFile["drc"]["threshold"];
    config.drcParam.ratio = jsonFile["drc"]["ratio"];
    config.drcParam.threshold_agg = jsonFile["drc"]["threshold_agg"];
    config.drcParam.ratio_agg = jsonFile["drc"]["ratio_agg"];
    config.drcParam.threshold_expander = jsonFile["drc"]["threshold_expander"];
    config.drcParam.ratio_expander = jsonFile["drc"]["ratio_expander"];
    config.drcParam.attack = jsonFile["drc"]["attack"];
    config.drcParam.release = jsonFile["drc"]["release"];
};
