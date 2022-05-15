#include "Engine.h"
#include "Config.h"
#include "System.h"
#include "NoteTrackEngine.h"
#include "swvPrint.h"

Engine::Engine(Model &model, ClockTimer& clockTimer, AdcInternal& adc, DacInternal& dac, Dio& dio) :
    _model(model),
    _project(model.project()),
    _clock(clockTimer),
    _adc(adc),
    _dac(dac),
    _dio(dio)
{
    _trackEngine = nullptr;
}

void Engine::init() {
    _clock.init();
    initClock();
    updateTrackSetup();
}

bool Engine::update() {
    uint32_t systemTicks = System::ticks();
    float dt = (0.001f * (systemTicks - _lastSystemTicks));
    _lastSystemTicks = systemTicks;

    updateClockSetup();

    // TODO:
    // read adc input
    // read buttons (?)

    uint32_t tick;
    bool outputUpdated = false;
    while(_clock.checkTick(&tick)) {
        _tick = tick;

        bool updated = _trackEngine->tick(tick);
        if(updated) {
            _trackEngine->update(0.f);
            updateTrackOutputs();
            outputUpdated = true;
            // notify UI?
        }
    }
    
    if(outputUpdated) {
        _trackEngine->update(dt);
        updateTrackOutputs();
    }

    return outputUpdated || static_cast<NoteTrackEngine*>(_trackEngine)->stepTriggered();
}

void Engine::togglePlay() {
    if(clockRunning()) {
        clockStop();
    } else {
        clockStart();
    }
}

void Engine::clockStart() {
    updateClockSetup();
    _clock.masterStart();
}

void Engine::clockStop() {
    _clock.masterStop();
}

bool Engine::clockRunning() {
    return _clock.isRunning();
}

uint32_t Engine::noteDivisor() const {
    return _project.timeSignature().noteDivisor();
}

uint32_t Engine::measureDivisor() const {
    return _project.timeSignature().measureDivisor();
}


void Engine::keyDown(KeyEvent &event) {
    if(event.key().isStep() && event.count() > 1) {
        static_cast<NoteTrackEngine*>(_trackEngine)->sequence().step(event.key().code()).toggleGate();
    }
}

void Engine::keyUp(KeyEvent &event) {

}

// called by Clock::notifyObservers
void Engine::onClockOutput(const IClockObserver::OutputState& state) {
    _dio.setClock(state.clock);
}

void Engine::updateTrackSetup() {
    if(!_trackEngine) {
        _trackEngine = new NoteTrackEngine(*this, _model, _project.track());
    }
}

void Engine::updateTrackOutputs() {
    float cvOutput = _trackEngine->cvOutput();
    bool gateOutput = _trackEngine->gateOutput();
    //DBG("Ticks: %ld: Progress: %.2f, Gate: %d, CV: %.2f", _lastSystemTicks, _trackEngine->sequenceProgress(), gateOutput, cvOutput);
    _dac.setValue(cvOutput);
    _dio.setGate(gateOutput);
}

void Engine::initClock() {
    _clock.attach(this);
}

void Engine::updateClockSetup() {
    auto &clockSetup = _project.clockSetup();

    _clock.outputConfigureSwing(clockSetup.clockOutputSwing() ? _project.swing() : 0);

    if(!clockSetup.isDirty()) {
        return;
    }
    
    _clock.outputConfigure(clockSetup.clockOutputDivisor() * (CONFIG_PPQN / CONFIG_SEQUENCE_PPQN), clockSetup.clockOutputPulse());

    onClockOutput(_clock.outputState());

    clockSetup.clearDirty();
}