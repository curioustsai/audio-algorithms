#include "CLI/CLI.hpp"
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <cctype>
#include "formant_shift.h"

using namespace ubnt;

int main(int argc, char **argv) {
    std::string inputFilePath, pitchFilePath, outputFilePath;
    short *raw_data, *ps_data, *out_data;
    float shiftTone = 0.0f;
    const int MaxFrameSize = 4096;
    int frame_size = 1000;

    CLI::App app{"Formant Shifter"};

    app.add_option("-i,--inFile", inputFilePath, "specify the original input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-p,--pitchFile", pitchFilePath, "specify the pitch shift file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("-s, --shift", shiftTone, "specify the formant shift in semi-tones")
        ->required()
        ->check(CLI::Number);
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

    raw_data = (short *)new short[MaxFrameSize]();
    ps_data = (short *)new short[MaxFrameSize]();
    out_data = (short *)new short[MaxFrameSize]();

    FormantShift formantShift(sample_rate);
    formantShift.setShiftTone(shiftTone);
    formantShift.setDelay(19456);
    clock_t tick = clock();

    int count = 0;
    while ((frame_size == sf_read_short(infile, raw_data, frame_size))
				&& (frame_size == sf_read_short(psfile, ps_data, frame_size))) {

        formantShift.process(ps_data, raw_data, out_data, frame_size);
        
        sf_write_short(outfile, out_data, frame_size);

        frame_size += 1000;
        frame_size %= MaxFrameSize;
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
  
    return 0;
}
