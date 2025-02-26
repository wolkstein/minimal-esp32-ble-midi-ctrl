/**
 * @file main.cpp
 * @brief This file contains the main code for the Little Helper BLE MIDI Controller.
 * @author Michael Wolkstein (wolke@k-27.de)
 * @version 1.0
 * @date 2024-07-07
 * 
 * @details The Little Helper BLE MIDI Controller is a device that allows you to control MIDI devices wirelessly using Bluetooth Low Energy (BLE) technology. It uses an ESP32 microcontroller and various libraries such as Arduino, BLEMidi, FastLED, and AceButton.
 * 
 * The code initializes the necessary libraries, defines constants for the WiFi network, button configurations, and LED settings. It also includes callback functions for the web UI and button events.
 * 
 * The main functionality of the code is to handle button events, such as button presses and long presses. It retrieves the button configuration based on the GPIO pin and the active map. It then performs the corresponding actions, such as sending MIDI messages or updating the button state.
 * 
 * The code also includes functions for saving and loading button settings from the preferences, as well as web UI callbacks for selecting button mappings and MIDI functions.
 * 
 * This code is licensed under the GNU General Public License version 3 (GPL-3.0).
 */

#include "main.h"
#include <Arduino.h>
#include <BLEMidi.h>
#include <AceButton.h>
#include <Preferences.h>
#include <FastLED.h>
#include <fx/1d/cylon.h>
#include <elapsedMillis.h>

using namespace ace_button;
using namespace fl;

const int buttonPins[] = {10,11, 12, 13, 14}; // Beispiel-Pins für die Buttons
const int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

#define NUBER_OF_MAPS 2

elapsedMillis cyclonTimer;
elapsedMillis tapTempoTimer;

bool __isTab = false;

CRGB myWS28XXLED[NUM_LEDS];
Cylon cylon(NUM_LEDS);

bool __activeMap = 0;

bool __isTuner = false;

bool __isConnected = false;

CRGB __oldLedColor = CRGB::Red;

struct myButton
{
  uint8_t btnId;
  bool btnState[NUBER_OF_MAPS];
  uint8_t btnMidiCC[NUBER_OF_MAPS];
  uint8_t midiccValueHigh[NUBER_OF_MAPS];
  uint8_t midiccValueLow[NUBER_OF_MAPS];
  uint32_t btnColorOff[NUBER_OF_MAPS];
  uint32_t btnColorOn[NUBER_OF_MAPS];
};

myButton realButtons[buttonCount] = {
  {0, {false,false} , {37, 42}, {127, 127}, {0, 0}, {CRGB::Red, CRGB::Yellow}, {CRGB::Green, CRGB::Purple}},// RealBTN C and H
  {1, {false,false}, {36, 40}, {127, 127}, {0, 0}, {CRGB::Red, CRGB::Yellow}, {CRGB::Green, CRGB::Purple}},// RealBTN B and F
  {2, {false,false}, {35, 39}, {127, 127}, {0, 0}, {CRGB::Red, CRGB::Yellow}, {CRGB::Green, CRGB::Purple}},// RealBTN A and E
  {3, {false,false}, {44, 44}, {127, 127}, {0, 0}, {CRGB::Red, CRGB::Yellow}, {CRGB::Green, CRGB::Purple}}, // Toggle Tuner Tabtempo
  {4, {false,false}, {8, 9}, {127, 127}, {0, 0}, {CRGB::Red, CRGB::Yellow}, {CRGB::Green, CRGB::Purple}} // Switch Map
};

AceButton buttons[buttonCount];



void setLedColor(){
  CRGB led_A_BtnColor = CRGB::Black;
  CRGB led_B_BtnColor = CRGB::Black;
  CRGB led_C_BtnColor = CRGB::Black;

  if(realButtons[2].btnState[__activeMap]){
    led_A_BtnColor = realButtons[2].btnColorOn[__activeMap];
  } else
  {
    led_A_BtnColor = realButtons[2].btnColorOff[__activeMap];
  }
  if(realButtons[1].btnState[__activeMap]){
    led_B_BtnColor = realButtons[1].btnColorOn[__activeMap];
  } else
  {
    led_B_BtnColor = realButtons[1].btnColorOff[__activeMap];
  }
  if(realButtons[0].btnState[__activeMap]){
    led_C_BtnColor = realButtons[0].btnColorOn[__activeMap];
  } else
  {
    led_C_BtnColor = realButtons[0].btnColorOff[__activeMap];
  }

  if (__isConnected) {
    for(int i = 0; i < NUM_LEDS; i++){
      myWS28XXLED[i] = CRGB::Black;
    }
    myWS28XXLED[2] = led_A_BtnColor;
    myWS28XXLED[7] = led_B_BtnColor;
    myWS28XXLED[12] = led_C_BtnColor;
    FastLED.show();
  } else
  {
    myWS28XXLED[7] = CRGB::Red;
    FastLED.show();
    __oldLedColor = CRGB::Red;
  }

}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp)
{
    Serial.printf("Received control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
    if(controller == 35){ // cc from map 0
      if(value == 127){
        realButtons[2].btnState[0] = true;
      } else
      {
        realButtons[2].btnState[0] = false;
      }
      setLedColor();
    }
    if(controller == 36){ // cc from map 0
      if(value == 127){
        realButtons[1].btnState[0] = true;
      } else
      {
        realButtons[1].btnState[0] = false;
      }
      setLedColor();
    }
    if(controller == 37){ // cc from map 0
      if(value == 127){
        realButtons[0].btnState[0] = true;
      } else
      {
        realButtons[0].btnState[0] = false;
      }
      setLedColor();
    }
}

/**
 * @brief connected callback
 *
 */
void connected() {
  // device is BLE MIDI connected
  Serial.println("Connected");
  __isConnected = true;
  FastLED.setBrightness(__BRIGHTNESS / 3);
  if (__activeMap % 2 == 0) {
    myWS28XXLED[7] = CRGB::Green;
    FastLED.show();
    __oldLedColor = CRGB::Green;
  } else
  {
    myWS28XXLED[7] = CRGB::Purple;
    __oldLedColor = CRGB::Purple;
    FastLED.show();
  }

  setLedColor();
}

/**
 * @brief disconnected callback
 * 
 */
void disconected() {
  // device is BLE MIDI disconnected
  log_i("Disconnected");
  __isConnected = false;
  myWS28XXLED[7] = CRGB::Red;
  FastLED.setBrightness(__BRIGHTNESS / 3);
  FastLED.show();
  __oldLedColor = CRGB::Red;

}


void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventLongPressed: // Used for Toggle Tuner
      Serial.printf("Long Press : BTN %d\n", button->getId());
      if(button->getId() == 3 ){ // Toggle Tuner
        Serial.printf("Toggle Tuner\n");
        __isTuner = true;
        BLEMidiServer.controlChange(0, 45, 127);
      }
      break;
    case AceButton::kEventPressed: // Used for Taptempo
      Serial.printf("Press :%d\n", millis());
      if(button->getId() == 3 ){ // Taptempo
        Serial.printf("Taptempo\n");
        BLEMidiServer.controlChange(0,44,127);
        __isTab = true;
        tapTempoTimer = 0;
        if(__isTuner){
          __isTuner = false;
          BLEMidiServer.controlChange(0, 45, 0);
          setLedColor();
        }
      }
      if(button->getId() == 4){
        __activeMap = !__activeMap;
        if(__activeMap == 1){
          BLEMidiServer.controlChange(0, 48, 0);
        } else
        {
          BLEMidiServer.controlChange(0, 48, 127);
        }
        Serial.printf("Switch Map\n");
        setLedColor();
      }
      if(button->getId() == 0){ // BTN C
        realButtons[0].btnState[__activeMap] = !realButtons[0].btnState[__activeMap];
        if(realButtons[0].btnState[__activeMap]){
          BLEMidiServer.controlChange(0, realButtons[0].btnMidiCC[__activeMap], realButtons[0].midiccValueHigh[__activeMap]);
        } else
        {
          BLEMidiServer.controlChange(0, realButtons[0].btnMidiCC[__activeMap], realButtons[0].midiccValueLow[__activeMap]);
        }
        setLedColor();
      }
      if(button->getId() == 1){ // BTN B
        realButtons[1].btnState[__activeMap] = !realButtons[1].btnState[__activeMap];
        if(realButtons[1].btnState[__activeMap]){
          BLEMidiServer.controlChange(0, realButtons[1].btnMidiCC[__activeMap], realButtons[1].midiccValueHigh[__activeMap]);
        } else
        {
          BLEMidiServer.controlChange(0, realButtons[1].btnMidiCC[__activeMap], realButtons[1].midiccValueLow[__activeMap]);
        }
        setLedColor();
      }
      if(button->getId() == 2){ // BTN A
        realButtons[2].btnState[__activeMap] = !realButtons[2].btnState[__activeMap];
        if(realButtons[2].btnState[__activeMap]){
          BLEMidiServer.controlChange(0, realButtons[2].btnMidiCC[__activeMap], realButtons[2].midiccValueHigh[__activeMap]);
        } else
        {
          BLEMidiServer.controlChange(0, realButtons[2].btnMidiCC[__activeMap], realButtons[2].midiccValueLow[__activeMap]);
        }
        setLedColor();
      }
      break;
  }
}

void setup() {
  Serial.begin(57600);
    // initialize WS28xx LED in GRB order
  FastLED.addLeds<WS2812B, WS28XX_LED_PIN, GRB>(myWS28XXLED, NUM_LEDS);
  FastLED.setBrightness(6);
  myWS28XXLED[7] = CRGB::Red;
  FastLED.show();
  __oldLedColor = CRGB::Red;

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttons[i].init(buttonPins[i], HIGH, i);
    ButtonConfig* config = buttons[i].getButtonConfig();
    config->setEventHandler(handleEvent);
    config->setFeature(ButtonConfig::kFeatureLongPress);
    config->setDebounceDelay(20); // Setzt die Entprellzeit auf 20 Millisekunden
  }

    log_i("Starting BLE MIDI server");
  BLEMidiServer.begin(midiDeviceName.c_str());
  //BLEMidiServer.enableDebugging();
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback(disconected);
  // BLEMidiServer.setNoteOnCallback(onNoteOn);
  // BLEMidiServer.setNoteOffCallback(onNoteOff);
  BLEMidiServer.setControlChangeCallback(onControlChange);
  // BLEMidiServer.setProgramChangeCallback(onProgramChange);
}

int counter = 0;

void loop() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].check();
  }

  if(__isTuner && cyclonTimer > 1){
    cylon.draw(Fx::DrawContext(millis(), myWS28XXLED));
    FastLED.show();
    cyclonTimer = 0;
    //delay(cylon.delay_ms);
  }

  if(__isTab && tapTempoTimer > 1){
    cylon.draw(Fx::DrawContext(millis(), myWS28XXLED));
    FastLED.show();
    tapTempoTimer = 0;
    counter++;
    if(counter >= 30){
      __isTab = false;
      counter = 0;
      setLedColor();
    }

  }
}