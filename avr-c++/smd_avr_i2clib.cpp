#include "smd_avr_i2clib.h"
#include <compat/twi.h>

namespace smd_avr_i2c {

void I2C_initialise_bus()
{
	I2C_initialise_bus(I2C_BUS_SPEED_STD, 0);
}

void I2C_initialise_bus(unsigned long busSpeed, uint8_t prescaler_mode)
{
	if (busSpeed != I2C_BUS_SPEED_STD && busSpeed != I2C_BUS_SPEED_FAST) {
		// we've passed an illegal value
		busSpeed = I2C_BUS_SPEED_STD;	// default to this
	}
	// *** PRESCALER ***
	if(prescaler_mode > 3) {	// illegal value
		prescaler_mode = 0;		// will give a prescaler of 1, the normal default
	}
	// By default, at power-up, bits 0 & 1 (known as TWPS0 and TWPS1) of the
	// TWSTAT_REG status register are set to 0. This gives a default prescaler
	// value of 1 which is what we'll pretty much always want to use with
	// ATMEGAs.
	// Let's first reset the two bits to 0, just to be sure.
	TWSTAT_REG &= 0b11111100;		// don't affect the other (status) bits
	// now let's set the bits to any prescaler value passed.
	TWSTAT_REG |= prescaler_mode;	// leaves the status bits alone.
	// In the future, we might want to provide a parameter for selecting the
	// prescaler by passing a setting of 0-3.
	// Valid prescaler values are 1, 4, 16, 64. These are powers of 4 - ie, whatever
	// is now set in the TWSP0 and TWSP1 bits (values 0-3) can give us
	// the actual prescaler value we want for later calculations with: 4 ^ <the_bits>
	uint8_t prescaler_value = pow(4, (TWSTAT_REG & 0b00000011));

	/*
	The standard bit rate formula is:
		SCL_freq = F_CPU / (16 + (2 * TWBITRATE_REG * Prescaler) )
	Multiply both sides by (16 + (2 * TWBITRATE_REG * Prescaler)):
		SCLfreq * (16 + (2 * TWBITRATE_REG * Prescaler)) = F_CPU.
	Divide both sides by SCLfreq:
		16 + (2 * TWBITRATE_REG * Prescaler) = F_CPU / SCLfreq
	Subtract 16 both sides:
		2 * TWBITRATE_REG * Prescaler = (F_CPU / SCLfreq) - 16
	Divide by 2 * Prescaler both sides:
		TWBITRATE_REG = ((F_CPU / SCLfreq) - 16) / (2 * Prescaler)
	*/
	unsigned long baseFreq = F_CPU / busSpeed;
	TWBITRATE_REG = (uint8_t)((baseFreq - 16) / (2 * prescaler_value));
	//TWBITRATE_REG = (uint8_t)((baseFreq / prescaler_value) - 16) / 2;
	TWCONTROL_REG |= (1 << TWEN);	// set TWI enable bit in control register
}

uint8_t I2C_readWithAck(void)
{
	// Enable interrupt, enable TWI and enable acknowledge.
	// In Master mode, setting TWEA acknowledges the receipt
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA));
	I2C_wait_for_completion;
	// return the value in the data register
	return (TWDATA_REG);
}

uint8_t I2C_readNoAck(void)
{
	// enable interrupt and enable TWI
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN));
	I2C_wait_for_completion;
	// return the value in the data register
	return (TWDATA_REG);
}

uint8_t I2C_sendByte(uint8_t byteData)
{
	// put byte value into data register
	TWDATA_REG = byteData;
	// enable interrupt and enable TWI
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN));
	I2C_wait_for_completion;
	return (TWSTAT_REG);
}

uint8_t I2C_start(void)
{
	uint8_t status = 0;
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTA));
	I2C_wait_for_completion;
	if ((TWSTAT_REG & I2C_STATUS_MASK) != I2C_TX_STARTED) status = I2C_ERR_START_FAILED;
	return status;
}

uint8_t I2C_start(uint8_t address, uint8_t mode)
{
	uint8_t status = 0;
	// Sets the start condition, then loads the slave address with the read or write bit and sends that.
	// Write start condition: set (ie, clear) interrupt, set enable and set start condition
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTA));
	I2C_wait_for_completion;			// blocking!!
	if((TWSTAT_REG & I2C_STATUS_MASK) == I2C_TX_STARTED) {
		// (TWSTAT_REG & I2C_STATUS_MASK) at this point should be 0x08 - start condition transmitted
		// put address of slave into data register, plus read or write bit
		TWDATA_REG = address + mode;
		// enable interrupt and enable TWI
		TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN));
		I2C_wait_for_completion;
		if((TWSTAT_REG & I2C_STATUS_MASK) != I2C_TX_SLAW_ACK) status = I2C_ERR_NO_ACK_RECVD;
	} else {
		status = I2C_ERR_START_FAILED;
	}
	return status;
}


} // namespace







// ********************* SMD_I2C_Device CLASS **************************************


//------------------------------------------------------------------------------
//-----   CONSTRUCTORS & INITIALISATION METHODS                            -----
//------------------------------------------------------------------------------
/**
	Constructor using default I2C bus speed of 100KHz
	@param address			Hex address of slave device.
**/
SMD_I2C_Device::SMD_I2C_Device(uint8_t address)
{
	_init(address, I2C_BUS_SPEED_STD, 0);
}

/**
	Constructor using selectable I2C bus speeds of 100kHz or 400kHz
	@param address			Hex address of slave device.
	@param busSpeed I2C_BUS_SPEED_STD or I2C_BUS_SPEED_FAST
**/
SMD_I2C_Device::SMD_I2C_Device(uint8_t address, unsigned long busSpeed)
{
	_init(address, busSpeed, 0);
}

/**
	Constructor using selectable I2C bus speeds of 100kHz or 400kHz and selectable prescaler
	@param busSpeed			I2C_BUS_SPEED_STD or I2C_BUS_SPEED_FAST
	@param prescaler_mode	Values 0-3. Default is 0.
**/
SMD_I2C_Device::SMD_I2C_Device(uint8_t address, unsigned long busSpeed, uint8_t prescaler_mode)
{
	_init(address, busSpeed, prescaler_mode);
}

/**
	Protected initialiser, called by the constructors.
	@param address			Hex address of slave device.
	@param busSpeed			I2C_BUS_SPEED_STD or I2C_BUS_SPEED_FAST
	@param prescaler_mode	Values 0-3. Default is 0.
**/
void SMD_I2C_Device::_init(uint8_t address, unsigned long busSpeed, uint8_t prescaler_mode)
{
	if (busSpeed != I2C_BUS_SPEED_STD && busSpeed != I2C_BUS_SPEED_FAST) {
		// we've passed an illegal value
		busSpeed = I2C_BUS_SPEED_STD;	// default to this
	}
	// *** PRESCALER ***
	if(prescaler_mode > 3) {	// illegal value
		prescaler_mode = 0;		// will give a prescaler of 1, the normal default
	}
	// By default, at power-up, bits 0 & 1 (known as TWPS0 and TWPS1) of the
	// TWSTAT_REG status register are set to 0. This gives a default prescaler
	// value of 1 which is what we'll pretty much always want to use with
	// ATMEGAs.
	// Let's first reset the two bits to 0, just to be sure.
	TWSTAT_REG &= 0b11111100;		// don't affect the other (status) bits
	// now let's set the bits to any prescaler value passed.
	TWSTAT_REG |= prescaler_mode;	// leaves the status bits alone.
	// In the future, might want to provide parameter for selecting the
	// prescaler by passing a setting of 0-3.
	// Valid prescaler values are 1, 4, 16, 64. These are powers of 4 - ie, whatever
	// is now set in the TWSP0 and TWSP1 bits (values 0-3) can give us
	// the actual prescaler value we want for later calculations with: 4 ^ <the_bits>
	uint8_t prescaler_value = pow(4, (TWSTAT_REG & 0b00000011));

	/*
	The standard bit rate formula is:
		SCL_freq = F_CPU / (16 + (2 * TWBITRATE_REG * Prescaler) )
	Multiply both sides by (16 + (2 * TWBITRATE_REG * Prescaler)):
		SCLfreq * (16 + (2 * TWBITRATE_REG * Prescaler)) = F_CPU.
	Divide both sides by SCLfreq:
		16 + (2 * TWBITRATE_REG * Prescaler) = F_CPU / SCLfreq
	Subtract 16 both sides:
		2 * TWBITRATE_REG * Prescaler = (F_CPU / SCLfreq) - 16
	Divide by 2 * Prescaler both sides:
		TWBITRATE_REG = ((F_CPU / SCLfreq) - 16) / (2 * Prescaler)
	*/
	unsigned long baseFreq = F_CPU / busSpeed;
	TWBITRATE_REG = (uint8_t)((baseFreq - 16) / (2 * prescaler_value));
	//TWBITRATE_REG = (uint8_t)((baseFreq / prescaler_value) - 16) / 2;
	TWCONTROL_REG |= (1 << TWEN);	// set TWI enable bit in control register
	setAddress(address);
	_byteOrder = I2C_BYTE_ORDER_MSB_FIRST;		// default to this
	//_twintSet = false;
}

uint8_t SMD_I2C_Device::start(void)
{
	// Start without setting mode or sending address - NOT SURE THIS MAKES SENSE!!!
	uint8_t err = 0;
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTA));
	_waitForComplete(); // while(!(TWCONTROL_REG & (1 << TWINT)));
	return err;
}

uint8_t SMD_I2C_Device::start(uint8_t mode)
{
	// This version sets the start condition, then loads the slave address with the read or write bit and sends that.
	uint8_t err = 0;
	// Write start condition: set (ie, clear) interrupt, set enable and set start condition
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTA));
	_waitForComplete();
	if((TWSTAT_REG & I2C_STATUS_MASK) == I2C_TX_STARTED) {
		// (TWSTAT_REG & I2C_STATUS_MASK) at this point should be 0x08 - start condition transmitted
		// put address of slave into data register, plus read or write bit
		TWDATA_REG = _address + mode;
		// enable interrupt and enable TWI
		TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN));
		_waitForComplete();
		// check value of TWI Status Register. Mask prescaler bits.
		uint8_t twst = TWSTAT_REG & 0xF8;
		if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) err = I2C_ERR_NO_ACK_RECVD;
	} else {
		err = I2C_ERR_START_FAILED;
	}
	return err;
}

uint8_t SMD_I2C_Device::checkAlive(void)
{
	uint8_t err = this->start(I2C_WRITE_MODE);
	this->stop();
	return err;
}
//------------------------------------------------------------------------------
//-----   METHODS UNDER DEVELOPMENT                                        -----
//------------------------------------------------------------------------------

void SMD_I2C_Device::enableI2Cpullups(volatile uint8_t * i2cPort, uint8_t sdaGpio, uint8_t sclGpio)
{
	// Enable internal pullups. Might not always want to do this because sometimes
	// you might want to use external pullups.
	// The output register needs to be passed by reference.
	// Example usage (using macros defined in smd_atmega_88_168_328.h):
	//	device.enableI2Cpullups(&I2C_PORT, I2C_SDA_GPIO, I2C_SCL_GPIO);
	*i2cPort |= ((1 << sdaGpio) | (1 << sclGpio));
}

//------------------------------------------------------------------------------
//-----  PUBLIC METHODS                                                    -----
//------------------------------------------------------------------------------

uint8_t SMD_I2C_Device::readWithAck(void)
{
	// Enable interrupt, enable TWI and enable acknowledge.
	// In Master mode, setting TWEA acknowledges the receipt
	//uint8_t data = TWDATA_REG;
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA));
	_waitForComplete();
	// return the value in the data register
	return TWDATA_REG;
}

uint8_t SMD_I2C_Device::readNoAck(void)
{
	// enable interrupt and enable TWI
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN));
	_waitForComplete();
	// return the value in the data register
	return (TWDATA_REG);
}

uint8_t SMD_I2C_Device::readRegister(uint8_t regAddr)
{
	uint8_t regVal = 0;
	// interrogate a register that returns an 8-bit value
	this->start(I2C_WRITE_MODE);
	this->sendByte(regAddr);
	this->start(I2C_READ_MODE);
	regVal = readNoAck();
	this->stop();
	return regVal;
}

uint16_t SMD_I2C_Device::readWord(void)
{
	return readWord(_byteOrder);
}

uint16_t SMD_I2C_Device::readWord(uint8_t byteOrder)
{
	uint16_t inWord = 0;
	if(byteOrder == I2C_BYTE_ORDER_MSB_FIRST) {
		inWord = readWithAck() << 8;
		inWord += readNoAck();
	} else {
		inWord += readWithAck();
		inWord = readNoAck() << 8;
	}
	return inWord;
}

void SMD_I2C_Device::sendByte(uint8_t byteData)
{
	// put byte value into data register
	TWDATA_REG = byteData;
	// enable interrupt and enable TWI
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN));
	_waitForComplete();
	//
}

uint8_t SMD_I2C_Device::sendWord(uint16_t wordData)
{
	uint8_t error = sendWord(wordData, _byteOrder);	// use current setting for byte order
	return error;
}

uint8_t SMD_I2C_Device::sendWord(uint16_t wordData, uint8_t byteOrder)
{
	uint8_t error = I2C_ERR_NONE;
	if (byteOrder == I2C_BYTE_ORDER_MSB_FIRST) {
		sendByte((uint8_t)(wordData >> 8));
		sendByte((uint8_t)(wordData));
	} else if (byteOrder == I2C_BYTE_ORDER_LSB_FIRST) {	// be specific to only
		sendByte((uint8_t)(wordData));					// only two values in
		sendByte((uint8_t)(wordData >> 8));				// param
	} else {
		error = I2C_ERR_UNKNOWN_BYTE_ORDER;
	}
	return error;
}

void SMD_I2C_Device::setAddress(uint8_t address)
{
	_address = address;
}

uint8_t SMD_I2C_Device::getAddress(void)
{
	return _address;
}

void SMD_I2C_Device::setByteOrder(uint8_t byteOrder)
{
	if (byteOrder == I2C_BYTE_ORDER_LSB_FIRST || byteOrder == I2C_BYTE_ORDER_MSB_FIRST) {
		_byteOrder = byteOrder;
	}
}

void SMD_I2C_Device::stop(void)
{
	// Enable interrupt, enable TWI and enable stop condition
	TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTO));
}

void SMD_I2C_Device::writeRegister(uint8_t register_addr, uint8_t value)
{
	start(I2C_WRITE_MODE);
	sendByte(register_addr);
	sendByte(value);
	this->stop();
}

/*------------------------------------------------------------------------------
  -----   PROTECTED METHODS                                                -----
  ------------------------------------------------------------------------------*/

void SMD_I2C_Device::_waitForComplete(void)
{
	while(!(TWCONTROL_REG & (1 << TWINT)));	// wait for TWINT bit to be set
}
