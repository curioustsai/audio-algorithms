#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

#include "compressor.h"
#include "config.h"
#include "equalizer.h"
#include "gain.h"
#include "highpass.h"
#include "lowpass.h"

#include "CLI/CLI.hpp"
#include "sndfile.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using Equalizer = ubnt::Equalizer;
using HPF = ubnt::HighPassFilter;
using LPF = ubnt::LowPassFilter;
using Gain = ubnt::Gain;

int main(int argc, char **argv) {
    SpeexPreprocessState *den_st;
    SpeexEchoState *echo_st;

    std::string inputFilePath, outputFilePath, refFilePath, configFilePath;
    short *data, *mic_data, *ref_data, *echo_out;
    float *f32data;
    int parameter;
    int count = 0;

    CLI::App app{"UI speexdsp"};

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-r,--refFile", refFilePath, "specify an reference file")
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-c,--config", configFilePath, "specify an config file")->required();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    std::ifstream ifs(configFilePath);
    nlohmann::json jsonFile = nlohmann::json::parse(ifs);
    Config config;
    ParseConfig(jsonFile, config);

    SNDFILE *infile, *outfile, *refile;
    SF_INFO sfinfo_in, sfinfo_out, sfinfo_ref;

    /**
     * check input files 
     */
    if (!refFilePath.empty()) {
        if (!(refile = sf_open(refFilePath.c_str(), SFM_READ, &sfinfo_ref))) {
            printf("Not able to open ref file %s.\n", refFilePath.c_str());
            puts(sf_strerror(NULL));
            return 1;
        }
    }

    if (!(infile = sf_open(inputFilePath.c_str(), SFM_READ, &sfinfo_in))) {
        printf("Not able to open input file %s.\n", inputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    }

    if (!refFilePath.empty() && sfinfo_ref.channels > 1) {
        printf("Only support mono aec now.\n");
        return 1;
    } else if (!refFilePath.empty() && sfinfo_in.samplerate != sfinfo_ref.samplerate) {
        printf("sample rate doesn't match between near-end and far-end.\n");
        return 1;
    }

    assert(sfinfo_in.format & SF_FORMAT_WAV);
    assert(sfinfo_in.format & SF_FORMAT_PCM_16);
    assert((sfinfo_in.channels == 1));

    /**
     * open output file
     */
    memcpy(&sfinfo_out, &sfinfo_in, sizeof(SF_INFO));
    sfinfo_out.channels = 1;
    if (!(outfile = sf_open(outputFilePath.c_str(), SFM_WRITE, &sfinfo_out))) {
        printf("Not able to open output file %s.\n", outputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

    int sample_rate = sfinfo_in.samplerate;
    int num_channel = sfinfo_in.channels;
    int frameSize = config.frameSize;
    int tail_length = config.aecParam.tailLength;
    bool enable_aec = config.enable.aec;

    mic_data = (short *)malloc(num_channel * frameSize * sizeof(short));
    f32data = (float *)malloc(frameSize * sizeof(float));

    float micgain_dB = jsonFile["micgain"]["gaindB"];
    Gain micgain(micgain_dB);

    if (enable_aec) {
        ref_data = (short *)malloc(frameSize * sizeof(short));
        echo_out = (short *)malloc(num_channel * frameSize * sizeof(short));
        echo_st = speex_echo_state_init_mc(frameSize, tail_length, num_channel, 1);
        den_st = speex_preprocess_state_init(frameSize, sample_rate);
        speex_echo_ctl(echo_st, SPEEX_ECHO_SET_SAMPLING_RATE, &sample_rate);
        speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_st);
    } else {
        den_st = speex_preprocess_state_init(frameSize, sample_rate);
    }

    /* denoise */
    parameter = config.enable.denoise;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_DENOISE, &parameter);
    parameter = config.denoiseParam.nsLevel;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &parameter);

    /* agc */
    parameter = config.enable.agc;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC, &parameter);
    parameter = config.agcParam.agcTarget;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC_TARGET, &parameter);

    /* EQ */
    int numEQ = config.eqParamSet.numEQ;
    Equalizer **eqs = new Equalizer *[numEQ];
    for (int i = 0; i < numEQ; ++i) {
        int f0 = config.eqParamSet.eqParamVec.at(i).f0;
        float gain = config.eqParamSet.eqParamVec.at(i).gain;
        float Q = config.eqParamSet.eqParamVec.at(i).Q;

        eqs[i] = new Equalizer(f0, sample_rate, gain, Q);
    }

    HPF::SampleRate HPF_FS = HPF::SampleRate::Fs_16kHz;
    LPF::SampleRate LPF_FS = LPF::SampleRate::Fs_16kHz;
    switch (sample_rate) {
        case 48000:
            HPF_FS = HPF::SampleRate::Fs_48kHz;
            LPF_FS = LPF::SampleRate::Fs_48kHz;
            break;
        case 3200:
            HPF_FS = HPF::SampleRate::Fs_32kHz;
            LPF_FS = LPF::SampleRate::Fs_32kHz;
            break;
        case 16000:
            HPF_FS = HPF::SampleRate::Fs_16kHz;
            LPF_FS = LPF::SampleRate::Fs_16kHz;
            break;
        case 8000:
            HPF_FS = HPF::SampleRate::Fs_8kHz;
            config.enable.lowpass = false;
            break;
        default:
            printf("not support");
            config.enable.highpass = false;
            config.enable.lowpass = false;
            return 1;
    }

    HPF::CutoffFreq HPF_FC = static_cast<HPF::CutoffFreq>(jsonFile["highpass"]["f0"]);
    LPF::CutoffFreq LPF_FC = static_cast<LPF::CutoffFreq>(jsonFile["lowpass"]["f0"]);
    HPF *hpf = new HPF(HPF_FS, HPF_FC);
    LPF *lpf = new LPF(LPF_FS, LPF_FC);

    /* DRC */
    sf_compressor_state_st compressor_st;
    float pregain = config.drcParam.pregain;
    float postgain = config.drcParam.postgain;
    float knee = config.drcParam.knee;
    float threshold = config.drcParam.threshold;
    float ratio = config.drcParam.ratio;
    float threshold_agg = config.drcParam.threshold_agg;
    float ratio_agg = config.drcParam.ratio_agg;
    float threshold_expander = config.drcParam.threshold_expander;
    float ratio_expander = config.drcParam.ratio_expander;
    float attack = config.drcParam.attack;
    float release = config.drcParam.release;

    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, knee, threshold, ratio,
                  threshold_agg, ratio_agg, threshold_expander, ratio_expander, attack, release);

    clock_t tick = clock();
    int readcount = 0;

    while (1) {
        readcount = sf_read_short(infile, mic_data, frameSize);
        if (config.enable.micgain) { micgain.Process(mic_data, frameSize); }
        if (readcount != frameSize) break;

        if (enable_aec) {
            readcount = sf_read_short(refile, ref_data, frameSize);
            if (readcount != frameSize) break;
            speex_echo_cancellation(echo_st, mic_data, ref_data, echo_out);
            data = echo_out;
        } else {
            data = mic_data;
        }

        if (config.enable.highpass) hpf->Process(data, data, frameSize);
        if (config.enable.denoise) speex_preprocess_run(den_st, data);
        if (config.enable.lowpass) lpf->Process(data, data, frameSize);
        if (config.enable.equalizer)
            for (int i = 0; i < numEQ; ++i) { eqs[i]->Process(data, data, frameSize); }

        if (config.enable.drc) sf_compressor_process(&compressor_st, frameSize, f32data, f32data);

        sf_write_short(outfile, data, frameSize);

        count++;
    }

    if (enable_aec) speex_echo_state_destroy(echo_st);
    speex_preprocess_state_destroy(den_st);

    tick = clock() - tick;
    printf("total tick: %ld, times: %f\n", tick, ((float)tick / CLOCKS_PER_SEC));

    /* Close input and output files. */
    sf_close(infile);
    sf_close(outfile);

    free(mic_data);
    free(f32data);

    if (enable_aec) {
        sf_close(refile);
        free(echo_out);
        free(ref_data);
    }

    for (int i = 0; i < numEQ; ++i) { delete eqs[i]; }
    delete[] eqs;
    delete hpf;
    delete lpf;

    return 0;
}
