/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */
#pragma once

#include <cstdint>

namespace ubnt {

class ComfortNoise {
private:
    int16_t _w16Seed{21835};
    int16_t RNDGEN_A{31821};
    int16_t RNDGEN_B{13849};
    int16_t _maxCngLevel{128};
    int16_t _w16LPFStateX{0};
    int16_t _w16LPFStateY{0};

public:
    ComfortNoise(int maxCngLevel);
    virtual ~ComfortNoise();

    bool Process(int16_t* buffer, int bufSize);
};

} // namespace ubnt
