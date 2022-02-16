/*
 * Copyright (C) 2014-2020, Ubiquiti Networks, Inc,
*/

#include "CLI/CLI.hpp"
#include "audio_events.h"
#include "sndfile.hh"
#include "version.h"
#include <cmath>
#include <iostream>
#include <string>

using namespace ubnt::smartaudio;

int main(int argc, char** argv) {
    CLI::App app{"UI audio event"};

    std::string inputFilePath;
    std::string outputFilePath;
    float smoke_threshold = -20.0f;
    float co_threshold = -20.0f;
    float levelUpperBound = -20.0f;
    float levelLowerBound = -70.0f;
    float loudThreshold = 80.0f;
    float quietThreshold = 0.0f;

    app.add_option("-i,--inFile", inputFilePath, "specify an input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outFile", outputFilePath, "specify an output file")->required();
    app.add_option("--smokeThreshold", smoke_threshold, "threshold for smoke")->check(CLI::Number);
    app.add_option("--coThreshold", co_threshold, "threshold for co")->check(CLI::Number);
    app.add_option("--levelUpperBound", levelUpperBound, "level upper bound for loudness detector")
        ->check(CLI::Number);
    app.add_option("--levelLowerBound", levelLowerBound, "level lower bound for loudness detector")
        ->check(CLI::Number);
    app.add_option("--loudThreshold", loudThreshold, "loud threshold for loudness detector")
        ->check(CLI::Number);
    app.add_option("--quietThreshold", quietThreshold, "quiet threshold for loudness detector")
        ->check(CLI::Number);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) { return app.exit(e); }

    std::cout << "AUDIO ALGO VERSION: " << AUDIO_ALGO_VERSION << std::endl;
    std::cout << g_AUDIO_ALGO_SHA1 << std::endl;

    SndfileHandle inFile = SndfileHandle(inputFilePath);
    SndfileHandle outFile;

    int samplerate = inFile.samplerate();
    int numTotalSamples = inFile.frames();

    int numSamplesPerWin = 2048;
    int numSamplesPerFrame = 64;
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
    auto* bufferOut = new float[numSamplesPerFrame * outputChannels];
    auto* dataFloat = new float[numSamplesPerFrame];

    outFile = SndfileHandle(outputFilePath, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT,
                            outputChannels, samplerate);

    int targetFrequencies[] = {3000, 3450};
    int numTargetFreq = sizeof(targetFrequencies) / sizeof(targetFrequencies[0]);

    Config configSmoke = {samplerate, numSamplesPerFrame, smoke_threshold};
    Config configCo = {samplerate, numSamplesPerFrame, co_threshold};
    SmokeDetector smokeDetector;
    CoDetector coDetector;
    LoudnessDetector loudnessDetector;

    smokeDetector.Init(configSmoke, targetFrequencies, numTargetFreq);
    coDetector.Init(configCo, targetFrequencies, numTargetFreq);
    loudnessDetector.Init("g4dome", 50.0, 0.0);
    loudnessDetector.SetDynamicHigh(levelUpperBound);
    loudnessDetector.SetDynamicLow(levelLowerBound);
    loudnessDetector.SetThresholdLoud(loudThreshold);
    loudnessDetector.SetThresholdQuiet(quietThreshold);
    loudnessDetector.ShowConfig();

    for (int windowCount = 0; windowCount < numTotalWindows; ++windowCount) {
        if (numSamplesPerWin != inFile.read(buffer, numSamplesPerWin)) {
            std::cout << "read-in size doesn't match" << std::endl;
        }

        for (int frameCount = 0; frameCount < numFramePerWin; ++frameCount) {
            short* data = buffer + frameCount * numSamplesPerFrame;

            for (int sampleCount = 0; sampleCount < numSamplesPerFrame; ++sampleCount) {
                dataFloat[sampleCount] = (float)data[sampleCount] / (float)maxSampleValue;
            }

            AudioEventType event;
            event = smokeDetector.Detect(dataFloat, numSamplesPerFrame);
            event |= coDetector.Detect(dataFloat, numSamplesPerFrame);
            event |= loudnessDetector.Detect(dataFloat, numSamplesPerFrame);

            for (int sampleCount = 0; sampleCount < numSamplesPerFrame; ++sampleCount) {
                bufferOut[outputChannels * sampleCount] = dataFloat[sampleCount];
                bufferOut[outputChannels * sampleCount + 1] = ((float)(1 << event) * 0.1f);
#ifdef AUDIO_ALGO_DEBUG
                bufferOut[outputChannels * sampleCount + 2] = sqrtf(smokeDetector.GetPowerAvg());
                bufferOut[outputChannels * sampleCount + 3] =
                    0.8f * (float)smokeDetector.GetStatus();
                // bufferOut[outputChannels * sampleCount + 4] = sqrtf(coDetector.GetPowerAvg());
                bufferOut[outputChannels * sampleCount + 4] = sqrtf(loudnessDetector.GetPowerAvg());
                bufferOut[outputChannels * sampleCount + 5] = 0.8f * (float)coDetector.GetStatus();
#endif
            }
            outFile.write(bufferOut, numSamplesPerFrame * outputChannels);
        }
    }

    smokeDetector.Release();
    coDetector.Release();

    delete[] dataFloat;
    delete[] buffer;
    delete[] bufferOut;

    return 0;
}
