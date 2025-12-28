/*
  Force Feedback Joystick
  Basic debugging utilities.

  This code is for Microsoft Sidewinder Force Feedback Pro joystick
  with some room for additional extra controls.

  Copyright 2022  peewee.hannover [at] gmail [dot] com
  Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
  MIT License.

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/
#include <stdbool.h>
#include "debug.h"
#include "ffb.h"
#include "ffb-wheel.h"

#include "lufa-iface.h" // for _dbg_dumpReport

char printf_buffer[84];
// beware stack size

uint8_t gLogLevel = 0
| DEBUG_CREATE_EFFECT
// | DEBUG_LOG_EFFECT
// | DEBUG_LOG_STARTSTOP
// | DEBUG_LOG_MIDI
 | DEBUG_LOG_PID
// | DEBUG_DATALINK
// | DEBUG_LOG_COND
// | DEBUG_DETAIL
//| DEBUG_ALL
;

#ifdef  ENABLE_SERIAL_DEBUG_CMDS
uint8_t gDebugMeasure;
#else
uint8_t gDebugMeasure = 1;
#endif
uint16_t gDebugMeasureInterval = 6000; // 6 seconds

#ifdef NDEBUG
bool isLogLevel(const uint8_t level) {
  return ((gLogLevel & level) == level);
//  return true;
}

#else
bool isLogLevel(const uint8_t level)
{
  return ((gLogLevel & level) == level);
}
#endif

static const PROGMEM char buf_hex_fmt_str[] = " %02x";
void writeCDC(uint8_t *p, uint16_t len);

void LogString()
{
  uint16_t n;
  uint8_t c;
  for (n = 0; (c = printf_buffer[n]) && (n < sizeof(printf_buffer)); n++)
  {
#ifdef DEBUG_ENABLE_UART
    board_uart_putc(c);
#endif // DEBUG_ENABLE_UART
  }

#ifdef DEBUG_ENABLE_USB
    writeCDC((uint8_t*) printf_buffer, n);
#endif

}

void log_crlf()
{
  printf_buffer[0] = '\r';
  printf_buffer[1] = '\n';
  printf_buffer[2] = '\0';  
  LogString();
}


#ifdef NDEBUG
void __dbg_ndebug() {};
#else

// the fmt pointer is an offset into flash/PROGMEM area
void log_P_nonl(const char *fmt_P, ...) {
  va_list args;
  va_start(args, fmt_P);
  vsnprintf_P(printf_buffer, sizeof(printf_buffer), fmt_P, args);
  va_end(args);
  LogString(printf_buffer);
}

void log_P(const char *fmt_P, ...) {
  va_list args;
  va_start(args, fmt_P);
  vsnprintf_P(printf_buffer, sizeof(printf_buffer), fmt_P, args);
  va_end(args);
  LogString(printf_buffer);
  log_crlf();
}

void log_buf_P(const char *text, const void *buf, uint8_t len)
{
  const uint8_t *data = buf;
  log_P_nonl(text);
  for (uint8_t i = 0; i < len; i++) {
    log_P_nonl(buf_hex_fmt_str, data[i]);
  }
  log_crlf();
}

void log_buf_PP(const char *text, const void *buf, uint8_t len)
{
  const uint8_t *data = buf;
  log_P_nonl(text);
  for (uint8_t i = 0; i < len; i++) {
    uint8_t b = pgm_read_byte( &data[i]);
    log_P_nonl(buf_hex_fmt_str, b);
  }
  log_crlf();
}
#endif

/* even on NDEBUG, errors/debug cmds via COM shall be logged */
void log_buf(uint8_t *data, uint8_t len)
{
  for (uint8_t i = 0; i < len; i++) {
    log_P_nonl(buf_hex_fmt_str, data[i]);
  }
  log_crlf();
}

void error_P(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf_P(printf_buffer, sizeof(printf_buffer), fmt, args);
  va_end(args);
  LogString(printf_buffer);
  log_crlf();
}

void error_P_nonl(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf_P(printf_buffer, sizeof(printf_buffer), fmt, args);
  va_end(args);
  LogString(printf_buffer);
}

void error_loop(uint8_t e)
{
  uint8_t a, n;
  while (1)  {
    for (a = 0; a < 50; a++) {
      LEDs_ToggleLEDs(LEDS_LED1);
      _delay_ms(70);
    }
    LEDs_SetAllLEDs(LEDS_NO_LEDS);
    // after long flickering you can count the "errorcode"
    for (n = e; n ; n--) {
      LEDs_SetAllLEDs(LEDS_ALL_LEDS);
      _delay_ms(500);
      LEDs_SetAllLEDs(LEDS_NO_LEDS);
      _delay_ms(500);
    }
  }
}


// Log all reports found in the given data (may have one or more)
void LogReportP(const char *text, const uint16_t *reportSizeArray, uint8_t *data, uint16_t len)
{
  uint8_t *p = data;
  log_P_nonl(text);

  while (p < data + len)
  {
    uint8_t replen = reportSizeArray[p[0] - 1];
    log_P_nonl(PSTR("<%d>:"), p[0]);
    log_buf(p + 1, replen - 1);
    p += replen;
    log_crlf();
  }
}

void printUnsupported(uint8_t value)
{
  static const char PROGMEM unsupported_fmt_str[] = " unsupported: %u";
  error_P(unsupported_fmt_str, value);
}

void printRamp(int8_t start, int8_t end)
{
  static const char PROGMEM ramp_fmt_str[] = "\tstart: %d end: %d";
  error_P(ramp_fmt_str, start, end);
}

void printEnvelope (uint8_t y1, uint8_t y2, uint16_t t1, uint16_t t2)
{
  static const char PROGMEM envelope_fmt_str[] = "\t y1: %3u y2: %3u \ttx1:%5u  tx2:%5u";
  error_P(envelope_fmt_str, y1, y2, t1, t2);
}

void printPeriodic(uint8_t p_amp, uint16_t p_t, uint8_t p_x_offset, int8_t p_y_offset)
{
  static const char PROGMEM periodic_fmt_str[] = "\tamp: %3u p_t: %5u  x_off: %5u  y_off: %3d";
  error_P(periodic_fmt_str, p_amp, p_t, p_x_offset, p_y_offset);
}

void printForce(uint8_t force, uint8_t force_dir, uint8_t precise_dir) 
{
  static const char PROGMEM constant_fmt_str[] = "\tforce: %3u\tf_dir: %3u (%c)\t";
  error_P_nonl(constant_fmt_str, force, force_dir, precise_dir == FFB_DIR_PRECISE ? 'P' : 'u');
}

void printFriction(uint8_t friction, uint8_t gain)
{
  static const char PROGMEM friction_fmt_str[] = "\tfriction: %u %%\t gain: %3u";
  error_P(friction_fmt_str, friction, gain);
}

void printCondition (int8_t posCoeff, uint8_t posSat, int8_t cpOffset, int8_t negCoeff, uint8_t negSat, uint8_t deadBand)
{
  static const char PROGMEM cond_param_fmt1 [] = "\t posCoeff: %4u\tposSat: %3u\tcpOffset: %d";
  static const char PROGMEM cond_param_fmt2 [] = "\t negCoeff: %4u\tnegSat: %3u\tdeadBand: %d";
  error_P(cond_param_fmt1, posCoeff, posSat, cpOffset);
  error_P(cond_param_fmt2, negCoeff, negSat, deadBand);
}

void printDeviceConfig()
{
  char *empty ="";
  char *cdc = empty;
  char *ndebug = empty;
  char *midi_uses = empty;
  uint8_t var_on_stack = 0;
#ifdef SUPPORT_CUSTOM_FORCE
  var_on_stack = 0xce;
#endif
#ifdef ENABLE_CDC
  cdc = "CDC";
#endif
#ifdef NDEBUG
  ndebug = "NDEBUG";
#endif
#ifdef MIDI_USES_HARDWARESERIAL
  midi_uses = "MIDI_HWSERIAL";
#endif
  gDevice.pool.freeMem = (uint16_t ) (&var_on_stack) - 128 - (uint16_t)__malloc_heap_start;
  
  error_P(PSTR("Device configured; %s %s %s %s"), cdc, ndebug, midi_uses,
      var_on_stack == 0xce ? "Custom_FORCE": empty);
  error_P(PSTR("heap (s:%p) stack: %p: free mem: %u"), 
      __malloc_heap_start, &var_on_stack, gDevice.pool.freeMem);
  error_P(PSTR("LogLevel: 0x%02X  more_friction: %u maxSentBytes(MIDI): %u"), gLogLevel, gDevice.car.more_friction, gDevice.maxSentBytes);
}

#ifdef ENABLE_CDC

void writeCDC(uint8_t *buf, uint16_t len)
{
  if (USB_DeviceState != DEVICE_STATE_Configured)
    return;
  uint8_t ep = Endpoint_GetCurrentEndpoint();
#if 0  // this tests if USB interface is configured/setup but does not work?
  CDC_Device_SendData(&Serial1_CDC_Interface, buf, len);
  CDC_Device_Flush(&Serial1_CDC_Interface);
#else
  // Select the Serial Tx Endpoint
  Endpoint_SelectEndpoint(CDC1_TX_EPNUM);
  // Write the String to the Endpoint
  Endpoint_Write_Stream_LE(buf, len, NULL);
  // Finalize the stream transfer to send the last packet
  Endpoint_ClearIN();
  // Wait until the endpoint is ready for another packet
  Endpoint_WaitUntilReady();
  // Send an empty packet to ensure that the host does not buffer data sent to it
  Endpoint_ClearIN();
#endif
  Endpoint_SelectEndpoint(ep);
}

#endif //ENABLE_CDC




#ifdef ENABLE_SERIAL_DEBUG_CMDS
/*  
  It is used for debugging and allows controlling the force feedback effects via
  serial line too.

  Commands:
    "l"
      List infos of all effects

    "e" toggles DEBUG_LOG_EFFECT
    "m" toggles DEBUG_LOG_MIDI
    
    "c"
      prints important values for car racing (force/friction) in intervals
    "r"
      prints the report data in intervals

    "d" SETTING
      Enable the given debug setting. Settings are cumulative:

    "D" SETTING
      Disable the given debug setting.

    + <friction>
      add more friction

    T <eid>
      remove the Traced flag

    t <eid>
      set Traced flag on the eid so that its dumped in intervals, e.g T0102

    i 0x <interval in ms>
      set the tracing interval, e.g. T 02 1000 sets the interval to 1000ms (2 hexadecimal bytes)

    "M" ...data...
      Send given data directly to joystick's MIDI channel. 

      e.g. "M A5 7F 00" will send bytes "A5 7F 00" as binary to joystick.

    "U" ...data...
      Process the given data as if it came as a force feedback report via USB.
      LENGTH is the number of bytes in the data.

      e.g. "U 0D FF" will send bytes "0D FF" as binary to adapter's FFB data
      processing. This example would trigger DeviceGain-report (id=0x0D) with one
      byte parameter 0xFF.
*/
void _dbg_listEffects(bool all);
void _dbg_setLogLevel(uint8_t command, uint8_t value);
void _dbg_sendFfb(uint8_t *data, uint8_t len);
void _dbg_injectSetReport(uint8_t *data, uint8_t len);
void _dbg_setTraceInterval(uint8_t *data, uint8_t len);
void _dbg_setTraceFlag(uint8_t cmd, uint8_t *data, uint8_t len);
void _dbg_adjustCenterSpring(uint8_t *data, uint8_t len);
void _dbg_adjustForceGain(uint8_t *data, uint8_t len);

void _dbg_dumpReport(uint8_t reportID);
void _dbg_dumpEffect(uint8_t eid, struct paramEffectBlock *peb);
void _dbg_dumpCar();    // force, force_dir and friction

void _dbg_command(uint8_t command, uint8_t *data, uint8_t len);
void _dbg_getc(uint8_t data);

static uint16_t _dbg_TraceInterval = 175;
static uint8_t _dbg_dumpReportFlag;
static uint8_t _dbg_dumpCarFlag;

static uint8_t _dbg_input[20];
static const PROGMEM uint8_t _dbg_valid_oneLetter[] = "?celmrz";
static const PROGMEM uint8_t _dbg_valid_Cmd[] = "+!CdDFiMtTU";

void _dbg_command(uint8_t command, uint8_t *data, uint8_t len)
{
  error_P_nonl(PSTR("dbgCmd:='%c' data:"), command);
  log_buf(data, len);

  switch (command) {
    case '?':
      printDeviceConfig(); break;
    case '+':
      gDevice.car.more_friction = data[0]; break;
    case '!':
      ffb_handleDeviceControl(data); break;
    case 'C':
#ifdef SUPPORT_ADJUST_CENTER_SPRING
      _dbg_adjustCenterSpring(data, len);
#endif
        break;
    case 'c':
      // toggle dumpCarFlag
      _dbg_dumpCarFlag = _dbg_dumpCarFlag ? 0 : 1; break;
    case 'd':
    case 'D':
      _dbg_setLogLevel(command, data[0]); break;
    case 'e':
      if (isLogLevel(DEBUG_LOG_EFFECT))
        gLogLevel &= ~DEBUG_LOG_EFFECT;
      else
        gLogLevel |= DEBUG_LOG_EFFECT;
      break;
    case 'F':
#ifdef    SUPPORT_ADJUST_FORCE_GAIN
      _dbg_adjustForceGain(data, len);
#endif
      break;
    case 'l':
      _dbg_listEffects(true); break;
    case 'i':
      _dbg_setTraceInterval(data, len); break;
    case 'm':
      if (isLogLevel(DEBUG_LOG_MIDI))
        gLogLevel &= ~DEBUG_LOG_MIDI;
      else
        gLogLevel |= DEBUG_LOG_MIDI;
      break;
    case 'M':
      _dbg_sendFfb((uint8_t*) data, len); break;
    case 'r':
      // toggle dumpReportFlag
      _dbg_dumpReportFlag = _dbg_dumpReportFlag ? 0 : 1; break;
    case 't':
    case 'T':
      _dbg_setTraceFlag(command, data, len); break;
    case 'U':
      _dbg_injectSetReport((uint8_t*) data, len); break;
    case 'z':
      gDebugMeasure = gDebugMeasure ? 0 : 1;
      gDevice.setEffectCount = 0;
      gDevice.sentBytes = 0;
      break;
    default:
      {
        error_P(PSTR("unknown command"));
      }
  }
}

uint8_t parseHexNibble(char data)
{
  if (data >= '0' && data <= '9')
    return data - '0';
  else if (data >= 'a' && data <= 'f')
    return (data - 'a') + 10;
  else if (data >= 'A' && data <= 'F')
    return (data - 'A') + 10;
  else
    return 0xFF; // not a hex nibble
}

// this code does not rely on data count
// just type i6400<CR> for setting interval to 0x0064-> 100ms
uint8_t _dbg_rewriteBuf(uint8_t len)
{
  uint8_t rwi = 0;
  uint8_t highn, lown;

  // skip first
  for (uint8_t i = 1; i < len; i++) {
    highn = parseHexNibble(_dbg_input[i]);
    if ( highn != 0xFF ) {
      lown = parseHexNibble(_dbg_input[i + 1]);
      if ( lown != 0xFF ) {
        _dbg_input[rwi++] = (highn << 4) + lown;;
      }
      i++;  // another one was consumed
    }
  }
  return rwi;
}

bool _dbg_isCmd(const uint8_t *set, uint8_t c)
{
  uint8_t ret;
  uint8_t i = 0;

  while (1) {
    ret = pgm_read_byte(&set[i]);
    if ( (ret == c) || !ret) {
      break;
    }
    i++;
  }
  return ret != 0;
}

#ifdef ENABLE_CDC
uint8_t readCDC()
{
  uint8_t len = 0;
  uint8_t ep = Endpoint_GetCurrentEndpoint();
  /* Select the Serial Rx Endpoint */
  Endpoint_SelectEndpoint(CDC1_RX_EPNUM);

  if (Endpoint_IsOUTReceived()) {
    while (Endpoint_BytesInEndpoint() && len < sizeof(_dbg_input)) {
      LEDs_ToggleLEDs(LEDS_LED2);
      while (Endpoint_Read_Stream_LE(&_dbg_input[len++], 1, NULL)
             == ENDPOINT_RWSTREAM_IncompleteTransfer)
      { // busy loop until all bytes are read in
        LEDs_ToggleLEDs(LEDS_LED2);
      }
    }
    Endpoint_ClearOUT();
  }
  Endpoint_SelectEndpoint(ep);
  return len;  
}

void CDC1_readTask()
{
  CDC_Device_USBTask(&Serial1_CDC_Interface);

  /* Device must be connected and configured for the task to run */
  if (USB_DeviceState != DEVICE_STATE_Configured)
    return;
  
  uint8_t len = readCDC();
  if (len) {
    if (_dbg_isCmd(_dbg_valid_oneLetter, _dbg_input[0]) || _dbg_isCmd(_dbg_valid_Cmd, _dbg_input[0])) {
      uint8_t cmd = _dbg_input[0];
      len = _dbg_rewriteBuf(len);
      _dbg_command(cmd, _dbg_input, len);
    }
    LEDs_ToggleLEDs(LEDS_LED2);
  }
}
#endif


void _dbg_getc(uint8_t input)
{
  static uint8_t linepos = 0;
  uint8_t len;

  if (!linepos && _dbg_isCmd(_dbg_valid_oneLetter, input)) {
    _dbg_command(input, _dbg_input, 0);  // you can poke the cmd into _dbg_line if you insist
    return;
  }
  _dbg_input[linepos++] = input;

  if (input == '\r') {
    if (_dbg_isCmd(_dbg_valid_Cmd, _dbg_input[0])) {
      uint8_t cmd = _dbg_input[0];
      len = _dbg_rewriteBuf(linepos);
      _dbg_command(cmd, _dbg_input, len);
    }
    linepos = 0;
  }
  if (linepos == sizeof(_dbg_input)) {
    error_P(PSTR("dbg: truncated input"));
    //_dbg_command(_dbg_input[0], _dbg_input, linepos);  // you can choose if you process truncated input
    linepos = 0;
  }

}

void printParams(uint8_t eid)
{
  struct paramEffectBlock *peb = getEffectBlock(eid);

  switch (peb->effectType) {
    case FFB_EFFECT_SQUARE:
    case FFB_EFFECT_SINE:
    case FFB_EFFECT_TRIANGLE:
    case FFB_EFFECT_SAWTOOTHDOWN:
    case FFB_EFFECT_SAWTOOTHUP:
    case FFB_EFFECT_RAMP: {
        struct periodic *p = getParamBlock(peb, 0);
        struct envelope *e = getParamBlock(peb, 1);
        printPeriodic(p->amplitude, p->period, p->phase, p->offset);
        if (peb->state & HasEnvelope)
          printEnvelope (e->attackLevel, e->fadeLevel, e->attackTime, e->fadeTime);
      } break;
    case FFB_EFFECT_CONSTANT: {
        struct constant *c = getParamBlock(peb, 0);
        struct envelope *e = getParamBlock(peb, 1);
        uint8_t force = calcForce(c->magnitude, peb->gain);
        error_P_nonl(PSTR("\tmag: %5d "), c->magnitude);
        printForce(force, peb->directionX, FFB_DIR_PRECISE);
        log_crlf();
        if (peb->state & HasEnvelope)
          printEnvelope (e->attackLevel, e->fadeLevel, e->attackTime, e->fadeTime);
      } break;
    case FFB_EFFECT_INERTIA:
    case FFB_EFFECT_DAMPER:
    case FFB_EFFECT_SPRING: {
        struct condition *c = getParamBlock(peb, 0);
        struct envelope *e = getParamBlock(peb, 1);
        printCondition(c->positiveCoefficient, c->positiveSaturation, c->cpOffset, 
                       c->negativeCoefficient, c->negativeSaturation, c->deadBand);
        if (peb->state & HasEnvelope)
          printEnvelope (e->attackLevel, e->fadeLevel, e->attackTime, e->fadeTime);
      } break;
    case FFB_EFFECT_FRICTION: {
        struct condition *c = getParamBlock(peb, 0);
        printFriction(c->positiveCoefficient, peb->gain);
      } break;
    default:
      printUnsupported(peb->effectType);
  }
}

void _dbg_dumpEffect(uint8_t eid, struct paramEffectBlock *peb)
{
  error_P_nonl(PSTR("%.2d: "), eid);
  if (peb->state & Allocated)
    printEffectType(peb->effectType);

  if (peb->state == Free) {
    error_P(PSTR(" Free"));
  } else if (peb->state) {
    error_P_nonl(PSTR(" status[0x%x]"), peb->state);
    if (peb->state & SentToDevice)
      error_P_nonl(PSTR(" Sent"));

    if (peb->state & Playing)
      error_P_nonl(PSTR(" Playing"));

    error_P(PSTR("\tdur: %u \tgain: %u sTime: %lu "), peb->duration, peb->gain, peb->startStopTime);

    printParams(eid);
  }
}

void _dbg_dumpReport(uint8_t reportID)
{
  struct WheelReport wheel;
  struct SimulationControlsReport simCtl;
  uint16_t len;

  len = Joystick_CreateReport(reportID, &wheel);
  if (len) {
    error_P_nonl(PSTR("X: %.4u Y: %.4u Z: %.4u B: 0x%02X "),
               wheel.X, wheel.Y, wheel.Z, wheel.Button );
  }
  len = Joystick_CreateReport(2, &simCtl);
  if (len) {
    error_P(PSTR("\tst: %4d acc: %.3u brake: %.3u"),
          simCtl.steering, simCtl.accelerator, simCtl.brake);
  }
}

void _dbg_dumpCar()
{
  printForce( gDevice.car.force, gDevice.car.force_dir, gDevice.car.precise);
  printFriction( gDevice.car.friction, gDevice.car.friction_gain);
}

void _dbg_listEffects(bool all)
{
  uint8_t eid = 0;
  if (all)
    log_P(PSTR("EffectTable:"));

  for (eid = 2; eid < MAX_EFFECTS; eid++) {
    struct paramEffectBlock *peb = getEffectBlock(eid);

    if (all || (peb->state & Traced)) {
      _dbg_dumpEffect(eid, peb);
    }
  }
}

#ifdef SUPPORT_ADJUST_FORCE_GAIN
void _dbg_adjustForceGain(uint8_t *data, uint8_t len)
{
  uint8_t level = *data;

  if (len > 2)
    return;
  if ( level > 3)
    return;
  if (sw_id == SW_ID_WHEEL)
    Ffbwheel_adjustForceGain(level);  
}
#endif

#ifdef SUPPORT_ADJUST_CENTER_SPRING
void _dbg_adjustCenterSpring(uint8_t *data, uint8_t len)
{
  uint8_t level = *data;

  if (len > 2)
    return;
  if ( level > 3)
    return;
  if (sw_id == SW_ID_WHEEL)
    Ffbwheel_adjustCenterSpring(level);
}
#endif

void _dbg_setLogLevel(uint8_t command, uint8_t value)
{
  if (command == 'D')
    gLogLevel = gLogLevel & (~value);
  else
    gLogLevel |= value;
}

void _dbg_setTraceInterval(uint8_t *data, uint8_t len)
{
  uint16_t n = * ((uint16_t*)data);

  if (len > 4)
    return;
  if ( n < 100)
    return;
  _dbg_TraceInterval = n;
}

void _dbg_setTraceFlag(uint8_t cmd, uint8_t *data, uint8_t len)
{
  uint8_t eid = data[0];
  if (!checkBlockIndex(eid) || !len )
    return;

  struct paramEffectBlock *peb = getEffectBlock(eid);
  if ( cmd == 'T')
    peb->state &= ~Traced;
  else
    peb->state |= Traced;
}

void _dbg_sendFfb(uint8_t *data, uint8_t len)
{
  ffb_sendData(data, len);
}

void _dbg_injectSetReport(uint8_t *data, uint8_t len)
{
  ffb_handleSetReport(data[0], data, len);
}


#endif // ENABLE_SERIAL_DEBUG_CMDS


void debugTask()
{
  static unsigned long last_poll;
  static unsigned long last_measure;
  unsigned long ts = millis();

#ifdef ENABLE_SERIAL_DEBUG_CMDS

#ifdef DEBUG_ENABLE_UART
  board_uart_poll( _dbg_getc );  // reads from HW Serial1
#endif

#ifdef ENABLE_CDC
  CDC1_readTask();
#endif

  /*  */
  if ( (ts - last_poll) >= _dbg_TraceInterval) {
    // dump "Traced" marked effects
    _dbg_listEffects(false);

    if (_dbg_dumpReportFlag)
      _dbg_dumpReport(1);

    if (_dbg_dumpCarFlag)
      _dbg_dumpCar();

    last_poll = ts;
  }
#endif  //ENABLE_SERIAL_DEBUG_CMDS

  if ( (ts - last_measure) >= gDebugMeasureInterval) {
    if (gDebugMeasure) {
      error_P(PSTR("num SetEffects: %u sentBytes: %u interval: %u ms"), gDevice.setEffectCount, gDevice.sentBytes, gDebugMeasureInterval);
    }
    if (gDevice.sentBytes > gDevice.maxSentBytes) {
      gDevice.maxSentBytes = gDevice.sentBytes;
    }
    gDevice.sentBytes = gDevice.setEffectCount = 0;
    last_measure = ts;
  }
}
