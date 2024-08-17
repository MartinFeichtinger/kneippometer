// Datasheet: /kneipometer/Datenblätter/T_Sensor_DS18B20-1.pdf

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tempsensor.h"
#include "main.h"

// just for more than 5µs
#define WAITUS(X)	{	\
						volatile uint16_t time=X/4.556;		\
						while(time>0)						\
							time--;							\
					}
					


// private functions:
static bool initialize(void);
static void write(uint8_t byte);
static bool readBit(void);
static void readBytes(uint8_t* bytes, uint16_t length);
static uint8_t runCRC(const uint8_t* bytes, uint8_t length);


Sensor sensor;


int8_t SENSOR_getTemp(Sensor s)
{
	sensor=s;
	
	uint8_t bytes[9];
	memset(bytes, 0, sizeof(bytes));
	int8_t temp=0;
	
	if(initialize())
	{
		write(0xCC);			// skip rom
		write(0x44);			// converting
		while(!readBit());		// wait until converting is finished
		
		if(initialize())
		{
			write(0xCC);		// skip rom
			write(0xBE);		// read scratchpad
			
			readBytes(bytes, 9);
			
			//char s[20];
			//snprintf(s, 19, "0=%d 1=%d	", bytes[0], bytes[1]);
			//send(s);
			
			if(runCRC(bytes, 8) == bytes[8])
			{
				temp = (bytes[0]>>4) | (bytes[1]<<4);
				
				if(bytes[0]&(1<<3))
				{
					temp++;
				}
			
			}
			else
			{
				// crc doesn't match
				return -100;
			}
		}
	}
	else
	{
		// temperature sensor not connected
		return -101;
	}
	
	return temp;
}


static bool initialize(void)
{
	bool presence=false;
	DDRD |= (1<<sensor);
	WAITUS(1000)
	DDRD &= ~(1<<sensor);	
	WAITUS(70)

	if(PIND&(1<<sensor))
	{
		//send("sensor not found\n");
		presence=false;
	}
	else
	{
		//send("sensor founded\n");
		presence=true;
	}
	WAITUS(1000);

	return presence;
}


static void write(uint8_t byte)
{
	for(int i=0; i<8; i++)
	{
		if(byte&(1<<i))
		{
			//sent a 1
			DDRD |= (1<<sensor);
			WAITUS(1)
			DDRD &= ~(1<<sensor);	
			WAITUS(70)
		}
		else
		{
			//sent a 0
			DDRD |= (1<<sensor);
			WAITUS(70)
			DDRD &= ~(1<<sensor);
			WAITUS(1)
		}
	}
	
}


static bool readBit(void)
{
	bool bit=false;
	
	DDRD |= (1<<sensor);
	WAITUS(1)
	DDRD &= ~(1<<sensor);	
	WAITUS(5)
	bit=PIND&(1<<sensor);	
	WAITUS(60)
	
	return bit;
}


static void readBytes(uint8_t* bytes, uint16_t length)
{
	for(int i=0; i<length; i++)
	{
		for(int j=0; j<8; j++)
		{
			if(readBit()==1)
			{
				bytes[i] |= (1<<j);
			}
			else
			{
				bytes[i] &= ~(1<<j);
			}

			WAITUS(1)
		}
	}
}


static uint8_t runCRC(const uint8_t* bytes, uint8_t length)
{
	uint8_t crc=0;
	
	for(int i=0; i < length; ++i)		// byte counter
	{	
		uint8_t inbyte = bytes[i];
		for (uint8_t i = 8; i; i--) 
		{
			uint8_t mix = (crc ^ inbyte)&1;
			crc >>= 1;
			if (mix) crc ^= 0x8C;		// crc mask
			inbyte >>= 1;
		}
	}	
	//char s[20];
	//snprintf(s, 19, "%d==%d	", crc, bytes[8]);
	//send(s);
	
	return crc;
}
