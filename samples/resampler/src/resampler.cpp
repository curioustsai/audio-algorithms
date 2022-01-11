#include "speex/speex_resampler.h"

#include "CLI/CLI.hpp"
#include "sndfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    std::string inputFilePath, outputFilePath;
    int resamplerate = 48000;

    CLI::App app{"Resampler"};

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-r,--resample", resamplerate, "resample rate (Hz)")->required();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SNDFILE *infile, *outfile;
    SF_INFO sfinfo_in, sfinfo_out;

    /**
     * check input files 
     */

    if (!(infile = sf_open(inputFilePath.c_str(), SFM_READ, &sfinfo_in))) {
        printf("Not able to open input file %s.\n", inputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    }

    /**
     * open output file
     */
    sfinfo_out.samplerate = resamplerate;
    sfinfo_out.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE;
    sfinfo_out.channels = 1;
    if (!(outfile = sf_open(outputFilePath.c_str(), SFM_WRITE, &sfinfo_out))) {
        printf("Not able to open output file %s.\n", outputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

    int samplerate = sfinfo_in.samplerate;
    int num_channel = sfinfo_in.channels;
    unsigned int frameSizeIn = 1024;
    unsigned int frameSizeOut;
    short *data_in, *data_out;

    float factor = (float)resamplerate / samplerate;
    frameSizeOut = (unsigned int)(factor * frameSizeIn);

    printf("orignal samplerate: %d\t resample rate: %d\n", samplerate, resamplerate);

    data_in = new short[frameSizeIn]{0};
    data_out = new short[frameSizeOut]{0};

    int error_code;
    SpeexResamplerState *st =
        speex_resampler_init(num_channel, samplerate, resamplerate, 10, &error_code);

    clock_t tick = clock();
    int readcount = 0;

    int count = 0;
    while (1) {
        readcount = sf_read_short(infile, data_in, num_channel * frameSizeIn);
        if ((unsigned int)readcount != num_channel * frameSizeIn) break;

        // resample here
        speex_resampler_process_int(st, 0, data_in, &frameSizeIn, data_out, &frameSizeOut);

        sf_write_short(outfile, data_out, frameSizeOut);

        count++;
    }

    tick = clock() - tick;
    printf("total tick: %ld, times: %f\n", tick, ((float)tick / CLOCKS_PER_SEC));

    speex_resampler_destroy(st);

    /* Close input and output files. */
    sf_close(infile);
    sf_close(outfile);

    delete[] data_in;
    delete[] data_out;

    return 0;
}
