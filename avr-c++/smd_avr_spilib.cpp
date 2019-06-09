/* smd_avr_spilib */

#include "smd_avr_spilib.h"
#if defined (__AVR_ATmega328P__)
#include <uproc/smd_atmega_88_168_328.h>
#elif defined (__AVR_ATmega168P__)
#include <uproc/smd_atmega_88_168_328.h>
#elif defined (__AVR_ATmega88P__)
#include <uproc/smd_atmega_88_168_328.h>
#elif defined (__AVR_ATtiny24A__)
#include <uproc/smd_attiny_24_44_84A.h>
#elif defined (__AVR_ATtiny44A__)
#include <uproc/smd_attiny_24_44_84A.h>
#elif defined (__AVR_ATtiny84A__)
#include <uproc/smd_attiny_24_44_84A.h>
#endif

SPI_Device::SPI_Device()
{
	// Configure with default SS settings.
	_init(&SPI_SS_PORT, SPI_SS_GPIO, SPI_SS_DDR);
}

SPI_Device::SPI_Device(volatile uint8_t * port, uint8_t gpio, uint8_t ddir)
{
	_init(port, gpio, ddir);
	//ssPort = port;
	//ssGpio = gpio;
	//ssDdir = ddir;
	//ssDdir |= (1 << gpio);	// set as output
}

void SPI_Device::_init(volatile uint8_t * port, uint8_t gpio, uint8_t ddir)
{
	ssPort = port;
	ssGpio = gpio;
	ssDdir = ddir;
	ssDdir |= (1 << gpio);	// set as output
}

void SPI_Device::enable(void)
{
	*ssPort &= ~(1 << ssGpio);	// take slave select pin low to enable device
}

void SPI_Device::disable(void)
{
	*ssPort |= (1 << ssGpio);	// take slave select pin high to disable device
}

//******************************************************************************************
//*****  BASE                                                                            ***
//******************************************************************************************

SMD_AVR_SPI_Base::SMD_AVR_SPI_Base(void)
{
	_byte_order = SPI_MSB_FIRST;		// default
}

/** This is the base method for sending and receiving data.
	It sends one byte of data and returns the byte received in
	exchange. **/
uint8_t SMD_AVR_SPI_Base::exchangeByte(uint8_t byte)
{
	SPDR = byte;	// automatically initiates sending of data
	while(!(SPSR & (1 << SPIF)));	// wait for SPIF bit to be set
	// returned data is now in SPDR
	return SPDR;
}

/** sendByte is for when you only want to send and don't care
	about what is returned. **/
void SMD_AVR_SPI_Base::sendByte(uint8_t byte)
{
	SPDR = byte;
	while(!(SPSR & (1 << SPIF)));
}

/** Prod slave device into sending data by transmitting dummy data **/
uint8_t SMD_AVR_SPI_Base::getByte(void)
{
	this->sendByte(0xFF);
	//return exchangeByte(0);
	uint8_t result = SPDR;
	return result;
}

uint16_t SMD_AVR_SPI_Base::getWord(void)
{
	uint16_t returnVal = 0;
	if (_byte_order == SPI_MSB_FIRST) {
		returnVal |= getByte() << 8;
		returnVal |= getByte();
	} else {
		returnVal |= getByte();
		returnVal |= getByte() << 8;
	}
	return returnVal;
}

/** Send 16 bits without expecting anything back **/
void SMD_AVR_SPI_Base::sendWord(uint16_t word)
{
	if (_byte_order == SPI_MSB_FIRST) {
		sendByte((uint8_t)(word >> 8));
		sendByte((uint8_t) word);
	} else {
		sendByte((uint8_t) word);
		sendByte((uint8_t)(word >> 8));
	}
}

void SMD_AVR_SPI_Base::setMSBfirst (void)
{
	SPSR &= ~(1 << DORD);						// MSB first is the default
	_byte_order = SPI_MSB_FIRST;
}

void SMD_AVR_SPI_Base::setLSBfirst (void)
{
	SPSR |= (1 << DORD);
	_byte_order = SPI_LSB_FIRST;
}


//******************************************************************************************
//*****  MASTER                                                                          ***
//******************************************************************************************

//------------------------------------------------------------------------------
//-----  CONSTRUCTORS                                                      -----
//------------------------------------------------------------------------------
SMD_AVR_SPI_Master::SMD_AVR_SPI_Master (uint8_t clk_rate)
{	// Simple constructor that defaults to Master mode and comm mode 0,0
	_init(SPI_DATA_MODE_0, clk_rate);
}

SMD_AVR_SPI_Master::SMD_AVR_SPI_Master (SPI_DATA_MODE data_mode, uint8_t clk_rate)
{
	_init(data_mode, clk_rate);
}

void SMD_AVR_SPI_Master::_init (SPI_DATA_MODE data_mode, uint8_t clk_rate)
{
	SPCR = 0;	// defaults to this value anyway, but let's reset to be sure
	// Set whether master or slave
	SPCR |= (1 << MSTR);				// set as master
	// Clock rate is revelant only in master mode
	if (clk_rate >= FOSC2x2) {			// values above 3 are for double clock freq
		clk_rate = clk_rate - FOSC2x2;	// reduce clk_rate so it can be ORed with SPCR later
		SPSR |= (1 << SPI2X);			// set SPI2X bit
	}
	SPCR |= clk_rate;					// set clock rate prescaler
	//SPI_MISO_DDR &= ~(1 << SPI_MISO_GPIO);	// MISO as input - this might be automatic
	SPI_MISO_PORT |= (1 << SPI_MISO_GPIO);	// set pull-up on MISO
	SPI_MOSI_DDR |= (1 << SPI_MOSI_GPIO);	// MOSI as output
	SPI_SCK_DDR |= (1 << SPI_SCK_GPIO);		// SCK as output
	SPI_SS_DDR |= (1 << SPI_SS_GPIO);		// slave select as output
	SPI_SS_PORT |= (1 << SPI_SS_GPIO);		// start as NOT selected (high)
	// Set data mode to take care of polarity and phase.
	SPCR |= data_mode;
	SPCR |= (1 << SPE);						// enable SPI
}

//------------------------------------------------------------------------------
//-----  PUBLIC METHODS                                                    -----
//------------------------------------------------------------------------------

//uint8_t SMD_AVR_SPI_Master::getByte(SPI_Device_Control device)
//{
	//return 0;
//}
//uint16_t SMD_AVR_SPI_Master::getWord(SPI_Device_Control device)
//{
	//return 0;
//}
//void SMD_AVR_SPI_Master::sendByte(int8_t byte, SPI_Device_Control device)
//{
	//device.enable();	// take SS pin low
	//SPDR = byte;
	//while(!(SPSR & (1 << SPIF)));
	//device.disable();	// take SS pin high again
//}
//void SMD_AVR_SPI_Master::sendWord(int16_t word, SPI_Device_Control device)
//{
//
//}

//*******************************************************************************************************
//*****  SLAVE                                                                                        ***
//*******************************************************************************************************

SMD_AVR_SPI_Slave::SMD_AVR_SPI_Slave()
{
	_init(SPI_DATA_MODE_0);
}

SMD_AVR_SPI_Slave::SMD_AVR_SPI_Slave(SPI_DATA_MODE data_mode)
{
	_init(data_mode);
}

void SMD_AVR_SPI_Slave::_init (SPI_DATA_MODE data_mode)
{
	SPCR = 0;	// defaults to this value anyway, but let's reset to be sure
	SPI_MISO_DDR |= (1 << SPI_MISO_GPIO);	// MISO as output
	SPI_MOSI_DDR &= ~(1 << SPI_MOSI_GPIO);	// MOSI as input - may be automatic
	SPI_MOSI_PORT |= (1 << SPI_MOSI_GPIO);	// set pull-up - REQUIRED?????
	SPI_SCK_DDR &= ~(1 << SPI_SCK_GPIO);	// SCK as input - may be automatic
	SPI_SS_DDR &= ~(1 << SPI_SS_GPIO);		// slave select as input - may be automatic - needs pull-up???
	SPI_SS_PORT |= (1 << SPI_SS_GPIO);		// set pull-up - REQUIRED?????
	// Set data mode to take care of polarity and phase.
	SPCR |= data_mode;
	SPCR |= (1 << SPE);						// enable SPI
}
