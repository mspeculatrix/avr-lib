/*	SHELNET

	*** OBSOLETE - NOW WORKING WITH smd_avr-sensorbuslib.h ***

*/
#include <avr/io.h>
#include <util/delay.h>

#ifndef __SMD_AVR_SHELNETLIB_H__
#define __SMD_AVR_SHELNETLIB_H__

#define SHELNET_BUFFER_SIZE 32	// number of bytes for input/output buffers

#define SHELNET_CLK_DELAY 20
#define SHELNET_ALERT_DELAY 20
#define SHELNET_HANDSHAKE_COUNT_MAX 255

#define SHELNET_ERR_NONE 				0x00
#define SHELNET_ERR_NOT_CONFIGURED		0x01
#define SHELNET_ERR_HANDSHAKE_TIMEOUT	0x05
#define SHELNET_INT_EN		PCIE0
#define SHELNET_INT_BIT		PCINT0

#if defined(__AVR_ATtiny24A__) | defined(__AVR_ATtiny44A__) | defined(__AVR_ATtiny84A__)
	#define DDR_SHELNET			DDRB
	#define PORT_SHELNET		PORTB
	#define PIN_SHELNET			PINB
	#define SHELNET_DATA		PB0
	#define SHELNET_CLK			PB1
#endif

#if defined(__AVR_ATmega168__) | defined(__AVR_ATmega328__)
	#define DDR_SHELNET         DDRA
	#define PORT_SHELNET        PORTA
	#define PIN_SHELNET         PINA
	#define SHELNET_DATA        PA2
	#define SHELNET_CLK         PA3
#endif

class Shelnet
{
public:
	volatile uint8_t *_portreg;
	volatile uint8_t *_pinreg;
	uint8_t _ddr;
	uint8_t _dataPin;
	uint8_t _clkPin;
	uint8_t _errorState;
protected:

};

Shelnet::Shelnet(void) 
{
	_portreg = PORT_SHELNET;
	_pinreg = PIN_SHELNET;
	_ddr = DDR_SHELNET;
	_dataPin = SHELNET_DATA;
	_clkPin = SHELNET_CLK;
	_errorState = SHELNET_ERR_NONE;
}

void Shelnet::init(void)
{
	GIMSK |= (1 << SHELNET_INT_EN);	// enable PC interrupts via PCI0 vector
	PCMSK0 |= (1 << SHELNET_INT_BIT);	// enable interrupts on data line
	this->listen();	// default to listening
}

void Shelnet::_listen(void)
{
	_ddr &= ~(1 << _dataPin | 1 << _clkPin);	// set as inputs
	sei();										// enable interrupts
}

/** OLD VERSION **

/*struct shelnet_config {
	volatile uint8_t *portreg;
	volatile uint8_t *pinreg;
	uint8_t ddr;
	uint8_t clk;
	uint8_t data;
	uint8_t alert;
	bool configured;
	uint8_t errorState;
	shelnet_config() {	// constructor to init values where this makes sense
		configured = false;
		errorState = SHELNET_ERR_NONE;
	}
} shelnetCfg;


void shelnetInit(volatile uint8_t *portreg, uint8_t *pinreg, uint8_t ddr, 
		uint8_t clk_pin, uint8_t data_pin, uint8_t alert_pin)
{
	shelnetCfg.data = data_pin;
	shelnetCfg.clk = clk_pin;
	shelnetCfg.alert = alert_pin;
	shelnetCfg.ddr = ddr;
	shelnetCfg.port = portreg;
	shelnetCfg.pin = pinreg;
	GIMSK |= (1 << PCIE0);	// Set General Interrupt Mask register to enable
							// PC interrupts via PCI0 interrupt vector (PA0-PA7)
	PCMSK0 |= (1 << PCINT0);	// enable PCINT0 (PA0)
	shelnetCfg.configured = true;
}

uint8_t shelnetListen()
{
	if(shelnetCfg.configured) {
		// clock, data and alert lines default to inputs
		shelnetCfg.ddr &= ~( (1 << shelnetCfg.data) | (1 << shelnetCfg.clk) | (1 << shelnetCfg.alert) );
		// enable interrupts
		sei();
	} else {
		shelnetCfg.errorState = SHELNET_ERR_NOT_CONFIGURED;
	}
	return shelnetCfg.errorState;
}

uint8_t shelnetRead(char * buffer, size_t bufferSize)
{
	//Dealing with incoming communication.
	//When interrupt is triggered by alert line:
	// turn off interrupts
	cli();
	uint8_t byteIndex = 0;
	uint8_t bitIndex = 7;
	uint8_t	byte = 0;
	// set clock line to output
	shelnetCfg.ddr |= (1 << shelnetCfg.clk);
	// set clock low
	*shelnetCfg.port &= ~(1 << shelnetCfg.clk);
	// pause
	_delay_us(SHELNET_CLK_DELAY);
	// set clock to high
	*shelnetCfg.port |= (1 << shelnetCfg.clk);
	// - set clock to input
	shelnetCfg.ddr &= ~(1 << shelnetCfg.clk);
	// - while alert line is low
	// 	- while clock low, loop
	// 	- read data line
	// 	- add bit to byte
	// 	- decrement bit counter
	// 	if bit counter == -1
	// 		add byte to buffer
	// 		bit counter = 7
	// 		byte index++
	// 		check index against buffer size
	// 	- while clock high, loop
	// - check that we got a full 8 bits last time
	// turn on interrupts
	sei();
	return shelnetCfg.errorState;
}

uint8_t SMD_ShelNet::send(char * buffer, size_t bufferSize)
{

	// disable interrupts
	cli();

	// set all lines to outputs
	shelnetCfg.ddr |= ( (1<<shelnetCfg.data) | (1<<shelnetCfg.alert) | 1<<shelnetCfg.clk);

	// take alert line low to initiate communications
	*shelnetCfg.port &= ~(1<<shelnetCfg.alert);
	// wait a wee while
	_delay_us(SHELNET_ALERT_DELAY);
	// release the alert line
	*shelnetCfg.port |= (1<<shelnetCfg.alert);
	// make alert line an input
	shelnetCfg.ddr &= ~(1<<shelnetCfg.alert);
	// wait for alert line to go low in response
	uint8_t counter = 0;
	bool receivedResponse = false;
	while(counter < SHELNET_HANDSHAKE_COUNT_MAX) {
		if( *shelnetCfg.pin & (1<<shelnetCfg.alert) ) {	// line is high
			counter++;
			_delay_us(SHELNET_ALERT_DELAY);
		} else {
			counter = SHELNET_HANDSHAKE_COUNT_MAX;
			receivedResponse = true;
			//now wait for it to go high again - released by receiver
			while(! *shelnetCfg.pin & (1<<shelnetCfg.alert));
			// make alert line output again
			shelnetCfg.ddr |= (1<<shelnetCfg.alert);
			// and take line low to complete handshake
			*shelnetCfg.port &= ~(1<<shelnetCfg.alert);
			// pause to ensure listened at other end can get ready
			_delay_us(SHELNET_ALERT_DELAY);
		}
	}
	if(receivedResponse) {
		// take clock low
		*shelnetCfg.port &= ~(1<<shelnetCfg.clk);
		bool done = false;
		size_t idx = 0;
		do {
			uint8_t byte = buffer[idx];
			for (uint8_t bit = 7; bit >= 0; bit--) {	// MSB first
				// take clock low
				*shelnetCfg.port &= ~(1<<shelnetCfg.clk);
				// put bit on data line
				*shelnetCfg.port |= (((byte & (1<<bit)) >> bit) << shelnetCfg.data);
				// slight pause
				_delay_us(SHELNET_CLK_DELAY);
				// take clock high to clock out bit
				*shelnetCfg.port |= (1<<shelnetCfg.clk);
				// slight pause
				_delay_us(SHELNET_CLK_DELAY);
			}
			idx++;
			if(idx == bufferSize - 1) done = true;
		} while !done;
	} else {
		shelnetCfg.errorState = SHELNET_ERR_HANDSHAKE_TIMEOUT;
	}
	// go back to listing state. This will release the alert line signalling
	// that communication is over.
	this->listen();
	return shelnetCfg.errorState;
}
*/

#endif
