#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void appMain(void);
void appTick(void);
void appBeat(void);
void buttonPressed();
uint32_t bpm2ARR(float bpm);
void startSequencer(float bpm);
void stopSequencer();

#ifdef __cplusplus
}
#endif
