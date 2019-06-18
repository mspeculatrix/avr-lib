/*
 * ShiftReg74hc595.h
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

	ShiftReg74hc595(uint8_t SH_CP_pin, uint8_t ST_CP_pin, uint8_t data_pin, volatile uint8_t * portReg, volatile uint8_t * dirReg) {
		_shift_CP = SH_CP_pin;
		_store_CP = ST_CP_pin;
		_data_DS = data_pin;
		_port = portReg;
		// Set pins as outputs
		*dirReg = *dirReg | (1 << _shift_CP) | (1 << _store_CP) | (1 << _data_DS);
		// set initial states for clock pins
		this->_setPin(_shift_CP, LOW);	// start LOW ready for shifting
		this->_setPin(_store_CP, HIGH);	// start HIGH ready for latching
		this->_shiftOut(0);
	}

	void setAllHigh(void)
	{
		_shiftOut(255);
	}

	void setAllLow (void)
	{
		_shiftOut(0);
	}

	void shiftOut(uint8_t byteVal, uint8_t bitOrder = MSBFIRST, bool invert = false)
	{
		// Synonym for shiftOutWithLatching. This is probably the most commonly used method.
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

	uint8_t _shift_CP;			// shift clock pin - SH_CP
	uint8_t _store_CP;			// store (latch) clock pin - ST_CP
	uint8_t _data_DS;			// data pin - DS or SER
	volatile uint8_t * _port;	// port

	void _setPin(uint8_t pin, uint8_t hilo)
	{
		// example call: writeBit(&PORTB, PB1, HIGH);
		if(hilo == HIGH) {
			*_port |= (1 << pin);
		} else {
			*_port &= ~(1 << pin);
		}
	}

	void _shiftOut(uint8_t byteVal, uint8_t bitOrder = MSBFIRST, bool invert = false, bool latching = true)
	{
		if (invert) {
			byteVal = 0xFF ^ byteVal;
		}
		if (latching) _setPin(_store_CP, LOW);		// store bits before making available on output
		for(uint8_t bit=0; bit<8; bit++) {			// Clock in data
			// the results of the following operations need to be a 1 or 0
			if (bitOrder == LSBFIRST) {
				_setPin(_data_DS, (byteVal & (1 << bit)) >> bit);
			} else {
				_setPin(_data_DS, (byteVal & (1 << (7 - bit))) >> (7 - bit) );
			}
			_setPin(_shift_CP, HIGH);		// take shift clock high to clock in bit
			_setPin(_shift_CP, LOW);			
		}
		_setPin(_store_CP, HIGH);			// bring storage clock pulse high
	}

};

#endif
