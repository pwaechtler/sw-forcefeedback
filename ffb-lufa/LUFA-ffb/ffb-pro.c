/*
  Joystick model specific code for handling force feedback data.
  This code is for Microsoft Sidewinder Force Feedback Pro joystick.

  Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
  Copyright 2013  Saku Kekkonen

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
#include "ffb-pro.h"
#include "ffb.h"

#include <util/delay.h>
#include "debug.h"
#include "board.h"

static const PROGMEM uint8_t _sysex_header[] = {0xf0, 0x00, 0x01, 0x0a, 0x01};
static const PROGMEM uint8_t ac_disable[] = {
  0xb5, 0x7c, 0x7f,   0xa5, 0x7f, 0x00,
  0xc5, 0x06,
};
static const PROGMEM uint8_t ac_enable[] = {
    0xc5, 0x01
  };


uint8_t Ffbpro_UsbToMidiEffectType(uint8_t drv_effect_type)
{
	static const PROGMEM uint8_t drvToMidiEffectType[] = {
		0x12,	// Constant, 
		0x06, 	// Ramp
		0x05, 	// Square
		0x02, 	// Sine
		0x08,	// Triangle
		0x0A,	// SawtoothDown
		0x0B,	// SawtoothUp
		0x0D,	// Spring
		0x0E,	// Damper
		0x0F,	// Inertia
		0x10,	// Friction
		0x01 	// Custom ?
	};
	
	if (drv_effect_type >= sizeof(drvToMidiEffectType))
		return 0;
		
	return pgm_read_byte( &drvToMidiEffectType[drv_effect_type]);
}

static void Ffbpro_InitPulses(uint8_t count)
{
	while (count--) {
		pull_trigger();
		_delay_us10(10);
	}
}

void Ffbpro_EnableFFB(void)
{
	static const PROGMEM uint8_t startupFfbData_0[] = {
		0xc5, 0x01        // <ProgramChange> 0x01
	};

	static const PROGMEM uint8_t startupFfbData_1[] = {
		0xf0,
		0x00, 0x01, 0x0a, 0x01, 0x10, 0x05,  // ???? - reset all?
		0x6b, 0xf7
	};

	static const uint8_t PROGMEM startupFfbData_2[] = {
    // <ControlChange>(Modify, 0x7f)  offset 0x72 := 0x57
		0xb5, 0x40, 0x7f,  		            0xa5, 0x72, 0x57,
		0xb5, 0x44, 0x7f, 		            0xa5, 0x3c, 0x43,
		0xb5, 0x48, 0x7f,                 0xa5, 0x7e, 0x00,
		0xb5, 0x4c, 0x7f, 		            0xa5, 0x04, 0x00,
		0xb5, 0x50, 0x7f, 		            0xa5, 0x02, 0x00,
		0xb5, 0x54, 0x7f, 		            0xa5, 0x02, 0x00,
		0xb5, 0x58, 0x7f, 		            0xa5, 0x00, 0x7e,
		0xb5, 0x5c, 0x7f, 		            0xa5, 0x3c, 0x00,
		0xb5, 0x60, 0x7f,              		0xa5, 0x14, 0x65,
		0xb5, 0x64, 0x7f,              		0xa5, 0x7e, 0x6b,
		0xb5, 0x68, 0x7f,             		0xa5, 0x36, 0x00,
		0xb5, 0x6c, 0x7f,             		0xa5, 0x28, 0x00,
		0xb5, 0x70, 0x7f,             		0xa5, 0x66, 0x4c,
		0xb5, 0x74, 0x7f,             		0xa5, 0x7e, 0x01
	};

    waitMs(100);
	pull_trigger();
    waitMs(7);
	Ffbpro_InitPulses(4);
	waitMs(35);
	Ffbpro_InitPulses(3);
	waitMs(14);
	Ffbpro_InitPulses(2);
	waitMs(78);
	Ffbpro_InitPulses(2);
    waitMs(4);
	Ffbpro_InitPulses(3);
	waitMs(59);

	Ffbpro_InitPulses(2);
	
	// -- START MIDI
	ffb_sendData_P(startupFfbData_0, sizeof(startupFfbData_0));	// Program change
	waitMs(20);
	ffb_sendData_P(startupFfbData_1, sizeof(startupFfbData_1));	// Init
	waitMs(57);
	ffb_sendData_P(startupFfbData_2, sizeof(startupFfbData_2));	// Initialize effects data memory
  Ffbpro_SetAutoCenter(0);
	waitMs(70);
	}

void Ffbpro_SetAutoCenter(uint8_t enable)
{
	
	ffb_sendData_P(ac_enable, sizeof(ac_enable));
	if (!enable) {
		waitMs(70);
		ffb_sendData_P(ac_disable, sizeof(ac_disable));
	}
}

const uint8_t* Ffbpro_GetSysExHeader(uint8_t* hdr_len)
{
	*hdr_len = sizeof(_sysex_header);
	return _sysex_header;
}

void Ffbpro_DeviceForceGain(uint8_t gain)
{
  gDevice.gainReport.gain = gain;
}

// effect operations ---------------------------------------------------------

static void Ffbpro_SendEffectOper(uint8_t effectId, uint8_t operation)
{
	uint8_t midi_cmd[3];
	midi_cmd[0] = 0xB5;
	midi_cmd[1] = operation;
	midi_cmd[2] = effectId;
	ffb_sendData(midi_cmd, 3);
}

void Ffbpro_startEffect(uint8_t effectId)
{
	Ffbpro_SendEffectOper(effectId, 0x20);
}

void Ffbpro_stopEffect(uint8_t effectId)
{
	Ffbpro_SendEffectOper(effectId, 0x30);
}

void Ffbpro_freeEffect(uint8_t effectId)
{
	Ffbpro_SendEffectOper(effectId, 0x10);
}

// modify operations ---------------------------------------------------------

// Send to MIDI effect data modification to the given address of the given effect
void Ffbpro_SendModify(uint8_t effectId, uint8_t address, uint16_t value)
{
	// Modify + Address
	uint8_t midi_cmd[3];
	midi_cmd[0] = 0xB5;
	midi_cmd[1] = address;
	midi_cmd[2] = effectId;
	ffb_sendData(midi_cmd, 3);

	// New value
	midi_cmd[0] = 0xA5;
	midi_cmd[1] = value & 0x7F;
	midi_cmd[2] = (value & 0x7F00) >> 8;
	ffb_sendData(midi_cmd, 3);
}

void Ffbpro_SetEnvelope(const void *buf)
{
  const FFBReport_SetEnvelope_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
  struct paramEffectBlock *peb = getEffectBlock(eid);	
  const struct envelope *params = &input->p;
	/*
	USB effect data:
		uint8_t	reportId;	// =2
		uint8_t	effectBlockIndex;
		uint8_t attackLevel;
		uint8_t	fadeLevel;
		uint16_t	attackTime;	// ms
		uint16_t	fadeTime;	// ms

	MIDI effect data:
		uint8_t command;	// always 0x23	-- start counting checksum from here
		uint8_t effectType;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
		uint8_t unknown1;	// ? always 0x7F
		uint16_t duration;	// unit=2ms
		uint16_t unknown2;	// ? always 0x0000
		uint16_t direction;
		uint8_t	unknown3[5];	// ? always 7f 64 00 10 4e
		uint8_t attackLevel;
		uint16_t	attackTime;
		uint8_t		magnitude;
		uint16_t	fadeTime;
		uint8_t	fadeLevel;
		uint8_t	waveLength;	// 0x6F..0x01 => 1/Hz
		uint8_t unknown5;	// ? always 0x00
		uint16_t param1;	// Constant: positive=7f 00, negative=01 01, Other effects: 01 01
		uint16_t param2;	// Constant: 00 00, Other effects 01 01
	*/
#ifndef SAVE_SPACE
	if (isLogLevel(DEBUG_DETAIL)) {
		log_buf_P(PSTR("Set Envelope:"), input, sizeof(FFBReport_SetEnvelope_t));
		log_P(PSTR("  id    =%u"), eid);
		log_P(PSTR("  attack=%u"), params->attackLevel);
		log_P(PSTR("  fade  =%u"), params->fadeLevel);
		log_P(PSTR("  attackTime=%u"), params->attackTime);
		log_P(PSTR("  fadeTime  =%u"), params->fadeTime);
	}
#endif
  FFP_MIDI_Effect_Basic *midi_data = (FFP_MIDI_Effect_Basic *) gDevice.msg;

  copyParameters(peb, 1, params);
  peb->state |= HasEnvelope;

	midi_data->attackLevel = calcAmplitude(params->attackLevel, peb->gain);
	midi_data->fadeLevel = calcAmplitude(params->fadeLevel, peb->gain);

	midi_data->attackTime = Uint16ToMidiUint14(params->attackTime);

	if (params->fadeTime >= 0x7FFF)
		midi_data->fadeTime = FFB_DURATION_INFINITE;
	else
		midi_data->fadeTime = Uint16ToMidiUint14(peb->duration - params->fadeTime);

	if (peb->state & SentToDevice) {
		Ffbpro_SendModify(eid, 0x60, midi_data->fadeTime);
		Ffbpro_SendModify(eid, 0x5C, midi_data->attackTime);
		Ffbpro_SendModify(eid, 0x6C, midi_data->fadeLevel);
		Ffbpro_SendModify(eid, 0x64, midi_data->attackLevel);
	}
}

void Ffbpro_SetCondition(const void *buf)
{
  const FFBReport_SetCondition_t *input = buf;
	uint8_t eid = input->effectBlockIndex;
  struct paramEffectBlock *peb = getEffectBlock(eid);
  FFP_MIDI_Effect_Basic *common_midi_data = (FFP_MIDI_Effect_Basic *) gDevice.msg;
  const struct condition *params = &input->p;

	/*
	USB effect data:
		uint8_t	effectBlockIndex;
		uint8_t	parameterBlockOffset;	// bits: 0..3=parameterBlockOffset, 4..5=instance1, 6..7=instance2
		int8_t cpOffset;	// -128..127
		uint8_t	positiveCoefficient;	// 0..255

	MIDI effect data:
		uint16_t coeffAxis0;
		uint16_t coeffAxis1;
		uint16_t offsetAxis0; // not in friction
		uint16_t offsetAxis1; // not in friction
	*/
#ifndef SAVE_SPACE	
	if (isLogLevel(DEBUG_DETAIL))	{
		log_buf_P(PSTR("Set Condition:"), input, sizeof(FFBReport_SetCondition_t));
		log_P(PSTR("  id   =%u"), eid);
		log_P(PSTR("  block =%u"), input->parameterBlockOffset);
		log_P(PSTR("  offset=%u"), params->cpOffset);
		log_P(PSTR("  coeff=%u"), params->positiveCoefficient);
	}
#endif
	switch (common_midi_data->effectType) {
		case 0x0d:	// spring (midi: 0x0d)
		case 0x0e:	// damper (midi: 0x0e)
		case 0x0f:	// inertia (midi: 0x0f)
		{
			  FFP_MIDI_Effect_Spring_Inertia_Damper *midi_data =
				(FFP_MIDI_Effect_Spring_Inertia_Damper *) gDevice.msg;

			if (input->parameterBlockOffset == 0) {
        copyParameters(peb, 0, params);

				midi_data->coeffAxis0 = Int8ToMidiInt14(params->positiveCoefficient);
				midi_data->offsetAxis0 = Int8ToMidiInt14(params->cpOffset);
			} else {
        copyParameters(peb, 1, params);

				midi_data->coeffAxis1 = Int8ToMidiInt14(params->positiveCoefficient);
				if (params->cpOffset == -128)
					midi_data->offsetAxis1 = 0x007f;
				else
					midi_data->offsetAxis1 = Int8ToMidiInt14(-params->cpOffset);
			}

			// Send data to MIDI
			if (peb->state & SentToDevice) {
				if (input->parameterBlockOffset == 0) {
					Ffbpro_SendModify(eid, 0x48, midi_data->coeffAxis0);
					Ffbpro_SendModify(eid, 0x50, midi_data->offsetAxis0);
				} else {
					Ffbpro_SendModify(eid, 0x4C, midi_data->coeffAxis1);
					Ffbpro_SendModify(eid, 0x54, midi_data->offsetAxis1);
				}
			}
		}
		break;
		
		case 0x10:	// friction (midi: 0x10)
		{
			 FFP_MIDI_Effect_Friction *midi_data = (FFP_MIDI_Effect_Friction *) gDevice.msg;

			if (input->parameterBlockOffset == 0) {
        copyParameters(peb, 0, params);
				midi_data->coeffAxis0 = Int8ToMidiInt14(params->positiveCoefficient);
			} else {
        copyParameters(peb, 1, params);
				midi_data->coeffAxis1 = Int8ToMidiInt14(params->positiveCoefficient);
			}
			// Send data to MIDI
			if (peb->state & SentToDevice) {	// Send update
				if (input->parameterBlockOffset == 0)
					Ffbpro_SendModify(eid, 0x48, midi_data->coeffAxis0);
				else
					Ffbpro_SendModify(eid, 0x4C, midi_data->coeffAxis1);
			}
		}
		break;
		
		default:
			break;
	}
}

void Ffbpro_SetPeriodic(const void *buf)
{
  const FFBReport_SetPeriodic_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
  struct paramEffectBlock *peb = getEffectBlock(eid);
  const struct periodic *params = &input->p;

	/*
	USB effect data:
		uint8_t	reportId;	// =4
		uint8_t	effectBlockIndex;
		uint8_t magnitude;
		int8_t	offset;
		uint8_t	phase;	// 0..255 (=0..359, exp-2)
		uint16_t	period;	// 0..32767 ms

	MIDI effect data:

		Offset values other than zero do not work and thus it is ignored on FFP
	*/
#ifndef SAVE_SPACE	
	if (isLogLevel(DEBUG_DETAIL)) {
		log_buf_P(PSTR("Set Periodic:"), input, sizeof(FFBReport_SetPeriodic_t));
		log_P(PSTR("  id=%u"), eid);
		log_P(PSTR("  magnitude=%u"), params->magnitude);
		log_P(PSTR("  offset   =%u"), params->offset);
		log_P(PSTR("  phase    =%u"), params->phase);
		log_P(PSTR("  period   =%u"), params->period);
	}
#endif
  FFP_MIDI_Effect_Basic *midi_data = (FFP_MIDI_Effect_Basic *) gDevice.msg;

  copyParameters(peb, 0, params);

	midi_data->param1 = 0x007f;
	midi_data->param2 = 0x0101;

	// Calculate waveLength (in MIDI it is in units of 1/Hz and can have value 0x6F..0x01)
	if (params->period >= 1000)
		midi_data->waveLength = 0x01;
	else if (params->period <= 9)
		midi_data->waveLength = 0x6F;
	else
		midi_data->waveLength = (1000 / params->period) & 0x7F;

	// Check phase if relevant (+90 phase for sine makes it a cosine)
	if (midi_data->effectType == 2 || midi_data->effectType == 3) // sine
	{
		if (params->phase >= 32 && params->phase <= 224) {
			midi_data->effectType = 3;	// cosine
		} else {
			midi_data->effectType = 2;	// sine
		}

		// Calculate min-max from magnitude and offset
		uint8_t amplitude = calcAmplitude(params->amplitude, peb->gain);
		midi_data->param1 = Int8ToMidiInt14(params->offset / 2 + amplitude); // max
		midi_data->param2 = Int8ToMidiInt14(params->offset / 2 - amplitude); // min
		if (peb->state & SentToDevice) {
			Ffbpro_SendModify(eid, 0x74, midi_data->param1);
			Ffbpro_SendModify(eid, 0x78, midi_data->param2);
		}
	}

	if (peb->state & SentToDevice) {
		// Ffbpro_SendModify(eid, 0x74, midi_data->magnitude); // FFP does not actually support changing magnitude on-fly here
		Ffbpro_SendModify(eid, 0x70, midi_data->waveLength);
	}
}

void Ffbpro_SetConstantForce(const void *buf)
{
  const FFBReport_SetConstantForce_t *input = buf;
	uint8_t eid = input->effectBlockIndex;
  struct paramEffectBlock *peb = getEffectBlock(eid);
  const struct constant *params = &input->p;
	/*
	USB data:
		uint8_t	reportId;	// =5
		uint8_t	effectBlockIndex;
		int16_t magnitude;	// -255..255

	MIDI effect data:
		uint8_t command;	// always 0x23	-- start counting checksum from here
		uint8_t effectType;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
		uint8_t unknown1;	// ? always 0x7F
		uint16_t duration;	// unit=2ms
		uint16_t unknown2;	// ? always 0x0000
		uint16_t direction;
		uint8_t	unknown3[5];	// ? always 7f 64 00 10 4e
		uint8_t attackLevel;
		uint16_t	attackTime;
		uint8_t		magnitude;
		uint16_t	fadeTime;
		uint8_t	fadeLevel;
		uint8_t	waveLength;	// 0x6F..0x01 => 1/Hz
		uint8_t unknown5;	// ? always 0x00
		uint16_t param1;	// Constant: positive=7f 00, negative=01 01, Other effects: 01 01
		uint16_t param2;	// Constant: 00 00, Other effects 01 01
	*/
#ifndef SAVE_SPACE	
	if (isLogLevel(DEBUG_DETAIL)) {
		log_buf_P(PSTR("Set Constant Force:"), input, sizeof(FFBReport_SetConstantForce_t));
		log_P(PSTR("  id=%u"), eid);
		log_P(PSTR("  magnitude=%d"), params->magnitude);
	}
#endif
  copyParameters(peb, 0, params);
	FFP_MIDI_Effect_Basic *midi_data = (FFP_MIDI_Effect_Basic *) gDevice.msg;

  midi_data->magnitude = calcForce(params->magnitude, peb->gain);
	if (params->magnitude >= 0) {
		midi_data->param1 = 0x007f;
	} else {
		midi_data->param1 = 0x0101;
	}

	midi_data->param2 = 0x0000;

	if (peb->state & SentToDevice) {
		Ffbpro_SendModify(eid, 0x74, midi_data->magnitude);
		Ffbpro_SendModify(eid, 0x7C, midi_data->param1);
	}
}

void Ffbpro_SetRampForce(const void *buf)
{
  const FFBReport_SetRampForce_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
  struct paramEffectBlock *peb = getEffectBlock(eid);
  const struct ramp *params = &input->p;
  
	if (isLogLevel(DEBUG_DETAIL))	{
		log_buf_P(PSTR("Set Ramp Force:"), input, sizeof(FFBReport_SetRampForce_t));
		log_P(PSTR("  id=%u"), input->effectBlockIndex);
		log_P(PSTR("  start=%d"), params->start);
		log_P(PSTR("  end  =%d"), params->end);
	}
  copyParameters(peb, 0, params);

	// FFP supports only ramp up from MIN to MAX and ramp down from MAX to MIN?
	/*
	USB effect data:
		uint8_t	reportId;	// =6
		uint8_t	effectBlockIndex;
		int8_t start;
		int8_t	end;
	*/
	
	FFP_MIDI_Effect_Basic *midi_data = (FFP_MIDI_Effect_Basic *) gDevice.msg;

	if (params->start < 0)
		midi_data->param1 = 0x0100 | (-(params->start+1));
	else
		midi_data->param1 = params->start;

	midi_data->param2 = Int8ToMidiInt14(params->end);

	if (peb->state & SentToDevice) {
		Ffbpro_SendModify(eid, 0x78, midi_data->param1);
		Ffbpro_SendModify(eid, 0x74, midi_data->param2);
	}
}

static void setCommon(uint8_t effectType)
{
  // Set defaults to the effect data at SetEffect-"time"
  FFP_MIDI_Effect_Basic *midi_data = (FFP_MIDI_Effect_Basic *) gDevice.msg;
  
  ((midi_data_common_t*) gDevice.msg)->effectType = Ffbpro_UsbToMidiEffectType(effectType);

  midi_data->magnitude = 0x7f;
  midi_data->waveLength = 0x01;
  midi_data->attackLevel = 0x00;
  midi_data->attackTime = 0x0000;
  midi_data->fadeLevel = 0x00;
  midi_data->fadeTime = 0x0000;

  // Constants
  midi_data->command = 0x23;
  midi_data->unknown1 = 0x7F;
  midi_data->unknown2 = 0x0000;
  midi_data->unknown3[0] = 0x7F;
  midi_data->unknown3[1] = 0x64;
  midi_data->unknown3[2] = 0x00;
  midi_data->unknown3[3] = 0x10;
  midi_data->unknown3[4] = 0x4E;

  if (effectType == 0x01)  // constant force
    midi_data->param2 = 0x0000;
  else
    midi_data->param2 = 0x0101;

}

int Ffbpro_SetEffect(const void *buf)
{
  const FFBReport_SetEffect_t *input = buf;

	/*
	USB effect data:
		uint8_t	reportId;	// =1
		uint8_t	effectBlockIndex;
		uint8_t	effectType;	// 1..12 (effect usages: 26,27,30,31,32,33,34,40,41,42,43,28)
		uint16_t	duration; // 0..32767 ms
		uint16_t	triggerRepeatInterval; // 0..32767 ms
		uint16_t	samplePeriod;	// 0..32767 ms
		uint8_t	gain;	// 0..255	 (physical 0..10000)
		uint8_t	triggerButton;	// button ID (0..8)
		uint8_t	enableAxis; // bits: 0=X, 1=Y, 2=DirectionEnable
		uint8_t	directionX;	// angle (0=0 .. 180=0..360deg)
		uint8_t	directionY;	// angle (0=0 .. 180=0..360deg)
	*/

  uint8_t eid = input->effectBlockIndex;
	FFP_MIDI_Effect_Basic *midi_data = (FFP_MIDI_Effect_Basic *) gDevice.msg;
	struct paramEffectBlock *peb = getEffectBlock(eid);
	uint8_t midi_data_len = sizeof(FFP_MIDI_Effect_Basic); 	// default MIDI data size
	bool is_periodic = false;

  setCommon(eid);
  
	// Fill in the effect type specific data
	switch (input->effectType)
	{
		case FFB_EFFECT_SQUARE:
		case FFB_EFFECT_SINE:
		case FFB_EFFECT_TRIANGLE:
		case FFB_EFFECT_SAWTOOTHDOWN:
		case FFB_EFFECT_SAWTOOTHUP:
			is_periodic = true;
		case FFB_EFFECT_CONSTANT:
		case FFB_EFFECT_RAMP:
		{
			/*
			MIDI effect data:
				uint8_t command;	// always 0x23	-- start counting checksum from here
				uint8_t effectType;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
				uint8_t unknown1;	// ? always 0x7F
				uint16_t duration;	// unit=2ms
				uint16_t unknown2;	// ? always 0x0000
				uint16_t direction;
				uint8_t	unknown3[5];	// ? always 7f 64 00 10 4e
				uint8_t attackLevel;
				uint16_t	attackTime;
				uint8_t		magnitude;
				uint16_t	fadeTime;
				uint8_t	fadeLevel;
				uint8_t	waveLength;	// 0x6F..0x01 => 1/Hz
				uint8_t unknown5;	// ? always 0x00
				uint16_t param1;	// Constant: positive=7f 00, negative=01 01, Other effects: 01 01
				uint16_t param2;	// Constant: 00 00, Other effects 01 01
			*/

			// Convert direction
			uint16_t drvdir = input->directionX;
			drvdir = drvdir * 2;
			uint16_t dir = (drvdir & 0x7F) + ( (drvdir & 0x0180) << 1 );
			midi_data->direction = dir;
      struct periodic *params = getParamBlock(peb, 0);
      struct envelope *envelope = getParamBlock(peb, 1);
      
			// Recalculate fadeTime for MIDI since change to duration changes the fadeTime too
			if (input->duration >= 0x7FFF) {
				midi_data->fadeTime = FFB_DURATION_INFINITE;
			} else {
        midi_data->fadeTime = midi_data->duration;
        if (peb->state & HasEnvelope) {
  				if (envelope->fadeTime >= 0x7FFF) {
  					midi_data->fadeTime = FFB_DURATION_INFINITE;
  				} else {
  					if (peb->duration > envelope->fadeTime) {
  						// add some safety and special case handling
  						midi_data->fadeTime = Uint16ToMidiUint14(peb->duration - envelope->fadeTime);
  					}
  				}
        }
			}

			// Gain and its effects (magnitude and envelope levels)
			bool gain_changed = (peb->gain != input->gain);
			if (gain_changed) {
//				log_P(PSTR("  New gain:"), data->gain);

				peb->gain = input->gain;
				midi_data->attackLevel = calcAmplitude(envelope->attackLevel, input->gain);
				midi_data->fadeLevel = calcAmplitude(envelope->fadeLevel, input->gain);

        midi_data->magnitude = calcAmplitude(params->amplitude, peb->gain);
				if (is_periodic) {
					// Calculate min-max from magnitude and offset, since magnitude may be affected by gain we must calc them here too for periodic effects
					midi_data->param1 = Int8ToMidiInt14(params->offset + midi_data->magnitude); // max
					midi_data->param2 = Int8ToMidiInt14(params->offset - midi_data->magnitude); // min
					if (peb->state & SentToDevice) {
						Ffbpro_SendModify(eid, 0x74, midi_data->param1);
						Ffbpro_SendModify(eid, 0x78, midi_data->param2);
					}
				}
			}

			// Send data to MIDI
			if (peb->state & SentToDevice)
			{
        Ffbpro_SendModify(eid, 0x40, midi_data->duration);
				Ffbpro_SendModify(eid, 0x48, midi_data->direction);
				Ffbpro_SendModify(eid, 0x60, midi_data->fadeTime);
				if (gain_changed) {
					Ffbpro_SendModify(eid, 0x6C, midi_data->fadeLevel);	// might have changed due gain
					Ffbpro_SendModify(eid, 0x64, midi_data->attackLevel);	// might have changed due gain
					if (!is_periodic) {
						Ffbpro_SendModify(eid, 0x74, midi_data->magnitude);	// might have changed due gain
					}
				}
			}
		}
		break;
	
		case FFB_EFFECT_SPRING:
		case FFB_EFFECT_DAMPER:
		case FFB_EFFECT_INERTIA:
		{
			/*
			MIDI effect data:
				uint8_t command;	// always 0x23	-- start counting checksum from here
				uint8_t effectType;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
				uint8_t unknown1;	// ? always 0x7F
				uint16_t duration;	// unit=2ms
				uint16_t unknown2;	// ? always 0x0000
				uint16_t coeffAxis0;
				uint16_t coeffAxis1;
				uint16_t offsetAxis0;
				uint16_t offsetAxis1;
			*/
			midi_data_len = sizeof(FFP_MIDI_Effect_Spring_Inertia_Damper);

		}
		break;
		
		case FFB_EFFECT_FRICTION:
		{
			/*
			MIDI effect data:
				uint8_t command;	// always 0x23	-- start counting checksum from here
				uint8_t effectType;	// 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triange, 0x12=Constant
				uint8_t unknown1;	// ? always 0x7F
				uint16_t duration;	// unit=2ms
				uint16_t unknown2;	// ? always 0x0000
				uint16_t coeffAxis0;
				uint16_t coeffAxis1;
			*/
			midi_data_len = sizeof(FFP_MIDI_Effect_Friction);

		}
		break;
		
		case FFB_EFFECT_CUSTOM:	// custom (midi: ? does FFP support custom forces?)
		{
		}
		break;
		
		default:
		break;
	}
	
	return midi_data_len;
}
