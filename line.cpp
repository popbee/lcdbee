/*
  BeeLcd - Arduino open source CPP library support for small color LCD Boards.
  
  Copyright (c)2011 Bernard Poulin. All rights reserved 
  Contact  bernard at acm.org
    
  Optimized for speed, rich features & optimal foot print. **automatically includes only what you need**

  Code inspired by Henning Karlsen's LcdBee_ILI935_16Bits_Landscape_Graph16 library 
  ( http://www.henningkarlsen.com/electronics ) for which it's basic 
  functionality was in turn based on demo code provided by ITead Studio.
 
  Credits:
  
  CK/ITDB02_Graph16 library - Copyright (C)2010-2011 Henning Karlsen. All right reserved
  ITead studio demo code - http://iteadstudio.com/
  Xiaolin Wu's fast anti-aliasing line algorithm 
  Abrash, Michael articles with integer-only computations
  Bresenham's fast line algorithm
 
----------------------------------------------------------------------------
  
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

#include "core.hpp"

namespace LcdBee
{

void line(int x1, int y1, int x2, int y2)
{
 	if (y1==y2)
	{
		if (x1>x2)
		{
			int tmp=x1;
			x1=x2;
			x2=tmp;
		}
		CS_LOW;
		drawHLine565(x1, x2, y1, fcolor565);
		CS_HIGH;
		return;
	}
	
	if (x1==x2)
	{
		if (y1>y2)
		{
			int tmp=y1;
			y1=y2;
			y2=tmp;
		}
		CS_LOW;
		drawVLine565(x1, y1, y2, fcolor565);
		CS_HIGH;
		return;
	}

	//----------------------------
	// Bresenham's line algorithm
	//----------------------------
	// TODO: discover if X-major line or Y-major line
	// setup "AM/ID(0/1)" fill direction to go in the same
	// "major axis" direction.
	// setPixel on first pixel, then just do a WR until 
	// we are still on the same pixel on the minor axis.
	// (fix back the fill direction)
	// NOTE: use traditional "setPixel" all the way if line 
	// is partially clipped.
	//----------------------------
	
	int dx = abs(x2-x1);
	int dy = abs(y2-y1); 
	
	int sx,sy; // step direction
	
	if(x1 < x2)
		sx = 1;
	else 
		sx = -1;
		
	if(y1 < y2)
		sy = 1;
	else
		sy = -1;
		
	int err = dx-dy;

	uint16_t color = fcolor565;

	CS_LOW;
	while(1)
	{
		setPixel565(x1,y1,color);
		if(x1 == x2 && y1 == y2)
			break;
			
		int e2 = 2*err;
		
		if(e2 > -dy)
		{		
			err = err - dy;
			x1 += sx;
		}
		
		if(e2 < dx)
		{
			err = err + dx;
			y1 += sy; 
		}
	}
	CS_HIGH;
}

/** draw an anti-aliased 1 pixel-width line from x1,y1 to x2,y2 inclusive 
 *
 * This is based on the Wu line antialiasing algorithm.
 *
 * Note: This logic is not meant to work nicely with lines greater than ~1K
 * (we would probably need to switch to a 32 bit error accumulator)
 */
 /*
void LcdBeeBase::drawSmoothLine(int x1, int y1, int x2, int y2)
{
	// Make sure the line runs top to bottom
	if (y1 > y2) 
	{
		int temp = y1; 
		y1 = y2; 
		y2 = temp;
		temp = x1; 
		x1 = x2; 
		x2 = temp;
	}

	uint16_t color = fcolor565;

	CS_LOW;

	int deltax = x2 - x1;
	int xdir;
	
	if (deltax >= 0) 
	{
		xdir = 1;
	} 
	else 
	{
		xdir = -1;
		deltax = -deltax; // make deltax positive
	}

	// Special-case horizontal, vertical, and diagonal lines, which
	// require no weighting because they go right through the center of
	// every pixel
	int deltay = y2 - y1;
	if (deltay == 0) 
	{
		// Horizontal line
		if(x1 < x2)
			drawHLine565(x1,x2,y1,color);
		else
			drawHLine565(x2,x1,y1,color);
		return;
	}

	if (deltax == 0) 
	{
		// Vertical line
		if(y1 < y2)
			drawVLine565(x1,y1,y2,color);
		else
			drawVLine565(x1,y2,y1,color);
		return;
	}

	if (deltax == deltay) 
	{
		// Diagonal line
		// Perfectly 45 degrees diagonal Wu lines do not do any anti-aliasing
		// so use a much faster logic
		
		while(deltax >= 0)
		{
			setPixel565(x1, y1, color);
			
			x1 += xdir;		
			y1++;
			
			deltax--;
		}
		return;
	}

	// Draw the initial and last pixels, which is always exactly intersected by
	// the line and so needs no weighting 
	setPixel565(x1, y1, color);
	setPixel565(x2, y2, color);

	if (deltay > deltax) 
	{
		// Y-major line
		uint16_t errorAcc = 0;  // initialize the line error accumulator to 0

		// calculate 16-bit fixed-point fractional part of a
		// pixel that X advances each time Y advances 1 pixel, truncating the
		// result so that we won't overrun the endpoint along the X axis
		uint16_t errorAdj = ((unsigned long) deltax << 16) / (unsigned long) deltay;

		// Draw all pixels other than the first and last
		while (--deltay)
		{
			uint16_t lastErrorAcc = errorAcc;   // remember currrent accumulated error
			errorAcc += errorAdj;      			// calculate error for next pixel
			
			if (errorAcc <= lastErrorAcc) 
			{
				// The error accumulator turned over, so advance the X coord
				x1 += xdir;
				x2 -= xdir;
			}

			y1++; // Y-major, so always advance Y
			y2--; // Y-major, so always advance Y
			
			// The most significant 8 bits of errorAcc give us the
			// intensity "weighting" for this pixel, and the complement of the
			// weighting for the paired pixel
			uint8_t weighting = errorAcc >> 8;
			setPixel565Alpha8(x1, y1, color, ~weighting);
			setPixel565Alpha8(x1 + xdir, y1, color, weighting);

			if(!--deltay)
				break;
			
			setPixel565Alpha8(x2, y2, color, ~weighting);
			setPixel565Alpha8(x2 - xdir, y2, color, weighting);
		}
	}
	else
	{
		// It's an X-major line
		uint16_t errorAcc = 0;  // initialize the line error accumulator to 0
		
		// calculate 16-bit fixed-point fractional part of a
		// pixel that Y advances each time X advances 1 pixel, truncating the
		// result to avoid overrunning the endpoint along the X axis
		uint16_t errorAdj = ((unsigned long) deltay << 16) / (unsigned long) deltax;

		// Draw all pixels other than the first and last
		while (--deltax) 
		{
			uint16_t lastErrorAcc = errorAcc;   	// remember currrent accumulated error
			errorAcc += errorAdj;      		// calculate error for next pixel
			
			if (errorAcc <= lastErrorAcc) 
			{
				// The error accumulator turned over, so advance the Y coord 
				y1++;
				y2--;
			}

			x1 += xdir; // X-major, so always advance X 
			x2 -= xdir;
			
			uint8_t weighting = errorAcc >> 8;
			setPixel565Alpha8(x1, y1, color, ~weighting);
			setPixel565Alpha8(x1, y1 + 1, color, weighting);
			
			if(!--deltax)
				break;		
			
			setPixel565Alpha8(x2, y2, color, ~weighting);
			setPixel565Alpha8(x2, y2 - 1, color, weighting);
		}
	}

	CS_HIGH;
}
*/

/** draw an anti-aliased 1 pixel-width line from fx1, fy1 to fx2, fy2. 
 *  This method supports sub-pixel accuracy. (e.g. fx1 can be 10.34 for example).
 * This is based on the Wu anti-aliasing technique with extra processing to allow
 * sub-pixel accuracy.  
 *
 * NOTE: This technique is not meant to work nicely with lines greater than ~1K
 * (we would probably need to switch to a 32 bit error accumulator)
 */

void smoothLine(double fx1, double fy1, double fx2, double fy2)
{
// NOTE: FRACTIONAL_BITS will use bits in an integer for "sub pixel" locations. 
#define FRACTIONAL_BITS	4

#define FRACTIONAL_SIZE	(1<<FRACTIONAL_BITS)
#define FRACTIONAL_MASK (FRACTIONAL_SIZE - 1)

	// TODO: make sure aligning to the center of the pixel makes sense with the rest of the algorithm!!!
	
	// Adjust to use the middle of the pixel as reference when fractional portion is 0. Also
	// converts to integers with a 'FRACTIONAL_BITS' portion.
	
//	int x1 = (int)(fx1 * FRACTIONAL_SIZE + ((FRACTIONAL_SIZE/2) + 0.5f));
//	int x2 = (int)(fx2 * FRACTIONAL_SIZE + ((FRACTIONAL_SIZE/2) + 0.5f));
//	int y1 = (int)(fy1 * FRACTIONAL_SIZE + ((FRACTIONAL_SIZE/2) + 0.5f));
//	int y2 = (int)(fy2 * FRACTIONAL_SIZE + ((FRACTIONAL_SIZE/2) + 0.5f));
	int x1 = (int)(fx1 * FRACTIONAL_SIZE + 0.5f);   // sub-pixel position "rounding"
	int x2 = (int)(fx2 * FRACTIONAL_SIZE + 0.5f);
	int y1 = (int)(fy1 * FRACTIONAL_SIZE + 0.5f);
	int y2 = (int)(fy2 * FRACTIONAL_SIZE + 0.5f);
	
	// Make sure the line runs top to bottom
	if (y1 > y2) 
	{
		int temp = y1; 
		y1 = y2; 
		y2 = temp;
		temp = x1; 
		x1 = x2; 
		x2 = temp;
	}

	int deltax = x2 - x1;
	int deltay = y2 - y1;

	uint8_t frac_x1 = (x1 & FRACTIONAL_MASK);
	uint8_t frac_x2 = (x2 & FRACTIONAL_MASK);
	uint8_t frac_y1 = (y1 & FRACTIONAL_MASK);
	uint8_t frac_y2 = (y2 & FRACTIONAL_MASK);

	// make x1,x2,y1,y2 return to pixel coordinates
	x1 >>= FRACTIONAL_BITS;
	x2 >>= FRACTIONAL_BITS;
	y1 >>= FRACTIONAL_BITS;
	y2 >>= FRACTIONAL_BITS;

	uint16_t color = fcolor565;

	CS_LOW;
	
	uint8_t weighting;
	
	
	if(deltay == 0 || deltax == 0)
	{
		if(deltay == 0 && deltax == 0)
		{
			// length of line to display is 0 (after rounding): do nothing
			return;
		}
		
		//deltay == 0  OR deltax == 0 -- appear to be a special case:
		//   draw a custom line in that case.
		return;
	}
	
	if(deltay == deltax)
	{
		// perfect diagonal line -- do we need to do something special here??
	}
	
	int xdir;
	
	if (deltax >= 0) 
	{
		xdir = 1;
	} 
	else 
	{
		xdir = -1;
		deltax = -deltax; // make deltax positive
	}

	if (deltay > deltax) 
	{
		// y-major line

		// calculate 16-bit fixed-point fractional part of a
		// pixel that X advances each time Y advances 1 pixel, truncating the
		// result so that we won't overrun the endpoint along the X axis
		uint16_t errorAdj = ((unsigned long) deltax << 16) / (unsigned long) deltay;
		
		//-------------------------------------------------------------------
		// Draw first pixel and compute initial error accumulator
		//-------------------------------------------------------------------
		// Unlike the other pixels, the line does not "cross" the entire pixel.
		// The line starts somewhere in the middle of the pixel, so the "intensity" 
		// needs to be reduced accordingly. Since this is a Y-major, we only look at
		// the fractional part of y1 as an additional "weighting". This is not accurate, but 
		// Wu lines are not more accurate either.

		uint16_t errorAcc = 0;
		
		if(frac_x1 < (FRACTIONAL_SIZE/2))  // TODO: this may depend on xdir !!!??  CHECK THAT LOGIC
		{
			// we are a little bit further away than the center of the pixel, so start higher:
			x1 - xdir;
			
			// at least half-a-pixel already accumulated 
			errorAcc = (1<<15);
		}
		
		errorAcc += ((uint16_t)frac_x1) << (16 - FRACTIONAL_BITS); // augment frac_y1 to 16 bits and add to the error
		
		// compute how much error accumulated at the middle Y point to 
		// align with the rest of the algorithm. The following modulates the errorAdj with the frac_y1 distance to
		// the middle Y point
		//
		// keep the following computation unsigned to be sure we do not overflow
		if(frac_y1 < (FRACTIONAL_SIZE/2))
			errorAcc += ((FRACTIONAL_SIZE/2) - frac_y1) * errorAdj / FRACTIONAL_SIZE;
		else
			errorAcc -= (frac_y1 - (FRACTIONAL_SIZE/2)) * errorAdj / FRACTIONAL_SIZE;
		
		weighting = (uint8_t)((errorAcc >> 8) * frac_y1 / FRACTIONAL_SIZE);

		setPixel565Alpha8(x1, y1, 0, ~weighting);
		setPixel565Alpha8(x1 + xdir, y1, 0, weighting);

		// compute the number of pixels between y1 and y2
		uint16_t deltay_pix = y2 - y1;

		// Draw all the pixels (excluding the first one)
		while (deltay_pix--)
		{
			uint16_t lastErrorAcc = errorAcc;   	// remember current accumulated error
			errorAcc += errorAdj;      		// calculate error for next pixel
			
			if (errorAcc <= lastErrorAcc) 
			{
				// The error accumulator turned over, so advance the X coord
				x1 += xdir;
			}

			y1++; // Y-major, so always advance Y

			// The most significant 8 bits of errorAcc give us the
			// intensity "weighting" for this pixel, and the complement of the
			// weighting for the paired pixel
			
			if(deltay_pix > 0)
			{
				// The most significant 8 bits of errorAcc give us the
				// intensity "weighting" for this pixel, and the complement of the
				// weighting for the paired pixel. 				
 				weighting = errorAcc >> 8;
			}
			else
			{
				// this is the last pixel
				
				// The weighting is multiplied by the (inverse of) 
				// the remaining length ratio to reduce the intensity further.
				weighting = (uint8_t)((errorAcc >> 8) * (FRACTIONAL_SIZE-frac_y2) / FRACTIONAL_SIZE);
 			}

			setPixel565Alpha8(x1, y1, color, ~weighting);
			setPixel565Alpha8(x1 + xdir, y1, color, weighting);
		}
	
	}
	else
	{
		// x-major line

		// calculate 16-bit fixed-point fractional part of a
		// pixel that Y advances each time X advances 1 pixel, truncating the
		// result to avoid overrunning the endpoint along the X axis
		uint16_t errorAdj = ((unsigned long) deltay << 16) / (unsigned long) deltax;

		
		//-------------------------------------------------------------------
		// Draw first pixel and compute initial error accumulator
		//-------------------------------------------------------------------
		// Unlike the other pixels, the line does not "cross" the entire pixel.
		// The line starts somewhere in the middle of the pixel, so the "intensity" 
		// needs to be reduced accordingly. Since this is a X-major, we only look at
		// the fractional part of x1 as an additional "weighting". This is not accurate, but 
		// Wu lines are not more accurate either.
	
		uint16_t errorAcc = ((uint16_t)frac_y1) << (16 - FRACTIONAL_BITS); // augment frac_y1 to 16 bits and add to the error
				
		// compute how much error accumulated at the middle X point to 
		// align with the rest of the algorithm. The following modulates the errorAdj with the frac_x1 distance to
		// the middle X point
		//
		// keep the following computation unsigned to be sure we do not overflow
		if(frac_x1 < (FRACTIONAL_SIZE/2))
		{
			uint16_t errorXGap = ((uint32_t)frac_x1) * errorAdj / FRACTIONAL_SIZE;
		
			if(xdir > 0)
				errorAcc -= errorXGap;
			else
				errorAcc += errorXGap;
		}
		else
		{
			uint16_t errorXGap = ((uint32_t)(FRACTIONAL_SIZE - frac_x1)) * errorAdj / FRACTIONAL_SIZE;

			if(xdir > 0)
				errorAcc += errorXGap;
			else
				errorAcc -= errorXGap;

			x1++;  // "round" the x1 position to the next pixel on the right
		}
		
		weighting = (uint8_t)((errorAcc >> 8) * frac_x1 / FRACTIONAL_SIZE);
			
		setPixel565Alpha8(x1, y1, color, ~weighting);
		setPixel565Alpha8(x1, y1 + 1, color, weighting);

		if(frac_x2 < (FRACTIONAL_SIZE/2))
		{
			// "round" the pixel position to the right
			x2++;
		}
		
		// compute the number of pixels between x1 and x2
		uint16_t deltax_pix = x2 - x1;
		
		if(deltax_pix < 0)
			deltax_pix = -deltax_pix;	

		// Draw all the pixels (excluding the first one)
		while (deltax_pix--) 
		{
			uint16_t lastErrorAcc = errorAcc;   	// remember currrent accumulated error
			errorAcc += errorAdj;      		// calculate error for next pixel
			
			if (errorAcc <= lastErrorAcc) 
			{
				// The error accumulator turned over, so advance the Y coord 
				y1++;
			}

			x1 += xdir; // X-major, so always advance X 
			
			// The most significant 8 bits of errorAcc give us the
			// intensity "weighting" for this pixel, and the complement of the
			// weighting for the paired pixel. 
			weighting = errorAcc >> 8;

			if(deltax_pix > 0)
			{
				setPixel565Alpha8(x1, y1, color, ~weighting);
				setPixel565Alpha8(x1, y1 + 1, color, weighting);
			}
			else
			{				
				// weightings is multiplied by the remaining length fraction to reduce the intensity further.	
				uint8_t frac_portion;
				
				if(frac_x2 < (FRACTIONAL_SIZE/2))
				{
					if(xdir > 0)
						frac_portion = (FRACTIONAL_SIZE/2) + frac_x2;
					else
						frac_portion = frac_x2;
				}
				else
				{
					if(xdir > 0)
						frac_portion = frac_x2 - (FRACTIONAL_SIZE/2);
					else
						frac_portion = (FRACTIONAL_SIZE/2) - frac_x2;
				}
				
				setPixel565Alpha8(x1, y1,     0, (uint8_t)(((uint16_t)(~weighting)) * frac_portion / FRACTIONAL_SIZE) );
				setPixel565Alpha8(x1, y1 + 1, 0, (uint8_t)(((uint16_t)( weighting)) * frac_portion / FRACTIONAL_SIZE) );	
			}			
		}
	}

	CS_HIGH;
}

}

