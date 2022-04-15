#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void appMain(void);
void appTick(void);
void appBeat(uint32_t type);
void appClockTimer();
void appButtonPressed();
void appADCCompleteRequest();
void appSPICompleteRequest();
void appLEDTxComplete();
void appLEDTxError();
float adc2bpm(uint16_t adcValue);
float adc2Volt(uint16_t adcValue);
uint32_t bpm2ARR(float bpm);
void startSequencer();
void stopSequencer();
void setTempo();
void setPitch();

#ifdef __cplusplus
}
#endif
