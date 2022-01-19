/**
 *  Copyright (C) 2021, Ubiquiti Networks, Inc,
 */

#pragma once

#include <memory>

namespace ubnt {

template <typename T>
class RingBuffer {
public:
    /* Constructor, create with default capacity 15360 samples */
    RingBuffer();

    /* Constructor
     * @parameter frameSize in samples
     * @parameter num, how many frames stored in buffer
     */
    RingBuffer(const int frameSize, const int num);

    /* Destructor, delete _data buffer */
    ~RingBuffer();

    /* Resize with capacity
     * @parameter capacity, in samples
     */
    bool resetCapacity(const int capacity);

    /* Put zeros into buffer
     * @parameter num, in samples
     */
    bool setDelay(const int delaySamples);

    /* Copy new frame into ring buffer
     * @parameter newFrame, pointer to new frame data
     * @parameter num, length of newFrame in samples
     */
    bool putFrame(const T* newFrame, const int num);

    /* Copy frame into buffer
     * @parameter buffer, destination of copy
     * @parameter num, length of buffer samples
     * @return how many samples copied
     */
    int getFrame(T* buffer, const int num);

    /* Get samples in use
     * @return how many samples in use
     */
    int getInUseSamples();

    /* print member info */
    void showInfo();

private:
    T* _data{nullptr};
    int _capacity{15360};
    int _inUseStart{0};
    int _inUseEnd{0};
    int _inUseLength{0};
};

template class RingBuffer<float>;
template class RingBuffer<int16_t>;

} // namespace ubnt
