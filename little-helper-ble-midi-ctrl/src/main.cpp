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

#define USE_OTA
#include "main.h"
#include <Arduino.h>
#include <BLEMidi.h>
#include <FastLED.h>
#include <AceButton.h>
#include <Preferences.h>
//#include "esp32-hal-log.h"
#include "esp_log.h"

#include <DNSServer.h>
#include <ESPUI.h>

#include <WiFi.h>

// // ota network requirements
#ifdef USE_OTA
  #include <HTTPClient.h>
  #include <WiFiClientSecure.h>
  #include <Update.h>
#endif


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 8, 1);
DNSServer dnsServer;


// // OTA Update globals
// // Constants for server details
#ifdef USE_OTA
  const char* SERVER = "https://raw.githubusercontent.com"; // Your server address
  const int SERVER_PORT = 443; // Typically 443 for HTTPS
  const char* PATH = "/wolkstein/minimal-esp32-ble-midi-ctrl/main/bin/s3miniV"; // Path to the firmware
  bool __ota_update_running = false;
#endif



unsigned int __FW_VERSION = 8; // Firmware Version only Major Versions number
bool __DO_UPDATE = false;
bool __UPDATE_FAILURE = false;
int __UPDATE_ERROR_CODE = -1;

Preferences prefs;

using namespace ace_button;

// LED Strucutre
CRGB myWS28XXLED[NUM_LEDS];

CRGB __oldLedColor;

uint8_t __numBlincs = 1;
uint32_t __oldNumBlinktime = 0;
uint32_t __oldBlinktime = 0;

// Button Structure now with n Maps, first we try 4 Maps
myButton myBtnMap[5] = { // 5 Buttons 4 Maps Map 1 und Map 2 are short press values, Map 3 and Map 4 are long press values
  { // Button 1
     10,  // GPIO Pin
     {false, false, false, false}, // Button Release
     {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
     {false, false, false, false}, // Button Long Press
     {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
     {CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
     {MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
     {MIDI_CH_1, MIDI_CH_1, MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
     {60, 60, 60, 60}, // Button MIDI Note 0 - 127
     {100, 100,100, 100}, // Button MIDI Velocity 0 - 127
     {43, 111, 43, 111}, // Button MIDI CC 0 - 127
     {127, 127, 127, 127}, // Button MIDI CC ON Value 0 - 127
     {0, 0, 0, 0}, // Button MIDI CC OFF Value 0 - 127
     {MMC_REWIND, MMC_REWIND, MMC_REWIND, MMC_REWIND} // Button MIDI MMC 0 - 13
  },
  { // Button 2
     11,  // GPIO Pin
     {false, false, false, false}, // Button Release
     {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
     {false, false, false, false}, // Button Long Press
     {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
     {CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, CRGB::Yellow}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
     {MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
     {MIDI_CH_1, MIDI_CH_1, MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
     {61, 61, 61, 61}, // Button MIDI Note 0 - 127
     {100, 100, 100, 100}, // Button MIDI Velocity 0 - 127
     {42, 42, 42, 42}, // Button MIDI CC 0 - 127
     {127, 127, 127, 127}, // Button MIDI CC ON Value 0 - 127
     {0, 0, 0, 0}, // Button MIDI CC OFF Value 0 - 127
     {MMC_STOP, MMC_STOP, MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
  },
  { // Button 3
     12,  // GPIO Pin
     {false, false, false, false}, // Button Release
     {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
     {false, false, false, false}, // Button Long Press
     {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
     {CRGB::Cyan, CRGB::Cyan, CRGB::Cyan, CRGB::Cyan}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
     {MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
     {MIDI_CH_1, MIDI_CH_1, MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
     {62, 62, 62, 62}, // Button MIDI Note 0 - 127
     {100, 100, 100, 100}, // Button MIDI Velocity 0 - 127
     {44, 112, 44, 112}, // Button MIDI CC 0 - 127
     {127, 127, 127, 127}, // Button MIDI CC ON Value 0 - 127
     {0, 0, 0, 0}, // Button MIDI CC OFF Value 0 - 127
     {MMC_STOP, MMC_STOP, MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
  },
  { // Button 4
     13,  // GPIO Pin
     {true, true, true, true}, // Button Release
     {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
     {false, true, false, true}, // Button Long Press
     {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
     {CRGB::Aquamarine, CRGB::Aquamarine, CRGB::Aquamarine, CRGB::Aquamarine}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
     {MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
     {MIDI_CH_1, MIDI_CH_1, MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
     {62, 62, 62, 62}, // Button MIDI Note 0 - 127
     {100, 100, 100, 100}, // Button MIDI Velocity 0 - 127
     {45, 64, 45, 64}, // Button MIDI CC 0 - 127
     {127, 127, 127, 127}, // Button MIDI CC ON Value 0 - 127
     {0, 0, 0, 0}, // Button MIDI CC OFF Value 0 - 127
     {MMC_STOP, MMC_STOP, MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
  },
  { // Button 5
     14,  // GPIO Pin
     {false, false, false, false}, // Button Release
     {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
     {false, false, false, false}, // Button Long Press
     {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
     {CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
     {MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
     {MIDI_CH_1, MIDI_CH_1, MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
     {62, 62, 62, 62}, // Button MIDI Note 0 - 127
     {100, 100, 100, 100}, // Button MIDI Velocity 0 - 127
     {41, 41, 41, 41}, // Button MIDI CC 0 - 127
     {127, 127, 127, 127}, // Button MIDI CC ON Value 0 - 127
     {0, 0, 0, 0}, // Button MIDI CC OFF Value 0 - 127
     {MMC_STOP, MMC_STOP, MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
  },
};


AceButton btn1(myBtnMap[0].btnGpio); // GPIO 10
AceButton btn2(myBtnMap[1].btnGpio); // GPIO 11
AceButton btn3(myBtnMap[2].btnGpio); // GPIO 12
AceButton btn4(myBtnMap[3].btnGpio); // GPIO 13
AceButton btn5(myBtnMap[4].btnGpio); // GPIO 14

myButton* getMyButton(int pin) {
    switch(pin) {
        case 10: return &myBtnMap[0];
        case 11: return &myBtnMap[1];
        case 12: return &myBtnMap[2];
        case 13: return &myBtnMap[3];
        case 14: return &myBtnMap[4];
        default: return nullptr; // oder einen gültigen Standardwert zurückgeben
    }
}

#ifdef USE_OTA

void otaUpdate(Control* sender, int type) {
  // set an boot variable into nvs to check next time boot.
  prefs.begin("doupdate");  //Open namespace Settings
  prefs.putBool("doupdate", true);
  prefs.end(); // close the Settings Namespace

  ESPUI.updateControlValue(sender, "Update requested need Reeboot");
}

void justotaUpdate() {

  // reset firmware update flag
  prefs.begin("doupdate");  //Open namespace Settings
  prefs.putBool("doupdate", false);
  prefs.end(); // close the Settings Namespace

  __UPDATE_FAILURE = true; // set this true, while this function is running
  prefs.begin("updatefail");  //Open namespace Settings
  prefs.putBool("updatefail", __UPDATE_FAILURE);
  prefs.end(); // close the Settings Namespace

  if (__ota_update_running) {
    Serial.println("OTA update already running.");
    return;
  }
  
  __ota_update_running = true;
  WiFiClientSecure wifiClientSSL;
  wifiClientSSL.setInsecure(); // Not recommended for production, better to handle certificates properly

  HTTPClient https;
  char updateURL[100];
  snprintf(updateURL, sizeof(updateURL), "%s%s%d.bin", SERVER, PATH, __FW_VERSION + 1);

  Serial.print("Checking for update file: ");
  Serial.println(updateURL);

  https.begin(wifiClientSSL, updateURL); // Begin connection

  int httpCode = https.GET(); // Make the GET request

  Serial.print("Update status code: ");
  Serial.println(httpCode);

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("Failed to fetch update.");
    https.end();
    __ota_update_running = false;
    __UPDATE_ERROR_CODE = httpCode;
    prefs.begin("updateerrorcode");  //Open namespace Settings
    prefs.putInt("updateerrorcode", __UPDATE_ERROR_CODE);
    prefs.end(); // close the Settings Namespace 
    ESP.restart();
    return;
  }

  int contentLength = https.getSize();
  if (contentLength <= 0) {
    Serial.println("Invalid content length. Can't continue with update.");
    https.end();
    __ota_update_running = false;
    return;
  }

  Serial.print("Server returned update file of size ");
  Serial.print(contentLength);
  Serial.println(" bytes");

  // Prepare for the update
  Serial.print("Free heap before OTA: ");
  Serial.println(ESP.getFreeHeap());


  if (!Update.begin(contentLength, U_FLASH, 15, 0, NULL)) {
      Serial.println("Not enough space to begin OTA");
      https.end();
      __ota_update_running = false;
      return;
  } else {
      Serial.println("OTA Update started");
  }

  size_t written = 0; // Variable to store how many bytes have been written
  WiFiClient *stream = https.getStreamPtr();
  Serial.println("stream-angefordert");
  int schleifeaussen = 0;
  https.setTimeout(60000); // 12 Sekunden Timeout

  uint8_t buff[128] = { 0 }; // Größe des Buffers anpassen
  size_t len = 0; // Variable to store the length of data available for reading

  bool toggleLed = false;
  while (https.connected() && (written < contentLength)) {

      len = stream->available();
      delay(2); // Kleine Verzögerung, um dem Stream Zeit zu geben, Daten zu sammeln

      if (len > 0) {
          int c = stream->readBytes(buff, ((len < sizeof(buff)) ? len : sizeof(buff)));
          if (Update.write(buff, c) != c) {
              Serial.println("Error writing to flash. Aborting OTA.");
              Update.abort();
              return; // Verlässt die Funktion oder Schleife
          }
          

          written += c;
          //Serial.printf("len: %d, written: %d\n", len, written);
          yield(); // Ermöglicht das Ausführen von Hintergrundaufgaben, verhindert WDT-Reset
      }

      if(toggleLed) {
        myWS28XXLED[0] = CRGB::Purple;
        FastLED.show();
      } else {
        myWS28XXLED[0] = CRGB::Black;
        FastLED.show();
      }
      toggleLed = !toggleLed;
  }

  if (written == contentLength) {
      Serial.println("Written : " + String(written) + " successfully");
  }

  if (Update.end()) {
      Serial.println("OTA done!");
      if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting.");
          __UPDATE_FAILURE = false; // set this true, while this function is running
          prefs.begin("updatefail");  //Open namespace Settings
          prefs.putBool("updatefail", __UPDATE_FAILURE);
          prefs.end(); // close the Settings Namespace

          __UPDATE_ERROR_CODE = -1;
          prefs.begin("updateerrorcode");  //Open namespace Settings
          prefs.putInt("updateerrorcode", __UPDATE_ERROR_CODE);
          prefs.end(); // close the Settings Namespace 

          Serial.println("OTA Done!");
          Serial.printf("update fw version: %d\n", __FW_VERSION + 1);
          __FW_VERSION++;
          prefs.begin("fwversion");  //Open namespace Settings
          prefs.putUInt("fwversion", __FW_VERSION);
          prefs.end(); // close the Settings Namespace
          Serial.println("Rebooting...");
          ESP.restart();
      } else {
          Serial.println("Update not finished? Something went wrong!");
          __ota_update_running = false;
      }
  } else {
      Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      __ota_update_running = false;
  }
}
#endif

// ~ OTA ~
// helper function to get the button configuration based on the GPIO pin and the active map
void saveActiveMap() {
  __numBlincs = (__active_map + 1) * 2;
    prefs.begin("active_map"); // Open NVS namespace "Settings" in RW mode
    prefs.putUInt("active_map", __active_map); // Store the active map
    prefs.end(); // Close NVS
    Serial.printf("Save Active Map: %d\n", __active_map);
    if (__active_map %2 == 0) {
      myWS28XXLED[0] = CRGB::Green;
      __oldLedColor = CRGB::Green;
    } else {
      myWS28XXLED[0] = CRGB::Purple;
      __oldLedColor = CRGB::Purple;
    }
    if(!__isConnected) myWS28XXLED[0] = CRGB::Red;
    FastLED.show();
}

// WEB UI Callbacks
void nothing(Control* sender, int type) {
    // Do nothing
}

void selectActiveMap(Control* sender, int value) {
    uint8_t active_map = static_cast<uint8_t>(String(sender->value).toInt());
    __active_map = active_map;
    __numBlincs = (__active_map + 1) * 2;
    saveActiveMap();
}

void switchShowPasswords(Control* sender, int type) {

  if(sender->value == "0") {
    ESPUI.setInputType(wlanApPasswordTxtField, "password");
    ESPUI.setInputType(wlanPasswordTxtField, "password");
  } else if(sender->value == "1") {  
    ESPUI.setInputType(wlanApPasswordTxtField, "text");
    ESPUI.setInputType(wlanPasswordTxtField, "text");
  }
}

void textCallBlueThoothName(Control* sender, int type)
{
    String value = sender->value;

    if(value.length() < 8) {
      ESPUI.updateControlValue(bleNameTxtField, "Name to short");
      return;
    }
    // Check for spaces in the string
    if (value.indexOf(' ') >= 0) {
      ESPUI.updateControlValue(bleNameTxtField, "Name contains spaces");
      return;
    }
    prefs.begin("blename", false); // Open NVS namespace "bluetooth" in RW mode
    prefs.putString("blename", value); // Store Bluetooth name
    prefs.end(); // Close NVS
    
}

void textCallSsidName(Control* sender, int type) {
    String value = sender->value;

    if(value.length() < 8) {
      ESPUI.updateControlValue(wlanSsidNameTxtField, "Name to short");
      return;
    }
    // Check for spaces in the string
    if (value.indexOf(' ') >= 0) {
      ESPUI.updateControlValue(wlanSsidNameTxtField, "Name contains spaces");
      return;
    }
    prefs.begin("wifi", false); // Open NVS namespace "wifi" in RW mode
    
    prefs.putString("ssid_local", value); // Store SSID
    
    prefs.end(); // Close NVS
    

}

void textCallWlanPassword(Control* sender, int type) {
    String value = sender->value;

    if(value.length() < 8) {
      ESPUI.updateControlValue(wlanPasswordTxtField, "Name to short");
      return;
    }
    // Check for spaces in the string
    if (value.indexOf(' ') >= 0) {
      ESPUI.updateControlValue(wlanPasswordTxtField, "Name contains spaces");
      return;
    }  
    prefs.begin("wifi", false);
    
    prefs.putString("password_local", value); // Store password
    
    prefs.end();
    

}

void textCallAPSsidName(Control* sender, int type) {
    String value = sender->value;

    if(value.length() > 16) {
      ESPUI.updateControlValue(wlanApSsidTxtField, "Name to long");
      return;
    }
    // Check for spaces in the string
    if (value.indexOf(' ') >= 0) {
      ESPUI.updateControlValue(wlanApSsidTxtField, "Name contains spaces");
      return;
    }
    prefs.begin("wifi", false); // Open NVS namespace "wifi" in RW mode
    
    prefs.putString("ssid_ap", value); // Store SSID
    
    prefs.end(); // Close NVS
    

}

void textCallAPPassword(Control* sender, int type) {
    String value = sender->value;

    if(value.length() < 8) {
      ESPUI.updateControlValue(wlanApPasswordTxtField, "Name to short");
      return;
    }
    // Check for spaces in the string
    if (value.indexOf(' ') >= 0) {
      ESPUI.updateControlValue(wlanApPasswordTxtField, "Name contains spaces");
      return;
    }
    prefs.begin("wifi", false);
    
    prefs.putString("password_ap", value); // Store password
    
    prefs.end();
    

}

void textCallHostname(Control* sender, int type) {
    String value = sender->value;

    if(value.length() < 8) {
      ESPUI.updateControlValue(hostnameTxtField, "Name to short");
      return;
    }
    // Check for spaces in the string
    if (value.indexOf(' ') >= 0) {
      ESPUI.updateControlValue(hostnameTxtField, "Name contains spaces");
      return;
    }
    WiFi.setHostname(value.c_str());
    prefs.begin("wifi", false);
    
    prefs.putString("hostname", value); // Store password
    
    prefs.end();
    
}

void textCallLedBrightness(Control* sender, int type) {


    __BRIGHTNESS = sender->value.toInt();
    FastLED.setBrightness(__BRIGHTNESS);
    FastLED.show();

    prefs.begin("wifi", false); // Open NVS namespace "wifi" in RW mode
    prefs.putUInt("LedBrightness", __BRIGHTNESS); // Store password
    
    prefs.end();
    
}

void saveSettings() {
    prefs.begin("Settings"); // Open NVS namespace "Settings" in RW mode
    
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
    
    prefs.end();
    
}


void updateUiActiveMap(){
    char str[10];
    sprintf(str, "%d", __active_map); // Convert the number to a string
    ESPUI.updateControlValue(activeMapChooser, str); // Update the control value
}

// Button 1 - x Web UI Callbacks ---------
void selectBtnMapFnc(Control* sender, int value) {

    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    
    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][0] == sender->id) {
        active_btn = i;
        break;
      }
    }

  __active_map_ui_btn[active_btn] = value_t;

    // // Enable or disable the controls based on the selected map
    // ESPUI.setEnabled(__selectUiBtn[active_btn][9], true);
    // ESPUI.setPanelStyle(__selectUiBtn[active_btn][9], ";");
    
    //update the value in the settings
    char str[10]; // Ensure this is large enough to hold the number and the null terminator

    uint8_t localvalue = myBtnMap[active_btn].btnMidiChannel[value_t]; // Get the MidiChannel value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][1], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnMidiFunction[value_t]; // get the MidiFunction value from the settings
    if(localvalue == 0){
       // ESPUI.setEnabled(__selectUiBtn[active_btn][9], false);
    }
    sprintf(str, "%u", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][2], str); // Update the control value
    log_d("Active Button: %d, String: %S, Value: %u\n", active_btn, str,localvalue);

    localvalue = myBtnMap[active_btn].btnMidiCC[value_t]; // Get the MidiCC value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][3], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnMidiCCValueStateOn[value_t]; // Get the MidiCCValueStateOn value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][4], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnMidiCCValueStateOff[value_t]; // Get the MidiCCValueStateOff value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][5], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnMidiNote[value_t]; // Get the MidiNote value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][6], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnMidiMMC[value_t]; // Get the MidiNote value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][7], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnMidiVelocity[value_t]; // Get the MidiVelocity value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][8], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnFunction[value_t]; // Get the Function value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][9], str); // Update the control value

    localvalue = myBtnMap[active_btn].needRelease[value_t]; // Get the NeedRelease value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][10], str); // Update the control value

    localvalue = myBtnMap[active_btn].btnColor[value_t]; // Get the Color value from the settings

    int colorval = 0;
    for(int i = 0; i < 141; i++) {
      if(__btnLookUpTable[i] == localvalue) {
        colorval = i;
        break;
      }
    }
    sprintf(str, "%d", colorval); // Convert the number to a string
    ESPUI.updateControlValue(__selectUiBtn[active_btn][11], str); // Update the control value

}

void selectBtnMidiChannelCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][1] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiChannel[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();

}

void selectBtnMidiFnc(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][2] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiFunction[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();

}
// ---- hier geht es weiter
void selectBtnMidiCCFunctionCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][3] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiCC[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();
}

void selectBtnCCValueMaxCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][4] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiCCValueStateOn[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();
}

void selectBtnCCValueMinCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][5] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiCCValueStateOff[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();
}

void selectBtnMidiNoteCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][6] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiNote[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();
}

void selectBtnMMCFnc(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][7] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiMMC[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();  
}

void selectBtnNoteVelocityCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][8] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnMidiVelocity[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();
}

void selectBtnBehaveFncCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][9] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].btnFunction[__active_map_ui_btn[active_btn]] = value_t;
    saveSettings();
}



void selectBtnTransitinCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][10] == sender->id) {
        active_btn = i;
        break;
      }
    }

    log_d("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[active_btn].needRelease[__active_map_ui_btn[active_btn]] = (bool)value_t;
    saveSettings();
}

void selectBtnColorCalback(Control *sender, int type) {

    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    int active_btn = 0;
    for(int i = 0; i < __HW_BUTTONS; i++) {
      if(__selectUiBtn[i][11] == sender->id) {
        active_btn = i;
        break;
      }
    }

    static char stylecol1[60], stylecol2[30]; 
    

    //uint32_t oppositeColor = 0xFFFFFF - __btnLookUpTable[value_t];

    sprintf(stylecol1, "border-bottom: #999 3px solid; background-color: #%06X;", __btnLookUpTable[value_t] );

    //sprintf(stylecol2, "background-color: #%06X;", oppositeColor);
    
    ESPUI.setPanelStyle(sender->id, stylecol1);
    //ESPUI.setElementStyle(sender->id, stylecol1);
    
    myBtnMap[active_btn].btnColor[__active_map_ui_btn[active_btn]] = __btnLookUpTable[value_t];
    saveSettings();


}

// ~ WEB UI Callbacks


// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /*buttonState*/) { 
    
    uint8_t pin = button->getPin();

    myButton* myBtn = getMyButton(pin);
    if (myBtn != nullptr) {
        log_d("Button %d found\n", pin);
    } else {
        log_d("Button %d not found\n", pin);
        return;
    }
   
    bool logpressevent = false;
    if(eventType == AceButton::kEventLongPressed) {
      logpressevent = true;
    }

    uint8_t active_mapper = __active_map;
    // 4 Maps for each Button Map1 = Short Press, Map2 = Short Press, Map3 = Long Press, Map4 = Long Press
    //if(logpressevent) active_mapper = __active_map + 2; // 0 = map 1, 1 = map 2, 2 = map 3, 3 = map 4
    
    bool needRelease = myBtn->needRelease[active_mapper];
    log_d("BTN: %d, Map:%d, NeedRelease:%d, Test valid Gpio %d \n", pin, active_mapper, needRelease, myBtn->btnGpio);
    uint8_t btnFunction = myBtn->btnFunction[active_mapper]; // 0 = Push, 1 = Toggle
    bool btnLongpress = myBtn->btnLongpress[active_mapper]; // 0 = Short Press, 1 = Long Press
    uint8_t btnState = myBtn->btnState[active_mapper]; // 0 = Off, 1 = On
    uint32_t btnColor = myBtn->btnColor[active_mapper]; // 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
    uint8_t btnMidiFunction = myBtn->btnMidiFunction[active_mapper]; // 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
    uint8_t btnMidiChannel = myBtn->btnMidiChannel[active_mapper]; // 0 - 15  MIDI Channel
    uint8_t btnMidiNote = myBtn->btnMidiNote[active_mapper]; // 0 - 127 MIDI Note
    uint8_t btnMidiVelocity = myBtn->btnMidiVelocity[active_mapper]; // 0 - 127 MIDI Velocity
    uint8_t btnMidiCC = myBtn->btnMidiCC[active_mapper]; // 0 - 127 MIDI CC
    uint8_t btnMidiCCValueStateOn = myBtn->btnMidiCCValueStateOn[active_mapper]; // 0 - 127 MIDI CC Value State On
    uint8_t btnMidiCCValueStateOff = myBtn->btnMidiCCValueStateOff[active_mapper]; // 0 - 127 MIDI CC Value State Off
    uint8_t btnMidiMMC = myBtn->btnMidiMMC[active_mapper]; // 0 - 13 MIDI MMC

    CRGB tmpBtncolor = (CRGB::HTMLColorCode)btnColor;

    // check the current button state BTN_ON

    switch (eventType) {
      case AceButton::kEventPressed:
        log_i("handleEvent(): BTN: %d Pressed", pin);
        log_d("BTN: %d Pressed, Map:%d\n ", pin, __active_map);


        if(btnMidiFunction == MIDI_NOTE) { // Note on need short press event
          if(btnFunction == BTN_PUSH){ // Push Button
            BLEMidiServer.noteOn(btnMidiChannel, btnMidiNote, btnMidiVelocity);
            myBtn->btnState[active_mapper] = BTN_ON;
          }
          if(btnFunction == BTN_TOGGLE){ // Toggle Button
            if(btnState == BTN_OFF){
              BLEMidiServer.noteOn(btnMidiChannel, btnMidiNote, btnMidiVelocity);
              myBtn->btnState[active_mapper] = BTN_ON;
            }
            else if(btnState == BTN_ON){
              BLEMidiServer.noteOff(btnMidiChannel, btnMidiNote, 0 );
              myBtn->btnState[active_mapper] = BTN_OFF;
            }
          }
        }
        else if(btnMidiFunction == MIDI_CC && !needRelease){ // CC on need short press event
          if(btnFunction == BTN_PUSH){ // Push Button
            BLEMidiServer.controlChange(btnMidiChannel, btnMidiCC, btnMidiCCValueStateOn);
            myBtn->btnState[active_mapper] = BTN_ON;
          }
          if(btnFunction == BTN_TOGGLE){ // Toggle Button
            if(btnState == BTN_OFF){
              BLEMidiServer.controlChange(btnMidiChannel, btnMidiCC, btnMidiCCValueStateOn);
              myBtn->btnState[active_mapper] = BTN_ON;
            }
            else if(btnState == BTN_ON){
              BLEMidiServer.controlChange(btnMidiChannel, btnMidiCC, btnMidiCCValueStateOff);
              myBtn->btnState[active_mapper] = BTN_OFF;
            }
          }
        }
        else if(btnMidiFunction == MIDI_MMC && !needRelease){
          Serial.printf("MMC: %u\n", btnMidiMMC);
          switch (btnMidiMMC)
          {
          case MMC_STOP:
          BLEMidiServer.mmcStop();
            BLEMidiServer.mmcStop();
            break;
          case MMC_PLAY:
            BLEMidiServer.mmcPlay();
            break;
          case MMC_DEFERRED_PLAY:
            BLEMidiServer.mmcDeferredPlay();
            break;
          case MMC_FAST_FORWARD:
            BLEMidiServer.mmcFastForward();
            break;
          case MMC_REWIND:
            BLEMidiServer.mmcRewind();
            break;
          case MMC_RECORD_STROBE:
            BLEMidiServer.mmcRecordStrobe();
            break;
          case MMC_RECORD_EXIT:
            BLEMidiServer.mmcRecordExit();
            break;
          case MMC_RECORD_PAUSE:
            BLEMidiServer.mmcRecordPause();
            break;
          case MMC_PAUSE:
            BLEMidiServer.mmcPause();
            break;
          default:
            break;
          }
          myBtn->btnState[active_mapper] = BTN_ON;
        }
        else if(btnMidiFunction == MIDI_PROGRAMCHANGE && !needRelease) return; // need implementation

        myWS28XXLED[0] = tmpBtncolor;
        FastLED.show();
        break;
      case AceButton::kEventReleased:
        log_i("handleEvent(): BTN: %d Released", pin);
        log_d("BTN: %d Released, Map:%d\n ", pin, __active_map);
        if(btnMidiFunction == MIDI_NOTE){ // Note on need short press event
          if(btnFunction == BTN_PUSH){ // Push Button
            BLEMidiServer.noteOff(btnMidiChannel, btnMidiNote, 0 ); // Note off
            myBtn->btnState[active_mapper] = BTN_OFF;
          }
        }
        else if(btnMidiFunction == MIDI_CC && needRelease){ // CC on need short press event
          BLEMidiServer.controlChange(btnMidiChannel, btnMidiCC, btnMidiCCValueStateOn);
          myBtn->btnState[active_mapper] = BTN_OFF;
        }
        else if(btnMidiFunction == MIDI_MMC && needRelease){
          switch (btnMidiMMC)
          {
          case MMC_STOP:
            BLEMidiServer.mmcStop();
            break;
          case MMC_PLAY:
            BLEMidiServer.mmcPlay();
            break;
          case MMC_DEFERRED_PLAY:
            BLEMidiServer.mmcDeferredPlay();
            break;
          case MMC_FAST_FORWARD:
            BLEMidiServer.mmcFastForward();
            break;
          case MMC_REWIND:
            BLEMidiServer.mmcRewind();
            break;
          case MMC_RECORD_STROBE:
            BLEMidiServer.mmcRecordStrobe();
            break;
          case MMC_RECORD_EXIT:
            BLEMidiServer.mmcRecordExit();
            break;
          case MMC_RECORD_PAUSE:
            BLEMidiServer.mmcRecordPause();
            break;
          case MMC_PAUSE:
            BLEMidiServer.mmcPause();
            break;
          default:
            break;
          }
          myBtn->btnState[active_mapper] = BTN_OFF;
        }
        else if(btnMidiFunction == MIDI_PROGRAMCHANGE && needRelease) return; // need implementation
        myWS28XXLED[0] = __oldLedColor;
        FastLED.show();
        break;
      case AceButton::kEventDoubleClicked:
        log_i("handleEvent(): BTN: %d DoubleClicked", pin);
        log_d("BTN: %d DoubleClicked, Map:%d\n ", pin, __active_map);
        break;
      case AceButton::kEventLongPressed:
        // Button 2 is used to change the active map
        // Switch between 2 maps. Map 1 and Map 2 or Map 3 and Map 4 and so on.
        // This is only an Quick access to change the active map via long button press
        // To change the active map to higer or lower maps we use Web ui or midi input commands
        // for example midi program change. the value of program change is the active map
        if(pin == 11) {
          if (__active_map %2 == 0 && __isConnected) {
            __active_map = __active_map + 1;
          } else {
            __active_map = __active_map -1;
            if(__active_map > NUBER_OF_MAPS) __active_map = 0; // this prevent uint8_t overflow from 0 to 255
          }
          saveActiveMap();
          updateUiActiveMap();

          return;
        }
        myWS28XXLED[0] = tmpBtncolor;
        FastLED.show();
        log_i("handleEvent(): BTN: %d LongPressed", pin);
        log_d("BTN: %d LongPressed, Map:%d\n ", pin, __active_map);
        if(btnLongpress){
          if(btnMidiFunction == MIDI_NOTE) // Note on need short press event
            BLEMidiServer.noteOff(btnMidiChannel, btnMidiNote, 0 );
          else if(btnMidiFunction == MIDI_CC && !needRelease) // CC on need short press event
            BLEMidiServer.controlChange(btnMidiChannel, btnMidiCC, btnMidiCCValueStateOn);
          else if(btnMidiFunction == MIDI_MMC && !needRelease) return; // need implementation
          else if(btnMidiFunction == MIDI_PROGRAMCHANGE && !needRelease) return; // need implementation
        }

        break;
      case AceButton::kEventLongReleased:
        // we can disable the note every time the button is logpress released even MidiFuction is not Note
        BLEMidiServer.noteOff(btnMidiChannel, btnMidiNote, 0 );
        
        log_i("handleEvent(): BTN: %d LongReleased", pin);
        log_d("BTN: %d LongReleased, Map:%d\n ", pin, __active_map);
        myWS28XXLED[0] = __oldLedColor;
        FastLED.show();
        break;
      default:
        break;
    }
}


/**
 * @brief connected callback
 *
 */
void connected() {
  // device is BLE MIDI connected
  log_i("Connected");
  __isConnected = true;
  if (__active_map % 2 == 0) {
    myWS28XXLED[0] = CRGB::Green;
    FastLED.show();
    __oldLedColor = CRGB::Green;
  } else
  {
    myWS28XXLED[0] = CRGB::Purple;
    __oldLedColor = CRGB::Purple;
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
  __isConnected = false;
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();
  __oldLedColor = CRGB::Red;

}

/**
 * @brief onProgramChange callback
 * 
 * @param channel 
 * @param program 
 * @param timestamp 
 */
void onProgramChange(uint8_t channel, uint8_t program, uint16_t timestamp){
  // program change received
  Serial.printf("Program Change: Channel: %d, Program: %d, Timestamp: %d\n", channel, program, timestamp);
  if(program > NUBER_OF_MAPS){
    program = NUBER_OF_MAPS - 1;
  }

  __active_map = program;
  saveActiveMap();
  updateUiActiveMap();
}

void blinkActiveMaps(){
  
  bool numblinkfirst = false;
  float blikBrightness = float(__BRIGHTNESS);
  if( millis() - __oldBlinktime > 5000){
    //Serial.printf("kk, %d %d %d \n", millis(), __numBlincs, __oldBlinktime);
    if(__numBlincs > 0){

      if(__numBlincs %2 == 0){
        blikBrightness = float(__BRIGHTNESS / 3.0f);
        if(__BRIGHTNESS < 2) blikBrightness = 0;
      }

      numblinkfirst = true;
      if(millis() - __oldNumBlinktime > 200){
        __numBlincs--;
        //Serial.printf("Blinki : %u\n", __numBlincs);
        FastLED.setBrightness(blikBrightness);
        FastLED.show();
        __oldNumBlinktime = millis();
      }

    }
    else
    {
      numblinkfirst = false;
    }
    
    if(!numblinkfirst){
        __oldBlinktime = millis();
        __numBlincs = (__active_map + 1) * 2;
    }
  }
}

void setup() {


  // initialize WS28xx LED in GRB order
  FastLED.addLeds<WS2812B, WS28XX_LED_PIN, GRB>(myWS28XXLED, NUM_LEDS);
  FastLED.setBrightness(6);
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();
  __oldLedColor = CRGB::Red;
    
  Serial.begin(57600);
  int timoutcounter = 0;
  while (!Serial) {
  delay(1000);
    timoutcounter++;
    if (timoutcounter > 1) {
      break;
    }
  }


// Set log level
// test serveral log levels without success something is broken in the library
// there is no output on the serial monitor with all log levels.
 //esp_log_level_set("*", ESP_LOG_INFO);
  log_i("Starting up with Loglevel Info");

  // Reset only Midi Settings
  if(digitalRead(10) == LOW && digitalRead(11) == LOW) {
    Serial.println("Reset Midi settings!");
    prefs.begin("Settings");  //Open namespace Settings
    log_d("Reset settings!");
    
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
    
    prefs.end(); // close the Settings Namespace
  }

  // reset all the settings
  if(digitalRead(10) == LOW && digitalRead(12) == LOW) {
    //reset settings
    prefs.begin("Settings");  //Open namespace Settings
    log_d("Reset settings!");
    
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
    
    prefs.end(); // close the Settings Namespace
    

    prefs.begin("blename", false); // Open NVS namespace "blename" in RW mode
    log_d("Reset blename!");
    
    prefs.putString("blename", midiDeviceName); // Save the default name
    
    prefs.end(); // Close NVS namespace "blename"
    

    prefs.begin("wifi", false); // Open NVS namespace "wifi" in RW mode
    log_d("Reset wifi!");
    
    prefs.putString("ssid_local", ssid); // Save the default name
    
    prefs.putString("password_local", password); // Save the default name
    
    prefs.putString("ssid_ap", ap_ssid); // Save the default name
    
    prefs.putString("password_ap", ap_password); // Save the default name
    
    prefs.putString("hostname", hostname); // Save the default name

    prefs.putUInt("LedBrightness", __BRIGHTNESS); // Save the default name
    
    prefs.end(); // Close NVS namespace "wifi"
    
  }

  prefs.begin("updateerrorcode");  //Open namespace Settings
  if (not prefs.isKey("updateerrorcode")) {
    Serial.println("do updateerrorcode not found, saving updateerrorcode");
    prefs.putInt("updateerrorcode", __UPDATE_ERROR_CODE);
  } else {
    log_d("updateerrorcode found, loading");
    __UPDATE_ERROR_CODE = prefs.getInt("updateerrorcode");
    Serial.printf("Fail Errorcode: %d\n", __UPDATE_ERROR_CODE);
  }
  prefs.end(); // close the Settings Namespace 


  prefs.begin("updatefail");  //Open namespace Settings
  if (not prefs.isKey("updatefail")) {
    Serial.println("do updatefail not found, saving updatefail");
    prefs.putBool("updatefail", __UPDATE_FAILURE);
  } else {
    log_d("update fail found, loading");
    __UPDATE_FAILURE = prefs.getBool("updatefail");
    Serial.printf("Fail Update: %d\n", __UPDATE_FAILURE);
  }
  prefs.end(); // close the Settings Namespace 

  prefs.begin("doupdate");  //Open namespace Settings
  if (not prefs.isKey("doupdate")) {
    Serial.println("do update not found, saving doupdate");
    prefs.putBool("doupdate", __DO_UPDATE);
  } else {
    log_d("do update found, loading");
    __DO_UPDATE = prefs.getBool("doupdate");
    Serial.printf("Do Update: %d\n", __DO_UPDATE);
  }
  prefs.end(); // close the Settings Namespace 

  prefs.begin("fwversion");  //Open namespace Settings
  //prefs.putUInt("fwversion", 1);// uncomment to reset internel firmware version
  if (not prefs.isKey("fwversion")) {
    Serial.println("firmware version not found, saving current version nr.");
    prefs.putUInt("fwversion", __FW_VERSION);
  } else {
    log_d("firmware version found, loading version number");
    __FW_VERSION = prefs.getUInt("fwversion");
    Serial.printf("FW Version: %d\n", __FW_VERSION);
  }
  prefs.end(); // close the Settings Namespace

  prefs.begin("Settings");  //Open namespace Settings
 
  if (not prefs.isKey("Settings")) {
    log_d("Settings not found, saving default settings");
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
  } else {
    log_d("Settings found, loading settings");
    prefs.getBytes("Settings", &myBtnMap, sizeof(myBtnMap));
  }
  
  prefs.end(); // close the Settings Namespace

  prefs.begin("active_map");  //Open namespace Settings
  if (not prefs.isKey("active_map")) {
    Serial.println("active_map not found, saving default active_map");
    prefs.putUInt("active_map", __active_map);
  } else {
    log_d("active_map found, loading map");
    __active_map = prefs.getUInt("active_map");
    Serial.printf("active_map: %d\n", __active_map);
  }
  prefs.end(); // close the Settings Namespace
  

  prefs.begin("blename", false); // Open NVS namespace "blename" in RW mode
  
  if (not prefs.isKey("blename")) {
    log_d("blename not found");
    prefs.putString("blename", midiDeviceName); // Save the default name
  } else {
    midiDeviceName = prefs.getString("blename"); // 
    log_d("blename found, loading settings: value: %S\n", midiDeviceName.c_str());
  }
  
  prefs.end(); // Close NVS namespace "blename"
  

  prefs.begin("wifi", false); // Open NVS namespace "wifi" in RW mode
  if (not prefs.isKey("ssid_local")) {
    log_d("ssid_local not found");
    prefs.putString("ssid_local", ssid); // Save the default name
  } else {
    ssid = prefs.getString("ssid_local"); // 
    log_d("ssid_local found, loading settings: value: %S\n", ssid.c_str());
  }
  
  if (not prefs.isKey("password_local")) {
    log_d("password_local not found");
    prefs.putString("password_local", password); // Save the default name
  } else {
    password = prefs.getString("password_local"); // 
    log_d("password_local found, loading settings: value: %S\n", password.c_str());
  }
  
  if (not prefs.isKey("ssid_ap")) {
    log_d("ssid_ap not found");
    prefs.putString("ssid_ap", ap_ssid); // Save the default name
  } else {
    ap_ssid = prefs.getString("ssid_ap"); // 
    log_d("ssid_ap found, loading settings: value: %S\n", ap_ssid.c_str());
  }
  
  if (not prefs.isKey("password_ap")) {
    log_d("password_ap not found");
    prefs.putString("password_ap", ap_password); // Save the default name
  } else {
    ap_password = prefs.getString("password_ap"); // 
    log_d("password_ap found, loading settings: value: %S\n", ap_password.c_str());
  }
  
  if (not prefs.isKey("hostname")) {
    log_d("hostname not found");
    prefs.putString("hostname", hostname); // Save the default name
  } else {
    hostname = prefs.getString("hostname"); // 
    log_d("hostname found, loading settings: value: %S\n", hostname.c_str());
  }

  if (not prefs.isKey("LedBrightness")) {
    log_d("LedBrightness not found");
    prefs.putUInt("LedBrightness", __BRIGHTNESS); // Save the default name
  } else {
    Serial.printf("LedBrightness found: %d\n", prefs.getUInt("LedBrightness"));
    __BRIGHTNESS = prefs.getUInt("LedBrightness"); // 
    log_d("LedBrightness found, loading settings: value: %d\n", __BRIGHTNESS);
  } 
  
  prefs.end(); // Close NVS namespace "wifi"
  
  FastLED.setBrightness(__BRIGHTNESS);
  FastLED.show();

  log_d("Testdate from settings: %d \n", myBtnMap[4].btnMidiCC[3]);



  // Button uses the built-in pull up register.
  // The button is connected to GPIO 0.
  pinMode(myBtnMap[0].btnGpio, INPUT);
  pinMode(myBtnMap[1].btnGpio, INPUT);
  pinMode(myBtnMap[2].btnGpio, INPUT);
  pinMode(myBtnMap[3].btnGpio, INPUT);
  pinMode(myBtnMap[4].btnGpio, INPUT);



   // Configure the ButtonConfig with the event handler.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setLongPressDelay(1500);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress); 

  log_d("warte 0.1s");
  delay(100);
  //----------------------------------------------------------------
  if((digitalRead(13) == LOW && digitalRead(14) == LOW) || __DO_UPDATE) {
    myWS28XXLED[0] = CRGB::Blue;
    if(__DO_UPDATE) myWS28XXLED[0] = CRGB::Yellow;
    FastLED.show();
    log_d("Start Wifi");
    if(!__DO_UPDATE){
      __configurator = true;
      ESPUI.setVerbosity(Verbosity::VerboseJSON);
    }
    else
    {
      __configurator = false;
    }

    WiFi.setHostname(hostname.c_str());

    if(digitalRead(12) == HIGH || __DO_UPDATE) {
      // try to connect to existing network
      WiFi.begin(ssid.c_str(), password.c_str());
      log_d("\n\nTry to connect to existing network");
    }

    {
        uint8_t timeout = 10;

        // Wait for connection, 5s timeout
        do
        {
            delay(500);
            log_d(".");
            timeout--;
        } while (timeout && WiFi.status() != WL_CONNECTED);

        Serial.printf("Local Ip Address: %s\n", WiFi.localIP().toString().c_str());

        if(WiFi.status() != WL_CONNECTED && __DO_UPDATE){
          Serial.println("Configure Local network first!");
        }
        // not connected -> create hotspot
        if (WiFi.status() != WL_CONNECTED && !__DO_UPDATE)
        {
            log_d("\n\nCreating hotspot");

            WiFi.mode(WIFI_AP);
            delay(100);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

            char local_ap_ssid[25];
            char local_ap_password[30];
            snprintf(local_ap_ssid, 26, "%S-%08X", ap_ssid.c_str(), ESP.getEfuseMac());
            snprintf(local_ap_password, 31, "%S", ap_password.c_str() );
            WiFi.softAP(local_ap_ssid, local_ap_password);

            timeout = 5;

            do
            {
                delay(500);
                log_d(".");
                timeout--;
            } while (timeout);
        }
    }

    if(!__DO_UPDATE){

      dnsServer.start(DNS_PORT, "LittleHelper", apIP);

      log_d("\n\nWiFi parameters:");
      log_d("Mode: ");
      log_d("%S\n", WiFi.getMode() == WIFI_AP ? "Station" : "Client");
      log_d("IP address: ");
      log_d("%S\n", WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());

      ESPUI.setVerbosity(Verbosity::Quiet);

      uint16_t tab1 = ESPUI.addControl(ControlType::Tab, "Button 1", "Button 1");
      uint16_t tab2 = ESPUI.addControl(ControlType::Tab, "Button 2", "Button 2");
      uint16_t tab3 = ESPUI.addControl(ControlType::Tab, "Button 3", "Button 3");
      uint16_t tab4 = ESPUI.addControl(ControlType::Tab, "Button 4", "Button 4");
      uint16_t tab5 = ESPUI.addControl(ControlType::Tab, "Button 5", "Button 5");
      uint16_t tab6 = ESPUI.addControl(ControlType::Tab, "Active Map", "Active Map");
      uint16_t tab7 = ESPUI.addControl(ControlType::Tab, "Settings", "Settings");

      // Active Map Chooser
      char activeMapString[10];
      sprintf(activeMapString, "%d", __active_map); // Convert the number to a string
      activeMapChooser = ESPUI.addControl(ControlType::Select, "Active Map:", activeMapString, ControlColor::Emerald, tab6, &selectActiveMap);
      ESPUI.addControl(ControlType::Option, "Map 1", "0", ControlColor::Dark, activeMapChooser);
      ESPUI.addControl(ControlType::Option, "Map 2", "1", ControlColor::Dark, activeMapChooser);
      ESPUI.addControl(ControlType::Option, "Map 3", "2", ControlColor::Dark, activeMapChooser);
      ESPUI.addControl(ControlType::Option, "Map 4", "3", ControlColor::Dark, activeMapChooser);
      

      // Wlan Settings and Bluethooth Settings

      bleNameTxtField = ESPUI.addControl(ControlType::Text, "Bluethooth Name:", midiDeviceName.c_str(), ControlColor::Dark, tab7, &textCallBlueThoothName);
      wlanSsidNameTxtField = ESPUI.addControl(ControlType::Text, "Wlan SSID:", ssid.c_str(), ControlColor::Dark, tab7, &textCallSsidName);
      wlanPasswordTxtField = ESPUI.addControl(ControlType::Text, "Wlan Password:", password.c_str(), ControlColor::Dark, tab7, &textCallWlanPassword);
      ESPUI.setInputType(wlanPasswordTxtField, "password");
      wlanApSsidTxtField = ESPUI.addControl(ControlType::Text, "Access Point SSID:", ap_ssid.c_str(), ControlColor::Dark, tab7, &textCallAPSsidName);
      wlanApPasswordTxtField = ESPUI.addControl(ControlType::Text, "Access Point Password:", ap_password.c_str(), ControlColor::Dark, tab7, &textCallAPPassword);
      ESPUI.setInputType(wlanApPasswordTxtField, "password");
      hostnameTxtField = ESPUI.addControl(ControlType::Text, "Hostname:", hostname.c_str(), ControlColor::Dark, tab7, &textCallHostname);
      ESPUI.addControl(ControlType::Switcher, "Show Passwords", "", ControlColor::Alizarin, tab7, &switchShowPasswords);

      // OTA Update
      #ifdef USE_OTA
      char fwv[3];
      sprintf(fwv, "s3miniV%d", __FW_VERSION);
      ESPUI.addControl(ControlType::Label, "Current Firmware", fwv, ControlColor::Alizarin, tab7, &nothing);

      char fwupdatestr[10];
      if(__UPDATE_FAILURE && __UPDATE_ERROR_CODE == 404){
        sprintf(fwupdatestr, "No Update foud"); // Convert the number to a string
      }
      else if(__UPDATE_FAILURE) sprintf(fwupdatestr, "Update Fail, retry"); // Convert the number to a string
      else sprintf(fwupdatestr, "Try Update"); // Convert the number to a string
      ESPUI.addControl(ControlType::Button, "Firmware Update", fwupdatestr, ControlColor::Alizarin, tab7, &otaUpdate);
      #endif

      // Led Brightness
      ledBrightnessTxtField = ESPUI.addControl(ControlType::Slider, "LED Brightness:", String(__BRIGHTNESS).c_str(), ControlColor::Dark, tab7, &textCallLedBrightness);
      ESPUI.addControl(Min, "", "0", None, ledBrightnessTxtField);
      ESPUI.addControl(Max, "", "255", None, ledBrightnessTxtField);

      // Buttons in a for loop
      
      for (size_t hw_B = 0; hw_B < __HW_BUTTONS; hw_B++) // HW Buttons * Ui Button Functions
      {
        uint16_t thistab = 0;
        switch ( hw_B )
        {
          case 0:
            thistab = tab1;
            break;
          case 1:
            thistab = tab2;
            break;
          case 2:
            thistab = tab3;
            break;
          case 3:
            thistab = tab4;
            break;
          case 4:
            thistab = tab5;
            break;
          default:
            break;
        }
        //HW Button 1
        // __selectUiBtn[5][8]
        // [5] = HW Button 1 -5
        // [8] = Ui Button 1 - 8
        __selectUiBtn[hw_B][0] = ESPUI.addControl(ControlType::Select, "Select Map:", "", ControlColor::Emerald, thistab, &selectBtnMapFnc);
        ESPUI.addControl(ControlType::Option, "Map 1", "0", ControlColor::Dark, __selectUiBtn[hw_B][0]);
        ESPUI.addControl(ControlType::Option, "Map 2", "1", ControlColor::Dark, __selectUiBtn[hw_B][0]);
        ESPUI.addControl(ControlType::Option, "Map 3", "2", ControlColor::Dark, __selectUiBtn[hw_B][0]);
        ESPUI.addControl(ControlType::Option, "Map 4", "3", ControlColor::Dark, __selectUiBtn[hw_B][0]);

        char convertstr[10];
        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiChannel[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][1] = ESPUI.addControl(ControlType::Number, "Midi Channel 0 - 15:", convertstr, ControlColor::Dark, thistab, &selectBtnMidiChannelCalback);
        ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][1]);
        ESPUI.addControl(Max, "", "15", None, __selectUiBtn[hw_B][1]);

        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiFunction[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][2] = ESPUI.addControl(ControlType::Select, "Midi Function:", convertstr, ControlColor::Dark, thistab, &selectBtnMidiFnc);
        // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
        ESPUI.addControl(ControlType::Option, "Note", "0", ControlColor::Dark, __selectUiBtn[hw_B][2]);
        ESPUI.addControl(ControlType::Option, "CC", "1", ControlColor::Dark, __selectUiBtn[hw_B][2]);
        ESPUI.addControl(ControlType::Option, "MMC", "2", ControlColor::Dark, __selectUiBtn[hw_B][2]);
        ESPUI.addControl(ControlType::Option, "PC", "3", ControlColor::Dark, __selectUiBtn[hw_B][2]);

        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiCC[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][3] = ESPUI.addControl(ControlType::Number, "Midi CC 0 - 127:", convertstr, ControlColor::Dark, thistab, &selectBtnMidiCCFunctionCalback);
        ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][3]);
        ESPUI.addControl(Max, "", "127", None, __selectUiBtn[hw_B][3]);

        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiCCValueStateOn[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][4] = ESPUI.addControl(ControlType::Number, "Midi CC Value On 0 - 127:", convertstr, ControlColor::Dark, thistab, &selectBtnCCValueMaxCalback);
        ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][4]);
        ESPUI.addControl(Max, "", "127", None, __selectUiBtn[hw_B][4]);

        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiCCValueStateOff[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][5] = ESPUI.addControl(ControlType::Number, "Midi CC Value Off 0 - 127:", convertstr, ControlColor::Dark, thistab, &selectBtnCCValueMinCalback);
        ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][5]);
        ESPUI.addControl(Max, "", "127", None, __selectUiBtn[hw_B][5]);

        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiNote[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][6] = ESPUI.addControl(ControlType::Number, "Midi Note 0 - 127:", convertstr, ControlColor::Dark, thistab, &selectBtnMidiNoteCalback);
        ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][6]);
        ESPUI.addControl(Max, "", "127", None, __selectUiBtn[hw_B][6]);

        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiMMC[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][7] = ESPUI.addControl(ControlType::Select, "MMC Function:", convertstr, ControlColor::Dark, thistab, &selectBtnMMCFnc);
        ESPUI.addControl(ControlType::Option, "STOP", "1", ControlColor::Dark, __selectUiBtn[hw_B][7]);
        ESPUI.addControl(ControlType::Option, "PLAY", "2", ControlColor::Dark, __selectUiBtn[hw_B][7]);
        ESPUI.addControl(ControlType::Option, "DEFERRED PLAY", "3", ControlColor::Dark, __selectUiBtn[hw_B][7]);
        ESPUI.addControl(ControlType::Option, "FAST FORWARD", "4", ControlColor::Dark, __selectUiBtn[hw_B][7]);      
        ESPUI.addControl(ControlType::Option, "REWIND", "5", ControlColor::Dark, __selectUiBtn[hw_B][7]);
        ESPUI.addControl(ControlType::Option, "RECORD STROBE", "6", ControlColor::Dark, __selectUiBtn[hw_B][7]);
        ESPUI.addControl(ControlType::Option, "RECORD EXIT", "7", ControlColor::Dark, __selectUiBtn[hw_B][7]);
        ESPUI.addControl(ControlType::Option, "RECORD PAUSE", "8", ControlColor::Dark, __selectUiBtn[hw_B][7]);
        ESPUI.addControl(ControlType::Option, "PAUSE", "9", ControlColor::Dark, __selectUiBtn[hw_B][7]);      

        sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiVelocity[__active_map_ui_btn[hw_B]]); // Convert the number to a string
        __selectUiBtn[hw_B][8] = ESPUI.addControl(ControlType::Number, "Midi Note Velocity 0 - 127:", convertstr, ControlColor::Dark, thistab, &selectBtnNoteVelocityCalback);
        ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][8]);
        ESPUI.addControl(Max, "", "127", None, __selectUiBtn[hw_B][8]);

        __selectUiBtn[hw_B][9] = ESPUI.addControl(ControlType::Select, "Button behave: Midi Note only", "", ControlColor::Dark, thistab, &selectBtnBehaveFncCalback);
        ESPUI.addControl(ControlType::Option, "Push", "0", ControlColor::Dark, __selectUiBtn[hw_B][9]);
        ESPUI.addControl(ControlType::Option, "Toggle", "1", ControlColor::Dark, __selectUiBtn[hw_B][9]);

        __selectUiBtn[hw_B][10] = ESPUI.addControl(ControlType::Select, "Button Transition: Midi Note excluded", "", ControlColor::Dark, thistab, &selectBtnTransitinCalback);
        ESPUI.addControl(ControlType::Option, "Push", "0", ControlColor::Dark, __selectUiBtn[hw_B][10]);
        ESPUI.addControl(ControlType::Option, "Release", "1", ControlColor::Dark, __selectUiBtn[hw_B][10]);

        uint32_t color = myBtnMap[hw_B].btnColor[__active_map_ui_btn[hw_B]];
        int colorval = 0;
        for(int i = 0; i < 141; i++) {
          if(__btnLookUpTable[i] == color) {
            colorval = i;
            break;
          }
        }
        sprintf(convertstr, "%d", colorval); // Convert the number to a string  
        __selectUiBtn[hw_B][11] = ESPUI.addControl(ControlType::Slider, "Button Color:", convertstr, ControlColor::Dark, thistab, &selectBtnColorCalback);
        ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][11]);
        ESPUI.addControl(Max, "", "139", None, __selectUiBtn[hw_B][11]);
        static char stylecol1[60];
        sprintf(stylecol1, "border-bottom: #999 3px solid; background-color: #%06X;", color );   
        ESPUI.setPanelStyle(__selectUiBtn[hw_B][11], stylecol1);
      
      }
      
      ESPUI.begin("Little Helper Web UI");
    }
  }

  __numBlincs = (__active_map + 1) * 2;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  log_i("Starting BLE MIDI server");
  BLEMidiServer.begin(midiDeviceName.c_str());
  //BLEMidiServer.enableDebugging();
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback(disconected);
  // BLEMidiServer.setNoteOnCallback(onNoteOn);
  // BLEMidiServer.setNoteOffCallback(onNoteOff);
  // BLEMidiServer.setControlChangeCallback(onControlChange);
  BLEMidiServer.setProgramChangeCallback(onProgramChange);

}

void loop() {

  // Call the button service routine every loop.
  btn1.check();
  btn2.check();
  btn3.check();
  btn4.check();
  btn5.check();

  static long oldTime = 0;
  if(__configurator) {
    if (millis() - oldTime > 50){ 
        dnsServer.processNextRequest();
        oldTime = millis();
    }
  }

  if(__DO_UPDATE) justotaUpdate();

  blinkActiveMaps();
  delay(1);
}



