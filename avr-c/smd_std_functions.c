/*
	std_functions

	Oft-used functions
*/

#include "smd_std_functions.h"

uint8_t readPin(volatile uint8_t * portreg, uint8_t pin)
{
	// The input register - eg, PINB - is passed by reference.
	// Example call: readPin(&PORTC, PC3);
	if((*portreg & (1 << pin)) == (1 << pin)) { // pin is high
		return 1;
	} else {
		return 0;
	}
}

void setPin(volatile uint8_t * portreg, uint8_t pin, uint8_t hilo)
{
	// The output register - eg, PORTB - is passed by reference.
	// Example call: setPin(&PORTB, PB1, HIGH);
	if(hilo == 1) {
		*portreg |= (1 << pin);
	} else {
		*portreg &= ~(1 << pin);
	}
}
