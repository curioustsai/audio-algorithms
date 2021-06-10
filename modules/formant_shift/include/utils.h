/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#ifndef __UTILS_H__
#define __UTILS_H__

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

#endif // __UTILS_H__
