/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "click_removal.h"
#include "biquad.h"
#include "frame.h"
#include "ring_buffer.h"
#include <cmath>
#include <cstring>

#define SAFEDELETE(ptr) \
    { \
        delete (ptr); \
        ptr = NULL; \
    }

namespace ubnt {

ClickRemoval::ClickRemoval(const int frameSize, const int subframeSize, const float threshold_all,
                           const float threshold_4kHz)
    : _frameSize(frameSize),
      _subframeSize(subframeSize),
      _threshold_all(threshold_all),
      _threshold_4kHz(threshold_4kHz) {
    _hopSize = _subframeSize / 2;

    /* 
     * high pass filter at 4kHz for fs=48kHz, 2 biquad cascaded 
     * import scipy.signal as signal
     * signal.cheby1(4, 3, 4000/24000, 'high', output='sos') 
     */
    const float coef_hpf4kHz[2][5] = {{0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.4169284},
                                      {1, -2, 1, -1.62913992, 0.9105507}};
    _hpf4kHz = new SosFilter;
    _hpf4kHz->reset(coef_hpf4kHz, 2);

    /*
     * low pass filter at 400Hz for fs=48kHz, 2 biquad cascaded
     * signal.cheby1(4, 3, 400/24000, 'low', output='sos')
     */
    const float coef_lpf400Hz[2][5] = {
        {5.79832612e-08, 1.15966522e-07, 5.79832612e-08, -1.97816320e+00, 9.78694943e-01},
        {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.98865955e+00, 9.91124026e-01}};
    _removeFilter = new SosFilter;
    _removeFilter->reset(coef_lpf400Hz, 2);

    _currFrame = new FrameOverlap{_subframeSize, _hopSize};
    _prevFrame = new FrameOverlap{_subframeSize, _hopSize};
    _outFrame = new FrameOverlap{_subframeSize, _hopSize};

    _inBuffer = new RingBuffer{frameSize * 4};

    _outBuffer = new RingBuffer{frameSize * 4};
    // delay 2 frameSize
    _outBuffer->setDelay(frameSize * 2);

    _hop = new Frame{_hopSize};
    _frameHP = new Frame{_subframeSize};
    _frameBP = new Frame{_subframeSize};
    _frameLP = new Frame{_subframeSize};
    _hann = new Frame{_subframeSize};
    _windowed = new Frame{_subframeSize};

    initHann(_hann);

#ifdef AUDIO_ALGO_DEBUG
    dbgInfo.reset(new float[_frameSize * dbgChannels]);
#endif
}

void ClickRemoval::threshold_all(const int threshold_all) { _threshold_all = threshold_all; }
int ClickRemoval::threshold_all() const { return _threshold_all; }
void ClickRemoval::threshold_4kHz(const int threshold_4kHz) { _threshold_4kHz = threshold_4kHz; }
int ClickRemoval::threshold_4kHz() const { return _threshold_4kHz; }

ClickRemoval::~ClickRemoval() {
    SAFEDELETE(_inBuffer);
    SAFEDELETE(_outBuffer);
    SAFEDELETE(_hpf4kHz);
    SAFEDELETE(_removeFilter);

    SAFEDELETE(_currFrame);
    SAFEDELETE(_prevFrame);
    SAFEDELETE(_outFrame);
    SAFEDELETE(_hop);
    SAFEDELETE(_frameHP);
    SAFEDELETE(_frameBP);
    SAFEDELETE(_frameLP);
    SAFEDELETE(_hann);
    SAFEDELETE(_windowed);

    delete[] _floatBuf;
    _floatBuf = nullptr;
}

void ClickRemoval::initHann(Frame *hann) {
    const float PI_2 = M_PI + M_PI;
    const int frameSize = hann->frameSize();
    const float denom = 1.0f / static_cast<float>(frameSize - 1);
    const unsigned int halfFrameSize = frameSize >> 1;
    for (unsigned int i = 0; i < halfFrameSize; i++) {
        hann->data()[i] = 0.5f * (1.0f - cos(PI_2 * static_cast<float>(i) * denom));
        hann->data()[frameSize - i - 1] = hann->data()[i];
    }
    // If the numSample is odd number, the center index of window should be calculated
    if ((frameSize & 1) == 1) { hann->data()[halfFrameSize + 1] = 0.5f; }
}

bool ClickRemoval::applyWindow(const Frame *frame, const Frame *window, Frame *windowed) {
    if ((frame->frameSize() != window->frameSize()) ||
        (frame->frameSize() != windowed->frameSize()))
        return false;

    for (int i = 0; i < frame->frameSize(); i++) {
        windowed->data()[i] = frame->data()[i] * window->data()[i];
    }

    return true;
}

void ClickRemoval::overlapAdd(Frame *previous, Frame *current, float *buf) {
    int overlapSize = _subframeSize - _hopSize;
    for (int i = 0; i < overlapSize; i++) { current->data()[i] += previous->data()[i + _hopSize]; }
    memcpy(buf, current->data(), _hopSize * sizeof(float));
    previous->copyFrame(current);
}

int ClickRemoval::process(float *buf, const int num) {
    if (num != _frameSize) return 0;

    int num_processed = 0;
    _inBuffer->putFrame(buf, num);

    while (_inBuffer->getFrame(_hop) == _hopSize && num_processed < num) {
        _currFrame->updateHop(_hop);

        applyWindow(_currFrame, _hann, _windowed);
        _hpf4kHz->process(_windowed->data(), _frameHP->data(), _subframeSize);

        float power = _currFrame->getPowerMean();
        float power_4kHz = _frameHP->getPowerMean();

        if ((power > _threshold_all) && (power_4kHz > _threshold_4kHz)) { _detected = 4; }

        _outBuffer->putFrame(_windowed);
        _outBuffer->getFrame(_outFrame);

        if (_detected) {
            _removeFilter->process(_outFrame->data(), _outFrame->data(), _subframeSize);
            _detected--;
        }
        overlapAdd(_prevFrame, _outFrame, buf + num_processed);
#ifdef AUDIO_ALGO_DEBUG
        for (int i = 0; i < _hopSize; i++) {
            dbgInfo[dbgChannels * (i + num_processed)] = buf[num_processed + i];
            dbgInfo[dbgChannels * (i + num_processed) + 1] = power;
            dbgInfo[dbgChannels * (i + num_processed) + 2] = power_4kHz;
        }
#endif
        num_processed += _hopSize;
    }
    memset(buf + num_processed, 0, (num - num_processed) * sizeof(float));

    return num_processed;
}

int ClickRemoval::process(int16_t *buf, const int num) {
    if (_floatBuf == nullptr) { _floatBuf = new float[_frameSize]{0}; }

    for (int i = 0; i < num; i++) { _floatBuf[i] = (float)buf[i] / 32768.f; }
    int num_processed = process(_floatBuf, num);
    for (int i = 0; i < num_processed; i++) { buf[i] = (int16_t)(_floatBuf[i] * 32768.f); }
    for (int i = num_processed; i < num; i++) { buf[i] = 0; }

    return num_processed;
}

} // namespace ubnt
