#include "main.h"
#include <Arduino.h>
#include <BLEMidi.h>
#include <FastLED.h>
#include <AceButton.h>
#include <Preferences.h>

Preferences prefs;

using namespace ace_button;

#define WS28XX_LED_PIN 33 // GPIO 33
#define NUM_LEDS  1
#define BRIGHTNESS 100

uint8_t __active_map = 0; // 0 = map 1, 1 = map 2 ... usw.

std::__cxx11::string midiDeviceName = "LITTLE_HELPER_TWO";

bool __isConnected = false;

// LED Strucutre
CRGB myWS28XXLED[NUM_LEDS];


myButton myBtnMap[5] = { // 5 Buttons 4 Maps Map 1 und Map 2 are short press values, Map 3 and Map 4 are long press values
    { // Button 1
         10,  // GPIO Pin
         {false, false, false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false, true, true}, // Button Long Press
         {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_RED, BTN_RED, BTN_RED, BTN_RED}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
         {MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
         {MIDI_CH_1, MIDI_CH_1, MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
         {60, 60, 60, 60}, // Button MIDI Note 0 - 127
         {100, 100, 100, 100}, // Button MIDI Velocity 0 - 127
         {43, 111, 43, 111}, // Button MIDI CC 0 - 127
         {127, 127, 127, 127}, // Button MIDI CC ON Value 0 - 127
         {0, 0, 0, 0}, // Button MIDI CC OFF Value 0 - 127
         {MMC_REWIND, MMC_REWIND, MMC_REWIND, MMC_REWIND} // Button MIDI MMC 0 - 13
    },
    { // Button 2
         11,  // GPIO Pin
         {false, false, false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false, true, true}, // Button Long Press
         {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_RED, BTN_RED, BTN_RED, BTN_RED}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
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
         {false, false, true, true}, // Button Long Press
         {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_RED, BTN_RED, BTN_RED, BTN_RED}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
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
         {true, true, false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false, true, true}, // Button Long Press
         {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_RED, BTN_RED, BTN_RED, BTN_RED}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
         {MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC, MIDIFUNC_CC}, // Button MIDI Function 0 = Note, 1 = CC, 2 = MMC, 3 = Program Change
         {MIDI_CH_1, MIDI_CH_1, MIDI_CH_1, MIDI_CH_1}, // Button MIDI Channel 0 - 15
         {62, 62, 62, 62}, // Button MIDI Note 0 - 127
         {100, 100, 100, 100}, // Button MIDI Velocity 0 - 127
         {64, 64, 45, 45}, // Button MIDI CC 0 - 127
         {127, 127, 127, 127}, // Button MIDI CC ON Value 0 - 127
         {0, 0, 0, 0}, // Button MIDI CC OFF Value 0 - 127
         {MMC_STOP, MMC_STOP, MMC_STOP, MMC_STOP} // Button MIDI MMC 0 - 13
    },
    { // Button 5
         14,  // GPIO Pin
         {false, false, false, false}, // Button Release
         {BTN_PUSH, BTN_PUSH, BTN_PUSH, BTN_PUSH},// Button Function 0 = Push, 1 = Toggle
         {false, false, true, true}, // Button Long Press
         {BTN_OFF, BTN_OFF, BTN_OFF, BTN_OFF}, // Button State 0 = Off, 1 = On
         {BTN_RED, BTN_RED, BTN_RED, BTN_RED}, // Button Color 0 = Red, 1 = Green, 2 = Blue, 3 = Yellow, 4 = Purple, 5 = Cyan, 6 = White
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
    if(logpressevent) active_mapper = __active_map + 2; // 0 = map 1, 1 = map 2, 2 = map 3, 3 = map 4
    
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
            FastLED.show();
          } else if (__active_map == 1 ) {
            __active_map = 0;
            myWS28XXLED[0] = CRGB::Green;
            if(!__isConnected) myWS28XXLED[0] = CRGB::Red;
            FastLED.show();
          }
          return;
        }

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
  } else if (__active_map == 1 ) {
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
  __isConnected = false;
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();
}


void setup() {
  prefs.begin("Settings");  //namespace


  // initialize WS28xx LED in GRB order
  FastLED.addLeds<WS2812B, WS28XX_LED_PIN, GRB>(myWS28XXLED, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();
    
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

  if (not prefs.isKey("Settings")) {
    Serial.println("Settings not found, saving default settings");
    prefs.putBytes("Settings", &myBtnMap, sizeof(myBtnMap));
  } else {
    Serial.println("Settings found, loading settings");
    prefs.getBytes("Settings", &myBtnMap, sizeof(myBtnMap));
  }

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

  log_i("Starting BLE MIDI server");
  BLEMidiServer.begin(midiDeviceName);
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
  delay(1);
}