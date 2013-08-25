#include <OctoWS2811.h>
#include "defs.h"
#include "framebuffer.h"
#include "platform.h"


/*
 * Strip layout parameters
 */
static const int ledsPerActualStrip = FB_PHYSICAL_HEIGHT;
static const int ledStripConfig = WS2811_GRB | WS2811_800kHz;

static_assert(ledsPerActualStrip * 8 == FB_VIRTUAL_WIDTH * FB_VIRTUAL_HEIGHT,
              "Framebuffer virtual/physical layout mismatch");

// XXX this is a poor version of FB_LAYOUT, only supporting the cases I
// need, which is 8 * x and 16 * x.
static const int stripsPerVStrip = FB_VIRTUAL_WIDTH / FB_PHYSICAL_WIDTH;

/*
 * Framebuffers
 */

DMAMEM byte displayMemory[ledsPerActualStrip * 24];
byte drawingMemory[ledsPerActualStrip * 24];
OctoWS2811 leds(ledsPerActualStrip, displayMemory, drawingMemory, ledStripConfig);


Framebuffer::Framebuffer() {
   this->width = FB_VIRTUAL_WIDTH;
   this->height = FB_VIRTUAL_HEIGHT;
   this->ledsPerStrip = ledsPerActualStrip;
   this->numPixels = width * height;
}

void Framebuffer::begin() {
   leds.begin();
   leds.show();
}

void Framebuffer::setGridPixel(int x, int y, int color) {
   if (x < 0 || x >= fb.width || y < 0 || y >= fb.height) return;

   // XXX should provide way to customize this for arbitrary layout
   // Right now, all I need is:
   // - backpack: alternating strip direction (like FB_LAYOUT 0)
   // - jacket: only 8 strips, but one side mirrors X
   if (FB_MIRROR_X) {
      x = fb.width - x - 1;
   }
   if (FB_MIRROR_Y) {
      y = fb.height - y - 1;
   }
   int realStrip = x / stripsPerVStrip;
   int realOffset = (x % stripsPerVStrip) ? y : ledsPerStrip - 1 - y;

   leds.setPixel(realOffset + realStrip * ledsPerStrip, color);
}

int Framebuffer::getGridPixel(int x, int y) {
   if (x < 0 || x >= fb.width || y < 0 || y >= fb.height) return 0;

   // XXX should provide way to customize this for arbitrary layout
   // Right now, all I need is:
   // - backpack: alternating strip direction (like FB_LAYOUT 0)
   // - jacket: only 8 strips, but one side mirrors X
   if (FB_MIRROR_X) {
      x = fb.width - x - 1;
   }
   if (FB_MIRROR_Y) {
      y = fb.height - y - 1;
   }
   int realStrip = x / stripsPerVStrip;
   int realOffset = (x % stripsPerVStrip) ? y : ledsPerStrip - 1 - y;

   return leds.getPixel(realOffset + realStrip * ledsPerStrip);
}

void Framebuffer::drawGridLine(int x0, int y0, int x1, int y1, int color) {
   int x, y;
   if (x0 == x1) { // vertical line
      int dy = (y0 < y1) ? 1 : -1;
      for (y = y0; y != y1 + dy; y += dy) {
         setGridPixel(x0, y, color);
      }
   } else if (y0 == y1) { // horizontal
      int dx = (x0 < x1) ? 1 : -1;
      for (x = x0; x != x1 + dx; x += dx) {
         setGridPixel(x, y0, color);
      }
   } else if (abs(x1 - x0) == abs(y1 - y0)) { // 45 degree diagonal
      int xStep = (x1 > x0) ? 1 : -1;
      int yStep = (y1 > y0) ? 1 : -1;
      for (x = x0, y = y0; x != x1 + xStep; x += xStep, y += yStep) {
         setGridPixel(x, y, color);
      }
   } else { // arbitrary angle, Bresenham DDA
      int dx = abs(x1 - x0);
      int dy = abs(y1 - y0);
      int sx = (x0 < x1) ? 1 : -1;
      int sy = (y0 < y1) ? 1 : -1;
      int err = dx - dy;

      while (true) {
         setGridPixel(x0, y0, color);
         if (x0 == x1 && y0 == y1) {
            break;
         }
         int e2 = err * 2;
         if (e2 > -dy) {
            err -= dy;
            x0 += sx;
         }
         if (x0 == x1 && y0 == y1) {
            setGridPixel(x0, y0, color);
            break;
         }
         if (e2 < dx) {
            err += dx;
            y0 += sy;
         }
      }
   }
}

int Framebuffer::scalePixel(int pixel, float scale) {
   int r = (pixel >> 16) & 0xFF;
   int g = (pixel >> 8) & 0xFF;
   int b = pixel & 0xFF;

   r *= scale;
   g *= scale;
   b *= scale;

   return r << 16 | g << 8 | b;
}

float Framebuffer::remap(float value, float from1, float from2, float to1, float to2) {
   float d1 = from2 - from1;
   float d2 = to2 - to1;

   value -= from1;
   value *= d2 / d1;
   return value + to1;
}

void Framebuffer::clearScreen() {
   memset(drawingMemory, 0, 3 * numPixels);
}

void Framebuffer::fillScreen(int color) {
   for (int i = 0; i < numPixels; i++) {
      leds.setPixel(i, color);
   }
}

void Framebuffer::fadeScreenByStep(int fade, int base) {
   for (int i = 0; i < numPixels; i++) {
      int pixel = leds.getPixel(i);
      int r = (pixel >> 16) & 0xFF;
      int g = (pixel >> 8) & 0xFF;
      int b = pixel & 0xFF;

      if (r + g + b > base) {
         r -= fade;
         g -= fade;
         b -= fade;
         if (r < 0) r = 0;
         if (g < 0) g = 0;
         if (b < 0) b = 0;
      }

      pixel = r << 16 | g << 8 | b;
      leds.setPixel(i, pixel);
   }
}

void Framebuffer::fadeScreenByScale(float scale) {
   for (int i = 0; i < numPixels; i++) {
      int pixel = leds.getPixel(i);
      pixel = scalePixel(pixel, scale);
      leds.setPixel(i, pixel);
   }
}


void Framebuffer::showWithLimit() {
   byte *start = (byte *)drawingMemory;
   byte *end = start + 3 * numPixels;
   byte *p;
   int whichBit = 7;
   int whichByte = 0;
   unsigned long values[3] = { 0 };

   // XXX I can't do this 24 bits at a time; need to do 8 (G R B separately), and sum or weighted-sum the results
   for (p = start; p < end; p++) {
      // count bits set in *p (Brian Kernighan's way, http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan)
      int thisByte = *p;
      int numSet;
      for (numSet = 0; thisByte; numSet++) {
         thisByte &= thisByte - 1; // clear least significant bit set
      }

      // depending on which virtual bit of the 24-bit cycle we're on, calculate this pixel's contribution to overall framebuffer brightness
      values[whichByte] += (numSet << whichBit);
    
      whichBit = (whichBit + 7) % 8; // subract 1, mod 8, stay positive
      if (whichBit == 7) { // wrapped around
         whichByte = (whichByte + 1) % 3;
      }
   }

   int value = values[0] + values[1] + values[2];
   int limit = 0x4000; // XXX just a guess: 1/4 of maximum possible

   if (value > limit) {
      fadeScreenByScale(1.0 * limit / value);
   }

   leds.show();
}


void Framebuffer::show() {
   leds.show();
}


int Framebuffer::randomColor(int componentMax) {
   return random(componentMax) << 16 | random(componentMax) << 8 | random(componentMax);
}

int Framebuffer::randomPrimary(int componentMax) {
   int component = random(componentMax);
   int shift = random(3);
   return component << (shift * 8);
}

int Framebuffer::randomPrimaryOrSecondary(int componentMax) {
   int component = random(componentMax);
   int component2 = random(componentMax);
   int shift = random(3);
   int shift2 = random(3);
   return component << (shift * 8) | component2 << (shift2 * 8);
}

bool Framebuffer::colorHasSameComponents(int c1, int c2) {
   int r1 = !!(c1 & 0xFF0000);
   int g1 = !!(c1 & 0x00FF00);
   int b1 = !!(c1 & 0x0000FF);
   int r2 = !!(c2 & 0xFF0000);
   int g2 = !!(c2 & 0x00FF00);
   int b2 = !!(c2 & 0x0000FF);

   return r1 == r2 && g1 == g2 && b1 == b2;
}

int Framebuffer::addPixelComponents(int p1, int p2) {
   int r1 = (p1 & 0xFF0000) >> 16;
   int g1 = (p1 & 0x00FF00) >> 8;
   int b1 = (p1 & 0x0000FF);
   int r2 = (p2 & 0xFF0000) >> 16;
   int g2 = (p2 & 0x00FF00) >> 8;
   int b2 = (p2 & 0x0000FF);

   r1 = min(r1 + r2, 255);
   g1 = min(g1 + g2, 255);
   b1 = min(b1 + b2, 255);
   return r1 << 16 | g1 << 8 | b1;
}

int Framebuffer::getPixelBrightness(int pixel) {
   int r = (pixel & 0xFF0000) >> 16;
   int g = (pixel & 0x00FF00) >> 8;
   int b = (pixel & 0x0000FF);
   return (r + g + b) / 3; // this is arbitrary, not perceptual brightness, but it will do
}

void Framebuffer::fillRect(int x0, int y0, int x1, int y1, int color) {
   for (int x = x0; x <= x1; x++) {
      for (int y = y0; y <= y1; y++) {
         setGridPixel(x, y, color);
      }
   }
}

void Framebuffer::drawNGram(float cx, float cy, unsigned numSides, float size, float rotate, int color) {
   unsigned i;
   int x[12], y[12];
   if (numSides < 3) {
      return;
   }
   if (numSides > ARRAYSIZE(x)) {
      numSides = ARRAYSIZE(x);
   }
   for (i = 0; i < numSides; i++) {
      x[i] = cx + size * cos(rotate + i * 2 * M_PI / numSides);
      y[i] = cy + size * sin(rotate + i * 2 * M_PI / numSides);
   }
   for (i = 0; i < numSides; i++) {
      int n = (i + 1) % numSides;
      drawGridLine(x[i], y[i], x[n], y[n], color);
   }
}

// void Framebuffer::drawCircle(float cx, float cy, float size, int color) {
// }
