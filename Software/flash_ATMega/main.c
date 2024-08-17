#include <avr/io.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


void wait(uint32_t time_ms);

int main(void)
{
	DDRC |= (1<<PC7);
	
	while(true)
	{

		PORTC |= (1<<PC7);
		wait(1000);
		PORTC &= (~(1<<PC7));
		wait(1000);
	
		/* // Port A
		for(int pin=3; pin<=7; pin++)
		{
			DDRA |= (1<<pin);
			PORTA |= (1<<pin);
			wait(1);
			PORTA &= (~(1<<pin));
			DDRA &= (~(1<<pin));
			wait(1);
		}
		// Port B	
		for(int pin=0; pin<=7; pin++)
		{
			DDRB |= (1<<pin);
			PORTB |= (1<<pin);
			wait(1);
			PORTB &= (~(1<<pin));
			DDRB &= (~(1<<pin));
			wait(1);
		}*/
		// Port C
		/*for(int pin=0; pin<=7; pin++)
		{
			PORTC |= (1<<pin);
			wait(5);
			PORTC &= (~(1<<pin));
			wait(500);
		}*/
		/* // Port D
		for(int pin=0; pin<=7; pin++)
		{
			DDRD |= (1<<pin);
			PORTD |= (1<<pin);
			wait(1);
			PORTD &= (~(1<<pin));
			DDRD &= (~(1<<pin));
			wait(1);
		}*/
	}
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
