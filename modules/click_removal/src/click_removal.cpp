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
    const float coef_hpf4kHz[2][5] = {{0.32483446, -0.64966892, 0.32483446, -0.65710985, 0.4169284},
                                      {1, -2, 1, -1.62913992, 0.9105507}};

    // // 10 kHz
    // const float coef[2][5] = { {0.07017989, -0.14035978, 0.07017989, 0.71398999, 0.42643369},
    //     {1.0, -2.0, 1.0, -0.38740178, 0.83889026}};

    _hpf4kHz.reset(new SosFilter);
    _hpf4kHz->reset(coef_hpf4kHz, 2);

    /*
     * low pass filter at 4kHz for fs=48kHz, 2 biquad cascaded
     * signal.cheby1(4, 3, 4000/24000, 'low', output='sos')
     */
    // const float coef_lpf4kHz[2][5] = {
    //     {5.17335477e-04, 1.03467095e-03, 5.17335477e-04, -1.75391386e+00, 8.03975970e-01},
    //     {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.68424486e+00, 9.17796589e-01}};

    // 2kHz
    // const float coef_lpf2kHz[2][5] = {
    //     {3.42871761e-05, 6.85743523e-05, 3.42871761e-05, -1.88476200e+00, 8.97609962e-01},
    //     {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.89647973e+00, 9.56793648e-01} };

    // // 1kHz
    // const float coef_lpf1kHz[2][5] = {
    //     {2.21649547e-06, 4.43299093e-06, 2.21649547e-06, -1.94427324e+00, 9.47549838e-01},
    //     {1.00000000e+00, 2.00000000e+00, 1.00000000e+0, -1.96271306e+00, 9.78001501e-01}};

    // 500Hz
    const float coef_lpf500Hz[2][5] = {
        {1.41041043e-07, 2.82082087e-07, 1.41041043e-07, -1.97260924e+00, 9.73438025e-01},
        {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.98507362e+00, 9.88919751e-01}};

    _lpf4kHz.reset(new SosFilter);
    _lpf4kHz->reset(coef_lpf500Hz, 2);

    const float coef_bandpass500_2kHz[2][5] = {
        {0.00454211, 0.00908423, 0.00454211, -1.85852125, 0.9090547},
        {1., -2., 1., -1.96404225, 0.96948968}};

    _bpf.reset(new SosFilter);
    _bpf->reset(coef_bandpass500_2kHz, 2);

    const float coef_click[4][5] = {
        // lpf 7kHz
        // {0.00454588, 0.00909175, 0.00454588,        -1.52318027, 0.6756387},
        // {1.,         2.,         1.,                 -1.19723873, 0.87112447},

        // lpf 4kHz
        // {5.17335477e-04, 1.03467095e-03, 5.17335477e-04, -1.75391386e+00, 8.03975970e-01},
        // {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.68424486e+00, 9.17796589e-01},

        // lpf 2kHz
        {3.42871761e-05, 6.85743523e-05, 3.42871761e-05, -1.88476200e+00, 8.97609962e-01},
        {1.00000000e+00, 2.00000000e+00, 1.00000000e+00, -1.89647973e+00, 9.56793648e-01},

        // 500-2kHz bandstop
        // { 0.64159094,-1.27216868, 0.64159094, -1.78975876,  0.86414217},
        // { 1.,        -1.98283455, 1.,         -1.96543886,  0.96902884}

        // 500-800Hz bandstop
        {0.69513219, -1.38550212, 0.69513219, -1.96648572, 0.97784994},
        {1., -1.99314914, 1., -1.98267113, 0.98672638}};

    _removeFilter.reset(new SosFilter);
    _removeFilter->reset(coef_click, 4);

    _inFrame.reset(new FrameOverlap{_subframeSize, _hopSize});
    _inFrame4kHz.reset(new FrameOverlap{_subframeSize, _hopSize});
    _inFrame500_2kHz.reset(new FrameOverlap{_subframeSize, _hopSize});
    _prevFrame.reset(new FrameOverlap{_subframeSize, _hopSize});

    _inBuffer.reset(new RingBuffer{frameSize});
    _hop.reset(new Frame{_hopSize});
    _frameHP.reset(new Frame{_hopSize});
    _frameBP.reset(new Frame{_hopSize});
    _frameLP.reset(new Frame{_hopSize});

#ifdef AUDIO_ALGO_DEBUG
    dbgInfo.reset(new float[_frameSize * dbgChannels]);
#endif
}

ClickRemoval::~ClickRemoval() {}

int ClickRemoval::process(float *buf, const int num) {
    if (num != _frameSize) return 0;

    int num_processed = 0;
    _inBuffer->putFrame(buf, num);

    while (_inBuffer->getFrame(_hop.get()) == _hopSize) {
        _inFrame->updateHop(_hop.get());

        _hpf4kHz->process(_hop->ptr(), _frameHP->ptr(), _hopSize);
        _inFrame4kHz->updateHop(_frameHP.get());

        float power = _inFrame->getPowerMean();
        float power_4kHz = _inFrame4kHz->getPowerMean();
        float power_500_2kHz = 0;

        _framePower = 0.9 * _framePower + 0.1 * power;

        if ((power > _threshold_all) && (power_4kHz > _threshold_4kHz)) {
            _bpf->process(_hop->ptr(), _frameBP->ptr(), _hopSize);
            _inFrame500_2kHz->updateHop(_frameBP.get());
            power_500_2kHz = _inFrame500_2kHz->getPowerMean();

            if (power_500_2kHz > 0.0001) { _detected = 3; }
        }

#ifdef AUDIO_ALGO_DEBUG
        for (int i = 0; i < _hopSize; i++) {
            dbgInfo[dbgChannels * (i + num_processed)] = power;
            dbgInfo[dbgChannels * (i + num_processed) + 1] = power_4kHz;
            dbgInfo[dbgChannels * (i + num_processed) + 2] = power_500_2kHz;
        }
#endif

        // method 1:
        _inFrame->getHop(buf + num_processed, _hopSize);

        if (_detected) {
            // if (_framePower > 0.01) {
            //     _removeFilter->process(_prevFrame->ptr(), _prevFrame->ptr(), _hopSize);
            //     _removeFilter->process(buf + num_processed, buf + num_processed, _hopSize);
            // } else {
                _lpf4kHz->process(_prevFrame->ptr(), _prevFrame->ptr(), _hopSize);
                _lpf4kHz->process(buf + num_processed, buf + num_processed, _hopSize);
            // }
            _detected--;
        }
        _prevFrame->copyFrame(_inFrame.get());


        // // method 1.a:
        // if (_detected) {
        //     // frame power vad
        //     if (_framePower > 0.01) {
        //         _prevFrame->getHop(buf + num_processed, _hopSize);
        //         // _removeFilter->process(buf + num_processed, buf + num_processed, _hopSize);
        //     } else {
        //         _inFrame->getHop(buf + num_processed, _hopSize);
        //         _lpf4kHz->process(buf + num_processed, buf + num_processed, _hopSize);
        //     }
        //     // memset(buf + num_processed, 0, sizeof(float) * _hopSize);
        //     _detected--;
        // } else {
        //     _inFrame->getHop(buf + num_processed, _hopSize);
        //     _prevFrame->copyFrame(_inFrame.get());
        //     // _removeFilter->process(_prevFrame->ptr(), _prevFrame->ptr(), _hopSize);
        // }

        // method 2:
        // if (!_detected) {
        //     _inFrame->getOutput(buf + num_processed, _hopSize);
        //     _prevFrame->copyFrame(*_inFrame.get());
        // } else {
        //     // _inFrame->getOutput(buf + num_processed, _hopSize);
        //     _prevFrame->getOutput(buf + num_processed, _hopSize);
        //     _lpf4kHz->process(buf + num_processed, buf + num_processed, _hopSize);
        //     // for (int i = 0; i < _hopSize; i++) { buf[i + num_processed] *= 0.25; }
        //     _detected--;
        // }

        num_processed += _hopSize;
    }
    memset(buf + num_processed, 0, (num - num_processed) * sizeof(float));

    return num_processed;
}

int ClickRemoval::process(int16_t *buf, const int num) {
    if (_floatBuf == nullptr) { _floatBuf.reset(new float[_frameSize]); }

    for (int i = 0; i < num; i++) { _floatBuf[i] = (float)buf[i] / 32768.f; }
    int num_processed = process(_floatBuf.get(), num);
    for (int i = 0; i < num_processed; i++) { buf[i] = (int16_t)(_floatBuf[i] * 32768.f); }
    for (int i = num_processed; i < num; i++) { buf[i] = 0; }

    return num_processed;
}

} // namespace ubnt
