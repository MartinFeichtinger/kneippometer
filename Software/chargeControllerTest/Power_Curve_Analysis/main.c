#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "chargeCon.h"
#include "main.h"

#define BAUD	25	// 9600 baud

// public functions
void wait(uint32_t time_ms);
void send(char string[]);


// main function
int main(void)
{
	// Port configuration
	DDRA |= (1<<PA3)|(1<<PA4)|(1<<PA5)|(1<<PA6)|(1<<PA7);	// 0b11111000
	DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4);	// 0b00011111
	DDRC |= 0xFF;											// 0b11111111
	DDRD |= (1<<PD0)|(1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD7);	// 0b11111101
	
	// initiate serial communication
	UBRRH=(unsigned char) (BAUD>>8);	// baudrate
	UBRRL=(unsigned char) BAUD;
	UCSRB=(1<<RXEN)|(1<<TXEN);			// enable receiver and transmitter
	UCSRC=(1<<URSEL)|(3<<UCSZ0);		// 8 Bit
	
	/*	10-Bit ADC	*/
	ADCSRA |= (1<<ADEN)|(0b111<<ADPS0);			// enable ADC, 128x prescaler => 125kHz @ 16MHz

	// timer Register
	TCCR0 |= (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS00);
	OCR0 = 0;
	
	
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	uint8_t dutyCycle;
	//while(true)
	{
		dutyCycle=CHARGECON_findMPP(20);
		//CHARGECON_trackMPP(dutyCycle, 20, 100);
	}
	
	return 0;
}


void wait(uint32_t time_ms)	  			// wait function
{
	time_ms*=10;
	TCCR2=0x02;		// 8 prescaler
	while(time_ms>0)
	{
		while(!(TIFR & 0x06));
		TIFR |= 0x06; //clear TOV0 
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
