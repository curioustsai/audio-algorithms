#include "CLI/CLI.hpp"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "CLI/Validators.hpp"
#include "nlohmann/json.hpp"
#include "tone_generator.h"

int main(int argc, char **argv) {
    std::string outputFilePath;
    short *data;
    int frameSize = 1024;
    int numFrames = 100;
    int bufSize = frameSize * numFrames;
    int samplerate = 48000;
    int f1 = 100;
    int f2 = 23000;
    int amplitude = 16384;

    CLI::App app{"Tone Generator"};

    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-r,--samplerate", samplerate, "specify sample rate")->check(CLI::Number);
    app.add_option("--f1", f1, "start frequency")->check(CLI::Number);
    app.add_option("--f2", f2, "end frequency")->check(CLI::Number);
    app.add_option("--amp", amplitude, "amplitude ranges 0~32767")->check(CLI::Number);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SNDFILE *outfile;

    SF_INFO sfinfo;
    sfinfo.channels = 1;
    sfinfo.format = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;
    sfinfo.samplerate = samplerate;
    sfinfo.frames = 0;

    if (!(outfile = sf_open(outputFilePath.c_str(), SFM_WRITE, &sfinfo))) {
        printf("Not able to open output file %s.\n", outputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

    printf("sample rate: %d\n", samplerate);

    data = (short *)new short[bufSize];
    generate_chirp_int16(data, bufSize, samplerate, f1, f2, amplitude);
    sf_write_short(outfile, data, bufSize);

    /* Close input and output files. */
    sf_close(outfile);

    delete[] data;

    return 0;
}
