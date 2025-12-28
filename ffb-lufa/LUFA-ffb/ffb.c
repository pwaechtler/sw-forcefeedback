/*
  This code is for Microsoft Sidewinder 
  Force Feedback Pro joystick and wheel.

  Copyright 2022  peewee.hannover [at] gmail [dot] com
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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "ffb.h"
#include "debug.h"
#include "board.h"
#include "ffb-pro.h"
#include "ffb-wheel.h"

void PidStateChanged();

struct FfbDevice  gDevice;

#ifdef WHEEL_ONLY_DRIVER

#define DRV_CALL(f)      Ffbwheel_##f

void ffb_setDriver(uint8_t id)  {}
#else

#define FFB_INDIRECT(f)   ffb->f
#define DRV_CALL(f)       FFB_INDIRECT(f)

const FFB_Driver ffb_drivers[2] =
{
  {
    .EnableFFB        = Ffbpro_EnableFFB,
    .GetSysExHeader   = Ffbpro_GetSysExHeader,
    .SetAutoCenter    = Ffbpro_SetAutoCenter,
    .DeviceForceGain  = Ffbpro_DeviceForceGain,
    .startEffect      = Ffbpro_startEffect,
    .stopEffect       = Ffbpro_stopEffect,
    .freeEffect       = Ffbpro_freeEffect,
    .SetEnvelope      = Ffbpro_SetEnvelope,
    .SetCondition     = Ffbpro_SetCondition,
    .SetPeriodic      = Ffbpro_SetPeriodic,
    .SetConstantForce = Ffbpro_SetConstantForce,
    .SetRampForce     = Ffbpro_SetRampForce,
    .SetEffect        = Ffbpro_SetEffect,
  },
  {
    .EnableFFB        = Ffbwheel_EnableFFB,
    .GetSysExHeader   = Ffbwheel_GetSysExHeader,
    .SetAutoCenter    = Ffbwheel_SetAutoCenter,
    .DeviceForceGain  = Ffbwheel_DeviceForceGain,
    .startEffect      = Ffbwheel_startEffect,
    .stopEffect       = Ffbwheel_stopEffect,
    .freeEffect       = Ffbwheel_freeEffect,
    .SetEnvelope      = Ffbwheel_SetEnvelope,
    .SetCondition     = Ffbwheel_SetCondition,
    .SetPeriodic      = Ffbwheel_SetPeriodic,
    .SetConstantForce = Ffbwheel_SetConstantForce,
    .SetRampForce     = Ffbwheel_SetRampForce,
    .SetEffect        = Ffbwheel_SetEffect,
  }
};

static const FFB_Driver* ffb;
void ffb_setDriver(uint8_t id)
{
  if ( SW_ID_WHEEL == id )
    id = 1;
  else id = 0;
  ffb = &ffb_drivers[id];
}
#endif  // WHEEL_ONLY_DRIVER



uint8_t checkBlockIndex(uint8_t eid)
{
  if (eid >= MAX_EFFECTS) {
    if (eid != 0xff)
      error_P(PSTR("<%u>  out of range"), eid);
    return 0;
  }
  return 1;
}

void changePidStateOnEffect(uint8_t eid)
{
  if (!checkBlockIndex(eid)) {
    return;
  }
  gDevice.pidState.effectBlockIndex = eid;

  if (eid) {
    if (getEffectBlock(eid)->state & Playing)
      gDevice.pidState.effectBlockIndex |= Playing; // must be 0x80;
  }
  gDevice.pidState.status =  (uint8_t)   gDevice.state;
  PidStateChanged();
}

/**
 * this is called on each loop and remove the Playing bit on effects which ended (with no explicit Stop)
 * also startDelay is implemented, but even fedit does not implement it correctly, fedit defers the play itself
 */
void updateEffectStates()
{
  uint8_t eid;
  uint32_t ts = millis();
  struct paramEffectBlock *peb;

  for (eid = 2; eid < MAX_EFFECTS; eid++) {
    peb = getEffectBlock(eid);
    // keep track of playing effects and when they are over
    if (peb->state & Playing) {
        if (ts > peb->startStopTime + peb->duration) {
          if (peb->duration != FFB_DURATION_INFINITE) {
            peb->state &= ~ Playing;
            stopEffect(eid);
            changePidStateOnEffect(eid);
          }
        }
    }
#ifdef SUPPORT_START_DELAY
    if (peb->state & DelayedPlay) {
      if ( ts > peb->startStopTime) {
        peb->state &= ~ (ToDelay | DelayedPlay);

        if ( !(peb->state & Playing) ) {
          startEffect(eid);
        }
        PidStateChanged();
#ifndef SAVE_SPACE
        if (isLogLevel(DEBUG_DETAIL))
          log_P(PSTR("startDelayed<%u>"), eid);
#endif
      }
    }
#endif  // SUPPORT_START_DELAY
  }
}

uint8_t allocParamBlock(/* uint8_t type */)
{
  uint8_t i = 0xFF;
  uint8_t block;
  struct memPool *pool = &gDevice.pool;
  uint8_t bit = 0;
  
  for (block = 0; block < sizeof(pool->used); block++) {
    if (pool->used[block] != 0xFF) {
      for (bit = 0; bit < 8; bit++) {
        if ((pool->used[block] & (1 << bit)) == 0) {
          i = (block * 8) + bit;
          pool->used[block] |= (1 << bit);
//          log_P(PSTR("alloc: i: %u bl: %u bit: %u"), i, block, bit);
          return i;
        }
      }
    }
  }
  return i;
}

void freeParamBlock(uint8_t index)
{
  uint8_t block;
  uint8_t bit;
  struct memPool *pool = &gDevice.pool;

  block = (index +7) / 8;
  bit = index % 8;
  
  pool->used[block] &= ~(1 << bit);
  memset(&pool->paramBlocks[index], 0, sizeof(union parameterBlock));
//  log_P(PSTR("free: i: %u bl: %u bit: %u"), index, block, bit);
}

struct paramEffectBlock *getEffectBlock(uint8_t eid)
{
  if (!checkBlockIndex(eid))
    return NULL;
  return &gDevice.pool.effects[eid];
}

void *getParamBlock(const struct paramEffectBlock *peb, uint8_t index)
{
  uint8_t blocknr = peb->paramBlocks[index];
  return &gDevice.pool.paramBlocks[blocknr];
}

void copyParameters(const struct paramEffectBlock *peb, uint8_t index, const void *params )
{
  size_t len = sizeof(union parameterBlock);
  void * dest;
  
  dest = getParamBlock(peb, index);
  if (dest)
    memcpy(dest, params, len);
}

struct paramEffectBlock *allocEffectBlock(uint8_t effectType, uint8_t *ret_eid)
{
  if (gDevice.pool.slotsAvail == 0) {
    error_loop(6);
    return NULL;
  }

  gDevice.pool.slotsAvail--;

  uint8_t eid = gDevice.pool.nextID++;
  
  while (getEffectBlock(gDevice.pool.nextID)->state != Free)
  {
    if (gDevice.pool.nextID >= MAX_EFFECTS)
      break;  // the last slot was taken
    gDevice.pool.nextID++;
  }
  
  struct paramEffectBlock *peb = getEffectBlock(eid);
  // TODO: only MAX_AXES+1 for effects using SetCondition
  for (uint8_t i = 0; i < MAX_AXES +1; i++) {
    peb->paramBlocks[i] = allocParamBlock();
    if (peb->paramBlocks[i] == 0xFF) {
      gDevice.pool.slotsAvail++;
      error_loop(7);
      return NULL;  // never reached
    }
  }
  peb->state = Allocated;
  peb->effectType = effectType;    
//  log_P(PSTR("allocEB: eid: %u type: %u"), eid, effectType);

  if (ret_eid)
    *ret_eid = eid;
  return peb;
}

void freeEffect(uint8_t eid)
{
  if (!checkBlockIndex(eid))
    return;
  // tell device
  DRV_CALL(freeEffect(eid));
  
  struct paramEffectBlock *peb = getEffectBlock(eid);
  peb->state = Free;
  if (eid < gDevice.pool.nextID)
    gDevice.pool.nextID = eid;
  freeParamBlock(peb->paramBlocks[0]);
  freeParamBlock(peb->paramBlocks[1]);
  gDevice.pool.slotsAvail++;
}

void startEffect(uint8_t eid)
{
  if (!checkBlockIndex(eid))
    return;
  struct paramEffectBlock *peb = getEffectBlock(eid);

  if ( ! (peb->state & Playing)) {

    if (peb->state & ToDelay) {
      // add current ts to the "startDelay"
      peb->startStopTime += millis();
      peb->state &= ~ToDelay;
      peb->state |= DelayedPlay;
    } else { 
      peb->state |= Playing;
      DRV_CALL(startEffect(eid));
      peb->startStopTime = millis();
      changePidStateOnEffect(eid);
    }
  }
}

void stopEffect(uint8_t eid)
{
  if (!checkBlockIndex(eid))
    return;
  struct paramEffectBlock *peb = getEffectBlock(eid);
  if (peb->state & Playing) {
    peb->state &= ~Playing;
    peb->startStopTime = millis();  
    changePidStateOnEffect(eid);
  }
  DRV_CALL(stopEffect(eid));
}

void stopAllEffects(void)
{
  for (uint8_t eid = 2; eid < MAX_EFFECTS; eid++)
    stopEffect(eid);
}


void freeAllEffects(void)
{
  uint8_t eid;
  for (eid = 2; eid < MAX_EFFECTS; eid++) {
    if (getEffectBlock(eid)->state == Allocated) {
      freeEffect(eid);
    }
  }
}


void ffb_handleBlockFree(void *buf)
{
  FFBReport_BlockFree_Output_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
#ifndef SAVE_SPACE
  if (isLogLevel(DEBUG_DETAIL)) {
    log_P(PSTR("Block Free:<%u>"), eid);
  }
#endif
  if (eid == 0xFF || eid == 0x7F)
  { // all effects
    freeAllEffects();
  } else {
    freeEffect(eid);
  }
}

void ffb_handleEffectOperation(const void *buf)
{
  const FFBReport_EffectOperation_Output_t *input = buf;
  uint8_t eid = input->effectBlockIndex;

  if (isLogLevel(DEBUG_LOG_EFFECT) || isLogLevel(DEBUG_LOG_STARTSTOP)) {
    error_P_nonl(PSTR("EffectOp<%u>: %u "), eid, input->operation);
  }
  if (input->loopCount > 1) {
    error_P_nonl(PSTR("loop:"));
    printUnsupported(input->loopCount);
  }
  if (eid == 0xFF)
    eid = 0x7F;  // All effects
  if (input->operation == DRV_CMD_START) {
    if (isLogLevel(DEBUG_LOG_EFFECT) || isLogLevel(DEBUG_LOG_STARTSTOP))
      log_P(PSTR("Start"));

    startEffect(input->effectBlockIndex);
  } else if (input->operation == DRV_CMD_SOLO) {
    if (isLogLevel(DEBUG_LOG_EFFECT) || isLogLevel(DEBUG_LOG_STARTSTOP))
      log_P(PSTR("Solo"));

    stopAllEffects();
    startEffect(input->effectBlockIndex);
  } else if (input->operation == DRV_CMD_STOP) {
    if (isLogLevel(DEBUG_LOG_EFFECT) || isLogLevel(DEBUG_LOG_STARTSTOP))
      log_P(PSTR("Stop"));

    stopEffect(input->effectBlockIndex);
  } else {
      error_P(PSTR("Unknown op %u"), input->operation);
  }
}

//------------------------------------------------------------------------------

void _delay_us10(uint8_t delay)
{
  while (delay--) {
    _delay_us(9); // .. compensate for loop handling
  }
}

void waitMs(int ms)
{
  while (ms--)
  {
    wdt_reset() ;
    _delay_ms(1);
  }
}

// Non-buffered MIDI
void FfbSendByte(uint8_t data)
{
   board_ffb_putc(data);
}

static const char PROGMEM MIDI_HEADING[] = "=>Midi:";

static uint8_t _fetch_sanitized(const uint8_t *addr)
{
  return (*addr) & 0x7f;
}

static uint8_t _fetch_ram(const uint8_t *addr)
{
  return *addr;
}

static uint8_t _fetch_pgm(const uint8_t *pgm_addr)
{
  return pgm_read_byte(pgm_addr);
}

static void _sendData_body(const uint8_t *data, uint16_t len, uint8_t (*fetch_p)(const uint8_t *addr))
{
  uint8_t byte;
  if (isLogLevel(DEBUG_LOG_MIDI)) {
    if (fetch_p == _fetch_pgm)
      log_buf_PP(MIDI_HEADING, data, len);
    else
      log_buf_P(MIDI_HEADING, data, len);
  }
  uint16_t i = 0;
  for (i = 0; i < len; i++) {
    byte = fetch_p( data + i );
    FfbSendByte(byte);
  }
  gDevice.sentBytes += len;
}

void ffb_sendData_masked(const uint8_t *data, uint16_t len)
{
  _sendData_body(data, len, _fetch_sanitized);
}

void ffb_sendData(const uint8_t *data, uint16_t len)
{
  _sendData_body(data, len, _fetch_ram);
}

void ffb_sendData_P(const uint8_t *data, uint16_t len)
{
  _sendData_body(data, len, _fetch_pgm);
}

void ffb_sendSysEx(const uint8_t* msg, uint8_t len)
{
  uint8_t hdr_len;
  uint8_t epilog[2];
  const uint8_t*  hdr = DRV_CALL(GetSysExHeader(&hdr_len));

  uint8_t restore = gLogLevel;
  gLogLevel = DEBUG_LOG_MIDI;
  ffb_sendData_P(hdr, hdr_len);
  ffb_sendData_masked( msg, len);

  epilog[0] = 0;        // checksum
  epilog[1] = 0xF7;    // SysEx End

  while (len--)
    epilog[0] += *msg++;
  epilog[0] = (0x80 - epilog[0]) & 0x7f;
  
  ffb_sendData(epilog, 2);
  gLogLevel = restore;
}


// Utilities

uint16_t Uint16ToMidiUint14(uint16_t inValue)
{
  if (inValue >= 0x7FFF)
    return 0x0000;

  return (inValue & 0x7F00) + ((inValue & 0x007F) );//>> 1);	// loss of the MSB-bit!
}

int16_t Int8ToMidiInt14(int8_t inValue)
{
  int16_t value;
  if (inValue < 0)
  {
    value = inValue;
    value += 0x7f80;
  }
  else
    value = inValue;

  return value;
}

int16_t Int16ToMidiInt14(int16_t val)
{
  uint8_t low, high;
  
     low = val & 0x7F; 
     high = (val >> 7) & 0x7F; 

  return (int16_t) ((int16_t)high << 8) | low;
}

// Calculates the magnitude taking a maximum <gain> of 127 into account.
// Returns MIDI value (i.e. max 0..7f).
uint8_t calcAmplitude(uint8_t value, uint8_t gain)
{
  int16_t v = value;
  return (((v * gain) / 127) ) & 0x7f;
} 

uint8_t calcForce(int16_t magnitude, uint8_t gain)
{
  /* like the MS descriptor: magnitude is in logical range -255..+255 */
  return calcAmplitude( (abs(magnitude) / 2), gain);
}


// Lengths of each "inflight" report type
const uint16_t OutReportLengths[] = {
  sizeof(FFBReport_SetEffect_t),		// 1
  sizeof(FFBReport_SetEnvelope_t),	// 2
  sizeof(FFBReport_SetCondition_t),	// 3
  sizeof(FFBReport_SetPeriodic_t),	// 4
  sizeof(FFBReport_SetConstantForce_t),	// 5
  sizeof(FFBReport_SetRampForce_t),	// 6
  sizeof(FFBReport_EffectOperation_Output_t),  // 7
  sizeof(FFBReport_BlockFree_Output_t),	//8
  sizeof(FFBReport_DeviceControl_Output_t),	// 9
  sizeof(FFBReport_DeviceGain_Output_t),	// 10
#ifdef SUPPORT_CUSTOM_FORCE
  sizeof(FFBReport_SetCustomForceData_Output_t),  // 11
  sizeof(FFBReport_SetDownloadForceSample_Output_t),  // 12
  sizeof(FFBReport_SetCustomForce_Output_t),  // 13
#endif
};
const uint8_t NumOfReports = sizeof(OutReportLengths)/sizeof(size_t);


enum deviceConfig {
  deviceMangedPool       = 0x01,    // the device manages the pool not the driver
  sharedParameterBlocks  = 0x02, // effects can share the same parameter block 
  customForceIsochronous = 0x04  //
};

uint16_t ffb_onPIDPool(uint8_t **response_ptr)
{
  FFBReport_PIDPool_Feature_t *report = &gDevice.poolReport; 

  report->reportId = FFB_PID_POOL_REPORT_ID;
  report->capabilities = deviceMangedPool | sharedParameterBlocks;   // bit 0: device/driver managed mem pool  1:/effects can share the same parameter block 
  report->maxSimultaneousEffects = 10;  // FFPro supports playing up to 10 simultaneous effects

  report->ramPoolSize = MAX_EFFECTS -2; //gDevice.pool.freeMem + (gDevice.pool.slotsAvail * sizeof(union parameterBlock)); //MAX_EFFECTS -2; // simplified "pool"
#ifdef NEW_PID_POOL_REPORT

  /* the device can offer read-only in ROM stored effects
   * the spec says: the driver can parse them - but it never tells how
   */
#ifdef PID_POOL_INCLUDES_ROMPOOL
  report->romPoolSize = 0;
  report->romBlockCount = 0;
#endif

  /* the spec says that these values only apply to a driver managed pool
   *  but Windows 7 does not accept the report without them when device managed
   * #ifdef  DRIVER_MANAGED_POOL
   */
//#ifdef  DRIVER_MANAGED_POOL
  // reportID and effectBlockIndex aka eid are not stored in Parameter block
  report-> set_Effect_Size = sizeof(FFBReport_SetEffect_t) -2;
  report-> set_Envelope_Size = sizeof(struct envelope);
  report-> set_Condition_Size = sizeof(struct condition);
  report-> set_Periodic_Size = sizeof(struct periodic);
  report-> set_Constant_Force_Size = sizeof(struct constant);
  report-> set_Ramp_Force_Size = sizeof(struct ramp);
#ifdef SUPPORT_CUSTOM_FORCE
  report-> set_Custom_Force_Size = sizeof(FFBReport_SetCustomForceData_Output_t) -2;
#ifdef SUPPORT_CUSTOM_FORCE_ISOCHRONUOS
  report-> capabilities |= customForceIsochronous;    // beware the order of initialization
#endif  //SUPPORT_CUSTOM_FORCE_ISOCHRONUOS

#endif  //SUPPORT_CUSTOM_FORCE
//#endif  //DRIVER_MANAGED_POOL


#endif  //NEW_PID_POOL_REPORT

#ifndef SAVE_SPACE
  if (isLogLevel(DEBUG_DETAIL))
    log_P(PSTR("PID Pool: simEff: %u  MM: 0x%x"), report->maxSimultaneousEffects, report->capabilities);
#endif
  if (response_ptr)
    *response_ptr = (uint8_t*) report;
  return sizeof(FFBReport_PIDPool_Feature_t);
}

// 1=Enable Actuators, 2=Disable Actuators, 3=Stop All Effects, 4=Reset, 5=Pause, 6=Continue

void ffb_handleDeviceControl(const void *buf)
{
  const FFBReport_DeviceControl_Output_t *input = buf;
  uint8_t control = input->control;
  error_P_nonl(PSTR("DevCtrl[0x%x]: "), control);

  if (control == 0x01) {
    error_P(PSTR("disable Act"));
    gDevice.state &= ~ActuatorsEnabled;
  } else if (control == 0x02) {
    error_P(PSTR("enable Act"));
      //TODO read the "Force button" state (override switch)
     gDevice.pidState.status |= ActuatorsEnabled;
  } else if (control == 0x03) {
    // Stop all effects (e.g. FFB-application to foreground)
    error_P(PSTR("Stop All"));
    stopAllEffects();
    gDevice.pidState.status |= Paused;
  } else if (control == 0x04) {
    error_P(PSTR("Reset"));
    // Reset (e.g. FFB-application out of focus)
    stopAllEffects();
    uint16_t save_max = gDevice.maxSentBytes;
    ffb_init();
    gDevice.maxSentBytes = save_max;
    DRV_CALL(EnableFFB());
    waitMs(15);
  } else if (control == 0x05) {
    error_P(PSTR("Pause")); // they really mean to pause "all effects" at the current sample period
    stopAllEffects();
    gDevice.state |= Paused;
    DRV_CALL(SetAutoCenter(1));
  } else if (control == 0x06) {
    error_P(PSTR("Continue"));  // they really mean to continue the "paused" effects where they were stopped
    gDevice.state &= ~Paused;
    DRV_CALL(EnableFFB());
    DRV_CALL(SetAutoCenter(0));
  }
}

void ffb_handleDeviceGain(const void *buf)
{
  const FFBReport_DeviceGain_Output_t *input = buf;
  error_P(PSTR("DeviceGain: %u"), input->gain);
  DRV_CALL(DeviceForceGain(input->gain));
}

#ifdef SUPPORT_CUSTOM_FORCE
void ffb_handleSetCustomForceData(void *buf)
{
    error_P(PSTR("SetCustomForceData"), buf, sizeof(FFBReport_SetCustomForceData_Output_t));
}

void ffb_handleSetDownloadForceSample(void *buf)
{
    error_P(PSTR("SetDownloadForceSample"), buf, sizeof(FFBReport_SetDownloadForceSample_Output_t));
}

void ffb_handleSetCustomForce(void *buf)
{
  error_P(PSTR("SetCustomForce"), buf, sizeof(FFBReport_SetCustomForce_Output_t));
}
#endif

// TODO: wait for the GetReport feature request, not just sending it
uint16_t ffb_onBlockLoad(uint8_t **response_ptr)
{
  FFBReport_PIDBlockLoad_Feature_t *output = &gDevice.blockLoadReport;
  output->reportId = FFB_FEATURE_BLOCK_LOAD_ID;
  
#ifndef SAVE_SPACE
  if (isLogLevel(DEBUG_DETAIL))
    log_P(PSTR("ffb_onBlockLoad<%u> load_stat: %u"), output->effectBlockIndex, output->loadStatus);
#endif
  if (response_ptr)
    *response_ptr = (uint8_t*) output;
  return sizeof(FFBReport_PIDBlockLoad_Feature_t);

}

// readable text for effectType
// use error_P so that it works even on NDEBUG via serial debug "list" cmd
void printEffectType(uint8_t effectType)
{
  const char *s;
    switch (effectType) {
      case FFB_EFFECT_SQUARE:
      case FFB_EFFECT_SINE:
      case FFB_EFFECT_TRIANGLE:
      case FFB_EFFECT_SAWTOOTHDOWN:
      case FFB_EFFECT_SAWTOOTHUP:
          s = PSTR("WAVE[%u]");
          break;
      case FFB_EFFECT_RAMP:
          s = PSTR("RAMP"); break;
      case FFB_EFFECT_DAMPER:
          s = PSTR("DAMPER"); break;
      case FFB_EFFECT_INERTIA:
          s = PSTR("INERTIA"); break;
     case FFB_EFFECT_SPRING:
          s = PSTR("SPRING"); break;
     case FFB_EFFECT_CONSTANT:
          s = PSTR("ConstForce"); break;
     case FFB_EFFECT_FRICTION:
          s = PSTR("FRICTION"); break;
     case FFB_EFFECT_CUSTOM:
          s = PSTR("CUSTOM"); break;
    default:
          s = PSTR("<?>"); break;
    }
    error_P_nonl(s, effectType);
}

void ffb_handleSetEffect(void *buf)
{
  FFBReport_SetEffect_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
  if ( !checkBlockIndex(eid))
    return;
#ifndef SAVE_SPACE
  if (isLogLevel(DEBUG_DETAIL)) {
    log_buf_P(PSTR("Set Effect:"), input, sizeof(FFBReport_SetEffect_t));
    log_P(PSTR("  id  = %u"), input->effectBlockIndex);
    log_P(PSTR("  type= %u"), input->effectType);
    log_P(PSTR("  gain= %u"), input->gain);
    log_P(PSTR("  dura= %X"), input->duration);
    if (params->enableAxis) {
#ifdef REPORT_ONLY_1AXIS
      log_P(PSTR("  X= %X"), input->directionX);
#else
      log_P(PSTR("  X= %X  Y= %X"), input->directionX, input->directionY);
#endif
    }
    if (input->triggerRepeatInterval) {
      log_P(PSTR("  repeat= %X"), input->triggerRepeatInterval);
    }
    if (input->triggerButton) {
      log_P(PSTR("  button= %X"), input->triggerButton);
    }
  }
#endif  // SAVE_SPACE

  gDevice.setEffectCount++;
  
  struct paramEffectBlock *peb = getEffectBlock(eid);
  midi_data_common_t* midi_data = (midi_data_common_t*) gDevice.msg;

  if ( input->duration >= 0x7FFF ) {
    midi_data->duration = FFB_DURATION_INFINITE;
  } else {
    midi_data->duration = Uint16ToMidiUint14(input->duration); // MIDI unit is 2ms
  }
#ifdef SEND_ONLY_CHANGED_PARAMS
  if (midi_data->duration != peb->duration)
    peb->changedParams = 0x03;
#endif
  peb->duration = midi_data->duration;  // store for later calculation of <fadeTime>

  uint8_t midi_data_len = DRV_CALL(SetEffect(input));

  // Send complete effect data to device if this effect has not been sent yet
  if (!(peb->state & SentToDevice)) {
    ffb_sendSysEx((const uint8_t*) gDevice.msg, midi_data_len);
    peb->state |= SentToDevice;
  }

}

enum loadStatus {
  Success = 1,
  Full    = 2,
  Error   = 3  
};

uint16_t ffb_onCreateEffect(const void *buf, uint8_t **response_ptr)
{  
  const FFBReport_CreateEffect_Feature_t *input = buf;
  FFBReport_PIDBlockLoad_Feature_t *output = &gDevice.blockLoadReport;

  output->reportId = FFB_FEATURE_BLOCK_LOAD_ID;
#ifdef SUPPORT_CUSTOM_FORCE
  if (input->effectType > FFB_EFFECT_CUSTOM)
#else
  if (input->effectType > FFB_EFFECT_FRICTION)
#endif
  {
    error_P_nonl(PSTR("CREATE(%u)"), input->effectType);
    printUnsupported(input->effectType);
    output->loadStatus = Error;
  } else {

    struct paramEffectBlock *peb = allocEffectBlock(input->effectType, &output->effectBlockIndex);

    if (output->effectBlockIndex == 0xFF) {
      output->loadStatus = Full;  // 1=Success,2=Full,3=Error
    } else if ( !peb) {
      output->loadStatus = Error;
    } else {
      output->loadStatus = Success;

      peb->effectType = input->effectType;
      peb->duration = FFB_DURATION_INFINITE;
      peb->gain =  gDevice.gainReport.gain;

      /* save input->byteCount for custom effect */
    }
  }
  //output->ramPoolAvailable = gDevice.pool.slotsAvail;
  output->ramPoolAvailable = gDevice.pool.freeMem + (gDevice.pool.slotsAvail * sizeof(union parameterBlock));
  
  if (isLogLevel(DEBUG_LOG_EFFECT) || isLogLevel(DEBUG_CREATE_EFFECT)) {
    error_P_nonl(PSTR("Create<%u> "), output->effectBlockIndex);
    printEffectType(input->effectType);
    error_P(PSTR(" load_stat: %u"), output->loadStatus);
  }
  if (response_ptr)
    *response_ptr = (uint8_t*) output;
  return sizeof(FFBReport_PIDBlockLoad_Feature_t);
}

void ffb_handleSetReport(uint8_t reportID, uint8_t *buf, uint16_t len)
{
#ifndef SAVE_SPACE
  if (len < OutReportLengths[reportID -1]) {
    error_P(("shortReport(%u, %p, %d)"), reportID, buf, len);
  }
#endif
  switch (reportID)
  {
    case PID_OUT_Set_Effect_Report_ID:
      ffb_handleSetEffect(buf);
      break;
    case PID_OUT_Set_Envelope_Report_ID:
      DRV_CALL(SetEnvelope(buf));
      break;
    case PID_OUT_Set_Condition_Report_ID:
      DRV_CALL(SetCondition(buf));
      break;
    case PID_OUT_Set_Periodic_Report_ID:
      DRV_CALL(SetPeriodic(buf));
      break;
    case PID_OUT_Set_Constant_Force_Report_ID:
      DRV_CALL(SetConstantForce(buf));
      break;
    case PID_OUT_Set_Ramp_Force_Report_ID:
      DRV_CALL(SetRampForce(buf));
      break;
    case PID_OUT_Effect_Operation_Report_ID:
      ffb_handleEffectOperation(buf);
      break;
    case PID_OUT_Block_Free_Report_ID:
      ffb_handleBlockFree(buf);
      break;
    case PID_OUT_Device_Control_Report_ID:
      ffb_handleDeviceControl(buf);
      break;
    case PID_OUT_Device_Gain_Report_ID:
      ffb_handleDeviceGain(buf);
      break;
#ifdef SUPPORT_CUSTOM_FORCE
    case PID_OUT_Custom_Force_Data_Report_ID:
      ffb_handleSetCustomForceData(buf);
      break;
    case PID_OUT_Download_Force_Sample_ID:
      ffb_handleSetDownloadForceSample(buf);
      break;
    case PID_OUT_Set_Custom_Force_Report_ID:
      ffb_handleSetCustomForce(buf);
      break;
#endif
    default:
      error_P_nonl(PSTR("SetReport:"));
      printUnsupported(reportID);
      break;
  }
}

// Initializes internal state
void ffb_init()
{
  // Initialize states
  memset((void*) & gDevice, 0, sizeof( gDevice));
  gDevice.pool.nextID = 2;
  gDevice.pool.slotsAvail = MAX_EFFECTS - 2;
  gDevice.state = ActuatorsEnabled | ActuatorPower;
  gDevice.pidState.reportId = FFB_PID_STATUS_REPORT_ID;
}
