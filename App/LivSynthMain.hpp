#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void appMain(void);
void appTick(void);
void appBeat(uint32_t type);
void appButtonPressed();
void appDMA2Request();
float adc2bpm(uint16_t adcValue);
uint32_t bpm2ARR(float bpm);
void startSequencer();
void stopSequencer();
void setTempo();

#ifdef __cplusplus
}
#endif
