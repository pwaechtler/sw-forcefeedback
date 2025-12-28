// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by // the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
#include "board.h"
#include "DigitalPin.h"

#ifndef SAVE_SPACE
#include "HidJoystick.h"
#include "CHFlightstickPro.h"
#include "GrIP.h"
#include "Logitech.h"
#include "ThrustMaster.h"
#endif
#include "GenericJoystick.h"
#include "Sidewinder.h"
#include "SwFFB.h"

#include "lufa-iface.h"
#include "debug.h"

static SwFFB joystick;


#ifndef SAVE_SPACE
static Joystick *createJoystick(byte sw) {
  switch (sw) {
    case 0b0001:
      return new GenericJoystick<2,4>;
    case 0b0010:
      return new GenericJoystick<3,4>;
    case 0b0011:
      return new GenericJoystick<4,4>;

    case 0b0111:
      return new Sidewinder;
    case 0b1111:
      return new SwFFB;
    case 0b0100:
      return new CHFlightstickPro;
    case 0b0101:
      return new ThrustMaster;
    case 0b1000:
      return new GrIP;
    case 0b1001:
      return new Logitech;
    default:
      return new SwFFB;
  }
}
#endif

void setup() {
  SetupHardware(); 


//  Serial.begin(9600);
  //while (!Serial);
#if 0
  const auto sw1 = DigitalInput<14, true>{}.isLow();
  const auto sw2 = DigitalInput<15, true>{}.isLow();
  const auto sw3 = DigitalInput<20, true>{}.isLow();
  const auto sw4 = DigitalInput<21, true>{}.isLow();

  const auto sw = sw4 << 3 | sw3 << 2 | sw2 << 1 | sw1;
  joystick.init(createJoystick(sw));
#endif
  joystick.init();

}


void loop() {
  static unsigned long last_led_toggle;
  static unsigned long last_poll;
  unsigned long ts = millis();
  
  taskLoop();
  /* don't read too fast, it upsets the wheels state machine */
  if ( (ts - last_poll) >= 8) {
    joystick.update();
    last_poll = ts;
  }

  if (ts - last_led_toggle > 1000) {
    LEDs_ToggleLEDs(LEDS_LED2);
    last_led_toggle = ts;
  }

}
