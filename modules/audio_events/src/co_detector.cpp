#include <cmath>
#include "co_detector.h"
#include "goertzel.h"
#include "countdown.h"
#include "observer.h"

using namespace ubnt::smartaudio;
const static int NUM_ON = 4;
const static int NUM_ON_OFF = 8;
const static float INTERVAL_SEC = 0.1f; // second

class CoDetector::Impl {
public:
    int numTargetFreq{0};
    int sampleRate{48000};
    int frameSize{128};
    float threshold{-20.0};
    Goertzel** goertzel;

    Observer shortObserver;
    Observer longObserver;
    int frameUpperBound{0};
    int frameLowerBound{0};
    CountDown holdOn;
    CountDown holdLong;
    int alarmCount{0};
    int alarmCountLong{0};
    bool shortPatternDetected{false};

    AudioEventType DetectShortPattern(float power);
    AudioEventType DetectLongPattern(float power);
    float getPower(float* data, int numSample);
#ifdef AUDIO_ALGO_DEBUG
    float powerAvg{0.0};
    bool status{false};
#endif
};

void CoDetector::Init(Config config, int* targetFrequencies, int numTargetFreq) {
    pimpl = new Impl();
    pimpl->numTargetFreq = numTargetFreq;
    pimpl->sampleRate = config.sampleRate;
    pimpl->frameSize = config.frameSize;
    pimpl->threshold = config.threshold;

    // 4 cycles of 100 ms and 100 ms off, then 5 seconds off
    // units in second
    pimpl->frameUpperBound = int(INTERVAL_SEC * (float)pimpl->sampleRate / (float)pimpl->frameSize * 8.5f);
    pimpl->frameLowerBound = int(INTERVAL_SEC * (float)pimpl->sampleRate / (float)pimpl->frameSize * 3.5f);
    pimpl->holdOn = CountDown(static_cast<unsigned int>(10.0f * (float)pimpl->sampleRate / (float)pimpl->frameSize));
    pimpl->holdOn.setCounter(0);
    pimpl->holdLong = CountDown(static_cast<unsigned int>(4.0f * (float)pimpl->sampleRate / (float)pimpl->frameSize));
    pimpl->holdLong.setCounter(0);
    
    pimpl->alarmCount = 0;

    pimpl->goertzel = new Goertzel*[pimpl->numTargetFreq];
    for (int i = 0; i < pimpl->numTargetFreq; ++i) {
        pimpl->goertzel[i] = new Goertzel(pimpl->sampleRate, pimpl->frameSize, targetFrequencies[i]);
    }

    pimpl->shortObserver = Observer(int(INTERVAL_SEC * (float)pimpl->sampleRate / (float)pimpl->frameSize * 10.0));
    pimpl->longObserver = Observer(int(10.6f * (float)pimpl->sampleRate / (float)pimpl->frameSize));
}

void CoDetector::Release() {
    for (int i = 0; i < pimpl->numTargetFreq; ++i) { delete pimpl->goertzel[i]; }
    delete[] pimpl->goertzel;
    pimpl->shortObserver.release();
    pimpl->longObserver.release();
    delete pimpl;
}

AudioEventType CoDetector::Detect(float* data, int numSample) {
    float power = pimpl->getPower(data, numSample);
    return pimpl->DetectLongPattern(power);
}

void CoDetector::SetThreshold(float threshold) { 
    pimpl->threshold = threshold;
}

float CoDetector::GetThreshold() const {
    return pimpl->threshold;
}

void CoDetector::ResetStates() {
    pimpl->holdOn.reset();
    pimpl->holdLong.reset();
    pimpl->shortObserver.reset();
    pimpl->longObserver.reset();
}

float CoDetector::Impl::getPower(float* data, int numSample) {
    float power = 0.f;

    for (int i = 0; i < numTargetFreq; ++i) { power += goertzel[i]->calculate(data, numSample); }
    power = 10.0f * (log10f(power) - log10f((float)numSample));

    return power;
}

AudioEventType CoDetector::Impl::DetectShortPattern(float power) {
    bool observePrev = false;
    bool observeNow = false;

#ifdef AUDIO_ALGO_DEBUG
    powerAvg = power / (float)numSample;
    powerAvg = (powerAvg >= 1.f) ? 1.0f : powerAvg;
#endif

    observePrev = shortObserver.get();
    shortObserver.put((power > threshold));
    observeNow = shortObserver.get();

#ifdef AUDIO_ALGO_DEBUG
    _status = power > _threshold;
#endif

    if (holdOn.count() == 0) { alarmCount = 0; }
    if (observePrev == observeNow) { return AUDIO_EVENT_NONE; }

    int numDetected = 0;
    int numDetectedOn[NUM_ON] = {0};
    int duration = int(INTERVAL_SEC * (float)sampleRate / (float)frameSize);

    int index = shortObserver.getCurrentIndex();
    for (int intervalCount = 0; intervalCount < NUM_ON_OFF; ++intervalCount) {
        int intervalCount_2 = intervalCount / 2;
        for (int step = 0; step < duration; ++step) {
            if ((intervalCount % 2) == 0) {
                numDetectedOn[intervalCount_2] += shortObserver.get(index);
            }
            numDetected += shortObserver.get(index);
            index = (index + 1 < shortObserver.getLength() ? index + 1 : 0);
        }
    }

    int stepRemain = shortObserver.getLength() - NUM_ON_OFF * duration;
    for (int step = 0; step < stepRemain; ++step) {
        numDetected += shortObserver.get(index);
        index = (index + 1 < shortObserver.getLength() ? index + 1 : 0);
    }

    int legalCount = 0;
    int threshold = int(INTERVAL_SEC * (float)sampleRate / (float)frameSize * 0.7);
    for (int i = 0; i < NUM_ON; ++i) {
        if (numDetectedOn[i] > threshold) { legalCount++; }
    }

    if ((frameLowerBound < numDetected) && (legalCount >= NUM_ON)) {
        holdOn.reset();
        alarmCount++;

        if (alarmCount >= 2) {
            alarmCount = 0;
            return AUDIO_EVENT_CO;
        }
    }
    return AUDIO_EVENT_NONE;
}

AudioEventType CoDetector::Impl::DetectLongPattern(float power) {
    longObserver.put(power > threshold);

    const int upperBound = int((float)sampleRate / (float)frameSize * 5.8f * 0.3f);
    const int lowerBound = frameLowerBound * 2;

    if (DetectShortPattern(power) == AUDIO_EVENT_CO) {
        shortPatternDetected = true;
        holdLong.reset();
    }

    if (shortPatternDetected && holdLong.count() == 0) {
        int numDetected = longObserver.sum();


        if ((lowerBound < numDetected) && (numDetected < upperBound)) {
            alarmCountLong++;
            if (alarmCountLong >= 2) {
                alarmCountLong = 0;
                return AUDIO_EVENT_CO;
            }
        }
        shortPatternDetected = false;
    }

    return AUDIO_EVENT_NONE;
}


#ifdef AUDIO_ALGO_DEBUG
float CoDetector::GetPowerAvg() const { return pimpl->powerAvg; }
bool CoDetector::GetStatus() const { return pimpl->status; }
#endif
