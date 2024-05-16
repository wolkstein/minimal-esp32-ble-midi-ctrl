/**
 * @file main.cpp
 * @author Michael Wolkstein (m.wolkstein@gmx.de)
 * @brief Simple example for a BLE MIDI controller with 5 buttons
 * @version 0.1
 * @date 2024-05-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <Arduino.h>
#include <BLEMidi.h>
#include <Button2.h>

// MIDI channel
#define MIDICHANNEL 0

// Button pins
#define PREV_REV 18
#define FUNCTION_STOP 19
#define NEXT_FWD 21
#define REC_ENABLED 22
#define PLAY_PAUSE 23

// Selected function states
#define FUNCTION_IS_PLAYHEAD 0
#define FUNCTION_IS_SELECTTRACK 1

// State of the function buttons
bool _function_prev_next_btn_state = FUNCTION_IS_PLAYHEAD;

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
void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
  // Print received note on
  log_i("Received note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
}

/**
 * @brief onNoteOff callback
 * 
 * @param channel 
 * @param note 
 * @param velocity 
 * @param timestamp 
 */
void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp)
{
  // Print received note off
  log_i("Received note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
}

/**
 * @brief onControlChange callback
 * 
 * @param channel 
 * @param controller 
 * @param value 
 * @param timestamp 
 */
void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp)
{
    log_i("Received control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
}

/**
 * @brief connected callback
 * 
 */
void connected()
{
  log_i("Connected");
}

/**
 * @brief btn_pressed callback
 * 
 * @param btn 
 */
void btn_pressed(Button2 &btn)
{
  log_i("Button %d pressed\n", btn.getPin());
  
  // if connected send midi
  if(BLEMidiServer.isConnected()){
    if(btn.getPin() == PREV_REV){
      if(_function_prev_next_btn_state == FUNCTION_IS_PLAYHEAD){
        BLEMidiServer.controlChange(MIDICHANNEL, 43, 127);
      } else if(_function_prev_next_btn_state == FUNCTION_IS_SELECTTRACK){
        BLEMidiServer.controlChange(MIDICHANNEL, 111, 127);
      }
    } else if(btn.getPin() == FUNCTION_STOP){
      BLEMidiServer.controlChange(MIDICHANNEL, 42, 127);
    } else if(btn.getPin() == NEXT_FWD){
      if(_function_prev_next_btn_state == FUNCTION_IS_PLAYHEAD){
        BLEMidiServer.controlChange(MIDICHANNEL, 44, 127);
      } else if(_function_prev_next_btn_state == FUNCTION_IS_SELECTTRACK){
        BLEMidiServer.controlChange(MIDICHANNEL, 112, 127);
      }
    } else if(btn.getPin() == REC_ENABLED){
      BLEMidiServer.controlChange(MIDICHANNEL, 64, 127);
    } else if(btn.getPin() == PLAY_PAUSE){
      BLEMidiServer.controlChange(MIDICHANNEL, 41, 127);
    }
  }
}

/**
 * @brief btn_longclick callback
 * 
 * @param btn 
 */
void btn_longclick(Button2 &btn)
{
  log_i("Button %d longclick\n", btn.getPin());

  if(btn.getPin() == FUNCTION_STOP){
    _function_prev_next_btn_state = !_function_prev_next_btn_state;
    if (_function_prev_next_btn_state == FUNCTION_IS_PLAYHEAD) {
      log_i("Function stop state: FUNCTION_IS_STOP");
    } else if (_function_prev_next_btn_state == FUNCTION_IS_SELECTTRACK) {
      log_i("Function stop state: FUNCTION_IS_SELECTTRACK");
    }
  }

  // if connected send midi
  if(BLEMidiServer.isConnected()){
    if(btn.getPin() == REC_ENABLED){
      BLEMidiServer.controlChange(MIDICHANNEL, 45, 127);
    }

    if(btn.getPin() == PREV_REV){
      BLEMidiServer.controlChange(MIDICHANNEL, 20, 127);
    }

    if(btn.getPin() == NEXT_FWD){
      BLEMidiServer.controlChange(MIDICHANNEL, 21, 127);
    }
  }
}

/**
 * @brief arduino setup
 * 
 */
void setup() {

  Serial.begin(57600);
  // wait for serial connection
  int timoutcounter = 0;
  while(!Serial){
    delay(1000);
    timoutcounter++;
    if(timoutcounter > 10){
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
  _next_fwd.setDebounceTime(20);
  _rec_enabled.setDebounceTime(20);
  _play_pause.setDebounceTime(20);
  // set calbacks
  _prev_rev.setPressedHandler(btn_pressed);
  _prev_rev.setLongClickDetectedHandler(btn_longclick);
  _function_stop.setPressedHandler(btn_pressed);
  _function_stop.setLongClickDetectedHandler(btn_longclick);
  _next_fwd.setPressedHandler(btn_pressed);
  _next_fwd.setLongClickDetectedHandler(btn_longclick);
  _rec_enabled.setPressedHandler(btn_pressed);
  _rec_enabled.setLongClickDetectedHandler(btn_longclick);
  _play_pause.setPressedHandler(btn_pressed);

  // initialize BLE MIDI server
  BLEMidiServer.begin("Little_Helper");
  // set Midi callbacks
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback([](){
    log_i("Disconnected");
  });
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
