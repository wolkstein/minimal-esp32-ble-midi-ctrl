
#include <Arduino.h> // Standard Arduino Library
#include <FastLED.h>
#include "esp_log.h"

#ifndef MAIN_H // Makro-Wächter, um Mehrfachinklusionen zu verhindern
#define MAIN_H

#define MIDIFUNC_NOTE 0
#define MIDIFUNC_CC 1
#define MIDIFUNC_SYSEX 2
#define MIDIFUNC_PC 3



String  midiDeviceName = "LITTLE_HELPER";

String ssid = "LocalWlan";
String password = "localWlanPassword";
String ap_ssid = "LittleHelperAP";
String ap_password = "12345678";
String hostname = "littlehelper";

uint8_t __HW_BUTTONS = 5; // Anzahl der Hardware Buttons
uint16_t button1;

uint16_t status;

uint16_t bleNameTxtField;
uint16_t wlanSsidNameTxtField;
uint16_t wlanPasswordTxtField;
uint16_t wlanApSsidTxtField;
uint16_t wlanApPasswordTxtField;
uint16_t hostnameTxtField;

bool __configurator = false;

#define WS28XX_LED_PIN 33 // GPIO 33
#define NUM_LEDS  1
#define BRIGHTNESS 100

//struct my_config_names
uint8_t __active_map = 0; // 0 = map 1, 1 = map 2 ... usw.
uint8_t __active_map_ui_btn[5] = {0, 0, 0, 0, 0};

bool __isConnected = false;

uint16_t __selectUiBtn[5][11]  = {
  // selectBtn1Map, selectBtn1MidiChannel, selectBtn1MidiFunction, selectBtn1CCFunction, selectBtn1CCValueMax, selectBtn1CCValueMin, selectBtn1MidiNote, selectBtn1NoteVelocity
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

enum my_mmc_t {
  MMC_STOP          = 0x01,
  MMC_PLAY          = 0x02,
  MMC_DEFERRED_PLAY = 0x03,
  MMC_FAST_FORWARD  = 0x04,
  MMC_REWIND        = 0x05,
  MMC_RECORD_STROBE = 0x06,
  MMC_RECORD_EXIT   = 0x07,
  MMC_RECORD_PAUSE  = 0x08,
  MMC_PAUSE         = 0x09,
  MMC_EJECT         = 0x0A,
  MMC_CHASE         = 0x0B,
  MMC_RESET         = 0x0D,
};

enum my_btn_function {
  BTN_PUSH = 0x00,
  BTN_TOGGLE = 0x01,
};

enum my_btn_state {
  BTN_OFF = 0x00,
  BTN_ON = 0x01,
};

enum my_midi_function {
  MIDI_NOTE = 0x00,
  MIDI_CC = 0x01,
  MIDI_MMC = 0x02,
  MIDI_PROGRAMCHANGE = 0x03,
};

enum my_midi_cc {
  MIDI_CC_VOLUME = 0x07,
  MIDI_CC_PAN = 0x0A,
  MIDI_CC_EXPRESSION = 0x0B,
  MIDI_CC_SUSTAIN = 0x40,
  MIDI_CC_PORTAMENTO = 0x41,
  MIDI_CC_DATAENTRY = 0x06,
  MIDI_CC_BANKSELECT = 0x00,
  MIDI_CC_MODULATION = 0x01,
  MIDI_CC_BREATH = 0x02,
  MIDI_CC_FOOT = 0x04,
  MIDI_CC_PORTAMENTOTIME = 0x05,
  MIDI_CC_REVERB = 0x5B,
  MIDI_CC_CHORUS = 0x5D,
  MIDI_CC_DELAY = 0x5E,
  MIDI_CC_PHASER = 0x5F,
};

enum my_midi_channel {
  MIDI_CH_1 = 0x00,
  MIDI_CH_2 = 0x01,
  MIDI_CH_3 = 0x02,
  MIDI_CH_4 = 0x03,
  MIDI_CH_5 = 0x04,
  MIDI_CH_6 = 0x05,
  MIDI_CH_7 = 0x06,
  MIDI_CH_8 = 0x07,
  MIDI_CH_9 = 0x08,
  MIDI_CH_10 = 0x09,
  MIDI_CH_11 = 0x0A,
  MIDI_CH_12 = 0x0B,
  MIDI_CH_13 = 0x0C,
  MIDI_CH_14 = 0x0D,
  MIDI_CH_15 = 0x0E,
  MIDI_CH_16 = 0x0F,
};

struct myButton
{
  uint8_t btnGpio; // GPIO Pin bleibt unverändert
  bool needRelease[2]; // Button Release als Array
  uint8_t btnFunction[2]; // Button Function als Array
  bool btnLongpress[2]; // Button Longpress als Array
  uint8_t btnState[2]; // Button State als Array
  uint32_t btnColor[2]; // Button Color als Array
  uint8_t btnMidiFunction[2]; // Button MIDI Function als Array
  uint8_t btnMidiChannel[2]; // Button MIDI Channel als Array
  uint8_t btnMidiNote[2]; // Button MIDI Note als Array
  uint8_t btnMidiVelocity[2]; // Button MIDI Velocity als Array
  uint8_t btnMidiCC[2]; // Button MIDI CC als Array
  uint8_t btnMidiCCValueStateOn[2]; // Button MIDI Value State On als Array
  uint8_t btnMidiCCValueStateOff[2]; // Button MIDI Value State Off als Array
  uint8_t btnMidiMMC[2]; // Button MIDI MMC als Array
};


#endif // MAIN_H
