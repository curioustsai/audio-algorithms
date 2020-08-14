/*
 * Copyright (C) 2014-2020, Ubiquiti Networks, Inc,
*/

#include "smoke_detector.h"
#include "co_detector.h"
#include "sndfile.hh"
// #include "matplotlibcpp.h"
#include <iostream>
#include <string>
// #include "CLI/CLI.hpp"

using namespace ubnt::smartaudio;
// namespace plt = matplotlibcpp;

int main(int argc, char** argv) {
    std::string inputFilePath;
    std::string outputFilePath;
    float smoke_threshold;
    float co_threshold;

    if (argc == 4) {
        inputFilePath = argv[1];
        outputFilePath = argv[2];
        smoke_threshold = (float) atof(argv[3]);
		co_threshold = (float) atof(argv[3]);
    } else if (argc == 3) {
        inputFilePath = argv[1];
        outputFilePath = argv[2];
        smoke_threshold = -20.f;
		co_threshold = -20.f;
	} else {
        std::cout << "please specify input file and output file." << std::endl;
        return -1;
    }

    SndfileHandle inFile = SndfileHandle(inputFilePath);
    SndfileHandle outFile;

    int samplerate = inFile.samplerate();
    int numTotalSamples = inFile.frames();

    int numSamplesPerWin =  2048;
    int numSamplesPerFrame = 128;
    int numTotalWindows = numTotalSamples / numSamplesPerWin;
    int numFramePerWin = numSamplesPerWin / numSamplesPerFrame;

//    int format = inFile.format();
    int maxSampleValue = 32768;

#ifdef AUDIO_ALGO_DEBUG
    int outputChannels = 6;
#else
    int outputChannels = 2;
#endif

    auto* buffer = new short[numSamplesPerWin];
    auto* bufferOut = new float[numSamplesPerFrame*outputChannels];
    auto* dataFloat = new float[numSamplesPerFrame];

    outFile = SndfileHandle(outputFilePath, SFM_WRITE, SF_FORMAT_WAV|SF_FORMAT_FLOAT, outputChannels, samplerate);

    int targetFrequencies[] = { 3000, 3450 };
    int numTargetFreq =  sizeof(targetFrequencies) / sizeof(targetFrequencies[0]);

    Config configSmoke = { samplerate, numSamplesPerFrame, smoke_threshold };
    Config configCo = { samplerate, numSamplesPerFrame, co_threshold };
    SmokeDetector smokeDetector;
    CoDetector coDetector;

    smokeDetector.Init(configSmoke, targetFrequencies, numTargetFreq);
    coDetector.Init(configCo, targetFrequencies, numTargetFreq);

    for (int windowCount = 0; windowCount < numTotalWindows; ++windowCount) {
        if (numSamplesPerWin != inFile.read(buffer, numSamplesPerWin)) {
            std::cout << "read-in size doesn't match" << std::endl;
        }

        for (int frameCount = 0; frameCount < numFramePerWin; ++frameCount) {
            short* data = buffer + frameCount * numSamplesPerFrame;

            for (int sampleCount = 0; sampleCount < numSamplesPerFrame; ++sampleCount) {
                dataFloat[sampleCount] = (float) data[sampleCount] / (float) maxSampleValue;
            }

            bool resultSmoke = smokeDetector.Detect(dataFloat, numSamplesPerFrame);
            bool resultCo = coDetector.Detect(dataFloat, numSamplesPerFrame);

            for (int sampleCount = 0; sampleCount < numSamplesPerFrame; ++sampleCount) {
                bufferOut[outputChannels * sampleCount] = dataFloat[sampleCount];
                bufferOut[outputChannels * sampleCount + 1] = ((float)resultSmoke * 0.5f + (float)resultCo * 0.8f);
#ifdef AUDIO_ALGO_DEBUG
                bufferOut[outputChannels * sampleCount + 2] = sqrtf(smokeDetector.GetPowerAvg());
                bufferOut[outputChannels * sampleCount + 3] = 0.8f * (float)smokeDetector.GetStatus();
                bufferOut[outputChannels * sampleCount + 4] = sqrtf(coDetector.GetPowerAvg());
                bufferOut[outputChannels * sampleCount + 5] = 0.8f * (float)coDetector.GetStatus();
#endif
            }
            outFile.write(bufferOut, numSamplesPerFrame * outputChannels);
        }
    }

    smokeDetector.Release();
    coDetector.Release();

    delete [] dataFloat;
    delete [] buffer;
    delete [] bufferOut;

	return 0;
}
