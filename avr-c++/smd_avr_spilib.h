/* smd_avr_spilib

*** IMPORTANT ***
The program using this library should include the relevant uproc file from my library
eg:
#include <uproc/smd_atmega_88_168_328.h>

*/

#ifndef __SMD_AVR_SPILIB_H__
#define __SMD_AVR_SPILIB_H__

#include <avr/io.h>
//#include <uproc/smd_atmega_88_168_328.h>

enum SPI_DEVICE_MODE { SPI_DEVICE_MODE_SLAVE, SPI_DEVICE_MODE_MASTER };

enum SPI_BYTE_ORDER { SPI_MSB_FIRST, SPI_LSB_FIRST };

enum SPI_DATA_MODE {	// The values are those that should be ORed with SPCR
	// the two bits of SPCR should be zeroed before starting. Maybe all of SPCR
	// should be zeroed before starting.
	SPI_DATA_MODE_0 = 0,
	SPI_DATA_MODE_1 = 4,
	SPI_DATA_MODE_2 = 8,
	SPI_DATA_MODE_3 = 12 };

enum SPI_CLK_RATE {		// The values are those that should be ORed with SPCR
	FOSC4		= 0,
	FOSC16		= 1,
	FOSC64		= 2,
	FOSC128		= 3,
	FOSC2x2		= 4,	// With the x2 values, you also need to set bit 0
	FOSC8x2		= 5,	// in the SPSR register. Subtract 4 from these
	FOSC32x2	= 6,	// values before ORing with SPCR
	FOSC64x2	= 7
	};

/************************************************************************
 * SPI_Device_Control                                                   *
 * Simple class to manage each device on the bus. All devices can use   *
 * the same master but this class allows us to create separate objects  *
 * each with their own Slave Select (SS) pin.                           *
 ************************************************************************/
class SPI_Device {
	public:
		uint8_t ssGpio;
		volatile uint8_t *ssPort;
		uint8_t ssDdir;

		SPI_Device();
		SPI_Device(volatile uint8_t *port, uint8_t gpio, uint8_t ddir);
		void enable(void);
		void disable(void);
	
	protected:
		void _init(volatile uint8_t *port, uint8_t gpio, uint8_t ddir);
};

/************************************************************************
 * SMD_AVR_SPI_Base                                                     *
 * Base class to handle common elements of SMD_AVR_SPI_Master and       *
 * SMD_AVR_SPI_Slave classes                                            *
 ************************************************************************/
class SMD_AVR_SPI_Base
{
	public:
		// constructor
		SMD_AVR_SPI_Base();
		// methods
		uint8_t exchangeByte(uint8_t byte);	// send & get one-byte values
		void sendByte(uint8_t byte);		// send one-byte value
		void sendWord(uint16_t word);		// send two-byte value
		uint8_t getByte(void);				// prompt device to send data - 1 byte
		uint16_t getWord(void);				//   "      "     "   "   "   - 2 bytes
		void setMSBfirst(void);				// default behaviour
		void setLSBfirst(void);

	protected:
		uint8_t _dataMode;
		SPI_BYTE_ORDER _byte_order;
};

/************************************************************************
 * SMD_AVR_SPI_Master                                                   *
 ************************************************************************/
class SMD_AVR_SPI_Master : public SMD_AVR_SPI_Base {
	public:
		// constructors
		SMD_AVR_SPI_Master(uint8_t clk_rate);
		SMD_AVR_SPI_Master(SPI_DATA_MODE data_mode, uint8_t clk_rate);

	protected:
		//uint8_t _deviceMode;
		void _init(SPI_DATA_MODE data_mode, uint8_t clk_rate);
};

/************************************************************************
 * SMD_AVR_SPI_Slave  ***NOT FINISHED***                                *
 ************************************************************************/
class SMD_AVR_SPI_Slave : public SMD_AVR_SPI_Base  {
	public:
		SMD_AVR_SPI_Slave();
		SMD_AVR_SPI_Slave(SPI_DATA_MODE data_mode);

	protected:
		void _init(SPI_DATA_MODE data_mode);
};

#endif
