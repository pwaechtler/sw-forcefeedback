/*
  Force Feedback Joystick
  Generic main file for force feedback joysticks.

  Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
  MIT License

  This file is based on LUFA Libarary for which
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
#ifndef _LUFA_IFACE_H_
#define _LUFA_IFACE_H_

#include <string.h>
#include <stdbool.h>
#include <stdint.h>


#include "Descriptors.h"
#include "board.h"



#define  SW_ID_3DP 1     /* 3DP connected */
#define  SW_ID_PP  2     /*  PP connected */
#define  SW_ID_FFP 3     /* FFP connected */
#define  SW_ID_WHEEL 4    /* FFP Wheel connected */


#define SW_REPSZ_3DP    7                       /* report size for 3DP */
#define SW_REPSZ_FFP    6                       /* report size for PP/FFP */


#ifdef __cplusplus
extern "C" {
#endif


/** Type define for the joystick HID report structure, for creating and sending HID reports to the host PC.
    This mirrors the layout described to the host in the HID report descriptor, in Descriptors.c.
*/
typedef struct
{
  // Joystick Input Report
  int16_t  X;
  int16_t  Y;
  int16_t  Z;
  int8_t  Rz, Rx, Ry;
  uint8_t  Rudder;
  uint8_t  Throttle;
  uint8_t Hat;
  uint16_t Button;

  uint8_t filler[2];
} JoystickReport_t;

struct WheelReport {

  uint16_t  X;
  uint16_t  Y;
  uint16_t  Z;

  /* -127..0..127 */
  int8_t  steering;
  /* -127..0..127 */
  int8_t  throttle;
  int8_t  shifter;
  /* 0..255 */
  uint8_t  accelerator;
  uint8_t  brake;

  uint8_t  Button;  /* bitfield with 8 button states */
};

struct SimulationControlsReport {

  /* -127..0..127 */
  int8_t  steering;
  int8_t  shifter;
  /* -127..0..127 */
//  int8_t  throttle;
  /* 0..255 */
  uint8_t  accelerator;
  uint8_t  brake;

//  uint8_t  Button;  /* bitfield with 8 button states */
};

extern USB_ClassInfo_CDC_Device_t Serial1_CDC_Interface;
extern USB_ClassInfo_HID_Device_t Wheel_HID_Interface;

extern uint8_t sw_id;
extern uint8_t sw_report[];
extern JoystickReport_t joystick_state;

uint16_t Joystick_CreateReport(uint8_t inReportId, void * const storage);

void SetupHardware(void);
void ffb_Task(void);
void taskLoop(void);

uint8_t connectDevice(uint8_t set, uint16_t deviceID);
uint16_t readFFB(const void *buf, uint16_t num);

void ffb_handleGetReport(uint8_t reportID);
void ffb_handleFeatureSetReport(uint8_t reportID, const void *input, uint16_t len);
void DeviceStateChanged(uint16_t m_deviceStateAndID);


void EVENT_USB_connectDevice(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);

#ifdef __cplusplus
}
#endif

#endif
