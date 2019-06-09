/*
	Macros for ATMEGA 88-168-328 family

	https://mansfield-devine.com/speculatrix

*/
#ifndef __SMD_ATMEGA_88_168_328_H__
#define __SMD_ATMEGA_88_168_328_H__

//namespace smd_avr {

// ----- SPI COMMS ----------------

#define SPI_SS_GPIO         PB2
#define SPI_SS_PORT         PORTB
#define SPI_SS_PIN			PINB
#define SPI_SS_DDR          DDRB

#define SPI_MOSI_GPIO       PB3
#define SPI_MOSI_PORT       PORTB
#define SPI_MOSI_PIN	    PINB
#define SPI_MOSI_DDR        DDRB

#define SPI_MISO_GPIO       PB4
#define SPI_MISO_PORT       PORTB
#define SPI_MISO_PIN        PINB
#define SPI_MISO_DDR        DDRB

#define SPI_SCK_GPIO        PB5
#define SPI_SCK_PORT        PORTB
#define SPI_SCK_PIN         PINB
#define SPI_SCK_DDR         DDRB

// ----- I2C COMMS ----------------
#define I2C_PORT	        PORTC
#define I2C_PIN		        PINC
#define I2C_DDR		        DDRC
#define I2C_SDA_GPIO        PC4
#define I2C_SCL_GPIO        PC5

// ----- SERIAL COMMS -------------
#define USART_TX_GPIO       PD1
#define USART_TX_PORT       PORTD
#define USART_TX_DDR        DDRD
#define USART_RX_GPIO       PORTD0
#define USART_RX_PIN        PIND
#define USART_RX_DDR        DDRD

//};	// namespace smd_avr

#endif
