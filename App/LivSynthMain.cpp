#include "LivSynthMain.hpp"
#include "main.h"
#include <cstdio>

#define NUM_ADC_VALUES 2

static volatile uint32_t _tick;
static volatile uint32_t _beat;
static volatile float    _bpm;
static volatile uint32_t _sequencerState;
static volatile uint32_t _sequenceDivisor;
static volatile float    _gateTime;
static volatile uint16_t _adcValues[NUM_ADC_VALUES];
static volatile float    _pitch;

void appMain() {
    // Configure and enable Systick timer including interrupt
    SysTick_Config((SystemCoreClock / 1000) - 1);

    // Start DMA and ADC
    LL_DMA_ConfigAddresses(
        DMA2,
        LL_DMA_STREAM_0,
        LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
        (uint32_t)&_adcValues,
        LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, NUM_ADC_VALUES);
    // Optional! DMA will transfer even without calling ISR
    // Can be enabled for debugging purposes
    //LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);

    LL_ADC_Enable(ADC1);
    LL_ADC_REG_StartConversionSWStart(ADC1);

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

    uint32_t startMillis, endMillis, logMillis = 0, updateMillis = 0;

    endMillis = _tick;

    while(true) {
        startMillis = endMillis;
        if(startMillis - updateMillis > 199) {
            updateMillis = startMillis;
            setTempo();
            setPitch();
        }
        if(startMillis - logMillis > 999) {
            logMillis = startMillis;
            DBG("ADC0=%d, ADC3=%d, bpm=%.2f, pitch=%.2f", _adcValues[0], _adcValues[1], _bpm, _pitch);
        }
        //while(!LL_GPIO_IsInputPinSet(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin));

        /*for(uint8_t i = 0; i < 20; ++i) {
            LL_GPIO_TogglePin(GPIOB, LED_GREEN_Pin|LED_RED_Pin|LED_BLUE_Pin);
            printf("loop %d\n", i);
            LL_mDelay(50);
        }*/
        endMillis = _tick;
    }
}

void appTick(void) {
    ++_tick;
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
        DBG("Beat %ld", _beat);
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
}

void appDMA2Request() {
    LL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_15);
    // TODO: apply some filtering and smoothing
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
    _bpm = adc2bpm(_adcValues[0]);
    //DBG("Tempo: %.2f", bpm);
    uint32_t ARR = bpm2ARR(_bpm);
    uint32_t CNT = LL_TIM_GetCounter(TIM2);
    if(CNT >= ARR) {
        LL_TIM_SetCounter(TIM2, CNT % ARR);
    }
    LL_TIM_SetAutoReload(TIM2, ARR - 1);
}

void setPitch() {
    _pitch = adc2Volt(_adcValues[1]);
}