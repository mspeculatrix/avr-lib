/*
	smd_avr_23lcv512

	Class for the 23lcv612 64KB serial RAM chip.
	This uses an SPI interface, so this class is derived from the classes in smd_avr_spilib

	***** NOT FINISHED *****
*/

#include <avr/io.h>
#include "smd_avr_spilib.h"

#define BUFFER_SIZE 32

#define RAM_READ_CMD  0x03
#define RAM_WRITE_CMD 0x02
#define RAM_RDMR	  0x05
#define RAM_WRMR	  0x01
#define RAM_PAGE_MODE 0b10000000
#define RAM_SEQ_MODE  0b01000000


class SMD_AVR_23lcv512 : public SMD_AVR_SPI_Base
{
	public:
		// Constructors
		SMD_AVR_23lcv512();						// using default SPI pins & clock rate of Fosc/64
		SMD_AVR_23lcv512(uint8_t clk_rate);		// using default SPI pins
		SMD_AVR_23lcv512(volatile uint8_t *ss_port, uint8_t ss_gpio, uint8_t ss_ddir, uint8_t clk_rate);

		//enum RAM_READWRITE_MODES { RAM_READ, RAM_WRITE };

		void writeByteAtAddress(uint8_t byte, uint16_t addr);		// write one byte at address - working
		void writeWordAtAddress(uint16_t word, uint16_t addr);		// write two-byte word at address - working
		void writePageAtAddress(uint8_t bytes[], uint16_t addr);
		uint8_t readByteAtAddress(uint16_t addr);					// read one byte at address - working
		uint16_t readWordAtAddress(uint16_t addr);					// read two-byte word at address - working
		uint8_t* readPageAtAddress(uint16_t addr);

		void openForWriteAtAddress(uint16_t addr);
		void openForReadAtAddress(uint16_t addr);
		void close(void);
		void writeNextByte(uint8_t byte);
		void writeNextWord(uint16_t word);
		uint8_t getNextByte();
		uint16_t getNextWord();

	protected:
		SPI_Device _ram;
		uint8_t _rw_buffer[BUFFER_SIZE];
		uint8_t *bufPtr;
		uint16_t current_addr;

		void _init(uint8_t clk_rate);
		void _openForReadWriteAtAddress(uint16_t addr, uint8_t rwMode);	// working
};
