#pragma once

#include "esphome/core/color.h"
#include "esphome/components/display/display.h"

class GraphicUtils {
  public:

    static void triangle(Display *display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2,
        uint16_t y2, Color color = display::COLOR_ON) {
      display->line(x0, y0, x1, y1, color);
      display->line(x1, y1, x2, y2, color);
      display->line(x2, y2, x0, y0, color);
    }

    static void filled_triangle(Display* display, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2,
        uint16_t y2, Color color = display::COLOR_ON) {
      triangle(display, x0, y0, x1, y1, x2, y2, color);
      int16_t a, b, y, last;
      if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
      }
      if (y1 > y2) {
        std::swap(y2, y1);
        std::swap(x2, x1);
      }
      if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
      }
      if (y0 == y2) {
        a = b = x0;
        if (x1 < a) {
          a = x1;
        } else if (x1 > b) {
          b = x1;
        }
        if (x2 < a) {
          a = x2;
        } else if (x2 > b) {
          b = x2;
        }
        display->horizontal_line(a, y0, b - a + 1, color);
        return;
      }
      int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
        dx12 = x2 - x1, dy12 = y2 - y1;
      int32_t sa = 0, sb = 0;

      if (y1 == y2) {
        last = y1;
      } else {
        last = y1 - 1;
      }

      for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) {
          std::swap(a, b);
        }
        display->horizontal_line(a, y, b - a + 1, color);
      }

      sa = (int32_t)dx12 * (y - y1);
      sb = (int32_t)dx02 * (y - y0);
      for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) {
          std::swap(a, b);
        }
        display->horizontal_line(a, y, b - a + 1, color);
      }
    }

    static void arc(Display *display, uint16_t x, uint16_t y, uint16_t r, uint16_t ir,
        uint16_t startAngle, uint16_t endAngle, Color color) {
      if (endAngle   > 360)   endAngle = 360;
  if (startAngle > 360) startAngle = 360;
  if (_vpOoB || startAngle == endAngle) return;
  if (r < ir) transpose(r, ir);  // Required that r > ir
  if (r <= 0 || ir < 0) return;  // Invalid r, ir can be zero (circle sector)

  if (endAngle < startAngle) {
    // Arc sweeps through 6 o'clock so draw in two parts
    if (startAngle < 360) drawArc(x, y, r, ir, startAngle, 360, fg_color, bg_color);
    if (endAngle == 0) return;
    startAngle = 0;
  }
  inTransaction = true;

  int32_t xs = 0;        // x start position for quadrant scan
  uint8_t alpha = 0;     // alpha value for blending pixels

  uint32_t r2 = r * r;   // Outer arc radius^2
  uint32_t r1 = r * r;   // Outer AA radius^2
  int16_t w  = r - ir;   // Width of arc (r - ir + 1)
  uint32_t r3 = ir * ir; // Inner arc radius^2
  uint32_t r4 = ir * ir; // Inner AA radius^2

  //     1 | 2
  //    ---¦---    Arc quadrant index
  //     0 | 3
  // Fixed point U16.16 slope table for arc start/end in each quadrant
  uint32_t startSlope[4] = {0, 0, 0xFFFFFFFF, 0};
  uint32_t   endSlope[4] = {0, 0xFFFFFFFF, 0, 0};

  // Ensure maximum U16.16 slope of arc ends is ~ 0x8000 0000
  constexpr float minDivisor = 1.0f/0x8000;

  // Fill in start slope table and empty quadrants
  float fabscos = fabsf(cosf(startAngle * deg2rad));
  float fabssin = fabsf(sinf(startAngle * deg2rad));

  // U16.16 slope of arc start
  uint32_t slope = (fabscos / (fabssin + minDivisor)) * (float)(1UL<<16);

  // Update slope table, add slope for arc start
  if (startAngle <= 90) {
    startSlope[0] =  slope;
  }
  else if (startAngle <= 180) {
    startSlope[1] =  slope;
  }
  else if (startAngle <= 270) {
    startSlope[1] = 0xFFFFFFFF;
    startSlope[2] = slope;
  }
  else {
    startSlope[1] = 0xFFFFFFFF;
    startSlope[2] =  0;
    startSlope[3] = slope;
  }

  // Fill in end slope table and empty quadrants
  fabscos  = fabsf(cosf(endAngle * deg2rad));
  fabssin  = fabsf(sinf(endAngle * deg2rad));

  // U16.16 slope of arc end
  slope   = (uint32_t)((fabscos/(fabssin + minDivisor)) * (float)(1UL<<16));

  // Work out which quadrants will need to be drawn and add slope for arc end
  if (endAngle <= 90) {
    endSlope[0] = slope;
    endSlope[1] =  0;
    startSlope[2] =  0;
  }
  else if (endAngle <= 180) {
    endSlope[1] = slope;
    startSlope[2] =  0;
  }
  else if (endAngle <= 270) {
    endSlope[2] =  slope;
  }
  else {
    endSlope[3] =  slope;
  }

  // Scan quadrant
  for (int32_t cy = r - 1; cy > 0; cy--)
  {
    uint32_t len[4] = { 0,  0,  0,  0}; // Pixel run length
    int32_t  xst[4] = {-1, -1, -1, -1}; // Pixel run x start
    uint32_t dy2 = (r - cy) * (r - cy);

    // Find and track arc zone start point
    while ((r - xs) * (r - xs) + dy2 >= r1) xs++;

    for (int32_t cx = xs; cx < r; cx++)
    {
      // Calculate radius^2
      uint32_t hyp = (r - cx) * (r - cx) + dy2;

      // If in outer zone calculate alpha
      if (hyp > r2) {
        alpha = ~sqrt_fraction(hyp); // Outer AA zone
      }
      // If within arc fill zone, get line start and lengths for each quadrant
      else if (hyp >= r3) {
        // Calculate U16.16 slope
        slope = ((r - cy) << 16)/(r - cx);
        if (slope <= startSlope[0] && slope >= endSlope[0]) { // slope hi -> lo
          xst[0] = cx; // Bottom left line end
          len[0]++;
        }
        if (slope >= startSlope[1] && slope <= endSlope[1]) { // slope lo -> hi
          xst[1] = cx; // Top left line end
          len[1]++;
        }
        if (slope <= startSlope[2] && slope >= endSlope[2]) { // slope hi -> lo
          xst[2] = cx; // Bottom right line start
          len[2]++;
        }
        if (slope <= endSlope[3] && slope >= startSlope[3]) { // slope lo -> hi
          xst[3] = cx; // Top right line start
          len[3]++;
        }
        continue; // Next x
      }
      else {
        if (hyp <= r4) break;  // Skip inner pixels
        alpha = sqrt_fraction(hyp); // Inner AA zone
      }

      if (alpha < 16) continue;  // Skip low alpha pixels

      // If background is read it must be done in each quadrant
      uint16_t pcol = fastBlend(alpha, fg_color, bg_color);
      // Check if an AA pixels need to be drawn
      slope = ((r - cy)<<16)/(r - cx);
      if (slope <= startSlope[0] && slope >= endSlope[0]) // BL
        drawPixel(x + cx - r, y - cy + r, pcol);
      if (slope >= startSlope[1] && slope <= endSlope[1]) // TL
        drawPixel(x + cx - r, y + cy - r, pcol);
      if (slope <= startSlope[2] && slope >= endSlope[2]) // TR
        drawPixel(x - cx + r, y + cy - r, pcol);
      if (slope <= endSlope[3] && slope >= startSlope[3]) // BR
        drawPixel(x - cx + r, y - cy + r, pcol);
    }
    // Add line in inner zone
    if (len[0]) drawFastHLine(x + xst[0] - len[0] + 1 - r, y - cy + r, len[0], fg_color); // BL
    if (len[1]) drawFastHLine(x + xst[1] - len[1] + 1 - r, y + cy - r, len[1], fg_color); // TL
    if (len[2]) drawFastHLine(x - xst[2] + r, y + cy - r, len[2], fg_color); // TR
    if (len[3]) drawFastHLine(x - xst[3] + r, y - cy + r, len[3], fg_color); // BR
    }
};