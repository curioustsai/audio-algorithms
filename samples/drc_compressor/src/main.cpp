#include "CLI/CLI.hpp"
#include "CLI/Validators.hpp"
#include "compressor.h"
#include "nlohmann/json.hpp"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct DrcParam {
    int frameSize{1024};
    float pregain{0.0f};
    float postgain{0.0f};

    float thresholdExpander{-60.f};
    float ratioExpander{1.f};
    float thresholdCompressor{-60.f};
    float ratioCompressor{1.f};
    float thresholdLimiter{-12.f};
    float ratioLimiter{4};

    float knee{1.0f};
    float attack{0.003f};
    float release{0.250f};
};

void ParseConfigParam(nlohmann::json &json, DrcParam &drcParam) {
    drcParam.frameSize = json["frameSize"];
    drcParam.pregain = json["drc"]["pregain"];
    drcParam.postgain = json["drc"]["postgain"];

    drcParam.thresholdExpander = json["drc"]["thresholdExpander"];
    drcParam.ratioExpander = json["drc"]["ratioExpander"];
    drcParam.thresholdCompressor = json["drc"]["thresholdCompressor"];
    drcParam.ratioCompressor = json["drc"]["ratioCompressor"];
    drcParam.thresholdLimiter = json["drc"]["thresholdLimiter"];
    drcParam.ratioLimiter = json["drc"]["ratioLimiter"];

    drcParam.knee = json["drc"]["knee"];
    drcParam.attack = json["drc"]["attack"];
    drcParam.release = json["drc"]["release"];
}

int main(int argc, char **argv) {
    std::string inputFilePath, outputFilePath, configFilePath;
    short *data;
    float *input_float;
    float *output_float;

    CLI::App app{"DRC compressor"};

    app.add_option("-i,--inFile", inputFilePath, "input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "output file")->required();
    app.add_option("-c,--config", configFilePath, "config json file")
        ->required()
        ->check(CLI::ExistingFile);

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

    DrcParam drcParam;
    std::ifstream ifs(configFilePath);
    nlohmann::json jf = nlohmann::json::parse(ifs);
    std::cout << jf.dump(4) << std::endl;
    ParseConfigParam(jf, drcParam);

    int frameSize = drcParam.frameSize;

    data = (short *)new short[frameSize];
    input_float = (float *)new float[frameSize];
    output_float = (float *)new float[frameSize];

    clock_t tick = clock();

    int count = 0;

    sf_compressor_state_st compressor_st;
    sf_simplecomp(&compressor_st, sample_rate, drcParam.pregain, drcParam.postgain, drcParam.knee,
                  drcParam.thresholdCompressor, drcParam.ratioCompressor, drcParam.thresholdLimiter,
                  drcParam.ratioLimiter, drcParam.thresholdExpander, drcParam.ratioExpander,
                  drcParam.attack, drcParam.release);

    while (frameSize == sf_read_short(infile, data, frameSize)) {
        for (int i = 0; i < frameSize; i++) { input_float[i] = (float)data[i] / 32768.0f; }
        sf_compressor_process(&compressor_st, frameSize, input_float, output_float);
        for (int i = 0; i < frameSize; i++) { data[i] = (short)(output_float[i] * 32768.0f); }
        sf_write_short(outfile, data, frameSize);
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
