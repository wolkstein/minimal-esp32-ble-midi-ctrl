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

#include <DNSServer.h>
#include <ESPUI.h>

#include <WiFi.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 8, 1);
DNSServer dnsServer;

Preferences prefs;

using namespace ace_button;

// LED Strucutre
CRGB myWS28XXLED[NUM_LEDS];

CRGB __oldLedColor;

myButton myBtnMap[5] = { // 5 Buttons 4 Maps Map 1 und Map 2 are short press values, Map 3 and Map 4 are long press values
    { // Button 1
         10,  // GPIO Pin
         {false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false}, // Button Long Press
         {BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_RED, BTN_RED}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
         {MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
         {MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
         {60, 60}, // Button MIDI Note 0 - 127
         {100, 100}, // Button MIDI Velocity 0 - 127
         {43, 111}, // Button MIDI CC 0 - 127
         {127, 127}, // Button MIDI CC ON Value 0 - 127
         {0, 0}, // Button MIDI CC OFF Value 0 - 127
         {MMC_REWIND, MMC_REWIND} // Button MIDI MMC 0 - 13
    },
    { // Button 2
         11,  // GPIO Pin
         {false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false}, // Button Long Press
         {BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_YELLOW, BTN_YELLOW}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
         {MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
         {MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
         {61, 61}, // Button MIDI Note 0 - 127
         {100, 100}, // Button MIDI Velocity 0 - 127
         {42, 42}, // Button MIDI CC 0 - 127
         {127, 127}, // Button MIDI CC ON Value 0 - 127
         {0, 0}, // Button MIDI CC OFF Value 0 - 127
         {MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
    },
    { // Button 3
         12,  // GPIO Pin
         {false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false}, // Button Long Press
         {BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_CYAN, BTN_CYAN}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
         {MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
         {MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
         {62, 62}, // Button MIDI Note 0 - 127
         {100, 100}, // Button MIDI Velocity 0 - 127
         {44, 112}, // Button MIDI CC 0 - 127
         {127, 127}, // Button MIDI CC ON Value 0 - 127
         {0, 0}, // Button MIDI CC OFF Value 0 - 127
         {MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
    },
    { // Button 4
         13,  // GPIO Pin
         {true, true}, // Button Release
         {BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, true}, // Button Long Press
         {BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_WHITE, BTN_WHITE}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
         {MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
         {MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
         {62, 62}, // Button MIDI Note 0 - 127
         {100, 100}, // Button MIDI Velocity 0 - 127
         {45, 64}, // Button MIDI CC 0 - 127
         {127, 127}, // Button MIDI CC ON Value 0 - 127
         {0, 0}, // Button MIDI CC OFF Value 0 - 127
         {MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
    },
    { // Button 5
         14,  // GPIO Pin
         {false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false}, // Button Long Press
         {BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_BLUE, BTN_BLUE}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
         {MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
         {MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
         {62, 62}, // Button MIDI Note 0 - 127
         {100, 100}, // Button MIDI Velocity 0 - 127
         {41, 41}, // Button MIDI CC 0 - 127
         {127, 127}, // Button MIDI CC ON Value 0 - 127
         {0, 0}, // Button MIDI CC OFF Value 0 - 127
         {MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
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



// WEB UI Callbacks

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
    prefs.begin("bluetooth", false); // Open NVS namespace "bluetooth" in RW mode
    
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

void saveSettings() {
    prefs.begin("Settings"); // Open NVS namespace "Settings" in RW mode
    
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
    
    prefs.end();
    
}

// Buttons dynamic callback test function
void buttonCallbackDynamicMap(Control* sender, int type) {
    Serial.printf("Button Callback: ID: %d, Value: %s\n", sender->id, sender->value);
    Serial.println("Dynamic MAp Function------------------------------------------------->");
}

void buttonCallbackDynamicAction(Control* sender, int type) {
    Serial.printf("Button Callback: ID: %d, Value: %s\n", sender->id, sender->value);
    Serial.println("Dynamic Action Function------------------------------------------------->");
}

// Button 1 Web UI Callbacks ---------
void selectBtnMapFnc(Control* sender, int value) {

    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    __active_map_ui_btn[0] = value_t;


    //update the value in the settings
    char str[10]; // Ensure this is large enough to hold the number and the null terminator
    uint8_t localvalue = myBtnMap[0].btnMidiFunction[__active_map_ui_btn[0]]; // get the MidiFunction value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1MidiFunction, str); // Update the control value

    localvalue = myBtnMap[0].btnMidiChannel[__active_map_ui_btn[0]]; // Get the MidiChannel value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1MidiChannel, str); // Update the control value

    localvalue = myBtnMap[0].btnMidiCC[__active_map_ui_btn[0]]; // Get the MidiCC value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1CCFunction, str); // Update the control value

    localvalue = myBtnMap[0].btnMidiCCValueStateOn[__active_map_ui_btn[0]]; // Get the MidiCCValueStateOn value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1CCValueMax, str); // Update the control value

    localvalue = myBtnMap[0].btnMidiCCValueStateOff[__active_map_ui_btn[0]]; // Get the MidiCCValueStateOff value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1CCValueMin, str); // Update the control value

    localvalue = myBtnMap[0].btnMidiNote[__active_map_ui_btn[0]]; // Get the MidiNote value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1MidiNote, str); // Update the control value

    localvalue = myBtnMap[0].btnMidiVelocity[__active_map_ui_btn[0]]; // Get the MidiVelocity value from the settings
    sprintf(str, "%d", localvalue); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1NoteVelocity, str); // Update the control value

}

void selectBtnMidiFnc(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[0].btnMidiFunction[__active_map_ui_btn[0]] = value_t;
    saveSettings();

}

void selectBtnMidiChannelCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[0].btnMidiChannel[__active_map_ui_btn[0]] = value_t;
    saveSettings();

}

void selectBtnMidiCCFunctionCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[0].btnMidiCC[__active_map_ui_btn[0]] = value_t;
    saveSettings();
}

void selectBtnCCValueMaxCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[0].btnMidiCCValueStateOn[__active_map_ui_btn[0]] = value_t;
    saveSettings();
}

void selectBtnCCValueMinCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[0].btnMidiCCValueStateOff[__active_map_ui_btn[0]] = value_t;
    saveSettings();
}

void selectBtnMidiNoteCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[0].btnMidiNote[__active_map_ui_btn[0]] = value_t;
    saveSettings();
}

void selectBtnNoteVelocityCalback(Control* sender, int value) {
    
    uint8_t value_t = static_cast<uint8_t>(String(sender->value).toInt());

    Serial.printf("Select: ID: %d, Value: %s, Value as int %d\n", sender->id, sender->value, value_t);
    myBtnMap[0].btnMidiVelocity[__active_map_ui_btn[0]] = value_t;
    saveSettings();
}
// Button 1 Web UI Callbacks end ---------





// ~ WEB UI Callbacks


// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /*buttonState*/) { 
    
    uint8_t pin = button->getPin();

    myButton* myBtn = getMyButton(pin);
    if (myBtn != nullptr) {
        Serial.printf("Button %d found\n", pin);
    } else {
        Serial.printf("Button %d not found\n", pin);
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
    Serial.printf("BTN: %d, Map:%d, NeedRelease:%d, Test valid Gpio %d \n", pin, active_mapper, needRelease, myBtn->btnGpio);
    uint8_t btnFunction = myBtn->btnFunction[active_mapper]; // 0 = Push, 1 = Toggle
    bool btnLongpress = myBtn->btnLongpress[active_mapper]; // 0 = Short Press, 1 = Long Press
    uint8_t btnState = myBtn->btnState[active_mapper]; // 0 = Off, 1 = On
    uint8_t btnColor = myBtn->btnColor[active_mapper]; // 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
    uint8_t btnMidiFunction = myBtn->btnMidiFunction[active_mapper]; // 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
    uint8_t btnMidiChannel = myBtn->btnMidiChannel[active_mapper]; // 0 - 15  MIDI Channel
    uint8_t btnMidiNote = myBtn->btnMidiNote[active_mapper]; // 0 - 127 MIDI Note
    uint8_t btnMidiVelocity = myBtn->btnMidiVelocity[active_mapper]; // 0 - 127 MIDI Velocity
    uint8_t btnMidiCC = myBtn->btnMidiCC[active_mapper]; // 0 - 127 MIDI CC
    uint8_t btnMidiCCValueStateOn = myBtn->btnMidiCCValueStateOn[active_mapper]; // 0 - 127 MIDI CC Value State On
    uint8_t btnMidiCCValueStateOff = myBtn->btnMidiCCValueStateOff[active_mapper]; // 0 - 127 MIDI CC Value State Off
    uint8_t btnMidiMMC = myBtn->btnMidiCCValueStateOff[active_mapper]; // 0 - 13 MIDI MMC

    CRGB tmpBtncolor = myledslookup[btnColor];

    switch (eventType) {
      case AceButton::kEventPressed:
        log_i("handleEvent(): BTN: %d Pressed", pin);
        Serial.printf("BTN: %d Pressed, Map:%d\n ", pin, __active_map);
        if(btnMidiFunction == MIDI_NOTE) // Note on need short press event
          BLEMidiServer.noteOn(btnMidiChannel, btnMidiNote, btnMidiVelocity);
        else if(btnMidiFunction == MIDI_CC && !needRelease) // CC on need short press event
          BLEMidiServer.controlChange(btnMidiChannel, btnMidiCC, btnMidiCCValueStateOn);
        else if(btnMidiFunction == MIDI_MMC && !needRelease) return; // need implementation
        else if(btnMidiFunction == MIDI_PROGRAMCHANGE && !needRelease) return; // need implementation
        myWS28XXLED[0] = tmpBtncolor;
        FastLED.show();
        break;
      case AceButton::kEventReleased:
        log_i("handleEvent(): BTN: %d Released", pin);
        Serial.printf("BTN: %d Released, Map:%d\n ", pin, __active_map);
        if(btnMidiFunction == MIDI_NOTE) // Note on need short press event
          BLEMidiServer.noteOff(btnMidiChannel, btnMidiNote, 0 );
        else if(btnMidiFunction == MIDI_CC && needRelease) // CC on need short press event
          BLEMidiServer.controlChange(btnMidiChannel, btnMidiCC, btnMidiCCValueStateOn);
        else if(btnMidiFunction == MIDI_MMC && needRelease) return; // need implementation
        else if(btnMidiFunction == MIDI_PROGRAMCHANGE && needRelease) return; // need implementation
        myWS28XXLED[0] = __oldLedColor;
        FastLED.show();
        break;
      case AceButton::kEventDoubleClicked:
        log_i("handleEvent(): BTN: %d DoubleClicked", pin);
        Serial.printf("BTN: %d DoubleClicked, Map:%d\n ", pin, __active_map);
        break;
      case AceButton::kEventLongPressed:
        // Button 2 is used to change the active map
        if(pin == 11) {
          if (__active_map == 0 && __isConnected) {
            __active_map = 1;
            myWS28XXLED[0] = CRGB::Purple;
            __oldLedColor = CRGB::Purple;
            FastLED.show();
          } else if (__active_map == 1 ) {
            __active_map = 0;
            myWS28XXLED[0] = CRGB::Green;
            __oldLedColor = CRGB::Green;
            if(!__isConnected) myWS28XXLED[0] = CRGB::Red;
            FastLED.show();
          }
          return;
        }
        myWS28XXLED[0] = tmpBtncolor;
        FastLED.show();
        log_i("handleEvent(): BTN: %d LongPressed", pin);
        Serial.printf("BTN: %d LongPressed, Map:%d\n ", pin, __active_map);
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
        log_i("handleEvent(): BTN: %d LongReleased", pin);
        Serial.printf("BTN: %d LongReleased, Map:%d\n ", pin, __active_map);
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
  if (__active_map == 0) {
    myWS28XXLED[0] = CRGB::Green;
    FastLED.show();
    __oldLedColor = CRGB::Green;
  } else if (__active_map == 1 ) {
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


void setup() {



  // initialize WS28xx LED in GRB order
  FastLED.addLeds<WS2812B, WS28XX_LED_PIN, GRB>(myWS28XXLED, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();
  __oldLedColor = CRGB::Red;
    
  Serial.begin(57600);
  int timoutcounter = 0;
   while (!Serial) {
    delay(1000);
     timoutcounter++;
     if (timoutcounter > 10) {
       break;
     }
   }
  log_i("Starting up");
  Serial.println("Starting up");

  // reset the settings
  if(digitalRead(10) == LOW && digitalRead(12) == LOW) {
    //reset settings
    prefs.begin("Settings");  //Open namespace Settings
    Serial.println("Reset settings!");
    
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
    
    prefs.end(); // close the Settings Namespace
    

    prefs.begin("blename", false); // Open NVS namespace "blename" in RW mode
    Serial.println("Reset blename!");
    
    prefs.putString("blename", midiDeviceName); // Save the default name
    
    prefs.end(); // Close NVS namespace "blename"
    

    prefs.begin("wifi", false); // Open NVS namespace "wifi" in RW mode
    Serial.println("Reset wifi!");
    
    prefs.putString("ssid_local", ssid); // Save the default name
    
    prefs.putString("password_local", password); // Save the default name
    
    prefs.putString("ssid_ap", ap_ssid); // Save the default name
    
    prefs.putString("password_ap", ap_password); // Save the default name
    
    prefs.putString("hostname", hostname); // Save the default name
    
    prefs.end(); // Close NVS namespace "wifi"
    
  }


  prefs.begin("Settings");  //Open namespace Settings

  
  if (not prefs.isKey("Settings")) {
    Serial.println("Settings not found, saving default settings");
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
  } else {
    Serial.println("Settings found, loading settings");
    prefs.getBytes("Settings", &myBtnMap, sizeof(myBtnMap));
  }
  
  prefs.end(); // close the Settings Namespace
  

  prefs.begin("blename", false); // Open NVS namespace "blename" in RW mode
  
  if (not prefs.isKey("blename")) {
    Serial.println("blename not found");
    prefs.putString("blename", midiDeviceName); // Save the default name
  } else {
    midiDeviceName = prefs.getString("blename"); // 
    Serial.printf("blename found, loading settings: value: %S\n", midiDeviceName.c_str());
  }
  
  prefs.end(); // Close NVS namespace "blename"
  

  prefs.begin("wifi", false); // Open NVS namespace "wifi" in RW mode
  if (not prefs.isKey("ssid_local")) {
    Serial.println("ssid_local not found");
    prefs.putString("ssid_local", ssid); // Save the default name
  } else {
    ssid = prefs.getString("ssid_local"); // 
    Serial.printf("ssid_local found, loading settings: value: %S\n", ssid.c_str());
  }
  
  if (not prefs.isKey("password_local")) {
    Serial.println("password_local not found");
    prefs.putString("password_local", password); // Save the default name
  } else {
    password = prefs.getString("password_local"); // 
    Serial.printf("password_local found, loading settings: value: %S\n", password.c_str());
  }
  
  if (not prefs.isKey("ssid_ap")) {
    Serial.println("ssid_ap not found");
    prefs.putString("ssid_ap", ap_ssid); // Save the default name
  } else {
    ap_ssid = prefs.getString("ssid_ap"); // 
    Serial.printf("ssid_ap found, loading settings: value: %S\n", ap_ssid.c_str());
  }
  
  if (not prefs.isKey("password_ap")) {
    Serial.println("password_ap not found");
    prefs.putString("password_ap", ap_password); // Save the default name
  } else {
    ap_password = prefs.getString("password_ap"); // 
    Serial.printf("password_ap found, loading settings: value: %S\n", ap_password.c_str());
  }
  
  if (not prefs.isKey("hostname")) {
    Serial.println("hostname not found");
    prefs.putString("hostname", hostname); // Save the default name
  } else {
    hostname = prefs.getString("hostname"); // 
    Serial.printf("hostname found, loading settings: value: %S\n", hostname.c_str());
  }
  
  prefs.end(); // Close NVS namespace "wifi"
  
  Serial.printf("Testdate from settings: %d \n", myBtnMap[4].btnMidiCC[3]);



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

  Serial.println("warte 1s");
  delay(1000);
  //----------------------------------------------------------------
  if(digitalRead(13) == LOW && digitalRead(14) == LOW) {
    myWS28XXLED[0] = CRGB::Blue;
    FastLED.show();
    Serial.println("Start in AP Mode");
    __configurator = true;

    ESPUI.setVerbosity(Verbosity::VerboseJSON);

    WiFi.setHostname(hostname.c_str());

    // try to connect to existing network
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("\n\nTry to connect to existing network");

    {
        uint8_t timeout = 10;

        // Wait for connection, 5s timeout
        do
        {
            delay(500);
            Serial.print(".");
            timeout--;
        } while (timeout && WiFi.status() != WL_CONNECTED);

        // not connected -> create hotspot
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("\n\nCreating hotspot");

            WiFi.mode(WIFI_AP);
            delay(100);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

            char local_ap_ssid[25];
            snprintf(local_ap_ssid, 26, "%S-%08X", ap_ssid.c_str(), ESP.getEfuseMac());
            WiFi.softAP(local_ap_ssid, ap_password.c_str());

            timeout = 5;

            do
            {
                delay(500);
                Serial.print(".");
                timeout--;
            } while (timeout);
        }
    }

    dnsServer.start(DNS_PORT, "LittleHelper", apIP);

    Serial.println("\n\nWiFi parameters:");
    Serial.print("Mode: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
    Serial.print("IP address: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
    uint16_t tab1 = ESPUI.addControl(ControlType::Tab, "Button 1", "Button 1");
    uint16_t tab2 = ESPUI.addControl(ControlType::Tab, "Button 2", "Button 2");
    uint16_t tab3 = ESPUI.addControl(ControlType::Tab, "Button 3", "Button 3");
    uint16_t tab4 = ESPUI.addControl(ControlType::Tab, "Button 4", "Button 4");
    uint16_t tab5 = ESPUI.addControl(ControlType::Tab, "Button 5", "Button 5");
    uint16_t tab7 = ESPUI.addControl(ControlType::Tab, "Settings", "Settings");

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

    // Buttons in a for loop
    int HW_BUTTONS = 5;
    for (size_t hw_B = 0; hw_B < HW_BUTTONS; hw_B++) // HW Buttons * Ui Button Functions
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
      //ESPUI.addControl(ControlType::Option, "MMC", "2", ControlColor::Dark, __selectUiBtn[hw_B][2]);
      //ESPUI.addControl(ControlType::Option, "PC", "3", ControlColor::Dark, __selectUiBtn[hw_B][2]);

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

      sprintf(convertstr, "%d", myBtnMap[hw_B].btnMidiVelocity[__active_map_ui_btn[hw_B]]); // Convert the number to a string
      __selectUiBtn[hw_B][7] = ESPUI.addControl(ControlType::Number, "Midi Note Velocity 0 - 127:", convertstr, ControlColor::Dark, thistab, &selectBtnNoteVelocityCalback);
      ESPUI.addControl(Min, "", "0", None, __selectUiBtn[hw_B][7]);
      ESPUI.addControl(Max, "", "127", None, __selectUiBtn[hw_B][7]);
    }


    ESPUI.begin("Little Helper Configuration");

    // set the rigth value to the select control
    // BTN 1 Values
    char str[10]; // Ensure this is large enough to hold the number and the null terminator
    uint8_t number = myBtnMap[0].btnMidiFunction[0]; // Assuming this is the uint8_t you mentioned
    sprintf(str, "%d", number); // Convert the number to a string
    ESPUI.updateControlValue(selectBtn1MidiFunction, str); // Update the control value
  }



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  log_i("Starting BLE MIDI server");
  BLEMidiServer.begin(midiDeviceName.c_str());
  //BLEMidiServer.enableDebugging();
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback(disconected);
  // BLEMidiServer.setNoteOnCallback(onNoteOn);
  // BLEMidiServer.setNoteOffCallback(onNoteOff);
  // BLEMidiServer.setControlChangeCallback(onControlChange);
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

  delay(1);
}