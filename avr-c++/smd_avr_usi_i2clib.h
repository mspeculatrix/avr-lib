/*
	Library for using I2C via the USI port on some AVRs - eg, ATTINY84

	This is heavily based on the code provided by Atmel in Application Note AN2561
	and the accompanying code AVR310.

*/

#include <avr/io.h>
#include <util/delay.h>

#ifndef __SMD_AVR_USI_I2CLIB_H__
#define __SMD_AVR_USI_I2CLIB_H__

/* Defines in common with other I2C library */
#define I2C_READ_MODE  1
#define I2C_WRITE_MODE 0
#define I2C_BUS_SPEED_STD  100000UL		// 100KHz
#define I2C_BUS_SPEED_FAST 400000UL		// 400KHz - this library can't cope with the even faster standards
#define I2C_BYTE_ORDER_LSB_FIRST 0
#define I2C_BYTE_ORDER_MSB_FIRST 1
#define I2C_ERR_NONE 0
#define I2C_ERR_START_FAILED 0x01	// error codes should always be odd so
#define I2C_ERR_NO_ACK_RECVD 0x03	// there's always a 1 in the LSB

#define I2C_USE_FAST_MODE

// let's assume we're using a fast bus
#ifndef I2C_USE_FAST_MODE
    #define T2_TWI    ((F_CPU *4700) /1000000) +1 // >4,7us
    #define T4_TWI    ((F_CPU *4000) /1000000) +1 // >4,0us
    #define START_DELAY (const int)(T2_TWI / 4)
#else
    #define T2_TWI    ((F_CPU *1300) /1000000) +1 // >1,3us
    #define T4_TWI    ((F_CPU * 600) /1000000) +1 // >0,6us
    #define START_DELAY (const int)(T4_TWI / 4)
#endif

/* *** DEFINES - taken from Atmel AN */

#define TWI_READ_BIT  0       // Bit position for R/W bit in "address byte".
#define TWI_ADR_BITS  1       // Bit position for LSB of the slave address bits in the init byte.
#define USI_I2C_NACK_BIT  0       // Bit position for (N)ACK bit.

#define USI_TWI_ERR_NO_DATA             0x01  // Transmission buffer is empty
//#define USI_TWI_DATA_OUT_OF_BOUND   0x01  // Transmission buffer is outside SRAM space
#define USI_TWI_ERR_UE_START_CON        0x02  // Unexpected Start Condition
#define USI_TWI_ERR_UE_STOP_CON         0x03  // Unexpected Stop Condition
#define USI_TWI_ERR_UE_DATA_COL         0x04  // Unexpected Data Collision (arbitration)
#define USI_TWI_ERR_NO_ACK_ON_ADDRESS   0x06  // The slave did not acknowledge  the address
// ones adopted by my code
#define USI_I2C_ERR_NONE                0x00
#define USI_I2C_ERR_NO_ACK              0x05  // The slave did not acknowledge  all data
#define USI_I2C_ERR_MISSING_START_CON   0x07  // Generated Start Condition not detected on bus
#define USI_I2C_ERR_MISSING_STOP_CON    0x08  // Generated Stop Condition not detected on bus

// new ones by me
#define USI_I2C_BYTE_COUNTER 0x00
#define USI_I2C_ACK_COUNTER 0x0E

// Device dependant defines

#if defined(__AVR_AT90Mega169__) | defined(__AVR_ATmega169PA__) | \
    defined(__AVR_AT90Mega165__) | defined(__AVR_ATmega165__) | \
    defined(__AVR_ATmega325__) | defined(__AVR_ATmega3250__) | \
    defined(__AVR_ATmega645__) | defined(__AVR_ATmega6450__) | \
    defined(__AVR_ATmega329__) | defined(__AVR_ATmega3290__) | \
    defined(__AVR_ATmega649__) | defined(__AVR_ATmega6490__)
    #define DDR_USI             DDRE
    #define PORT_USI            PORTE
    #define PIN_USI             PINE
    #define PORT_USI_SDA        PORTE5
    #define PORT_USI_SCL        PORTE4
    #define PIN_USI_SDA         PINE5
    #define PIN_USI_SCL         PINE4
#endif

#if defined(__AVR_ATtiny25__) | defined(__AVR_ATtiny45__) | defined(__AVR_ATtiny85__) | \
    defined(__AVR_AT90Tiny26__) | defined(__AVR_ATtiny26__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB0
    #define PORT_USI_SCL        PORTB2
    #define PIN_USI_SDA         PINB0
    #define PIN_USI_SCL         PINB2
#endif

#if defined(__AVR_AT90Tiny2313__) | defined(__AVR_ATtiny2313__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB5
    #define PORT_USI_SCL        PORTB7
    #define PIN_USI_SDA         PINB5
    #define PIN_USI_SCL         PINB7
#endif

// added by me
#if defined(__AVR_ATtiny24A__) | defined(__AVR_ATtiny44A__) | defined(__AVR_ATtiny84A__)
    #define DDR_USI             DDRA
    #define PORT_USI            PORTA
    #define PIN_USI             PINA
    #define PORT_USI_SDA        PORTA6
    #define PORT_USI_SCL        PORTA4
    #define PIN_USI_SDA         PINA6
    #define PIN_USI_SCL         PINA4
#endif

// General defines
#ifndef TRUE
	#define TRUE  1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

void USI_I2C_initialiseBus(void);
uint8_t USI_I2C_start(uint8_t address, uint8_t mode);    // returns error, if any
uint8_t USI_I2C_sendByte(uint8_t byte);
uint8_t USI_I2C_stop();

uint8_t USI_I2C_readRegisterByte(uint8_t address, uint8_t register);
uint16_t USI_I2C_readRegisterWord(uint8_t address, uint8_t register);


uint8_t _USI_I2C_transceive(uint8_t counter);

#endif
