#include "main.h"
#include <cstdint>

#define PCA9685_MODE1  0x00 // location for Mode1 register address
#define PCA9685_MODE2  0x01 // location for Mode2 reigster address
#define PCA9685_LED0   0x06 // location for start of LED0 registers
#define PRE_SCALE_MODE 0xFE //location for setting prescale (clock speed)

#define NUM_PWM_LED_CHIPS 2
#define NUM_LEDS_PER_CHIP 16

#define LED_I2C        I2C2
#define I2C_SPEEDCLOCK 100000
#define I2C_DUTYCYCLE  LL_I2C_DUTYCYCLE_2
#define LED_ON         0
#define LED_OFF        1

class LEDDriver {
public:
    void init();
    void process();
    void notifyTxComplete();
    void notifyTxError();
    void LEDDriver_setRGBLED_RGB(uint8_t led_number, uint16_t c_red, uint16_t c_green, uint16_t c_blue);
    void LEDDriver_set_single_LED(uint8_t led_element_number, uint16_t brightness);


private:
    void resetChip(uint32_t chipNumber);
    ErrorStatus writeSingleRegister(uint8_t chipNumber, uint8_t registerAddress, uint8_t registerValue);
    ErrorStatus writeRegisters(uint8_t chipNumber, uint8_t startRegister, uint8_t* registerValues, uint32_t count);
    void  checkStatus();
    ErrorStatus startTransfer();
    ErrorStatus stopTransfer();
    ErrorStatus sendAddress(uint8_t chipNumber);
    ErrorStatus sendData(uint8_t data);
    ErrorStatus sendData(uint8_t* data, uint32_t lenData);
    ErrorStatus sendSoftwareReset();
    void ledEnable();
    void ledDisable();
    
    uint8_t LEDDriver_get_cur_buf(void);
    uint8_t LEDDriver_get_cur_chip(void);

    uint32_t LEDDriver_init_dma(uint8_t numdrivers, uint8_t *led_image1, uint8_t *led_image2);
    uint32_t LEDDriver_init_direct(uint8_t numdrivers);

    uint8_t get_red_led_element_id(uint8_t rgb_led_id);
    uint8_t get_chip_num(uint8_t rgb_led_id);

    uint32_t _updateMillis;
    uint8_t _transmissionBusy = 0;
    uint16_t _pwmLeds[NUM_PWM_LED_CHIPS][NUM_LEDS_PER_CHIP][2];
    uint8_t _chipAddress[NUM_PWM_LED_CHIPS] = {1, 2};
};