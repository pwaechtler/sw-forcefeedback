/*
  Force Feedback LUFA interface code
  Copyright 2022  peewee.hannover [at] gmail [dot] com
  Copyright 2012  Tero Loimuneva (tloimu [at] gmail [dot] com) MIT License

  Some code is based on LUFA Library, for which the license:
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

#include "LUFAConfig.h"
#include "board.h"

#include "lufa-iface.h"
#include "ffb.h"
#include "debug.h"

#include "Descriptors.h"

#ifdef ENABLE_CDC
/** Contains the current baud rate and other settings of the first virtual serial port. While this demo does not use
    the physical USART and thus does not use these settings, they must still be retained and returned to the host
    upon request or the host will assume the device is non-functional.
    These values are set by the host via a class-specific request, however they are not required to be used accurately.
    It is possible to completely ignore these value or use other settings as the host is completely unaware of the physical
    serial link characteristics and instead sends and receives data in endpoint streams.
*/
static CDC_LineEncoding_t LineEncoding1 = { .BaudRateBPS = 0,
                                            .CharFormat  = CDC_LINEENCODING_OneStopBit,
                                            .ParityType  = CDC_PARITY_None,
                                            .DataBits    = 8
                                          };
#endif // ENABLE_CDC

/** Current Idle period. This is set by the host via a Set Idle HID class request to silence the device's reports
    for either the entire idle duration, or until the report status changes (e.g. the user moves the mouse).
*/
static uint16_t IdleCount = 0;
/** Current Idle period remaining. When the IdleCount value is set, this tracks the remaining number of idle
    milliseconds. This is separate to the IdleCount timer and is incremented and compared as the host may request
    the current idle period via a Get Idle HID class request, thus its value must be preserved.
*/
static uint16_t IdleMSRemaining = 0;
/* SwFFb.h:SwFFb:read_decode() fills this global struct and that's get passed to a HID report
    by Joystick_CreateReport()
*/
JoystickReport_t joystick_state;
/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
/* that's used in LUFA and a buffer on the stack is written to */
static JoystickReport_t previous_joystick_state;
static uint8_t usb_configOnce = 0;


uint8_t sw_id = SW_ID_WHEEL;
uint8_t sw_report[SW_REPSZ_3DP];      // Report buffer

/** LUFA HID Class driver interface configuration and state information. This structure is
    passed to all HID Class driver functions, so that multiple instances of the same class
    within a device can be differentiated from one another.
*/
USB_ClassInfo_HID_Device_t Wheel_HID_Interface =
{
  .Config =
  {
    .InterfaceNumber              = 0,
    .ReportINEndpoint             =
    {
      .Address              = (ENDPOINT_DIR_IN | JOYSTICK_EPNUM),
      .Size                 = JOYSTICK_EPSIZE,
      .Banks                = 1,
    },
    .PrevReportINBuffer           = (uint8_t*) &previous_joystick_state,
    .PrevReportINBufferSize       = sizeof(previous_joystick_state),   // LUFA depends on such a value, for me a bug
  },
};


/** Fills the given HID report data structure with the next HID report to send to the host.


    \return length of the report
*/
uint16_t Joystick_CreateReport(uint8_t inReportId, void * const storage)
{
  int len = 0;

  if (sw_id == SW_ID_WHEEL)
  {

    if (inReportId == FFB_PID_REPORT_INPUT) {
      struct SimulationControlsReport *outReport = (struct SimulationControlsReport*) storage;
      uint16_t inverted;
      int8_t  throttle;

      outReport->steering = ((joystick_state.X >>2 ) -1) -127;  /* -128..0..127 */

      inverted = 1023 - joystick_state.Y;
      outReport->accelerator = inverted >> 3; /* 0(neutral)..127(full acc) */
      inverted = 1023 - joystick_state.Z;
      outReport->brake = inverted >> 3;   /* 0(neutral)..127(full brake) */

      throttle =  outReport->accelerator - outReport->brake;
      if (throttle < -127)
        throttle = -127;
//      outReport->throttle = throttle;  /* -127(full brake)..0(neutral)..127(full acc) */

      outReport->shifter = 0;
      
//      outReport->Button = (uint8_t) (joystick_state.Button & 0xff);
      len = sizeof(struct SimulationControlsReport);
      
    } else {
      struct WheelReport *outReport = storage;
      uint16_t inverted;
      int8_t  throttle;
      
      outReport->X = joystick_state.X;    /* left: 0 .. 512(neutral)..1023(right) */
      outReport->Y = joystick_state.Y;    /* 1023(neutral) .. 0(full acc) */
      outReport->Z = joystick_state.Z;    /* 1023(neutral) .. 0(full brake) */

      outReport->steering = ((joystick_state.X >>2 ) -1) -127;  /* -128..0..127 */

      inverted = 1023 - joystick_state.Y;
      outReport->accelerator = inverted >> 2; /* 0(neutral)..255(full acc) */
      inverted = 1023 - joystick_state.Z;
      outReport->brake = inverted >> 2;   /* 0(neutral)..255(full brake) */

      throttle =  (outReport->accelerator>>1) - (outReport->brake>>1);
      if (throttle < -127)
        throttle = -127;
      outReport->throttle = throttle;  /* -127(full brake)..0(neutral)..127(full acc) */

      outReport->shifter = 0;

      outReport->Button = (uint8_t) (joystick_state.Button & 0xff);
      len = sizeof(struct WheelReport);
    }
    
  } else {
    // Convert data from Sidewinder Force Feedback Pro
    JoystickReport_t *outReport = (JoystickReport_t*) storage;
    outReport->X = sw_report[0] + ((sw_report[1] & 0x03) << 8);
    if (sw_report[1] & 0x02)
      outReport->X |= (0b11111100 << 8);


    outReport->Y = (sw_report[1] >> 2) + ((sw_report[2] & 0x0F) << 6);
    if (sw_report[2] & 0x08)
      outReport->Y |= (0b11111100 << 8);
    outReport->Button = ((sw_report[4] & 0x7F) << 2) + ((sw_report[3] & 0xC0) >> 6);
    outReport->Hat = sw_report[2] >> 4;
    outReport->Rz = (sw_report[3] & 0x3f) - 32;
    outReport->Throttle = ((sw_report[5] & 0x3f) << 1) + (sw_report[4] >> 7);
    if (sw_report[5] & 0x20)
      outReport->Throttle |= 0b11000000;
    outReport->Z = 0;   // not used at the moment

    len = sizeof(JoystickReport_t);
  }
  return len;
}

//------------------------------------------------------------------------------

void initDevice(void)
{
  ffb_init();
}

void SetupHardware(void)
{
  board_init();
  USB_Init();
  initDevice();
}

// PID State Report:
//  uint8_t reportId; //
//  uint8_t status; //  0x01=Device Paused, 0x02=Actuators Enabled,0x04=Safety Switch,0x08=Actuator Override Switch,0x10=Actuator Power
//  uint8_t effectBlockIndex; // Bit7=Effect Playing, Bit0..7=EffectId (1..40)

void PidStateChanged(void)
{
  bool sendState;

  /* Create the next report for transmission to the host */
  /* Check to see if the report data has changed - if so a report MUST be sent */
  sendState = (memcmp(& gDevice.previousPidState, & gDevice.pidState, sizeof(FFBReport_PIDStatus_t)) != 0);

  /* Check if the idle period is set and has elapsed */
  if (IdleCount && (!(IdleMSRemaining))) {
    /* Reset the idle time remaining counter */
    IdleMSRemaining = IdleCount;
    /* Idle period is set and has elapsed, must send a report to the host */
    sendState = true;
  }
  /* Select the interrupt Report Endpoint */
  Endpoint_SelectEndpoint(JOYSTICK_EPNUM );

  /* Check if  Endpoint Ready for Read/Write and if we should send a new report */
  if (Endpoint_IsReadWriteAllowed()) {
    if (sendState) {
      /* Save the current report data for later comparison to check for changes */
       gDevice.previousPidState =  gDevice.pidState;

      /* Write Report Data */
      Endpoint_Write_Stream_LE( (const void*) & gDevice.pidState, sizeof(FFBReport_PIDStatus_t), NULL);
      /* Finalize the stream transfer to send the last packet */
      Endpoint_ClearIN();
      if (isLogLevel(DEBUG_LOG_PID)) {
        log_P(PSTR("PID: [avail:%d] state: 0x%x  eid<%u>  %s"),  gDevice.pool.slotsAvail,
            gDevice.pidState.status, gDevice.pidState.effectBlockIndex & 0x7f,
            (gDevice.pidState.effectBlockIndex & Playing) ? "Play":"stop" );
      }
    }
#ifdef SUPPORT_DEVICE_GAIN
    if (gDevice.gainReport.gain != gDevice.previousGain) {
      if (isLogLevel(DEBUG_LOG_PID)) {
        log_P(PSTR("DevFeatureGain: %u"), gDevice.gainReport.gain);
      }
      /* Save the current report data for later comparison to check for changes */
      gDevice.previousGain =  gDevice.gainReport.gain;
      gDevice.gainReport.reportId = FFB_FEATURE_DEVICE_GAIN_ID;   
      /* Write Report Data */
      Endpoint_Write_Stream_LE( (const void*) & gDevice.gainReport, sizeof(gDevice.gainReport), NULL);
      /* Finalize the stream transfer to send the last packet */
      Endpoint_ClearIN();
    }
#endif  
  }
}

/*
 * if the wheel is left untouched for a while, I see: 0xFFFC and immediately changing to 0x8004
 */
enum wheelState {
  SEEN_THAT            = 0x02,    /* together with 0x4000 */
  IDLE_MAYBE           = 0x04,    
  
  AUTOCENTER_MAYBE     = 0x20,
  ACTUATORS_ENABLED    = 0x80,    /* never seen */
  FORCE_OVERRIDE       = 0x100,    /* means user switched "Force button" OFF */
  DEVICE_PAUSED        = 0x2000,   /* seen after Eop(stop) - no effect playing */
  DEVICE_ERROR_MAYBE   = 0x4000,  /* if "unplausible" parameters send (on spring) */
  DEVICE_BUSY_OR_RESET = 0x8000
};
/* I can't see something like an ID only state bits */

void DeviceStateChanged(uint16_t newState)
{
  static uint16_t oldState = 0;
  
  if (newState & FORCE_OVERRIDE) {
      gDevice.deviceStateAndID |= ActuatorOverride;
  } else {
      gDevice.deviceStateAndID &= ~ActuatorOverride;
  }
  if (newState & DEVICE_PAUSED) {
      gDevice.deviceStateAndID |= Paused;
  } else {
      gDevice.deviceStateAndID &= ~Paused;
  }
  if (oldState != newState)
      PidStateChanged();
  if (isLogLevel(DEBUG_LOG_PID)) {
    log_P(PSTR("PidState 0x%04x -> 0x%04x"), oldState, newState);
  }
  oldState = newState;
}

uint8_t connectDevice(uint8_t set, uint16_t deviceID)
{
  static uint8_t devConnected = 0;
  if (set == 1) {
    devConnected = 1;
    gDevice.deviceStateAndID = deviceID;
    ffb_setDriver(sw_id);
  }
  if (set == 0xFF)
    devConnected = 0;
  return devConnected;
}


void taskLoop(void)
{
  static unsigned long last_led;
  unsigned long ts = millis();

  HID_Device_USBTask(&Wheel_HID_Interface);
  USB_USBTask();

  debugTask();

  if (!connectDevice(0, 0)) {
    static unsigned long last_connect;
    if (ts - last_connect  > 200) {
      LEDs_ToggleLEDs(LEDS_LED1);
      last_connect = ts;
    }
  } else {    
    ffb_Task();
  }
  if (ts - last_led > 500) {
    LEDs_ToggleLEDs(LEDS_LED1);
    last_led = ts;
  }
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  bool ConfigSuccess = true;

#ifdef ENABLE_CDC
  /* Reset line encoding baud rates so that the host knows to send new values */
  LineEncoding1.BaudRateBPS = 0;

  ConfigSuccess &= CDC_Device_ConfigureEndpoints(&Serial1_CDC_Interface);
 
  USB_ClassInfo_CDC_Device_t CDCInterfaceInfo;
  memset(&CDCInterfaceInfo, 0, sizeof(CDCInterfaceInfo));
  EVENT_CDC_Device_ControLineStateChanged(&CDCInterfaceInfo);

#endif  //ENABLE_CDC

  // this only configures 1 INendpoint
  ConfigSuccess &= HID_Device_ConfigureEndpoints(&Wheel_HID_Interface);

  ConfigSuccess &= Endpoint_ConfigureEndpoint(FFB_EPNUM, EP_TYPE_INTERRUPT, FFB_EPSIZE, 1);
  if (isLogLevel(DEBUG_DATALINK))
    error_P(PSTR("ConfigChanged: %d"), ConfigSuccess);
  /* Enable Start-Of-Frame event */
  USB_Device_EnableSOFEvents();
}
/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
    starts the library USB task to begin the enumeration and USB management process.
*/
void EVENT_USB_Device_Connect(void)
{
  /* Indicate USB enumerating */
  if (isLogLevel(DEBUG_DATALINK))
    log_P(PSTR("Connect:event"));
  gDevice.state |= Paused | ActuatorsEnabled | ActuatorPower;
  usb_configOnce = 0;
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
    the status LEDs and stops the USB management and joystick reporting tasks.
*/
void EVENT_USB_Device_Disconnect(void)
{
  if (isLogLevel(DEBUG_DATALINK))
    log_P(PSTR("Disconnect:event"));
}
// without getting those I can't implement the Reset on Open with 1200 baud + close
#ifdef ENABLE_CDC

static uint8_t readyForReset = 0;

void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t * const CDCInterfaceInfo)
{
    log_P(PSTR("CDC_LineEncodingChanged:event"));
}

void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t * const CDCInterfaceInfo)
{
    if (( CDCInterfaceInfo->State.ControlLineStates.HostToDevice & 0x01) == 0) {
      log_P(PSTR("connected.."));
    } else {
      log_P(PSTR("disconnect.."));
      if (readyForReset) {
        wdt_enable(WDTO_120MS);
      }
    }
    
    if (CDCInterfaceInfo->State.LineEncoding.BaudRateBPS == 1200) {
       readyForReset = 1;
    }
    log_P(PSTR("CDC_ControLineStateChanged:event: reset:%u"), readyForReset);
}
#endif

/*
  USB_ControlRequest :=
  uint8_t   bmRequestType
  uint8_t   bRequest
  uint16_t  wValue
  uint16_t  wIndex
  uint16_t  wLength
*/

void Handle_USB_DeviceToHost_ControlRequest(void)
{
  uint8_t reportID = (uint8_t) (USB_ControlRequest.wValue & 0x00FF);

#ifndef SAVE_SPACE
  if (isLogLevel(DEBUG_DETAIL))
  {
    log_P(PSTR("DeviceToHost_CR(type: 0x%X, req: %u, val: 0x%X,idx: 0x%X, len: %d):"),
      USB_ControlRequest.bmRequestType, USB_ControlRequest.bRequest,
      USB_ControlRequest.wValue, USB_ControlRequest.wIndex, USB_ControlRequest.wLength);
  }
  if (USB_ControlRequest.bmRequestType & (REQDIR_HOSTTODEVICE) )
  {
    if (isLogLevel(DEBUG_DETAIL)) log_P(PSTR("Ctrl.HostToDev"));
  } else {
    if (isLogLevel(DEBUG_DETAIL)) log_P_nonl(PSTR("Ctrl."));
  }
#endif

  switch (USB_ControlRequest.bRequest)
  {
    case HID_REQ_GetReport:
//      if (isLogLevel(DEBUG_DETAIL)) log_P_nonl(PSTR("Get->"));

      ffb_handleGetReport(reportID);
      break;
    case HID_REQ_SetReport:
      {
//      if (isLogLevel(DEBUG_DETAIL)) log_P_nonl(PSTR("Set<-"));

      uint8_t data[16];  // This is enough room for all reports
      uint16_t len = USB_ControlRequest.wLength;
      Endpoint_ClearSETUP();
      // Read the report data from the control endpoint - without a loop
      Endpoint_Read_Control_Stream_LE(&data, len);
      Endpoint_ClearStatusStage();

      // Process the incoming report
      ffb_handleFeatureSetReport(reportID, data, len);
      }
      break;

    default:
        error_P(PSTR("<<-- CtrlReq:unhandled"));
  }
  // honestly, if you turn off debugging then u get "USB device was not detected" on Windows (at least in VirtualBox :)...
  delay(6);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
  if (isLogLevel(DEBUG_DATALINK))
  {
    log_P(PSTR("CtrlReq(type: 0x%X, req: %u, val: 0x%X,idx: 0x%X, len: %u)"), 
      USB_ControlRequest.bmRequestType, USB_ControlRequest.bRequest, USB_ControlRequest.wValue,
      USB_ControlRequest.wIndex, USB_ControlRequest.wLength);
  }

#ifdef ENABLE_CDC
  CDC_Device_ProcessControlRequest(&Serial1_CDC_Interface);
#endif

  uint8_t bmRequestType = USB_ControlRequest.bmRequestType;

  switch (USB_ControlRequest.bRequest)
  {
    case REQ_ClearFeature:
    case REQ_SetFeature:
      if ((bmRequestType == (REQDIR_DEVICETOHOST | REQREC_INTERFACE | REQREC_DEVICE)) ||
          (bmRequestType == ( REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE )))
      {
        Handle_USB_DeviceToHost_ControlRequest();
        //          HID_Device_ProcessControlRequest(&Wheel_HID_Interface);
      }
      break;
    default:
      HID_Device_ProcessControlRequest(&Wheel_HID_Interface);
      break;
  }
}


uint8_t sendControlStream(uint8_t *buf, uint16_t len)
{
  uint8_t ret;
  uint8_t ep = Endpoint_GetCurrentEndpoint();

  Endpoint_SelectEndpoint(0);

  // TOOD: timing issue with Windows
  delay(6);
  if (Endpoint_IsStalled())
    Endpoint_ClearStall();

  Endpoint_ClearSETUP();
  // Write the "response" to the control endpoint
  ret = Endpoint_Write_Control_Stream_LE(buf, len);
  Endpoint_ClearOUT();
  if (isLogLevel(DEBUG_DATALINK))
    log_buf_P(PSTR("sendControlStream:"), buf, len);

  if ( ret != ENDPOINT_RWSTREAM_NoError) {
    error_P(PSTR("error sendControl: %d"), ret);
  }
  Endpoint_StallTransaction();

  Endpoint_SelectEndpoint(ep);
  return ret;
}

void ffb_handleGetReport(uint8_t reportID)
{
  uint16_t len = 0;
  uint8_t *response_ptr = NULL;

  switch (reportID) {
    case FFB_FEATURE_BLOCK_LOAD_ID:
      len = ffb_onBlockLoad(&response_ptr);
      break;
    case FFB_PID_POOL_REPORT_ID:
      len = ffb_onPIDPool(&response_ptr);
      break;
    default:
      error_P(PSTR("unhandled GetReport(%d)"), reportID);

  }
//  if (isLogLevel(DEBUG_DATALINK))  log_P(PSTR("GetReport.Handler(%d): %d <%p>"), reportID, len, response_ptr);

  if (len && response_ptr) {
    // Write the report data to the control endpoint - it's so defined in the spec

    len = sendControlStream(response_ptr, len);
  }
}

void ffb_handleFeatureSetReport(uint8_t reportID, const void *input, uint16_t len)
{
  uint16_t out_len;
  uint8_t *response_ptr;

  switch (reportID) {
    case FFB_FEATURE_NEW_EFFECT_ID:
      out_len = ffb_onCreateEffect(input, &response_ptr);
      //log_P(PSTR("SetFeature.H(%d): %d <%p>\n"), reportID, out_len, response_ptr);
      break;
    default:
      error_P(PSTR("unhandled SetReport(%d)"), reportID);

  }

  if (out_len && response_ptr) {
    sendControlStream(response_ptr, out_len);
  }
}

/** HID class driver callback function for the creation of HID reports to the host.

    \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
    \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
    \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
    \param[out]    ReportData  Pointer to a buffer where the created report should be stored
    \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)

    \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
*/
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
    uint8_t* const ReportID,
    const uint8_t ReportType,
    void* ReportData,
    uint16_t* const ReportSize)
{
  uint16_t len;

  //log_P(PSTR("CB_HID_CreateR(type: %u,ID: %u): size: %d"), ReportType, ReportID, ReportSize);

  if (*ReportID) {
    switch (*ReportID) {
      case FFB_HID_REPORT_INPUT:
//      log_P(PSTR("HID_REPORT_INPUT"));
        break;
      case FFB_PID_REPORT_INPUT:
//      log_P(PSTR("FFB_PID_REPORT"));
        break;
    }
  }
  /* Create the next HID report to send to the host */
  len = Joystick_CreateReport(*ReportID,  ReportData);
  * ReportSize = len;
  * ReportID = FFB_HID_REPORT_INPUT;

  return true;
}

/** HID class driver callback function for the processing of HID reports from the host.

    \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
    \param[in] ReportID    Report ID of the received report from the host
    \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
    \param[in] ReportData  Pointer to a buffer where the received report has been stored
    \param[in] ReportSize  Size in bytes of the received HID report
*/
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
    const uint8_t ReportID,
    const uint8_t ReportType,
    const void* ReportData,
    const uint16_t ReportSize)
{
#if 0
  if (isLogLevel(DEBUG_DETAIL)) {
    log_P(PSTR("CB_HID_Process(type: %u,ID: %u): size: %d"), ReportType, ReportID, ReportSize);
    log_buf_P(PSTR(" data_in:"), ReportData, ReportSize);
  }
#endif
  ffb_handleFeatureSetReport(ReportID, ReportData, ReportSize);
}


void printError(uint8_t eid)
{
  // this is not so important, and the cmds on eid 1 maybe set autocenter spring
  if (isLogLevel(DEBUG_DATALINK)) {
    log_P(PSTR("drv falsely acks on %u"), eid);
  }
}

uint8_t checkState(uint8_t eid, enum eState state, uint8_t logic)
{
  bool err_cond = true;
  if (checkBlockIndex(eid)) {
    bool set = getEffectBlock(eid)->state & state;

    if (logic == 0 && !set)
      err_cond = false;
    if (logic == 1 && set)
      err_cond = false;
    if (err_cond)
      printError(eid);
  }
  return err_cond;
}

/*
 * the driver sends "funny" cmds
 */
uint8_t handleAckFromDriver(uint8_t *buf, uint8_t len)
{
  uint8_t ret = 0;

  if (len >= 2) {
    switch (buf[0]) {
      case 0x9f:  // driver acks playing of effect
        ret = 1;
        checkState(buf[1], Playing, true);
        break;
      case 0x5f:  // driver acks stopped effect
        ret = 1;
        checkState(buf[1], Playing, false);
        break;
    }
  }
  return ret;  
}

uint16_t readFFB(const void *buf, uint16_t num)
{
  uint16_t len = 0;
  const uint8_t *dest = buf;
  uint8_t ep = Endpoint_GetCurrentEndpoint();

  Endpoint_SelectEndpoint(FFB_EPNUM);

  if (Endpoint_IsOUTReceived()) {
    while (Endpoint_BytesInEndpoint() && len < num) {
      LEDs_ToggleLEDs(LEDS_LED2);
      
      while (Endpoint_Read_Stream_LE( &dest[len++], 1, NULL)
             == ENDPOINT_RWSTREAM_IncompleteTransfer)
      { // busy loop until all bytes are read in
        LEDs_ToggleLEDs(LEDS_LED2);
      }
    }
    Endpoint_ClearOUT();
  }
  Endpoint_SelectEndpoint(ep);
  return len;
}

/** Function to manage FFB report generation and transmission to the host. */
void ffb_Task(void)
{
  uint16_t wait_for_n = 0;
  // enough for any single OUT-report, yes but if u read too slow u get several at once
  // on "focus lost" you get a bunch of report(7) aka EffectOperation(stop) with 4 bytes each
  static uint8_t buf[32];  
  /* Device must be connected and configured for the task to run */
  if (USB_DeviceState != DEVICE_STATE_Configured) {
    return;
  } else {
    if (!usb_configOnce) {
      EVENT_USB_Device_ConfigurationChanged();
      usb_configOnce = 1;
      printDeviceConfig();
    }
  }
  updateEffectStates();
  
  PidStateChanged();

  // Receive FFB reports
  Endpoint_SelectEndpoint(FFB_EPNUM);

  if (Endpoint_IsOUTReceived())
  {
    uint8_t total_read = 0;

    while (Endpoint_BytesInEndpoint() && (total_read < sizeof(buf)))
    {
      uint16_t numRead = 0;
      buf[0] = 0;
      // Read the reportID from the package to determine amount of data to expect next
      while (Endpoint_Read_Stream_LE(buf, 1, NULL) == ENDPOINT_RWSTREAM_IncompleteTransfer)
      { // busy loop until the first byte is read out
      }
      total_read = 1; // the reports are all stored at the beginning of the buffer
      // sanity check
      if (buf[0] > 0 && buf[0] <= NumOfReports ) {

        wait_for_n = OutReportLengths[buf[0] - 1] - 1;
        if (wait_for_n + total_read >= sizeof(buf)) {
          error_P(PSTR("Report[%d]: overrun: wfor: %d tread: %d"), buf[0], wait_for_n, total_read);
          error_loop(3);          
        }
      } else {
          // the drivers send pairs of bytes ("reports" outside our scope)
          wait_for_n = 1;
      }    
      numRead = 0;
      while (Endpoint_Read_Stream_LE(&buf[1], wait_for_n, &numRead)
             == ENDPOINT_RWSTREAM_IncompleteTransfer)
      { // busy loop until the rest of the report data is read out
      }
      total_read += wait_for_n;

      if (buf[0] > 0 && buf[0] <= NumOfReports) {
        if (isLogLevel(DEBUG_DATALINK)) {
#ifndef SAVE_SPACE
          if (isLogLevel(DEBUG_DETAIL)) log_P(PSTR(" tread: %d inEP: %d"), total_read, Endpoint_BytesInEndpoint() );
          log_P(PSTR("new report(Id: %d, wait_for: %d)"), buf[0], wait_for_n);
#endif
          LogReportP(PSTR("FFB:"), OutReportLengths, buf, wait_for_n +1);
        }

        ffb_handleSetReport(buf[0], &buf[0], wait_for_n + 1);
      } else {
        if ( !handleAckFromDriver(buf, wait_for_n+1)) {
          log_buf_P(PSTR("FFB input ignored:"), buf, wait_for_n + 1);          
        }
      }
    }
    // Clear the endpoint ready for new packet
    Endpoint_ClearOUT();
  }
}
