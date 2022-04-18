#include "Clock.h"
#include "main.h"
#include "Groove.h"
#include <cstdio>

Clock::Clock(ClockTimer &timer) :
    _timer(timer)
{
    resetTicks();
    _ppqn = CONFIG_PPQN;
    _masterBpm = 120.f;
    _runState = RunState::Idle;
    _timer.attach(this);
}

void Clock::init() {
    _timer.disable();
}

void Clock::masterStart() {
    setRunState(RunState::MasterRunning);
    resetTicks();

    _timer.disable();
    setupMasterTimer();
    _timer.enable();
}

void Clock::masterStop() {
    setRunState(RunState::Idle);
    _timer.disable();
}

void Clock::resetTicks() {
    _tick = 0;
    _tickProcessed = 0;
    _output.nextTick = 0;
}

void Clock::setMasterBpm(float bpm) {
    _masterBpm = bpm;
    setupMasterTimer();
}

void Clock::outputConfigure(int divisor, int pulse) {
    _output.divisor = divisor;
    _output.pulse = pulse;
}

void Clock::outputConfigureSwing(int swing) {
    _output.swing = swing;
}
 // called from ClockTimer ISR
void Clock::onClockTimerTick() {
    //DBG("Clock Timer Tick");
    switch(_runState) {
    case RunState::MasterRunning: {
        outputTick(_tick);
        ++_tick;
        _elapsedUs += _timer.period();
        break;
    }
    default:
        break;
    }
}

void Clock::setupMasterTimer() {
    _elapsedUs = 0;
    uint32_t us = (60 * 1000000) / (_masterBpm * _ppqn);
    _timer.setPeriod(us);
}

// called from onClockTimerTick
void Clock::outputTick(uint32_t tick) {

    auto applySwing = [this] (uint32_t tick) {
        return _output.swing != 0 ? Groove::applySwing(tick, _output.swing) : tick;
    };

    if(tick == _output.nextTick) {
        uint32_t divisor = _output.divisor;
        uint32_t clockDuration = std::max(uint32_t(1), uint32_t(_masterBpm * _ppqn * _output.pulse / (60 * 1000)));

        _output.nextTickOn = applySwing(_output.nextTick);
        _output.nextTickOff = std::min(_output.nextTickOn + clockDuration, applySwing(_output.nextTick + divisor) - 1);
        
        _output.nextTick += divisor;
    }

    if(tick == _output.nextTickOn) {
        outputClock(true);
    }

    if(tick == _output.nextTickOff) {
        outputClock(false);
    }
}

void Clock::outputClock(bool clock) {
    if(clock != _outputState.clock) {
        _outputState.clock = clock;
        notifyObservers();
    }
}