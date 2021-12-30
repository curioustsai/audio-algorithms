#include "CLI/CLI.hpp"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "CLI/Validators.hpp"
#include "equalizer.h"
#include "nlohmann/json.hpp"

struct EQParam {
    float f0;
    float gain;
    float Q;
};

int main(int argc, char **argv) {
    std::string inputFilePath, outputFilePath;
    std::string jsonConfigPath;
    short *data;
    int frame_size = 1024;

    CLI::App app{"UI Sample App"};

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-c,--config", jsonConfigPath, "specify an json config file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("--frameSize", frame_size, "frame size in samples")->check(CLI::Number);

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

    int sample_rate = sfinfo.samplerate;
    printf("sample rate: %d\n", sample_rate);

    // read json config file
    std::ifstream ifs(jsonConfigPath);
    nlohmann::json jf = nlohmann::json::parse(ifs);

    int NUM_EQ = jf["NUM_EQ"];

    EQParam *param = new EQParam[NUM_EQ];
    ubnt::Equalizer **eqs = new ubnt::Equalizer *[NUM_EQ];

    for (int i = 0; i < NUM_EQ; ++i) {
        std::string key = "EQ" + std::to_string(i);
        param[i].f0 = jf[key]["f0"];
        param[i].gain = jf[key]["gain"];
        param[i].Q = jf[key]["Q"];
    }

    for (int i = 0; i < NUM_EQ; ++i) {
        eqs[i] = new ubnt::Equalizer(param[i].f0, sample_rate, param[i].gain, param[i].Q);
    }

    data = (short *)new short[frame_size];

    clock_t tick = clock();

    int count = 0;

    while (frame_size == sf_read_short(infile, data, frame_size)) {
        /* run something here */
        /* run something here */
        /* run something here */
        for (int i = 0; i < NUM_EQ; ++i) {
           eqs[i]->Process(data, data, frame_size);
        }
        sf_write_short(outfile, data, frame_size);
        count++;
    }

    tick = clock() - tick;
    printf("total tick: %ld, times: %f\n", tick, ((float)tick / CLOCKS_PER_SEC));

    /* Close input and output files. */
    sf_close(infile);
    sf_close(outfile);

    delete[] data;
    delete[] param;
    for (int i = 0; i < NUM_EQ; ++i) { delete eqs[i]; }
    delete[] eqs;

    return 0;
}
