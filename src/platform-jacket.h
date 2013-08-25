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
static const int FB_PHYSICAL_HEIGHT = 23;

// Framebuffer "virtual" layout is the way you want to treat the framebuffer:
// rows and columns (we might split one strip into multiple shorter columns),
// and orientation (I chose to make the lower left the origin, and y grows up,
// because the backpack is wired that way).
static const int FB_VIRTUAL_WIDTH = 8;
static const int FB_VIRTUAL_HEIGHT = 23;
// The jacket exists in two halves, one of which is mirrored.
// Just recompile with the right definition here before upload.
#if 0 // right side of jacket (facing forward)
static const bool FB_MIRROR_X = false;
#else // left side of jacket (facing forward)
static const bool FB_MIRROR_X = true;
#endif
static const bool FB_MIRROR_Y = false;

// Pins to use for control-button inputs
static const int CONTROL_MAJMODE_PREV_PIN = 10;
static const int CONTROL_MAJMODE_NEXT_PIN = 18;
static const int CONTROL_MINMODE_PREV_PIN = 11;
static const int CONTROL_MINMODE_NEXT_PIN = 17;
static const int CONTROL_SPEED_PIN = 9;
static const bool CONTROL_NEEDS_PULLUP = false;


// See scene.cpp.
#define Platform_DeclareRoutines \
   USE(DripRoutine)              \
   USE(Sparkle)                  \
   USE(PlasmaRoutine)            \
   USE(ColorWash)                \
   USE(ThrobRoutine)             \
   USE(Simon)                    \
   /* end */

// The following would run on the jacket but are of dubious value,
// since they require a larger contiguous grid and/or better alignment
// (the back of the jacket is an 8x23 grid but the two sides are not
// currently synced up).
   // USE(SwirlRoutine)
   // USE(TranslucentSquares)
   // USE(GeoGrow)
   // USE(StripeRoutine)
   // USE(OrientationRoutine)
