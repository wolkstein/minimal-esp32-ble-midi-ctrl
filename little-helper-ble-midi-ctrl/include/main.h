
#include <Arduino.h> // Standard Arduino Library
#include <FastLED.h>
#include "esp_log.h"

#ifndef MAIN_H // Makro-Wächter, um Mehrfachinklusionen zu verhindern
#define MAIN_H

#define MIDIFUNC_NOTE 0
#define MIDIFUNC_CC 1
#define MIDIFUNC_SYSEX 2
#define MIDIFUNC_PC 3

#define NUBER_OF_MAPS 4



String  midiDeviceName = "LITTLE_HELPER";

String ssid = "LocalWlan";
String password = "localWlanPassword";
String ap_ssid = "LittleHelperAP";
String ap_password = "12345678";
String hostname = "littlehelper";

uint8_t __HW_BUTTONS = 5; // Number of HW Buttons;

uint16_t status;

uint16_t bleNameTxtField;
uint16_t wlanSsidNameTxtField;
uint16_t wlanPasswordTxtField;
uint16_t wlanApSsidTxtField;
uint16_t wlanApPasswordTxtField;
uint16_t hostnameTxtField;
uint16_t ledBrightnessTxtField;
uint16_t activeMapChooser;

bool __configurator = false;

#define WS28XX_LED_PIN 33 // GPIO 33
#define NUM_LEDS  1

uint8_t __BRIGHTNESS = 85;

//struct my_config_names
uint8_t __active_map = 0; // 0 = map 1, 1 = map 2 ... usw.
uint8_t __active_map_ui_btn[5] = {0, 0, 0, 0, 0};

bool __isConnected = false;

uint16_t __selectUiBtn[5][12]  = {
  // selectBtn1Map, selectBtn1MidiChannel, selectBtn1MidiFunction, selectBtn1CCFunction, selectBtn1MMCFunction, selectBtn1CCValueMax, selectBtn1CCValueMin, selectBtn1MidiNote, selectBtn1NoteVelocity
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
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
  bool needRelease[NUBER_OF_MAPS]; // Button Release als Array
  uint8_t btnFunction[NUBER_OF_MAPS]; // Button Function als Array
  bool btnLongpress[NUBER_OF_MAPS]; // Button Longpress als Array
  uint8_t btnState[NUBER_OF_MAPS]; // Button State als Array
  uint32_t btnColor[NUBER_OF_MAPS]; // Button Color als Array
  uint8_t btnMidiFunction[NUBER_OF_MAPS]; // Button MIDI Function als Array
  uint8_t btnMidiChannel[NUBER_OF_MAPS]; // Button MIDI Channel als Array
  uint8_t btnMidiNote[NUBER_OF_MAPS]; // Button MIDI Note als Array
  uint8_t btnMidiVelocity[NUBER_OF_MAPS]; // Button MIDI Velocity als Array
  uint8_t btnMidiCC[NUBER_OF_MAPS]; // Button MIDI CC als Array
  uint8_t btnMidiCCValueStateOn[NUBER_OF_MAPS]; // Button MIDI Value State On als Array
  uint8_t btnMidiCCValueStateOff[NUBER_OF_MAPS]; // Button MIDI Value State Off als Array
  uint8_t btnMidiMMC[NUBER_OF_MAPS]; // Button MIDI MMC als Array
};




uint32_t __btnLookUpTable[141] = {
  CRGB::HTMLColorCode::AliceBlue,
  CRGB::HTMLColorCode::Amethyst,
  CRGB::HTMLColorCode::AntiqueWhite,
  CRGB::HTMLColorCode::Aqua,
  CRGB::HTMLColorCode::Aquamarine,
  CRGB::HTMLColorCode::Azure,
  CRGB::HTMLColorCode::Beige,
  CRGB::HTMLColorCode::Bisque,
  CRGB::HTMLColorCode::Black,
  CRGB::HTMLColorCode::BlanchedAlmond,
  CRGB::HTMLColorCode::Blue,
  CRGB::HTMLColorCode::BlueViolet,
  CRGB::HTMLColorCode::Brown,
  CRGB::HTMLColorCode::BurlyWood,
  CRGB::HTMLColorCode::CadetBlue,
  CRGB::HTMLColorCode::Chartreuse,
  CRGB::HTMLColorCode::Chocolate,
  CRGB::HTMLColorCode::Coral,
  CRGB::HTMLColorCode::CornflowerBlue,
  CRGB::HTMLColorCode::Cornsilk,
  CRGB::HTMLColorCode::Crimson,
  CRGB::HTMLColorCode::Cyan,
  CRGB::HTMLColorCode::DarkBlue,
  CRGB::HTMLColorCode::DarkCyan,
  CRGB::HTMLColorCode::DarkGoldenrod,
  CRGB::HTMLColorCode::DarkGray,
  CRGB::HTMLColorCode::DarkGreen,
  CRGB::HTMLColorCode::DarkKhaki,
  CRGB::HTMLColorCode::DarkMagenta,
  CRGB::HTMLColorCode::DarkOliveGreen,
  CRGB::HTMLColorCode::DarkOrange,
  CRGB::HTMLColorCode::DarkOrchid,
  CRGB::HTMLColorCode::DarkRed,
  CRGB::HTMLColorCode::DarkSalmon,
  CRGB::HTMLColorCode::DarkSeaGreen,
  CRGB::HTMLColorCode::DarkSlateBlue,
  CRGB::HTMLColorCode::DarkSlateGray,
  CRGB::HTMLColorCode::DarkTurquoise,
  CRGB::HTMLColorCode::DarkViolet,
  CRGB::HTMLColorCode::DeepPink,
  CRGB::HTMLColorCode::FloralWhite,
  CRGB::HTMLColorCode::ForestGreen,
  CRGB::HTMLColorCode::Fuchsia,
  CRGB::HTMLColorCode::Gainsboro,
  CRGB::HTMLColorCode::GhostWhite,
  CRGB::HTMLColorCode::Gold,
  CRGB::HTMLColorCode::Goldenrod,
  CRGB::HTMLColorCode::Gray,
  CRGB::HTMLColorCode::Green,
  CRGB::HTMLColorCode::GreenYellow,
  CRGB::HTMLColorCode::Honeydew,
  CRGB::HTMLColorCode::HotPink,
  CRGB::HTMLColorCode::IndianRed,
  CRGB::HTMLColorCode::Indigo,
  CRGB::HTMLColorCode::Ivory,
  CRGB::HTMLColorCode::Khaki,
  CRGB::HTMLColorCode::Lavender,
  CRGB::HTMLColorCode::LavenderBlush,
  CRGB::HTMLColorCode::LawnGreen,
  CRGB::HTMLColorCode::LemonChiffon,
  CRGB::HTMLColorCode::LightBlue,
  CRGB::HTMLColorCode::LightCoral,
  CRGB::HTMLColorCode::LightCyan,
  CRGB::HTMLColorCode::LightGoldenrodYellow,
  CRGB::HTMLColorCode::LightGreen,
  CRGB::HTMLColorCode::LightGrey,
  CRGB::HTMLColorCode::LightPink,
  CRGB::HTMLColorCode::LightSalmon,
  CRGB::HTMLColorCode::LightSeaGreen,
  CRGB::HTMLColorCode::LightSkyBlue,
  CRGB::HTMLColorCode::LightSlateGray,
  CRGB::HTMLColorCode::LightSlateGrey,
  CRGB::HTMLColorCode::LightSteelBlue,
  CRGB::HTMLColorCode::LightYellow,
  CRGB::HTMLColorCode::Lime,
  CRGB::HTMLColorCode::LimeGreen,
  CRGB::HTMLColorCode::Linen,
  CRGB::HTMLColorCode::Magenta,
  CRGB::HTMLColorCode::Maroon,
  CRGB::HTMLColorCode::MediumAquamarine,
  CRGB::HTMLColorCode::MediumBlue,
  CRGB::HTMLColorCode::MediumOrchid,
  CRGB::HTMLColorCode::MediumPurple,
  CRGB::HTMLColorCode::MediumSeaGreen,
  CRGB::HTMLColorCode::MediumSlateBlue,
  CRGB::HTMLColorCode::MediumSpringGreen,
  CRGB::HTMLColorCode::MediumTurquoise,
  CRGB::HTMLColorCode::MediumVioletRed,
  CRGB::HTMLColorCode::MidnightBlue,
  CRGB::HTMLColorCode::MintCream,
  CRGB::HTMLColorCode::MistyRose,
  CRGB::HTMLColorCode::Moccasin,
  CRGB::HTMLColorCode::NavajoWhite,
  CRGB::HTMLColorCode::Navy,
  CRGB::HTMLColorCode::OldLace,
  CRGB::HTMLColorCode::Olive,
  CRGB::HTMLColorCode::OliveDrab,
  CRGB::HTMLColorCode::Orange,
  CRGB::HTMLColorCode::OrangeRed,
  CRGB::HTMLColorCode::Orchid,
  CRGB::HTMLColorCode::PaleGoldenrod,
  CRGB::HTMLColorCode::PaleGreen,
  CRGB::HTMLColorCode::PaleTurquoise,
  CRGB::HTMLColorCode::PaleVioletRed,
  CRGB::HTMLColorCode::PapayaWhip,
  CRGB::HTMLColorCode::PeachPuff,
  CRGB::HTMLColorCode::Peru,
  CRGB::HTMLColorCode::Pink,
  CRGB::HTMLColorCode::Plaid,
  CRGB::HTMLColorCode::Plum,
  CRGB::HTMLColorCode::PowderBlue,
  CRGB::HTMLColorCode::Purple,
  CRGB::HTMLColorCode::Red,
  CRGB::HTMLColorCode::RosyBrown,
  CRGB::HTMLColorCode::RoyalBlue,
  CRGB::HTMLColorCode::SaddleBrown,
  CRGB::HTMLColorCode::Salmon,
  CRGB::HTMLColorCode::SandyBrown,
  CRGB::HTMLColorCode::SeaGreen,
  CRGB::HTMLColorCode::Seashell,
  CRGB::HTMLColorCode::Sienna,
  CRGB::HTMLColorCode::Silver,
  CRGB::HTMLColorCode::SkyBlue,
  CRGB::HTMLColorCode::SlateBlue,
  CRGB::HTMLColorCode::SlateGray,
  CRGB::HTMLColorCode::SlateGrey,
  CRGB::HTMLColorCode::Snow,
  CRGB::HTMLColorCode::SpringGreen,
  CRGB::HTMLColorCode::SteelBlue,
  CRGB::HTMLColorCode::Tan,
  CRGB::HTMLColorCode::Teal,
  CRGB::HTMLColorCode::Thistle,
  CRGB::HTMLColorCode::Tomato,
  CRGB::HTMLColorCode::Turquoise,
  CRGB::HTMLColorCode::Violet,
  CRGB::HTMLColorCode::Wheat,
  CRGB::HTMLColorCode::White,
  CRGB::HTMLColorCode::WhiteSmoke,
  CRGB::HTMLColorCode::Yellow,
  CRGB::HTMLColorCode::YellowGreen
};

#endif // MAIN_H
