#include "LEDDriver.h"
#include "System.h"
#include <cstdio>

void LEDDriver::init() {
    //LL_I2C_Enable(LED_I2C); // -> already called in LL_I2C_Init
    //LL_I2C_EnableIT_EVT(LED_I2C);
    //LL_I2C_EnableIT_ERR(LED_I2C);

    for(uint32_t chip = 0; chip < NUM_PWM_LED_CHIPS; ++chip) {
        for(uint32_t led = 0; led < NUM_LEDS_PER_CHIP; ++led) {
            _pwmLeds[chip][led][LED_ON] = 1;
            _pwmLeds[chip][led][LED_OFF] = 1;
        }
    }

    for(uint32_t chip = 0; chip < NUM_PWM_LED_CHIPS; ++chip) {
        resetChip(_chipAddress[chip]);
    }

    ledEnable();

    _updateMillis = System::ticks();
}

void LEDDriver::process() {
    uint32_t curMillis = System::ticks();
    if((curMillis - _updateMillis) > 499) {
        // http://stefanfrings.de/stm32/stm32f1.html#i2c
        // https://community.st.com/s/question/0D50X00009bLPuwSAG/busy-bus-after-i2c-reading
        _updateMillis = curMillis;

        LL_GPIO_SetOutputPin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);

        _pwmLeds[0][0][LED_OFF] = 0x0FFF;

        ErrorStatus status;
        status = writeRegisters(_chipAddress[0], PCA9685_LED0, reinterpret_cast<uint8_t*>(_pwmLeds), 2 << 1);
        if(status == ERROR) {
            DBG("error writing to register");
        }

        checkStatus();

        LL_GPIO_ResetOutputPin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    }
}

void LEDDriver::notifyTxComplete() {
    DBG("LED Tx Complete!");
}

void LEDDriver::notifyTxError() {
    DBG("LED Tx Error!");
}

void LEDDriver::resetChip(uint32_t chipNumber) {
    // clear sleep mode
    ErrorStatus status = writeSingleRegister(chipNumber, PCA9685_MODE1, 0b00000000);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);

    // start reset mode
    status = writeSingleRegister(chipNumber, PCA9685_MODE1, 0b10000000);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);

    // enable auto increment
    status = writeSingleRegister(chipNumber, PCA9685_MODE1, 0b00100000);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);

    // INVERT=1, OUTDRV=0, OUTNE=01
    status = writeSingleRegister(chipNumber, PCA9685_MODE2, 0b00010001);
    if(status == ERROR) {
        DBG("error writing to register");
    }
    LL_mDelay(1);
}

ErrorStatus LEDDriver::writeSingleRegister(uint8_t chipNumber, uint8_t registerAddress, uint8_t registerValue) {
    return writeRegisters(chipNumber, registerAddress, &registerValue, 1);
}

ErrorStatus LEDDriver::writeRegisters(uint8_t chipNumber, uint8_t startRegister, uint8_t* registerValues, uint32_t count) {
    ErrorStatus ret = SUCCESS, status;

    status = startTransfer();
    if(status == SUCCESS) {
        status = sendAddress(chipNumber);
        if(status == ERROR) {
            ret = status;
        }
    
        status = sendData(startRegister);
        if(status == ERROR) {
            ret = status;
        }

        for(uint32_t i = 0; i < count; ++i) {
            status = sendData(registerValues[i]);
            if(status == ERROR) {
                ret = status;
            }
        }
    } else {
        ret = status;
    }

    stopTransfer();

    return ret;
}

void LEDDriver::checkStatus() {
    if(LL_I2C_IsActiveFlag_SB(LED_I2C))    { DBG("SB active"); }
    if(LL_I2C_IsActiveFlag_ADDR(LED_I2C))  { DBG("ADDR active"); }
    if(LL_I2C_IsActiveFlag_BTF(LED_I2C))   { DBG("BTF active"); }
    if(LL_I2C_IsActiveFlag_ADD10(LED_I2C)) { DBG("ADD10 active"); }
    if(LL_I2C_IsActiveFlag_STOP(LED_I2C))  { DBG("STOPF active"); }
    if(LL_I2C_IsActiveFlag_RXNE(LED_I2C))  { DBG("RxNE active"); }
    if(LL_I2C_IsActiveFlag_TXE(LED_I2C))   { DBG("TxE active"); }
    if(LL_I2C_IsActiveFlag_BERR(LED_I2C))  { DBG("BERR active"); }
    if(LL_I2C_IsActiveFlag_ARLO(LED_I2C))  { DBG("ARLO active"); }
    if(LL_I2C_IsActiveFlag_AF(LED_I2C))    { DBG("AF active"); }
    if(LL_I2C_IsActiveFlag_OVR(LED_I2C))   { DBG("OVR active"); }
}

ErrorStatus LEDDriver::startTransfer() {
    DBG("start transfer");
    ErrorStatus ret = SUCCESS;

    // wait for device to become ready
    while(LL_I2C_IsActiveFlag_BUSY(LED_I2C));
    uint32_t ticks = System::ticks();

    LL_I2C_GenerateStartCondition(LED_I2C);

    while(!LL_I2C_IsActiveFlag_SB(LED_I2C) && (System::ticks() - ticks) < 2);
    if(!LL_I2C_IsActiveFlag_SB(LED_I2C)) {
        ret = ERROR;
    }

    return ret;
}

ErrorStatus LEDDriver::stopTransfer() {
    DBG("stop transfer");
    ErrorStatus ret = SUCCESS;

    // wait until last byte transfer has finished
    uint32_t ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_BTF(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    }
    LL_I2C_GenerateStopCondition(LED_I2C);

    return ret;
}

ErrorStatus LEDDriver::sendAddress(uint8_t chipNumber) {
    DBG("send address");
    ErrorStatus ret = SUCCESS;
    uint8_t address = 0b10000000 | (chipNumber << 1);
    LL_I2C_TransmitData8(LED_I2C, address);
    
    uint32_t ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_ADDR(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    };
    
    if(ret == SUCCESS) {
        LL_I2C_ClearFlag_ADDR(LED_I2C);
    }

    return ret;
}

ErrorStatus LEDDriver::sendData(uint8_t data) {
    DBG("send data");
    ErrorStatus ret = SUCCESS;
    
    uint32_t ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_TXE(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    }

    if(ret == ERROR || !LL_I2C_IsActiveFlag_TXE(LED_I2C)) {
        DBG("TX buffer not empty, aborting");
        return ERROR;
    }
    
    LL_I2C_TransmitData8(LED_I2C, data);
    ticks = System::ticks();
    while(!LL_I2C_IsActiveFlag_TXE(LED_I2C) && (System::ticks() - ticks) < 2) {
        if(LL_I2C_IsActiveFlag_AF(LED_I2C)) {
            ret = ERROR;
        }
    }

    return ret;
}

ErrorStatus LEDDriver::sendData(uint8_t* data, uint32_t count) {
    ErrorStatus ret = SUCCESS;
    for(uint32_t i = 0; i < count; ++i) {
        ret = sendData(data[i]);
        if(ret == ERROR) {
            break;
        }
    }

    return ret;
}

ErrorStatus LEDDriver::sendSoftwareReset() {
    ErrorStatus ret = SUCCESS, status;

    status = startTransfer();
    if(status == SUCCESS) {
        for(uint32_t chip = 0; chip < NUM_PWM_LED_CHIPS; ++chip) {
            status = writeSingleRegister(chip + 1, 0x00, 0x06);
            if(status == ERROR) {
                ret = status;
            }
        }
    }

    stopTransfer();
    
    return ret;
}

void LEDDriver::ledEnable() {
    LL_GPIO_ResetOutputPin(LED_OEN_GPIO_Port, LED_OEN_Pin);
}

void LEDDriver::ledDisable() {
    LL_GPIO_SetOutputPin(LED_OEN_GPIO_Port, LED_OEN_Pin);
}