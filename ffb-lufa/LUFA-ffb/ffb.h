/*
  This code is for Microsoft Sidewinder Force Feedback Pro joystick.
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

#ifndef _FFB_H_
#define _FFB_H_

#include "board.h"
#include "Descriptors.h"


#ifdef __cplusplus
extern "C" {
#endif

#define FFB_DURATION_INFINITE  0

#define FFB_EFFECT_CONSTANT      0x01
#define FFB_EFFECT_RAMP          0x02
#define FFB_EFFECT_SQUARE        0x03
#define FFB_EFFECT_SINE          0x04
#define FFB_EFFECT_TRIANGLE      0x05
#define FFB_EFFECT_SAWTOOTHDOWN  0x06
#define FFB_EFFECT_SAWTOOTHUP    0x07
#define FFB_EFFECT_SPRING        0x08
#define FFB_EFFECT_DAMPER        0x09
#define FFB_EFFECT_INERTIA       0x0A
#define FFB_EFFECT_FRICTION      0x0B
#define FFB_EFFECT_CUSTOM        0x0C

// instead of assuming USB I use "DRV" because the "driver layer" sends it
#define DRV_CMD_START   0x01
#define DRV_CMD_SOLO    0x02
#define DRV_CMD_STOP    0x03

// that's a bit on SetEffect(enableAxis/Direction)
#define FFB_DIRECTION_ENABLE  0x04
// if unprecise: just clockwise (cw) or counter clockwise (ccw)
#define FFB_DIR_PRECISE    0x7f
#define FFB_DIR_UNPRECISE  0x7d
// that's in force_direction
#define FFB_DIR_CW        0x7d
#define FFB_DIR_CCW       0x00

/* on the wheel UP: right, DOWN: left */
#define FFB_DIR_UP   0x40
#define FFB_DIR_DOWN 0x00

#define MAX_MIDI_MSG_LEN 27 
/* enough to hold longest midi message data part, FFP_MIDI_Effect_Basic */

enum PidState {
  Paused            = 0x01,
  ActuatorsEnabled  = 0x02,
  SafetySwitch      = 0x04,
  ActuatorOverride  = 0x08,
  ActuatorPower     = 0x10
};

/* Type Defines: */
/** Type define for the joystick HID report structure, for creating and sending HID reports to the host PC.
    This mirrors the layout described to the host in the HID report descriptor, in Descriptors.c.
*/
// ---- Input Report (from Host view)

typedef struct
{
  uint8_t	reportId;
  uint8_t	status;	// Bits: 0=Device Paused,1=Actuators Enabled,2=Safety Switch,3=Actuator Override Switch,4=Actuator Power
  uint8_t	effectBlockIndex;	// Bit7=Effect Playing, Bit0..7=EffectId (1..40)
} FFBReport_PIDStatus_t;

typedef struct
{
  uint8_t reportId;
  uint8_t gain;
} FFBReport_DeviceGain_Feature_t;

// ---- Output Report (from Host view)
extern const uint8_t NumOfReports;  // closed the gap (9 was unused)
// Lengths of each report type
extern const uint16_t OutReportLengths[];

typedef struct
{
  uint8_t	  reportId;
  uint8_t	  effectBlockIndex;
  uint8_t	  effectType;	// 1..12 (effect usages: 26,27,30,31,32,33,34,40,41,42,43,28)
  uint16_t	duration; // 0..32767 ms
  uint16_t	triggerRepeatInterval; // 0..32767 ms
  uint16_t	samplePeriod;	// 0..32767 ms
  uint8_t	  gain;	// 0..255
  uint8_t	  triggerButton;	// button ID (0..8)
  uint8_t	  enableAxis; // bits: 0=X, 1=Y, 2=DirectionEnable (PID force Axes)
  uint8_t	  directionX;	// angle (0=0 .. 255=360deg)
#ifndef REPORT_ONLY_1AXIS
  uint8_t	  directionY;	// angle (0=0 .. 255=360deg)
#endif
  uint16_t	startDelay;	// 0..32767 ms
} FFBReport_SetEffect_t;

  struct envelope {
    uint8_t  attackLevel;
    uint8_t  fadeLevel;
    uint16_t attackTime;  // ms
    uint16_t fadeTime;  // ms
  };
typedef struct
{
  uint8_t	 reportId;
  uint8_t	 effectBlockIndex;
  struct   envelope p;
} FFBReport_SetEnvelope_t;

#if 0
  struct condition {
    int16_t   cpOffset;  // -10000..10000
    int16_t   positiveCoefficient;  // -10000..10000
    int16_t   negativeCoefficient;  // -10000..10000
    uint16_t  positiveSaturation; // 0..10000
    uint16_t  negativeSaturation; // 0..10000
    int16_t   deadBand; // -10000..10000
  } param;
#else
  struct condition {
    int8_t   cpOffset; // -128..127
    uint8_t  positiveCoefficient;  // 0..100
    uint8_t  negativeCoefficient;  // 0..100
    uint8_t  positiveSaturation; // 0..100
    uint8_t  negativeSaturation; // 0..100
    uint8_t  deadBand; // 0..100
  };
#endif

typedef struct
{
  uint8_t   reportId;
  uint8_t   effectBlockIndex; // 1..40
  uint8_t   parameterBlockOffset; // bits: 0..3=parameterBlockOffset, 4..5=instance1, 6..7=instance2
  struct    condition p;
} FFBReport_SetCondition_t;

  struct periodic {
    uint8_t   amplitude;
    int8_t    offset;
    uint8_t   phase;  // 0..255 (=0..359, exp-2)
    uint16_t  period; // 0..32767 ms
  };

typedef struct
{
  uint8_t	  reportId;
  uint8_t	  effectBlockIndex;
  struct    periodic p;
} FFBReport_SetPeriodic_t;

  struct constant {
    int16_t magnitude;  // -10000..10000
  };
typedef struct
{
  uint8_t	reportId;
  uint8_t	effectBlockIndex;
  struct  constant p;
} FFBReport_SetConstantForce_t;

  struct ramp {
    int8_t  start;
    int8_t  end;
  };
typedef struct
{
  uint8_t	reportId;
  uint8_t	effectBlockIndex;
  struct  ramp p;
} FFBReport_SetRampForce_t;

typedef struct
{
  uint8_t	reportId;
  uint8_t	effectBlockIndex;
  uint8_t dataOffset;
  int8_t	data[12];
} FFBReport_SetCustomForceData_Output_t;

typedef struct
{
  uint8_t	reportId;
  int8_t	x;
  int8_t	y;
} FFBReport_SetDownloadForceSample_Output_t;

typedef struct
{
  uint8_t	reportId;
  uint8_t effectBlockIndex;
  uint8_t operation; // 1=Start, 2=StartSolo, 3=Stop
  uint8_t	loopCount;
} FFBReport_EffectOperation_Output_t;

typedef struct
{
  uint8_t	reportId;
  uint8_t effectBlockIndex;
} FFBReport_BlockFree_Output_t;

typedef struct
{
  uint8_t	reportId;
  uint8_t control;	// 1=Enable Actuators, 2=Disable Actuators, 4=Stop All Effects, 8=Reset, 16=Pause, 32=Continue
} FFBReport_DeviceControl_Output_t;

typedef struct
{
  uint8_t	reportId;
  uint8_t gain;
} FFBReport_DeviceGain_Output_t;

typedef struct
{
  uint8_t   reportId;
  uint8_t   effectBlockIndex;
  uint8_t   sampleCount;
  uint16_t	samplePeriod;	// 0..32767 ms
} FFBReport_SetCustomForce_Output_t;

// ---- Features

/*
 * when using LUFA Callbacks for handling this "feature" report: 
 * the first byte with the reportID is skipped but passed as parameter
 */
typedef struct
{

  //	uint8_t		reportId;
  uint8_t	  effectType;	// Enum (1..12): ET 26,27,30,31,32,33,34,40,41,42,43,28
  uint16_t	byteCount;	// 0..511 for custom effect
} FFBReport_CreateEffect_Feature_t;

typedef struct
{
  uint8_t	 reportId;
  uint8_t  effectBlockIndex;
  uint8_t	 loadStatus;	// 1=Success,2=Full,3=Error
  uint16_t ramPoolAvailable;	//how much of the "device memory pool" is free
} FFBReport_PIDBlockLoad_Feature_t;

#ifdef NEW_PID_POOL_REPORT
typedef struct
{
  uint8_t   reportId;
  uint16_t  ramPoolSize;  // for custom effect: the sample data must be stored
#ifdef PID_POOL_INCLUDES_ROMPOOL
  uint16_t  romPoolSize;
  uint16_t  romBlockCount;
#endif
//#ifdef DRIVER_MANAGED_POOL
  uint8_t   set_Effect_Size;
  uint8_t   set_Envelope_Size;
  uint8_t   set_Condition_Size;
  uint8_t   set_Periodic_Size;
  uint8_t   set_Constant_Force_Size;
  uint8_t   set_Ramp_Force_Size;
#ifdef SUPPORT_CUSTOM_FORCE
  uint8_t   set_Custom_Force_Size;
#endif
//#endif //DRIVER_MANAGED_POOL

  uint8_t   maxSimultaneousEffects; // how many the device can "play" at the same time
  uint8_t   capabilities; // Bits: 0=DeviceManagedPool, 1=SharedParameterBlocks
} FFBReport_PIDPool_Feature_t;

#else
typedef struct
{
  uint8_t	  reportId;
  uint16_t	ramPoolSize;	// ?
  uint8_t		maxSimultaneousEffects;	// how many the device can "play" at the same time
  uint8_t		capabilities;	          // Bits: 1=DeviceManagedPool, 0=SharedParameterBlocks
} FFBReport_PIDPool_Feature_t;
#endif  //NEW_PID_POOL_REPORT


void ffb_init(void);
void ffb_setDriver(uint8_t id);
void changePidStateOnEffect(uint8_t eid);
void updateEffectStates();
uint8_t checkBlockIndex(uint8_t eid);


// Handle incoming SetReport requests
void ffb_handleSetReport(uint8_t reportID, uint8_t *data, uint16_t len);
void ffb_handleDeviceControl(const void *buf);

// Handle incoming feature requests
uint16_t ffb_onCreateEffect(const void *buf, uint8_t **response_ptr);
uint16_t ffb_onPIDPool(uint8_t **response_ptr);
uint16_t ffb_onBlockLoad(uint8_t **response_ptr);


// Utility to wait any amount of milliseconds.
// Resets watchdog for each 1ms wait.
void waitMs(int ms);

// delay_us has max limits and the wait time must be known at compile time.
// function for making 10us delays that don't have be known at compile time.
// max delay 2560us.
void _delay_us10(uint8_t delay);

void ffb_sendData(const uint8_t *data, uint16_t len);
void ffb_sendData_P(const uint8_t *data, uint16_t len);
void ffb_sendSysEx(const uint8_t* midi_data, uint8_t len);

void printEffectType(uint8_t effectType);


uint16_t Uint16ToMidiUint14(uint16_t inValue);
int16_t  Int8ToMidiInt14(int8_t inValue);
int16_t  Int16ToMidiInt14(int16_t inS16);

uint8_t calcAmplitude(uint8_t drvValue, uint8_t gain);
uint8_t calcForce(int16_t magnitude, uint8_t gain);


void     startEffect(uint8_t id);
void     stopEffect(uint8_t id);
void     stopAllEffects(void);
void     freeEffect(uint8_t id);
void     freeAllEffects(void);



typedef struct {
  uint8_t  command;  // either 0x23 or 0x20  -- start counting checksum from here
  uint8_t  effectType; // 2=sine, 5=Square, 6=RampUp, 7=RampDown, 8=Triangel, 0x12=Constant
  uint8_t  unknown1; // ? always 0x7F
  uint16_t duration;  // unit=2ms
  uint16_t unknown2;  // ? always 0x0000 should be sample_period
  uint16_t direction;
  uint8_t  unknown3[5];  // ? always 7f 64 00 10 4e
  uint8_t  attackLevel;
  uint16_t attackTime;
  uint8_t  magnitude;
  uint16_t fadeTime;
  uint8_t  fadeLevel;
  uint8_t  waveLength; // 0x6F..0x01 => 1/Hz
  uint8_t  unknown5; // ? always 0x00
  uint16_t param1;  // Varies by effect type; Constant: positive=7f 00, negative=01 01, Other effects: 01 01
  uint16_t param2;  // Varies by effect type; Constant: 00 00, Other effects 01 01
} FFP_MIDI_Effect_Basic;

/* start of midi data common for both pro and wheel protocols */
typedef struct {
  uint8_t  command;	// 0x23 for pro, 0x20 for wheel (it's an address, the cmd is the last byte of your sysex header)
  uint8_t  effectType;	// different enumeration for pro/wheel
  uint8_t  unknown1;	// always 0x7F
  uint16_t duration;	// unit=2ms
} midi_data_common_t;


// Bit-masks for effect states
enum eState {
  Free  =         0x00,
  Allocated    =  0x01,
  SentToDevice =  0x02,
  ToDelay      =  0x04,
  DelayedPlay  =  0x08,
  HasEnvelope  =  0x10,
  Disabled =      0x20,
  Traced      =   0x40,
  
  Playing =       0x80 // keep 0x80 -> PID state report uses this bit as effect is playing
};

//  -- parameter block handling ---------------
/*
 * space savings strategy: only store parameters that are needed for a given effect type
 * each effect uses 1 parameter_effect_block (minimal size 11) and 1 or 2 type spcific blocks
 * the most expensive effect, besides custom force, is e.g. damper with 2 axes:  11+9+9
 * a wave would need: 11+7+7 (effect_param + periodic + envelope)
 * with the old scheme almost 40 bytes (with MAX_MIDI_MESSAGE = 27 + ~12) for every effect are used
 */

// if the platform has more than 64KiB pool mem (e.g. Teensy 4.0)
#ifdef LARGE_RAM
# define OFFSET_TYPE  uint32_t
#else
# define OFFSET_TYPE  uint16_t
#endif


#define MAX_AXES 1

struct paramEffectBlock {
  uint8_t  state;
  uint8_t  gain;
  uint16_t duration;
  // the startStopTime is always needed to know when an effect is over without explicit STOP
  uint32_t startStopTime;  // after EOP(play) delay the play - a loop checks against millis() 
#ifdef SUPPORT_TRIGGER_BUTTON
  uint8_t  triggerButton;
  uint16_t triggerRepeatInterval;
#endif
#ifdef SUPPORT_LOOP_COUNT
  uint8_t  loopCount;
#endif
  uint8_t  effectType :4;
  uint8_t  changedParams :2;
  uint8_t  enableAxis :2;
  uint8_t  directionX;
#ifdef SUPPORT_Y_AXIS
  uint8_t  directionY;
#endif
  uint8_t paramBlocks[MAX_AXES+1];
};

union parameterBlock {
  struct  envelope  envelope;
  struct  condition condition;
  struct  periodic  periodic;
  struct  constant  constant;
  struct  ramp      ramp;
};

struct paramEffectBlock *allocEffectBlock(uint8_t effectType, uint8_t *ret_eid);
struct paramEffectBlock *getEffectBlock(uint8_t eid);
void *getParamBlock(const struct paramEffectBlock *peb, uint8_t index);
void copyParameters(const struct paramEffectBlock *peb, uint8_t index, const void *params );

//  -- /parameter block handling ---------------
// this is the older implementation
#if 0
typedef struct  {
  // lets waste even more RAM
  uint32_t  startStopTime;  // after EOP(play) delay the play 

  // every effect has these properties
  enum eState state;  // >
  uint8_t  drv_effectType;
  uint16_t drv_duration;
  // These are used to calculate effects of USB gain to MIDI data
  uint8_t  drv_gain;

  // parameterBlocks in a pool were intended for this usage
  // depending on the effectType you have a maximum of 2 parameterBlocks or 3 with 2 axes (see pid1_01.pdf 5.1 Table 3)
  // param block envelope
  uint8_t  drv_attackLevel;
  uint8_t  drv_fadeLevel;
  uint16_t drv_fadeTime;  // used to calculate fadeTime to MIDI, since in USB it is given as time difference from the end while in MIDI it is given as time from start
  // param block periodic
  uint8_t  drv_offset;    // periodic x_offset
  // param block constant
  uint8_t  drv_magnitude;

  // with storing the sysex midi msg you hold the "parameters" a second time in (precious) RAM
  // but the bigger problem is: they are set in an inconvenient order  (duration after envelope)
  // so all calculations shall take place on SetEffect-"time"
  union {
    uint8_t  data[MAX_MIDI_MSG_LEN];
    midi_data_common_t midi_common;
    FFP_MIDI_Effect_Basic midi_basic;
  };
} effectState_s;
  effectState_s EffectStates[MAX_EFFECTS + 1];

effectState_s *getEffect(uint8_t);
#endif

struct FfbDevice {
  uint16_t deviceStateAndID;  // hmh, there is no ID.. only state bits
  enum PidState state;
  
  // all paramBlocks have the same size for simplicity and to avoid fragmentation
  struct memPool {
    uint16_t                 freeMem;
    uint8_t                  slotsAvail;
    uint8_t                  nextID;
    struct  paramEffectBlock effects[MAX_EFFECTS];
    union   parameterBlock   paramBlocks[MAX_EFFECTS * (MAX_AXES +1)];
    uint8_t                  used[ ((MAX_EFFECTS * (MAX_AXES +1)) +7 ) / 8 ];
  } pool;
  // one buffer for all effects, the complete msg is assembled in SetEffect and send out
  uint8_t msg[MAX_MIDI_MSG_LEN];  
  uint8_t previousGain;
  FFBReport_DeviceGain_Feature_t gainReport;
  FFBReport_PIDPool_Feature_t  poolReport;
  FFBReport_PIDStatus_t pidState;
  FFBReport_PIDStatus_t previousPidState;
  FFBReport_PIDBlockLoad_Feature_t blockLoadReport;

  uint16_t  setEffectCount; // to allow measurement the frequency of FFB
  uint16_t  sentBytes;  // measure bitrate downwards
  uint16_t  maxSentBytes; // keep the maximum to display on "?" debug cmd
  // for car racing
  struct car_trace {
    uint8_t force;
    uint8_t force_dir;
    uint8_t precise;
    uint8_t friction;
    uint8_t friction_gain;
    uint8_t more_friction;  // add some percent if the app uses a too low range (e.g. 40..65%)
  } car; 
};
extern struct FfbDevice  gDevice;

typedef struct
{
  void (*EnableFFB)(void);
  const uint8_t* (*GetSysExHeader)(uint8_t* hdr_len);
  void (*SetAutoCenter)(uint8_t enable);
  void (*DeviceForceGain)(uint8_t gain);

  void (*startEffect)(uint8_t eid);
  void (*stopEffect)(uint8_t eid);
  void (*freeEffect)(uint8_t eid);

  void (*SetEnvelope)(const void *buf);
  void (*SetCondition)(const void *buf);
  void (*SetPeriodic)(const void *);
  void (*SetConstantForce)(const void *buf);
  void (*SetRampForce)(const void *buf);
  int  (*SetEffect)(const void *buf);
} FFB_Driver;

#ifdef __cplusplus
}
#endif

#endif // _FFB_H_
