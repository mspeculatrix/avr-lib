/*
	Class for DS3107 real-time clock (RTC) chip.
	I2C device with optional square-wave output and 59 bytes
	of battery-backed RAM.

	This device has a fixed bus address of 0x68, hence the
	initialisation list used in the constructor.
*/

#include <avr/io.h>
#include "smd_avr_i2clib.h"

#ifndef __SMD_AVR_DS3107_H__
#define __SMD_AVR_DS3107_H__

#define DS3107_HOUR_MODE_BIT 6	// bit 6 of reg 2
#define DS3107_24HR_MODE 0
#define DS3107_12HR_MODE 1
#define DS3107_REG_SECS 0
#define DS3107_REG_MINS 1
#define DS3107_REG_HOUR 2
#define DS3107_REG_DAY 3
#define DS3107_REG_DATE 4
#define DS3107_REG_MNTH 5
#define DS3107_REG_YEAR 6

#define DS3107_AM 0
#define DS3107_PM 1

#define DS3107_RAM_START_ADDR 0x08
#define DS3107_RAM_END_ADDR 0x3F

#define DS3107_ERROR_PARAM_VALUE 5
#define DS3107_ERROR_MEMORY_ADDR 10

class SMD_AVR_DS3107 : public SMD_I2C_Device
{
	public:
		// PROPERTIES
		uint8_t hour_mode;
		bool output_sqwe;
		uint8_t ampm;		// AM or PM
		// METHODS
		SMD_AVR_DS3107(void);	// constructor

		uint8_t getMinsOrSecs(uint8_t minsOrSecsReg);
		uint8_t getHour(void);

		uint8_t setMinsOrSecs(uint8_t minsOrSecsReg, uint8_t value);
		uint8_t setHour(uint8_t hour, uint8_t hourmode);

		void startClock(void);
		void stopClock(void);

		uint8_t writeByteToRAM(uint8_t byte, uint8_t location);
		uint8_t readByteFromRAM(uint8_t location);

	protected:
		// PROPERTIES
		uint8_t _bus_address;
		uint8_t _error_code;
		// METHODS
		void _configure(void);			// main initialisation method, called by constructors
};





#endif
