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

void rect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	CS_LOW;
	drawHLine565(x1, x2, y1, fcolor565);
	drawHLine565(x1, x2, y2, fcolor565);
	drawVLine565(x1, y1, y2, fcolor565);
	drawVLine565(x2, y1, y2, fcolor565);
	CS_HIGH;
}

void circle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	uint16_t color = fcolor565;
	
	CS_LOW;
	setPixel565(x, y + radius, color);
	setPixel565(x, y - radius, color);
	setPixel565(x + radius, y, color);
	setPixel565(x - radius, y, color);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;
		setPixel565(x + x1, y + y1, color);
		setPixel565(x - x1, y + y1, color);
		setPixel565(x + x1, y - y1, color);
		setPixel565(x - x1, y - y1, color);
		setPixel565(x + y1, y + x1, color);
		setPixel565(x - y1, y + x1, color);
		setPixel565(x + y1, y - x1, color);
		setPixel565(x - y1, y - x1, color);
	}
	CS_HIGH;
}

void fillRect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	CS_LOW;
	fillRect565(x1,y1,x2,y2, fcolor565);
	CS_HIGH;
}


void roundRect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}
	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		CS_LOW;
		setPixel565(x1+1,y1+1, fcolor565);
		setPixel565(x2-1,y1+1, fcolor565);
		setPixel565(x1+1,y2-1, fcolor565);
		setPixel565(x2-1,y2-1, fcolor565);
		drawHLine565(x1+2, x2-2, y1, fcolor565);
		drawHLine565(x1+2, x2-2, y2, fcolor565);
		drawVLine565(x1, y1+2, y2-2, fcolor565);
		drawVLine565(x2, y1+2, y2-2, fcolor565);
		CS_HIGH;
	}
}


void fillCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
	
	CS_LOW;
	setPixel565(x, y + radius, fcolor565);
	setPixel565(x, y - radius, fcolor565);
	drawHLine565(x - radius, x + radius, y, fcolor565);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;
		
		drawHLine565(x - x1, x + x1, y + y1, fcolor565);
		drawHLine565(x - x1, x + x1, y - y1, fcolor565);
		drawHLine565(x - y1, x + y1, y + x1, fcolor565);
		drawHLine565(x - y1, x + y1, y - x1, fcolor565);
	}
	CS_HIGH;
}

void fillRoundRect(int x1, int y1, int x2, int y2)
{
	int tmp;

	if (x1>x2)
	{
		tmp=x1;
		x1=x2;
		x2=tmp;
	}

	if (y1>y2)
	{
		tmp=y1;
		y1=y2;
		y2=tmp;
	}

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		CS_LOW;
		fillRect565(x1+1, y1+1, x2-1, y2-1, fcolor565); // middle
		drawHLine565(x1+2, x2-2, y1, fcolor565);
		drawHLine565(x1+2, x2-2, y2, fcolor565);
		drawVLine565(x1, y1+2, y2-2, fcolor565);
		drawVLine565(x2, y1+2, y2-2, fcolor565);
		CS_HIGH;
	}
}



}