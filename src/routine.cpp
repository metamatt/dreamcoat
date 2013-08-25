#include <Arduino.h>
#include "defs.h"
#include "routine.h"
#include "framebuffer.h"

void OrientationRoutine::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->oldX = -1;
}

void OrientationRoutine::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   if (!data->x && !data->y) {
      data->color = fb.randomColor();
   }

   fb.setGridPixel(data->oldX, data->oldY, 0);
   fb.setGridPixel(data->x, data->y, data->color);
   data->oldX = data->x;
   data->oldY = data->y;

   data->x++;
   if (data->x >= fb.width) {
      data->x = 0;
      data->y++;
      if (data->y >= fb.height) {
         data->y = 0;
      }
   }

   fb.show();
}



void StripeRoutine::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->color = fb.randomPrimaryOrSecondary(0x80);
}

void StripeRoutine::adjustParam(int step) {
   data->direction = (data->direction + step + 4) % 4;
}

void StripeRoutine::startBeatMeasure() {
   data->beat = -1;
}

void StripeRoutine::drawOnBeatSync(FrameTimingInfo *frameTiming) {
   // change color
   data->color = fb.randomPrimaryOrSecondary(0x80);

   data->beat = (data->beat + 1) % 4;
   // DebugPrint("beat now %d\n", data->beat);
   if (data->beat == 3) { // measure end
      fb.fillScreen(fb.randomPrimaryOrSecondary(0x20));
      fb.show();
      data->skipErase = fb.height;
   }
}

void StripeRoutine::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   int x1, y1, x2, y2, end;
   int step = (data->direction & 2) ? -1 : 1;
   switch (data->direction) {
      case 0:
      case 2:
         x1 = 0; x2 = fb.width - 1; y1 = y2 = data->row;
         end = fb.height;
         break;
      case 1:
      case 3:
         x1 = x2 = data->row; y1 = 0; y2 = fb.height - 1;
         end = fb.width;
         break;
      default:
         return;
   }

   // erase current stripe
   if (data->skipErase) {
      data->skipErase--;
   } else {
      fb.drawGridLine(x1, y1, x2, y2, 0);
   }

   // advance
   data->row = (data->row + step + end) % end;
   switch (data->direction) {
      case 0:
      case 2:
         x1 = 0; x2 = fb.width - 1; y1 = y2 = data->row;
         break;
      case 1:
      case 3:
         x1 = x2 = data->row; y1 = 0; y2 = fb.height - 1;
         break;
   }

   // draw next stripe
   fb.drawGridLine(x1, y1, x2, y2, data->color);

   fb.show();
}



void ColorWash::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   // direction and color will get initialized on first draw
   // because step == 0
}

void ColorWash::adjustParam(int step) {
   data->mode = (data->mode + step + 3) % 3;
}

void ColorWash::drawOnBeatSync(FrameTimingInfo *frameTiming) {
}

void ColorWash::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   int limit = 0xFF;

   switch (data->mode) {
      case 0: // medium bright
         limit = 0x80;
         break;
      case 1: // bright, fade 1
         fb.fadeScreenByStep(16, 10);
         break;
      case 2: // bright, fade 2
         fb.fadeScreenByScale(0.9);
         break;
   }

   if (!data->step) {
      int oldColor = data->color;
      data->direction = random(8);
      // avoid choosing same color (regardless of brightness)
      while (fb.colorHasSameComponents(data->color, oldColor)) {
         data->color = fb.randomPrimaryOrSecondary(limit);
      }
   }

   int& step = data->step;    // aliases to avoid having to say data->
   int& color = data->color;  // in all the cases below
   int end;

   switch (data->direction) {
      case 0: // top to bottom
         end = fb.height;
         fb.drawGridLine(0, step, end - 1, step, color);
         break;
      case 1: // bottom to top
         end = fb.height;
         fb.drawGridLine(0, end - 1 - step, end - 1, end - 1 - step, color);
         break;
      case 2: // left to right
         end = fb.width;
         fb.drawGridLine(step, 0, step, end - 1, color);
         break;
      case 3: // right to left
         end = fb.width;
         fb.drawGridLine(end - 1 - step, 0, end - 1 - step, end - 1, color);
         break;
      case 4: // diagonal 2 corners to middle
         end = max(fb.width, fb.height);
         fb.drawGridLine(0, step, step, 0, color);
         fb.drawGridLine(end - 1, end - 1 - step, end - 1 - step, end - 1, color);
         break;
      case 5: // diagonal middle out to 2 corners
         end = max(fb.width, fb.height);
         fb.drawGridLine(0, end - 1 - step, end - 1 - step, 0, color);
         fb.drawGridLine(end - 1, step, step, end - 1, color);
         break;
      case 6: // other diagonal 2 corners to middle
         end = max(fb.width, fb.height);
         fb.drawGridLine(0, end - 1 - step, step, end - 1, color);
         fb.drawGridLine(end - 1, step, end - 1 - step, 0, color);
         break;
      case 7: // other  diagonal middle out to 2 corners
         end = max(fb.width, fb.height);
         fb.drawGridLine(0, step, end - 1 - step, end - 1, color);
         fb.drawGridLine(end - 1, end - 1 - step, step, 0, color);
         break;
      default:
         return;
   }
   data->step = (data->step + 1) % end;

   fb.show();
}



// XXX how do I want to parameterize and speed-control this guy?

void Sparkle::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);
}

void Sparkle::adjustParam(int step) {
   data->mode = (data->mode + step + 3) % 3;
}

void Sparkle::drawOnBeatSync(FrameTimingInfo *frameTiming) {
}

void Sparkle::drawOnFrameSync(FrameTimingInfo *frameTiming) {
}

bool Sparkle::drawBetweenFrames(FrameTimingInfo *frameTiming) {
   // draw at most 1 per millisecond
   int now = millis();
   if (now == data->last) {
      return false;
   }
   data->last = now;

   int limit = 0xFF;

   switch (data->mode) {
      case 0: // dim, no fade
         limit = 0x10;
         break;
      case 1: // bright, fade 1
         fb.fadeScreenByStep(2, 10);
         break;
      case 2: // bright, fade 2
         fb.fadeScreenByScale(0.99);
         break;
   }

   int x = random(fb.width);
   int y = random(fb.height);
   int color = fb.randomColor(limit);
   fb.setGridPixel(x, y, color);
   fb.showWithLimit();

   return true;
}



void ThrobRoutine::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);
}

void ThrobRoutine::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   if (!data->step) {
      data->color = fb.randomColor(0x80);
   }

   int numSteps = frameTiming->beatLength * 4 / frameTiming->frameLength;
   int center = numSteps / 2;
   int dist = abs(center - data->step);
   float scale = float(center - dist) / center;
   scale = scale * scale * scale; // decay faster
   int color = fb.scalePixel(data->color, scale);
   data->step = (data->step + 1) % numSteps;

   fb.fillScreen(color);
   fb.show();
}



void GeoGrow::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->end = max(fb.width, fb.height) / 2 * 1.5; // 1.5 is arbitrary to make it bleed off edge
   data->sizeStep = data->end / 30.0; // target one growth per second
}

void GeoGrow::adjustParam(int step) {
   data->brightness = (data->brightness + step + numBrightSteps) % numBrightSteps;
   if (!data->brightness) {
      data->mode = (data->mode + step + 2) % 2;
   }
}

void GeoGrow::drawOnBeatSync(FrameTimingInfo *frameTiming) {
}

void GeoGrow::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   // if we just finished a pass, generate new random parameters for next pass
   if (!data->current.size) {
      int limit = 0xFF >> (2 * (numBrightSteps - 1 - data->brightness));
      data->current.color = fb.randomColor(limit);
      data->current.numSides = random(3, 9);
      data->current.rotation = M_PI / 128.0 * random(256);
      data->current.cx = fb.width / 2;
      data->current.cy = fb.height / 2;
      if (data->mode == 1) {
         int xOfs = fb.width / 2;
         int yOfs = fb.height / 2;
         data->current.cx += random(xOfs) - xOfs / 2;
         data->current.cy += random(yOfs) - yOfs / 2;
      }
   }

   // compute this pass
   data->current.size += data->sizeStep;
   addToHistory();
   drawFromHistory();

   if (data->current.size >= data->end) {
      data->current.size = 0;
   }

   fb.showWithLimit();
}

void GeoGrow::addToHistory() {
   Step *step = data->historyRingBuf + data->ringIndex;
   int limit = ARRAYSIZE(data->historyRingBuf);

   // overwrite next entry and make it active
   step->active = true;
   step->color = data->current.color;
   step->numSides = data->current.numSides;
   step->cx = data->current.cx;
   step->cy = data->current.cy;
   step->rotation = data->current.rotation;
   step->size = data->current.size;

   // fade remaining steps
   for (int i = 0; i < limit; i++) {
      if (i == data->ringIndex) continue;
      step = data->historyRingBuf + i;
      step->color = fb.scalePixel(step->color, 0.7);
   }   

   // advance ring pointer
   data->ringIndex = (data->ringIndex + 1) % limit;
}

void GeoGrow::drawFromHistory() {
   int limit = ARRAYSIZE(data->historyRingBuf);
   for (int i = 0; i < limit; i++) {
      Step *step = data->historyRingBuf + i;
      if (step->active) {
         fb.drawNGram(step->cx, step->cy, step->numSides, step->size, step->rotation, step->color);
      }
   }   
}



void Simon::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->numSteps = 4; // XXX lg2 max(fb.width, fb.height)
}

void Simon::drawOnFrameSync(FrameTimingInfo *frameTiming) {
}

void Simon::drawOnBeatSync(FrameTimingInfo *frameTiming) {
   int size = max(fb.width, fb.height);
   int blockSize = size >> data->step;
   int blocks = size / blockSize;

   // Don't change all the blocks in a given iteration; change
   // approximately 3/4 and let some show through.
   // Crucially, in the first iteration (size == fb.width) we need to fill.
   unsigned numBlocks = blocks * blocks;
   unsigned numBlocksToChange = (numBlocks + 3) * 3 / 4; // +3 to round up

   for (int iy = 0; iy < blocks; iy++) {
      for (int ix = 0; ix < blocks; ix++) {
         bool change = random(numBlocks) < numBlocksToChange;
         if (change) {
            int x1 = ix * blockSize;
            int x2 = x1 + blockSize - 1;
            int y1 = iy * blockSize;
            int y2 = y1 + blockSize - 1;

            fb.fillRect(x1, y1, x2, y2, fb.randomPrimaryOrSecondary(0x20));
            --numBlocksToChange;
         }
         --numBlocks;
      }
   }

   data->step++;
   if (data->step == data->numSteps) {
      data->step = 0;
   }

   fb.showWithLimit();
}



void TranslucentSquares::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->square[0].color = 0xFF0000;
   data->square[1].color = 0x00FF00;
   data->square[2].color = 0x0000FF;

   adjustParam(0);
}

void TranslucentSquares::adjustParam(int step) {
   data->brightness = (data->brightness + step + numBrightSteps) % numBrightSteps;

   // randomize all the squares
   int size = random(minSize, maxSize + 1);
   for (int i = 0; i < numSquares; i++) {
      Square *s = data->square + i;
      s->x = random(fb.width);
      s->y = random(fb.height);
      do {
         s->dx =random(3) - 1;
         s->dy = random(3) - 1;
      } while (!s->dx && !s->dy);
      s->size = size;
   }
}

void TranslucentSquares::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   fb.clearScreen();
   for (int i = 0; i < numSquares; i++) {
      Square *s = data->square + i;
      float brightScale = 1.0 / (1 << 2 * (numBrightSteps - data->brightness - 1));
      int squareColor = fb.scalePixel(s->color, brightScale);

      // draw translucent via oring in bits
      for (int x = s->x; x < s->x + s->size; x++) {
         for (int y = s->y; y < s->y + s->size; y++) {
            int pixel = fb.getGridPixel(x, y);
            pixel |= squareColor;
            fb.setGridPixel(x, y, pixel);
         }
      }

      // move, and if offscreen, bounce
      s->x += s->dx;
      s->y += s->dy;

      if (s->x < 0) {
         s->x = 0;
         s->dx = 1;
         s->dy = random(3) - 1;
      } else if (s->x + s->size >= fb.width) {
         s->x = fb.width - s->size;
         s->dx = -1;
         s->dy = random(3) - 1;
      }

      if (s->y < 0) {
         s->y = 0;
         s->dy = 1;
         s->dx = random(3) - 1;
      } else if (s->y + s->size >= fb.height) {
         s->y = fb.height - s->size;
         s->dy = -1;
         s->dx = random(3) - 1;
      }
   }
   fb.showWithLimit();
}



void SwirlRoutine::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->cx = fb.width / 2;
   data->cy = fb.width / 2;
}

void SwirlRoutine::adjustParam(int step) {
   int delta = millis() - data->lastClick;
   int delta2 = data->lastClick - data->olderClick;
   if (delta > 100 && delta < 300 && delta2 > 100 && delta2 < 300) {
      // triple click: start wandering around
      data->dcx = random(5) - 2;
      data->dcy = random(5) - 2;
      data->brightness = (data->brightness - 2 + numBrightSteps) % numBrightSteps;
   } else if (delta > 100 && delta < 300) {
      // double click: move center to new location
      data->cx = random(fb.width);
      data->cy = random(fb.height);
      data->dcx = 0;
      data->dcy = 0;
      data->brightness = (data->brightness - 1 + numBrightSteps) % numBrightSteps;
   } else {
     // single click: moderate brightness
      data->brightness = (data->brightness + step + numBrightSteps) % numBrightSteps;
   }

   data->olderClick = data->lastClick;
   data->lastClick = millis();
}

void SwirlRoutine::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   static const int swirlColors[7] = {
      0x600000, // red
      0x502000, // orange
      0x404000, // yellow
      0x006000, // green
      0x004040, // cyan
      0x000060, // blue
      0x400040, // purple
   };
   float brightScale = 1.0 / (1 << 2 * (numBrightSteps - data->brightness - 1));

   // draw
   for (int x = 0; x < fb.width; x++) {
      for (int y = 0; y < fb.height; y++) {
         float dx = x - data->cx + 0.5;
         float dy = y - data->cy + 0.5;

         // calculate angle, with offset
         // and offset range from [-pi, pi] to [0, 2 * pi]
         float angle = atan2(dy, dx);
         angle += data->angleOffset + M_PI;
         if (angle > 2 * M_PI) {
            angle -= 2 * M_PI;
         }

         // bin by number of colors
         float scale = angle / (2 * M_PI); // on scale of 0 to 1 now
         int bin = scale * ARRAYSIZE(swirlColors);

         int color = fb.scalePixel(swirlColors[bin], brightScale);
         fb.setGridPixel(x, y, color);
      }
   }
   fb.showWithLimit();

   // rotate; angle depends on beat-match speed
   float speedFactor = (frameTiming->frameLength * 15.0 / frameTiming->beatLength) / 6.0;
   data->angleOffset += 2 * M_PI / numSteps * speedFactor; // 2*PI/steps is angle of each step
   if (data->angleOffset > 2 * M_PI) {
      data->angleOffset -= 2 * M_PI;
   }

   // and maybe move center
   data->cx += data->dcx / 12.0;
   data->cy += data->dcy / 12.0;
   if (data->cx < 0 || data->cx >= fb.width) {
      data->dcx *= -1;
   }
   if (data->cy < 0 || data->cy >= fb.height) {
      data->dcy *= -1;
   }
}



void SnakeRoutine::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->lastClick = millis() - 200;
   adjustParam(0);
}

void SnakeRoutine::adjustParam(int step) {
   // if this was a double-click, clear the screen
   int delta = millis() - data->lastClick;
   if (delta > 100 && delta < 300) {
      fb.clearScreen();
   }
   data->lastClick = millis();

   // randomize parameters to start new snake
   data->x = random(fb.width);
   data->y = random(fb.height);
   newDirection();
   int oldColor = data->color;
   do {
      data->color = fb.randomPrimaryOrSecondary(0x10);
   } while (fb.colorHasSameComponents(data->color, oldColor));
}

void SnakeRoutine::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   // brighten pixel
   int pixel = fb.getGridPixel(data->x, data->y);
   pixel = fb.addPixelComponents(pixel, data->color);
   fb.setGridPixel(data->x, data->y, pixel);
   fb.showWithLimit();

   // move
   switch (data->direction) {
      case RIGHT: data->x += 1; break;
      case UP: data->y += 1; break;
      case LEFT: data->x -= 1; break;
      case DOWN: data->y -= 1; break;
   }

   // keep onscreen
   if (data->x < 0) {
      data->x = 0;
      while (data->direction == LEFT) newDirection();
   }
   if (data->x >= fb.width) {
      data->x = fb.width - 1;
      while (data->direction == RIGHT) newDirection();
   }
   if (data->y < 0) {
      data->y = 0;
      while (data->direction == DOWN) newDirection();
   }
   if (data->y >= fb.height) {
      data->y = fb.height - 1;
      while (data->direction == UP) newDirection();
   }

   // randomize direction once in a while
   // (more likely at edge of screen, to avoid getting stuck)
   bool atEdge = (data->x == 0 || data->x == fb.width - 1
               || data->y == 0 || data->y == fb.height - 1);
   int likely = atEdge ? fb.width / 3 : fb.width * 2;
   if (!random(likely)) {
      newDirection();
   }
}

void SnakeRoutine::newDirection() {
   // choose new direction randomly, but no u-turns
   int oldDirection = data->direction;
   do {
      data->direction = (Direction) random(4);
   } while (((data->direction + 2) % 4) == oldDirection);
}



void DripRoutine::begin(void *stateBuf) {
   static_assert(sizeof(Data) < ROUTINE_STATEBUF_SIZE, "buffer overflow");
   data = (Data *) stateBuf;
   memset(data, 0, sizeof *data);

   data->trailLength = 4;
   fb.clearScreen();
}

void DripRoutine::adjustParam(int step) {
   data->trailLength++;
   if (data->trailLength > maxTrailLength) {
      data->trailLength = minTrailLength;
   }
}

void DripRoutine::drawOnBeatSync(FrameTimingInfo *frameTiming) {
   // add a bright trail on every beat
   addPoint(0xFF);
}

void DripRoutine::drawOnFrameSync(FrameTimingInfo *frameTiming) {
   int i;

   // existing points: draw, then drip down
   for (i = 0; i < maxTrails; i++) {
      PointTrail *trail = data->trails + i;
      if (!trail->active) continue;

      int color = trail->color;
      int y;
      for (y = trail->y; y < trail->y + data->trailLength; y++) {
         fb.setGridPixel(trail->x, y, color);
         color = fb.scalePixel(color, 0.8);
      }
      // one extra draw to erase behind trail
      fb.setGridPixel(trail->x, y, 0);

      // the enemy's gate is down
      trail->y--;

      // check for expire
      if (trail->y < -data->trailLength) {
         trail->active = false;
      }
   }

   fb.showWithLimit();

   // consolidate point array, keeping in order
   PointTrail *keepers = data->trails;
   for (PointTrail *trail = data->trails; trail < data->trails + maxTrails; trail++) {
      if (trail->active) {
         if (keepers != trail) {
            *keepers = *trail;
         }
         keepers++;
      }
   }

   // check for add of new point
   if (!random(fb.width / 2)) {
      addPoint(0x40);
   }
}

void DripRoutine::addPoint(int colorLimit) {
   int numActive = 0;
   for (int i = 0; i < maxTrails; i++) {
      if (data->trails[i].active) {
         numActive++;
      }
   }
   if (numActive < maxTrails) {
      PointTrail *trail = data->trails + numActive;
      trail->x = random(fb.width);
      trail->y = fb.height;
      trail->color = fb.randomPrimaryOrSecondary(colorLimit);
      trail->active = true;
   }
}
