#pragma once

#include <stdio.h>
#include <string.h>

namespace ubnt {
namespace smartaudio {

class Observer {
public:
    Observer() = default;
    ~Observer() = default;
    Observer(int length);
    void release();
    unsigned int sum();
    void put(bool data);
    bool get();
    bool get(int index);
    void reset();
    int getLength();
    int getCurrentIndex();

private:
    bool *buffer{nullptr};
    int curIndex{0};
    int bufferLen{0};
};

} // namespace smartaudio
} // namespace ubnt