#ifndef __BOARD_H__
#define __BOARD_H__

#include <util/delay.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include "LUFAConfig.h"
#include <LUFA.h>

#define MIDI_BAUD 31250


#define BOARD_NECROWARE_REV03_ATMEGA32


#ifdef __cplusplus
  extern "C" {
#endif

extern void board_init(void);
extern void pull_trigger(void);
extern void PulseAndWait( uint16_t dly_us );
extern void board_uart_poll( void (*callback)(uint8_t b) );
extern int  board_ffb_putc(uint8_t byte);
extern void board_uart_putc(uint8_t byte);

/* don't have a better name, it's a custom adapter board */

#ifdef BOARD_NECROWARE_REV03_ATMEGA32

// Maximum number of parallel effects in memory (TODO: fix 2 wasted)
#define MAX_EFFECTS      12

#define USE_SOFTWARESERIAL

// MIDI uses SoftwareSerial
//#define rxPin 9
//#define txPin 5
//#define MIDI_INTERFACE    SoftSerial
// use the HW serial interface: for this to work you have to connect TX pin and pin 5 on Arduino micro with a wire
#define MIDI_USES_HARDWARESERIAL (1)
#define MIDI_INTERFACE    Serial1

// Debug port is 115200 8N1
// Method of debugging
#define DEBUG_ENABLE_UART 1
#define DEBUG_INTERFACE   SoftSerial
#define rxPin 3
#define txPin 2

// you can list effects and "trace" them (within an interval, e.g. 300ms) 
// if you set LogLevel to DEBUG_LOG_EFFECT and the apps use 10-30Hz on FFB, it slows down a lot
//#define ENABLE_SERIAL_DEBUG_CMDS  1

// Define: ENABLE_CDC
//  if defined, includes USB virtual serial COM port to the device
//#define ENABLE_CDC    (1)

#ifdef ENABLE_CDC
#define DEBUG_ENABLE_USB  (1)
#else
#endif

// if you define NDEBUG lots of PSTR/PROGMEM is saved (~3KiB)

#define NDEBUG  (1)


// parameter PSTR savings
// no other joysticks supported
#define SAVE_SPACE  1

// do not support FFPro joystick
// to fit into flash: NDEBUG & SAVE_SPACE & !ENABLE_SERIAL_DEBUG_CMDS
// it compiles but its likely broken (just follow the apply* logic in SetEffect)
#define WHEEL_ONLY_DRIVER 1


// Windows 7 accepts now the descriptor, Win10/Dirt2 & Dirt 4 & Dirt 2.0 do, 
//  but they don't use custom forces
// CMR Rally Dirt 2.0 uses custom force - but hangs because the driver does not handle it 
//#define SUPPORT_CUSTOM_FORCE

// Windows 7 does not like that (but the driver does not offer such an endpoint)
//#define SUPPORT_CUSTOM_FORCE_ISOCHRONUOS

// define for a more complete version
// Win10 fedit does not accept that (pid.dll requests the PID_POOL)
#define NEW_PID_POOL_REPORT   (1)

// for now: always manage the pool by device/uController
#define DEVICE_MANAGED_POOL   1

// this is stated in the spec, but fedit also does it wrong, it uses that for defering the play! 
#define SUPPORT_START_DELAY   (1)
// report the gain (upsets Windows7/fedit)
//#define SUPPORT_DEVICE_GAIN   (1)

// instead of having no AutoCenter: level 2 gives a softer one but it also affects the strength of FFB?
//#define SUPPORT_ADJUST_CENTER_SPRING  2
// that does not work yet
//#define SUPPORT_ADJUST_FORCE_GAIN     (1)

/* try to report only x-axis on the wheel
 * fedit only shows one axis in force/properties but then:  
 * couldn't create the effect!
 * hResult = 0x80070057: invalid parameters
 */
//#define REPORT_ONLY_1AXIS


// experimental: only send modify cmds if parameters have changed
// it looks like the PIDState goes to 0x6xxx if the MIDI load/byte rate gets too high
#define SEND_ONLY_CHANGED_PARAMS
//#define DONT_ROTATE_DIRECTION_ON_CONSTANTFORCE


#if defined(MIDI_USES_HARDWARESERIAL) && defined(MIDI_USES_SOFTWARESERIAL)
#error one or the other
#endif

#undef BOARD
#define BOARD BOARD_MICRO
#include <LUFA/LUFA/Drivers/Board/LEDs.h>

#define __board_init()  {}

#define __board_midi_putc(byte) MIDI_INTERFACE.write(byte)

        // Wait if a byte is being transmitted
//#define __board_uart_putc(byte)   while ((UCSR1A & (1 << UDRE1)) == 0); UDR1 = byte;
#define __board_uart_putc(byte)  DEBUG_INTERFACE.write(byte)

#define __board_pull_trigger()     x1.setHigh();  \
    x2.setHigh();       \
    delayMicroseconds(256); \
    x1.setLow(); \
    x2.setLow();  


// for further space savings remove arduino main.c and replace it with something like below
/*
sh nm-size-sort.sh  <.elf> 
00000290 t Sidewinder::readPacket(unsigned char)
00000296 t ffb_handleDeviceControl
00000306 t SwFFB::update()
00000312 T malloc
00000328 t applyPeriod
00000365 b gDevice
00000370 t applyEnvelopeOnWave.part.1
00000420 T __vector_10
00000450 t _dbg_command.constprop.55
00000556 t ffb_handleFeatureSetReport.constprop.38
00000828 t _dbg_dumpEffect
00000902 t global constructors keyed to 65535_0_Descriptors.c.o.4647
00000948 T vfprintf
00001056 t WheelReportDescriptor
00001806 t USB_Device_ProcessControlRequest
00002028 t Sidewinder::decode(Buffer<68u> const&, Joystick::State&) const [clone .isra.3]
00002944 T main
00002954 t ffb_handleSetReport.constprop.57

__crt0_init which copies initialized data, zeroes uninitialized data, sets __heap_start

a minimal   pinMode(tx, OUTPUT);
{
  _transmitBitMask = digitalPinToBitMask(tx);
  uint8_t port = digitalPinToPort(tx);
  _transmitPortRegister = portOutputRegister(port);
}
*/


#else





#ifdef BOARD_NECROWARE_REV03_TEENSY40

// Maximum number of parallel effects in memory (TODO: fix 2 wasted)
#define MAX_EFFECTS      20

// MIDI uses SoftwareSerial
#define rxPin 9
#define txPin 5
#define MIDI_USES_SOFTWARESERIAL    1
// for this to work you have to connect TX pin and pin 5 on Arduino micro with a wire
//#define MIDI_USES_HARDWARESERIAL    (1)

// Define: ENABLE_CDC
//  if defined, includes USB virtual serial COM port to the device
#define ENABLE_CDC    (1)

#ifdef ENABLE_CDC
#define DEBUG_ENABLE_USB  (1)
#else
#endif

// Debug port is HardwareSerial 115200 8N1
// Method of debugging
#define DEBUG_ENABLE_UART   1

// if you define NDEBUG lots of PSTR/PROGMEM is saved (~3KiB)

//#define NDEBUG  (1)


// parameter PSTR savings
// no other joysticks supported
#define SAVE_SPACE  1

// you can list effects and "trace" them (within an interval, e.g. 300ms) 
// if you set LogLevel to DEBUG_LOG_EFFECT and the apps use 10-30Hz on FFB, it slows down a lot
#define ENABLE_SERIAL_DEBUG_CMDS  1

// do not support FFPro joystick
// to fit into flash: NDEBUG & SAVE_SPACE & !ENABLE_SERIAL_DEBUG_CMDS
// it compiles but its likely broken (just follow the apply* logic in SetEffect)
#define WHEEL_ONLY_DRIVER 1


// Windows 7 accepts now the descriptor, Win10/Dirt2 & Dirt 4 & Dirt 2.0 do, 
//  but they don't use custom forces
// CMR Rally Dirt 2.0 uses custom force - but hangs because the driver does not handle it 
#define SUPPORT_CUSTOM_FORCE

// Windows 7 does not like that (but the driver does not offer such an endpoint)
//#define SUPPORT_CUSTOM_FORCE_ISOCHRONUOS

// define for a more complete version
// Win10 fedit does not accept that (pid.dll requests the PID_POOL while win7 does not)
#define NEW_PID_POOL_REPORT   (1)

// for now: always manage the pool by device/uController
#define DEVICE_MANAGED_POOL   1

// this is stated in the spec, but fedit also does it wrong, it uses that for defering the play! 
#define SUPPORT_START_DELAY   (1)
// report the gain (upsets Windows7/fedit)
//#define SUPPORT_DEVICE_GAIN   (1)

//#define SUPPORT_ADJUST_CENTER_SPRING  (1)
// that does not work yet
//#define SUPPORT_ADJUST_FORCE_GAIN     (1)

/* try to report only x-axis on the wheel
 * fedit only shows one axis in force/properties but then:  
 * couldn't create the effect!
 * hResult = 0x80070057: invalid parameters
 */
//#define REPORT_ONLY_1AXIS


// experimental: only send modify cmds if parameters have changed
// it looks like the PIDState goes to 0x6xxx if the MIDI load/byte rate gets too high
#define SEND_ONLY_CHANGED_PARAMS
//#define DONT_ROTATE_DIRECTION_ON_CONSTANTFORCE

#if defined(MIDI_USES_HARDWARESERIAL) && defined(DEBUG_ENABLE_UART)
#error can not use serial port for both
#endif

#if defined(MIDI_USES_HARDWARESERIAL) && defined(MIDI_USES_SOFTWARESERIAL)
#error one or the other
#endif

#undef BOARD
#define BOARD BOARD_MICRO
#include <LUFA/LUFA/Drivers/Board/LEDs.h>

#define __board_init()  {}

#ifdef MIDI_USES_HARDWARESERIAL
#define __board_midi_putc(byte) Serial1.write(byte)
#else
#define __board_midi_putc(byte) SerialMidi.write(byte)
#endif

        // Wait if a byte is being transmitted
#define __board_uart_putc(byte)   while ((UCSR1A & (1 << UDRE1)) == 0); UDR1 = byte;

#define __board_pull_trigger()     x1.setHigh();  \
    x2.setHigh();       \
    delayMicroseconds(256); \
    x1.setLow(); \
    x2.setLow();  





#else

// this needs work, I just collected pieces that need to be adapted
#include <avr/sfr_defs.h>


#define LEDPORT         PORTD
#define LEDBIT          PD6

#define TRGDDR          DDRB

#define TRGY2BIT        DDB5
#define TRGX1BIT        DDB4

#define BUTPIN          PINB
#define BUT1            PORTB0
#define BUT2            PORTB1
#define BUT3            PORTB2
#define BUT4            PORTB3

// TODO: find those
// TODO: force as __attribute_inline__

#define __board_init () {_NOP();}

inline __attribute__((always_inline)) __board_pull_trigger()
{ 
        clr_bit( TRGDDR, TRGX1BIT ) ; \
        clr_bit( TRGDDR, TRGY2BIT ) ; \
        _delay_us10(5); \
        set_bit( TRGDDR, TRGX1BIT ) ; \
        set_bit( TRGDDR, TRGY2BIT ) ; \
        _delay_us10(1); 
}

#define __board_midi_putc(data) { \
  /* Wait if a byte is being transmitted */ \
  while ((UCSR1A & (1 << UDRE1)) == 0); \
  UDR1 = data;\
}\

#define __board_init_HWserial  \
  /* Check TX-pin (PD3) settings */ \
  DDRD = DDRD | 0b00001000;        \
\
  /* Set baud rate */     \
  UCSR1A = 0;             \
  UBRR1 = ((F_CPU / (31250ul << 4)) - 1);   \
\
  /* Set frame format to 8 data bits, no parity, 1 stop bit, 1 start bit */   \
  UCSR1C = (1 << 7) | (1 << UCSZ11) | (1 << UCSZ10);              \
  /* Enable transmitter only */                             \
  UCSR1B = (1 << TXEN1);                                    \
                                                            \
  UDR1 = 0;  /* write something to get things going */      \


// ----------------------------------------------
// Ring buffer for sending MIDI data to joystick
// ----------------------------------------------

// Buffer for sending data to MIDI
//#define MIDI_BUFFER_SIZE 128

#ifndef MIDI_BUFFER_SIZE

#else

// Buffered MIDI

volatile uint8_t gMidiBuffer[MIDI_BUFFER_SIZE];
volatile unit8_t *midi_buffer_head = gMidiBuffer;
volatile unit8_t *midi_buffer_tail = gMidiBuffer;

void FfbSendByte(uint8_t data)
{
  cli();

  *gMidiBufferHead++ = data;

  if (gMidiBufferHead == gMidiBufferTail)
  { // Ouch - buffer overflown!

    return;
  }

  if (gMidiBufferHead - gMidiBuffer >= MIDI_BUFFER_SIZE)
    gMidiBufferHead = gMidiBuffer;

  sei();
}


ISR(USART1_UDRE_vect)
{
  cli();

  uint8_t i;

  if (gMidiBufferHead == gMidiBufferTail)
  {
    // Buffer is empty, disable transmit interrupt
    UCSR1B = (1 << TXCIE1) | (1 << TXEN1);
  }
  else
  {
    i = gMidiBufferTail + 1;
    if (i >= MIDI_BUFFER_SIZE) i = 0;
    UDR1 = gMidiBuffer[i];
    gMidiBufferTail = i;
  }

  sei();
}
#endif // MIDI_BUFFER_SIZE


#endif

#endif   // BOARD_NECROWARE_REV03_TEENSY40



#ifdef __cplusplus
  }
#endif

#endif
