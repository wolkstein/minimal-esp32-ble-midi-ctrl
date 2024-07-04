#include <Arduino.h>
#include <BLEMidi.h>
#include <FastLED.h>

#define WS28XX_LED_PIN 33
#define NUM_LEDS  1
#define BRIGHTNESS 15

// LED Strucutre
CRGB myWS28XXLED[NUM_LEDS];

#include <AceButton.h>

using namespace ace_button;

// The pin number attached to the button.
const uint8_t BUTTON_PIN = 10;

// Create one button.
AceButton button;

// counters to determine the duration of a single call to AceButton::check()
uint16_t innerLoopCounter = 0;
uint16_t outerLoopCounter = 0;
unsigned long startMillis = 0;
unsigned long stopMillis  = 0;

// states of the stopwatch
const uint8_t STOPWATCH_INIT = 0;
const uint8_t STOPWATCH_STARTED = 1;
const uint8_t STOPWATCH_STOPPED = 2;

// implements a finite state machine (FSM)
uint8_t stopwatchState = STOPWATCH_INIT;

bool allEventsEnabled = false;

void enableAllEvents() {
  Serial.println(F("enabling high level events"));
  button.getButtonConfig()->setFeature(ButtonConfig::kFeatureClick);
  button.getButtonConfig()->setFeature(ButtonConfig::kFeatureDoubleClick);
  button.getButtonConfig()->setFeature(ButtonConfig::kFeatureLongPress);
  button.getButtonConfig()->setFeature(ButtonConfig::kFeatureRepeatPress);
}

void disableAllEvents() {
  Serial.println(F("disabling high level events"));
  button.getButtonConfig()->clearFeature(ButtonConfig::kFeatureClick);
  button.getButtonConfig()->clearFeature(ButtonConfig::kFeatureDoubleClick);
  button.getButtonConfig()->clearFeature(ButtonConfig::kFeatureLongPress);
  button.getButtonConfig()->clearFeature(ButtonConfig::kFeatureRepeatPress);
}

// The event handler for the button.
void handleEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  unsigned long now = millis();
  switch (eventType) {
    case AceButton::kEventPressed:
      if (stopwatchState == STOPWATCH_INIT) {

        // enable or disable higher level events, to get different performance
        // numbers
        if (allEventsEnabled) {
          enableAllEvents();
        } else {
          disableAllEvents();
        }

        Serial.println(F("handleEvent(): stopwatch started"));
        startMillis = now;
        innerLoopCounter = 0;
        outerLoopCounter = 0;
        stopwatchState = STOPWATCH_STARTED;
      } else if (stopwatchState == STOPWATCH_STARTED) {
        stopMillis = now;
        stopwatchState = STOPWATCH_STOPPED;
        unsigned long duration = stopMillis - startMillis;
        uint32_t loopCounter = ((uint32_t) outerLoopCounter << 16) +
            innerLoopCounter;
        float microsPerLoop = duration * 1000.0f / loopCounter;

        // reenable all events after stopping
        enableAllEvents();

        Serial.println(F("handleEvent(): stopwatch stopped"));
        Serial.print(F("handleEvent(): duration (ms): "));
        Serial.print(duration);
        Serial.print(F("; loopCount: "));
        Serial.print(loopCounter);
        Serial.print(F("; micros/loop: "));
        Serial.println(microsPerLoop);

        // Setting 0 allows the loop() function to return periodically.
        innerLoopCounter = 0;
      }
      break;
    case AceButton::kEventLongPressed:
      if (stopwatchState == STOPWATCH_STOPPED) {
        stopwatchState = STOPWATCH_INIT;
        Serial.println(F("handleEvent(): stopwatch reset"));
        allEventsEnabled = !allEventsEnabled;
      }
      break;
  }
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
  Serial.println("Initializing bluetooth");


  // Button uses the built-in pull up register.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button.init(BUTTON_PIN);

  // Configure the ButtonConfig with the event handler and enable LongPress.
  // SupressAfterLongPress is optional since we don't do anything if we get it.
  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setLongPressDelay(2000);

  Serial.println(F("setup(): stopwatch ready"));












  BLEMidiServer.begin("Basic MIDI device2");
  Serial.println("Waiting for connections...");
  BLEMidiServer.enableDebugging();  // Uncomment if you want to see some debugging output from the library (not much for the server class...)
}

void loop() {


    // We split the loop into an inner loop and an outer loop. The inner loop
  // allows us to measure the speed of button.check() without the overhead of
  // the outer loop. However, we must allow the outer loop() method to return
  // periodically to allow the microcontroller to its own stuff. This is
  // especially true on an ESP8266 board, where a Watch Dog Timer will
  // soft-reset the board if loop() doesn't return every few seconds.
  do {
    // button.check() Should be called every 4-5ms or faster for the default
    // debouncing time of ~20ms.
    button.check();

    // increment loop counter
    if (stopwatchState == STOPWATCH_STARTED) {
      innerLoopCounter++;
    }
  } while (innerLoopCounter);
      if(BLEMidiServer.isConnected()) {             // If we've got a connection, we send an A4 during one second, at full velocity (127)
      BLEMidiServer.noteOn(0, 69, 127);
     // Serial.println("Note on");
      myWS28XXLED[0] = CRGB::Purple;
      FastLED.show();
      delay(100);
      BLEMidiServer.noteOff(0, 69, 127);        // Then we make a delay of one second before returning to the beginning of the loop
    //Serial.println("Note off");
       myWS28XXLED[0] = CRGB::Green;
      FastLED.show(); 


  }
  // Each time the innerLoopCounter rolls over (65536), increment the outer loop
  // counter, and return from loop(), to prevent WDT errors on ESP8266.
  outerLoopCounter++;
}