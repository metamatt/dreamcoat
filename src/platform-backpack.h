/*
 * platform.h
 *
 * Hardware configuration parameters for each device.
 */

#pragma once

// Framebuffer "physical" layout is the LED layout: the way it's actually wired
// (physical width is the number of strips, probably 8;
// physical height is the number of LEDs in the longest strip)
static const int FB_PHYSICAL_WIDTH = 8;
static const int FB_PHYSICAL_HEIGHT = 32;

// Framebuffer "virtual" layout is the way you want to treat the framebuffer:
// rows and columns (we might split one strip into multiple shorter columns),
// and orientation (I chose to make the lower left the origin, and y grows up,
// because the backpack is wired that way).
static const int FB_VIRTUAL_WIDTH = 16;
static const int FB_VIRTUAL_HEIGHT = 16;
static const bool FB_MIRROR_X = false;
static const bool FB_MIRROR_Y = false;

// Pins to use for control-button inputs
static const int CONTROL_MAJMODE_PREV_PIN = 0;
static const int CONTROL_MAJMODE_NEXT_PIN = 9;
static const int CONTROL_MINMODE_PREV_PIN = 0;
static const int CONTROL_MINMODE_NEXT_PIN = 10;
static const int CONTROL_SPEED_PIN = 11;
static const bool CONTROL_NEEDS_PULLUP = true;

// See scene.cpp.
#define Platform_DeclareRoutines \
   USE(DripRoutine)              \
   USE(Sparkle)                  \
   USE(SnakeRoutine)             \
   USE(SwirlRoutine)             \
   USE(TranslucentSquares)       \
   USE(ThrobRoutine)             \
   USE(GeoGrow)                  \
   USE(ColorWash)                \
   USE(PlasmaRoutine)            \
   USE(ThemeImageRoutine)        \
   USE(MarioImageRoutine)        \
   USE(RetroImageRoutine)        \
   USE(SmileyImageRoutine)       \
   USE(Simon)                    \
   USE(StripeRoutine)            \
   /* end */
// USE(OrientationRoutine)
