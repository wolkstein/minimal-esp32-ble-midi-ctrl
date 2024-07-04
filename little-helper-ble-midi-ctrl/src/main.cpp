#include "main.h"
#include <Arduino.h>
#include <BLEMidi.h>
#include <FastLED.h>
#include <AceButton.h>

using namespace ace_button;

#define WS28XX_LED_PIN 33 // GPIO 33
#define NUM_LEDS  1
#define BRIGHTNESS 15

uint8_t __active_map = 0; // 0 = map 1, 1 = map 2 ... usw.

std::__cxx11::string midiDeviceName = "LITTLE_HELPER_ONE";


// LED Strucutre
CRGB myWS28XXLED[NUM_LEDS];

AceButton btn1; // GPIO 10
AceButton btn2; // GPIO 11
AceButton btn3; // GPIO 12
AceButton btn4; // GPIO 13
AceButton btn5; // GPIO 14


myButton myBtnMap1_1_s = {10, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 60, 100, 43, 127, 0, MMC_REWIND};
myButton myBtnMap2_1_s = {11, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 61, 100, 42, 127, 0, MMC_STOP};
myButton myBtnMap3_1_s = {12, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 62, 100, 44, 127, 0, MMC_FAST_FORWARD};
myButton myBtnMap4_1_s = {13, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 63, 100, 64, 127, 0, MMC_RECORD_STROBE};
myButton myBtnMap5_1_s = {14, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 64, 100, 41, 127, 0, MMC_PLAY};

myButton myBtnMap1_1_l = {10, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 60, 100, 43, 127, 0, MMC_REWIND};
// myButton myBtnMap2_1_l = {11, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 61, 100, 42, 127, 0, MMC_STOP}; // btn 2 longpree is change mapping
myButton myBtnMap3_1_l = {12, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 62, 100, 44, 127, 0, MMC_FAST_FORWARD};
myButton myBtnMap4_1_l = {13, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 45, 100, 45, 127, 0, MMC_RECORD_STROBE};
myButton myBtnMap5_1_l = {14, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 64, 100, 41, 127, 0, MMC_PLAY};

myButton myBtnMap1_2_s = {10, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 60, 100, 111, 127, 0, MMC_REWIND};
myButton myBtnMap2_2_s = {11, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 61, 100, 42, 127, 0, MMC_STOP};
myButton myBtnMap3_2_s = {12, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 62, 100, 112, 127, 0, MMC_FAST_FORWARD};
myButton myBtnMap4_2_s = {13, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 63, 100, 64, 127, 0, MMC_RECORD_STROBE};
myButton myBtnMap5_2_s = {14, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 64, 100, 41, 127, 0, MMC_PAUSE};

myButton myBtnMap1_2_l = {10, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 60, 100, 111, 127, 0, MMC_REWIND};
// myButton myBtnMap2_2_l = {11, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 61, 100, 42, 127, 0, MMC_STOP}; // btn 2 longpree is change mapping
myButton myBtnMap3_2_l = {12, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 62, 100, 112, 127, 0, MMC_FAST_FORWARD};
myButton myBtnMap4_2_l = {13, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 45, 100, 45, 127, 0, MMC_RECORD_STROBE};
myButton myBtnMap5_2_l = {14, BTN_PUSH, BTN_OFF, BTN_RED, MIDIFUNC_CC, MIDI_CH_1, 64, 100, 41, 127, 0, MMC_PLAY};


// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /*buttonState*/) { 
    
    uint8_t pin = button->getPin();

    switch (eventType) {
      case AceButton::kEventPressed:
        Serial.println(F("handleEvent(): Pressed"));
        break;
      case AceButton::kEventReleased:
        Serial.println(F("handleEvent(): Released"));
        break;
      case AceButton::kEventClicked:
        Serial.println(F("handleEvent(): Clicked"));
        break;
      case AceButton::kEventDoubleClicked:
        Serial.println(F("handleEvent(): DoubleClicked"));
        break;
      case AceButton::kEventLongPressed:
        Serial.println(F("handleEvent(): LongPressed"));
        break;
      // case AceButton::kEventLongPressStop:
      //   Serial.println(F("handleEvent(): LongPressStop"));
      //   break;
      // case AceButton::kEventVeryLongPressed:
      //   Serial.println(F("handleEvent(): VeryLongPressed"));
      //   break;
      // case AceButton::kEventVeryLongPressStop:
      //   Serial.println(F("handleEvent(): VeryLongPressStop"));
      //   break;
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
  myWS28XXLED[0] = CRGB::Red;
  FastLED.show();
}


void setup() {
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


  // Button uses the built-in pull up register.
  // The button is connected to GPIO 0.
  pinMode(myBtnMap1_1_s.btnGpio, INPUT);
  pinMode(myBtnMap2_1_s.btnGpio, INPUT);
  pinMode(myBtnMap3_1_s.btnGpio, INPUT);
  pinMode(myBtnMap4_1_s.btnGpio, INPUT);
  pinMode(myBtnMap5_1_s.btnGpio, INPUT);



   // Configure the ButtonConfig with the event handler.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setLongPressDelay(2000);
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