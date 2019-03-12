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

//TODO:  get rid of sx,sy, this should be part of the source data.
//TODO:  have a generic drawBitmap that copies only a portion of the bitmap

void drawBitmap(int x, int y, int sx, int sy, prog_uint16_t* data)
{
	CS_LOW;
	
	if(!setCompleteAreaOnly(x, y, x+sx-1, y+sy-1))
	{
		// bitmap not visible or partially visible  TODO: clip bitmap drawing
		return;
	}
	
	RS_HIGH;
	
	uint32_t tcmax = (sx*sy);
	for (int tc=0; tc<tcmax; tc++)
	{
		uint16_t col = LCDBEE_READPROGMEM_UINT16(&data[tc]);

		WRITE_DB16(col);
		WR_LOW_PULSE;
	}
	
	CS_HIGH;
}

void drawBitmap(int x, int y, int sx, int sy, prog_uint16_t* data, int scale)
{
	if (scale==1)
	{
		drawBitmap(x,y,sx,sy,data);
		return;
	}

	CS_LOW;
	for (int tx=0,txscaled=x; tx<sx; tx++,txscaled+=scale)
	{
		if(!setCompleteAreaOnly(txscaled, y, txscaled+scale-1, y + (sy*scale) - 1 ))
			continue;
			
		RS_HIGH;

		if(scale == 2)
		{
			for (int ty=0; ty<sy; ty++)
			{
				uint16_t col=LCDBEE_READPROGMEM_UINT16(&data[(ty*sx)+tx]);
			
				WRITE_DB16(col);
				WR_LOW_PULSE;
				WR_LOW_PULSE;
				WR_LOW_PULSE;
				WR_LOW_PULSE;
			}
		}
		else
		{
			for (int ty=0; ty<sy; ty++)
			{
				uint16_t col=LCDBEE_READPROGMEM_UINT16(&data[(ty*sx)+tx]);
			
				WRITE_DB16(col);
				uint8_t pixels = scale*scale;
				while(pixels--)
					WR_LOW_PULSE;
			}
		}
	}
	CS_HIGH;
}

void drawBitmap(int x, int y, int sx, int sy, prog_uint16_t* data, int deg, int rox, int roy)
{
	double radian = deg*0.0175;  
	double cos_radian = cos(radian);
	double sin_radian = sin(radian);
	
	if (deg==0)
		drawBitmap(x, y, sx, sy, data);
	else
	{
		CS_LOW;
		for (int ty=0; ty<sy; ty++)
		{
			for (int tx=0; tx<sx; tx++)
			{
				uint16_t color = LCDBEE_READPROGMEM_UINT16(&data[(ty*sx)+tx]);

				int newx = x+rox+(((tx-rox)*cos_radian)-((ty-roy)*sin_radian));
				int newy = y+roy+(((ty-roy)*cos_radian)+((tx-rox)*sin_radian));

				setPixel565(newx, newy, color);
			}
		}
		
		CS_HIGH;
	}
}

}