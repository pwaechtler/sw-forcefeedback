// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

/// Simple GamePort pins to Arduino pins mapper.
template <int I>
struct GamePort {
  enum { pin = -1 };
};

template <>
struct GamePort<2> {    // /Button1 
  enum { pin = 10 };
};
template <>
struct GamePort<3> {  // X1
  enum { pin = A0 };
};                    // Gameport<4> and <5> GND
template <>
struct GamePort<6> {  // Y1
  enum { pin = A7 };
};
template <>
struct GamePort<7> {  // /Button2
  enum { pin = 8 };
};                    // Gameport<1>, <8> and <9>   +5V
template <>
struct GamePort<10> { // /Button4
  enum { pin = 16 };
};
template <>
struct GamePort<11> { // X2
  enum { pin = A1 };
};
template <>
struct GamePort<12> {   // MIDI_TX/OUT
  enum { pin = 5 };
};
template <>
struct GamePort<13> { // Y2
  enum { pin = A6 };
};
template <>
struct GamePort<14> { // /Button3
  enum { pin = 7 };
};
template <>
struct GamePort<15> { // MIDI_RX/IN
  enum { pin = 9 };
};
