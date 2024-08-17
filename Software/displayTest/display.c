#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "display.h"


#define NDIGITS 4

// digit Pins
#define HIGH0	(1<<PA6)
#define HIGH1	(1<<PA4)
#define HIGH2	(1<<PA5)
#define HIGH3	(1<<PA3)
#define LOW0	(1<<PB0)
#define LOW1	(1<<PB2)
#define LOW2	(1<<PB1)
#define LOW3	(1<<PB4)

// segment Pins
#define SEG0	(1<<PC6)
#define SEG1	(1<<PC5)
#define SEG2	(1<<PC4)
#define SEG3	(1<<PC3)
#define SEG4	(1<<PC2)
#define SEG5	(1<<PC1)
#define SEG6	(1<<PC0)


// private functions:
static void wait(uint32_t time_ms);		// wait funktion
static void send(char string[]);
static uint8_t charTo7Seg(char chr);
static void setPins(int digit, int seg, bool showSeg);

static uint8_t segmentBitsOld [NDIGITS]={0};


// hide all segments
void DISPLAY_clearDisplay(void)
{	
	for(int digit=0; digit<NDIGITS; digit++)
	{
		for(int seg=0; seg<7; seg++)
		{
			setPins(digit, seg, false);
		}
	}
	
	memset(segmentBitsOld, 0, sizeof(segmentBitsOld));
}


void DISPLAY_printTemp(int tempWater, int tempAir)
{
	char chrBuff[NDIGITS+1];
	char string[60];

	uint8_t segmentBits [NDIGITS];
	memset(segmentBits, 0, sizeof(segmentBits));

	
	// convert the decimal temperature into a string: chrBuff[tempWater, tempAir, \0]
	if((tempWater<-9 || tempWater>99) && (tempAir<-9 || tempAir>99))
	{
		snprintf(chrBuff, NDIGITS+1, "----");
	}
	else if((tempWater<-9 || tempWater>99))
	{
		snprintf(chrBuff, NDIGITS+1, "--%2d", tempAir);
	}
	else if(tempAir<-9 || tempAir>99)
	{
		snprintf(chrBuff, NDIGITS+1, "%2d--", tempWater);
	}
	else
	{
		snprintf(chrBuff, NDIGITS+1, "%2d%2d", tempWater, tempAir);
	}
	
	snprintf(string, 60, "charBuffer:\"%s\"\n", chrBuff);
	send(string);
	

	// convert chrBuff to binary output values and write display
	for(int digit=0; digit<NDIGITS; digit++)
	{
		// cahrTo7Seg() returns an uint8_t with the binary valu of the singel segments
		segmentBits[digit]=charTo7Seg(chrBuff[digit]);
		
		snprintf(string, 60, "segmentBitsOld[%d]:%d\n", digit, segmentBitsOld[digit]);
		send(string);
		snprintf(string, 60, "segmentBits[%d]:%d\n", digit, segmentBits[digit]);
		send(string);
		
		// write digit on the display
		for(int seg=0; seg<7; seg++)
		{
			// if segmentBits is 1 and segmentBitsOld is 0
			if((segmentBits[digit]&(1<<seg)) > (segmentBitsOld[digit]&(1<<seg)))
			{
				setPins(digit, seg, true);
			}
			// if segmentBits is 0 and segmentBitsOld is 1
			else if((segmentBits[digit]&(1<<seg)) < (segmentBitsOld[digit]&(1<<seg)))
			{
				setPins(digit, seg, false);
			}
		}
	}
	send("\n");
	memcpy(segmentBitsOld, segmentBits, sizeof(segmentBitsOld[0])*NDIGITS);		// save current display output
	wait(1000);
}


// convert string to actual binary value of the segments
static uint8_t charTo7Seg(char chr)
{
	uint8_t digit=0;

	switch(chr)
	{
		//		segments   GFEDCBA
		case '0': digit=0b00111111; break;
		case '1': digit=0b00000110; break;
		case '2': digit=0b01011011; break;
		case '3': digit=0b01001111; break;
		case '4': digit=0b01100110; break;
		case '5': digit=0b01101101; break;
		case '6': digit=0b01111101; break;
		case '7': digit=0b00000111; break;
		case '8': digit=0b01111111; break;
		case '9': digit=0b01101111; break;
		case ' ': digit=0b00000000; break;
		case '-': digit=0b01000000; break;
	}
	return digit;
}


// set all pins so that the right segment flips in the right direction
static void setPins(int digit, int seg, bool showSeg)
{
	char string[60];
	snprintf(string, 60, "setPins(), digit:%d, seg:%d, showSeg:%d\n", digit, seg, showSeg);
	send(string);
	
	if(showSeg)
	{
		// alle segmente auf low setzen und alle digit inputs in der luft hängen lassen
		// dass heißt alle leitungen zu den mosfets für die digits auf low setzen
		PORTC&=(~SEG0)&(~SEG1)&(~SEG2)&(~SEG3)&(~SEG4)&(~SEG5)&(~SEG6);
		PORTA&=(~HIGH0)&(~HIGH1)&(~HIGH2)&(~HIGH3);
		PORTB&=(~LOW0)&(~LOW1)&(~LOW2)&(~LOW3);
		
		switch(digit)
		{			
			case 0: PORTB|=LOW0; break;
			case 1: PORTB|=LOW1; break;
			case 2: PORTB|=LOW2; break;
			case 3: PORTB|=LOW3; break;
		}
		switch(seg)
		{
			case 0: PORTC|=SEG0; break;
			case 1: PORTC|=SEG1; break;
			case 2: PORTC|=SEG2; break;
			case 3: PORTC|=SEG3; break;
			case 4: PORTC|=SEG4; break;
			case 5: PORTC|=SEG5; break;
			case 6: PORTC|=SEG6; break;
		}
	}
	else
	{	
		// alle segmente auf high setzen und alle digit inputs in der luft hängen lassen
		PORTC|=SEG0|SEG1|SEG2|SEG3|SEG4|SEG5|SEG6;
		PORTA&=(~HIGH0)&(~HIGH1)&(~HIGH2)&(~HIGH3);
		PORTB&=(~LOW0)&(~LOW1)&(~LOW2)&(~LOW3);
		
		switch(digit)
		{			
			case 0: PORTA|=HIGH0; break;
			case 1: PORTA|=HIGH1; break;
			case 2: PORTA|=HIGH2; break;
			case 3: PORTA|=HIGH3; break;
		}
		switch(seg)
		{
			case 0: PORTC&=(~SEG0); break;
			case 1: PORTC&=(~SEG1); break;
			case 2: PORTC&=(~SEG2); break;
			case 3: PORTC&=(~SEG3); break;
			case 4: PORTC&=(~SEG4); break;
			case 5: PORTC&=(~SEG5); break;
			case 6: PORTC&=(~SEG6); break;
		}
	}
	wait(20);	// umschaltzeit von mosfets berücksichtigen
	
	// +BATT und +6V verbinden
	PORTA|=(1<<PA7);
	wait(5);
	PORTA&=~(1<<PA7);
	
	// alle Ausgänge auf low schalten 
	PORTC&=(~SEG0)&(~SEG1)&(~SEG2)&(~SEG3)&(~SEG4)&(~SEG5)&(~SEG6);
	PORTA&=(~HIGH0)&(~HIGH1)&(~HIGH2)&(~HIGH3);
	PORTB&=(~LOW0)&(~LOW1)&(~LOW2)&(~LOW3);
	
	wait(100);
}

void wait(uint32_t time_ms)	  			// wait function
{
	time_ms*=2;
	TCCR0=0x02;		// 8 prescaler
	while(time_ms>0)
	{
		while(!(TIFR & 0x01));
		TIFR |= 0x01; //clear TOV0 
		time_ms--;
	}
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
