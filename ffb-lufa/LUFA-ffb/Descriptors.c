/*
  This code is for Microsoft Sidewinder Force Feedback wheel

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


/** \file
 *
 *  USB Device Descriptors, for library use when in USB device mode. Descriptors are special
 *  computer-readable structures which the host requests upon device enumeration, to determine
 *  the device's capabilities and functions.
 */
#include "board.h"
#include "Descriptors.h"

const USB_Descriptor_HIDReport_Datatype_t PROGMEM WheelReportDescriptor[] =
{

0x05, 0x01,           /* Usage Page (Generic Desktop) */ 
0x09, 0x04,           /* Usage (Joystick:4 gamepad:5) */ 
0xA1, 0x01,           /* Collection (Application) */ 

    0x05, 0x01,           /*   Usage Page (Generic Desktop) */ 

    0x85, FFB_HID_REPORT_INPUT,  // REPORT_ID 

    0xA1,0x02,  // COLLECTION (Logical)

      0x05, 0x01,           /*   Usage Page (Generic Desktop) */
      0x09, 0x30,         // Usage X
      0x09, 0x31,        // Usage Y
      0x09, 0x32,       // Usage Z
        HID_RI_LOGICAL_MINIMUM(16, 0),
        HID_RI_LOGICAL_MAXIMUM(16, 1023),
        HID_RI_REPORT_SIZE(8, 0x10),    // 16 bits
        HID_RI_REPORT_COUNT(8, 0x03),
        HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

     0xA1, 0x00,           /*   Collection (Physical) */ 

        0x05, 0x02,           /* Usage Page (Simulation Controls) */
        0x15, 0x81,         /*   Logical Minimum (-127) */ 
        0x26, 0x7F, 0x00,   /*   Logical Maximum (127) */ 
        0x75, 0x08,         /*     Report Size (8) */ 

        0x09, 0xC8,         /*   Usage (Steering) */ 
        0x09, 0xbb,         /*   USAGE (Throttle) */
        0x09, 0xC7,         /*   Usage (Shifter) */ 
        0x95, 0x03,         /*     Report Count (3) */ 
        0x81, 0x42,         /*     Input (variable,absolute, null_state) null_state is important */

        0x15, 0x00,           /*   Logical Minimum (0) */ 
        0x26, 0xFF, 0x00,     /*   Logical Maximum (255) */ 
        0x09, 0xC4,           /*     Usage (Accelerator) */ 
        0x09, 0xC5,           /*     Usage (Brake) */ 
        0x75, 0x08,           /*     Report Size (8) */ 
        0x95, 0x02,           /*     Report Count (2) */ 
        0x81, 0x02,           /*     Input (variable,absolute) */ 

      0xC0,                 /*   End Collection */ 

      0x05, 0x09,           /* Usage Page (Button) */ 
      0x19, 0x01,           /*   Usage Minimum (Button #1) */ 
      0x29, 0x08,           /*   Usage Maximum (Button #8) */ 
      0x15, 0x00,           /*   Logical Minimum (0) */ 
      0x25, 0x01,           /*   Logical Maximum (1) */ 
      0x75, 0x01,           /*   Report Size (1) */ 
      0x95, 0x08,           /*   Report Count (8) */ 
      0x81, 0x02,           /*   Input (variable,absolute) */  

    0xC0,                 /*   End Collection */ 
#if 0
    0x05, 0x02,           /* Usage Page (Simulation Controls) */
    0x85, FFB_PID_REPORT_INPUT,  // REPORT_ID 

      0xA1, 0x00,           /*   Collection (Physical) */ 

        0x75, 0x08,           /*     Report Size (8) */ 
        0x09, 0xC8,         /*     Usage (Steering) */ 
//        0x09, 0xbb,         /*   USAGE (Throttle) */
        0x09, 0xC7,         /*     Usage (Shifter) */ 
        0x15, 0x81,         /*   Logical Minimum (-127) */ 
        0x26, 0x7F, 0x00,   /*   Logical Maximum (127) */ 
        0x95, 0x02,         /*     Report Count (2) */ 
        0x81, 0x02,         /*     Input (variable,absolute) */ 

        0x15, 0x00,           /*   Logical Minimum (0) */ 
        0x26, 0xFF, 0x00,     /*   Logical Maximum (255) */ 
        0x09, 0xC4,           /*     Usage (Accelerator) */ 
        0x09, 0xC5,           /*     Usage (Brake) */ 
        0x95, 0x02,           /*     Report Count (2) */ 
        0x81, 0x02,           /*     Input (variable,absolute) */ 

    0xC0,                 /*   End Collection */ 
#endif

  
  0x05,0x0F,  // USAGE_PAGE (Physical Interface)

  0x09,0x21,  // USAGE (Set Effect Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Set_Effect_Report_ID,  // REPORT_ID (01)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (40)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x25,  // USAGE (25)
    0xA1,0x02,  // COLLECTION (Logical)
      0x09,0x26,  // USAGE (26  ET Constant Force)
      0x09,0x27,  // USAGE (27  ET Ramp)
      0x09,0x30,  // USAGE (30  ET Square)
      0x09,0x31,  // USAGE (31  ET Sine)
      0x09,0x32,  // USAGE (32  ET Triangle)
      0x09,0x33,  // USAGE (33  ET Sawtooth Up)
      0x09,0x34,  // USAGE (34  ET Sawtooth Down)
      0x09,0x40,  // USAGE (40  ET Spring)
      0x09,0x41,  // USAGE (41  ET Damper)
      0x09,0x42,  // USAGE (42  ET Inertia)
      0x09,0x43,  // USAGE (43  ET Friction)
#ifdef SUPPORT_CUSTOM_FORCE
      0x09,0x28,  // USAGE (28 Custom Force)
      0x25,0x0C,  // LOGICAL_MAXIMUM (12)
#else
      0x25,0x0B,  // LOGICAL_MAXIMUM (11)
#endif
      0x15,0x01,  // LOGICAL_MINIMUM (01)
      0x75,0x08,  // REPORT_SIZE (08)
      0x95,0x01,  // REPORT_COUNT (01)
      0x91,0x00,  // OUTPUT (Data)
    0xC0, // END COLLECTION ()
    0x09,0x50,  // USAGE (Duration)
    0x09,0x54,  // USAGE (Trigger Repeat Interval)
    0x09,0x51,  // USAGE (Sample Period)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0x10,0x27, // LOGICAL_MAXIMUM (10000)
    0x35,0x00,  // PHYSICAL_MINIMUM (00)
    0x46,0x10,0x27, // PHYSICAL_MAXIMUM (10000)   46 10 27
    0x66,0x03,0x10, // UNIT (Eng Lin:Time)
    0x55,0xFD,  // UNIT_EXPONENT (-3)
    0x75,0x10,  // REPORT_SIZE (10)
    0x95,0x03,  // REPORT_COUNT (03)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x55,0x00,  // UNIT_EXPONENT (00)   disable
    0x66,0x00,0x00, // UNIT (None)      disable
    0x45,0x00,  // PHYSICAL_MAXIMUM (0)   disable because they are "global"

    0x09,0x52,  // USAGE (Gain)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0xFF,0x00, // LOGICAL_MAXIMUM (00 FF)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    
    0x09,0x53,  // USAGE (Trigger Button)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x08,  // LOGICAL_MAXIMUM (08)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    
    0x09,0x55,  // USAGE (Axes Enable)
    0xA1,0x02,  // COLLECTION (Logical)
      0x05,0x01,  // USAGE_PAGE (Generic Desktop)
      0x09,0x30,  // USAGE (X)
#ifdef REPORT_ONLY_1AXIS
      0x95,0x01,  // REPORT_COUNT (01)
#else
      0x09,0x31,  // USAGE (Y)
      0x95,0x02,  // REPORT_COUNT (02)
#endif
      0x15,0x00,  // LOGICAL_MINIMUM (00)
      0x25,0x01,  // LOGICAL_MAXIMUM (01)
      0x75,0x01,  // REPORT_SIZE (01)
      0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0xC0, // END COLLECTION ()

    0x05,0x0F,  // USAGE_PAGE (Physical Interface)
    0x09,0x56,  // USAGE (Direction Enable)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
#ifdef REPORT_ONLY_1AXIS
    0x95,0x06,  // REPORT_COUNT (06)    padding
#else
    0x95,0x05,  // REPORT_COUNT (05)    padding
#endif
    0x91,0x03,  // OUTPUT (Constant,Var,Abs)

    0x09,0x57,  // USAGE (Direction)
    0xA1,0x02,  // COLLECTION (Logical)
      0x0B,0x01,0x00,0x0A,0x00,
      0x0B,0x02,0x00,0x0A,0x00,
      0x66,0x14,0x00, // UNIT (Eng Rot:Angular Pos)
      0x55,0xFE,  // UNIT_EXPONENT (FE)  -2
      0x15,0x00,  // LOGICAL_MINIMUM (00)
      0x26,0xB4,0x00, // LOGICAL_MAXIMUM (00 B4)  180  hmh
      0x35,0x00,  // PHYSICAL_MINIMUM (00)
      0x47,0xA0,0x8C,0x00,0x00, // PHYSICAL_MAXIMUM (00 00 8C A0) 36000 hmh
      0x66,0x00,0x00, // UNIT (None)   now the unit is gone before OUTPUT
      0x75,0x08,  // REPORT_SIZE (08)
#ifdef REPORT_ONLY_1AXIS
      0x95,0x01,  // REPORT_COUNT (01)
#else
      0x95,0x02,  // REPORT_COUNT (02)
#endif
      0x91,0x02,  // OUTPUT (Data,Var,Abs)
      0x55,0x00,  // UNIT_EXPONENT (00)
      0x66,0x00,0x00, // UNIT (None)
    0xC0, // END COLLECTION ()

    0x09,0xA7,  // USAGE (Start Delay)
    0x66,0x03,0x10, // UNIT (Eng Lin:Time)
    0x55,0xFD,  // UNIT_EXPONENT (-3)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0xFF,0x7F, // LOGICAL_MAXIMUM (7F FF)
    0x35,0x00,  // PHYSICAL_MINIMUM (00)
    0x46,0xFF,0x7F, // PHYSICAL_MAXIMUM (7F FF)
    0x75,0x10,  // REPORT_SIZE (10)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x66,0x00,0x00, // UNIT (None)
    0x55,0x00,  // UNIT_EXPONENT (00)
    0x45,0x00,  // PHYSICAL_MAXIMUM (0)   disable because they are "global"
  0xC0, // END COLLECTION ()
  
  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0x5A,  // USAGE (Set Envelope Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Set_Envelope_Report_ID,  // REPORT_ID (02)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x5B,  // USAGE (Attack Level)
    0x09,0x5D,  // USAGE (Fade Level)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0x7F,0x00, // LOGICAL_MAXIMUM (127)
    0x95,0x02,  // REPORT_COUNT (02)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x5C,  // USAGE (Attack time)
    0x09,0x5E,  // USAGE (Fade time)
    0x66,0x03,0x10, // UNIT (Eng Lin:Time)
    0x55,0xFD,  // UNIT_EXPONENT (-3)
    0x26,0xFF,0x7F, // LOGICAL_MAXIMUM (7F FF)
    0x46,0xFF,0x7F, // PHYSICAL_MAXIMUM (7F FF)
    0x75,0x10,  // REPORT_SIZE (10)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x45,0x00,  // PHYSICAL_MAXIMUM (00)
    0x66,0x00,0x00, // UNIT (None)
    0x55,0x00,  // UNIT_EXPONENT (00)
  0xC0, // END COLLECTION ()
  
  0x09,0x5F,  // USAGE (Set Condition Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Set_Condition_Report_ID,  // REPORT_ID (03)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)

    0x09,0x23,  // USAGE (Parameter Block Offset)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x25,0x03,  // LOGICAL_MAXIMUM (03)
    0x75,0x04,  // REPORT_SIZE (04)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x58,  // USAGE (Type Specific Block Offset)
    0xA1,0x02,  // COLLECTION (Logical)
      0x0B,0x01,0x00,0x0A,0x00, // USAGE (Instance 1)
      0x0B,0x02,0x00,0x0A,0x00, // USAGE (Instance 2)
      0x75,0x02,  // REPORT_SIZE (02)
      0x95,0x02,  // REPORT_COUNT (02)
      0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0xC0, // END COLLECTION ()
     
    0x09,0x60,  // USAGE (CP Offset)
    0x15,0x80,  // LOGICAL_MINIMUM (-128)
    0x25,0x7F,  // LOGICAL_MAXIMUM (127)
    0x75,0x08,  // REPORT_SIZE (8)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)

    0x09,0x61,  // USAGE (Positive Coefficient)
    0x09,0x62,  // USAGE (Negative Coefficient)
    0x09,0x63,  // USAGE (Positive Saturation)
    0x09,0x64,  // USAGE (Negative Saturation)
    0x15,0x00,  // LOGICAL_MINIMUM (0)
    0x25,0x64,  // LOGICAL_MAXIMUM (100)
    0x75,0x08,  // REPORT_SIZE (8)
    0x95,0x04,  // REPORT_COUNT (04)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)

    0x09,0x65,  // USAGE (Dead Band )
    0x15,0x00,  // LOGICAL_MINIMUM (0)
    0x25,0x64,  // LOGICAL_MAXIMUM (100)
    0x75,0x08,  // REPORT_SIZE (8)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
  0xC0, // END COLLECTION ()
  
  0x09,0x6E,  // USAGE (Set Periodic Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Set_Periodic_Report_ID,  // REPORT_ID (04)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x70,  // USAGE (Magnitude)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0x7F,0x00, // LOGICAL_MAXIMUM (127)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x6F,  // USAGE (Offset)
    0x15,0x80,  // LOGICAL_MINIMUM (-128)
    0x25,0x7F,  // LOGICAL_MAXIMUM (127)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x71,  // USAGE (Phase)
    0x66,0x14,0x00, // UNIT (Eng Rot:Angular Pos)
    0x55,0xFE,  // UNIT_EXPONENT (FE)  -2
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0xFF,0x00, // LOGICAL_MAXIMUM (00 FF)
    0x35,0x00,  // PHYSICAL_MINIMUM (00)
    0x47,0xA0,0x8C,0x00,0x00, // PHYSICAL_MAXIMUM (36000 d)
    0x95,0x01,  // REPORT_COUNT (01)
    0x75,0x08,  // REPORT_SIZE (08)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x72,  // USAGE (Period)
    0x26,0xFF,0x7F, // LOGICAL_MAXIMUM (7F FF)
    0x46,0xFF,0x7F, // PHYSICAL_MAXIMUM (7F FF)
    0x66,0x03,0x10, // UNIT (Eng Lin:Time)
    0x55,0xFD,  // UNIT_EXPONENT (-3)
    0x75,0x10,  // REPORT_SIZE (10)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x66,0x00,0x00, // UNIT (None)
    0x55,0x00,  // UNIT_EXPONENT (00)
    0x45,0x00,  // PHYSICAL_MAXIMUM (0)   disable because they are "global"
  0xC0, // END COLLECTION ()
  
  0x09,0x73,  // USAGE (Set Constant Force Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Set_Constant_Force_Report_ID,  // REPORT_ID (05)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x70,  // USAGE (Magnitude)
    0x16,0x01,0xFF,     /*      Logical Minimum (-255) */ 
    0x26,0xFF,0x00,     /*      Logical Maximum (+255) */
    0x36,0xF0,0xD8, // PHYSICAL_MINIMUM (-10000)
    0x46,0x10,0x27, // PHYSICAL_MAXIMUM (+10000)
    0x75,0x10,  // REPORT_SIZE (16)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x45,0x00,  // PHYSICAL_MAXIMUM (0)   disable because they are "global"
  0xC0, // END COLLECTION ()
  
  0x09,0x74,  // USAGE (Set Ramp Force Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Set_Ramp_Force_Report_ID,  // REPORT_ID (06)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x75,  // USAGE (Ramp Start)
    0x09,0x76,  // USAGE (Ramp End)
    0x15,0x80,  // LOGICAL_MINIMUM (-128)
    0x25,0x7F,  // LOGICAL_MAXIMUM (127)
    0x36,0xF0,0xD8, // PHYSICAL_MINIMUM (-10000)
    0x46,0x10,0x27, // PHYSICAL_MAXIMUM (+10000)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x02,  // REPORT_COUNT (02)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
  0xC0, // END COLLECTION ()

  0x09,0x77,  // USAGE (Effect Operation)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Effect_Operation_Report_ID,  // REPORT_ID (0A)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x78,  // USAGE (78)
    0xA1,0x02,  // COLLECTION (Logical)
      0x09,0x79,  // USAGE (Op Effect Start)
      0x09,0x7A,  // USAGE (Op Effect Start Solo)
      0x09,0x7B,  // USAGE (Op Effect Stop)
      0x15,0x01,  // LOGICAL_MINIMUM (01)
      0x25,0x03,  // LOGICAL_MAXIMUM (03)
      0x75,0x08,  // REPORT_SIZE (08)
      0x95,0x01,  // REPORT_COUNT (01)
      0x91,0x00,  // OUTPUT (Data,Ary,Abs)
    0xC0, // END COLLECTION ()
    0x09,0x7C,      // USAGE (Loop Count)
    0x15,0x00,      // LOGICAL_MINIMUM (00)
    0x26,0xFF,0x00, // LOGICAL_MAXIMUM (00 FF)
    0x35,0x00,      // Physical Minimum (0)
    0x46,0xFF,0x00, // Physical Maximum (255)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
  0xC0, // END COLLECTION ()
    
  0x09,0x90,  // USAGE (PID Block Free)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Block_Free_Report_ID,  // REPORT_ID (0B)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
  0xC0, // END COLLECTION ()

  0x09,0x96,  // USAGE (PID Device Control)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Device_Control_Report_ID,  // REPORT_ID (0C)
    0x09,0x97,  // USAGE (DC Enable Actuators)
    0x09,0x98,  // USAGE (DC Disable Actuators)
    0x09,0x99,  // USAGE (DC Stop All Effects)
    0x09,0x9A,  // USAGE (DC Device Reset)
    0x09,0x9B,  // USAGE (DC Device Pause)
    0x09,0x9C,  // USAGE (DC Device Continue)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x06,  // LOGICAL_MAXIMUM (06)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (1)
    0x91,0x00,  // OUTPUT (Data)
  0xC0, // END COLLECTION ()
  

  0x09,0x7D,  // USAGE (Device Gain)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Device_Gain_Report_ID,
    0x09,0x7E,  // USAGE (Device Gain)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0x7F,0x00, // LOGICAL_MAXIMUM (127)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
  0xC0, // END COLLECTION ()

#ifdef SUPPORT_CUSTOM_FORCE
  0x09,0x68,  // USAGE (Custom Force Data Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Custom_Force_Data_Report_ID,
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x6C,  // USAGE (Custom Force Data Offset)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0x10,0x27, // LOGICAL_MAXIMUM (10000)
    0x75,0x10,  // REPORT_SIZE (10)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x69,  // USAGE (Custom Force Data)
    0x15,0x81,  // LOGICAL_MINIMUM (-127)
    0x25,0x7F,  // LOGICAL_MAXIMUM (127)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x0C,  // REPORT_COUNT (0C)
    0x92,0x02,0x01, // OUTPUT ( Data,Var,Abs,Buf)
  0xC0, // END COLLECTION ()
  
  0x09,0x66,  // USAGE (Download Force Sample)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Download_Force_Sample_ID,  // REPORT_ID (xx)
    0x05,0x01,  // USAGE_PAGE (Generic Desktop)
    0x09,0x30,  // USAGE (X)
    0x09,0x31,  // USAGE (Y)
    0x15,0x81,  // LOGICAL_MINIMUM (-127)
    0x25,0x7F,  // LOGICAL_MAXIMUM (127)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x02,  // REPORT_COUNT (02)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
  0xC0, // END COLLECTION ()

  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0x6B,  // USAGE (Set Custom Force Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,PID_OUT_Set_Custom_Force_Report_ID,  // REPORT_ID (xx)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x35,0x01,  // Physical_MINIMUM (01)
    0x45,0x28,  // Physical_MAXIMUM (40)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x6D,  // USAGE (Sample Count)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0xFF,0x00, // LOGICAL_MAXIMUM (00 FF)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x09,0x51,  // USAGE (Sample Period)
    0x66,0x03,0x10, // UNIT (Eng Lin:Time)
    0x55,0xFD,  // UNIT_EXPONENT (-3)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0xFF,0x7F, // LOGICAL_MAXIMUM (32767)
    0x35,0x00,  // PHYSICAL_MINIMUM (00)
    0x46,0xFF,0x7F, // PHYSICAL_MAXIMUM (32767)
    0x75,0x10,  // REPORT_SIZE (10)
    0x95,0x01,  // REPORT_COUNT (01)
    0x91,0x02,  // OUTPUT (Data,Var,Abs)
    0x55,0x00,  // UNIT_EXPONENT (00)
    0x66,0x00,0x00, // UNIT (None)
    0x45,0x00,  // PHYSICAL_MAXIMUM (0)   disable because they are "global"
  0xC0, // END COLLECTION ()
#endif

  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0x92,  // USAGE (PID State Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,FFB_PID_STATUS_REPORT_ID,  // REPORT_ID (02)
    0x09,0x9F,  // USAGE (Device Paused)
    0x09,0xA0,  // USAGE (Actuators Enabled)
    0x09,0xA4,  // USAGE (Safety Switch)
    0x09,0xA5,  // USAGE (Actuator Override Switch)
    0x09,0xA6,  // USAGE (Actuator Power)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x25,0x01,  // LOGICAL_MAXIMUM (01)
    0x75,0x08,  // REPORT_SIZE (01)
    0x95,0x05,  // REPORT_COUNT (05)
    0x81,0x02,  // INPUT (Data,Var,Abs)
    0x95,0x03,  // REPORT_COUNT (03)
    0x81,0x03,  // INPUT (Constant,Var,Abs)
    0x09,0x94,  // USAGE (Effect Playing)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x25,0x01,  // LOGICAL_MAXIMUM (01)
    0x75,0x01,  // REPORT_SIZE (01)  
    0x95,0x01,  // REPORT_COUNT (01)
    0x81,0x02,  // INPUT (Data,Var,Abs)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x25,0x28,  // LOGICAL_MAXIMUM (0x28 -> 40)
    0x75,0x07,  // REPORT_SIZE (07)
    0x95,0x01,  // REPORT_COUNT (01)
    0x81,0x02,  // INPUT (Data,Var,Abs)
  0xC0, // END COLLECTION (PID State Report)
  

  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0xAB,  // USAGE (Create New Effect Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,FFB_FEATURE_NEW_EFFECT_ID,  // REPORT_ID (05)
    0x09,0x25,  // USAGE (Effect Type)
    0xA1,0x02,  // COLLECTION (Logical)
      0x09,0x26,  // USAGE (26)
      0x09,0x27,  // USAGE (27)
      0x09,0x30,  // USAGE (30)
      0x09,0x31,  // USAGE (31)
      0x09,0x32,  // USAGE (32)
      0x09,0x33,  // USAGE (33)
      0x09,0x34,  // USAGE (34)
      0x09,0x40,  // USAGE (40)
      0x09,0x41,  // USAGE (41)
      0x09,0x42,  // USAGE (42)
      0x09,0x43,  // USAGE (43)  Friction
#ifdef SUPPORT_CUSTOM_FORCE
      0x09,0x28,  // USAGE (28 Custom Force)
      0x25,0x0C,  // LOGICAL_MAXIMUM (0C)
#else
      0x25,0x0B,  // LOGICAL_MAXIMUM (0B)
#endif
      0x15,0x01,  // LOGICAL_MINIMUM (01)
      0x75,0x08,  // REPORT_SIZE (08)
      0x95,0x01,  // REPORT_COUNT (01)
      0xB1,0x00,  // FEATURE (Data)
    0xC0, // END COLLECTION ()
    0x05,0x01,  // USAGE_PAGE (Generic Desktop)
    0x09,0x3B,  // USAGE (Byte Count)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x26,0xFF,0x01, // LOGICAL_MAXIMUM (511)
    0x75,0x0A,  // REPORT_SIZE (0A)
    0x95,0x01,  // REPORT_COUNT (01)
    0xB1,0x02,  // FEATURE (Data,Var,Abs)
    0x75,0x06,  // REPORT_SIZE (06)
    0xB1,0x01,  // FEATURE (Constant,Ary,Abs)
  0xC0, // END COLLECTION (Create New Effect Report)
  
  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0x89,  // USAGE (PID Block Load Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,FFB_FEATURE_BLOCK_LOAD_ID,  // REPORT_ID (06)
    0x09,0x22,  // USAGE (Effect Block Index)
    0x25,0x28,  // LOGICAL_MAXIMUM (28)
    0x15,0x01,  // LOGICAL_MINIMUM (01)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0xB1,0x02,  // FEATURE (Data,Var,Abs)
    0x09,0x8B,  // USAGE (Block Load Status)
    0xA1,0x02,  // COLLECTION (Logical)
      0x09,0x8C,  // USAGE (Block Load Success)
      0x09,0x8D,  // USAGE (Block Load Full)
      0x09,0x8E,  // USAGE (Block Load Error)
      0x25,0x03,  // LOGICAL_MAXIMUM (03)
      0x15,0x01,  // LOGICAL_MINIMUM (01)
      0x75,0x08,  // REPORT_SIZE (08)
      0x95,0x01,  // REPORT_COUNT (01)
      0xB1,0x00,  // FEATURE (Data)
    0xC0, // END COLLECTION ()
    0x09,0xAC,  // USAGE (RAM Pool Available)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x27,0xFF,0xFF,0x00,0x00, // LOGICAL_MAXIMUM (00 00 FF FF)
    0x75,0x10,  // REPORT_SIZE (10)
    0x95,0x01,  // REPORT_COUNT (01)
    0xB1,0x00,  // FEATURE (Data)
  0xC0, // END COLLECTION (PID Block Load Report)


#ifdef SUPPORT_DEVICE_GAIN
  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0x7D,  // USAGE (Device Gain Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,FFB_FEATURE_DEVICE_GAIN_ID,  // REPORT_ID (03)
    0x09,0x7E,  // USAGE (Device Gain)
    0x26,0x7F,0x00, // LOGICAL_MAXIMUM (127) 
    0x75, 0x08,   // REPORT_SIZE (8) 
    0x95, 0x01,   // REPORT_COUNT (1) 
    0xB1, 0x02,   // FEATURE (Data,Var,Abs)    
  0xC0, // END COLLECTION (Device Gain Report)
#endif

#ifdef NEW_PID_POOL_REPORT

  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0x7F,  // USAGE (PID Pool Report)
  0xA1,0x02,  // COLLECTION (Logical)
  0x85,FFB_PID_POOL_REPORT_ID,
    0x09,0x80,  // USAGE (RAM Pool Size)
#ifdef PID_POOL_INCLUDES_ROMPOOL
    0x09,0x81,  // USAGE (ROM Pool Size)
    0x09,0x82,  // USAGE (ROM Effect Block Count)
    0x95,0x03,  // REPORT_COUNT (03)
#else
    0x95,0x01,  // REPORT_COUNT (01)
#endif
    0x15,0x00,           // LOGICAL_MINIMUM (00)
    0x27,0xFF, 0xFf,0,0, // LOGICAL_MAXIMUM (65535)
    0x35,0x00,           // Physical_Minimum(0)
    0x47,0xff,0xff,0,0,  // Phsyical Maximum (65535)
    0x75,0x10,  // REPORT_SIZE (16)
    0xB1,0x02,  // FEATURE (Data,Var,Abs)

//#ifdef  DRIVER_MANAGED_POOL
    0x09,0xA8,  // USAGE (Parameter Block Size)
    0xA1,0x02,  // COLLECTION (Logical)

      0x09,0x21,  // USAGE (Set Effect Report)
      0x09,0x5a,  // USAGE (Set Envelope Report)
      0x09,0x5f,  // USAGE (Set Condition Report)
      0x09,0x6e,  // USAGE (Set Periodic Report)
      0x09,0x73,  // USAGE (Set Constant Force Report)
      0x09,0x74,  // USAGE (Set Ramp Force Report)
#ifdef SUPPORT_CUSTOM_FORCE
      0x09,0x6b,  // USAGE (Set Custom Force Report)
      0x95,0x07,  // REPORT_COUNT (07)
#else
      0x95,0x06,  // REPORT_COUNT (06)
#endif

      0x26,0xFF,0x00, // LOGICAL_MAXIMUM (255)
      0x75,0x8,   // REPORT_SIZE (8)
      0xB1,0x02,  // FEATURE (Data,Var,Abs)
     0xC0, // END COLLECTION (Parameter Block Size)

//#endif // DRIVER_MANAGED_POOL

      0x09,0x83,  // USAGE (Simultaneous Effects Max)
      0x26,0xFF,0x00, // LOGICAL_MAXIMUM (00 FF)
      0x46,0xff,0x00, // Phsyical Maximum (255)
      0x75,0x08,  // REPORT_SIZE (08)
      0x95,0x01,  // REPORT_COUNT (01)
      0xB1,0x02,  // FEATURE (Data,Var,Abs)
      
      0x09,0xA9,  // USAGE (Device Managed Pool)
      0x09,0xAA,  // USAGE (Shared Parameter Blocks)
#ifdef SUPPORT_CUSTOM_FORCE_ISOCHRONUOS
        0x09,0x67,  // USAGE (Isoch Custom Force Enable)
        0x95,0x02,  // REPORT_COUNT (03)
#else
         0x95,0x02,  // REPORT_COUNT (02)
#endif
        0x75,0x01,  // REPORT_SIZE (01)
        0x15,0x00,  // LOGICAL_MINIMUM (00)
        0x25,0x01,  // LOGICAL_MAXIMUM (01)
        0x35,0x00,  // Physical_Minimum(0)
        0x45,0x01,  // Pysical_Maximum(1)
        0xB1,0x02,  // FEATURE (Data,Var,Abs)
  
#ifdef SUPPORT_CUSTOM_FORCE_ISOCHRONUOS
      0x95,0x05,  // REPORT_COUNT (05) pad
#else
      0x95,0x06,  // REPORT_COUNT (06) pad
#endif      
      0xB1,0x03,  // FEATURE ( Cnst,Var,Abs)

  0xC0, // END COLLECTION (PID Pool Report)

#else
  0x05,0x0F,  // USAGE_PAGE (Physical Interface)
  0x09,0x7F,  // USAGE (PID Pool Report)
  0xA1,0x02,  // COLLECTION (Logical)
    0x85,FFB_PID_POOL_REPORT_ID,  // REPORT_ID (07)
    0x09,0x80,  // USAGE (RAM Pool Size)
    0x75,0x10,  // REPORT_SIZE (16)
    0x95,0x01,  // REPORT_COUNT (01)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x27,0xFF,0xFF,0x00,0x00, // LOGICAL_MAXIMUM (00 00 FF FF)
    0x35,0x00,           // Physical_Minimum(0)
    0x47,0xff,0xff,0,0,  // Phsyical Maximum (65535)

    0xB1,0x02,  // FEATURE (Data,Var,Abs)

    0x09,0x83,  // USAGE (Simultaneous Effects Max)
    0x26,0x64,0x00, // LOGICAL_MAXIMUM (100)
    0x46,0xff,0x00,  // Pysical_Maximum(255)
    0x75,0x08,  // REPORT_SIZE (08)
    0x95,0x01,  // REPORT_COUNT (01)
    0xB1,0x02,  // FEATURE (Data,Var,Abs)
    0x09,0xA9,  // USAGE (Device Managed Pool)
    0x09,0xAA,  // USAGE (Shared Parameter Blocks)
    0x75,0x01,  // REPORT_SIZE (01)
    0x95,0x02,  // REPORT_COUNT (02)
    0x15,0x00,  // LOGICAL_MINIMUM (00)
    0x25,0x01,  // LOGICAL_MAXIMUM (01)
    0x35,0x00,  // Physical_Minimum(0)
    0x45,0x01,  // Pysical_Maximum(1)
    0xB1,0x02,  // FEATURE (Data,Var,Abs)
    0x75,0x06,  // REPORT_SIZE (06)
    0x95,0x01,  // REPORT_COUNT (01)
    0xB1,0x03,  // FEATURE ( Cnst,Var,Abs)
  0xC0, // END COLLECTION (PID Pool Report)
#endif
0xC0, // END COLLECTION (Application)
};


/** HID class report descriptor. This is a special descriptor constructed with values from the
 *  USBIF HID class specification to describe the reports and capabilities of the HID device. This
 *  descriptor is parsed by the host and its contents used to determine what data (and in what encoding)
 *  the device will send, and what it may be sent back from the host. Refer to the HID specification for
 *  more details on HID report descriptors.
 */
const USB_Descriptor_HIDReport_Datatype_t PROGMEM JoystickReportDescriptor[] =
{
  /* removed for space savings */
};


/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
  .Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
  .USBSpecification       = VERSION_BCD(1,1,0),
#ifdef ENABLE_CDC
  .Class                  = USB_CSCP_IADDeviceClass,
  .SubClass               = USB_CSCP_IADDeviceSubclass,
  .Protocol               = USB_CSCP_IADDeviceProtocol,
#else
  .Class                  = USB_CSCP_NoDeviceClass,
  .SubClass               = USB_CSCP_NoDeviceSubclass,
  .Protocol               = USB_CSCP_NoDeviceProtocol,
#endif // ENABLE_CDC
  .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
#if 0
  .VendorID               = 0x03EB, 
  .ProductID              = 0x8037,
#else
  .VendorID               = 0x045E, //Microsoft
  .ProductID              = 0x0034, // FFB wheel
#endif // ENABLE_CDC
  .ReleaseNumber          = VERSION_BCD(0,1,0),
  .ManufacturerStrIndex   = 0x01,
  .ProductStrIndex        = 0x02,
  .SerialNumStrIndex      = 0x03,
  .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

#ifdef ENABLE_CDC
USB_ClassInfo_CDC_Device_t Serial1_CDC_Interface =
{
    .Config =
      {
        .ControlInterfaceNumber   = 0,
        .DataINEndpoint           =
          {
            .Address          = CDC1_TX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
          },
        .DataOUTEndpoint =
          {
            .Address          = CDC1_RX_EPADDR,
            .Size             = CDC_TXRX_EPSIZE,
            .Banks            = 1,
          },
        .NotificationEndpoint =
          {
            .Address          = CDC1_NOTIFICATION_EPADDR,
            .Size             = CDC_NOTIFICATION_EPSIZE,
            .Banks            = 1,
          },
      },
};
#endif

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
/* they differ: HIDReportLength        = sizeof(JoystickReportDescriptor) */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptorWheel =
{
  .Config =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},
      .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
#ifdef ENABLE_CDC
      .TotalInterfaces        = 3,
#else
      .TotalInterfaces        = 1,
#endif // ENABLE_CDC
      .ConfigurationNumber    = 1,
      .ConfigurationStrIndex  = NO_DESCRIPTOR,
      .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),
      .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
    },
  .HID_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber        = 0,
      .AlternateSetting       = 0,
      .TotalEndpoints         = 2,
      .Class                  = HID_CSCP_HIDClass,
      .SubClass               = HID_CSCP_NonBootSubclass,
      .Protocol               = HID_CSCP_NonBootProtocol,
      .InterfaceStrIndex      = NO_DESCRIPTOR
    },
  .HID_JoystickHID =
    {
      .Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
      .HIDSpec                = VERSION_BCD(1,1,0),
      .CountryCode            = 0x00,
      .TotalReportDescriptors = 1,
      .HIDReportType          = HID_DTYPE_Report,
      .HIDReportLength        = sizeof(WheelReportDescriptor)
    },
  .HID_ReportINEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_IN | JOYSTICK_EPNUM),
      .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = JOYSTICK_EPSIZE,
      .PollingIntervalMS      = 10
    },
  .HID_ReportOUTEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_OUT | FFB_EPNUM),
      .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = FFB_EPSIZE,
      .PollingIntervalMS      = 10
    },
#ifdef ENABLE_CDC
  .CDC1_IAD =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_Association_t), .Type = DTYPE_InterfaceAssociation},
      .FirstInterfaceIndex    = 1,
      .TotalInterfaces        = 2,
      .Class                  = CDC_CSCP_CDCClass,
      .SubClass               = CDC_CSCP_ACMSubclass,
      .Protocol               = CDC_CSCP_ATCommandProtocol,
      .IADStrIndex            = NO_DESCRIPTOR
    },
  .CDC1_CCI_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber        = 1,
      .AlternateSetting       = 0,
      .TotalEndpoints         = 1,
      .Class                  = CDC_CSCP_CDCClass,
      .SubClass               = CDC_CSCP_ACMSubclass,
      .Protocol               = CDC_CSCP_ATCommandProtocol,
      .InterfaceStrIndex      = NO_DESCRIPTOR
    },
  .CDC1_Functional_Header =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_Header,
      .CDCSpecification       = VERSION_BCD(1,1,0),
    },
  .CDC1_Functional_ACM =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_ACM,
      .Capabilities           = 0x06,
    },
  .CDC1_Functional_Union =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_Union,
      .MasterInterfaceNumber  = 1,
      .SlaveInterfaceNumber   = 2,
    },
  .CDC1_ManagementEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_IN | CDC1_NOTIFICATION_EPNUM),
      .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_NOTIFICATION_EPSIZE,
      .PollingIntervalMS      = 0xFF
    },
  .CDC1_DCI_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber        = 2,
      .AlternateSetting       = 0,
      .TotalEndpoints         = 2,
      .Class                  = CDC_CSCP_CDCDataClass,
      .SubClass               = CDC_CSCP_NoDataSubclass,
      .Protocol               = CDC_CSCP_NoDataProtocol,
      .InterfaceStrIndex      = NO_DESCRIPTOR
    },
  .CDC1_DataOutEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_OUT | CDC1_RX_EPNUM),
      .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_TXRX_EPSIZE,
      .PollingIntervalMS      = 10
    },
  .CDC1_DataInEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_IN | CDC1_TX_EPNUM),
      .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_TXRX_EPSIZE,
      .PollingIntervalMS      = 10
    }
#endif // ENABLE_CDC

};

/* they only differ in their descriptor size in .HIDReportLength */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptorJoystick =
{
#if 0
  .Config =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},
      .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
#ifdef ENABLE_CDC
      .TotalInterfaces        = 3,
#else
      .TotalInterfaces        = 1,
#endif // ENABLE_CDC
      .ConfigurationNumber    = 1,
      .ConfigurationStrIndex  = NO_DESCRIPTOR,
      .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),
      .MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
    },
#ifdef ENABLE_CDC
  .CDC1_IAD =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_Association_t), .Type = DTYPE_InterfaceAssociation},
      .FirstInterfaceIndex    = 1,
      .TotalInterfaces        = 2,
      .Class                  = CDC_CSCP_CDCClass,
      .SubClass               = CDC_CSCP_ACMSubclass,
      .Protocol               = CDC_CSCP_ATCommandProtocol,
      .IADStrIndex            = NO_DESCRIPTOR
    },
  .CDC1_CCI_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber        = 1,
      .AlternateSetting       = 0,
      .TotalEndpoints         = 1,
      .Class                  = CDC_CSCP_CDCClass,
      .SubClass               = CDC_CSCP_ACMSubclass,
      .Protocol               = CDC_CSCP_ATCommandProtocol,
      .InterfaceStrIndex      = NO_DESCRIPTOR
    },
  .CDC1_Functional_Header =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_Header,
      .CDCSpecification       = VERSION_BCD(1,1,0),
    },
  .CDC1_Functional_ACM =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_ACM,
      .Capabilities           = 0x06,
    },
  .CDC1_Functional_Union =
    {
      .Header                 = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t), .Type = DTYPE_CSInterface},
      .Subtype                = CDC_DSUBTYPE_CSInterface_Union,
      .MasterInterfaceNumber  = 1,
      .SlaveInterfaceNumber   = 2,
    },
  .CDC1_ManagementEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_IN | CDC1_NOTIFICATION_EPNUM),
      .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_NOTIFICATION_EPSIZE,
      .PollingIntervalMS      = 0xFF
    },
  .CDC1_DCI_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber        = 2,
      .AlternateSetting       = 0,
      .TotalEndpoints         = 2,
      .Class                  = CDC_CSCP_CDCDataClass,
      .SubClass               = CDC_CSCP_NoDataSubclass,
      .Protocol               = CDC_CSCP_NoDataProtocol,
      .InterfaceStrIndex      = NO_DESCRIPTOR
    },
  .CDC1_DataOutEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_OUT | CDC1_RX_EPNUM),
      .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_TXRX_EPSIZE,
      .PollingIntervalMS      = 0x01
    },
  .CDC1_DataInEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_IN | CDC1_TX_EPNUM),
      .Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = CDC_TXRX_EPSIZE,
      .PollingIntervalMS      = 0x01
    },
#endif // ENABLE_CDC
  .HID_Interface =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
      .InterfaceNumber        = 0x00,
      .AlternateSetting       = 0x00,
      .TotalEndpoints         = 2,
      .Class                  = HID_CSCP_HIDClass,
      .SubClass               = HID_CSCP_NonBootSubclass,
      .Protocol               = HID_CSCP_NonBootProtocol,
      .InterfaceStrIndex      = NO_DESCRIPTOR
    },
  .HID_JoystickHID =
    {
      .Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
      .HIDSpec                = VERSION_BCD(1,1,0),
      .CountryCode            = 0x00,
      .TotalReportDescriptors = 1,
      .HIDReportType          = HID_DTYPE_Report,
      .HIDReportLength        = sizeof(JoystickReportDescriptor)
    },
  .HID_ReportINEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_IN | JOYSTICK_EPNUM),
      .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = JOYSTICK_EPSIZE,
      .PollingIntervalMS      = 10
    },
  .HID_ReportOUTEndpoint =
    {
      .Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
      .EndpointAddress        = (ENDPOINT_DIR_OUT | FFB_EPNUM),
      .Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
      .EndpointSize           = FFB_EPSIZE,
      .PollingIntervalMS      = 10
    }
#endif
};

/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"peewee.hannover.de");
/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ProductStringJoystick = USB_STRING_DESCRIPTOR(L"MS Sidewinder FFB");
const USB_Descriptor_String_t PROGMEM ProductStringWheel = USB_STRING_DESCRIPTOR(L"MS Sidewinder FFB");

const USB_Descriptor_String_t PROGMEM SerialNumberString = USB_STRING_DESCRIPTOR(L"00001234");

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void** const DescriptorAddress)
{
  const uint8_t  DescriptorType   = (wValue >> 8);
  const uint8_t  DescriptorNumber = (wValue & 0xFF);

  const void* Address = NULL;
  uint16_t    Size    = NO_DESCRIPTOR;

  if (isLogLevel(DEBUG_DATALINK)) log_P_nonl(PSTR("CB_USB_GetDescr(Type: %d,num: %d)"), DescriptorType, DescriptorNumber);
  
  switch (DescriptorType)
  {
    case DTYPE_Device:
      if (isLogLevel(DEBUG_DATALINK)) log_P_nonl(PSTR(" /Device:"));
      Address = &DeviceDescriptor;
      Size    = sizeof(DeviceDescriptor);
      break;
    case DTYPE_Configuration:
      if (isLogLevel(DEBUG_DATALINK)) log_P_nonl(PSTR(" /Conf:"));
      if (sw_id == SW_ID_WHEEL) {
        Address = &ConfigurationDescriptorWheel;
        Size    = sizeof(USB_Descriptor_Configuration_t);
      } else {
        Address = &ConfigurationDescriptorJoystick;
        Size    = sizeof(USB_Descriptor_Configuration_t);
      }
      break;
    case DTYPE_String:
      if (isLogLevel(DEBUG_DATALINK)) log_P_nonl(PSTR(" /String:"));
      switch (DescriptorNumber)
      {
        case 0x00:
          Address = &LanguageString;
          Size    = pgm_read_byte(&LanguageString.Header.Size);
          break;
        case 0x01:
          Address = &ManufacturerString;
          Size    = pgm_read_byte(&ManufacturerString.Header.Size);
          break;
        case 0x02:
          if (sw_id == SW_ID_WHEEL) {
            Address = &ProductStringWheel;
            Size    = pgm_read_byte(&ProductStringWheel.Header.Size);
          } else {
            Address = &ProductStringJoystick;
            Size    = pgm_read_byte(&ProductStringJoystick.Header.Size);
          }
          break;
        case 0x03:
            Address = &SerialNumberString;
            Size    = pgm_read_byte(&SerialNumberString.Header.Size);
          break;
      }

      break;
    case HID_DTYPE_HID:
      if (isLogLevel(DEBUG_DATALINK)) log_P_nonl(PSTR(" /HID: "));
      if (sw_id == SW_ID_WHEEL) {
        Address = &ConfigurationDescriptorWheel.HID_JoystickHID;
        Size    = sizeof(USB_HID_Descriptor_HID_t);
      } else {
        Address = &ConfigurationDescriptorJoystick.HID_JoystickHID;
        Size    = sizeof(USB_HID_Descriptor_HID_t);
      }
      break;
    case HID_DTYPE_Report:
      if (isLogLevel(DEBUG_DATALINK)) log_P_nonl(PSTR(" /Report: "));
      if (sw_id == SW_ID_WHEEL) {
        Address = &WheelReportDescriptor;
        Size    = sizeof(WheelReportDescriptor);
      } else {
        Address = &JoystickReportDescriptor;
        Size    = sizeof(JoystickReportDescriptor);
      }
      break;

    case DTYPE_Interface:
      if (isLogLevel(DEBUG_DATALINK)) log_P_nonl(PSTR(" /Interface:"));
      Size    = sizeof(USB_Descriptor_Interface_t);
      if (wIndex == 0) {
        if (sw_id == SW_ID_WHEEL) 
          Address = &ConfigurationDescriptorWheel.HID_Interface;
        else
          Address = &ConfigurationDescriptorJoystick.HID_Interface;
      }
#ifdef ENABLE_CDC
      else if (wIndex == 1) {
        if (sw_id == SW_ID_WHEEL) 
            Address = &ConfigurationDescriptorWheel.CDC1_CCI_Interface;
          else
            Address = &ConfigurationDescriptorJoystick.CDC1_CCI_Interface;
      } else {
        if (sw_id == SW_ID_WHEEL) 
          Address = &ConfigurationDescriptorWheel.CDC1_DCI_Interface;
        else
          Address = &ConfigurationDescriptorJoystick.CDC1_DCI_Interface;
      }
#endif
      break;
      default:
        error_P_nonl(PSTR("<-- unhandled:"));
      
  }
  if (isLogLevel(DEBUG_DATALINK)) log_P(PSTR("size: %d"), Size);
  /* without this delay and no debug logging, I get Windows error: "USB device could not be detected" */
  delay(1);
  *DescriptorAddress = Address;
  return Size;
}
