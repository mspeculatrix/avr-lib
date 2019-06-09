/*
  C++ Serial Library for AVR microcontrollers
   *** IMPORTANT ***
  F_CPU must be set.
  Do this in the project Properties:
  Toolchain -> AVR/GNU C++ Compiler -> Symbols
  and add, under 'Defined symbols':
  F_CPU=16000000UL
  (or whatever the value should be.)

  Typical use:
  SMD_AVR_Serial serial = SMD_AVR_Serial(19200);

  uint8_t error = serial.begin();

  serial.write(msg);
  serial.writeln("Hello world");
  serial.write(255);
  serial.writeln(65535);
  serial.writeln(32767);
  serial.writeln(-32768);

  char inStr[127];
  serial.readLine(inStr, 127);

*/
#ifndef __SMD_AVR_SERIALLIB_H__
#define __SMD_AVR_SERIALLIB_H__

#include <avr/io.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SER_NUL 0x00  // 0  - NULL - often used for string termination
// reserving 1-3 for special uses
#define SER_EOT 0x04  // 4  - end of transmission
#define SER_ENQ 0x05  // 5  - enquiry - might come in handy one day
#define SER_BEL 0x07  // 7  - bell
#define SER_NL  0x0A  // 10 - newline/line feed
#define SER_CR  0x0D  // 13 - carriage return
#define SER_ESC 0x1B  // 27 - escape
#define SER_SPC 0x20  // 32 - space
#define NO_DATA 0x80  // 128 use this as a flag to indicate no data waiting in
                      // byte buffer. It's the high-order version of null

#define SER_READLINE_BUFFER_MAX 255

// ERROR & RESULT CODES
//#define SER_RES_INT_TOO_LARGE 1
#define SER_ERR_INCORRECT_DATA_BITS 1
#define SER_ERR_INCORRECT_STOP_BITS 2
#define SER_ERR_BAUD_TOO_LOW 3
#define SER_ERR_BAUD_TOO_HIGH 4
#define SER_ERR_UBRR_OVERFLOW 5
#define SER_RES_EMPTY_STRING 6
#define SER_ERR_READLINE_BUFFER_TOO_SMALL 7

class SMD_AVR_Serial {
    public:
        // Constructors
        SMD_AVR_Serial();
        SMD_AVR_Serial(uint16_t baudrate);
        SMD_AVR_Serial (uint16_t baudrate, uint8_t dataBits, uint8_t stopBits);

        // Methods
        uint8_t begin();                // initialise
        bool started();
		bool clearBuffer();

        // Receiving
        uint8_t getByte();              // read a byte - blocking
        bool inWaiting();               // is there a byte waiting?
        uint8_t readByte();             // read a byte - non-blocking
        uint8_t readLine(char * buffer, size_t bufferSize, bool preserveNewline);

        // Transmitting
        bool sendByte(uint8_t byteVal); 		// send single byte

        uint8_t write(const char string[]);
        uint8_t write(const int twoByteInt);	// max value 32767
        uint8_t write(const long longInt);
        uint8_t write(const double fnum);

        uint8_t writeln(const char string[]);
        uint8_t writeln(const int twoByteInt);
        uint8_t writeln(const long longInt);
        uint8_t writeln(const double fnum);

    protected:
        uint16_t _baud;
        uint8_t _dataBits;
        bool _echo;
        uint8_t _stopBits;
        bool _started;

        void _init (uint16_t baudrate, uint8_t dataBits, uint8_t stopBits);
        //int _max_int_size;
        //uint8_t _itoa_str_length;
};


#endif
