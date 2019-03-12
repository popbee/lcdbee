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

void setFont(uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=LCDBEE_READPROGMEM_UINT8(&cfont.font[0]);
	cfont.y_size=LCDBEE_READPROGMEM_UINT8(&cfont.font[1]);
	cfont.offset=LCDBEE_READPROGMEM_UINT8(&cfont.font[2]);
	cfont.numchars=LCDBEE_READPROGMEM_UINT8(&cfont.font[3]);
}

void printChar(byte c, int x, int y)
{
	uint16_t cfont_x_size_bytes = cfont.x_size/8;
	uint16_t cfont_size_bytes = cfont_x_size_bytes * cfont.y_size;

	CS_LOW;

	if(!setCompleteAreaOnly(x,y,x+cfont.x_size-1,y+cfont.y_size-1))
	{
		//TODO: logic to display partial character
		return; 
	}		

	RS_HIGH;

	uint16_t temp=((c-cfont.offset)*cfont_size_bytes)+4;

	while(cfont_size_bytes--)
	{
		uint8_t ch = LCDBEE_READPROGMEM_UINT8(&cfont.font[temp]);

		uint8_t mask = 0x80;

		do  // will go through 0x80, 0x40, 0x20, 0x10...0x2,0x1,0x0 (and stop at zero)
		{
			uint16_t color = (ch & mask) ? fcolor565 : bcolor565;
			WRITE_DB16(color);
			WR_LOW_PULSE;
		} while(mask>>=1);

		temp++;
	}

	CS_HIGH;
}

void rotateChar(byte c, int x, int y, int pos, int deg)
{
	word temp; 
	int newx,newy;
	
	double radian = deg*0.0175;  
	double cos_radian = cos(radian);
	double sin_radian = sin(radian);
	byte font_x_size_bytes = cfont.x_size/8;
	
	CS_LOW;

	temp=((c-cfont.offset)*(font_x_size_bytes*cfont.y_size))+4;
	for(byte j=0;j<cfont.y_size;j++) 
	{
		for (int zz=0; zz<font_x_size_bytes; zz++)
		{
			byte ch=LCDBEE_READPROGMEM_UINT8(&cfont.font[temp+zz]); 
			for(byte i=0;i<8;i++)
			{
				newx=x+(((i+(zz*8)+(pos*cfont.x_size))*cos_radian)-((j)*sin_radian));
				newy=y+(((j)*cos_radian)+((i+(zz*8)+(pos*cfont.x_size))*sin_radian));

				setPixel565(newx, newy, (ch&(1<<(7-i)))!=0 ? fcolor565 : bcolor565);
			}
		}
		temp+=font_x_size_bytes;
	}
	CS_HIGH;
}

void print(char *st, int x, int y)
{
	//TODO: optimize so we do not get all that rotation "junk".
	print(st, x, y, 0);
}


// TODO: change this to pass a separate "alignment" in regards to the x,y position (instead of aligning with the screen)
void print(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

#ifdef LCDBEE_ORIENTATION_DYNAMIC
	if(orientation == PORTRAIT) // if PORTRAIT
	{
#endif
#if defined(LCDBEE_ORIENTATION_PORTRAIT) || defined(LCDBEE_ORIENTATION_DYNAMIC)
		if (x==RIGHT)
			x=LCDBEE_LCD_WIDTH-(stl*cfont.x_size);
		else if (x==CENTER)
			x=(LCDBEE_LCD_WIDTH-(stl*cfont.x_size))/2;
#endif
#ifdef LCDBEE_ORIENTATION_DYNAMIC
	}
	else
	{
#endif
#ifdef LCDBEE_ORIENTATION_LANDSCAPE || defined(LCDBEE_ORIENTATION_DYNAMIC)
		if (x==RIGHT)
			x=LCDBEE_LCD_HEIGHT-(stl*cfont.x_size);
		else if (x==CENTER)
			x=(LCDBEE_LCD_HEIGHT-(stl*cfont.x_size))/2;
#endif
#ifdef LCDBEE_ORIENTATION_DYNAMIC
	}
#endif

	for (i=0; i<stl; i++)
		if (deg==0)
			printChar(*st++, x + (i*(cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}

void printNumI(long num, int x, int y)
{
  char buf[25];
  char st[27];
  int neg=0;
  int c=0;
  
  if (num==0)
  {
	  st[0]=48;
	  st[1]=0;
  }
  else
  {
	  if (num<0)
	  {
		neg=1;
		num=-num;
	  }
	  
	  while (num>0)
	  {
		buf[c]=48+(num % 10);
		c++;
		num=(num-(num % 10))/10;
	  }
	  buf[c]=0;
	  
	  if (neg)
	  {
		st[0]=45;
	  }
	  
	  for (int i=0; i<c; i++)
	  {
		st[i+neg]=buf[c-i-1];
	  }
	  st[c+neg]=0;
  }

  print(st,x,y);
}

void printNumF(double num, byte dec, int x, int y)
{
  char buf[25];
  char st[27];
  boolean neg=false;
  int c=0;
  int c2;
  unsigned long inum;
  
  if (num==0)
  {
	  st[0]=48;
	  st[1]=46;
	  for (int i=0; i<dec; i++)
		  st[2+i]=48;
	  st[2+dec]=0;
  }
  else
  {
	  if (num<0)
	  {
		neg=true;
		num=-num;
	  }
	  
	  if (dec<1)
		dec=1;
	  if (dec>5)
		dec=5;
	  
	  inum=long(num*pow(10,dec));
	  
	  while (inum>0)
	  {
		buf[c]=48+(inum % 10);
		c++;
		inum=(inum-(inum % 10))/10;
	  }
	  if ((num<1) and (num>0))
	  {
		  buf[c]=48;
		  c++;
	  }
	  buf[c]=0;
	  
	  if (neg)
	  {
		st[0]=45;
	  }
	  
	  c2=neg;
	  for (int i=0; i<c; i++)
	  {
		st[c2]=buf[c-i-1];
		c2++;
		if ((c-(c2-neg))==dec)
		{
		  st[c2]=46;
		  c2++;
		}
	  }
	  st[c2]=0;
  }

  print(st,x,y);
}

}