/* smd_avr_23lcv512.cpp */

#include "smd_avr_23lcv512.h"

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

// CONSTRUCTORS

SMD_AVR_23lcv512::SMD_AVR_23lcv512()
{
	this->_ram = SPI_Device();	// configure with default SS setting for AVR
	this->_init((uint8_t)FOSC64);
}

SMD_AVR_23lcv512::SMD_AVR_23lcv512(uint8_t clk_rate)
{
	this->_ram = SPI_Device();	// configure with default SS setting for AVR
	this->_init(clk_rate);
}

SMD_AVR_23lcv512::SMD_AVR_23lcv512(volatile uint8_t *ss_port, uint8_t ss_gpio, uint8_t ss_ddir, uint8_t clk_rate)
{
	this->_ram = SPI_Device(ss_port, ss_gpio, ss_ddir);
	this->_init(clk_rate);
}

void SMD_AVR_23lcv512::_init(uint8_t clk_rate)
{
	SPCR = 0;							// defaults to 0 anyway, but reset to be sure
	SPCR |= (1 << MSTR);				// set as master
	if (clk_rate >= FOSC2x2) {			// values above 3 are for double clock freq
		clk_rate = clk_rate - FOSC2x2;	// reduce clk_rate so it can be ORed with SPCR later
		SPSR |= (1 << SPI2X);			// set SPI2X bit
	}
	SPCR |= clk_rate;							// set clock rate prescaler
	//SPI_MISO_DDR &= ~(1 << SPI_MISO_GPIO);	// MISO as input - this might be automatic
	SPI_MISO_PORT |= (1 << SPI_MISO_GPIO);		// set pull-up on MISO
	SPI_MOSI_DDR  |= (1 << SPI_MOSI_GPIO);		// MOSI as output
	SPI_SCK_DDR   |= (1 << SPI_SCK_GPIO);		// SCK as output
	SPI_SS_DDR    |= (1 << SPI_SS_GPIO);		// slave select as output
	SPI_SS_PORT   |= (1 << SPI_SS_GPIO);		// start as NOT selected (high)
	SPCR          |= (1 << SPE);				// enable SPI

	for (uint8_t i=0; i < BUFFER_SIZE; i++) {
		this->_rw_buffer[i] = 0;		// initialise buffer and ensure it's empty
	}
	current_addr = 0x0000;
	// set up write mode
	this->_ram.enable();
	this->sendByte((uint8_t)RAM_WRMR);
	this->sendByte((uint8_t)RAM_SEQ_MODE);
	// set up read mode
	this->sendByte((uint8_t)RAM_RDMR);
	this->sendByte((uint8_t)RAM_SEQ_MODE);
	this->_ram.disable();
}

/************************************************************************
 *    WRITE METHODS                                                     *
 ************************************************************************/
void SMD_AVR_23lcv512::writeByteAtAddress(uint8_t byte, uint16_t addr)
{
	this->_openForReadWriteAtAddress(addr, (uint8_t)RAM_WRITE_CMD);
	this->sendByte(byte);
	this->_ram.disable();
}

void SMD_AVR_23lcv512::writeWordAtAddress(uint16_t word, uint16_t addr)
{
	this->_openForReadWriteAtAddress(addr, (uint8_t)RAM_WRITE_CMD);
	this->sendWord(word);
	this->_ram.disable();
}

void SMD_AVR_23lcv512::openForWriteAtAddress(uint16_t addr)
{
	_openForReadWriteAtAddress(addr, (uint8_t)RAM_WRITE_CMD);
}

/************************************************************************
 *    READ METHODS                                                      *
 ************************************************************************/
uint8_t SMD_AVR_23lcv512::readByteAtAddress(uint16_t addr)
{
	this->_openForReadWriteAtAddress(addr, (uint8_t)RAM_READ_CMD);
	uint8_t result = this->getByte();
	this->_ram.disable();
	return result;
}

uint16_t SMD_AVR_23lcv512::readWordAtAddress(uint16_t addr)
{
	this->_openForReadWriteAtAddress(addr, (uint8_t)RAM_READ_CMD);
	uint16_t result = this->getWord();
	this->_ram.disable();
	return result;
}

void SMD_AVR_23lcv512::openForReadAtAddress(uint16_t addr)
{
	_openForReadWriteAtAddress(addr, (uint8_t)RAM_READ_CMD);
}



void SMD_AVR_23lcv512::close(void)
{
	this->_ram.disable();
}

/************************************************************************
 *    PROTECTED METHODS                                                 *
 ************************************************************************/

void SMD_AVR_23lcv512::_openForReadWriteAtAddress(uint16_t addr, uint8_t rwMode)
{
	current_addr = addr;
	this->_ram.enable();
	this->sendByte(rwMode);
	this->sendWord(addr);
}
