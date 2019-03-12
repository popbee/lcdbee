/*
  BeeLcd - Arduino open source CPP library support for small color LCD Boards.
  
  http://code.google.com/p/lcdbee/
  
  Copyright (c)2011 Bernard Poulin. All rights reserved 

------------------------------------------------------------------------------
  
  LcdBee.h - public header

  Optimized for speed, rich features & optimal foot print. **automatically 
  includes only what you need**

  Code inspired by Henning Karlsen's LcdBee_ILI935_16Bits_Landscape_Graph16 
  library ( http://www.henningkarlsen.com/electronics ) for which it's basic 
  functionality was in turn based on demo code provided by ITead Studio.
 
  Credits:
  
  CK/ITDB02_Graph16 library - Copyright (C)2010-2011 Henning Karlsen
  ITead studio demo code - http://iteadstudio.com/
  Xiaolin Wu's fast anti-aliasing line algorithm 
  Abrash, Michael articles with integer-only computations
  Bresenham's fast line algorithm
 
------------------------------------------------------------------------------
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef LCDBEE_H
#define LCDBEE_H

#include "WProgram.h"

#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

#include "core_conf.hpp"

#include "colors.hpp"
#define LCDBEE_RGB565(rgb) ((((rgb) >> 8) & 0xF800) | (((rgb) >> 5) & 0x07E0) | ((rgb)>>3 & 0xff))

namespace LcdBee
{

enum AlphaMethod
{
#ifdef LCDBEE_SUPPORT_READING_BACK
      ALPHA_MULTIPLY = 1,
      SINGLE_COLOR_SHAPE = 2,
#endif
      BACKGROUND_COLOR_MIX = 0
};

#ifdef LCDBEE_ORIENTATION_DYNAMIC
enum Orientation
{
      PORTRAIT = 0,
      LANDSCAPE = 1,
      PORTRAIT2 = 2,	// unsupported for now
      LANDSCAPE2 = 3	// unsupported for now
};
#endif

// text horizontal alignment
#define LEFT	0
#define CENTER	1
#define RIGHT	2

// text vertical alignment
#define TOP		0
#define MIDDLE	1
#define BOTTOM	2

//------------------------------------------------------------------------------------------
// PUBLIC API
//------------------------------------------------------------------------------------------

		/** perform a reset cycle on the LCD and setup parameters */
		void lcdReset();

#ifdef LCDBEE_ORIENTATION_DYNAMIC
		void setOrientation(Orientation orient);
#endif
		
		/** set foreground color */
		void setColor(uint8_t r, uint8_t g, uint8_t b);

		/** set foreground color as a 24 bit R-G-B value. (e.g. 0x00FF21) */
		void setColor(uint32_t rgb);

		/** set foreground color as a special 16 bit 565 R-G-B value encoding */
		void setColor565(uint16_t rgb565);

		/** set background color as a special 16 bit 565 R-G-B value encoding */
		void setBackColor565(uint16_t rgb565);

		/** set background color */
		void setBackColor(uint8_t r, uint8_t g, uint8_t b);
		
		/** set background color as a 24 bit R-G-B value. (e.g. 0x00FF21) */
		void setBackColor(uint32_t rgb);
		
		/** erase screen with background color */
		void clearScreen();	
		/** fill screen with a single foreground color */
		void fillScreen();

		void setPixel(int x, int y);
		
#ifdef LCDBEE_SUPPORT_READING_BACK
		/** requires RD line to be connected 
		 * return a 16 bit 565 RGB value
		 */
		uint16_t getPixel565(int x, int y);
		//TODO: do a getPixel() to get 24 bit color
#endif

		//TODO: do a lineTo moveTo smoothLineTo
		void line(int x1, int y1, int x2, int y2);
		void smoothLine(double fx1, double fy1, double fx2, double fy2);

		void rect(int x1, int y1, int x2, int y2);
		void roundRect(int x1, int y1, int x2, int y2);
		void circle(int x, int y, int radius);
		
		void fillRect(int x1, int y1, int x2, int y2);
		void fillRoundRect(int x1, int y1, int x2, int y2);
		void fillCircle(int x, int y, int radius);

		boolean textFont(PROGMEM uint8_t* fontdata);
		void textAlign(uint8_t horizontalAlign, uint8_t verticalAlign=TOP);
		void text(char* text, int x, int y);
		void text(int v, int x, int y);
		void text(float v, int x, int y);
		void text(double v, int x, int y);
		int textWidth(char* text);
		
		void drawBitmap(int x, int y, int sx, int sy, prog_uint16_t* data);
		void drawBitmap(int x, int y, int sx, int sy, prog_uint16_t* data, int scale);
		void drawBitmap(int x, int y, int sx, int sy, prog_uint16_t* data, int deg, int rox, int roy);

		/** all drawing operations will not exceed the boundaries of this rectangle 
		 *  Coordinates are inclusive.  (e.g. operations will affect pixels at x2 and y2)
		 */
		void setClipRect(int x1, int y1, int x2, int y2);
		
		/**
		 * remove clipping rectangle
		 */
		void resetClipRect();
	
		/** miscelaneous option to control how the alpha is blended */
		void setAlphaMethod(AlphaMethod alphaMethod);
		
		// legacy text
		void setFont(uint8_t* font);
		void print(char *st, int x, int y);
		void print(char *st, int x, int y, int degrees);
		void printNumI(long num, int x, int y); // TODO: add print Hex 
		void printNumF(double num, byte nb_decimals, int x, int y);
}

using namespace LcdBee;

#endif // LCDBEE_H
