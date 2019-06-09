
#include <avr/io.h>
#include "smd_avr_usi_i2clib.h"

struct I2C_bus {
    unsigned long busSpeed;
    uint8_t errorState;
} USI_I2C_bus;



void USI_I2C_initialiseBus(void)
{
	DDR_USI  |= (1<<PIN_USI_SCL);           // Enable SCL as output.
	DDR_USI  |= (1<<PIN_USI_SDA);           // Enable SDA as output.

	PORT_USI |= (1<<PIN_USI_SDA);           // set high - released state.
	PORT_USI |= (1<<PIN_USI_SCL);           // set high - released state.

	USIDR    = 0xFF;                       // Preload with "released level" data.
	USICR    = (0<<USISIE)|(0<<USIOIE)|				// Disable Interrupts
               (1<<USIWM1)|(0<<USIWM0)|				// Set USI in Two-wire mode
               (1<<USICS1)|(0<<USICS0)|(1<<USICLK)| // Software stobe as counter clock source
               (0<<USITC);
	USISR   =  (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|	// Clear flags,
               (0x0<<USICNT0);									// reset counter

    USI_I2C_bus.errorState = USI_I2C_ERR_NONE;
}


uint8_t USI_I2C_start(uint8_t address, uint8_t mode)
{
	DDR_USI |= (1<<PIN_USI_SDA);		// Enable SDA as output
	PORT_USI |= (1 << PIN_USI_SCL);		// Release SCL by taking high
	while( !(PIN_USI & (1<<PIN_USI_SCL)) ); // Verify that SCL is high
	_delay_us(START_DELAY);
	PORT_USI &= ~(1 << PIN_USI_SDA);	// bring SDA LOW
	_delay_us(T4_TWI / 4);                       
	PORT_USI &= ~(1 << PIN_USI_SCL);	// Pull SCL LOW
	PORT_USI |= (1 << PIN_USI_SDA);		// Release SDA
	while(!(USISR & (1 << USISIF)));
	// if( !(USISR & (1 << USISIF)) ) {
	// 	USI_I2C_bus.errorState = USI_I2C_ERR_MISSING_START_CON;
	// }
	USI_I2C_sendByte(address + mode);
	//_delay_us(T4_TWI);
	//USIDR = 0xFF;
	return USI_I2C_bus.errorState;
}


uint8_t USI_I2C_sendByte(uint8_t byte)
{
	//PORT_USI &= ~(1<<PIN_USI_SCL);	// Pull SCL LOW
	USIDR = byte;					// put byte in data register
	_USI_I2C_transceive(USI_I2C_BYTE_COUNTER);
	/* Clock and verify (N)ACK from slave */
	DDR_USI  &= ~(1<<PIN_USI_SDA);	// Enable SDA as input
	if( _USI_I2C_transceive(USI_I2C_ACK_COUNTER) & (1 << USI_I2C_NACK_BIT) ) {
		USI_I2C_bus.errorState = USI_I2C_ERR_NO_ACK;
	}
	return USI_I2C_bus.errorState;
}

uint8_t USI_I2C_stop(void)
{
	// the master signals the end of the transmission – the ‘stop condition’ 
	// by holding SDA low, allowing SCL to go high where it stays, and then 
	// allowing SDA to go high.
	PORT_USI &= ~(1<<PIN_USI_SDA);			// ensure SDA is low
	PORT_USI |= (1<<PIN_USI_SCL);			// Release SCL
	while( !(PIN_USI & (1<<PIN_USI_SCL)) );	// Wait for SCL to go high
	_delay_us(T4_TWI / 4);               
	PORT_USI |= (1 << PIN_USI_SDA);			// Release SDA
	_delay_us(T2_TWI / 4);                
	// if( !(USISR & (1<<USIPF)) ) {
	// 	USI_I2C_bus.errorState = USI_I2C_ERR_MISSING_STOP_CON;
	// }
	//return USI_I2C_bus.errorState;
	return true;
}


uint8_t USI_I2C_readRegisterByte(uint8_t address, uint8_t reg)
{
	uint8_t regVal = 0;
	USI_I2C_start(address, I2C_WRITE_MODE);
	USI_I2C_sendByte(reg);
	USI_I2C_start(address, I2C_READ_MODE);
	DDR_USI &= ~(1<<PIN_USI_SDA);				// Enable SDA as input
	regVal = _USI_I2C_transceive(USI_I2C_BYTE_COUNTER);
	USIDR = 0xFF;					// Load NACK to confirm End Of Transmission
	_USI_I2C_transceive(USI_I2C_ACK_COUNTER);	// Generate ACK/NACK
	USI_I2C_stop();
	return regVal;
}

uint16_t USI_I2C_readRegisterWord(uint8_t address, uint8_t reg)
{
	// ***** THIS FUNCTION IS UNTESTED *****
	uint16_t regVal = 0;
	USI_I2C_start(address, I2C_WRITE_MODE);
	USI_I2C_sendByte(reg);
	USI_I2C_start(address, I2C_READ_MODE);
	DDR_USI &= ~(1<<PIN_USI_SDA);		// Enable SDA as input
	regVal = (_USI_I2C_transceive(USI_I2C_BYTE_COUNTER)) << 8;	// MSB first
	USIDR = 0x00;						// Load ACK
	_USI_I2C_transceive(USI_I2C_ACK_COUNTER);			// Generate ACK
	regVal += _USI_I2C_transceive(USI_I2C_BYTE_COUNTER);
	USIDR = 0xFF;						// Load NACK
	_USI_I2C_transceive(USI_I2C_ACK_COUNTER);			// Generate NACK
	USI_I2C_stop();
	return regVal;
}


uint8_t _USI_I2C_transceive(uint8_t counter)
{
	// assumes byte to be sent is in USIDR
	// If counter == 0 (USI_I2C_BYTE_COUNTER), then the do..while loop below 
	// will need to execute 8 times before 4-bit counter overflows and sets 
	// USIOIF. So that's used for clocking out a byte.
	// If counter == 0x0E (0b0001110 or USI_I2C_ACK_COUNTER), then a single
	// clock - ie, once through the loop - will achieve this.
	USISR = (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC); // clear flags
	//USISR |= (counter << USICNT0);	// set counter value for clocking
	USISR |= counter;	// set counter value for clocking

	uint8_t toggle_clock = (0<<USISIE)|(0<<USIOIE)| // Interrupts disabled
           (1<<USIWM1)|(0<<USIWM0)|                 // Set USI in Two-wire mode.
           (1<<USICS1)|(0<<USICS0)|(1<<USICLK)|     // Software clock strobe as source
           (1<<USITC); // toggles clock to increment the 4-bit counter
    do {
    	_delay_us(T2_TWI / 4);
		USICR = toggle_clock;	// toggle SCL high, shifts next data bit
    	while( !(PIN_USI & (1 << PIN_USI_SCL)) );	// Wait for SCL to go high
    	_delay_us(T4_TWI / 4);
		USICR = toggle_clock;	// toggle SCL low again
    } while( !(USISR & (1<<USIOIF)) );		// Wait until transfer complete
	_delay_us(T2_TWI / 4);
	uint8_t data = USIDR;					// read response
	USIDR = 0xFF;							// Release SDA.
	DDR_USI |= (1<<PIN_USI_SDA);			// Enable SDA as output.
	return data;
}
