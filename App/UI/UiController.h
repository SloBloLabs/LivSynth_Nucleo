#include "Engine.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include "Event.h"
#include "KeyPressEventTracker.h"

class UiController {
public:
    UiController(Model &model, Engine &engine, AdcInternal &adc, DacInternal &dac, Dio &dio, ButtonMatrix &buttonMatrix, LEDDriver &leds) :
        _model(model),
        _engine(engine),
        _adc(adc),
        _dac(dac),
        _dio(dio),
        _buttonMatrix(buttonMatrix),
        _leds(leds)
    {}

    void init();
    void handleKeys();
    void renderSequence();

private:
    
    float hueFromNote(uint32_t note);
    void handleEvent(KeyEvent event);

    Model &_model;
    Engine &_engine;
    AdcInternal &_adc;
    DacInternal &_dac;
    Dio &_dio;
    ButtonMatrix &_buttonMatrix;
    LEDDriver &_leds;

    KeyState _keyState;
    KeyPressEventTracker _keyPressEventTracker;

    uint32_t _lastControllerUpdateTicks;
};