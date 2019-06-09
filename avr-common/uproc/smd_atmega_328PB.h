/*
	Macros for ATMEGA 88-168-328 family
*/
#ifndef __SMD_ATMEGA_328PB_H__
#define __SMD_ATMEGA_328PB_H__

namespace smd_avr {

#define U_CONTROLLER ATMEGA328PB
// ----- SPI COMMS ----------------
// Compatible with 88/168/328P
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
// Compatible with 88/168/328P
#define I2C_PORT	        PORTC
#define I2C_PIN		        PINC
#define I2C_DDR		        DDRC
#define I2C_SDA_GPIO        PC4
#define I2C_SCL_GPIO        PC5

// ----- SERIAL COMMS -------------
// Compatible with 88/168/328P
#define USART_TX_GPIO       PD1
#define USART_TX_PORT       PORTD
#define USART_TX_DDR        DDRD
#define USART_RX_GPIO       PORTD0
#define USART_RX_PIN        PIND
#define USART_RX_DDR        DDRD

// Additional ports (not found on P version of chip)

#define SPI_SS1_GPIO         PE2
#define SPI_SS1_PORT         PORTE
#define SPI_SS1_PIN			 PINE
#define SPI_SS1_DDR          DDRE

#define SPI_MOSI1_GPIO       PE3
#define SPI_MOSI1_PORT       PORTE
#define SPI_MOSI1_PIN	     PINE
#define SPI_MOSI1_DDR        DDRE

#define SPI_MISO1_GPIO       PC0
#define SPI_MISO1_PORT       PORTC
#define SPI_MISO1_PIN        PINC
#define SPI_MISO1_DDR        DDRC

#define SPI_SCK1_GPIO        PC1
#define SPI_SCK1_PORT        PORTC
#define SPI_SCK1_PIN         PINC
#define SPI_SCK1_DDR         DDRC

#define I2C1_PORT	         PORTE
#define I2C1_PIN		     PINE
#define I2C1_DDR		     DDRE
#define I2C1_SDA_GPIO        PE0
#define I2C1_SCL_GPIO        PE1

};	// namespace smd_avr

#endif
