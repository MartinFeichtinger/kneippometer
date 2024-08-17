#ifndef __CHARGECON__
#define __CHARGECON__


#include <stdint.h>

uint8_t CHARGECON_findMPP(int tempAir);
void CHARGECON_trackMPP(uint8_t dutyCycle, int tempAir, int duration);


#endif
