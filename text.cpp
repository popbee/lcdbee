/*
  LcdBee - Arduino open source CPP library support for small color LCD Boards.
  
  Copyright (c)2011 Bernard Poulin. All rights reserved 
  Contact  bernard at acm.org
 
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

/*
typedef struct _BFont
Header
------
uint8_t	'0xBF'		magic number for BFonts
uint8_t	type 		low nibble(0x0F) = type: '0x1' for 'BF1' type 0x2 for 'BF2' type, etc., 
					bit 4&5   (0x30) = bitsperpixel-1:  0x00=1 bit, 0x10=2 bits, 0x20=3 bits, 0x30=4 bits
					bit 6&7   (0xC0) = compression: 0x00=none, 0x40=B1RLE/B2RLE/B3RLE/B4RLE, 0x80=B2SBAL
uint8_t	height
uint8_t	width		// glyph width for monospaced 'BF1' fonts (not there for variable width BF2 fonts)
------
uint16_t glyphIndex[32]; // Lookup table for the position in "glyphs[]". This is a 5 bit hashtable.
						 // hash to index logic: (c * 31) & 0x1f  (c is an unsigned byte)
------
uint8_t glyphs[...]
} Bfont;


typedef struct _glyphBF1
{
	uint8_t code;		// 1-255 glyph code (e.g. ASCII code), 0 = last entry.
	uint8_t	xybitmapsize;	// b[7..4] = x_size-1, b[3..0] = y_size-1
	uint8_t	xybitmapoffset;	// b[7..4] = x_offset, b[3..0] = y_offset
	uint8_t bitmap[]	// 1 bit per pixel, MSBit first, no "line end padding". (unused bits in the last byte set to 0)
} GlyphBF1;

typedef struct _glyphBF2
{
	uint8_t code;		// 1-255 glyph code (e.g. ASCII code), 0 = last entry.
	uint8_t	xybitmapsize;	// b[7..4] = x_size-1, b[3..0] = y_size-1
	uint8_t	xybitmapoffset;	// b[7..4] = x_offset, b[3..0] = y_offset
	uint8_t	width;		// space accounted by the glyph before the next (0..255)
	uint8_t bitmap[]	// 1 bit per pixel, MSBit first, no "line end padding". (unused bits in the last byte set to 0)
} GlyphBF2;

typedef struct _glyphBF3
{
	uint8_t code;		// 1-255 glyph code (e.g. ASCII code), 0 = last entry.
	uint8_t	xbitmapsize;	// b[7..0] (1..256)
	uint8_t	ybitmapsize;	// b[6..0] (1..128)
	uint8_t	xbitmapoffset;	// b[7..0] (0..255) 
	int8_t	ybitmapoffset;	// b[7..0] (-128...127)
	uint8_t width;		// space accounted by the glyph
	
	uint8_t bitmap[]	// 1,2,3 or 4 bits per pixel, MSBit first, no "line end padding". (unused bits in the last byte set to 0)
} GlyphBF3;


*/

static uint8_t bfont_height;
static uint8_t* bfont_glyphIndex;
static uint8_t* bfont_glyphData;
static uint8_t bfont_type;  // 2=BF2, 3=BF3 ...
static uint8_t bfont_bitsPerPixel;

static uint8_t textHAlign = LEFT;
static uint8_t textVAlign = TOP;

static uint16_t mixedColors[4];

void textAlign(uint8_t horizontalAlign, uint8_t verticalAlign)
{
	textHAlign = horizontalAlign;
	textVAlign = verticalAlign;
}

void text(int v, int x, int y)
{
	char buf[15];
	itoa(v, buf, 10);
	text(buf, x , y);
}

void text(float v, int x, int y)
{
	char buf[25];
	snprintf(buf,25,"%g",v);
	text(buf, x , y);
}

void text(double v, int x, int y)
{
	char buf[25];
	snprintf(buf,25,"%f",v);
	text(buf, x , y);
}

boolean textFont(PROGMEM uint8_t* fontdata)
{
	bfont_glyphIndex = NULL;

	int magicNumber = LCDBEE_READPROGMEM_UINT8(fontdata);	
	if(magicNumber != (uint8_t)0xBF)
	{
		// invalidate font
		bfont_glyphIndex = NULL;
		return false; // not a valid bfont
	}
	
	fontdata++;
	
	bfont_type = LCDBEE_READPROGMEM_UINT8(fontdata);
	bfont_bitsPerPixel = ((bfont_type >> 4) & 0x3) + 1;
	bfont_type &= 0x0F;
	
	fontdata++;
	if(bfont_type != 0x2 && bfont_type != 0x3)
	{
		// invalidate font
		bfont_glyphIndex = NULL;
		return false; // unsupported type
	}
	
	bfont_height = LCDBEE_READPROGMEM_UINT8(fontdata);	
	fontdata++;

	bfont_glyphIndex = fontdata;
	fontdata += (32*2);

	bfont_glyphData = fontdata;
	
	return true;
}

static PROGMEM uint8_t* firstGlyphFromIndex(uint8_t code)
{
	int mult31 = ((int)code) * 31;
	int index = (mult31 ^ (mult31>>5)) & 0x1F;
	
	uint16_t glyph_offset = LCDBEE_READPROGMEM_UINT16(bfont_glyphIndex + (index*2));

	if(glyph_offset == 0xFFFF)
		return NULL;	// glyph not found
		
	return bfont_glyphData + glyph_offset;
}

static uint8_t* findGlyphBF2(uint8_t code)
{
	PROGMEM uint8_t* glyph = firstGlyphFromIndex(code);

	if(glyph == NULL)
		return NULL;
	
	// got the first glyph from the lookup table: Start scanning for a code match.

	uint8_t glyphcode = LCDBEE_READPROGMEM_UINT8(glyph);
	
	while(glyphcode != code)
	{	
		if(glyphcode == 0)
		{
			// glyph not found!
			//
			// TODO: display debugging error 
			return NULL;
		}
		
		// jump to next glyph
		uint8_t xybitmapsize = LCDBEE_READPROGMEM_UINT8(glyph+1);
		uint8_t x_size = 1+((xybitmapsize >> 4) & 0xF);
		uint8_t y_size = 1+(xybitmapsize & 0xF);
		uint8_t size_glyph = 4 + (((x_size * y_size) + 7 ) / 8); 
		
		glyph += size_glyph;
		
		// read next code
		glyphcode = LCDBEE_READPROGMEM_UINT8(glyph);
	}

	return glyph;
}

static uint8_t* findGlyphBF3(uint8_t code)
{
	PROGMEM uint8_t* glyph = firstGlyphFromIndex(code);

	if(glyph == NULL)
		return NULL;

	// got the first glyph from the lookup table: Start scanning for a code match.

	uint8_t glyphcode = LCDBEE_READPROGMEM_UINT8(glyph);
	
	while(glyphcode != code)
	{
		if(glyphcode == 0)
		{
			// glyph not found!
			//
			// TODO: display debugging error 
			return NULL;
		}
		
		// jump to next glyph
		int x_size = 1+LCDBEE_READPROGMEM_UINT8(glyph+1);
		int y_size = 1+LCDBEE_READPROGMEM_UINT8(glyph+2);
		int size_glyph = 6 + (((x_size * y_size * bfont_bitsPerPixel) + 7 ) / 8); 
		
		glyph += size_glyph;
		
		// read next code
		glyphcode = LCDBEE_READPROGMEM_UINT8(glyph);
	}

	return glyph;
}


static int computeGlyphWidthBF2(PROGMEM uint8_t* glyph)
{
	return LCDBEE_READPROGMEM_UINT8(glyph+3);
}

static int computeGlyphWidthBF3(PROGMEM uint8_t* glyph)
{
	return LCDBEE_READPROGMEM_UINT8(glyph+5);
}

static void blitzOneBitGlyph(PROGMEM uint8_t* glyphData, int size_full_bytes, uint8_t remaining_bits)
{
	while(size_full_bytes--)
	{
		uint8_t rasterByte = LCDBEE_READPROGMEM_UINT8(glyphData);

		uint8_t mask = 0x80;

		do  // will go through 0x80, 0x40, 0x20, 0x10...0x2,0x1  			
		{
			uint16_t color = (rasterByte & mask) ? fcolor565 : bcolor565;
			WRITE_DB16(color);
			WR_LOW_PULSE;
		} while(mask >>= 1);  // the last shift will yield 0x0 which will make it stop

		glyphData++;
	}

	if(remaining_bits > 0)
	{
		uint8_t rasterByte = LCDBEE_READPROGMEM_UINT8(glyphData);

		uint8_t mask = 0x80;

		do		
		{
			// mask will go through 0x80, 0x40, 0x20, 0x10...0x2,0x1
			uint16_t color = (rasterByte & mask) ? fcolor565 : bcolor565;
			WRITE_DB16(color);
			WR_LOW_PULSE;
			mask >>= 1;
			remaining_bits--;
		} while(remaining_bits > 0);
	}
}

static void blitzTwoBitGlyph(PROGMEM uint8_t* glyphData, int size_full_bytes, uint8_t remaining_bits)
{
	while(size_full_bytes--)
	{
		uint8_t rasterByte = LCDBEE_READPROGMEM_UINT8(glyphData);

		uint16_t color = mixedColors[(rasterByte >> 6)];
		
		WRITE_DB16(color);
		WR_LOW_PULSE;

		color = mixedColors[(rasterByte >> 4) & 0x3];
		
		WRITE_DB16(color);
		WR_LOW_PULSE;
		
		color = mixedColors[(rasterByte >> 2) & 0x3];
		
		WRITE_DB16(color);
		WR_LOW_PULSE;
		
		color = mixedColors[rasterByte & 0x3];
		
		WRITE_DB16(color);
		WR_LOW_PULSE;
		
		glyphData++;
	}

	if(remaining_bits == 0)
		return;

	uint8_t rasterByte = LCDBEE_READPROGMEM_UINT8(glyphData);

	uint16_t color = mixedColors[(rasterByte >> 6)];
	
	WRITE_DB16(color);
	WR_LOW_PULSE;

	if(remaining_bits == 2)
		return;

	color = mixedColors[(rasterByte >> 4) & 0x3];
	
	WRITE_DB16(color);
	WR_LOW_PULSE;
	
	if(remaining_bits == 4)
		return;

	color = mixedColors[(rasterByte >> 2) & 0x3];
	
	WRITE_DB16(color);
	WR_LOW_PULSE;
}


static int drawGlyphBF2(PROGMEM uint8_t* glyph, int x, int y)
{
	glyph++; // skip code
	
	uint8_t xybitmapsize = LCDBEE_READPROGMEM_UINT8(glyph);
	glyph++;
	
	uint8_t xybitmapoffset = LCDBEE_READPROGMEM_UINT8(glyph);
	glyph++;

	uint8_t width = LCDBEE_READPROGMEM_UINT8(glyph);
	glyph++;

	uint8_t x_size_minus1 = (xybitmapsize >> 4) & 0xF;
	uint8_t y_size_minus1 = (xybitmapsize & 0xF);

	// offset displacement
	int ox = x + ((xybitmapoffset >> 4) & 0xF);
	int oy = y + (xybitmapoffset & 0xF);
	
	CS_LOW;

	if(!setCompleteAreaOnly(ox,oy,ox+x_size_minus1,oy+y_size_minus1))
	{
		//TODO: logic to display partially cropped character
		return  x + width; 
	}		

	RS_HIGH;

	int size_pixels = (x_size_minus1+1) * (y_size_minus1+1);

	if(bfont_bitsPerPixel == 1)
	{
		int size_full_bytes = size_pixels / 8;	
		uint8_t remaining_bits = size_pixels & 7;
		
		blitzOneBitGlyph(glyph, size_full_bytes, remaining_bits);
	}
	else if(bfont_bitsPerPixel == 2) 
	{
		int size_bits = size_pixels * 2;
	
		int size_full_bytes = size_bits / 8;	
		uint8_t remaining_bits = size_bits & 7;
		
		blitzTwoBitGlyph(glyph, size_full_bytes, remaining_bits);
	}

	CS_HIGH;
	
	return x + width;
}

static int drawGlyphBF3(PROGMEM uint8_t* glyph, int x, int y)
{
	glyph++; // skip code
	
	uint8_t x_size_minus1 = LCDBEE_READPROGMEM_UINT8(glyph);
	glyph++;
	
	uint8_t y_size_minus1 = LCDBEE_READPROGMEM_UINT8(glyph);
	glyph++;
	
	// offset displacement
	int ox = x + (int8_t)LCDBEE_READPROGMEM_UINT8(glyph); // value is signed
	glyph++;

	int oy = y + LCDBEE_READPROGMEM_UINT8(glyph);	
	glyph++;
	
	uint8_t width = LCDBEE_READPROGMEM_UINT8(glyph);
	glyph++;
	
	CS_LOW;

	if(!setCompleteAreaOnly(ox,oy,ox+x_size_minus1,oy+y_size_minus1))
	{
		//TODO: logic to display partially cropped character
		return  x + width; 
	}		

	RS_HIGH;

	int size_pixels = (((int)x_size_minus1)+1) * (((int)y_size_minus1)+1);
	
	if(bfont_bitsPerPixel == 1)
	{
		int size_full_bytes = size_pixels / 8;	
		uint8_t remaining_bits = size_pixels & 7;
		
		blitzOneBitGlyph(glyph, size_full_bytes, remaining_bits);
	}
	else if(bfont_bitsPerPixel == 2) 
	{
		int size_bits = size_pixels * 2;

		int size_full_bytes = size_bits / 8;	
		uint8_t remaining_bits = size_bits & 7;
		
		blitzTwoBitGlyph(glyph, size_full_bytes, remaining_bits);
	}

	CS_HIGH;
	
	return x + width;
}

uint16_t mixColor(uint16_t color1, uint16_t color2, uint16_t alpha, int16_t alphamax)
{
	uint16_t ralpha = alphamax - alpha;

	uint16_t color =
			((( (color1 >> 11) 			* ralpha / alphamax ) & 0x1F) << 11) |
			((( ((color1 >> 5) & 0x3F)	* ralpha / alphamax ) & 0x3F) << 5) |
			((  ((color1       & 0x1F) 	* ralpha / alphamax ) & 0x1F));

	if(color2 != 0)
	{
		color +=
				((( (color2 >> 11) 			* alpha / alphamax ) & 0x1F) << 11) |
				((( ((color2 >> 5) & 0x3F)	* alpha / alphamax ) & 0x3F) << 5) |
				((  ((color2       & 0x1F) 	* alpha / alphamax ) & 0x1F));
	}
		
	return color;
}

void text(char* text, int x, int y)
{
	if(bfont_glyphIndex == NULL)
		return;

	switch(textHAlign)
	{
		default:
		case LEFT:
			break;
		case CENTER:
		{
			int width = textWidth(text);
			x -= (width/2);
			break;
		}	
		case RIGHT:
		{
			int width = textWidth(text);
			x -= width;
			break;
		}
	}

	switch(textHAlign)
	{
		switch(textVAlign)
		{
			default:
			case TOP:
				break;
			case MIDDLE:
			{
				y -= bfont_height/2;
				break;
			}
			case BOTTOM:
			{
				y -= bfont_height;
				break;
			}
		}
	}
	
	if(bfont_bitsPerPixel == 2)
	{
		mixedColors[0] = bcolor565;
		mixedColors[1] = mixColor(bcolor565, fcolor565, 1, 3);
		mixedColors[2] = mixColor(bcolor565, fcolor565, 2, 3);
		mixedColors[3] = fcolor565;
	}
	
	for(char *c = text; *c != 0; c++)
	{
		uint8_t byte = (uint8_t)*c;
		
		if(byte >= 0x80)
		{
			c++;
			uint8_t b2 = (uint8_t)*c;
			if(b2 == 0)
				break;
			
			if(byte == 0xC3) // latin-1 extended
			{
				byte = b2 + 0x40;
			}
			else if(byte == 0xC2) // latin-1 extended
			{
				byte = b2;
			}
			else
			{
				// unsupported character skip it
				continue;
			}
		}

		uint8_t* glyph;
		if(bfont_type == 0x02)
		{
			glyph = findGlyphBF2(byte);
			if(glyph == NULL)
				continue;		
			x = drawGlyphBF2(glyph, x, y);
		}
		else if(bfont_type == 0x03)
		{
			glyph = findGlyphBF3(byte);
			if(glyph == NULL)
				continue;		
			x = drawGlyphBF3(glyph, x, y);
		}
	}
}

int fontHeight()
{
	if(bfont_glyphIndex == NULL)
		return 0;

	return bfont_height;
}

int textWidth(char *text)
{
	if(bfont_glyphIndex == NULL)
		return 0;

	int width = 0;
		
	for(char *c = text; *c != 0; c++)
	{
		uint8_t byte = (uint8_t)*c;
		
		if(byte >= 0x80)
		{
			c++;
			uint8_t b2 = (uint8_t)*c;
			if(b2 == 0)
				break;
			
			if(byte == 0xC3) // latin-1 extended
			{
				byte = b2 + 0x40;
			}
			else if(byte == 0xC2) // latin-1 extended
			{
				byte = b2;
			}
			else
			{
				// unsupported character skip it
				continue;
			}
		}

		if(bfont_type == 0x02)
		{
			uint8_t* glyph = findGlyphBF2(byte);
			if(glyph == NULL)
				continue;		
			
			width += computeGlyphWidthBF2(glyph);
		}
		else
		{
			uint8_t* glyph = findGlyphBF3(byte);
			if(glyph == NULL)
				continue;		
			
			width += computeGlyphWidthBF3(glyph);
		}
	}
	
	return width;
}


}