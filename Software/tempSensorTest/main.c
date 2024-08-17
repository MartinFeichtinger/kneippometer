#include <stdio.h>
#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "tempsensor.h"

#define BAUD	25	// 9600 baud


#define WAITUS(X)	{	\
						volatile uint16_t time=X/4.52;		\
						while(time>0)						\
							time--;							\
					}


// used sub functions:
void send(char string[]);

// ************ main function ****************

int main(void)
{
		// initiate serial communication
	UBRRH=(unsigned char) (BAUD>>8);	// baudrate
	UBRRL=(unsigned char) BAUD;
	UCSRB=(1<<RXEN)|(1<<TXEN);			// enable receiver and transmitter
	UCSRC=(1<<URSEL)|(3<<UCSZ0);		// 8 Bit
	
	
	DDRA |= 0x00;
	DDRB |= 0x00;
	DDRC |= (1<<PC7);
	DDRD |= 0x00;
	
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	PORTD &= ~(1<<PD5);	
	PORTD &= ~(1<<PD6);
	
	int8_t tempWater=0;
	int8_t tempAir=0;
	
	while(true)
	{
		char string [100];
		memset(string, 0, sizeof(string));
		
		
		tempWater=SENSOR_getTemp(WATER);
		WAITUS(5000);
		tempAir=SENSOR_getTemp(AIR);
		
		snprintf(string, 99, "Wasser Temperatur =%ddeg	Luft Temperatur =%ddeg\n", tempWater, tempAir);
		
		
		send(string);
		
		WAITUS(5000);	
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
