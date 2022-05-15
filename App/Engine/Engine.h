#pragma once

#include "Model.h"
#include "Project.h"
#include "Clock.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "Dio.h"
#include "TrackEngine.h"
#include "Event.h"

class Engine : private IClockObserver {
public:
    Engine(Model &model, ClockTimer& clockTimer, AdcInternal& adc, DacInternal& dac, Dio& dio);
    void init();
    bool update();

    void togglePlay();
    void clockStart();
    void clockStop();
    bool clockRunning();
    void updateClockSetup();

    inline uint32_t tick() const { return _tick; }
    uint32_t noteDivisor() const;
    uint32_t measureDivisor() const;

    inline TrackEngine* trackEngine() {
        return _trackEngine;
    }

    // event handlers
    void keyDown(KeyEvent &event);
    void keyUp(KeyEvent &event);

private:
    virtual void onClockOutput(const IClockObserver::OutputState& state) override;

    void updateTrackSetup();
    void updateTrackOutputs();

    void initClock();
    
    Model &_model;
    Project &_project;
    
    Clock _clock;
    AdcInternal _adc;
    DacInternal _dac;
    Dio _dio;

    TrackEngine* _trackEngine;

    uint32_t _tick = 0;
    uint32_t _lastSystemTicks = 0;
};