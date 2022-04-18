#include "Dio.h"
#include "main.h"

void Dio::init() {

}

void Dio::setGate(bool gate) {
    if(gate) {
        LL_GPIO_SetOutputPin(GATE_GPIO_Port, GATE_Pin);
    } else {
        LL_GPIO_ResetOutputPin(GATE_GPIO_Port, GATE_Pin);
    }
}