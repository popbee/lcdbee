/*
  BeeLcd - Arduino open source CPP library support for small color LCD Boards.
  
  http://code.google.com/p/lcdbee/
  
  Copyright (c)2011 Bernard Poulin. All rights reserved 

------------------------------------------------------------------------------
  
  LcdBeeBase.h - master header file for internal purposes

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
#ifndef LCDBEE_CORE_H
#define LCDBEE_CORE_H

#include "LcdBee.h"

#if defined(__PIC32MX__)
#define cbi(sfr, bit) ((sfr) &= ~_BV(bit))
#define sbi(sfr, bit) ((sfr) |= _BV(bit))

#define gpio_cbi(sfr, bit) ((*((&sfr)+5)) = _BV(bit))
#define gpio_sbi(sfr, bit) ((*((&sfr)+6)) = _BV(bit))

#endif

#if defined(__AVR__)
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define gpio_cbi(sfr, bit) cbi(sfr,bit)
#define gpio_sbi(sfr, bit) sbi(sfr,bit)
#endif

#define WR_LOW	gpio_cbi(WR_SFR, WR_BIT)
#define WR_HIGH	gpio_sbi(WR_SFR, WR_BIT)


#define WR_LOW_PULSE {\
	WR_LOW;\
	asm volatile("nop\n nop\n nop\n nop\n");\
	WR_HIGH;\
}

#ifdef CS_IS_NOT_USED
#define CS_LOW 		
#define CS_HIGH		
#else
#define CS_LOW 		gpio_cbi(CS_SFR, CS_BIT)
#define CS_HIGH		gpio_sbi(CS_SFR, CS_BIT)
#endif

#define RS_LOW 		 gpio_cbi(RS_SFR, RS_BIT)
#define RS_HIGH		 gpio_sbi(RS_SFR, RS_BIT)

#ifdef RD_IS_NOT_USED
#define RD_LOW 		
#define RD_HIGH		
#else
#define RD_LOW 		gpio_cbi(RD_SFR, RD_BIT)
#define RD_HIGH		gpio_sbi(RD_SFR, RD_BIT)
#endif

#ifdef RST_IS_NOT_USED
#define RST_LOW 		
#define RST_HIGH		
#else
#define RST_LOW 	gpio_cbi(RST_SFR, RST_BIT)
#define RST_HIGH	gpio_sbi(RST_SFR, RST_BIT)
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define WRITE_DB16(v) { \
	PORTA = ((uint8_t)((v) >> 8));\
	PORTC = (uint8_t)(v);	}
#elif defined(__AVR__)
#define WRITE_DB16(v) { \
	PORTD = (uint8_t)((v) >> 8);\
	PORTC &= 0xFC;\
	PORTC |= (((uint8_t)(v))>>6) & 0x03; \
	PORTB = ((uint8_t)(v)) & 0x3F; }
#endif

#if defined(__PIC32MX__)  //TODO: try to find ways to use the "storebyte" fonctionality of the MIPS cpu
#define WRITE_DB16(v) { \
	LATDCLR = 0xFF;\
	LATDSET = (uint8_t)(v) & 0xFF;\
	LATECLR = 0xFF;\
	LATESET = ((uint8_t)((v) >> 8)) & 0xFF;}
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define READ_DB16(v) { \
	v = PINA;\
	v <<= 8;\
	v |= PINC;	}
#elif defined(__AVR__)
#define READ_DB16(v) { \
	v = PIND;\
	v <<=8;\
	v |= (PINC << 6) & 0xC0;\
	v |= (PINB & 0x3F); }
#endif

#if defined(__PIC32MX__)
#define READ_DB16(v) { \
	v = ((PORTE & 0xff) << 8) | (PORTD & 0xff); }
#endif

#if defined(__AVR__)
#define LCDBEE_SET_PIN_INPUT_DIR(sfr, bit)	cbi((*((& sfr )-1)),(bit))
#define LCDBEE_SET_PIN_OUTPUT_DIR(sfr, bit)	sbi((*((& sfr )-1)),(bit))
#endif

#if defined(__PIC32MX__)
#define LCDBEE_SET_PIN_INPUT_DIR(sfr, bit)	((*((&sfr)-2)) = _BV(bit))
#define LCDBEE_SET_PIN_OUTPUT_DIR(sfr, bit) ((*((&sfr)-3)) = _BV(bit))
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define BUS_OUTPUT {\
	DDRA = 0xFF;\
	DDRC = 0xFF; }
#else
#define BUS_OUTPUT {\
	DDRD = 0xFF;\
	DDRB |= 0x3F;\
	DDRC |= 0x03;}
#endif
#if defined(__PIC32MX__)
#define BUS_OUTPUT {\
	TRISD &= ~0xFF;\
	TRISE = 0;}
#endif
	
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define BUS_INPUT {\
	DDRA = 0x00;\
	DDRC = 0x00; }
#else
#define BUS_INPUT {\
	DDRD = 0x00;\
	DDRB &= 0xC0;\
	DDRC &= 0xFC;}
#endif
#if defined(__PIC32MX__)
#define BUS_INPUT {\
	TRISD |= 0xFF;\
	TRISE |= 0xFF;}
#endif

#if defined(__AVR__)
#define LCDBEE_READPROGMEM_UINT8(addr)	(uint8_t)pgm_read_byte(addr)
#define LCDBEE_READPROGMEM_UINT16(addr)	(uint16_t)pgm_read_word(addr)
#elif defined(__PIC32MX__)
#define LCDBEE_READPROGMEM_UINT8(addr)	((uint8_t)(*(addr)))
#define LCDBEE_READPROGMEM_UINT16(addr)	((uint16_t)(*(addr)))
#endif

// Max X and Max Y coordinates of the native resolution in the "natural" orientation
#define	LCDBEE_LCDMAX_Y (LCDBEE_LCD_HEIGHT-1)
#define	LCDBEE_LCDMAX_X (LCDBEE_LCD_WIDTH-1)

// Max Y and Max Y coordinates from a "logical" point of view (but in 'portrait' with dynamic orientation)
#if defined(LCDBEE_ORIENTATION_PORTRAIT) || defined(LCDBEE_ORIENTATION_DYNAMIC)
#define	LCDBEE_MAX_Y LCDBEE_LCDMAX_Y
#define	LCDBEE_MAX_X LCDBEE_LCDMAX_X
#endif
#if defined(LCDBEE_ORIENTATION_LANDSCAPE)
#define	LCDBEE_MAX_X LCDBEE_LCDMAX_Y
#define	LCDBEE_MAX_Y LCDBEE_LCDMAX_X
#endif

#define LCDBEE_GRAM_REGISTER	0x22

#define LCDBEE_SET_GRAM_REGISTER {\
	RS_LOW;\
	WRITE_DB16((uint16_t)(LCDBEE_GRAM_REGISTER));\
	WR_LOW_PULSE;\
}

namespace LcdBee
{

// internal variables
typedef struct _current_font
{
	uint8_t* font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
};

extern uint16_t 		fcolor565;
extern uint16_t 		bcolor565;
extern bool 			readyForSinglePixelOperations;
extern struct _current_font		cfont;

extern void lcd_write_data16_repeat(uint16_t v, long repeat);
extern void lcd_current_register(uint8_t v) ;
extern void lcd_write_data16(uint16_t v);
extern uint16_t LCD_Read_GRAM();
extern void lcd_set_register(uint8_t com, uint16_t data);
extern void doSetArea(int x1, int y1, int x2, int y2);
extern long setArea(int x1, int y1, int x2, int y2);
extern bool setCompleteAreaOnly(int x1, int y1, int x2, int y2);
extern void prepForSinglePixelOperations();
extern uint16_t readPixel565(int x, int y);

extern enum AlphaMethod alpha_method;
#ifdef LCDBEE_ORIENTATION_DYNAMIC
	extern enum Orientation orientation;
#endif

extern void setPixel565(int x, int y, uint16_t color);
extern void setPixel565Alpha8(int x, int y, uint16_t color, uint8_t alpha);
extern void drawHLine565(int x1, int x2, int y, uint16_t color);
extern void drawVLine565(int x, int y1, int y2, uint16_t color);
extern void fillScr565(uint16_t color);
extern void fillRect565(int x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

extern void printChar(byte c, int x, int y);
extern void rotateChar(byte c, int x, int y, int pos, int deg);

}
#endif // LCDBEEBASE_H
