#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define NDIGITS 4
#define BAUD	25	// 9600 baud

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



// used sub functions:

void wait(uint32_t time_ms);		// wait funktion
void send(char string[]);
uint8_t charTo7Seg(char chr);
void clearDisplay(void);
void setPins(int digit, int seg, bool showSeg);

// ************ main function ****************

int main(void)
{
		// initiate serial communication
	UBRRH=(unsigned char) (BAUD>>8);	// baudrate
	UBRRL=(unsigned char) BAUD;
	UCSRB=(1<<RXEN)|(1<<TXEN);			// enable receiver and transmitter
	UCSRC=(1<<URSEL)|(3<<UCSZ0);		// 8 Bit
	
	DDRA |= HIGH0|HIGH1|HIGH2|HIGH3|(1<<PA7);
	DDRB |= LOW0|LOW1|LOW2|LOW3;
	DDRC |= 0xff;
	DDRD |= 0x00;
	
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	int tempWater=89;
	int tempAir=5;
	char chrBuff[NDIGITS+1];
	char string[60];

	uint8_t segmentBitsOld [NDIGITS];
    uint8_t segmentBits [NDIGITS];
    memset(segmentBitsOld, 0, sizeof(segmentBitsOld));
    memset(segmentBits, 0, sizeof(segmentBits));
    
    clearDisplay();
    /*snprintf(string, 60, "clearDisplay()---finished;");
	send(string);*/
    
	while(true)
	{
        memcpy(segmentBitsOld, segmentBits, sizeof(segmentBitsOld[0])*NDIGITS);
        
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
		
		/*snprintf(string, 60, "   charBuffer:\"%s\"\n", chrBuff);
		send(string);*/
		
        for(int digit=0; digit<NDIGITS; digit++)
        {
			// cahrTo7Seg() gibt einen uint8_t mit den einzelen Segmenten in binär zurück
			segmentBits[digit]=charTo7Seg(chrBuff[digit]);
			
			/*snprintf(string, 60, "segmentBitsOld[%d]:%d\n", digit, segmentBitsOld[digit]);
			send(string);
			snprintf(string, 60, "segmentBits[%d]:%d\n", digit, segmentBits[digit]);
			send(string);*/
			
			for(int seg=0; seg<7; seg++)
			{
				// wenn segmentBits 1 ist und segmentBitsOld 0 ist
				if((segmentBits[digit]&(1<<seg)) > (segmentBitsOld[digit]&(1<<seg)))
				{
					setPins(digit, seg, true);
				
				}
				// wenn segmentBits 0 ist und segmentBitsOld 1 ist
				else if((segmentBits[digit]&(1<<seg)) < (segmentBitsOld[digit]&(1<<seg)))
				{
					setPins(digit, seg, false);
				
				}
			}
		}
		tempWater++;
		tempAir--;
		
		wait(10000);
	}
}


uint8_t charTo7Seg(char chr)
{
	uint8_t digit=0;

	switch(chr)
	{
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


void setPins(int digit, int seg, bool showSeg)
{
	/*char string[60];
	snprintf(string, 60, "setPins(), digit:%d, seg:%d, showSeg:%d\n", digit, seg, showSeg);
	send(string);*/
	
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
	wait(20);
	PORTA&=~(1<<PA7);
	
	PORTC&=(~SEG0)&(~SEG1)&(~SEG2)&(~SEG3)&(~SEG4)&(~SEG5)&(~SEG6);
	PORTA&=(~HIGH0)&(~HIGH1)&(~HIGH2)&(~HIGH3);
	PORTB&=(~LOW0)&(~LOW1)&(~LOW2)&(~LOW3);
	
	wait(500);
}


void clearDisplay(void)
{
	/*char string[20];
	snprintf(string, 20, "clearDisplay()\n");
	send(string);*/
	
	for(int digit=0; digit<NDIGITS; digit++)
	{
		for(int seg=0; seg<7; seg++)
		{
			setPins(digit, seg, false);
		}
	}
}


void wait(uint32_t time_ms)	  			// wait function
{
	time_ms*=3;
	TCCR0=0x01;		// 8 prescaler
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
