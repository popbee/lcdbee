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

#define ALPHA_CACHE_BITS	5
#define ALPHA_MAX		((1<<ALPHA_CACHE_BITS)-1)

uint16_t baseColorCache;
uint16_t baseBackColorCache;
uint32_t alpha4MultCacheBitMap = 0;
uint16_t alpha4MultCache[ALPHA_MAX-1]; // we do not bother caching alpha = 31 (nor alpha = 0 btw)

void setPixel565Alpha8(int x, int y, uint16_t color, uint8_t alpha)
{
	alpha >>= (8-ALPHA_CACHE_BITS);
	
	if(alpha == 0)
	{
		// transparent pixel: do nothing
		return;
	}
		
	if(alpha == ALPHA_MAX)
	{
		setPixel565(x,y,color);
		return;
	}
		
	uint16_t premultcolor;
	
	if(baseColorCache != color)
	{
		// new color. reset cache
		baseColorCache = color;
		alpha4MultCacheBitMap = 0;
	}
	
	uint32_t alpha_mask = ((uint32_t)1) << alpha;
	
	if(alpha4MultCacheBitMap & alpha_mask)
	{
		premultcolor = alpha4MultCache[alpha-1];
	}
	else
	{
		// color not in cache, premultiply color
		premultcolor =
				((( (color >> 11) 			* alpha / ALPHA_MAX ) & 0x1F) << 11) |
				((( ((color >> 5) & 0x3F)	* alpha / ALPHA_MAX ) & 0x3F) << 5) |
				((  ((color 	  & 0x1F) 	* alpha / ALPHA_MAX ) & 0x1F));
	
		if(alpha_method == BACKGROUND_COLOR_MIX && bcolor565 != 0)
		{
			premultcolor +=
					((( (bcolor565 >> 11) 		* alpha / ALPHA_MAX ) & 0x1F) << 11) |
					((( ((bcolor565 >> 5) & 0x3F)	* alpha / ALPHA_MAX ) & 0x3F) << 5) |
					((  ((bcolor565 	  & 0x1F) 	* alpha / ALPHA_MAX ) & 0x1F));
		}
	
	
		// store in cache
		alpha4MultCacheBitMap |= alpha_mask;
		alpha4MultCache[alpha-1] = premultcolor;
	}
	
	switch(alpha_method)
	{	
		case BACKGROUND_COLOR_MIX:    	// no pixel reading. Uses background color as matte color.
		{
			// background color is pre-baked into the premult cache in this mode
			break;
		}
#ifdef LCDBEE_SUPPORT_READING_BACK		
		case ALPHA_MULTIPLY:		// "normal" alpha blending. Appears to be on a different layer.
		{
			// read the pixel color at x,y
			color = readPixel565(x,y);
			if(color != 0)
			{
				alpha = ALPHA_MAX - alpha;
				premultcolor +=
							((( (color >> 11) 			* alpha / ALPHA_MAX ) & 0x1F) << 11) |
							((( ((color >> 5) & 0x3F)	* alpha / ALPHA_MAX ) & 0x3F) << 5) |
							((  ((color 	  & 0x1F) 	* alpha / ALPHA_MAX ) & 0x1F));
			}
			break;
		}
		case SINGLE_COLOR_SHAPE: // works with black background and 
		{
					 // drawing with shades of the same color 
					 // to be combined to look like it is on the same "layer".
			color = readPixel565(x,y);
			if(premultcolor < color)
			{
				premultcolor = color;
			}
			break;
		}
#endif
	}
	
	setPixel565(x,y,premultcolor);
}

void setAlphaMethod(AlphaMethod alphaMethod)
{
	alpha_method = alphaMethod;
}

}