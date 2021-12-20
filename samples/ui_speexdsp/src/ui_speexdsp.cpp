#ifdef ENABLE_BF
#include "MicArray.h"
#endif
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

#include "compressor.h"
#include "equalizer.h"
#include "highpass.h"
#include "lowpass.h"
#include "config.h"

#include "CLI/CLI.hpp"
#include "sndfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(int argc, char **argv) {
    SpeexPreprocessState *den_st;
    SpeexEchoState *echo_st;
#ifdef ENABLE_BF
    void *hMicArray;
    int fftlen = 512;
    bool enable_bf = false;
#endif

    std::string inputFilePath, outputFilePath, refFilePath, configFilePath;
    short *data, *mic_data, *ref_data, *echo_out, *bf_out;
    float *f32data, *f32data_out;
    int parameter;
    int count = 0;

    CLI::App app{"UI speexdsp"};

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-r,--refFile", refFilePath, "specify an reference file")
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-c,--config", configFilePath, "specify an configu file")->required();

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
    bool enable_aec = config.moduleParam.aec;
#ifdef ENABLE_BF
    enable_bf = (num_channel > 1) ? true : false;
#endif

    mic_data = (short *)malloc(num_channel * frameSize * sizeof(short));
    bf_out = (short *)malloc(frameSize * sizeof(short));
    f32data = (float *)malloc(frameSize * sizeof(float));
    f32data_out = (float *)malloc(frameSize * sizeof(float));

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

#ifdef ENABLE_BF
    if (enable_bf) { MicArray_Init(&hMicArray, sample_rate, num_channel, fftlen, frameSize); }
#endif

    /* denoise */
    parameter = config.moduleParam.denoise;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_DENOISE, &parameter);
    parameter = config.denoiseParam.nsLevel;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &parameter);

    /* agc */
    parameter = config.moduleParam.agc;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC, &parameter);
    parameter = config.agcParam.agcTarget;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC_TARGET, &parameter);

    /* EQ */
    ubnt::Equalizer *eq = new ubnt::Equalizer(4000, 6, 1, sample_rate);

    /* DRC */
    sf_compressor_state_st compressor_st;
    float pregain = 6;
    float postgain = 0;
    float knee = 1;
    float threshold = -12.0f;
    float ratio = 3.f;
    float threshold_agg = -6.0f;
    float ratio_agg = 12.f;
    float threshold_expander = -96;
    float ratio_expander = 1.0f;
    float attack = 0.003f;
    float release = 0.250f;

    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, knee, threshold, ratio,
                  threshold_agg, ratio_agg, threshold_expander, ratio_expander, attack, release);

    clock_t tick = clock();
    int readcount = 0;

    using HPF = ubnt::HighPassFilter;
    HPF *hpf = new HPF(HPF::SampleRate::Fs_16kHz, HPF::CutoffFreq::Fc_500Hz);
    using LPF = ubnt::LowPassFilter;
    LPF *lpf = new LPF(LPF::SampleRate::Fs_16kHz, LPF::CutoffFreq::Fc_6kHz);

    while (1) {
        readcount = sf_read_short(infile, mic_data, num_channel * frameSize);
        if (readcount != num_channel * frameSize) break;

        if (enable_aec) {
            readcount = sf_read_short(refile, ref_data, frameSize);
            if (readcount != frameSize) break;
            speex_echo_cancellation(echo_st, mic_data, ref_data, echo_out);
            data = echo_out;
        } else {
            data = mic_data;
        }

#ifdef ENABLE_BF
        if (num_channel >= 2) {
            MicArray_Process(hMicArray, data, bf_out);
            data = bf_out;
        }
#endif

        hpf->Process(data, data, frameSize);
        speex_preprocess_run(den_st, data);
        sf_write_short(outfile, data, frameSize);

        /* HPF / LPF */
        lpf->Process(data, data, frameSize);

        /* Parameter EQ */
        // eq->process(data, data, frameSize);

        /* DRC */
        // sf_compressor_process(&compressor_st, frameSize, f32data_out, f32data_out);

        count++;
    }

#ifdef ENABLE_BF
    if (enable_bf) MicArray_Release(hMicArray);
#endif
    if (enable_aec) speex_echo_state_destroy(echo_st);
    speex_preprocess_state_destroy(den_st);

    tick = clock() - tick;
    printf("total tick: %ld, times: %f\n", tick, ((float)tick / CLOCKS_PER_SEC));

    /* Close input and output files. */
    sf_close(infile);
    sf_close(outfile);

    free(bf_out);
    free(mic_data);
    free(f32data);
    free(f32data_out);

    if (enable_aec) {
        sf_close(refile);
        free(echo_out);
        free(ref_data);
    }

    delete eq;

    return 0;
}
