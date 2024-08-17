#ifndef __CHARGECON__
#define __CHARGECON__

typedef struct{
	uint8_t mpp;
	uint16_t VBattMin;		// battery idle voltage
	uint16_t IMax;
	
}MPPData;


#include <stdint.h>

MPPData CHARGECON_findMPP(int tempAir);
void CHARGECON_trackMPP(uint8_t dutyCycle, int tempAir, int duration);


#endif
