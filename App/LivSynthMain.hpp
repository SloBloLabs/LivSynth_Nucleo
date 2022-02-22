#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void appMain(void);
void appTick(void);
void appBeat(void);
uint32_t bpm2ARR(float bpm);

#ifdef __cplusplus
}
#endif
