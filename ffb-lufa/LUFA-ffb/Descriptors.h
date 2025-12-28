/*
  USB HID descriptor for Microsoft Sidewinder Force Feedback wheel

  Copyright 2022  peewee.hannover [at] gmail [dot] com
  Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com)
  MIT License.
  Some code is based on LUFA Library, for which uses MIT license:
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
#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "LUFAConfig.h"
#include <LUFA.h>

#include <stdint.h>

#include "lufa-iface.h"
#include "debug.h"

/* that's a report IN number space */
#define FFB_HID_REPORT_INPUT    1
#define FFB_PID_REPORT_INPUT    2

// that's a feature "number space"
#define FFB_PID_STATUS_REPORT_ID    2
#define FFB_FEATURE_DEVICE_GAIN_ID  3
#define FFB_FEATURE_NEW_EFFECT_ID   5
#define FFB_FEATURE_BLOCK_LOAD_ID   6
#define FFB_PID_POOL_REPORT_ID      7

/* this is the report out number space */
#define PID_OUT_Set_Effect_Report_ID              1
#define PID_OUT_Set_Envelope_Report_ID            2
#define PID_OUT_Set_Condition_Report_ID           3
#define PID_OUT_Set_Periodic_Report_ID            4
#define PID_OUT_Set_Constant_Force_Report_ID      5
#define PID_OUT_Set_Ramp_Force_Report_ID          6
#define PID_OUT_Effect_Operation_Report_ID        7
#define PID_OUT_Block_Free_Report_ID              8
#define PID_OUT_Device_Control_Report_ID          9
#define PID_OUT_Device_Gain_Report_ID             10
/* the custom force has a FLAG in the PID Pool Report, maybe 
that's only for an additional USB isochronous pipe */
#define PID_OUT_Custom_Force_Data_Report_ID       11
#define PID_OUT_Download_Force_Sample_ID          12
#define PID_OUT_Set_Custom_Force_Report_ID        13

/** Endpoint number of the Joystick HID reporting IN endpoint. */
#define JOYSTICK_EPNUM        1
/** Size in bytes of the Joystick HID reporting IN endpoint. */
#define JOYSTICK_EPSIZE       32
/** Endpoint number of the Joystick HID reporting IN endpoint. */
#define FFB_EPNUM            2
/** Size in bytes of the Joystick FFB HID reporting OUT endpoint. */
#define FFB_EPSIZE           32

/** Endpoint address of the HID interface's device-to-host data IN endpoint. */
#define HID_TX_EPADDR                 (ENDPOINT_DIR_IN  | JOYSTICK_EPNUM)
/** Endpoint address of the HID interface's host-to-device data OUT endpoint. */
#define HID_RX_EPADDR                 (ENDPOINT_DIR_OUT | FFB_EPNUM)
/** Endpoint address of the HID interface's device-to-host notification IN endpoint. */
#define HID_NOTIFICATION_EPADDR       (ENDPOINT_DIR_IN  | 6)


#define CDC1_TX_EPNUM 3
#define CDC1_RX_EPNUM 4
#define CDC1_NOTIFICATION_EPNUM        5

/** Endpoint address of the first CDC interface's device-to-host data IN endpoint. */
#define CDC1_TX_EPADDR                 (ENDPOINT_DIR_IN  | CDC1_TX_EPNUM)
/** Endpoint address of the first CDC interface's host-to-device data OUT endpoint. */
#define CDC1_RX_EPADDR                 (ENDPOINT_DIR_OUT | CDC1_RX_EPNUM)
/** Endpoint address of the first CDC interface's device-to-host notification IN endpoint. */
#define CDC1_NOTIFICATION_EPADDR       (ENDPOINT_DIR_IN  | CDC1_NOTIFICATION_EPNUM)
/** Size in bytes of the CDC device-to-host notification IN endpoints. */
#define CDC_NOTIFICATION_EPSIZE        8
/** Size in bytes of the CDC data IN and OUT endpoints. */
#define CDC_TXRX_EPSIZE                16
/** Size in bytes of the CDC device-to-host notification IN endpoints. */
#define CDC_NOTIFICATION_EPSIZE        8


/** Type define for the device configuration descriptor structure. This must be defined in the
    application code, as the configuration descriptor contains several sub-descriptors which
    vary between devices, and which describe the device's usage to the host.
*/
typedef struct
{
  USB_Descriptor_Configuration_Header_t Config;

  USB_Descriptor_Interface_t            HID_Interface;

  USB_HID_Descriptor_HID_t              HID_JoystickHID;
  USB_Descriptor_Endpoint_t             HID_ReportINEndpoint;
  USB_Descriptor_Endpoint_t             HID_ReportOUTEndpoint;

#ifdef ENABLE_CDC
  // First CDC Control Interface
  USB_Descriptor_Interface_Association_t   CDC1_IAD;
  USB_Descriptor_Interface_t               CDC1_CCI_Interface;
  USB_CDC_Descriptor_FunctionalHeader_t    CDC1_Functional_Header;
  USB_CDC_Descriptor_FunctionalACM_t       CDC1_Functional_ACM;
  USB_CDC_Descriptor_FunctionalUnion_t     CDC1_Functional_Union;
  USB_Descriptor_Endpoint_t                CDC1_ManagementEndpoint;

  // First CDC Data Interface
  USB_Descriptor_Interface_t               CDC1_DCI_Interface;
  USB_Descriptor_Endpoint_t                CDC1_DataOutEndpoint;
  USB_Descriptor_Endpoint_t                CDC1_DataInEndpoint;
#endif // ENABLE_CDC

} USB_Descriptor_Configuration_t;


#ifdef __cplusplus
}
#endif

#endif
