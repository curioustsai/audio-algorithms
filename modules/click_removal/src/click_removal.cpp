/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#include "click_removal.h"
#include <cmath>
#include <cstring>

ClickRemoval::ClickRemoval(const int frameSize, const int subframeSize, const float threshold_all,
                           const float threshold_4kHz)
    : _frameSize(frameSize),
      _subframeSize(subframeSize),
      _threshold_all(threshold_all),
      _threshold_4kHz(threshold_4kHz) {

    /* high pass filter, 2 biquad cascasded 
     * import scipy.signal as signal
     * signal.cheby1(4, 3, 4000/24000, 'high', output='sos') */
    const float ba[2][5] = {{0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.4169284},
                            {1, -2, 1, -1.62913992, 0.9105507}};
    _hpf4kHz.reset(ba, 2);

    _hopSize = _subframeSize / 2;
    _inFrame.reset(_subframeSize, _hopSize);
    _inFrame4kHz.reset(_subframeSize, _hopSize);

#ifdef AUDIO_ALGO_DEBUG
    dbgInfo = new float[_frameSize * 2];
#endif
};

ClickRemoval::~ClickRemoval() {
#ifdef AUDIO_ALGO_DEBUG
    delete[] dbgInfo;
#endif
}

int ClickRemoval::process(const float *input, float *output, const int num) {
    if (num != _frameSize) return 0;

    int num_processed = 0;
    float *data_hop = new float[_hopSize];
    _rawBuffer.putFrame(input, num);

    while (_rawBuffer.getFrame(data_hop, _hopSize) == _hopSize) {
        _inFrame.updateFrame(data_hop, _hopSize);

        _hpf4kHz.process(data_hop, data_hop, _hopSize);
        _inFrame4kHz.updateFrame(data_hop, _hopSize);

        float power = _inFrame.getPowerMean();
        float power_4kHz = _inFrame4kHz.getPowerMean();

#ifdef AUDIO_ALGO_DEBUG
        for (int i = 0; i < _hopSize; i++) {
            dbgInfo[2 * (i + num_processed)] = power;
            dbgInfo[2 * (i + num_processed) + 1] = power_4kHz;
        }
#endif

        _inFrame.getOutput(output + num_processed, _hopSize);
        if ((power > _threshold_all) && (power_4kHz > _threshold_4kHz)) { _detected = 3; }

        if (_detected) {
            float gain = fmin(0.1, _framePowerSmooth / power);
            for (int i = 0; i < _hopSize; i++) { output[i + num_processed] *= gain; }
            _detected--;
        }
        _framePowerSmooth = 0.9f * _framePowerSmooth + 0.1f * power;

        num_processed += _hopSize;
        // bypass
        // for (int i = 0; i < _hopSize; i++) { output[i + num_processed] = data_hop[i]; }
        // num_processed += _hopSize;
    }

    delete[] data_hop;

    return num_processed;
}

