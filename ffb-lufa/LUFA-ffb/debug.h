/*
  Basic debugging utilities.

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

#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdint.h>
#include "lufa-iface.h"

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DEBUG_NONE             0 
#define DEBUG_CREATE_EFFECT 0x01
#define DEBUG_LOG_EFFECT    0x02
#define DEBUG_LOG_MIDI      0x04
#define DEBUG_LOG_PID       0x08
#define DEBUG_LOG_STARTSTOP 0x10
#define DEBUG_LOG_COND      0x20
#define DEBUG_DATALINK      0x40
#define DEBUG_DETAIL        0x80
#define DEBUG_ALL           0xFF 

//TODO: changeable on runtime when compiled in
#define LOG_DEST_UART       0x01
#define LOG_DEST_CDC        0x02
extern uint8_t gLogDest;

extern void __dbg_ndebug();
extern uint8_t gLogLevel;
extern uint8_t gDebugMeasure;
extern uint16_t gDebugMeasureInterval;
  
#ifdef NDEBUG

#define NOTHING  do {} while (0)
#define log_P_nonl(FMT,...)  __dbg_ndebug()
#define log_P(FMT,...)       __dbg_ndebug()
#define log_buf_PP(fmt, pgm_data, len) __dbg_ndebug()
#define log_buf_P(fmt, data, len)  __dbg_ndebug()

#else

#endif

void printRamp(int8_t start, int8_t end);
void printForce(uint8_t force, uint8_t force_dir, uint8_t precise_dir);
void printEnvelope (uint8_t y1, uint8_t y2, uint16_t t1, uint16_t t2);
void printPeriodic(uint8_t p_amp, uint16_t p_t, uint8_t p_x_offset, int8_t p_y_offset);
void printCondition (int8_t posCoeff, uint8_t posSat, int8_t cpOffset, int8_t negCoeff, uint8_t negSat, uint8_t deadBand);
void printFriction(uint8_t friction, uint8_t gain);
void printUnsupported(uint8_t value);
void printDeviceConfig();


// Returns true if debug settings contain one of the given attributes
bool isLogLevel(const uint8_t type);

void debugTask();

void log_P(const char *fmt, ...);
void log_P_nonl(const char *fmt, ...);
// logs a buffer with a prefix of text in PROGMEM
void log_buf_P(const char *text, const void *data, uint8_t len);
// logs a buffer in PROGMEM with a prefix of text in PROGMEM
void log_buf_PP(const char *txt, const void *pgm_data, uint8_t len);
void log_buf(uint8_t *data, uint8_t len);
void log_crlf();
void error_P(const char *fmt, ...);
void error_P_nonl(const char *fmt, ...);
void error_loop(uint8_t errorcode);

// Log all reports found in the given data (may have one or more)
// The <text> must point to string in program memory.
void LogReportP(const char *text, const uint16_t *reportSizeArray, uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif
#endif // _DEBUG_H_
