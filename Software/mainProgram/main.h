#ifndef __MAIN__
#define __MAIN__

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

#define SIG_LED_ON 	PORTC|=(1<<PC7)
#define SIG_LED_OFF	PORTC&=(~(1<<PC7))


void wait(uint32_t time_ms);	  			// wait function
void send(char string[]);


#endif
