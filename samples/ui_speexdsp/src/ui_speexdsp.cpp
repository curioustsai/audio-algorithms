#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "CLI/CLI.hpp"
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    std::string inputFilePath, outputFilePath, refFilePath;
    short *data, *mic_data, *ref_data, *echo_out;
    int parameter;
    SpeexPreprocessState *den_st;
    SpeexEchoState *echo_st;
    int count = 0;
	int frame_size = 1024;
    int nsLevel = 15;
    int agcTarget = 16000;
    int tail_length = 1024;
    bool enable_agc = false;
    bool enable_aec = false;

    CLI::App app{"UI speexdsp"};

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-r,--refFile", refFilePath, "specify an reference file")
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_flag("--enable-agc", enable_agc, "enable agc");
    app.add_option("--nsLevel", nsLevel, "noise suppression level")->check(CLI::Number);
    app.add_option("--agcTarget", agcTarget, "agc target sample value")->check(CLI::Number);
    app.add_option("--tailLength", tail_length, "tail length in samples")->check(CLI::Number);
    app.add_option("--framesize", frame_size, "frame size in samples")->check(CLI::Number);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SNDFILE *infile, *outfile, *refile;

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

    if (!refFilePath.empty()) {
        if (!(refile = sf_open(refFilePath.c_str(), SFM_READ, &sfinfo))) {
            printf("Not able to open ref file %s.\n", refFilePath.c_str());
            puts(sf_strerror(NULL));
            return 1;
        }
        enable_aec = true;
    }

    if (!(infile = sf_open(inputFilePath.c_str(), SFM_READ, &sfinfo))) {
        printf("Not able to open input file %s.\n", inputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

    if (!(outfile = sf_open(outputFilePath.c_str(), SFM_WRITE, &sfinfo))) {
        printf("Not able to open output file %s.\n", outputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

    int sample_rate = sfinfo.samplerate;

    mic_data = (short *)malloc(frame_size * sizeof(short));
    if (enable_aec) {
        ref_data = (short *)malloc(frame_size * sizeof(short));
        echo_out = (short *)malloc(frame_size * sizeof(short));
        echo_st = speex_echo_state_init(frame_size, tail_length);
        den_st = speex_preprocess_state_init(frame_size, sample_rate);
        speex_echo_ctl(echo_st, SPEEX_ECHO_SET_SAMPLING_RATE, &sample_rate);
        speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_st);
    } else {
        den_st = speex_preprocess_state_init(frame_size, sample_rate);
    }

    /* denoise */
    parameter = 1;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_DENOISE, &parameter);
    parameter = nsLevel;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &parameter);

    /* agc */
    parameter = enable_agc;
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC, &parameter);
    parameter = agcTarget;
    printf("agc target: %d\n", agcTarget);
    speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC_TARGET, &parameter);
    // parameter = 20; // FIXME: not sure how to set
    // speex_preprocess_ctl(den_st, SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &parameter);

    clock_t tick = clock();

    int readcount = 0;

    while (1) {
        readcount = sf_read_short(infile, mic_data, frame_size);
        if (readcount != frame_size) break;

        if (enable_aec) {
            readcount = sf_read_short(refile, ref_data, frame_size);
            if (readcount != frame_size) break;
            speex_echo_cancellation(echo_st, mic_data, ref_data, echo_out);
            data = echo_out;
        } else {
            data = mic_data;
        }
        speex_preprocess_run(den_st, data);
        /* int vad; */
        /* vad = speex_preprocess_run(den_st, data); */
        /*fprintf (stderr, "%d\n", vad);*/
        sf_write_short(outfile, data, frame_size);
        count++;
    }
    if (enable_aec) speex_echo_state_destroy(echo_st);
    speex_preprocess_state_destroy(den_st);

    tick = clock() - tick;
    printf("total tick: %ld, times: %f\n", tick, ((float)tick / CLOCKS_PER_SEC));

    /* Close input and output files. */
    sf_close(infile);
    sf_close(outfile);
	if (enable_aec) sf_close(refile);

    free(data);

    return 0;
}
