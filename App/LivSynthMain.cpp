#include "LivSynthMain.h"
#include "main.h"
#include "System.h"
#include "ClockTimer.h"
#include "Clock.h"
#include "Adc.h"
#include "ShiftRegister.h"
#include "ButtonMatrix.h"
#include "LEDDriver.h"
#include <cstdio>
#include <bitset>

#define CCMRAM_BSS __attribute__((section(".ccmram")))

#define DAC_DELAY_VOLTAGE_SETTLING_CYCLES 29

static volatile uint32_t _beat;
static volatile float    _bpm;
static volatile uint32_t _sequencerState;
static volatile uint32_t _sequenceDivisor;
static volatile float    _gateTime;
static volatile float    _pitch;
static volatile uint32_t _dacValue;

static CCMRAM_BSS ClockTimer    clockTimer;
// TODO: move to engine class
static            Clock         clock(clockTimer);
static            Adc           adc;
static CCMRAM_BSS ShiftRegister shiftRegister;
static CCMRAM_BSS ButtonMatrix  buttonMatrix(shiftRegister);
static            LEDDriver     ledDriver;

void appMain() {
    System::init();
    clockTimer.init();
    adc.init();
    shiftRegister.init();
    ledDriver.init();
    //ledDriver.enableTestMode();
    LL_mDelay(300);

    // Start DAC
    LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, 0x00);
    LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
    volatile uint32_t wait_loop_index = ((LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US * (SystemCoreClock / (100000 * 2))) / 10);
    while(wait_loop_index != 0) {
        wait_loop_index--;
    }
    LL_DAC_EnableTrigger(DAC1, LL_DAC_CHANNEL_1);

    stopSequencer();

    // Enable TIM2 update event interrupt (Call TIM2_IRQHandler)
    LL_TIM_EnableIT_UPDATE(TIM2);
    // Enable TIM2 update event on CC1
    LL_TIM_EnableIT_CC1(TIM2);

    _bpm = 120.;
    //_sequenceDivisor = 1; // 4th
    //_sequenceDivisor = 2; // 8th
    _sequenceDivisor = 4; // 16th

    _gateTime = .02; // 20ms

    startSequencer();

    // uint8_t curLed = 0, lastLed = 0;
    float hue = 0.;

    uint32_t curMillis
           , logMillis    = 0
           , updateMillis = 0;
    
    while(true) {

        curMillis = System::ticks();
        
        // update sequencer input and state
        if(curMillis - updateMillis > 99) {
            updateMillis = curMillis;
            
            //ledDriver.setSingleLED(lastLed, 0x0);
            //ledDriver.setSingleLED(curLed, 0xFFF);
            //lastLed = curLed;
            //if(!(++curLed % 15)) curLed++;
            //if(curLed > 24) curLed = 0;
            for(uint8_t led = 0; led < 8; ++led) {
                ledDriver.setColourHSV(led, hue, 1., 1.);
            }
            hue += 10;
            if(hue > 360.) hue -= 360.;
            ledDriver.process();

            std::bitset<8> myBitset;
            shiftRegister.process();
            setTempo();
            setPitch();
        }
        
        // render debug log output
        if(curMillis - logMillis > 999) {
            logMillis = curMillis;
            DBG("ADC0=%d, ADC1=%d, bpm=%.2f, pitch=%.2f, buttons=0x%02X", adc.channel(0), adc.channel(1), _bpm, _pitch, shiftRegister.read(0));
            if(ledDriver.ledEnabled()) {
                ledDriver.ledDisable();
            } else {
                ledDriver.ledEnable();
            }
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
        if(_sequencerState == 1) {
            stopSequencer();
        } else if(_sequencerState == 0) {
            startSequencer();
        }
    }
}

void appBeat(uint32_t type) {
    switch(type) {
    case 0:
        ++_beat;
        //DBG("Beat %ld", _beat);
        LL_GPIO_SetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
        //LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_6);
        break;
    case 1:
        LL_GPIO_ResetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
        //LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);
        break;
    default:
        ;
    }

    _dacValue = (_beat % 2) == 0 ? 0x00 : 0xFFF;
    LL_DAC_ConvertData12RightAligned(DAC1, LL_DAC_CHANNEL_1, _dacValue);
    LL_DAC_TrigSWConversion(DAC1, LL_DAC_CHANNEL_1);
    volatile uint32_t wait_loop_index = DAC_DELAY_VOLTAGE_SETTLING_CYCLES;
    while(wait_loop_index != 0) {
        wait_loop_index--;
    }
    if(_dacValue) {
        LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_6);
    } else {
        LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);
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
    // see Adc.cpp -> LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0)
    //LL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_15);
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

uint32_t bpm2ARR(float bpm) {
    /*
    target clock: 20bpm - 300bpm
    1 bpm = 1 / 60 Hz
    resolution = 0,01 bpm => 0,00016666s = 1/6000Hz
    20bpm = ,33333Hz =  2000 * 1/6000Hz
    300bpm = 5Hz     = 30000 * 1/6000Hz
    
    source clock = 90MHz
    90MHz / x = 6kHz -> x = 90MHz / 6kHz = 15000

    PSC = 15000 -> 6000Hz
    ARR = 18000 -> 20bpm
    ARR = 1200  -> 300bpm

    ARR = 6000 / bpm * 60;
    */
    float arr = 360000. / bpm / _sequenceDivisor;
    return (static_cast<uint32_t>(arr));
}

void startSequencer() {
    setTempo();
    //uint32_t ARR_VALUE = bpm2ARR(bpm);
    //DBG("ARR_VALUE = %ld", ARR_VALUE);
    //LL_TIM_SetAutoReload(TIM2, ARR_VALUE - 1);
    LL_TIM_SetCounter(TIM2, 0);
    _beat = 0;

    // Enable OC1 for gate off
    LL_TIM_OC_SetCompareCH1(TIM2, 6000 * _gateTime);
    //LL_TIM_GenerateEvent_CC1(TIM2);

    // Start TIM2
    LL_TIM_EnableCounter(TIM2);
    // Generate initial update event (sequencer beats immediately upon start)
    //LL_TIM_GenerateEvent_UPDATE(TIM2);
    clockTimer.enable();
    _sequencerState = 1;
    DBG("Sequencer started.");
}

void stopSequencer() {
    LL_TIM_DisableCounter(TIM2);
    LL_GPIO_ResetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    //LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);
    _sequencerState = 0;
    DBG("Sequencer stopped.");
}

void setTempo() {
    _bpm = adc2bpm(adc.channel(0));
    //DBG("Tempo: %.2f", bpm);
    uint32_t ARR = bpm2ARR(_bpm);
    uint32_t CNT = LL_TIM_GetCounter(TIM2);
    if(CNT >= ARR) {
        LL_TIM_SetCounter(TIM2, CNT % ARR);
    }
    LL_TIM_SetAutoReload(TIM2, ARR - 1);
}

void setPitch() {
    _pitch = adc2Volt(adc.channel(0));
}