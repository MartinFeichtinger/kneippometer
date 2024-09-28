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

#define LED_ON	PORTC |= (1<<PC7)
#define LED_OFF	PORTC &= (~(1<<PC7))

#define BAUD	25	// 9600 baud

void wait(uint32_t time_ms);		// wait funktion
void send(char string[]);


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
	PORTC &= (~(1<<PC7));	// LED
	
	int tempWater=88;
	int tempAir=88;

	DISPLAY_clearDisplay();
	while(true)
	{
		send("stayAlive\n");
		//DISPLAY_printTemp(0, 0);	// tempAir, tempWater
		send("clear display...\n");
		LED_ON;
		DISPLAY_clearDisplay();
		wait(2000);
		
		send("open display...\n");
		LED_OFF;
		DISPLAY_printTemp(tempAir, tempWater);
		wait(2000);
	}
	
	return 0;
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
