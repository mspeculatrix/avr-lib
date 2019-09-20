/*
  C++ Serial Library for AVR microcontrollers

  A lot of this comes from the Make book.
  Bits marked [*] come from: https://www.cl.cam.ac.uk/teaching/1011/P31/lib/usart1.c
*/

#include "smd_avr_seriallib.h"

// -------------------------------------------------------------------------
// -----  CONSTRUCTORS                                                 -----
// -------------------------------------------------------------------------
SMD_AVR_Serial::SMD_AVR_Serial ()  // instantiate with default baudrate,
{                                  // 8 data bits, 1 stop bit
    _init(19200, 8, 1);
}

SMD_AVR_Serial::SMD_AVR_Serial (uint16_t baudrate)    // instantiate with definable
{                                           // baudrate, 8 data bits, 1 stop bit
    _init(baudrate, 8, 1);
}

SMD_AVR_Serial::SMD_AVR_Serial (uint16_t baudrate, uint8_t dataBits, uint8_t stopBits)
{
    _init(baudrate, dataBits, stopBits);
}

void SMD_AVR_Serial::_init (uint16_t baudrate, uint8_t dataBits, uint8_t stopBits)
{
    _baud = baudrate;
    _dataBits = dataBits;
    _stopBits = stopBits;
    _started = false;
	_useCR = false;
	_sendNullTerminator = false;
}

// -------------------------------------------------------------------------
// -----  METHODS                                                      -----
// -------------------------------------------------------------------------
uint8_t SMD_AVR_Serial::begin()
{
    /* This is adapted from the setbaud.h header which essentially creates the
    macros UBRRH_VALUE, UBRRL_VALUE and USE_2X */
    uint8_t baud_tolerance = 2;
    bool use2x = false;
    unsigned long ubrr_value = ((F_CPU + 8UL * _baud) / (16UL * _baud) -1UL);
    uint8_t error = 0;

    if ( (100 * (F_CPU)) > (16 * (ubrr_value + 1)) * (100 * _baud + _baud * baud_tolerance)) {
        use2x = true;
    } else if (100 * (F_CPU) < (16 * (ubrr_value + 1)) * (100 * _baud - _baud * baud_tolerance)) {
        use2x = true;
    }

    if (use2x) {
        // recalculate
        ubrr_value = (((F_CPU) + 4UL * _baud) / (8UL * _baud) - 1UL);
        // UBRR_VALUE (((F_CPU) + 4UL * (BAUD)) / (8UL * (BAUD)) -1UL)
        // check for sane values
        // if 100 * (F_CPU) > (8 * ((UBRR_VALUE) + 1)) * (100 * (BAUD) + (BAUD) * (BAUD_TOL))
        if ( (100 * F_CPU) > (8 * (ubrr_value + 1)) * (100 * _baud + _baud * baud_tolerance)) {
            //error = SER_ERR_BAUD_TOO_HIGH; // *** THIS KEEPS GETTING SET!
        } else if (100 * (F_CPU) < (8 * (ubrr_value + 1)) * (100 * _baud - _baud * baud_tolerance)) {
            error = SER_ERR_BAUD_TOO_LOW;
        }
        UCSR0A |= (1 << U2X0);
    } else {
        UCSR0A &= ~(1 << U2X0);
    }
    // check for overflow
    if (ubrr_value >= (1 << 12)) {
        error = SER_ERR_UBRR_OVERFLOW;
    }

    if (error == 0) {
        UBRR0H = (uint8_t)(ubrr_value >> 8);	      // Set baud rate
        UBRR0L = (uint8_t)ubrr_value;
        UBRR0 = (uint16_t)ubrr_value;
        UCSR0B = (1 << TXEN0) | (1 << RXEN0);   // Enable USART transmitter/receiver

        // CONFIGURE COMMS SETTINGS
        UCSR0C = 0; // ensure all are unset
        switch(_dataBits) {
            // Allowing only values of 7 or 8. For this reason, we can ignore
            // bit 2 (UCSZ20) of register UCSR0B which only needs to be set to 1
            // if you want 9 data bits.
            case 7:
                UCSR0C |= (1 << UCSZ01);
                break;
            case 8:
                UCSR0C |=  (1 << UCSZ01) | (1 << UCSZ00);
                break;
            default:
                error = SER_ERR_INCORRECT_DATA_BITS;
                break;
        }
        switch(_stopBits) {
            case 2:
                UCSR0C &= ~(1 << USBS0);    // unset bit
                break;
            case 1:
                UCSR0C |= (1 << USBS0);     // set bit
                break;
            default:
                error = SER_ERR_INCORRECT_STOP_BITS;
                break;
        }
        UCSR0A |= (1 << TXC0);				// clear any existing transmits [*]
        if (error == 0) _started = true;
    }
    return error;
}

bool SMD_AVR_Serial::started(void)
{
    return _started;
}

bool SMD_AVR_Serial::clearBuffer()
{
	bool dataCleared = false;
	while(this->inWaiting()) {
		dataCleared = true;
		this->getByte();
	}
	return dataCleared;
}
// -------------------------------------------------------------------------
// -----  RECEIVING                                                    -----
// -------------------------------------------------------------------------
uint8_t SMD_AVR_Serial::getByte()
{
    // This method is blocking. It keeps checking for incoming data and
    // returns only when it has got some.
    loop_until_bit_is_set(UCSR0A, RXC0);       // Wait for incoming data
    return UDR0;                               // return register value
}


bool SMD_AVR_Serial::inWaiting()
{   // this might be a good candidate either for a macro or inline assembler
    return bit_is_set(UCSR0A, RXC0);
}

uint8_t SMD_AVR_Serial::readByte()
{
    // Non-blocking, but returns 0 if there's no data in the receive register.
	// As 0 might be a significant value in some cases, only use this if
	// testing for 0 as, say, a terminating value.
    uint8_t inByte = NO_DATA;
    if (bit_is_set(UCSR0A, RXC0)) {
        inByte = UDR0;
    }
    return inByte;
}

uint8_t SMD_AVR_Serial::readLine(char * buffer, size_t bufferSize, bool preserveNewline = true)
{
    // You must pass a buffer and the size of the buffer. Giving a buffer
    // size larger than the size of the actual buffer will result in a buffer
    // overflow and unpredictable results. The length of the string is
    // always one less than the size of the buffer because of the
    // null termination.
    // This reads input until:
    //		* It encounters a 0 (NULL)
    //		* It encounters a newline which is or is not included in the output depending on third param
    //		* It reaches the length of the buffer.
    // The incoming data is placed into the buffer. The method returns any
    // error encountered.
    uint8_t error = 0;
    if (bufferSize > SER_READLINE_BUFFER_MAX) bufferSize = SER_READLINE_BUFFER_MAX;
    if (bufferSize > 1) {
        bool ended = false;
        size_t index = 0;
        do {
			uint8_t byte = getByte();
            if (byte == 0) {
				buffer[index] = byte;
                ended = true;
            } else if (byte == SER_NL) {
                ended = true;
                if(preserveNewline) {
                    buffer[index] = SER_NL;
					if(index < bufferSize - 1) {
						buffer[index + 1] = 0;
					} else {
						// sorry, but the newline is toast
						buffer[index] = 0;	// think this duplicates case further down, but hey ho
					}
                } else {
                    buffer[index] = 0;
                }
			} else if(byte == SER_CR) {
				// ignore carriage returns???
            } else if (index == bufferSize - 2) {
				// we're at the penultimate char. The next one has to be a
				// terminating null, so let's add that and be done with it.
                ended = true;
                buffer[index + 1] = 0;
            } else {
                index++;
            }
        } while (!ended);
    } else {
        // minimum buffer size for this method is 2.
        error = SER_ERR_READLINE_BUFFER_TOO_SMALL;
    }
    return error;
}

// -------------------------------------------------------------------------
// -----  TRANSMITTING                                                 -----
// -------------------------------------------------------------------------

/** NB: currently we're not doing anything with the error codes - they're
	not getting set anywhere, we're just returning default values meaning
	success. They are in here for future development. **/

bool SMD_AVR_Serial::sendByte(uint8_t byteVal)
{
    bool _error = false;
    loop_until_bit_is_set(UCSR0A, UDRE0);   // Wait for empty transmit buffer
    UCSR0A |= (1<<TXC0); 					// clear txc flag [*]
    UDR0 = byteVal;                         // put data in transmit buffer
    return _error;
}

//uint8_t SMD_AVR_Serial::writeAscii(const uint8_t charcode)
//{
	//uint8_t resultCode = 0;
	//// could do all sorts of checking here to ensure code is within range
	//sendByte(charcode);
	//return resultCode;
//}
uint8_t SMD_AVR_Serial::write(const char * string)
{
	uint8_t error = _writeStr(string, false);
    return error;
}

uint8_t SMD_AVR_Serial::write(const double fnum)
{
	uint8_t error = _writeDouble(fnum, false);
    return error;
}

uint8_t SMD_AVR_Serial::write(const int twoByteInt)
{
	uint8_t error = _writeInt16(twoByteInt, false);
    return error;
}

uint8_t SMD_AVR_Serial::write(const long longInt)
{
	uint8_t error = _writeLongInt(longInt, false);
    return error;
}

uint8_t SMD_AVR_Serial::writeln(const char * string)
{
    uint8_t error = _writeStr(string, true);
    return error;
}

uint8_t SMD_AVR_Serial::writeln(const int twoByteInt)
{
    uint8_t error = _writeInt16(twoByteInt, true);
    return error;
}

uint8_t SMD_AVR_Serial::writeln(const long longInt)
{
    uint8_t error = _writeLongInt(longInt, true);
    return error;
}

uint8_t SMD_AVR_Serial::writeln(const double fnum)
{
    uint8_t error = _writeDouble(fnum, true);
    return error;
}



uint8_t SMD_AVR_Serial::_writeDouble(const double fnum, bool addReturn = false)
{
	uint8_t resultCode = 0;
	char numStr[30];
	// see: http://www.atmel.com/webdoc/AVRLibcReferenceManual/group__avr__stdlib_1ga060c998e77fb5fc0d3168b3ce8771d42.html
	dtostrf(fnum,3,5,numStr);
	_writeStr(numStr, addReturn);
	return resultCode;
}

uint8_t SMD_AVR_Serial::_writeInt16(const int twoByteInt, bool addReturn = false)
{
	uint8_t resultCode = 0;
	char numStr[20];
	itoa(twoByteInt, numStr, 10);
	//sprintf(numStr, "%i", twoByteInt);
	_writeStr(numStr, addReturn);
	return resultCode;
}

uint8_t SMD_AVR_Serial::_writeLongInt(const long longInt, bool addReturn = false)
{
	uint8_t resultCode = 0;
	char numStr[30];
	ltoa(longInt, numStr, 10);
	_writeStr(numStr, addReturn);
	return resultCode;
}

uint8_t SMD_AVR_Serial::_writeStr(const char * string, bool addReturn)
{
	// This is the main function used by the other write() and writeln() methods.
    uint8_t resultCode = 0;
    if (strlen(string) > 0) {
        uint8_t i = 0;
        do {
            sendByte(string[i]);
            i++;
        } while (string[i] != 0);
		if(addReturn) {
			if(_useCR) sendByte(SER_CR);
			sendByte(SER_NL);
		}
		if(_sendNullTerminator) sendByte(SER_NUL);
    } else {
        resultCode = SER_RES_EMPTY_STRING;
    }
    return resultCode;
}
