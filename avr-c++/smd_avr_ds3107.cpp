
#include "smd_avr_ds3107.h"

/************************************************************************/
/* PUBLIC METHODS                                                       */
/************************************************************************/
SMD_AVR_DS3107::SMD_AVR_DS3107(void) : SMD_I2C_Device(0x68)
{
	_configure();
}
/* ----- TIME GETTER METHODS -------------------------------------------------*/

uint8_t SMD_AVR_DS3107::getMinsOrSecs(uint8_t minsOrSecsReg)
{
	uint8_t reg_value = this->readRegister(minsOrSecsReg);
	uint8_t tens_val = (reg_value & 0b01110000) >> 4;
	uint8_t units_val = reg_value & 0b00001111;
	return (tens_val * 10) + units_val;
}

uint8_t SMD_AVR_DS3107::getHour(void)
{
	uint8_t reg_value = this->readRegister(DS3107_REG_HOUR);
	// check if we're in 12-hour or 24-hour mode
	this->hour_mode = (reg_value & 0b01000000) >> 6;	// reset property while we're at it

	uint8_t hour_mask = 0b00110000;	// for 24-hour mode
	if (this->hour_mode == DS3107_12HR_MODE) {
		hour_mask = 0b00010000;
	}
	uint8_t tens_val = (reg_value & hour_mask) >> 4;
	uint8_t units_val = reg_value & 0b00001111;
	return (tens_val * 10) + units_val;
}


/* ----- TIME SETTER METHODS -------------------------------------------------*/

uint8_t SMD_AVR_DS3107::setMinsOrSecs(uint8_t minsOrSecsReg, uint8_t value)
{
	this->_error_code = 0;
	if(value > 59) {
		this->_error_code = DS3107_ERROR_PARAM_VALUE;
	} else {
		uint8_t tens_val = (value / 10);
		uint8_t units_val = value % 10;
		this->writeRegister(minsOrSecsReg, (tens_val << 4) + units_val);
	}
	return this->_error_code;
}

/* ----- GENERAL CONTROL METHODS ---------------------------------------------*/

void SMD_AVR_DS3107::startClock(void)
{
	// read the seconds register
	uint8_t secs_reg = this->readRegister(DS3107_REG_SECS);
	// set bit 7 to 0
	secs_reg &= ~(1 << 7);
	// write it back out
	this->writeRegister(DS3107_REG_SECS, secs_reg);
}


/* ----- RAM METHODS ---------------------------------------------------------*/
uint8_t SMD_AVR_DS3107::readByteFromRAM(uint8_t location)
{
	this->_error_code = 0;
	uint8_t byte_value = 0;
	if (location < DS3107_RAM_START_ADDR || location > DS3107_RAM_END_ADDR) {
		this->_error_code = DS3107_ERROR_MEMORY_ADDR;
	} else {
		byte_value = this->readRegister(location);
	}
	return byte_value;
}

uint8_t SMD_AVR_DS3107::writeByteToRAM(uint8_t byte, uint8_t location)
{
	this->_error_code = 0;
	if (location < DS3107_RAM_START_ADDR || location > DS3107_RAM_END_ADDR) {
		this->_error_code = DS3107_ERROR_MEMORY_ADDR;
	} else {
		this->writeRegister(location, byte);
	}
	return this->_error_code;
}

/************************************************************************/
/* PROTECTED METHODS                                                    */
/************************************************************************/
void SMD_AVR_DS3107::_configure(void)
{
	//_bus_address = 0x68;	// address is fixed for this device
	hour_mode = DS3107_24HR_MODE;
	output_sqwe = false;
	_error_code = 0;
}
