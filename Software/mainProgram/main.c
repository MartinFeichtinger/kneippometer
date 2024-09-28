#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "tempsensor.h"
#include "display.h"
#include "chargeCon.h"		// Charge controller
#include "main.h"			// to use wait() and send() in the orther source files


#define BAUD	25			// 9600 baud

// private functions
static void sleep(void);

volatile bool awake=true;


int main(void)
{
	// initiate serial communication
	UBRRH=(unsigned char) (BAUD>>8);	// baudrate
	UBRRL=(unsigned char) BAUD;
	UCSRB=(1<<RXEN)|(1<<TXEN);			// enable receiver and transmitter
	UCSRC=(1<<URSEL)|(3<<UCSZ0);		// 8 Bit
	send("Atmega16 is alive!\n");
	
	// initiate controller pins 
	DDRA |= HIGH0|HIGH1|HIGH2|HIGH3|(1<<PA7);
	DDRB |= LOW0|LOW1|LOW2|LOW3|(1<<PB3);
	DDRC |= 0xff;
	DDRD |= 0x00;
	
	// initiate 10-Bit ADC
	ADCSRA |= (1<<ADEN)|(0b111<<ADPS0);			// enable ADC, 128x prescaler => 125kHz @ 16MHz

	// initiate PWM timer Register
	TCCR0 |= (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS00);
	OCR0 = 0;
	
	// set all output pins low
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;

	// enable interrupt
	sei();		
	
	// initiate variables
	int8_t tempWater=0;
	int8_t tempAir=0;
	uint16_t VBatt=0;
	MPPData data;
	
	// charge the battery after ervery boot to at least 6.2V
	tempAir=SENSOR_getTemp(AIR);
	data=CHARGECON_findMPP(tempAir);
	
	do{
		CHARGECON_trackMPP(data.mpp, tempAir, 4600);		// duration=4600; => ca. 10 min
		VBatt=CHARGECON_getVBatt();
	}
	while(VBatt < 620);		// VBatt < 6.2V
	
	// clear display for initialisation
	DISPLAY_clearDisplay();
	
	while(true)
	{
		// for debug via serial outputs
		//const int stringLen=99;
		//char string [stringLen+1];
		//memset(string, 0, stringLen+1);
		
		// get temperature from sensors
		tempWater=SENSOR_getTemp(WATER);
		tempAir=SENSOR_getTemp(AIR);
		
		// findMPP() returns a struct which contains the MPP, the battery voltage and the maximal charging current
		// it needs the Air temperatur to calculate the allowed battery voltage
		data=CHARGECON_findMPP(tempAir);
		VBatt=CHARGECON_getVBatt();
		
		// to prevent the battery from deep discharge
		// changed
		if(VBatt > 620)
		{
			DISPLAY_printTemp(tempWater, tempAir);
			//snprintf(string, stringLen, "Wasser Temperatur =%ddeg	Luft Temperatur =%ddeg\n", tempWater, tempAir);
			//send(string);
		}
		// TO DO
		
		// decide between sleep mode and track MPP
		if(data.IMax < 8)
		{
			sleep();
		}
		else
		{
			CHARGECON_trackMPP(data.mpp, tempAir, 4600);		// duration=460; => ca. 1 min		
		}
	}
}

// this function was made by Friedrich Feichtinger
void wait(uint32_t time_ms)	  			
{
	time_ms*=1;
	TCCR2=0x02;		// 8 prescaler
	while(time_ms>0)
	{
		while(!(TIFR & (1<<TOV2)));
		TIFR |= (1<<TOV2); //clear TOV2 
		time_ms--;
	}
}


// this function was made by Friedrich Feichtinger
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


static void sleep(void)
{
	awake=false;
	//SIG_LED_ON;
	
	/*		16 Bit Timer 1  	*/
	TIFR|=(1<<TOV1);					// Interrupts löschen
	TIMSK|=(1<<TOIE1);					// Timer overflow Interrupt freischalten;
	TCNT1=0;							// Timer zurücksetzen
	TCCR1B|=(1<<CS12)|(1<<CS10);		// Vorteiler 1024
	
	
	while(!awake)
	{
		set_sleep_mode(SLEEP_MODE_IDLE);	// set POWER DOWN IDLE
		sleep_enable();
	}
	
	TCCR1B=0;
	//SIG_LED_OFF;
}

// interupt for the sleep-mode
ISR(TIMER1_OVF_vect)
{
	static uint16_t ticks=0;
	
	ticks++;
	if(ticks>=30)	// 30 ticks = 8min
	{
		ticks=0;
		awake=true;
	}
}
