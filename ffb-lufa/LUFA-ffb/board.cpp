#include "board.h"

#include "DigitalPin.h"
#include "GamePort.h"
#include "lufa-iface.h"

#if  defined(DEBUG_ENABLE_UART)  ||  defined(MIDI_USES_HARDWARESERIAL)
#include <HardwareSerial.h>
#else
#endif

DigitalOutput<GamePort<3>::pin> x1;
DigitalOutput<GamePort<11>::pin> x2;
// pins for FFPro joystick
//DigitalOutput<GamePort<6>::pin> y1;
//DigitalOutput<GamePort<13>::pin> y2;

#ifdef USE_SOFTWARESERIAL
#include <SoftwareSerial.h>
SoftwareSerial SoftSerial = SoftwareSerial(rxPin, txPin, false);
#endif

#ifdef ENABLE_CDC
#include "LUFACDCSerial.h"
LUFACDCSerial  SerialCDC( &Serial1_CDC_Interface );
#endif

#ifdef __cplusplus
  extern "C" {
#endif
  
int  board_ffb_putc(uint8_t byte)
{
  return __board_midi_putc(byte);
}

void  board_uart_putc(uint8_t byte)
{
  __board_uart_putc(byte);
}

void board_uart_poll( void (*callback)(uint8_t c) )
{
#ifdef DEBUG_ENABLE_UART
  uint8_t c;
  
  if (DEBUG_INTERFACE.available()) {
    c = DEBUG_INTERFACE.read();  
    callback(c); 
  }
#endif
}

void board_init(void)
{
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();
  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  LEDs_Init();
  
  // fast flickering to detect a reboot
  for (uint8_t i = 0; i < 5; i++) {
      LEDs_ToggleLEDs(LEDS_LED1);
      delay(50);
      LEDs_ToggleLEDs(LEDS_LED1);
      delay(100);
  }
  GlobalInterruptEnable();

  MIDI_INTERFACE.begin(MIDI_BAUD);

#ifdef USE_SOFTWARESERIAL
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
#endif

#ifdef DEBUG_ENABLE_UART
  DEBUG_INTERFACE.begin(115200);
  DEBUG_INTERFACE.println("Serial1:board_init");
#endif

#ifdef ENABLE_CDC
  SerialCDC.begin(9600);
#endif
}

void pull_trigger(void)
{
  __board_pull_trigger();
}

void PulseAndWait ( uint16_t dly_us )
{
    pull_trigger ();
    delayMicroseconds(dly_us);
}

#ifdef __cplusplus
  } 
#endif
