#include "countdown.h"

using namespace ubnt::smartaudio;


CountDown::CountDown(unsigned int countNumber) { 
    CountNumber = countNumber;
}

unsigned int CountDown::count() {
    if (_counter > 0) _counter--;
    return _counter;
}

void CountDown::reset() {
    _counter = CountNumber;
}

void CountDown::setCounter(unsigned int count) {
    _counter = count;
}