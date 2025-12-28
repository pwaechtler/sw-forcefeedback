/*
	Copyright 2013  Saku Kekkonen

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _FFB_WHEEL_
#define _FFB_WHEEL_

#include <stdint.h>
#include "ffb.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct
{
  uint8_t   cmd;    // f2
  uint8_t   operation_and_checksum;
  uint8_t   effect_id;
} cmd_f2_t;

typedef struct
{
  uint8_t   cmd;    // f1
  uint8_t   checksum;
  uint8_t   flag_address;
  uint8_t   effect_id;
  uint16_t  value;
} cmd_f1_t;

typedef struct
{
	uint8_t		command; 	// always 0x20, to be pedantic: its an address, the cmd is the last byte of sysex_header
	uint8_t 	effect_type;
	uint8_t 	unknown;	// always 0x7f
	
	uint16_t	duration;   // offset 0   field 1 or 0?
	uint8_t		direction;  // offset 2   field 2
} cmd_f0_common_t;

/* type for sine, square, triangle, sawtooth and ramp */
typedef struct
{
	cmd_f0_common_t	common;
	
	uint8_t		precise_dir;    //3       field 3
	uint16_t	p_x_offset;     // 4+5    field 4
  uint8_t   e_y1;           // 6      field 5
  uint16_t  e_x1;           // 7+8    field 6
	uint8_t		p_amplitude;    // 9      field 7
  uint16_t  e_x2;           // a+b    field 8
  uint8_t   e_y2;           // c      field 9
	uint16_t	p_t;            // d+e    field a
	 int8_t		p_y_offset;     // f      field b
} cmd_f0_wave_t;

typedef struct
{
	cmd_f0_common_t	common;
	uint8_t		precise_dir;  // 3    field 3
	uint8_t		e_y1;  // offset  4   field 4
	uint16_t 	e_x1;  // offset  5+6 field 5
	uint8_t 	force; // offset  7   field 6
	uint16_t 	e_x2;  // offset 8+9  field 7
	uint8_t 	e_y2;  // a           field 8
	uint8_t 	force_direction; // b field 9
} cmd_f0_constant_force_t;

// it's only the begining of a <cmd_f0_condition> , only send one extra byte, otherwise error
typedef struct
{
  cmd_f0_common_t common;
  int8_t  posCoeff;       // field 3
} cmd_f0_friction_t;

/*
 * a change on fedit/Spring/Properties/Deadband slider pokes on 0x43, 0x44, 0x45 and 0x06
 * it subtract values from Coeff and Saturation
 * maybe we have to rotate the effect and calculate everything ourself
 */
typedef struct
{
  cmd_f0_common_t common;
  uint8_t  posCoeff;
  uint8_t  negSat;  // 7d

  uint8_t  deadBand1;// 3e
  uint8_t  negCoeff; // 3f spring: 2f
  uint8_t  deadBand2;// 3e

  uint8_t  magic;   // 3f spring: 2f
  uint8_t  posSat;  // 7d
  uint8_t  cpOffset;  // 0
} cmd_f0_condition;// 0xd=Spring, 0x0e=Damper, 0xf=Inertia

#if 0
// Spring does not work with that, not sure if damper,inertia is different
typedef struct
{
  cmd_f0_common_t common;
  int8_t   cpOffset;    // center point offset      
  uint8_t  posSaturation;
  uint8_t  negSaturation;
  int16_t  posCoeff;
  int16_t  negCoeff;
  int8_t   deadBand;
} cmd_f0_spring_t;
#endif

void Ffbwheel_EnableFFB(void);
const uint8_t* Ffbwheel_GetSysExHeader(uint8_t* hdr_len);
uint8_t Ffbwheel_mapToDevEffectType(uint8_t drv_effect_type);

void Ffbwheel_SetAutoCenter(uint8_t enable);
void Ffbwheel_DeviceForceGain(uint8_t gain);

void Ffbwheel_startEffect(uint8_t effectId);
void Ffbwheel_stopEffect(uint8_t effectId);
void Ffbwheel_freeEffect(uint8_t effectId);

void Ffbwheel_CreateEffect(const void *buf);

void Ffbwheel_SetEnvelope(const void *buf);
void Ffbwheel_SetCondition(const void *buf);
void Ffbwheel_SetPeriodic(const void *buf);
void Ffbwheel_SetConstantForce(const void *buf);
void Ffbwheel_SetRampForce(const void *buf);
int  Ffbwheel_SetEffect(const void *buf);

void Ffbwheel_adjustForceGain(uint8_t level);
void Ffbwheel_adjustCenterSpring(uint8_t level);

#ifdef __cplusplus
  }
#endif

#endif // _FFB_WHEEL_
