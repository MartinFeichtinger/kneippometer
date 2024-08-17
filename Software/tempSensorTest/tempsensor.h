

#ifndef __TEMPSENSOR__
#define __TEMPSENSOR__

#include <stdint.h>

typedef enum {AIR=5, WATER=6} Sensor;

int8_t SENSOR_getTemp(Sensor s);


#endif
