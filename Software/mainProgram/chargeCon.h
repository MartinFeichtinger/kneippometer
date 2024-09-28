#ifndef __CHARGECON__
#define __CHARGECON__

typedef struct{
	uint8_t mpp;
	uint16_t IMax;
}MPPData;


#include <stdint.h>

MPPData CHARGECON_findMPP(int tempAir);
uint16_t CHARGECON_getVBatt(void);
void CHARGECON_trackMPP(uint8_t dutyCycle, int tempAir, int duration);


#endif
