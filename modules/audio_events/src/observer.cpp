#include "observer.h"

using namespace ubnt::smartaudio;

Observer::Observer(int length) {
    bufferLen = length;
    buffer = new bool[bufferLen];
    memset(buffer, 0, bufferLen);
}
    
void Observer::release() {
    if (buffer != nullptr) {
        delete[] buffer; buffer = nullptr;
    }
}
    
unsigned int Observer::sum() {
    if (buffer == nullptr) return 0;

    unsigned int ret = 0;
    for (int i = 0; i < bufferLen; i++) {
        ret += buffer[i];
    }

    return ret;
}

void Observer::put(bool data) {
    buffer[curIndex] = data;
    curIndex = curIndex + 1 < bufferLen ? curIndex + 1 : 0;
}

bool Observer::get() {
    return buffer[curIndex];
}

bool Observer::get(int index) {
    while( index >= bufferLen ) index -= bufferLen;
    while ( index < 0 ) index += bufferLen;
    return buffer[index];
}

void Observer::reset() {
    curIndex = 0;
    memset(buffer, 0, sizeof(bool) * bufferLen);
}

int Observer::getLength() {
    return bufferLen;
}

int Observer::getCurrentIndex() {
    return curIndex;
}