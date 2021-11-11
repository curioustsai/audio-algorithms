#ifdef ENABLE_BF
#include "MicArray.h"
#endif
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"

#include "CLI/CLI.hpp"
#include <sndfile.h>
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

    std::string inputFilePath, outputFilePath, refFilePath;
    short *data, *mic_data, *ref_data, *echo_out, *bf_out;
    int parameter;
    int count = 0;
    int frame_size = 256;
    int nsLevel = 15;
    int agcTarget = 16000;
    int tail_length = 1024;
    bool enable_aec = false;
    bool enable_ns = true, disable_ns = false;
    bool enable_agc = false;

    CLI::App app{"UI speexdsp"};

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-r,--refFile", refFilePath, "specify an reference file")
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_flag("--disable-ns", disable_ns, "disable ns");
    app.add_flag("--enable-agc", enable_agc, "enable agc");
    app.add_option("--nsLevel", nsLevel, "noise suppression level")->check(CLI::Number);
    app.add_option("--agcTarget", agcTarget, "agc target sample value")->check(CLI::Number);
    app.add_option("--tailLength", tail_length, "tail length in samples")->check(CLI::Number);
    app.add_option("--frameSize", frame_size, "frame size in samples")->check(CLI::Number);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    enable_ns = !disable_ns;
    SNDFILE *infile, *outfile, *refile;
    SF_INFO sfinfo_in, sfinfo_out, sfinfo_ref;
    memset(&sfinfo_in, 0, sizeof(SF_INFO));

    /**
     * check input files 
     */
    if (!refFilePath.empty()) {
        if (!(refile = sf_open(refFilePath.c_str(), SFM_READ, &sfinfo_ref))) {
            printf("Not able to open ref file %s.\n", refFilePath.c_str());
            puts(sf_strerror(NULL));
            return 1;
        }
        enable_aec = true;
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
#ifdef ENABLE_BF
    enable_bf = (num_channel > 1) ? true : false;
#endif

    mic_data = (short *)malloc(num_channel * frame_size * sizeof(short));
    bf_out = (short *)malloc(frame_size * sizeof(short));

    if (enable_aec) {
        ref_data = (short *)malloc(frame_size * sizeof(short));
        echo_out = (short *)malloc(num_channel * frame_size * sizeof(short));
        echo_st = speex_echo_state_init_mc(frame_size, tail_length, num_channel, 1);
        den_st = speex_preprocess_state_init(frame_size, sample_rate);
        speex_echo_ctl(echo_st, SPEEX_ECHO_SET_SAMPLING_RATE, &sample_rate);
        speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_st);
    } else {
        den_st = speex_preprocess_state_init(frame_size, sample_rate);
    }

#ifdef ENABLE_BF
    if (enable_bf) {
        MicArray_Init(&hMicArray, sample_rate, num_channel, fftlen, frame_size);
    }
#endif

    /* denoise */
    parameter = enable_ns;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_DENOISE, &parameter);
    parameter = nsLevel;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &parameter);

    /* agc */
    parameter = enable_agc;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC, &parameter);
    parameter = agcTarget;
    if (enable_agc) { printf("agc target: %d\n", agcTarget); }
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC_TARGET, &parameter);

    clock_t tick = clock();
    int readcount = 0;

    while (1) {
        readcount = sf_read_short(infile, mic_data, num_channel * frame_size);
        if (readcount != num_channel * frame_size) break;

        if (enable_aec) {
            readcount = sf_read_short(refile, ref_data, frame_size);
            if (readcount != frame_size) break;
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

        speex_preprocess_run(den_st, data);
        sf_write_short(outfile, data, frame_size);
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

    if (enable_aec) {
        sf_close(refile);
        free(echo_out);
        free(ref_data);
    }


    return 0;
}
