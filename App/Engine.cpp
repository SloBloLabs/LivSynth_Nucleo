#include "Engine.h"

Engine::Engine(ClockTimer& clockTimer, Adc& adc, DacInternal& dac, Dio& dio) :
    _clock(clockTimer),
    _adc(adc),
    _dac(dac),
    _dio(dio)
{
    
}

void Engine::init() {
    _clock.init();
    initClock();
}

void Engine::update() {
    updateClockSetup();
}

void Engine::togglePlay() {
    if(clockRunning()) {
        clockStop();
    } else {
        clockStart();
    }
}

void Engine::clockStart() {
    _clock.masterStart();
}

void Engine::clockStop() {
    _clock.masterStop();
}

bool Engine::clockRunning() {
    return _clock.isRunning();
}

void Engine::onClockOutput(const IClockObserver::OutputState& state) {
    _dio.setGate(state.clock);
}

void Engine::initClock() {
    _clock.attach(this);
}

void Engine::updateClockSetup() {
    _clock.outputConfigureSwing(0);
// Sequence parts per quarter note resolution
#define SEQUENCE_PPQN 48
    uint8_t outputDivisor = 12;
    uint8_t outputPulse = 1;
    _clock.outputConfigure(outputDivisor * (_clock.ppqn() / SEQUENCE_PPQN), outputPulse);
}