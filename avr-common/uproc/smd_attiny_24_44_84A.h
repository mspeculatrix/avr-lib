/*
	Macros for ATTINY 24-44-84A family
*/

#ifndef __SMD_ATMEGA_24_44_84A_H__
#define __SMD_ATMEGA_24_44_84A_H__

// ----- SPI COMMS ----------------
#define SPI_SS_GPIO         PA7
#define SPI_SS_PORT         PORTA
#define SPI_SS_PIN          PINA
#define SPI_SS_DDR          DDRA

#define SPI_MOSI_GPIO       PA6
#define SPI_MOSI_PORT       PORTA
#define SPI_MOSI_PIN        PINA
#define SPI_MOSI_DDR        DDRA

#define SPI_MISO_GPIO       PA5
#define SPI_MISO_PORT       PORTA
#define SPI_MISO_PIN        PINA
#define SPI_MISO_DDR        DDRA

#define SPI_SCK_GPIO        PA4
#define SPI_SCK_PORT        PORTA
#define SPI_SCK_PIN         PINA
#define SPI_SCK_DDR         DDRA

// ----- I2C COMMS ----------------
#define I2C_PORT			PORTA
#define I2C_PIN			    PINA
#define I2C_DDR			    DDRA
#define I2C_SDA_GPIO        PA6
#define I2C_SCL_GPIO        PA4

// ----- SERIAL COMMS -------------
// No UART on this chip

#endif
