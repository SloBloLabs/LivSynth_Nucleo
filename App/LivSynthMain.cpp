#include "LivSynthMain.h"
#include "main.h"
#include "System.h"
#include "ClockTimer.h"
#include "Clock.h"
#include "AdcInternal.h"
#include "DacInternal.h"
#include "Dio.h"
#include "ShiftRegister.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include "Model.h"
#include "Engine.h"
#include "UiController.h"
#include <cstdio>
#include <bitset>

#define CCMRAM_BSS __attribute__((section(".ccmram")))

static volatile float    _bpm;
static volatile float    _pitch;
static volatile uint32_t _dacValue;

static CCMRAM_BSS ClockTimer    clockTimer;
static            AdcInternal   adc;
static CCMRAM_BSS DacInternal   dac;
static CCMRAM_BSS Dio           dio;
static CCMRAM_BSS ShiftRegister shiftRegister;
static CCMRAM_BSS ButtonMatrix  buttonMatrix(shiftRegister);
static            LEDDriver     ledDriver;
static            Model         model;
static CCMRAM_BSS Engine        engine(model, clockTimer, adc, dac, dio);
static CCMRAM_BSS UiController  uiController(model, engine, adc, dac, dio, buttonMatrix, ledDriver);

void appMain() {
    System::init();
    clockTimer.init();
    adc.init();
    dac.init();
    //dio.init();
    shiftRegister.init();
    ledDriver.init();
    engine.init();
    uiController.init();

    stopSequencer();

    _bpm = 120.;

    startSequencer();

    uint8_t curLed = 0, lastLed = 0;
    float hue = 0.;

    uint32_t curMillis
           , logMillis    = 0
           , updateMillis = 0
           , engineMillis = 0;
    
    bool debug = false;
    
    while(true) {

        curMillis = System::ticks();

        if(curMillis - engineMillis > 1) {
            engineMillis = curMillis;
            uiController.update();
            engine.update();
        }
        
        // update sequencer input and state
        if(curMillis - updateMillis > 79) {
            updateMillis = curMillis;

            /*for(uint8_t led = 0; led < 8; ++led) {
                ledDriver.setColourHSV(led, hue, 1., 1.);
            }
            hue += 10;
            if(hue >= 360.) hue -= 360.;

            ledDriver.setSingleLED(lastLed, 0x0);
            ledDriver.setSingleLED(curLed, 0xFFF);
            lastLed = curLed;
            if(!(++curLed % 15)) curLed++;
            if(curLed > 24) curLed = 0;*/

            ledDriver.process();

            std::bitset<8> myBitset;
            shiftRegister.process();
            setTempo();
            setPitch();
        }
        
        // render debug log output
        if(debug && curMillis - logMillis > 999) {
            logMillis = curMillis;
            DBG("ADC0=%d, ADC1=%d, bpm=%.2f, pitch=%.2f, buttons=0x%02X", adc.channel(0), adc.channel(1), _bpm, _pitch, shiftRegister.read(0));
        }
    }
}

// glue code
void appTick(void) {
    System::tick();
}

void appButtonPressed() {
    uint32_t buttonState = LL_GPIO_IsInputPinSet(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);
    if(buttonState) {
        engine.togglePlay();
    }
}

void appClockTimer() {
    clockTimer.notifyTimerUpdate();
}

void appLEDTxComplete() {
    ledDriver.notifyTxComplete();
}

void appLEDTxError() {
    ledDriver.notifyTxError();
}

void appADCCompleteRequest() {
    // only called if interrupt is enabled
    // see AdcInternal.cpp -> LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0)
    LL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}

float adc2bpm(uint16_t adcValue) {
    /*
    0 - 4095 ^= 20 - 300 bpm
    0    -> 20
    4095 -> 300
    f(x) = mx + b = dy/dx * x + b = (300 - 20) / 4095 * x + b
    f(0) = 280/4095 * 0 + b = 20
    f(x) = 280/4096 * x + 20
    */
   return 280.f/4096 * adcValue + 20.;
}

float adc2Volt(uint16_t adcValue) {
    /*
    we want 5 octaves
    0 - 4096 ^= 0 - 5 Volt
    f(x) = 5/4096 * x
    */
    return 5.f/4096 * adcValue;
}

void startSequencer() {
    setTempo();
    engine.clockStart();
    DBG("Sequencer started.");
}

void stopSequencer() {
    LL_GPIO_ResetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    engine.clockStop();
    DBG("Sequencer stopped.");
}

void setTempo() {
    _bpm = adc2bpm(adc.channel(0));
    //DBG("Tempo: %.2f", bpm);
}

void setPitch() {
    _pitch = adc2Volt(adc.channel(1));
    //DBG("Pitch: %.2f", _pitch);
}