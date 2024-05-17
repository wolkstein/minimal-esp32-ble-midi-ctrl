/**
 * @file main.cpp
 * @author Michael Wolkstein (m.wolkstein@gmx.de)
 * @brief Simple example for a BLE MIDI controller with 5 buttons
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright (C) 2024 Michael Wolkstein m.wolkstein@gmx.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <Arduino.h>
#include <BLEMidi.h>
#include <Button2.h>
#include <FastLED.h>

// MIDI channel
#define MIDICHANNEL 0

// Button pins
#define PREV_REV 18
#define FUNCTION_STOP 19
#define NEXT_FWD 21
#define REC_ENABLED 22
#define PLAY_PAUSE 23

// WS28xx LED pin
#define WS28XX_LED_PIN 32
#define NUM_LEDS  1
#define BRIGHTNESS 15

// LED Strucutre
CRGB myWS28XXLED[NUM_LEDS];


// Selected function states
#define FUNCTION_IS_PLAYHEAD 0
#define FUNCTION_IS_SELECTTRACK 1



// State of the function buttons
bool _function_prev_next_btn_state = FUNCTION_IS_PLAYHEAD;

// Longpressed Buttuns
bool _longpressed_prev_rev = false;
bool _longpressed_function_stop = false;
bool _longpressed_next_fwd = false;
bool _longpressed_rec_enabled = false;

// Button objects
Button2 _prev_rev, _function_stop, _next_fwd, _rec_enabled, _play_pause;

// Callbacks
/**
 * @brief onNoteOn callback
 *
 * @param channel
 * @param note
 * @param velocity
 * @param timestamp
 */
void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity,
              uint16_t timestamp) {
  // Print received note on
  log_i(
      "Received note on : channel %d, note %d, velocity %d (timestamp %dms)\n",
      channel, note, velocity, timestamp);
}

/**
 * @brief onNoteOff callback
 *
 * @param channel
 * @param note
 * @param velocity
 * @param timestamp
 */
void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity,
               uint16_t timestamp) {
  // Print received note off
  log_i(
      "Received note off : channel %d, note %d, velocity %d (timestamp %dms)\n",
      channel, note, velocity, timestamp);
}

/**
 * @brief onControlChange callback
 *
 * @param channel
 * @param controller
 * @param value
 * @param timestamp
 */
void onControlChange(uint8_t channel, uint8_t controller, uint8_t value,
                     uint16_t timestamp) {
  log_i(
      "Received control change : channel %d, controller %d, value %d "
      "(timestamp %dms)\n",
      channel, controller, value, timestamp);
}

/**
 * @brief connected callback
 *
 */
void connected() {
  // device is BLE MIDI connected
  log_i("Connected");
  if (_function_prev_next_btn_state == FUNCTION_IS_PLAYHEAD) {
    myWS28XXLED[0] = CRGB::Green;
    FastLED.show();
  } else if (_function_prev_next_btn_state == FUNCTION_IS_SELECTTRACK) {
    myWS28XXLED[0] = CRGB::Purple;
    FastLED.show();
  }
}

/**
 * @brief disconnected callback
 * 
 */
void disconected() {
  // device is BLE MIDI disconnected
  log_i("Disconnected");
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();
}

/**
 * @brief btn_released callback
 *
 * @param btn
 */
void btn_released(Button2 &btn) {
  // if connected send midi
  if (BLEMidiServer.isConnected()) {
    // check which button was pressed
    // and send midi command

    // check for prev/rev button
    if (btn.getPin() == PREV_REV) {
      // check if button was longpressed
      if (_longpressed_prev_rev) {
        _longpressed_prev_rev = false;
        return;
      }
      // toggle function state of prev/rev button
      if (_function_prev_next_btn_state == FUNCTION_IS_PLAYHEAD) {
        BLEMidiServer.controlChange(MIDICHANNEL, 43, 127);
      } else if (_function_prev_next_btn_state == FUNCTION_IS_SELECTTRACK) {
        BLEMidiServer.controlChange(MIDICHANNEL, 111, 127);
      }
    }
    // check for stop button
    else if (btn.getPin() == FUNCTION_STOP) {
      // check if button was longpressed
      if (_longpressed_function_stop) {
        _longpressed_function_stop = false;
        return;
      }
      // send stop command
      BLEMidiServer.controlChange(MIDICHANNEL, 42, 127);
    }
    // check for next/fwd button
    else if (btn.getPin() == NEXT_FWD) {
      // check if button was longpressed
      if (_longpressed_next_fwd) {
        _longpressed_next_fwd = false;
        return;
      }
      if (_function_prev_next_btn_state == FUNCTION_IS_PLAYHEAD) {
        BLEMidiServer.controlChange(MIDICHANNEL, 44, 127);
      } else if (_function_prev_next_btn_state == FUNCTION_IS_SELECTTRACK) {
        BLEMidiServer.controlChange(MIDICHANNEL, 112, 127);
      }
    }
    // check for rec enabled button
    else if (btn.getPin() == REC_ENABLED) {
      // check if button was longpressed
      if (_longpressed_rec_enabled) {
        _longpressed_rec_enabled = false;
        return;
      }
      // send rec enabled command
      BLEMidiServer.controlChange(MIDICHANNEL, 64, 127);
    }
    // check for play/pause button
    else if (btn.getPin() == PLAY_PAUSE) {
      BLEMidiServer.controlChange(MIDICHANNEL, 41, 127);
    }
  }

  log_i("Button %d pressed\n", btn.getPin());
}

/**
 * @brief btn_longclick callback
 *
 * @param btn
 */
void btn_longclick(Button2 &btn) {
  // check which button was longpressed
  log_i("Button %d longclick\n", btn.getPin());
  // check for prev/rev button
  if (btn.getPin() == FUNCTION_STOP) {
    _function_prev_next_btn_state = !_function_prev_next_btn_state;
    _longpressed_function_stop = true;
    // check function state and set LED color
    if (_function_prev_next_btn_state == FUNCTION_IS_PLAYHEAD) {
      log_i("Function stop state: FUNCTION_IS_PLAYHEAD");
      myWS28XXLED[0] = CRGB::Green;
      FastLED.show();
    } else if (_function_prev_next_btn_state == FUNCTION_IS_SELECTTRACK) {
      log_i("Function stop state: FUNCTION_IS_SELECTTRACK");
      myWS28XXLED[0] = CRGB::Purple;
      FastLED.show();
    }
  }

  // if connected send midi
  if (BLEMidiServer.isConnected()) {
    // check which button was longpressed
    // btn rec enabled
    if (btn.getPin() == REC_ENABLED) {
      _longpressed_rec_enabled = true;
      BLEMidiServer.controlChange(MIDICHANNEL, 45, 127);
    }
    // btn prev/rev
    if (btn.getPin() == PREV_REV) {
      _longpressed_prev_rev = true;
      BLEMidiServer.controlChange(MIDICHANNEL, 20, 127);
    }
    // btn next/fwd
    if (btn.getPin() == NEXT_FWD) {
      _longpressed_next_fwd = true;
      BLEMidiServer.controlChange(MIDICHANNEL, 21, 127);
    }
  }
}

/**
 * @brief arduino setup
 *
 */
void setup() {

  // initialize WS28xx LED in GRB order
  FastLED.addLeds<WS2812B, WS28XX_LED_PIN, GRB>(myWS28XXLED, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();

  // initialize serial
  Serial.begin(57600);
  // wait for serial connection
  int timoutcounter = 0;
  while (!Serial) {
    delay(1000);
    timoutcounter++;
    if (timoutcounter > 10) {
      break;
    }
  }
  // initialize buttons
  _prev_rev.begin(PREV_REV);
  _function_stop.begin(FUNCTION_STOP);
  _next_fwd.begin(NEXT_FWD);
  _rec_enabled.begin(REC_ENABLED);
  _play_pause.begin(PLAY_PAUSE);
  // set debounce time
  _prev_rev.setDebounceTime(20);
  _function_stop.setDebounceTime(20);
  _function_stop.setLongClickTime(750);
  _next_fwd.setDebounceTime(20);
  _rec_enabled.setDebounceTime(20);
  _play_pause.setDebounceTime(20);
  // set calbacks
  // _prev_rev.setReleasedHandler(btn_released);
  _prev_rev.setReleasedHandler(btn_released);
  _prev_rev.setLongClickDetectedHandler(btn_longclick);
  _function_stop.setReleasedHandler(btn_released);
  _function_stop.setLongClickDetectedHandler(btn_longclick);
  _next_fwd.setReleasedHandler(btn_released);
  _next_fwd.setLongClickDetectedHandler(btn_longclick);
  _rec_enabled.setReleasedHandler(btn_released);
  _rec_enabled.setLongClickDetectedHandler(btn_longclick);
  _play_pause.setReleasedHandler(btn_released);

  // initialize BLE MIDI server
  BLEMidiServer.begin("Little_Helper");
  // set Midi callbacks
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback(disconected);
  BLEMidiServer.setNoteOnCallback(onNoteOn);
  BLEMidiServer.setNoteOffCallback(onNoteOff);
  BLEMidiServer.setControlChangeCallback(onControlChange);
}

/**
 * @brief ardunio main loop
 *
 */
void loop() {
  // check for button presses
  _prev_rev.loop();
  _function_stop.loop();
  _next_fwd.loop();
  _rec_enabled.loop();
  _play_pause.loop();
}
