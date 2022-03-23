#pragma once

#include <stdio.h>
#include <string.h>

namespace ubnt {
namespace smartaudio {

class CountDown {
public:
    CountDown() = default;
    CountDown(unsigned int countNumber);
    unsigned int count();
    void reset();
    void setCounter(unsigned int count);

private:
    unsigned int CountNumber{0};
    unsigned int _counter{0};
};

} // namespace smartaudio
} // namespace ubnt