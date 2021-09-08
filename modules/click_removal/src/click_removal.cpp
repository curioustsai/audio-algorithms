/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "click_removal.h"
#include "biquad.h"
#include "frame.h"
#include "ring_buffer.h"
#include <cmath>
#include <cstring>

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
    const float coef[2][5] = {{0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.4169284},
                              {1, -2, 1, -1.62913992, 0.9105507}};
    _hpf4kHz = new SosFilter;
    _hpf4kHz->reset(coef, 2);

    /*
     * low pass filter at 4kHz for fs=48kHz, 2 biquad cascaded
     * signal.cheby1(4, 3, 4000/24000, 'low', output='sos')
     */
    const float coef_lpf4kHz[2][5] = {
        {5.17335477e-04, 1.03467095e-03, 5.17335477e-04, -1.75391386e+00, 8.03975970e-01},
        {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.68424486e+00, 9.17796589e-01}};

    _lpf4kHz = new SosFilter;
    _lpf4kHz->reset(coef_lpf4kHz, 2);

    _inFrame = new FrameOverlap{_subframeSize, _hopSize};
    _inFrame->reset(_subframeSize, _hopSize);

    _inFrame4kHz = new FrameOverlap{_subframeSize, _hopSize};
    _inFrame4kHz->reset(_subframeSize, _hopSize);
    _inBuffer = new RingBuffer{frameSize * 16};
    _hop = new float[_hopSize];

#ifdef AUDIO_ALGO_DEBUG
    dbgInfo = new float[_frameSize * 2];
#endif
}

ClickRemoval::~ClickRemoval() {
    if (_hpf4kHz) {
        delete _hpf4kHz;
        _hpf4kHz = nullptr;
    }

    if (_lpf4kHz) {
        delete _lpf4kHz;
        _lpf4kHz = nullptr;
    }

    if (_inFrame) {
        delete _inFrame;
        _inFrame = nullptr;
    }

    if (_inFrame4kHz) {
        delete _inFrame4kHz;
        _inFrame4kHz = nullptr;
    }

    if (_inBuffer) {
        delete _inBuffer;
        _inBuffer = nullptr;
    }

    if (_hop) {
        delete[] _hop;
        _hop = nullptr;
    }
#ifdef AUDIO_ALGO_DEBUG
    if (dbgInfo) {
        delete[] dbgInfo;
        dbgInfo = nullptr;
    }
#endif
}

int ClickRemoval::process(const float *input, float *output, const int num) {
    if (num != _frameSize) return 0;

    int num_processed = 0;
    _inBuffer->putFrame(input, num);

    while (_inBuffer->getFrame(_hop, _hopSize) == _hopSize) {
        _inFrame->updateFrame(_hop, _hopSize);

        _hpf4kHz->process(_hop, _hop, _hopSize);
        _inFrame4kHz->updateFrame(_hop, _hopSize);

        float power = _inFrame->getPowerMean();
        float power_4kHz = _inFrame4kHz->getPowerMean();

#ifdef AUDIO_ALGO_DEBUG
        for (int i = 0; i < _hopSize; i++) {
            dbgInfo[2 * (i + num_processed)] = power;
            dbgInfo[2 * (i + num_processed) + 1] = power_4kHz;
        }
#endif

        if ((power > _threshold_all) && (power_4kHz > _threshold_4kHz)) { _detected = 2; }

        _inFrame->getOutput(output + num_processed, _hopSize);
        if (_detected) {
            _lpf4kHz->process(output + num_processed, output + num_processed, _hopSize);
            for (int i = 0; i < _hopSize; i++) { output[i + num_processed] *= 0.25; }
            _detected--;
        }

        num_processed += _hopSize;
    }

    return num_processed;
}

} // namespace ubnt
