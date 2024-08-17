#include <stdio.h>
#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#define WAITUS(X)	{	\
						volatile uint16_t time=X/4.52;		\
						while(time>0)						\
							time--;							\
					}
					
#define BAUD	25	// 9600 baud
#define VBATT	0  
#define VSOLAR	1
#define VSHUNT	2


// used sub functions:
void send(char string[]);
uint16_t readAnanlogPin(uint8_t pin);
int getVBattMax(int tempAir);
uint8_t findMPP(int tempAir);

// ************ main function ****************

int main(void)
{
	// initiate serial communication
	UBRRH=(unsigned char) (BAUD>>8);	// baudrate
	UBRRL=(unsigned char) BAUD;
	UCSRB=(1<<RXEN)|(1<<TXEN);			// enable receiver and transmitter
	UCSRC=(1<<URSEL)|(3<<UCSZ0);		// 8 Bit
	
	// Port configuration
	DDRA |= (1<<PA3)|(1<<PA4)|(1<<PA5)|(1<<PA6)|(1<<PA7);	// 0b11111000
	DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4);	// 0b00011111
	DDRC |= 0xFF;											// 0b11111111
	DDRD |= (1<<PD0)|(1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD7);	// 0b11111101
	
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	// timer Register
	TCCR0 |= (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS00);
	OCR0 = 0;
	
	
	uint16_t VBatt=0;
	uint16_t VBattMax=0;
	uint16_t VSolar=0;
	
	uint16_t VShunt=0;
	uint16_t ISolar=0;
	uint16_t IMax=200;
	
	uint32_t Pl=0;
	uint32_t PlOld=0;
	
	uint8_t dutyCycle=0;
	bool increaseDuty=true;
	
	int tempAir=20;
	char string [60];


	/*	10-Bit ADC	*/
	ADCSRA |= (1<<ADEN)|(0b111<<ADPS0);			// enable ADC, 128x prescaler => 125kHz @ 16MHz
	
	
	while(true)		// Endlosschleife
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
			dutyCycle=findMPP(tempAir);
		}
		
		OCR0 = dutyCycle;
		WAITUS(100000)
	}
}

// check all meaningful dutycycles to find the MPP
uint8_t findMPP(int tempAir)
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
	
	for(int dutyCycle=30; dutyCycle<=200; dutyCycle=dutyCycle+5)
	{
		OCR0 = dutyCycle;
		WAITUS(70000);
		
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
		
		snprintf(string, 60, "VBatt=%d	ISolar=%d	D=%d	Pl=%lu	MPP=%d\n", VBatt, ISolar, dutyCycle, Pl, mpp);			
		send(string);
	}
	
	if(IMax<=5)
	{
		// sleep(10 min);
	}
	
	return mpp;
}


// calc max allowed battery chargevoltage depending on temperature 
int getVBattMax(int tempAir)
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
uint16_t readAnanlogPin(uint8_t pin)
{
	ADMUX = (0b01<<REFS0)|(pin<<MUX0);
	ADCSRA|=(1<<ADSC);						// start ADC
	while(ADCSRA & (1<<ADSC));				// wait for ADC
	
	return ADC;
}


// function to send a string over the serial port.
// WARNING: this function blocks until the whole string is sent.
void send(char string[])
{
	int i=0;							// point to first character
	while(string[i]!='\0')				// do until end of string ('\0') is reached
	{
		while(!(UCSRA & (1<<UDRE)));	// wait until serial port is free for next character
		UDR=string[i];					// send character
		i++;							// next character
	}
}
