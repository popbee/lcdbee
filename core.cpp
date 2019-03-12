/*
  BeeLcd - Arduino open source CPP library support for small color LCD Boards.
  
  Copyright (c)2011 Bernard Poulin. All rights reserved 
  Contact  bernard at acm.org
  
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

#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

#if defined(__PIC32MX__)
    #include <p32xxxx.h>    /* this gives all the CPU/hardware definitions */
    #include <plib.h>       /* this gives the i/o definitions */
#endif

#include "lcd_init.hpp"

namespace LcdBee
{
uint16_t 		fcolor565;
uint16_t 		bcolor565;
/*
#if defined(__AVR__)
volatile uint8_t	
#elif defined(__PIC32MX__)
volatile uint32_t
#endif
						*P_RS, 
						*P_WR, 
						*P_CS, 
						*P_RST, 
						*P_RD;
						
#if defined(__AVR__)
uint8_t	
#elif defined(__PIC32MX__)
uint32_t
#endif
						B_RS, 
						B_WR, 
						B_CS, 
						B_RST, 
						B_RD,
						NB_RS, 
						NB_WR, 
						NB_CS, 
						NB_RD;
*/

/*
  sequence selon le datasheet:
  
  RS LOW
  CS LOW & WR LOW -- mais seulement apres 10 ns -- encore la, pas vraiment un probleme!
  AH! le data doit etre valide a la MONTÉ de WR et non a la descente.
  write data (DBx)
  WR HIGH  &  CS HIGH
  DATA MUST STAY FOR ANOTHER 15 ns after WR HIGH
  ..et si on doit refaire un autre WR, on doit attendre au moins 50ns.  (on fait pas ca pour un seul pixel)
  RS HIGH -- mais seulement apres 5ns apres WR HIGH, pas avant (pas un probleme ca, 5ns c'est minuscule)

  ici il y a 2 transactions
  
  Je pense que pour "CS" il peut rester LOW tout le temps. ca marche bien avec l'arduino(?)

*/
#define LCDBEE_SET_REGISTER_INLINE(com, data)\
{\
	RS_LOW;\
	WRITE_DB16((uint16_t)(com));\
	WR_LOW_PULSE;\
	RS_HIGH;\
	WRITE_DB16((data));\
	WR_LOW_PULSE;\
}


bool 			areaFull;

enum AlphaMethod alpha_method = 
#ifdef LCDBEE_SUPPORT_READING_BACK
	ALPHA_MULTIPLY;
#else
	BACKGROUND_COLOR_MIX;
#endif

#ifdef LCDBEE_ORIENTATION_DYNAMIC
enum Orientation orientation = PORTRAIT;
#endif

bool readyForSinglePixelOperations;
struct _current_font		cfont;

#ifdef LCDBEE_ILI9325
#define LCDBEE_LCD_INIT_DATA lcd_init_data_ili9325
#endif

extern "C" PROGMEM lcd_init_t LCDBEE_LCD_INIT_DATA[];


void lcdReset()
{
	CS_HIGH;

	BUS_OUTPUT;

	// set all control pins as output
	
	LCDBEE_SET_PIN_OUTPUT_DIR(WR_SFR, WR_BIT);
	LCDBEE_SET_PIN_OUTPUT_DIR(RS_SFR, RS_BIT);

#ifndef RD_IS_NOT_USED
	LCDBEE_SET_PIN_OUTPUT_DIR(RD_SFR, RD_BIT);
#endif

#ifndef RST_IS_NOT_USED
	LCDBEE_SET_PIN_OUTPUT_DIR(RST_SFR, RST_BIT);
#endif
	
#ifndef CS_IS_NOT_USED
	LCDBEE_SET_PIN_OUTPUT_DIR(CS_SFR, CS_BIT);
#endif
	
	RST_HIGH;
	delay(5); 
	RST_LOW;
	delay(15);
	RST_HIGH;
	delay(15);

	CS_LOW;
	
#if defined(__AVR__)
	uint16_t init_cur_entry = (uint16_t)LCDBEE_LCD_INIT_DATA;
#elif defined(__PIC32MX__)
	lcd_init_t* init_cur_entry = LCDBEE_LCD_INIT_DATA;
#endif

	while(1)
	{
#if defined(__AVR__)
		uint8_t reg = pgm_read_byte(init_cur_entry);
		uint16_t data = pgm_read_word(init_cur_entry+1);
#elif defined(__PIC32MX__)
		uint8_t reg = init_cur_entry->registerindex;
		uint16_t data = init_cur_entry->data;
#endif
	
		if(reg == LCDBEE_EOL)
			break;
			
		if(reg == LCDBEE_DELAY)
			delay(data);
		else
			lcd_set_register(reg,data);
			
#if defined(__AVR__)
		init_cur_entry += 3;
#elif defined(__PIC32MX__)
		init_cur_entry++;		
#endif
	}
	
#ifdef LCDBEE_ORIENTATION_LANDSCAPE
	lcd_set_register(0x03, 0x1018); // set GRAM write direction and BGR=1.  
#endif
#ifdef LCDBEE_ORIENTATION_PORTRAIT	
	lcd_set_register(0x03, 0x1030); // set GRAM write direction and BGR=1.  
#endif

	CS_HIGH;
	
	setColor565(255);
	setBackColor565(0);
	cfont.font=0;

	readyForSinglePixelOperations = false;
}

void lcd_write_data16_repeat(uint16_t v, long repeat)
{
	RS_HIGH;
	WRITE_DB16(v);

	int repeat16 = (int)(repeat >> 4); // max repeat16 is 240 * 400 / 16 = 6000 which fits a 16 bits nicely
	
	while(repeat16--)
	{
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
		WR_LOW_PULSE;
	}
		
	int8_t remaining = (int8_t)repeat & 0xF;
	while(remaining--)
	{
		WR_LOW_PULSE;
	}
}

void lcd_register_index(uint8_t v)  
{   
	RS_LOW;
	WRITE_DB16(v);
	WR_LOW_PULSE;
}

uint16_t LCD_Read_GRAM()
{
	uint16_t busdata;
	RS_HIGH;
	RD_LOW;		// (must be 5ns after RS_HIGH) first read is "dummy"
	BUS_INPUT;  // 12.5 * 2 = 25ns
#if defined(__PIC32MX__)
	asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); // 125 ns
#endif
	RD_HIGH;     // doit etre > 150ns de RD_LOW (!)  // 12.5ns
#if defined(__PIC32MX__)
	asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); // 150 ns
#endif
	RD_LOW;		// doit etre > 150ns  de RD_HIGH (!!!!)  // 12.5ns 
#if defined(__PIC32MX__)
	asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); // 112.5 ns
#endif
	READ_DB16(busdata); // doit attendre > 100ns avec de lire le data  // 25ns minimum
	RD_HIGH; // 12.5 ns minimum ;  doit etre > 150ns de RD_LOW -- ca devrait etre correct 
	BUS_OUTPUT;  // 25ns minimum  -- bus est disponible apres 5ns de RD_HIGH
#if defined(__PIC32MX__)
	asm volatile("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"); // 125 ns
	// on doit attendre 150ns avant de jouer avec RD, on a mis 125ns ici, il reste donc 25 ns -- ca va etre correct, on lit jamais 2 fois de suite
#endif
	
	return busdata;
}

void lcd_set_register(uint8_t com, uint16_t data)
{
	RS_LOW;
	
/*		asm volatile(
		"lui %t0,0xf88\n "
		);
*/
	WRITE_DB16((uint16_t)(com)); // ceci prends au minimum 4 instructions (12.5 * 4) =  50 ns

//	WR_LOW;  // ceci prends au moins 12.5 ns   (correct pour le minimum 10 ns depuis RS_LOW)
//	asm volatile("nop\n nop\n nop\n nop\n");  // 50ns
//	WR_HIGH;  // ceci prends au moins 12.5 ns -- ce qui fait un total de 50ns + 12.5 = 62.5 ns  -- important: doit etre entre 50ns et 500ns
	WR_LOW_PULSE;

	RS_HIGH;  // ceci doit etre au moins 5ns de WR_HIGH -- 12.5ns est donc correct.
	
#if defined(__PIC32MX__)
	asm volatile("nop\n nop\n nop\n");  // 3 instructions (12.5 * 3 = 37.5 ns)
#endif
	
	WRITE_DB16((data));  // avant de changer data, on doit attendre 15ns depuis WR_HIGH -- on est a 12.5 + 12.5  = 25ns (pour la premiere instruction effective du write_db16)
	
	// WR_HIGH doit etre 50 ns de WR_LOW 
	
//	WR_LOW;  // ceci prends au moins 12.5 ns   (correct pour le minimum 10 ns depuis RS_LOW)
//	asm volatile("nop\n nop\n nop\n nop\n");  // 50ns
//	WR_HIGH;  // ceci prends au moins 12.5 ns -- ce qui fait un total de 50ns + 12.5 = 62.5 ns  -- important: doit etre entre 50ns et 500ns
	WR_LOW_PULSE;

#if defined(__PIC32MX__)
    // apres le WR_HIGH, on doit attendre 15 ns avant de changer le data
	asm volatile("nop\n");  // 1 instructions (12.5 ns)  -- la prochaine instruction est juste dans 12.5ns	
#endif

// prochain RW_LOW doit etre a 5ns -- ce qui est correct, car la prochaine instruction sera a 12.5ns pas avant.

//	LCDBEE_SET_REGISTER_INLINE(com,data);
}

#ifdef LCDBEE_ORIENTATION_DYNAMIC
void setOrientation(Orientation orient)
{
	orientation = orient;
}
#endif

/** unchecked and unrotated coordinates */
void doSetArea(int x1, int y1, int x2, int y2)
{	
	lcd_set_register(0x20,x1);	//Setting X      
#if defined(LCDBEE_ORIENTATION_PORTRAIT) || defined(LCDBEE_ORIENTATION_DYNAMIC)
	lcd_set_register(0x21,y1);	//Setting Y      
#endif
#ifdef LCDBEE_ORIENTATION_LANDSCAPE
	lcd_set_register(0x21,y2);	//Setting Y
#endif
	lcd_set_register(0x50,x1);	//start X      
	lcd_set_register(0x52,y1);	//start Y      
	lcd_set_register(0x51,x2);	//end X      
	lcd_set_register(0x53,y2);	//end Y
	LCDBEE_SET_GRAM_REGISTER;
	
	readyForSinglePixelOperations = false;
}
long setArea(
#if defined(LCDBEE_ORIENTATION_PORTRAIT) || defined(LCDBEE_ORIENTATION_DYNAMIC)
		int x1, int y1, int x2, int y2)
#endif
#if defined(LCDBEE_ORIENTATION_LANDSCAPE)
		int y2, int x1, int y1, int x2)
#endif
{
#ifdef LCDBEE_ORIENTATION_DYNAMIC
	if(orientation == LANDSCAPE)
	{
		// rotate coordinates 90 degrees
		int tmp = y2;
		y2 = LCDBEE_LCDMAX_Y-x1;
		x1 = y1;
		y1 = LCDBEE_LCDMAX_Y-x2;
		x2 = tmp;
	}
#endif
	
	// clip area to lcd	
	if(x1<0)
	{
		if(x2<0)
			return 0; 	// if both x are less than zero, return 0 (else it would have returned 1 or more)

		x1 = 0;
	}
	else if(x1 > LCDBEE_LCDMAX_Y)
	{
		if(x2 > LCDBEE_LCDMAX_Y)
			return 0; 	// if both x are outside, return 0 (else it would have returned 1 or more)
		x1 = LCDBEE_LCDMAX_Y;
	}

	if(x2<0)
		x2 = 0;
	else if(x2 > LCDBEE_LCDMAX_Y)
		x2 = LCDBEE_LCDMAX_Y;

	if(y1<0)
	{
		if(y2<0)
			return 0; 	// if both y are less than zero, return 0 (else it would have returned 1 or more)

		y1 = 0;
	}
	else if(y1 > LCDBEE_LCDMAX_Y)
	{
		if(y2 > LCDBEE_LCDMAX_Y)
			return 0; 	// if both y are outside, return 0 (else it would have returned 1 or more)
			
		y1 = LCDBEE_LCDMAX_Y;
	}
	
	if(y2<0)
		y2 = 0;
	else if(y2 > LCDBEE_LCDMAX_Y)
		y2 = LCDBEE_LCDMAX_Y;

#if defined(LCDBEE_ORIENTATION_LANDSCAPE)
	y1 = LCDBEE_LCDMAX_Y - y1;
	y2 = LCDBEE_LCDMAX_Y - y2;
#endif

	doSetArea(x1,y1,x2,y2);

	return (long)((y2 - y1)+1) * ((x2 - x1)+1);
}	

bool setCompleteAreaOnly(int x1, int y1, int x2, int y2)
{
#ifdef LCDBEE_ORIENTATION_DYNAMIC
	if(orientation == LANDSCAPE)
	{	
		int tmp = y2;
		y2 = LCDBEE_LCDMAX_Y-x1;
		x1 = y1;
		y1 = LCDBEE_LCDMAX_Y-x2;
		x2 = tmp;
	}
#endif
#if defined(LCDBEE_ORIENTATION_PORTRAIT) || defined(LCDBEE_ORIENTATION_DYNAMIC)
	// check if it fits enitrely within clipping area	
	if(x1<0 || x1 > LCDBEE_LCDMAX_X || x2<0 || x2 > LCDBEE_LCDMAX_X ||
	   y1<0 || y1 > LCDBEE_LCDMAX_Y || y2<0 || y2 > LCDBEE_LCDMAX_Y )
		return false;
		
	doSetArea(x1,y1,x2,y2);
#endif	
#ifdef LCDBEE_ORIENTATION_LANDSCAPE
	// check if it fits enitrely within clipping area
	if(x1<0 || x1 > LCDBEE_LCDMAX_Y || x2<0 || x2 > LCDBEE_LCDMAX_Y ||
	   y1<0 || y1 > LCDBEE_LCDMAX_X || y2<0 || y2 > LCDBEE_LCDMAX_X )
		return false;
		
	doSetArea(y1,LCDBEE_LCDMAX_Y-x2,y2,LCDBEE_LCDMAX_Y-x1);
#endif	
	return true;
}

void prepForSinglePixelOperations()
{
	lcd_set_register(0x50,0);				//start X      
	lcd_set_register(0x51,LCDBEE_LCDMAX_X);	//end X      
	lcd_set_register(0x52,0);				//start Y      
	lcd_set_register(0x53,LCDBEE_LCDMAX_Y);	//end Y
	LCDBEE_SET_GRAM_REGISTER;

	readyForSinglePixelOperations = true; 
}

void setPixel565(
#if	defined(LCDBEE_ORIENTATION_DYNAMIC) || defined(LCDBEE_ORIENTATION_PORTRAIT)
			int x, int y, 
#endif
#if	defined(LCDBEE_ORIENTATION_LANDSCAPE)
			int y, int x, 
#endif
			uint16_t color)  // TODO: faster to dig fcolor565?
{
#if	defined(LCDBEE_ORIENTATION_DYNAMIC)
	if(orientation == LANDSCAPE) // TODO: rotate coordinates before calling setPixel?
	{
		int tmp = x;
		x = y;
		y = LCDBEE_LCDMAX_Y-tmp;
	}
#endif
	
	// lcd clipping filter // TODO: clip before calling setPixel?
	if(x<0 || x>LCDBEE_LCD_WIDTH || y<0 || y>LCDBEE_LCD_HEIGHT)
		return;

#if	defined(LCDBEE_ORIENTATION_LANDSCAPE)
	y = LCDBEE_LCDMAX_Y-y;
#endif

	if(!readyForSinglePixelOperations)	
	{
		// this is to save on setting up the 'window' area 
		// for many "random locations" single pixel operations in a row
		// set it once the first time only 
		prepForSinglePixelOperations();
	}

	lcd_set_register(0x20,x);  // pos x
	lcd_set_register(0x21,y); // pos y

//	LCDBEE_SET_REGISTER_INLINE(0x20,x); // Setting X
//	LCDBEE_SET_REGISTER_INLINE(0x21,y); // Setting Y
	
	LCDBEE_SET_GRAM_REGISTER;

	RS_HIGH;  // ceci doit etre au moins 5ns de WR_HIGH -- 12.5ns est donc correct.
	
#if defined(__PIC32MX__)
	asm volatile("nop\n nop\n nop\n");  // 3 instructions (12.5 * 3 = 37.5 ns)
#endif	
	WRITE_DB16((color));  // avant de changer data, on doit attendre 15ns depuis WR_HIGH -- on est a 12.5 + 12.5  = 25ns (pour la premiere instruction effective du write_db16)
	
	// WR_HIGH doit etre 50 ns de WR_LOW 
	
//	WR_LOW;  // ceci prends au moins 12.5 ns   (correct pour le minimum 10 ns depuis RS_LOW)
//	asm volatile("nop\n nop\n nop\n nop\n");  // 50ns
//	WR_HIGH;  // ceci prends au moins 12.5 ns -- ce qui fait un total de 50ns + 12.5 = 62.5 ns  -- important: doit etre entre 50ns et 500ns

	WR_LOW_PULSE;
}

/** coordinates are inclusive: a pixel will be drawn at the "x2" position */
void drawHLine565(int x1, int x2, int y, uint16_t color)
{
	long visiblePixels = setArea(x1, y, x2, y);
	lcd_write_data16_repeat(color, visiblePixels);
}

/** coordinates are inclusive: a pixel will be drawn at the "y2" position */
void drawVLine565(int x, int y1, int y2, uint16_t color)
{
	long visiblePixels = setArea(x, y1, x, y2);
	lcd_write_data16_repeat(color, visiblePixels);
}

void fillScr565(uint16_t color)
{	
	doSetArea(0,0,LCDBEE_LCDMAX_X,LCDBEE_LCDMAX_Y);
	readyForSinglePixelOperations = true; // configuring the "full area" makes it ready for single pixel operations too.
	
	lcd_write_data16_repeat(color, (long)LCDBEE_LCD_HEIGHT * LCDBEE_LCD_WIDTH);
}

void fillRect565(int x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{	
	long visiblePixels = setArea(x1,y1,x2,y2);

	lcd_write_data16_repeat(color, visiblePixels);
}



inline uint16_t convertRGBto565(uint8_t r, uint8_t g, uint8_t b)
{
	// Note for "pure grays" -- pure gray (r=g=b) may not always appear gray because 
	// the green component may end up with a different value. This _could_ be visible 
	// in gray gradients (depending on the color used).
	//
	// We could fix this internally in two ways:
	// 1- always simulate 5 bit green (EQUALIZED_RGB_CHANNELS)
	// 2- simulate 5 bit green only for pure-gray colors only (if r == g == b) 
	//		EQUALIZED_PURE_GRAYS

#ifdef EQUALIZED_RGB_CHANNELS
	return (uint16_t)(
				( ((((unsigned int)r) << 8 ) & 0xF800) |
				( ((((unsigned int)g) << 3 ) & 0x07C0) |
				( ((((unsigned int)g) >> 2 ) & 0x0020) |
				  ((((unsigned int)b) >> 3 ) & 0x001F) );
#else 
#ifdef EQUALIZED_PURE_GRAYS
	if(r == b && r == g)
	{
		r &= 0xF8;
	        return (uint16_t)(
				( ((((unsigned int)r) << 8 )) |
				( ((((unsigned int)r) << 3 )) |
				( ((((unsigned int)r) >> 2 ) & 0x0020) |
				  ((((unsigned int)r) >> 3 )) );
	}
	else
	{
		return (uint16_t)(
				( ((((unsigned int)r) << 8 ) & 0xF800) |
				( ((((unsigned int)g) << 3 ) & 0x07E0) |
				  ((((unsigned int)b) >> 3 ) & 0x001F) );
	}
#else
	return (uint16_t)(
				  ((((unsigned int)r) << 8 ) & 0xF800) |
				  ((((unsigned int)g) << 3 ) & 0x07E0) |
				  ((((unsigned int)b) >> 3 ) & 0x001F) );
#endif
#endif
}

uint16_t readPixel565(
#if	defined(LCDBEE_ORIENTATION_DYNAMIC) || defined(LCDBEE_ORIENTATION_PORTRAIT)
			int x, int y) 
#endif
#if	defined(LCDBEE_ORIENTATION_LANDSCAPE)
			int y, int x) 
#endif
{
#if	defined(LCDBEE_ORIENTATION_DYNAMIC)
	if(orientation == LANDSCAPE)
	{
		int tmp = x;
		x = y;
		y = LCDBEE_LCDMAX_Y-tmp;
	}
#endif
	
	// lcd clipping filter
	if(x<0 || x>LCDBEE_LCDMAX_X || y<0 || y>LCDBEE_LCDMAX_Y)
		return 0;

#if	defined(LCDBEE_ORIENTATION_LANDSCAPE)
	y = LCDBEE_LCDMAX_Y-y;
#endif
	
	lcd_set_register(0x20,x);	// Setting X      
	lcd_set_register(0x21,y);	// Setting Y      

	LCDBEE_SET_GRAM_REGISTER;
	
	return LCD_Read_GRAM();
}


//==============================================================================
// API
//==============================================================================


void clearScreen()
{
	CS_LOW;
	fillScr565(bcolor565);
	CS_HIGH;
}

void fillScreen()
{
	fillScr565(fcolor565);
	CS_HIGH;
}

void setColor565(uint16_t rgb565)
{
	fcolor565 = rgb565;
}

void setBackColor565(uint16_t rgb565)
{
	bcolor565 = rgb565;
}

void setColor(uint32_t rgb)
{
	fcolor565 = convertRGBto565((rgb >> 16) & 0xff,(rgb >> 8) & 0xff,rgb & 0xff);
}

void setColor(uint8_t r, uint8_t g, uint8_t b)
{
	fcolor565 = convertRGBto565(r,g,b);
}

void setBackColor(uint8_t r, uint8_t g, uint8_t b)
{
	bcolor565 = convertRGBto565(r,g,b);
}

void setBackColor(uint32_t rgb)
{
	bcolor565 = convertRGBto565((rgb >> 16) & 0xff,(rgb >> 8) & 0xff,rgb & 0xff);
}

void setPixel(int x, int y)
{
	CS_LOW;
	setPixel565(x, y, fcolor565);
	CS_HIGH;
}

uint16_t getPixel565(int x, int y)
{
	CS_LOW;
	uint16_t v = readPixel565(x, y);
	CS_HIGH;
	
	return v;
}


}