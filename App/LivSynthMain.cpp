#include "LivSynthMain.hpp"
#include "main.h"
#include <cstdio>

static volatile uint32_t _tick;
static volatile uint32_t _beat;
static volatile float    _bpm;
static volatile uint32_t _sequencerState;

void appMain() {
    // Configure and enable Systick timer including interrupt
    SysTick_Config(SystemCoreClock / 1000 - 1);
    stopSequencer();

    // Enable TIM2 update event interrupt (Call TIM2_IRQHandler)
    LL_TIM_EnableIT_UPDATE(TIM2);

    _bpm = 120.;

    uint32_t startMillis, endMillis, logMillis = 0;

    endMillis = _tick;

    while(true) {
        startMillis = endMillis;
        if(startMillis - logMillis > 100) {
            logMillis = startMillis;
            //printf("TIM2 Cnt: %ld\n", TIM2->CNT);
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

void buttonPressed() {
    if(_sequencerState == 1) {
        stopSequencer();
    } else if(_sequencerState == 0) {
        startSequencer(_bpm);
    }
}

void appBeat(void) {
    ++_beat;
    printf("Beat %ld\n", _beat);
    LL_GPIO_SetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_6);
    LL_mDelay(5);
    LL_GPIO_ResetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);
}

uint32_t bpm2ARR(float bpm) {
    /*
    target clock: 20bpm - 300bpm
    resolution = 0,01 bpm => 0,00016666s = 1/6000Hz
    20bpm = ,33333Hz =  2000 * 1/6000Hz
    300bpm = 5Hz     = 30000 * 1/6000Hz
    
    source clock = 90MHz
    90MHz / x = 6kHz -> x = 90MHz / 6kHz = 15000

    TIM2
    Presaler(max) = 65536
    PSC = 15000 -> 6000Hz
    ARR = 18000 -> 20bpm
    ARR = 1200  -> 300bpm

    ARR = 6000 / bpm * 60;
    
    */
    float arr = 360000. / bpm;
    return (static_cast<uint32_t>(arr));
}

void startSequencer(float bpm) {
    uint32_t ARR_VALUE = bpm2ARR(bpm);
    printf("ARR_VALUE = %ld\n", ARR_VALUE);
    LL_TIM_SetAutoReload(TIM2, ARR_VALUE);
    LL_TIM_SetCounter(TIM2, 0);
    _beat = 0;

    // Start TIM2
    LL_TIM_EnableCounter(TIM2);
    // Generate initial update event (sequencer beats immediately upon start)
    LL_TIM_GenerateEvent_UPDATE(TIM2);
    _sequencerState = 1;
    printf("Sequencer started.\n");
}

void stopSequencer() {
    LL_TIM_DisableCounter(TIM2);
    _sequencerState = 0;
    printf("Sequencer stopped.\n");
}