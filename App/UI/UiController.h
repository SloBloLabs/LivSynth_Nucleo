#include "Engine.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include "Event.h"
#include "KeyPressEventTracker.h"

class UiController {
public:
    UiController(Model &model, Engine &engine) :
        _model(model),
        _engine(engine)
    {}

    void init();
    void handleKeys();
    void renderSequence();

private:
    
    float hueFromNote(uint32_t note);
    void handleEvent(KeyEvent event);

    Model &_model;
    Engine &_engine;

    KeyState _keyState;
    KeyPressEventTracker _keyPressEventTracker;

    uint32_t _lastControllerUpdateTicks;
};