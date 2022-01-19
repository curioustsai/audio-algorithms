#include "CLI/CLI.hpp"
#include "CLI/Validators.hpp"
#include "nlohmann/json.hpp"

#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ring_buffer.h"
#include "ubnt_logger/ubnt_logger.h"

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
    app.add_option("-r,--refFile", refFilePath, "specify an ref file")->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-c,--config", jsonConfigPath, "specify an json config file")
        ->required()
        ->check(CLI::ExistingFile);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SF_INFO info, rinfo, oinfo;
    SNDFILE *inwav, *refwav, *outwav;

    ASSERT_LOG_RET(inwav = sf_open(inputFilePath.c_str(), SFM_READ, &info), -1, "open %s failed",
                   inputFilePath.c_str());

    if (!refFilePath.empty()) {
        ASSERT_LOG_ACT(refwav = sf_open(refFilePath.c_str(), SFM_READ, &rinfo), sf_close(inwav), -1,
                       "open %s failed", refFilePath.c_str());
    }

    oinfo.channels = 1;
    oinfo.samplerate = info.samplerate;
    oinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    oinfo.sections = 0;
    oinfo.seekable = 1;

    ASSERT_LOG_ACT(outwav = sf_open(outputFilePath.c_str(), SFM_WRITE, &oinfo),
                   sf_close(inwav) && sf_close(refwav), -1, "open %s failed",
                   outputFilePath.c_str());

    // read json config file
    std::ifstream ifs(jsonConfigPath);
    nlohmann::json jf = nlohmann::json::parse(ifs);

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

    if (param.AEC_KEY == 1) {
        param.aec_param.dt_thr_factor = jf["aec"]["dt_thr_factor"];
        param.aec_param.dt_min_thr = jf["aec"]["dt_min_thr"];

        param.aec_param.res1_echo_noise_factor = jf["aec"]["res1_echo_noise_factor"];
        param.aec_param.res2_echo_noise_factor = jf["aec"]["res2_echo_noise_factor"];
        param.aec_param.res1_echo_suppress_default = jf["aec"]["res1_echo_suppress_default"];
        param.aec_param.res2_st_echo_suppress_default = jf["aec"]["res2_st_echo_suppress_default"];
        param.aec_param.res2_dt_echo_suppress_default = jf["aec"]["res2_dt_echo_suppress_default"];
        param.aec_param.res1_echo_suppress_active_default =
            jf["aec"]["res1_echo_suppress_active_default"];
        param.aec_param.res2_st_echo_suppress_active_default =
            jf["aec"]["res2_st_echo_suppress_active_default"];
        param.aec_param.res2_dt_echo_suppress_active_default =
            jf["aec"]["res2_dt_echo_suppress_active_default"];
        param.aec_param.res1_suppress_factor = jf["aec"]["res1_suppress_factor"];
        param.aec_param.res2_st_suppress_factor = jf["aec"]["res2_st_suppress_factor"];
        param.aec_param.res2_dt_suppress_factor = jf["aec"]["res2_dt_suppress_factor"];
    }

    memset(param.mic_coord, 0, sizeof(param.mic_coord));

    int ret;
    void *hssp;

    ASSERT_LOG_RET(hssp = dios_ssp_init_api(&param), -4, "dios_ssp_init_api failed");
    ASSERT_LOG_RET((ret = dios_ssp_reset_api(hssp, &param)) == 0, -5,
                   "dios_ssp_reset_api failed, return %d", ret);

    // Note: athena-signal only take 128 samples as the frame size, but we apply queue for practical use
    // Simulation frameSize 1024 on our camera
    int frameLen = 1024;
    int subframeLen = 128;
    int bufNum = frameLen / subframeLen * 8;

    ubnt::RingBuffer<short> inputRingBuf(subframeLen, bufNum);
    ubnt::RingBuffer<short> refRingBuf(subframeLen, bufNum);
    ubnt::RingBuffer<short> outputRingBuf(subframeLen, bufNum);

    short micbuf[frameLen];
    short refbuf[frameLen];
    short sspbuf[frameLen];
    short micbuf_sub[subframeLen];
    short refbuf_sub[subframeLen];
    short sspbuf_sub[subframeLen];

    while (1) {
        if (sf_readf_short(inwav, micbuf, frameLen) != frameLen) {
            fprintf(stderr, "processing all data, exiting...\n");
            break;
        }
        ASSERT_LOG(LOG_INFO, inputRingBuf.putFrame(micbuf, frameLen), "inputRingBuf Full");

        if (!refFilePath.empty() && param.AEC_KEY) {
            if (sf_readf_short(refwav, refbuf, frameLen) != frameLen) {
                fprintf(stderr, "processing all data, exiting...\n");
                break;
            }
            ASSERT_LOG(LOG_INFO, refRingBuf.putFrame(refbuf, frameLen), "refRingBuf Full");
        }

        while (inputRingBuf.getInUseSamples() > subframeLen) {

            ASSERT_LOG(LOG_INFO, subframeLen == inputRingBuf.getFrame(micbuf_sub, subframeLen),
                       "Failed to get subframe from inputRingBuf");

            if (!refFilePath.empty() && param.AEC_KEY) {
                ASSERT_LOG(LOG_INFO, subframeLen == refRingBuf.getFrame(refbuf_sub, subframeLen),
                           "Failed to get subframe from refRingBuf");
            }

            ret = dios_ssp_process_api(hssp, micbuf_sub, refbuf_sub, sspbuf_sub, &param);
            ASSERT_LOG(LOG_ERR, (ret == 0), "dios_ssp_process_api failed, return %d", ret);

            ASSERT_LOG(LOG_INFO, outputRingBuf.putFrame(sspbuf_sub, subframeLen),
                       "outputRingBuf Full");
        }

        while (outputRingBuf.getInUseSamples() > frameLen) {
            ASSERT_LOG(LOG_INFO, frameLen == outputRingBuf.getFrame(sspbuf, frameLen),
                       "Faile to get frame from outputRingBuf");
            sf_writef_short(outwav, sspbuf, frameLen);
        }
    }

    ASSERT_LOG_RET(OK_AUDIO_PROCESS == dios_ssp_uninit_api(hssp, &param), -1,
                   "dios_ssp_uninit_api failed");

    /* Close input and output files. */
    sf_close(outwav);
    sf_close(inwav);
    if (!refFilePath.empty()) { sf_close(refwav); }

    return 0;
}
