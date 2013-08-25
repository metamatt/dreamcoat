#pragma once

/*
 * Useful C stuff
 */
typedef char BOOL;

#define FALSE 0
#define TRUE 1

#define ARRAYSIZE(a) ((sizeof(a) / sizeof(a[0])))


/*
 * Debugging helpers
 */

#define DEBUG 1

#if DEBUG
#define DebugPrint(...) {                        \
   char buf[200];                                \
   snprintf(buf, ARRAYSIZE(buf), __VA_ARGS__);   \
   Serial.print(buf);                            \
}

#else

#define DebugPrint(x)

#endif


/*
 * Teensy config
 */
const int builtinLedPin = 13;
