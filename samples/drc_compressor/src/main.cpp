#include "CLI/CLI.hpp"
#include "compressor.h"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    std::string inputFilePath, outputFilePath;
    short *data;
    float *input_float;
    float *output_float;
    int frame_size = 1024;
    float pregain = 0;  //Decibel amount to perform gain before compression (0 to 40)
    float postgain = 0; //Decibel amount to perform gain after compression (0 to 40)
    float knee = 1;        //Decibel width of the knee (0 to 40)"

    float threshold_expander = -60; //Decibel level that triggers the compression (-100 to 0)"
    float ratio_expander = 2;     //Ratio of compression after the threshold (1 to 20)"

    float threshold = -24; //Decibel level that triggers the compression (-100 to 0)"
    float ratio = 2;       //Ratio of compression after the threshold (1 to 20)"

    float threshold_agg = -12; //Decibel level that triggers the compression aggressive (-100 to 0)"
    float ratio_agg = 4;       //Ratio of compression aggressive after the threshold (1 to 20)"

    float attack = 0.003f;  //Seconds for the compression to kick in (0 to 1)"
    float release = 0.250f; //Seconds for the compression to release (0 to 1)"

    CLI::App app{"DRC compressor"};

    app.add_option("-i,--inFile", inputFilePath, "input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "output file")->required();
    app.add_option("--frameSize", frame_size, "frame size in samples, default: 1024")
        ->check(CLI::Number);
    app.add_option("--pregain", pregain, "pregain before compression, default: 0 dB");
    app.add_option("--postgain", postgain, "postgain after compression, default: 0 dB");
    app.add_option("--knee", knee, "width of soft knee, defalt: 1 dB");

    app.add_option("--threshold_expander", threshold_expander,
                   "threshold for expander, default: -60 dB");
    app.add_option("--ratio_expander", ratio_expander, "expander ratio, defaul: 2");

    app.add_option("--threshold", threshold, "threshold for compression, default: -24 dB");
    app.add_option("--ratio", ratio, "compression ratio, defaul: 2");

    app.add_option("--threshold_agg", threshold_agg,
                   "threshold for compression aggressive, default: -12 dB");
    app.add_option("--ratio_agg", ratio_agg, "compression ratio aggressive, defaul: 4");

    app.add_option("--attack", attack, "compression attack time, default: 0.003");
    app.add_option("--release", release, "compression release time, default: 0.250");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SNDFILE *infile, *outfile;

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));

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

    if (sfinfo.channels > 1) {
        printf("Only support single channel now\n");
        puts(sf_strerror(NULL));
        return 1;
    }

    int sample_rate = sfinfo.samplerate;
    printf("sample rate: %d\n", sample_rate);

    data = (short *)new short[frame_size];
    input_float = (float *)new float[frame_size];
    output_float = (float *)new float[frame_size];

    clock_t tick = clock();

    int count = 0;

    sf_compressor_state_st compressor_st;
    sf_simplecomp(&compressor_st, sample_rate, pregain, postgain, knee, threshold, ratio,
                  threshold_agg, ratio_agg, threshold_expander, ratio_expander, attack, release);

    while (frame_size == sf_read_short(infile, data, frame_size)) {
        for (int i = 0; i < frame_size; i++) { input_float[i] = (float)data[i] / 32768.0f; }
        sf_compressor_process(&compressor_st, frame_size, input_float, output_float);
        for (int i = 0; i < frame_size; i++) { data[i] = (short)(output_float[i] * 32768.0f); }
        sf_write_short(outfile, data, frame_size);
        count++;
    }

    tick = clock() - tick;
    printf("total tick: %ld, times: %f\n", tick, ((float)tick / CLOCKS_PER_SEC));

    /* Close input and output files. */
    sf_close(infile);
    sf_close(outfile);

    delete[] data;
    delete[] input_float;
    delete[] output_float;

    return 0;
}
