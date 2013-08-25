/*
 * Control software for the dreamcoat,
 * which is several separate arrays of WS2812 LEDs controlled by Teensy 3 and OctoWS2811.
 *
 * (c) 2013 Matt Ginzton, matt@ginzton.net
 */

#include <OctoWS2811.h>
#include "defs.h"
#include "routine.h"
#include "scene.h"
#include "framebuffer.h"
#include "control_pad.h"
#include "platform.h"


/*
 * Input and output utilities, higher order objects
 */

Framebuffer fb;
ControlPad controls;
Scene scene;


void setup() {
#if DEBUG
   Serial.begin(9600); // Teensy ignores baud rate parameter
#endif

   randomSeed(analogRead(0));  // read from an analog port with nothing connected
   pinMode(builtinLedPin, OUTPUT);

   fb.begin();
   controls.begin(CONTROL_MAJMODE_PREV_PIN, CONTROL_MAJMODE_NEXT_PIN,
                  CONTROL_MINMODE_PREV_PIN, CONTROL_MINMODE_NEXT_PIN,
                  CONTROL_SPEED_PIN, CONTROL_NEEDS_PULLUP);
   scene.begin();
}


void loop() {
   while (TRUE) {
      scene.loop();
   }
}
