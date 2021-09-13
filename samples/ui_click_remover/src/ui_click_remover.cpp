#include <cstring>
#include <iostream>
#include <sndfile.h>

#include "CLI/CLI.hpp"
#include "click_removal.h"

int main(int argc, char *argv[]) {
    std::string inputFilePath;
    std::string outputFilePath;
    float threshold_all = 0.01;
    float threshold_4kHz = 0.008; //0.01
    int frameSize = 1024;
    int subframeSize = 1024;

    CLI::App app{"UI click remover"};
    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("--frameSize", frameSize, "frame size in samples");
    app.add_option("--subframeSize", subframeSize, "subframe size in samples");
    app.add_option("--threshold_all", threshold_all, "threshold of all bands");
    app.add_option("--threshold_4kHz", threshold_4kHz, "threshold above 4kHz");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SNDFILE *infile, *outfile;
    SF_INFO sfinfo_in, sfinfo_out;
    memset(&sfinfo_in, 0, sizeof(SF_INFO));
    memset(&sfinfo_out, 0, sizeof(SF_INFO));

    if (!(infile = sf_open(inputFilePath.c_str(), SFM_READ, &sfinfo_in))) {
        printf("Not able to open input file %s.\n", inputFilePath.c_str());
        return -1;
    }

    memcpy(&sfinfo_out, &sfinfo_in, sizeof(SF_INFO));
    if (!(outfile = sf_open(outputFilePath.c_str(), SFM_WRITE, &sfinfo_out))) {
        printf("Not able to open outputfile file %s.\n", outputFilePath.c_str());
        return -1;
    }

    ubnt::ClickRemoval remover(frameSize, subframeSize, threshold_all, threshold_4kHz);
#ifdef AUDIO_ALGO_DEBUG
    SNDFILE *dbgfile;
    SF_INFO sfinfo_dbg;
    memcpy(&sfinfo_dbg, &sfinfo_in, sizeof(SF_INFO));
    sfinfo_dbg.channels = remover.dbgChannels;

    std::string dbgFilePath;
    dbgFilePath.assign(outputFilePath.begin(), outputFilePath.end() - 4);
    dbgFilePath += "_dbg.wav";

    if (!(dbgfile = sf_open(dbgFilePath.c_str(), SFM_WRITE, &sfinfo_dbg))) {
        printf("Not able to open outputfile file %s.\n", dbgFilePath.c_str());
        return -1;
    }
    int16_t *dbgBuf_q15 = (int16_t *)new int16_t[frameSize * remover.dbgChannels]{0};
#endif
    int16_t *buf_q15 = (int16_t *)new int16_t[frameSize]{0};

    while (frameSize == sf_read_short(infile, buf_q15, frameSize)) {

        remover.process(buf_q15, frameSize);
        sf_write_short(outfile, buf_q15, frameSize);

#ifdef AUDIO_ALGO_DEBUG
        for (int i = 0; i < frameSize * remover.dbgChannels; i++) {
            dbgBuf_q15[i] = (int16_t)(remover.dbgInfo[i] * 32768.f);
        }
        sf_write_short(dbgfile, dbgBuf_q15, frameSize * remover.dbgChannels);
#endif
    }

    delete[] buf_q15;
    sf_close(infile);
    sf_close(outfile);

#ifdef AUDIO_ALGO_DEBUG
    delete[] dbgBuf_q15;
    sf_close(dbgfile);
#endif
}
