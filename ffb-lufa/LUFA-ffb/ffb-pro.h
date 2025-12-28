
#ifndef _FFB_PRO_
#define _FFB_PRO_

#include <stdint.h>
#include "ffb.h"

// ----------------------------------
// Microsoft Sidewinder Force Feedback Pro FFB structures

typedef struct
	{
	uint8_t command;	// always 0x23	-- start counting checksum from here
	uint8_t effectType;	// 0xd=Spring, 0x0e=Damper, 0xf=Inertia
	uint8_t unknown1;	// ? always 0x7F
	uint16_t duration;	// unit=2ms
	uint16_t unknown2;	// ? always 0x0000
	uint16_t coeffAxis0;
	uint16_t coeffAxis1;
	uint16_t offsetAxis0;
	uint16_t offsetAxis1;
	} FFP_MIDI_Effect_Spring_Inertia_Damper;

typedef struct
	{
	uint8_t command;	// always 0x23	-- start counting checksum from here
	uint8_t effectType;	// 0x10=Friction
	uint8_t unknown1;	// ? always 0x7F
	uint16_t duration;	// unit=2ms
	uint16_t unknown2;	// ? always 0x0000
	uint16_t coeffAxis0;
	uint16_t coeffAxis1;
	} FFP_MIDI_Effect_Friction;

void Ffbpro_EnableFFB(void);
const uint8_t* Ffbpro_GetSysExHeader(uint8_t* hdr_len);
void Ffbpro_SetAutoCenter(uint8_t enable);
void Ffbpro_DeviceForceGain(uint8_t gain);


void Ffbpro_startEffect(uint8_t id);
void Ffbpro_stopEffect(uint8_t id);
void Ffbpro_freeEffect(uint8_t id);

void FfbproModifyDuration(uint8_t effectId, uint16_t duration);

void Ffbpro_SetEnvelope(const void *buf);
void Ffbpro_SetCondition(const void *buf);
void Ffbpro_SetPeriodic(const void *buf);
void Ffbpro_SetConstantForce(const void *buf);
void Ffbpro_SetRampForce(const void *buf);
int  Ffbpro_SetEffect(const void *buf);
void Ffbpro_CreateEffect(const void *buf);

uint8_t Ffbpro_UsbToMidiEffectType(uint8_t drv_effect_type);

#endif // _FFB_PRO_
