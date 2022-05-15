#include "Dio.h"
#include "main.h"

void Dio::setClock(bool clock) {
    _clock = clock;
    if(_clock) {
        LL_GPIO_SetOutputPin(CLOCK_GPIO_Port, CLOCK_Pin);
    } else {
        LL_GPIO_ResetOutputPin(CLOCK_GPIO_Port, CLOCK_Pin);
    }
}

void Dio::setReset(bool reset) {
    _reset = reset;
    if(_reset) {
        LL_GPIO_SetOutputPin(RESET_GPIO_Port, RESET_Pin);
    } else {
        LL_GPIO_ResetOutputPin(RESET_GPIO_Port, RESET_Pin);
    }
}

void Dio::setGate(bool gate) {
    _gate = gate;
}

void Dio::update() {
    if(_gate) {
        LL_GPIO_SetOutputPin(GATE_GPIO_Port, GATE_Pin);
    } else {
        LL_GPIO_ResetOutputPin(GATE_GPIO_Port, GATE_Pin);
    }
}