/*
  Device specific code for handling force feedback.
  This code is for Microsoft Sidewinder Force Feedback wheel.

  Copyright 2022  peewee.hannover [at] gmail [dot] com
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

#include "ffb-wheel.h"

#include "debug.h"
#include "board.h"
#include <util/delay.h>

static const uint8_t DIVISOR = 100;

static const PROGMEM uint8_t _sysex_header[] = { 0xf0, 0x00, 0x01, 0x0a, 0x15};
static const PROGMEM uint8_t  ac_attention[] = { 0xf3, 0x1d };
static const PROGMEM uint8_t  ac_level_off[] = { 0xf1, 0x10, 0x40, 0x00, 0x7f, 0x00 };
static const PROGMEM uint8_t     ac_commit[] = { 0xf3, 0x6a };

static const PROGMEM uint8_t drvToDevEffectType[] = {
  0x06, /* Constant */ 0x05, /* Ramp */         0x03, /* Square */     0x02, /* Sine */
  0x04, /* Triangle */ 0x05, /* SawtoothDown */ 0x05, /* SawtoothUp */ 0x08, /* Spring */
  0x09, /* Damper */   0x0a, /* Inertia */      0x0b, /* Friction */   0x0c  /* Custom ? */
};

static uint8_t _mapToDevEffectType(uint8_t drv_etype)
{
  if (--drv_etype >= sizeof(drvToDevEffectType))
    return 0;

  return pgm_read_byte( &drvToDevEffectType[drv_etype] );
}

const uint8_t* Ffbwheel_GetSysExHeader(uint8_t* hdr_len)
{
  *hdr_len = sizeof(_sysex_header);
  return _sysex_header;
}
/*
   these are the pulses sent by Windows98SE running Sidewinder Game Controller Software 3.0 and fedit.exe
   the patent mentions 4 states (DigitalIdle, SendData, SendID, GoAnalog [Fig 4]) but with 5 or 6 pulses the device
   switches through "FFB on" state 
   PW thinks that he has a gen1 gameport wheel that expects pulses on X2 (Y2 is not connected, the pin is missing)
   he bets "skoo" has a >=gen2 gameport wheel with different pulse requirements
   maybe we could check the DeviceID and send different pulses 
   (the DeviceID contains state variables and the meaning is unknown, there is 1 bit for the "override" Force button)
*/
struct ffb_pulses {
  uint8_t  num;
  uint16_t wait;
};
static const PROGMEM struct ffb_pulses gen1_seq[] = { {1, 4000}, {1, 14000}, {5, 4200}, {6, 4200}, {5, 4200}, {3, 450}, {2, 0} };
static const PROGMEM struct ffb_pulses gen1_seq2[] = { {2, 400}, {3, 1300}, {2, 400}, {3, 0} };

static void pulseSequence(const struct ffb_pulses *seq, uint8_t num)
{
  for (uint8_t i = 0; i < num; i++) {
    uint8_t c = pgm_read_byte(&seq[i].num);
    noInterrupts();
    for (uint8_t n = 0; n < c; n++) {
      PulseAndWait(190);
    }
    interrupts();
    uint16_t wait = pgm_read_word(&seq[i].wait);
    if (wait < 1000) {
      delayMicroseconds(wait);
    } else if (wait) {
      delay(wait / 1000);
      delayMicroseconds(wait % 1000);
    }
  }
}

static void  wheel_gen1_pulses()
{
  log_P(PSTR("gen1_pulses"));

  pulseSequence(gen1_seq, sizeof(gen1_seq) / sizeof(struct ffb_pulses));
  delay(278); // 278 ms measured
   board_ffb_putc((uint8_t) 0xF3);
   board_ffb_putc((uint8_t) 0x1d);
  delayMicroseconds(50);

  pulseSequence(gen1_seq2, sizeof(gen1_seq2) / sizeof(struct ffb_pulses));
}


void Ffbwheel_SetAutoCenter(uint8_t enable)
{
  ffb_sendData_P(ac_attention, sizeof(ac_attention));

  if (!enable) {
    ffb_sendData_P(ac_level_off, sizeof(ac_level_off));
    waitMs(100);
    ffb_sendData_P(ac_commit, sizeof(ac_commit));
  }
}

// modify operation ---------------------------------------------------------

/* the poke address skips common "command", effectType, unknown/unaligner (would be 3 - but that's not working)
 * it looks like they just count the fields and have another table to translate that into an offset 
 */

/* POKE1 only pokes 1 byte (f1[4]) if the second value (f1[5])is zero */
#define POKE_1(s,f) (0x40 | offsetof(s,f) -3)
#define POKE_2(s,f) (offsetof(s,f) -3)

static void modifyAttribute(uint8_t effectId, uint8_t address, uint16_t value)
{
  cmd_f1_t op;

  op.cmd = 0xf1;
  op.flag_address = address;
  op.effect_id = effectId;
  op.value = value;

  uint8_t* d = (uint8_t*) &op;
  uint8_t sum = d[0] + (d[2] & ~0x40) + d[3] + d[4] + d[5];
  op.checksum = (0x80 - sum) & 0x7f;

  ffb_sendData(d, sizeof(op));
}

/*
 * On Effects "0" and "1" configuration is stored
 * Effect 0: how strong is autocenter spring effect?
 * Effect 1: how strong is the force/gain?
 */

/* these are F1 cmds, and are built with modifyAttribute */
static void setForceGainLevel(uint8_t level, uint8_t delay_after_first)
{
  static const PROGMEM uint8_t strength[] = { 0x57, 0x6a, 0x7d };
  static const PROGMEM uint8_t   coeff1[] = { 0x44, 0x49 ,0x4e};
  static const PROGMEM uint8_t   coeff2[] = { 0x39, 0x34 ,0x2f};
  static const PROGMEM uint8_t   coeff3[] = { 0x26, 0x13 ,0x00};
  uint16_t v;
  
  if (level > 3)
    level = 3;

  if ( !level) {
    ;
  } else {
    level--;
    v = pgm_read_byte(&strength[level]) << 8;
    modifyAttribute(1, 0x43, v );
    if (delay_after_first) delay(delay_after_first);
    v = (pgm_read_byte(&coeff1[level]) << 8) | 0x3e;
    modifyAttribute(1, 0x44, v);
    v = (pgm_read_byte(&coeff2[level]) << 8) | 0x3e;
    modifyAttribute(1, 0x45, v);
    v = (pgm_read_byte(&coeff3[level]) << 8) | 0x7d;
    modifyAttribute(1, 0x06, v);
  }
}

static void init_sequence()
{
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  waitMs(50);
  setForceGainLevel(3, 0);
}

/**
   Initialize wheel for FF. Releases spring effect.

   Force Editor with Windows XP and gameport sends
   X1 & X2 pulse groups during initialization, but those are not (always?) needed for enabling FF - 
*/
void Ffbwheel_EnableFFB(void)
{
  waitMs(100);
//  wheel_gen1_pulses();
  init_sequence();
  waitMs(100);
  Ffbwheel_SetAutoCenter(0);
  waitMs(50);
  // TODO: configurable, store in EEPROM and reload
#ifdef SUPPORT_ADJUST_CENTER_SPRING
  Ffbwheel_adjustCenterSpring(SUPPORT_ADJUST_CENTER_SPRING);
#endif
}


// effect operations ---------------------------------------------------------

static void wheel_EffectOp(uint8_t eid, uint8_t operation)
{
  cmd_f2_t op;

  op.cmd = 0xf2;
  op.effect_id = eid;
  op.operation_and_checksum = operation << 4;

  uint8_t sum = 0xf ^ 0x2 ^ (op.operation_and_checksum >> 4)
                ^ (op.effect_id >> 4) ^ (op.effect_id & 0x0f);

  op.operation_and_checksum &= 0xf0;
  op.operation_and_checksum |= sum;

  ffb_sendData((const uint8_t*) &op, sizeof(op));
}

void Ffbwheel_startEffect(uint8_t eid)
{
  wheel_EffectOp(eid, 2);
}

void Ffbwheel_stopEffect(uint8_t eid)
{
  wheel_EffectOp(eid, 3);
}

void Ffbwheel_freeEffect(uint8_t eid)
{
  wheel_EffectOp(eid, 1);
}

static void delete_chan_macros(uint8_t last_chan)
{
  uint8_t chan;
  uint8_t chan_del[3];
  chan_del[1] = 0x40;
  chan_del[2] = 0x0;
  
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  for (chan = 0xB0; chan <= last_chan; chan ++) {
    chan_del[0] = chan;
    ffb_sendData(chan_del, sizeof(chan_del));
  }
  delay(100);  // it's far longer measured: 1870ms insane! 
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  delay(37);
}


/*
 * looks like they have to be "morsed": a sequence with repetition and delays and other obfuscating
 */
void Ffbwheel_adjustForceGain(uint8_t level)
{
  if (level > 3)
    level = 3;

  if (level == 3) {
    setForceGainLevel(level -1, 36);
    setForceGainLevel(level, 0);
    setForceGainLevel(level, 0);
    setForceGainLevel(level, 0);
 }

  setForceGainLevel(level, 0);

  delete_chan_macros(level == 3 ? 0xB5: 0xB4);
  setForceGainLevel(level, 0);
  setForceGainLevel(level, 0);
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  delay(44);
  modifyAttribute(0, 0x40, (uint16_t) 0x002a );
  ffb_sendData_P(ac_commit, sizeof(ac_commit));
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  modifyAttribute(0, 0x40, (uint16_t) 0x002a );
  delay(36);
  
  setForceGainLevel(level,  0);
  setForceGainLevel(level, level == 2 ? 36:0);
  
}

void Ffbwheel_DeviceForceGain(uint8_t gain)
{
#if 0  
  uint8_t level;
  /*
     they are just repeated between (ac_on and ac_off)?
     maybe they were floppy driver programmers or morse fans...
  */
  //TODO trace again & check
  
  level = 1;
  setForceGainLevel(level, 0);
  delay(level * 100);
  setForceGainLevel(level, 0);
  delay(level *100);
  setForceGainLevel(level, 0);
  delay(level *100);
  setForceGainLevel(level, 0);
#endif
  if (gain > 127)
    gain = 127;
  gDevice.gainReport.gain = gain;
}

#ifdef SUPPORT_ADJUST_CENTER_SPRING

void Ffbwheel_adjustCenterSpring(uint8_t level)
{
  uint8_t strength[] = { 0x00, 0x10, 0x2a, 0x3e };
  
  if (level > 3)
    level = 3;

  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  // address/offset 0x40 would be "duration" maybe something like PWM?
  modifyAttribute(0, 0x40, (uint16_t) strength[level] );
  delay(36);
  ffb_sendData_P(ac_commit, sizeof(ac_commit));
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  ffb_sendData_P(ac_attention, sizeof(ac_attention));
  modifyAttribute(0, 0x40, (uint16_t) strength[level] );
}
#endif

static void applyTypeSpecificEnvelope(struct paramEffectBlock *peb)
{
  switch (peb->effectType) {
    case FFB_EFFECT_CONSTANT: {
        cmd_f0_constant_force_t *msg = (cmd_f0_constant_force_t *) gDevice.msg;
        // "defaults" in case no envelope set
        msg->e_y1 = msg->e_y2 = 0x7f;
        msg->e_x1 = 0x0000; // left
        msg->e_x2 = 0x1e6e; // right magic
        msg->precise_dir = FFB_DIR_UNPRECISE;
      } break;
    case FFB_EFFECT_SQUARE:
    case FFB_EFFECT_SINE:
    case FFB_EFFECT_TRIANGLE:
    case FFB_EFFECT_SAWTOOTHDOWN:
    case FFB_EFFECT_SAWTOOTHUP:
    case FFB_EFFECT_RAMP:
      {
        cmd_f0_wave_t *msg = (cmd_f0_wave_t *) gDevice.msg;
        // "defaults" in case no envelope set on wave forms what fedit does
        msg->e_y1 = msg->e_y2 = 0x7f;
        msg->e_x1 = 0x0000;

        msg->precise_dir = FFB_DIR_PRECISE;
        msg->p_y_offset = 0x3e; // 62 d is that level zero ((127-3)/2)?

        if (peb->effectType == FFB_EFFECT_RAMP) {
          msg->p_x_offset = 0x0000;
          msg->e_x2 = 0x1e6e;  //right
          msg->p_t = 0x1e6e;  //7790
          msg->common.direction = 0x40;  //TODO
        } else {
          msg->p_x_offset = 0x4000; // 0x00 0x40
          msg->e_x2 = 0x1265;
          msg->p_t = 0x0374;
        }
      }
      break;
  }
}

/**
 * on constant force direction zero is rotated to the left
 */
static uint8_t convertDirection(uint8_t directionX)
{
  uint8_t new_dir;
  uint16_t scaled;

  // Convert direction: zero is the new left (270/2)
  scaled = (uint16_t) directionX * 4 / 3; // scale from 0..45 to 0..60
  // looks good in fedit - try in Dirt2/Dirt 2.0 - dirX stays always to 45
  if ( /* >= 0 && */ (directionX <= 45))
     new_dir = scaled + 65;     // right 65..125
  else if ((directionX > 45) && (directionX <= 90))
    new_dir = 184 - scaled;      // right 124..64
  else if ((directionX > 90) && (directionX <= 135))
    new_dir = 180 - scaled;     // left 60..0
  else // 135 <= f_dir <= 180 
    new_dir = scaled - 180;    // left 0..60
  return new_dir;   
}

static void applyFriction(uint8_t eid, struct paramEffectBlock *peb)
{
  struct condition *params = getParamBlock(peb, 0);
  uint8_t posCoeff;

  /* if this value gets too low (zero is definetly too low): pid state goes to 0x6xxx 
   *  and the wheel turns from left to right 
   */
  if (params->positiveCoefficient < 50)
    params->positiveCoefficient = 50;
  // a calculated value is send as the only 1-byte parameter on SetEffect - normalized means in percent
  posCoeff = (127 * params->positiveCoefficient) / DIVISOR;
  posCoeff += gDevice.car.more_friction;
  
  posCoeff &= 0x7f;
  gDevice.car.friction = posCoeff;
  
  if (isLogLevel(DEBUG_LOG_EFFECT)) {
      printFriction( gDevice.car.friction, gDevice.car.friction_gain);
  }
  if (peb->state & SentToDevice) {
      // this is never 6 - I bet it's 3
#ifdef SEND_ONLY_CHANGED_PARAMS
    if (peb->changedParams) {
#endif
      modifyAttribute(eid, 0x43, posCoeff);  // at least the force changes ;)
      modifyAttribute(eid, 0x46, posCoeff);
#ifdef SEND_ONLY_CHANGED_PARAMS
    }
#endif
  } else {
    cmd_f0_friction_t *msg = (cmd_f0_friction_t *) gDevice.msg;
    msg->posCoeff = posCoeff;
  }
}

/* pid1_01.pdf chapter 5.3
 * If the metric A(q -q0) is less than CP Offset - Dead Band, then the resulting force
 * is given by the following formula:
 * force = Positive Coefficient * (q - (CP Offset + Dead Band))
 * otherwise
 * force = Negative Coefficient * (q - (CP Offset – Dead Band))
 * A spring condition uses axis position as the metric.
 * A damper condition uses axis velocity as the metric.
 * An inertia condition uses axis acceleration as the metric.
 */

void Ffbwheel_SetCondition(const void *buf)
{
  const FFBReport_SetCondition_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
  const struct condition *params = &input->p;

#ifndef SAVE_SPACE
  if (isLogLevel(DEBUG_LOG_EFFECT) || isLogLevel(DEBUG_LOG_COND)) {
    log_P(PSTR("SetCondition<%u> paramBlock: 0x%x"), input->effectBlockIndex, input->parameterBlockOffset);
    printCondition( params->positiveCoefficient, params->positiveSaturation, params->cpOffset,
                    params->negativeCoefficient, params->negativeSaturation, params->deadBand);
  }
#endif
  if (!checkBlockIndex(eid)) {
    return;
  }
  struct paramEffectBlock *peb = getEffectBlock(eid);

  /* the wheel has only one axis for FFB, but FEdit complains if only one is offered */
  if (input->parameterBlockOffset == 0x0) {
    struct condition *old = getParamBlock(peb, 0);
    if (memcmp(input, old, sizeof(*params) ))
      peb->changedParams = 1;
    copyParameters(peb, 0, params);
//    only for fedit not sending SetEffect if properties changed    
//    if (peb->effectType == FFB_EFFECT_FRICTION)
//      applyFriction(eid, peb);
  } else {
    copyParameters(peb, 1, params);
  }
}

/* damper f0 00 01 0a 15 20 09 7f 6e 1e 00 00 7d 3e 3f 3e 3f 7d 00  58 f7 */
/* spring f0 00 01 0a 15 20 08 7f 6e 1e 00 00 7d 3e 4e 3e 2f 7d 00  5a f7 */

// so this is a NOP;  the structure is just a guess
// parameters are calculated (maybe "rotated") and combined
// PosCoeff changes negSat, deadBand changes this and that
// can someone shed some light on this?
static void applyCondition(uint8_t eid, struct paramEffectBlock *peb /* ,uint8_t axis */)
{
  struct condition *c = getParamBlock(peb, 0);

  if (isLogLevel(DEBUG_LOG_EFFECT)) {
    printEffectType(peb->effectType);
    error_P_nonl(PSTR("<%u> "), eid);
    printCondition(c->positiveCoefficient, c->positiveSaturation, c->cpOffset, 
                   c->negativeCoefficient, c->negativeSaturation, c->deadBand);
  }

  if (peb->state & SentToDevice) {
  } else {
    cmd_f0_condition *msg = (cmd_f0_condition *) gDevice.msg;
    msg-> posCoeff = 0;
    msg-> negSat = 0x7d;
    msg-> deadBand1 = 0x3e;
    msg-> negCoeff = 0x3f;
    msg-> deadBand2 = 0x3e;      
    msg-> magic = 0x3f;
    msg-> posSat = 0x7d;
    msg-> cpOffset = 0;
    if (peb->effectType == FFB_EFFECT_SPRING)   
      msg-> magic = 0x2f;
  }
}

/**
  uint8_t   precise_dir;    //3       field 3
  uint16_t  p_x_offset;     // 4+5    field 4
  uint8_t   e_y1;           // 6      field 5
  uint16_t  e_x1;           // 7+8    field 6
  uint8_t   p_amplitude;    // 9      field 7
  uint16_t  e_x2;           // a+b    field 8
  uint8_t   e_y2;           // c      field 9
  uint16_t  p_t;            // d+e    field 0a
  uint8_t   p_y_offset;     // f      field 0b
*/
const char PROGMEM periodic_fmt_head[] = "SetPeriod<%u>:";

static void applyPeriod(uint8_t eid, struct paramEffectBlock *peb)
{
  struct periodic *params = getParamBlock(peb, 0);
  uint16_t period, x_offset;
  uint8_t amplitude, direction = 0x40, preciseFlag = FFB_DIR_UNPRECISE;
  int8_t y_offset;
  
  if (params->period > 10000)
    period = 10000;
  else if (params->period < 150)
    period = 150;
  else
    period = Uint16ToMidiUint14(params->period);

  //x_offset = Uint16ToMidiUint14(params->phase);
  x_offset = 0x4000;    // another magic number
  
  if (peb->enableAxis) {
    if (peb->directionX <= 90)
      direction = FFB_DIR_UP;
    else
      direction = FFB_DIR_DOWN;
    preciseFlag = FFB_DIR_UNPRECISE;
  }
  
  // Calculate min-max from magnitude and offset
  // here params->magnitude is uint8_t
//  y_offset = params->offset;
  y_offset = 0x3e;  // that means level zero ?
  amplitude = calcAmplitude(params->amplitude, peb->gain);
  
  if (isLogLevel(DEBUG_LOG_EFFECT)) {
    printEffectType(peb->effectType);
    error_P_nonl(PSTR("<%u> "), eid);
    printPeriodic( amplitude, period, x_offset, y_offset);
  }
  if (peb->state & SentToDevice) {
//    modifyAttribute(eid, 0x04, x_offset);
    modifyAttribute(eid, 0x42, direction);
    modifyAttribute(eid, 0x47, amplitude);
    modifyAttribute(eid, 0x0A, period);
    modifyAttribute(eid, 0x4B, y_offset);
  } else {
    cmd_f0_wave_t *msg = (cmd_f0_wave_t *) gDevice.msg;
    msg->common.direction = direction;
    msg->precise_dir = preciseFlag;
    msg->p_x_offset = x_offset;
    msg->p_amplitude = amplitude;
    msg->p_y_offset = y_offset;
    msg->p_t = period;
  }
}

/* ramp   f0 00 01 0a 15 20 05 7f 6e 1e 40 7f 00 00 7f 00 00 7f 6e 1e 7f 6e 1e 3e  3e f7 */

static void applyRamp(uint8_t eid, struct paramEffectBlock *peb)
{
  struct ramp *params = getParamBlock(peb, 0);
  uint8_t delta, midpoint, preciseFlag, direction;
  
  if (peb->enableAxis) {

    // this direction comes on SetEffect(dir/Axis enable dirX and is "precise"?)
    if (peb->directionX <= 90)
      direction = FFB_DIR_UP;
    else
      direction = FFB_DIR_DOWN;
    preciseFlag = FFB_DIR_PRECISE;
  } else {
    // start&end are signed in range -128..127    
    if (params->end < params->start)
      direction = FFB_DIR_UP;   // increasing
    else
      direction = FFB_DIR_DOWN;   // decreasing
    preciseFlag = FFB_DIR_UNPRECISE;
  }  
  delta = abs((int)params->end - params->start);
  midpoint = abs( params->start - (delta/2));
  
  if (peb->state & SentToDevice) {
    modifyAttribute(eid, 0x42, direction);
    modifyAttribute(eid, 0x43, preciseFlag);
    modifyAttribute(eid, 0x47, delta);
    modifyAttribute(eid, 0x4b, midpoint);
  } else {
    cmd_f0_wave_t *msg = (cmd_f0_wave_t *) gDevice.msg;
    msg->common.direction = direction;
    msg->precise_dir = preciseFlag;
    msg->p_amplitude = delta;
    msg->p_y_offset  = midpoint;
    msg->p_t         = 0x1e6e;    // another magic number
  }
}

static void applyEnvelopeOnWave(uint8_t eid, struct paramEffectBlock *peb)
{
  if (peb->state & HasEnvelope) {
    struct envelope *params = getParamBlock(peb, 1);
    uint8_t attackLevel, fadeLevel;
    uint16_t attackTime, fadeTime;
      
    attackLevel = calcAmplitude(params->attackLevel, peb->gain);
    fadeLevel   = calcAmplitude(params->fadeLevel, peb->gain);
    attackTime  = Uint16ToMidiUint14(params->attackTime);
    fadeTime    = Uint16ToMidiUint14(params->fadeTime);
    
    if (peb->duration > params->fadeTime)
      fadeTime = Uint16ToMidiUint14(peb->duration - params->fadeTime);
        
    if (isLogLevel(DEBUG_LOG_EFFECT)) {
      printEffectType(peb->effectType);
      error_P_nonl(PSTR("<%u> "), eid);
      printEnvelope( attackLevel, fadeLevel, attackTime, fadeTime);
    }
    if (peb->state & SentToDevice) {
      modifyAttribute(eid, 0x45, attackLevel);
      modifyAttribute(eid, 0x06, attackTime);
      modifyAttribute(eid, 0x08, fadeTime);
      modifyAttribute(eid, 0x49, fadeLevel);
    } else {
      cmd_f0_wave_t *msg = (cmd_f0_wave_t *) gDevice.msg;
      msg->e_y1 = attackLevel;
      msg->e_x1 = attackTime;
      msg->e_x2 = fadeTime;
      msg->e_y2 = fadeLevel;
    }
  }
}

void Ffbwheel_SetRampForce(const void *buf)
{
  const FFBReport_SetRampForce_t* input = buf;
  uint8_t eid = input->effectBlockIndex;
  struct paramEffectBlock *peb = getEffectBlock(eid);
  const struct ramp *params = &input->p;

  if (isLogLevel(DEBUG_LOG_EFFECT)) {
    log_P_nonl(PSTR("SetRamp<%u>: "), eid);
    printRamp(params->start, params->end);
  }
  if (!checkBlockIndex(eid))
    return;
  copyParameters(peb, 0, params);  
}

void Ffbwheel_SetPeriodic(const void *buf)
{
  const FFBReport_SetPeriodic_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
  struct paramEffectBlock *peb = getEffectBlock(eid);
  const struct periodic *params = &input->p;

  if (isLogLevel(DEBUG_LOG_EFFECT)) {
    log_P_nonl(periodic_fmt_head, eid);
    printPeriodic( params->amplitude, params->period, params->phase, params->offset);
  }
  if (!checkBlockIndex(eid))
    return;
  copyParameters(peb, 0, params);
  applyPeriod(eid, peb);
}

void Ffbwheel_SetEnvelope(const void *buf)
{
  const FFBReport_SetEnvelope_t* input = buf;
  uint8_t eid = input->effectBlockIndex;
  const struct envelope *params = &input->p;

  if (isLogLevel(DEBUG_LOG_EFFECT)) {
    log_P_nonl(PSTR("SetEnvlop<%u> "), eid);
    printEnvelope( params->attackLevel, params->fadeLevel, params->attackTime, params->fadeTime);
  }
  if (!checkBlockIndex(eid))
    return;
  struct paramEffectBlock *peb = getEffectBlock(eid);
#ifdef SEND_ONLY_CHANGED_PARAMS
  struct envelope *old = getParamBlock(peb, 1);
  if (memcmp(input, old, sizeof(*params) ))
    peb->changedParams = 2;
#endif
  copyParameters(peb, 1, params);
  peb->state |= HasEnvelope;
}

static void applyEnvelopeOnConstant(uint8_t eid, struct paramEffectBlock *peb)
{
  if (peb->state & HasEnvelope) {
    struct envelope *params = getParamBlock(peb, 1);
    uint8_t attackLevel, fadeLevel;
    uint16_t attackTime, fadeTime;
    
    attackLevel = calcAmplitude(params->attackLevel, peb->gain);
    fadeLevel   = calcAmplitude(params->fadeLevel, peb->gain);

    attackTime = Uint16ToMidiUint14(params->attackTime);
    fadeTime   = Uint16ToMidiUint14(params->fadeTime);
    
    if (fadeTime == 0)
      fadeTime = 0x16e6;  // right magic

    if (isLogLevel(DEBUG_LOG_EFFECT)) {
      printEffectType(peb->effectType);
      error_P_nonl(PSTR("<%u> "), eid);
      printEnvelope( attackLevel, fadeLevel, attackTime, fadeTime);
    }
    if (peb->state & SentToDevice) {
      if (peb->changedParams == 2) {
        modifyAttribute(eid, 0x44, attackLevel); 
        modifyAttribute(eid, 0x45, attackTime);
        modifyAttribute(eid, 0x47, fadeTime);
        modifyAttribute(eid, 0x48, fadeLevel);
      }
    } else {
      cmd_f0_constant_force_t *msg = (cmd_f0_constant_force_t *) gDevice.msg;
      msg->e_y1 = attackLevel;
      msg->e_x1 = attackTime;
      msg->e_x2 = fadeTime;
      msg->e_y2 = fadeLevel;      
    }
  }
}

static void applyConstant(uint8_t eid, struct paramEffectBlock *peb)
{
  struct constant *params = getParamBlock(peb, 0);
  uint8_t force, forceDirection, preciseFlag;

  force = calcForce(params->magnitude, peb->gain);

  // force_direction is set by SetEffect(dur, .. enableAxes/Dir, dirX)
  // but it _has_ to be ignored
//  if (peb->enableAxis) {
//    preciseFlag = FFB_DIR_PRECISE;
//    forceDirection = convertDirection(peb->directionX);
//  }

    preciseFlag = FFB_DIR_UNPRECISE;
    if (params->magnitude >= 0) {
      forceDirection = FFB_DIR_CW;
    } else {
      forceDirection = FFB_DIR_CCW;
    }
  if (isLogLevel(DEBUG_LOG_EFFECT)) {
      log_P_nonl(PSTR("SetConstantF<%u>:\tmag:%d "), eid, params->magnitude);
      printForce(force, forceDirection, preciseFlag);
      log_crlf();
  }
  if (peb->state & SentToDevice) {
#ifdef SEND_ONLY_CHANGED_PARAMS
    if (peb->changedParams) {
#endif
//      if (peb->enableAxis) {
//        modifyAttribute(eid, 0x43, preciseFlag);
//      }
      modifyAttribute(eid, 0x49, forceDirection);
      modifyAttribute(eid, 0x46, force);
#ifdef SEND_ONLY_CHANGED_PARAMS
    }
#endif
  } else {
    cmd_f0_constant_force_t *msg = (cmd_f0_constant_force_t *) gDevice.msg;
    msg->force = force;
    msg->precise_dir = preciseFlag;
    msg->force_direction = forceDirection;
  }
  gDevice.car.force = force;
  gDevice.car.precise = preciseFlag;
  gDevice.car.force_dir = forceDirection;
}

void Ffbwheel_SetConstantForce(const void *buf)
{
  const FFBReport_SetConstantForce_t *input = buf;
  uint8_t eid = input->effectBlockIndex;
  const struct constant *params = &input->p;

  if (isLogLevel(DEBUG_LOG_EFFECT)) {
    log_P(PSTR("SetForce<%u> magnitude: %d"), eid, params->magnitude);
  }
  if (!checkBlockIndex(eid))
    return;
  struct paramEffectBlock *peb = getEffectBlock(eid);
#ifdef SEND_ONLY_CHANGED_PARAMS
  struct constant *old = getParamBlock(peb, 0);
  if (memcmp(input, old, sizeof(*params) ))
    peb->changedParams = 1;
#endif
  copyParameters(peb, 0, params);
  // fedit just calls SetForce if the slider for the magnitude is changed
  applyConstant(eid, peb);
}

static void setCommon(uint8_t effectType)
{
  cmd_f0_common_t *cmsg = (cmd_f0_common_t*) gDevice.msg;

  cmsg->command = 0x20; // always 0x20 for wheel
  cmsg->effect_type = _mapToDevEffectType(effectType);
  cmsg->unknown = 0x7f; // always 0x7f
  cmsg->duration = FFB_DURATION_INFINITE;
  
  // 0: effect play at the default sample period; on Ramp it should be "direction"?
  cmsg->direction = 0x00; // used.. on Ramp/Periodic
}

int Ffbwheel_SetEffect(const void *buf)
{
  const FFBReport_SetEffect_t *input = buf;
  uint8_t msg_len = 0;
  uint8_t eid = input->effectBlockIndex;

  if (isLogLevel(DEBUG_LOG_EFFECT)) {
    log_P_nonl(PSTR("SetEffect<%u>: type: %u dura: %u gain: %u "), 
                    eid, input->effectType, input->duration, input->gain);
    log_P(PSTR("eAxis: 0x%x dir: %u  sDelay: %u ms"), 
                input->enableAxis, input->directionX, input->startDelay);
  }
  if (!checkBlockIndex(eid))
    return 0;

  struct paramEffectBlock *peb = getEffectBlock(eid);
  if (input->enableAxis & FFB_DIRECTION_ENABLE) {
    peb->enableAxis = 1;    // just a bit
    peb->directionX = input->directionX;
  }
  peb->gain = input->gain;
  
  // duration was set one level higher in ffb_handleSetEffect
  if (peb->state & SentToDevice) {
#ifdef SEND_ONLY_CHANGED_PARAMS
    if (peb->changedParams == 0x03)
#endif
      modifyAttribute(eid, 0x00, peb->duration);
  }
  setCommon(input->effectType);
  switch (input->effectType)
  {
    case FFB_EFFECT_RAMP:{
        msg_len = sizeof(cmd_f0_wave_t);
        applyTypeSpecificEnvelope(peb);
        applyEnvelopeOnWave(eid, peb);
        applyRamp(eid, peb);
      } break;
    case FFB_EFFECT_SQUARE:
    case FFB_EFFECT_SINE:
    case FFB_EFFECT_TRIANGLE:
    case FFB_EFFECT_SAWTOOTHDOWN:
    case FFB_EFFECT_SAWTOOTHUP:
        msg_len = sizeof(cmd_f0_wave_t);
        applyTypeSpecificEnvelope(peb);
        applyPeriod(eid, peb);
        applyEnvelopeOnWave(eid, peb);
      break;
    case FFB_EFFECT_CONSTANT:
        msg_len = sizeof(cmd_f0_constant_force_t);
        applyConstant(eid, peb);
        applyTypeSpecificEnvelope(peb);
        applyEnvelopeOnConstant(eid, peb);
      break;
    case FFB_EFFECT_SPRING:
    case FFB_EFFECT_DAMPER:
    case FFB_EFFECT_INERTIA:
      msg_len = sizeof(cmd_f0_condition);
      applyTypeSpecificEnvelope(peb);
      applyCondition(eid, peb);
      break;
    case FFB_EFFECT_FRICTION:
        msg_len = sizeof(cmd_f0_friction_t);
        applyFriction(eid, peb);
        gDevice.car.friction_gain = input->gain;
      break;
    case FFB_EFFECT_CUSTOM:
    default: {
        error_P(PSTR("custom effect"));
      } break;
  }
#ifdef SUPPORT_START_DELAY
  if (input->startDelay > 0) {
    peb->state |= ToDelay;
    peb->startStopTime = input->startDelay;
  }
#endif
  return msg_len;
}
