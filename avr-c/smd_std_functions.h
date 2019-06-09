/*
	std_functions
	Oft-used functions for AVR-based projects.
*/

#ifndef __SMD_STD_FUNCTIONS__
#define __SMD_STD_FUNCTIONS__

#include <avr/io.h>
//#include "std_defines.h" // try not to use this - make this lib self-contained

uint8_t readPin(volatile uint8_t * portreg, uint8_t pin);
void setPin(volatile uint8_t * portreg, uint8_t pin, uint8_t hilo);

#endif
