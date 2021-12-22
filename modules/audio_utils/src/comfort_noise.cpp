#include "comfort_noise.h"

namespace ubnt {

ComfortNoise::ComfortNoise(int maxCngLevel) : _maxCngLevel(maxCngLevel) {}

ComfortNoise::~ComfortNoise() {}

bool ComfortNoise::Process(int16_t* buf, int32_t bufSize) {
    int32_t w32Temp;
    int16_t w16Seed = _w16Seed;

    for (int i = 0; i < bufSize; ++i) {
        // generate random seed
        w32Temp = w16Seed * RNDGEN_A + RNDGEN_B;
        w16Seed = (int16_t)w32Temp;
        w32Temp = (int32_t)w16Seed * _maxCngLevel;
        w32Temp = (w32Temp + 16384) >> 15; // rounded

        // use LPF, freqz([1 0.75], [1 -0.75])
        _w16LPFStateY = (int16_t)((((int32_t)_w16LPFStateY * 3) + 2) >> 2);
        _w16LPFStateY += (int16_t)w32Temp;
        _w16LPFStateY += _w16LPFStateX;
        _w16LPFStateX = (int16_t)((((int32_t)((int16_t)w32Temp) * 3) + 2) >> 2);
        w32Temp = (_w16LPFStateY + 2) >> 2;

        // output
        buf[i] += (int16_t)w32Temp;
    }
    _w16Seed = w16Seed;

    return true;
}

} // namespace ubnt
