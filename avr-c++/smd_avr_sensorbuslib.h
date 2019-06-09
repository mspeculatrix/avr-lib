/* some thinking out loud for a class for Sheldon Sensor Bus (SSB) 

use pin-change interrupts

Need to be able to define what pin to use for clock. Also
which DDRx, PORTx and PINx to use. SHould we standardise
on these for simplicity? Might be 

Going to need an array to hold the Data pins for a hub?

*/

enum SB_Device_Type { NODE, HUB };
enum UCONT { ATTINY84, ATMEGA328 };

#ifndef __SMD_AVR_SENSORBUSLIB__
#define __SMD_AVR_SENSORBUSLIB__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SB_ACK_PULSE 10			// ms ??
#define SB_ACK_PULSE_PADDING 5		// ms

#define SB_ERR_NONE 				0x00
#define SB_ERR_NOT_CONFIGURED		0x01
#define SB_ERR_HANDSHAKE_TIMEOUT	0x05

//const uint8_t ATTINY_INTS = {PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7};
//const uint8_t ATMEGA_INTS = {PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7};

#if defined(__AVR_ATtiny24A__) | defined(__AVR_ATtiny44A__) | defined(__AVR_ATtiny84A__)
	#define MICROCONTROLLER ATTINY84
	#define INT_CONT_REG	GIMSK
	#define SB_DDR			DDRB
	#define SB_PORT			PORTB
	#define SB_PIN			PINB
	//#define SB_DAT			PB2			// INT0
	#define SB_CLK			PB1
#endif

#if defined(__AVR_ATmega168__) | defined(__AVR_ATmega328__)
	#define MICROCONTROLLER ATMEGA328
	#define INT_CONT_REG	PCICR
	#define SB_DDR			DDRD
	#define SB_PORT			PORTD
	#define SB_PIN			PIND
	//#define SB_DAT			PD3			// INT0
	#define SB_CLK			PD4
#endif

// ISR(INT0_vect)
// {
//     // COPY THIS INTO THE MAIN PROGRAM
// 	// assumes you have an instance of SensorBus called: ssb
// 	ssb.setMsgIncoming(true);
// }

class SensorBus
{
public:
	SensorBus();
	void addChannel(uint8_t intBit);
	bool incomingMsgStatus(void) { return _incomingMsg; }
	void init(void);
	void setMsgIncoming(bool status);

protected:
	//uint8_t _dataPin;	// these don't make sense now
	uint8_t _clkPin;
	uint8_t _errorState;
	bool _incomingMsg;
	uint8_t _channels[8];
	uint8_t _numChannels;

	void _listen(void);
	
};

SensorBus::SensorBus() 
{
	_clkPin = SB_CLK;
	_errorState = SB_ERR_NONE;
	_incomingMsg = false;
	_numChannels = 0;
}

/**
@function	addChannel(uint8_t)
@brief		Add a data channel to this device.

Add a channel to the device. This sets up the interrupts on the data
line. The parameter should be the pin-change interrupt number for the
pin you want to use as a data line - eg, PCINT2.
Devices generally will have only one channel - typically to a hub.
There's no error-checking here to ensure that you've passed a valid
pin number, because that will vary depending on the microcontroller.
*/
void SensorBus::addChannel(uint8_t intBit)
{
	if(intBit < 8) {
		INT_CONT_REG |= (1 << PCIE0);
		PCMSK0 |= (1 << intBit);
	// } else if(intBit < 16) {
	// 	INT_CONT_REG |= (1 << PCIE1);
	// 	PCMSK1 |= (1 << intBit);
	// }// else {
//		INT_CONT_REG |= (1 << PCIE2);  // NOT COMPATIBLE WITH ATTINY
//		PCMSK2 |= (1 << intBit);
//	}
		this->_channels[_numChannels] = intBit;	// these three lines must go in this order
		this->_listenOnChannel(_numChannels);
		this->_numChannels++;
		//of course, also need to add an ISR function for each vector (two or three depending on chip)
	} else {
		// this is an error
	}

}

void SensorBus::init(void)
{
	//this->_listen();				// default to listening
}

void SensorBus::setMsgIncoming(bool status)
{
	this->_incomingMsg = status;
}

void SensorBus::_listenOnChannel(uint8_t channel)
{
	SB_DDR &= ~(1 << _channels[channel] | 1 << _clkPin);	// set as inputs ***NEEDS CHANGING***
	sei();										// enable interrupts
}

#endif
