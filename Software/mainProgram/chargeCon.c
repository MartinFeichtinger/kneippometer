#include <stdio.h>
#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "chargeCon.h"
#include "main.h"


#define VBATT	0  
#define VSOLAR	1
#define VSHUNT	2

static MPPData data={0};

// privat functions:
static uint16_t readAnalogPin(uint8_t pin);
static int getVBattMaxAllowed(int tempAir);

// ************ main function ****************
void CHARGECON_trackMPP(uint8_t dutyCycle, int tempAir, int duration)
{		
	int cycle=0;
	
	uint16_t VBatt=0;
	uint16_t VBattMaxAllowed=getVBattMaxAllowed(tempAir);	// get the maximal allowed battery voltage dependent on the temperature;
	//uint16_t VSolar=0;
	
	uint16_t VShunt=0;
	uint16_t ISolar=0;
	const uint16_t IMax=200;
	
	uint32_t Pl=0;
	uint32_t PlOld=0;
	
	bool increaseDuty=true;
	
	//char string [60];
	

	while(cycle<duration)
	{
		VBatt=readAnalogPin(VBATT);
		//VSolar=(uint32_t)readAnalogPin(VSOLAR)*100/420;
		VShunt=readAnalogPin(VSHUNT);
		
		Pl=(uint32_t)VBatt*(uint32_t)VShunt;			// P=U*I	// this is an unscaled calculation
		ISolar=VShunt/4;								// we have to scale the charge current because of the ADC and the amplifier
		VBatt=(uint32_t)VBatt*1000/1012;				// we have to scale the battery voltage because of the ADC and the votage divider
		
		//snprintf(string, 59, "VBatt=%d	I=%d	D=%d	Pl=%lu	PlOld=%lu\n", VBatt, ISolar, dutyCycle, Pl, PlOld);			
		//send(string);
		
		
		if(dutyCycle > 5 && dutyCycle < 170)	// keep the duty cycle in a realistic area
		{
			if(Pl<PlOld)
			{
				increaseDuty=!increaseDuty;		// go back in the other direction of the power curve
			}
			
			// go in the direction of open circuit votage if the battery votage or the loading current is to high
			// see also ./Programm/chargeControllerTest/Power_Curve_Analysis/CuteCom_log/Stromvergleich
			if(increaseDuty && VBatt<VBattMaxAllowed && ISolar<IMax)
			{
				dutyCycle++;
			}
			else
			{
				dutyCycle--;
			}	
			
			PlOld=Pl;
		}
		else
		{
			data=CHARGECON_findMPP(tempAir);
			
			if(data.IMax < 8)
			{
				OCR0=0;
				return;			
			}
		}
		
		OCR0 = dutyCycle;
		wait(200);
		cycle++;
	}
	
	OCR0=50;	// keep loading with 15mA while doing someting else
	return;
}

// check all meaningful dutycycles to find the MPP
MPPData CHARGECON_findMPP(int tempAir)
{	
	
	uint16_t VBatt=0;
	uint16_t VBattMaxAllowed=getVBattMaxAllowed(tempAir);
	
	uint16_t VShunt=0;
	uint16_t ISolar=0;
	const uint16_t IMaxAllowed=200;
	
	data.IMax=0;
	data.mpp=5;
	
	uint32_t Pl=0;
	uint32_t PlMax=0;
	
	/*char string [60];
	send("findMPP();\n");
	snprintf(string, 59, "VBattMaxAllowed=%d\n", VBattMaxAllowed);			
	send(string);*/

	for(int dutyCycle=75; dutyCycle<=170; dutyCycle=dutyCycle+5)		// go over all duty cycles
	{
		OCR0 = dutyCycle;
		wait(200);
		
		VBatt=readAnalogPin(VBATT);
		VShunt=readAnalogPin(VSHUNT);
		
		Pl=(uint32_t)VBatt*(uint32_t)VShunt;
		ISolar=VShunt/4;
		VBatt=(uint32_t)VBatt*1000/1012;

		
		if(VBatt>VBattMaxAllowed || ISolar>IMaxAllowed)		// cancle findMPP() if the battery voltage or charging current gets to high
		{
			break;
		}
		
		if(Pl>PlMax)
		{
			PlMax=Pl;
			data.mpp=dutyCycle;
		}
		if(ISolar>data.IMax)
		{
			data.IMax=ISolar;
		}
		
		//snprintf(string, 60, "VBatt=%d	ISolar=%d	D=%d	Pl=%lu	MPP=%d\n", VBatt, ISolar, dutyCycle, Pl, data.mpp);			
		//send(string);
	}
	
	OCR0=data.mpp;	// set the PWM dutycycle to the mpp
	
	return data;
}

// returns the current battery voltage (the charging process must be paused)
uint16_t CHARGECON_getVBatt(void)
{
	OCR0=0;		// stop the charging process
	
	wait(500);
	uint16_t VBattMin=readAnalogPin(VBATT);
	VBattMin=(uint32_t)VBattMin*1000/1012;		// scale the battery voltage
	
	//snprintf(string, 59, "VBattMin=%d\n", VBattMin);			
	//send(string);
	
	return VBattMin;
}

// calculate maximal allowed battery chargevoltage depending on the temperature 
static int getVBattMaxAllowed(int tempAir)
{
	int tempBatt=0;
	uint16_t VBattMaxAllowed=0;
	
	// keep tempBatt in a relistic area if tempAir input is nonsenes
	if(tempAir > -30 && tempAir < 50)
	{
		tempBatt=tempAir;
	}
	else
	{
		tempBatt=25;
	}
	
	// linear function to get max standby voltage for 3 cell battery
	// Quelle: https://www.powerstream.com/SLA.htm
	VBattMaxAllowed=((-0.2)*tempBatt+235)*3;
	
	return VBattMaxAllowed;
}


// this function was made by Friedrich Feichtinger
// read input voltage from analog pin
static uint16_t readAnalogPin(uint8_t pin)
{
	ADMUX = (0b01<<REFS0)|(pin<<MUX0);
	ADCSRA|=(1<<ADSC);						// start ADC
	while(ADCSRA & (1<<ADSC));				// wait for ADC
	
	return ADC;
}
