#include <Arduino.h>
#include "defs.h"
#include "routine.h"
#include "framebuffer.h"
#include "images.h"


#define IMAGE(name) { ARRAYSIZE(name[0]), ARRAYSIZE(name), name[0] }

#include "glyphs.h"
#include "glyph_supermario.h"
#include "glyph_retro.h"
#include "glyph_mac.h"
#include "glyph_smileys.h"



void ImageRoutine::begin(void *stateBuf) {
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);
   data->sparkle = true; // because all the routines immediately invert it
   data->randomizeOnBeat = true;
}

void ImageRoutine::adjustParam(int step) {
   // press changes image
   data->whichImage = (data->whichImage + step + data->numImages) % data->numImages;
   fb.clearScreen();

   // after cycling through image, start doing funner stuff
   if (!data->whichImage) {
      data->sparkle = !data->sparkle;
   }

   // double-click toggles randomize
   int now = millis();
   int delta = now - data->lastClick;
   if (delta > 150 && delta < 500) {
      data->randomizeOnBeat = !data->randomizeOnBeat;
   }
   data->lastClick = now;
}

void ImageRoutine::drawOnBeatSync(FrameTimingInfo *frameTiming) {
   if (data->randomizeOnBeat && data->beatStep == 0) {
      data->whichImage = random(data->numImages);
      data->throbStep = 0;
   }
   data->beatStep = (data->beatStep + 1) % 4;
}

void ImageRoutine::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   int numThrobSteps = frameTiming->beatLength * 4 / frameTiming->frameLength;
   int center = numThrobSteps / 2;
   int dist = abs(center - data->throbStep);
   float scale = (center - dist) * 1.0 / center;
   scale = scale * scale * scale; // decay faster
   data->throbStep = (data->throbStep + 1) % numThrobSteps;

   const ImageInfo *image = data->images + data->whichImage;

   for (int y = 0; y < image->cy; y++) {
      const uint8_t *row = image->data + y * image->cx;
      for (int x = 0; x < image->cx / 3; x++) {
         // calculate pixel
         int r = row[3 * x + 0];
         int g = row[3 * x + 1];
         int b = row[3 * x + 2];
         int pixel = r << 16 | g << 8 | b;

         // fade pixel
         pixel = fb.scalePixel(pixel, scale);

         // we have origin at bottom left; bitmaps have origin at top left
         // so flip vertically
         fb.setGridPixel(x, fb.height - 1 - y, pixel);
      }
   }

   fb.showWithLimit();
   data->frameSparkled = false;
}

bool ImageRoutine::drawBetweenFrames(FrameTimingInfo *frameTiming) {
   if (data->sparkle && !data->frameSparkled) {
      int numToSparkle = fb.width * fb.height / 10;
      for (int i = 0; i < numToSparkle; i++) {
         int x = random(fb.width);
         int y = random(fb.height);
         int pixel = fb.getGridPixel(x, y);
         int limit = fb.getPixelBrightness(pixel);
         pixel = fb.randomPrimaryOrSecondary(limit);
         fb.setGridPixel(x, y, pixel);
      }
      data->frameSparkled = true;
      fb.showWithLimit();
      return true;
   }

   return false;
}



void ThemeImageRoutine::begin(void *stateBuf) {
   ImageRoutine::begin(stateBuf);

   data->images = theme_images;
   data->numImages = ARRAYSIZE(theme_images);
   adjustParam(0);
}

void MarioImageRoutine::begin(void *stateBuf) {
   ImageRoutine::begin(stateBuf);

   data->images = mario_images;
   data->numImages = ARRAYSIZE(mario_images);
   adjustParam(0);
}

