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
#include <FastLED.h>
#include <AceButton.h>
#include <Preferences.h>


void setup(){
  Serial.begin(57600);
  while(!Serial);
  
}

void main(){
  Serial.printf("Hello World at:%d\n",millis());
  delay(1000);
}