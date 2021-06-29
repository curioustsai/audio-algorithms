/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

inline void freeBuffer(float **buf) {
    if (*buf != nullptr) { 
        delete[] *buf; *buf = nullptr;
    }
}

inline int circIndex(int idx, int boundary) {
    while (idx >= boundary) {
        idx -= boundary;
    }

    while (idx < -boundary) {
        idx += boundary;
    }

    return idx;
}