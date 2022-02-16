#include <cmath>
#include "smoke_detector.h"
#include "goertzel.h"
#include "countdown.h"
#include "observer.h"

using namespace ubnt::smartaudio;
const static int NUM_ON = 3;
const static int NUM_ON_OFF = 6;
const static float INTERVAL_SEC = 0.5; // second

inline bool anyTrueInArray(bool *in, unsigned int arraySize) {
    unsigned int i = 0;
    while (!in[i] && i < arraySize) i++;
    return (i < arraySize);
}

class SmokeDetector::Impl {
public:
    int sampleRate{48000};
    int frameSize{128};
    float threshold{-20.0};
    int numTargetFreq{2};
    Goertzel** goertzel;

    bool* candidateBuf{0};
    int candidateBufLen{0};
    int candidateBufIndex{0};

    Observer observer;
    int frameUpperBound{0};
    int frameLowerBound{0};
    CountDown holdOn;
    int alarmCount{0};
    int onThreshold{0};
    float framesPerSec{0.0f};

    float getPower(float* data, int numSample);
    AudioEventType DetectPattern(float *data, int numSample);

#ifdef AUDIO_ALGO_DEBUG
    float powerAvg{0.0};
    bool status{0};
#endif
};

void SmokeDetector::Init(Config config, int* targetFrequencies, int numTargetFreq) {
    pimpl = new Impl();
    pimpl->numTargetFreq = numTargetFreq;
    pimpl->sampleRate = config.sampleRate;
    pimpl->frameSize = config.frameSize;
    pimpl->threshold = config.threshold;

    /*
     |-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|-0.5s-|
     ON,     OFF,   ON,    OFF,   ON,    OFF,   OFF,   OFF
     INTERVAL ON: 3
     INTERVAL OFF: 5
     */

    pimpl->framesPerSec = (float)pimpl->sampleRate / (float)pimpl->frameSize;
    pimpl->onThreshold = static_cast<int>(INTERVAL_SEC * pimpl->framesPerSec * 0.7f);

    float observeBufLen = int(8.0f * INTERVAL_SEC * pimpl->framesPerSec);
    pimpl->frameUpperBound = int(observeBufLen * 4.0 / 8.0);
    pimpl->frameLowerBound = int(observeBufLen * 2.5 / 8.0);

    pimpl->candidateBufLen = 10;
    pimpl->holdOn = CountDown(static_cast<unsigned int>(5.0 * pimpl->framesPerSec));
    pimpl->holdOn.setCounter(0);
    pimpl->alarmCount = 0;

    pimpl->goertzel = new Goertzel*[pimpl->numTargetFreq];
    for (int i = 0; i < pimpl->numTargetFreq; ++i) {
        pimpl->goertzel[i] = new Goertzel(pimpl->sampleRate, pimpl->frameSize, targetFrequencies[i]);
    }

    pimpl->candidateBufIndex = 0;
    pimpl->candidateBuf = new bool[pimpl->candidateBufLen];
    for (int i = 0; i < pimpl->candidateBufLen; ++i) { pimpl->candidateBuf[i] = false; }

    pimpl->observer = Observer(observeBufLen);
}

void SmokeDetector::Release() {
    for (int i = 0; i < pimpl->numTargetFreq; ++i) { delete pimpl->goertzel[i]; }
    delete[] pimpl->goertzel;
    delete[] pimpl->candidateBuf;
    pimpl->observer.release();
    delete pimpl;
}

AudioEventType SmokeDetector::Detect(float* data, int numSample) {
    return pimpl->DetectPattern(data, numSample);
}

AudioEventType SmokeDetector::Impl::DetectPattern(float* data, int numSample) {
    bool filterOut = false;
    bool observe_prev = false;
    bool observe_now = false;
    float power = getPower(data, numSample);

#ifdef AUDIO_ALGO_DEBUG
    _powerAvg = power / (float)numSample;
    _powerAvg = (_powerAvg >= 1.f) ? 1.0f : _powerAvg;
#endif

    candidateBuf[candidateBufIndex] = power > threshold;
    candidateBufIndex = (candidateBufIndex + 1 != candidateBufLen) ? candidateBufIndex + 1 : 0;

    // Consider a range of frames in order to prevent sudden drop caused by audio codec processing
    filterOut = anyTrueInArray(candidateBuf, candidateBufLen);

#ifdef AUDIO_ALGO_DEBUG
    status = power > threshold;
#endif

    observe_prev = observer.get();
    observer.put(filterOut);
    observe_now = observer.get();

    if (holdOn.count() == 0) { alarmCount = 0; }
    if (observe_prev == observe_now) { return AUDIO_EVENT_NONE; }

    int numDetected = 0;
    int numDetectedOn[NUM_ON] = {0};
    int duration = int(INTERVAL_SEC * framesPerSec);

    int index = observer.getCurrentIndex();
    for (int intervalCount = 0; intervalCount < NUM_ON_OFF; ++intervalCount) {
        int intervalCount_2 = intervalCount / 2;

        for (int step = 0; step < duration; ++step) {
            if ((intervalCount % 2) == 0) {
                numDetectedOn[intervalCount_2] += observer.get(index);
            }
            numDetected += observer.get(index);
            index = (index + 1 < observer.getLength() ? index + 1 : 0);
        }
    }

    int stepRemain = observer.getLength() - NUM_ON_OFF * duration;
    for (int step = 0; step < stepRemain; ++step) {
        numDetected += observer.get(index);
        index = (index + 1 < observer.getLength() ? index + 1 : 0);
    }

    int legalCount = 0;
    for (int i = 0; i < NUM_ON; ++i) {
        if (numDetectedOn[i] > onThreshold) { legalCount++; }
    }

    if ((frameLowerBound < numDetected) && (numDetected < frameUpperBound) &&
        (legalCount >= NUM_ON)) {
        holdOn.reset();
        alarmCount++;

        if (alarmCount >= 5) {
            alarmCount = 0;
            return AUDIO_EVENT_SMOKE;
        }
    }

    return AUDIO_EVENT_NONE;
}

float SmokeDetector::Impl::getPower(float* data, int numSample) {
    float power = 0.f;

    for (int i = 0; i < numTargetFreq; ++i) { power += goertzel[i]->calculate(data, numSample); }
    power = 10.0f * (log10f(power) - log10f((float)numSample));

    return power;
}

void SmokeDetector::SetThreshold(float threshold) { pimpl->threshold = threshold; }
float SmokeDetector::GetThreshold() const { return pimpl->threshold; }

void SmokeDetector::ResetStates() {
    pimpl->candidateBufIndex = 0;
    for (int i = 0; i < pimpl->candidateBufLen; ++i) { pimpl->candidateBuf[i] = false; }
    pimpl->observer.reset();
}

#ifdef AUDIO_ALGO_DEBUG
float SmokeDetector::GetPowerAvg() const { return pimpl->powerAvg; }
bool SmokeDetector::GetStatus() const { return pimpl->status; }
#endif
