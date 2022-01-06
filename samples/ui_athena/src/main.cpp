#include "CLI/CLI.hpp"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "CLI/Validators.hpp"
#include "nlohmann/json.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "dios_ssp_api.h"

#ifdef __cplusplus
}
#endif

int main(int argc, char **argv) {
    std::string inputFilePath, refFilePath, outputFilePath;
    std::string jsonConfigPath;

    CLI::App app{"Athena app"};

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-r,--refFile", refFilePath, "specify an ref file");
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-c,--config", jsonConfigPath, "specify an json config file")
        ->required()
        ->check(CLI::ExistingFile);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SF_INFO info;
    SNDFILE *inwav = sf_open(inputFilePath.c_str(), SFM_READ, &info);
    if (NULL == inwav) {
        fprintf(stderr, "open %s failed\n", inputFilePath.c_str());
        return -1;
    }

    SF_INFO rinfo;
    SNDFILE *refwav;
    if (!refFilePath.empty()) {
        refwav = sf_open(refFilePath.c_str(), SFM_READ, &rinfo);
        if (NULL == refwav) {
            fprintf(stderr, "open %s failed\n", refFilePath.c_str());
            sf_close(inwav);
            return -1;
        }
    }

    SF_INFO onfo;
    onfo.channels = 1;
    onfo.samplerate = info.samplerate;
    onfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    onfo.sections = 0;
    onfo.seekable = 1;
    SNDFILE *outwav = sf_open(outputFilePath.c_str(), SFM_WRITE, &onfo);
    if (NULL == outwav) {
        fprintf(stderr, "open %s failed\n", outputFilePath.c_str());
        sf_close(inwav);
        sf_close(refwav);
        return -2;
    }

    // read json config file
    std::ifstream ifs(jsonConfigPath);
    nlohmann::json jf = nlohmann::json::parse(ifs);
    std::string js_str = jf.dump();
    // std::cout << "json string: " << js_str << std::endl;

    // // iterate all items
    // for (auto & el : jf.items()) {
    //     std::cout << el.key() << " : " << el.value() << std::endl;
    // }

    objSSP_Param param;
    param.AEC_KEY = jf["AEC_KEY"];
    param.NS_KEY = jf["NS_KEY"];
    param.AGC_KEY = jf["AGC_KEY"];
    param.HPF_KEY = jf["HPF_KEY"];
    param.BF_KEY = jf["BF_KEY"];
    param.DOA_KEY = jf["DOA_KEY"];
    param.mic_num = jf["mic_num"];
    param.ref_num = jf["ref_num"];
    param.loc_phi = jf["loc_phi"];
    memset(param.mic_coord, 0, sizeof(param.mic_coord));

    void *hssp = dios_ssp_init_api(&param);
    if (NULL == hssp) {
        fprintf(stderr, "dios_ssp_init_api failed\n");
        return -4;
    }

    int ret = dios_ssp_reset_api(hssp, &param);
    if (ret) {
        fprintf(stderr, "dios_ssp_reset_api failed, return %d\n", ret);
        return -5;
    }

    int framelen = 128;
    while (1) {
        short micbuf[framelen];
        short refbuf[framelen];
        short sspbuf[framelen];

        int readsize = sf_readf_short(inwav, micbuf, framelen);
        if (readsize != framelen) {
            fprintf(stderr, "processing all data, exiting...\n");
            break;
        }

        if (!refFilePath.empty() && param.AEC_KEY) {
            readsize = sf_readf_short(refwav, refbuf, framelen);
            if (readsize != framelen) {
                fprintf(stderr, "processing all data, exiting...\n");
                break;
            }
        }

        ret = dios_ssp_process_api(hssp, micbuf, refbuf, sspbuf, &param);
        if (ret) {
            fprintf(stderr, "dios_ssp_process_api failed, return %d\n", ret);
            break;
        }

        sf_writef_short(outwav, sspbuf, framelen);
    }

    ret = dios_ssp_uninit_api(hssp, &param);
    if (OK_AUDIO_PROCESS != ret) {
        fprintf(stderr, "dios_ssp_uninit_api failed\n");
        return -1;
    }

    /* Close input and output files. */
    sf_close(outwav);
    sf_close(inwav);
    if (!refFilePath.empty()) {
        sf_close(refwav);
    }

    return 0;
}
