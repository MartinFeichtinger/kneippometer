#include <stdio.h>
#include <avr/io.h>
#include <math.h>
#include <stdbool.h>

#include "chargeCon.h"
#include "main.h"


#define VBATT	0  
#define VSOLAR	1
#define VSHUNT	2


// privat functions:
static uint16_t readAnanlogPin(uint8_t pin);
static int getVBattMax(int tempAir);

// ************ main function ****************
void CHARGECON_trackMPP(uint8_t dutyCycle, int tempAir, int duration)
{	
	
	int time=0;
	
	uint16_t VBatt=0;
	uint16_t VBattMax=0;
	uint16_t VSolar=0;
	
	uint16_t VShunt=0;
	uint16_t ISolar=0;
	uint16_t IMax=200;
	
	uint32_t Pl=0;
	uint32_t PlOld=0;
	
	bool increaseDuty=true;
	
	char string [60];


	while(time<duration)		// Endlosschleife
	{
		VBatt=readAnanlogPin(VBATT);
		VSolar=(uint32_t)readAnanlogPin(VSOLAR)*100/420;
		VShunt=readAnanlogPin(VSHUNT);
		
		Pl=(uint32_t)VBatt*(uint32_t)VShunt;		
		ISolar=VShunt/4;
		VBatt=(uint32_t)VBatt*1000/1012;
		VBattMax=getVBattMax(tempAir);	
		
		snprintf(string, 60, "VBatt=%d	I=%d	D=%d	Pl=%lu	PlOld=%lu\n", VBatt, ISolar, dutyCycle, Pl, PlOld);			
		send(string);
		
		
		if(dutyCycle > 30 && dutyCycle < 200)
		{
			// mpp tracking
			if(Pl<PlOld)
			{
				increaseDuty=!increaseDuty;
			}
			
			if(increaseDuty && VBatt<VBattMax && ISolar<IMax)
			{	
				if(dutyCycle<200)
				{
					dutyCycle++;
				}
			}
			else
			{
				if(dutyCycle>30)
				{
					dutyCycle--;
				}
			}
			
			PlOld=Pl;
		}
		else
		{
			dutyCycle=CHARGECON_findMPP(tempAir);
		}
		
		OCR0 = dutyCycle;
		wait(100);
		time++;
	}
	
	send("bye\n");
}

// check all meaningful dutycycles to find the MPP
uint8_t CHARGECON_findMPP(int tempAir)
{
	uint8_t mpp=0;
	
	uint16_t VBatt=0;
	uint16_t VBattMax=getVBattMax(tempAir);
	
	uint16_t VShunt=0;
	uint16_t ISolar=0;
	uint16_t IMaxAllowed=200;
	uint16_t IMax=0;
	
	uint32_t Pl=0;
	uint32_t PlMax=0;
	char string [60];
	
	send("findMPP();\n");
	snprintf(string, 60, "VBattMax=%d\n", VBattMax);			
	send(string);
	send("VBatt	ISolar	D	Pl	MPP\n");
	
	for(int dutyCycle=0; dutyCycle<=255; dutyCycle=dutyCycle+5)
	{
		OCR0 = dutyCycle;
		wait(50);
		
		VBatt=readAnanlogPin(VBATT);
		VShunt=readAnanlogPin(VSHUNT);
		
		Pl=(uint32_t)VBatt*(uint32_t)VShunt;
		ISolar=VShunt/4;
		VBatt=(uint32_t)VBatt*1000/1012;

		
		if(VBatt>VBattMax || ISolar>IMaxAllowed)
		{
			break;
		}
		if(Pl>PlMax)
		{
			PlMax=Pl;
			mpp=dutyCycle;
		}
		if(ISolar>IMax)
		{
			IMax=ISolar;
		}
		
		snprintf(string, 60, "%d	%d	%d	%lu	%d\n", VBatt, ISolar, dutyCycle, Pl, mpp);			
		send(string);
	}
	
	if(IMax<=5)
	{
		// sleep(10 min);
	}
	
	OCR0 = 0;
	
	return mpp;
}


// calc max allowed battery chargevoltage depending on temperature 
static int getVBattMax(int tempAir)
{
	int tempBatt=0;
	uint16_t VBattMax=0;
	
	// keep tempBatt in a relistical area if tempAir input is nonsenes
	if(tempAir > -30 && tempAir < 50)
	{
		tempBatt=tempAir;
	}
	else
	{
		tempBatt=25;
	}
	
	// linear funktion to get max standby voltage for 3 cell battery
	// Quelle: https://www.powerstream.com/SLA.htm
	VBattMax=((-0.2)*tempBatt+235)*3;
	
	return VBattMax;
}


// read input voltage from analog pin
static uint16_t readAnanlogPin(uint8_t pin)
{
	ADMUX = (0b01<<REFS0)|(pin<<MUX0);
	ADCSRA|=(1<<ADSC);						// start ADC
	while(ADCSRA & (1<<ADSC));				// wait for ADC
	
	return ADC;
}
