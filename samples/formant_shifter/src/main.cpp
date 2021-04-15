#include "CLI/CLI.hpp"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "formant_shift.h"

using namespace ubnt;

int main(int argc, char **argv) {
    std::string inputFilePath, pitchFilePath, outputFilePath;
    short *raw_data, *ps_data, *out_data;
    int frame_size = 1024;

    CLI::App app{"Formant Shifter"};

    app.add_option("-i,--inFile", inputFilePath, "specify the original input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-p,--pitchFile", pitchFilePath, "specify the pitch shift file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("--frameSize", frame_size, "frame size in samples")->check(CLI::Number);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) { return app.exit(e); }

    SNDFILE *infile, *psfile, *outfile;

    SF_INFO in_sfinfo, ps_sfinfo;
    memset(&in_sfinfo, 0, sizeof(SF_INFO));
    memset(&ps_sfinfo, 0, sizeof(SF_INFO));

    if (!(infile = sf_open(inputFilePath.c_str(), SFM_READ, &in_sfinfo))) {
        printf("Not able to open input file %s.\n", inputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

    if (!(psfile = sf_open(pitchFilePath.c_str(), SFM_READ, &ps_sfinfo))) {
        printf("Not able to open input file %s.\n", pitchFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

	if ((ps_sfinfo.samplerate != in_sfinfo.samplerate) 
			|| ps_sfinfo.channels != 1 
			|| in_sfinfo.channels != 1) {
        printf("Sample rate or channel number doesn't match");
		return 1;
	}

    if (!(outfile = sf_open(outputFilePath.c_str(), SFM_WRITE, &in_sfinfo))) {
        printf("Not able to open output file %s.\n", outputFilePath.c_str());
        puts(sf_strerror(NULL));
        return 1;
    };

    int sample_rate = in_sfinfo.samplerate;
    printf("sample rate: %d\n", sample_rate);

    raw_data = (short *)new short[frame_size];
    ps_data = (short *)new short[frame_size];
    out_data = (short *)new short[frame_size];

    float normalize = 1.0f / 32768.0f;
    float *raw_data_f = (float *)new float[frame_size]();
    float *ps_data_f = (float *)new float[frame_size]();
    float *out_data_f = (float *)new float[frame_size]();

    FormantShift formantShift;
    formantShift.init();
    formantShift.setShiftTone(3.0f);
    clock_t tick = clock();

    int count = 0;
    while ((frame_size == sf_read_short(infile, raw_data, frame_size))
				&& (frame_size == sf_read_short(psfile, ps_data, frame_size))) {
        // Normalize input and pitch shifted data into range -1 ~ 1.
        for (int i = 0; i < frame_size; i++) {
            raw_data_f[i] = static_cast<float>(raw_data[i]) * normalize;
            ps_data_f[i] = static_cast<float>(ps_data[i]) * normalize;
            out_data_f[i] = 0.0f;
        }

        formantShift.process(ps_data_f, raw_data_f, out_data_f, frame_size);

        for (int i = 0; i < frame_size; i++) {
            out_data[i] = static_cast<short>(out_data_f[i] * 32768.0f);
        }
        
        sf_write_short(outfile, out_data, frame_size);
        count++;
    }

    tick = clock() - tick;
    printf("total tick: %ld, times: %f\n", tick, ((float)tick / CLOCKS_PER_SEC));

    /* Close input and output files. */
    sf_close(infile);
    sf_close(psfile);
    sf_close(outfile);

    delete[] raw_data;
    delete[] ps_data;
    delete[] out_data;
    delete[] raw_data_f;
    delete[] ps_data_f;
    delete[] out_data_f;

    formantShift.release();
    return 0;
}
