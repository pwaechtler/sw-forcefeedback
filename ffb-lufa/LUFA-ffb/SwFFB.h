#include <stdint.h>
//#include "ffb_component.h"
#include "lufa-iface.h"
#include "ffb-wheel.h"

#ifdef __cplusplus
  extern "C" {
    void startEffect(uint8_t id);
    void stopEffect(uint8_t id);
  }
#endif

/// Class for Sidewinder Force Feedback wheel
class SwFFB : public Sidewinder {
private:

protected:
  unsigned long m_last_devID_poll;
  uint16_t m_deviceStateAndID;

  void connect() {
      readDeviceStateAndID();
      DeviceStateChanged(m_deviceStateAndID);
      connectDevice(1, m_deviceStateAndID);
      error_P(PSTR("SwFFB::connected: %s"), getDescription().name);
      error_P(PSTR("SwFFB::init done - dev_state: %X "), m_deviceStateAndID); 
  }

public:
   bool update() override {
    
      if (m_model != Model::SW_UNKNOWN) {
        unsigned long ts = millis();
        if ( (ts - m_last_devID_poll) >= 1011) {
          (void) readDeviceStateAndID();
          m_last_devID_poll = ts;
        }
        return readAndDecode();
       } else {
          readPacket();
          m_model = guessModel();
          if (m_model == Model::SW_UNKNOWN) {
            // No data. 3d Pro analog mode?
            enableDigitalMode();
            readPacket();
            m_model = guessModel();
            if (m_model == Model::SW_UNKNOWN) {
              LEDs_ToggleLEDs(LEDS_LED2);
              delay(100);
            }
          }
          if (m_model != Model::SW_UNKNOWN) {
             connect();
          }
       }
       return true;
   }

  bool init() override {

    m_errors = 0;
    m_model = Model::SW_UNKNOWN;
    
    return true;
  }

  /// Read the device ID from the joystick.
  /// first trigger sets "DATA SEND" state
  /// second trigger wants "SEND ID" state
  /// the wheel doesn't send a 64bit packet. I tried with triggering after 4,5,..10 clocks
  /// for the wheel you just have to read a second time - definetly within less than 1ms

  int readDeviceStateAndID()  {
    uint16_t last_value = m_deviceStateAndID;

    readPacket(2);
    if (m_packet.size == 6) {
      m_deviceStateAndID = (m_packet.data[0]&1)<< 15 | m_packet.data[1] << 12 | m_packet.data[2] << 9 | 
                              m_packet.data[3] << 6  | m_packet.data[4] << 3  | m_packet.data[5];
      //if (isLogLevel(DEBUG_DETAIL))   log_buf(m_packet.data , m_packet.size);

      if (last_value != m_deviceStateAndID) {
//        if (isLogLevel(DEBUG_LOG_PID)) log_p(PSTR("new deviceState: 0x%02X"), m_deviceStateAndID);
        DeviceStateChanged(m_deviceStateAndID);
      }
    } else {
      //log_p(PSTR("readDeviceID failed size: %d"), m_packet.size);
    }
    /* avoid a too fast trigger pulse */
    delay(2);
    return m_packet.size == 6 ? 1:0;

  }


protected:

  /// Read positional and button state data from the joystick.
  /// the stick has to be in "Digital IDLE" state
  bool readAndDecode() {
        readPacket();
        State state;
        
        if (decode(m_packet, state)) {
          m_state = state;
          m_errors = 0;
          if (m_state.buttons != m_last_buttons) {
            for_each_button();
          }

          joystick_state.Button = m_state.buttons & 0x00ff;

          joystick_state.X = m_state.axes[0];
          joystick_state.Y = m_state.axes[1];
          joystick_state.Z = m_state.axes[2];

          m_last_buttons = m_state.buttons;
          return true;
        }
        m_errors++;
        if (m_errors > 3) {
          log_P(PSTR("decoding failed %d time(s)  packet.size: %d"), m_errors, m_packet.size);
        }
        if (m_errors > 7) {
          return init();
        }
        return false;
  }



          
  void for_each_button() {
#if 0
        uint16_t bn = 1;
        uint8_t button = 1;
        bool pressed = true;
        uint8_t tristate = 2;

        for (uint8_t b = 0; b < getDescription().numButtons; b++) {
          tristate = 2;
          if ((m_last_buttons & bn) && !(m_state.buttons & bn))
            tristate = pressed = false;  // released
          if ( !(m_last_buttons & bn) && (m_state.buttons & bn))
            tristate = pressed = true;   // freshly pressed
          
          if (tristate != 2) {
            log_P(PSTR("SwFFB:button: %02d - %s"), button, pressed?"on ":"off");

            switch (button) {
              case 1:
              case 2:
              case 3:
              case 4:
                if (  pressed) {
                  ;
                } else {
                  Ffbwheel_adjustForceGain( button -1);
                }
                break;
              case 5:
              case 6:
              case 7:
              case 8:
                if (  pressed) {
                  ;
                } else {
                  Ffbwheel_adjustCenterSpring( button -5);
                }
                break;
              default:
                  break;
            }
          }
          bn <<= 1;
          button++;  
        }
#endif
      }

};
