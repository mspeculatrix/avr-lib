/*
 * clsShiftReg74hc165.cpp
 * Class for the 75HC165 parallel-in, serial-out shift register.

 * Created: 11/21/2017 10:21:45 AM
 * Author : Steve
 */
#ifndef __SHIFTREG_74HC165__
#define __SHIFTREG_74HC165__

#include <avr/io.h>

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

class ShiftReg74hc165
{
	public:
		uint8_t shift_LD;	// shift/load pin - SH_LD
		uint8_t shift_CLK;	// clock pin
		uint8_t clock_INH;	// clock inhibit pin
		uint8_t data_Q;		// data pin
		volatile uint8_t * port;		// port - output pin register
		volatile uint8_t * inReg;		// input pin register

		ShiftReg74hc165(uint8_t SH_LD_pin, uint8_t CLK_pin, uint8_t CLK_INH_pin, uint8_t data_Q_pin,
								volatile uint8_t * portReg, volatile uint8_t * pinReg, volatile uint8_t * dirReg) {
			shift_LD = SH_LD_pin;
			shift_CLK = CLK_pin;
			data_Q = data_Q_pin;
			clock_INH = CLK_INH_pin;
			port = portReg;
			inReg = pinReg;
			// set output pins
			*dirReg = *dirReg | (1 << shift_LD) | (1 << shift_CLK) | (1 << clock_INH);
			// set input pin
			*dirReg = *dirReg & ~(1 << data_Q);
			// set initial states for output pins
			_setPin(clock_INH, HIGH);
			_setPin(shift_CLK, HIGH);
			_setPin(shift_LD, LOW);
		}

		//uint8_t clockIn( uint8_t bitOrder = MSBFIRST)
		//{
			//// This assumes you only want an eight-bit value
			//uint16_t value = read8(bitOrder);
			//return (uint8_t)value;
		//}

		uint16_t read16( uint8_t bitOrder = MSBFIRST)
		{
			return _clockIn(16, bitOrder);
		}

		uint8_t read8( uint8_t bitOrder = MSBFIRST)
		{
			return (uint8_t)_clockIn(8, bitOrder);
		}

	protected:

		void _setPin(uint8_t pin, uint8_t hilo)
		{
			if(hilo == LOW) {
				*port &= ~(1 << pin);
			} else {
				*port |= (1 << pin);
			}
		}

		uint16_t _clockIn(uint8_t numbits, uint8_t bitOrder = MSBFIRST)
		{
			uint16_t regVal = 0;
			_setPin(shift_LD, HIGH);
			_setPin(clock_INH, LOW);
			for (int8_t b = 0; b < numbits; b++) {		// shift bits in MSB first
				_setPin(shift_CLK, LOW);
				if((*inReg & (1 << data_Q)) == (1 << data_Q)) { // pin is high
					if (bitOrder == MSBFIRST) {
						regVal = regVal | (1 << (numbits - b - 1));
					} else {
						regVal = regVal | (1 << b);
					}
				}
				_setPin(shift_CLK, HIGH);			// toggle clock to shift in next bit
				_setPin(shift_CLK, LOW);// this line should not be needed, but is here to avoid weird compiler bug
			}
			_setPin(shift_LD, LOW);		// return these to default settings
			_setPin(shift_CLK, HIGH);	// This is needed only if we have the weird, unnecessary line above
			_setPin(clock_INH, HIGH);
			return regVal;
		}

};

#endif
