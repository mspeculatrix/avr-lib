/*
	smd_avr_i2clib

	*** NB: MAY WANT TO REVISIT THIS ***
	Maybe the I2C device class idea isn't that great. For example, it
	sets the bus in a specific mode, which presumably then has to be
	shared by all I2C devices connected to this master.
	We already have functions, so maybe the 'device' should be
	little more than a union to contain various parameters for the
	device - eg, address, error state. Anyway, something to review.


	*** IMPORTANT ***
	The program using this library should include the relevant uproc file from my library
	eg:
	#include <uproc/smd_atmega_88_168_328.h>
	(actually, not sure this is necessary...)

	This is only for microcontrollers that have proper I2C interfaces - don't think it
	will work for simpler devices with USI.

	USAGE:
	#include <smd_avr_i2clib.h>
	SMD_I2C_Device dev = SMD_I2C_Device(0x68);

	STATUS CODES - TWSR
	Master mode - transmitter
	0x08 - start condition set
	0x18 - SLA+W sent & ACK received
	0x20 - SLA+W sent & NOT ACK received
	0x28 - Data byte sent - ACK received
	0x30 - data byte sent - NOT ACK received
	0x38 - Arbitration lost
*/

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

#ifndef __SMD_AVR_I2CLIB_H__
#define __SMD_AVR_I2CLIB_H__

#if defined(__AVR_ATmega328P__)
	#define TWBITRATE_REG TWBR
	#define TWDATA_REG TWDR
	#define TWCONTROL_REG TWCR
	#define TWSTAT_REG TWSR
#elif defined(__AVR_ATmega328PB__)
	#define TWBITRATE_REG TWBR0
	#define TWDATA_REG TWDR0
	#define TWCONTROL_REG TWCR0
	#define TWSTAT_REG TWSR0
#else
	#define TWBITRATE_REG TWBR
	#define TWDATA_REG TWDR
	#define TWCONTROL_REG TWCR
	#define TWSTAT_REG TWSR
#endif

#define I2C_READ_MODE  1
#define I2C_WRITE_MODE 0

// CONDITIONS/ERRORS IN MASTER TRANSMITTER MODE
#define I2C_TX_STARTED		0x08	// start condition has been transmitted
#define I2C_TX_SLAW_ACK		0x10	// SLA+W transmitted, ACK received
#define I2C_TX_SLAW_NACK	0x20	// SLA+W transmitted, NOT ACK received
#define I2C_TX_DATA_ACK		0x28	// data byte transmitted, ACK received
#define I2C_TX_DATA_NACK	0x30	// data byte transmitted, NOT ACK received
#define I2C_TX_ARB_LOST		0x38	// arbitration lost

// CONDITIONS/ERRORS IN MASTER RECEIVER MODE
#define I2C_RX_STARTED		0x08	// start condition has been transmitted
#define I2C_RX_RESTARTED	0x10	// repeated start condition has been transmitted
#define I2C_RX_ARB_LOST		0x38	// arbitration lost
#define I2C_RX_SLAR_ACK		0x40	// SLA+R transmitted, ACK received
#define I2C_RX_SLAR_NACK	0x48	// SLA+R transmitted, NOT ACK received
#define I2C_RX_DATA_ACK		0x50	// data byte has been received, ACK returned
#define I2C_RX_DATA_NACK	0x58	// data byte has been received, NOT ACK returned

#define I2C_BUS_SPEED_STD  100000UL		// 100KHz
#define I2C_BUS_SPEED_FAST 400000UL		// 400KHz - this library can't cope with the even faster standards

#define I2C_BYTE_ORDER_LSB_FIRST 0
#define I2C_BYTE_ORDER_MSB_FIRST 1

#define I2C_STATUS_MASK 0b11111100

#define I2C_TWINT_NOT_SET 0
#define I2C_TWINT_SET 1
#define I2C_TIMEOUT_LOOP_DELAY 1	// us
#define I2C_TIMEOUT_LOOP_MAX 10	// number of times aorund the loop

#define I2C_ERR_NONE 0
#define I2C_ERR_START_FAILED 0x01	// error codes should always be odd so
#define I2C_ERR_NO_ACK_RECVD 0x03	// there's always a 1 in the LSB

#define I2C_ERR_UNKNOWN_BYTE_ORDER 0x11

#define I2C_wait_for_completion while(!(TWCONTROL_REG & (1 << TWINT)))

namespace smd_avr_i2c {

void I2C_initialise_bus();
void I2C_initialise_bus(unsigned long busSpeed, uint8_t prescaler_mode);
//uint8_t checkAlive();
uint8_t I2C_readNoAck(void);
uint8_t I2C_readWithAck(void);
uint8_t I2C_sendByte(uint8_t byteData);
uint8_t I2C_start(void);							// set start condition without a specific mode
uint8_t I2C_start(uint8_t address, uint8_t mode);	// set start condition with address and mode
inline void I2C_stop(void) { TWCONTROL_REG = ((1 << TWINT) | (1 << TWEN) | (1 << TWSTO)); }
//inline void I2C_waitForComplete(void) {	while(!(TWCR & (1 << TWINT))); }

} // namespace

class SMD_I2C_Device
{
/* Class to talk to an I2C slave device */

public:
	SMD_I2C_Device(uint8_t address);
	SMD_I2C_Device(uint8_t address, unsigned long busSpeed);
	SMD_I2C_Device(uint8_t address, unsigned long busSpeed, uint8_t prescaler_mode);
	//~SMD_I2C_Device();

	void enableI2Cpullups(volatile uint8_t * i2cPort, uint8_t sdaGpio, uint8_t sclGpio);

	uint8_t readRegister(uint8_t regAddr);

	uint8_t start(void);
	uint8_t start(uint8_t mode);
	uint8_t checkAlive(void);
	//void setMode(uint8_t mode);

	uint8_t readWithAck(void);
	uint8_t readNoAck(void);
	uint16_t readWord(void);
	uint16_t readWord(uint8_t byteOrder);

	void sendByte(uint8_t byteData);
	uint8_t sendWord(uint16_t wordData);
	uint8_t sendWord(uint16_t wordData, uint8_t byteOrder);

	void setAddress(uint8_t address);
	uint8_t getAddress();
	void setByteOrder(uint8_t byteOrder);
	void stop(void);
	void writeRegister(uint8_t register_addr, uint8_t value);

protected:
	uint8_t _address;
	uint8_t _byteOrder;

	void _init(uint8_t address, unsigned long busSpeed, uint8_t prescaler_mode);
	void _waitForComplete(void);		// blocking but fast

};

#endif
