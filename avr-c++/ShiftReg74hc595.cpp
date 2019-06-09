/*
 * clsShiftReg74hc595.cpp
 * Class for the 75HC595 serial-in, parallel-out shift register.

 * Created: 11/21/2017 10:21:45 AM
 * Author : Steve
 */
#ifndef __SHIFTREG_74HC595__
#define __SHIFTREG_74HC595__

#include <avr/io.h>
#include <util/delay.h>			// probably not needed once we've finished

#ifndef LOW
	#define LOW 0
#endif
#ifndef HIGH
	#define HIGH 1
#endif
#ifndef LSBFIRST
	#define LSBFIRST 0
#endif
#ifndef MSBFIRST
	#define MSBFIRST 1
#endif

class ShiftReg74hc595
{
	public:
		uint8_t shift_CP;			// clock pin - SH_CP
		uint8_t store_CP;			// clock pin - ST_CP
		uint8_t data_DS;			// data pin - DS or SER
		volatile uint8_t * port;	// port

	ShiftReg74hc595(uint8_t SH_CP_pin, uint8_t ST_CP_pin, uint8_t data_pin, volatile uint8_t * portReg, volatile uint8_t * dirReg) {
		shift_CP = SH_CP_pin;
		store_CP = ST_CP_pin;
		data_DS = data_pin;
		port = portReg;
		// Set pins as outputs
		*dirReg = *dirReg | (1 << shift_CP) | (1 << store_CP) | (1 << data_DS);
		// set initial states for clock pins
		_setPin(shift_CP, LOW);	// start LOW ready for shifting
		_setPin(store_CP, LOW);	// start LOW ready for latching
	}

	void shiftOut(uint8_t byteVal, uint8_t bitOrder = MSBFIRST, bool invert = false)
	{
		// Synonym for shiftOutWithLatching
		_shiftOut(byteVal, bitOrder, invert, true);
	}

	void shiftOutWithLatching(uint8_t byteVal, uint8_t bitOrder = MSBFIRST, bool invert = false)
	{
		_shiftOut(byteVal, bitOrder, invert, true);
	}

	void shiftOutNoLatching(uint8_t byteVal, uint8_t bitOrder = MSBFIRST, bool invert = false)
	{
		_shiftOut(byteVal, bitOrder, invert, false);
	}

	protected:
		void _setPin(uint8_t pin, uint8_t hilo)
		{
			// example call: writeBit(&PORTB, PB1, HIGH);
			if(hilo == HIGH) {
				*port |= (1 << pin);
			} else {
				*port &= ~(1 << pin);
			}
		}

		void _shiftOut(uint8_t byteVal, uint8_t bitOrder = MSBFIRST, bool invert = false, bool latching = true)
		{
			if (invert) {
				byteVal = 0xFF ^ byteVal;
			}
			if (!latching) _setPin(store_CP, HIGH);
			for(uint8_t bit=0; bit < 8; bit++) {			// Clock in data
				// the results of the following operations need to be a 1 or 0
				if (bitOrder == LSBFIRST) {
					_setPin(data_DS, (byteVal & (1 << bit)) >> bit);
				} else {
					_setPin(data_DS, (byteVal & (1 << (7 - bit))) >> (7 - bit) );
				}
				_setPin(shift_CP, HIGH);		// shift clock going high shifts in bit
				_setPin(shift_CP, LOW);
			}
			_setPin(store_CP, HIGH);			// bring storage clock pulse high
			_setPin(store_CP, LOW);				// then low to latch in data
		}

};

#endif
