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

#include "lcd_init.hpp"
#include <WProgram.h>


PROGMEM lcd_init_t lcd_init_data_ili9325[] = 
{
	{0x01, 0x0100}, // set Driver Output Control  
	{0x02, 0x0200}, // set 1 line inversion  
//	{0x03, 0x1030}, // set GRAM write direction and BGR=1.  (done in lcdreset method)
	{0x04, 0x0000}, // Resize register  
	{0x08, 0x0207}, // set the back porch and front porch  
	{0x09, 0x0000}, // set non-display area refresh cycle ISC[3:0]  
	{0x0A, 0x0000}, // FMARK function  
	{0x0C, 0x0000}, // RGB interface setting  
	{0x0D, 0x0000}, // Frame marker Position  
	{0x0F, 0x0000}, // RGB interface polarity  
	// *************Power On sequence **************** //  
	{0x10, 0x0000}, // SAP, BT[3:0], AP, DSTB, SLP, STB  
	{0x11, 0x0007}, // DC1[2:0], DC0[2:0], VC[2:0]  
	{0x12, 0x0000}, // VREG1OUT voltage  
	{0x13, 0x0000}, // VDV[4:0] for VCOM amplitude  
	{0x07, 0x0001},  
	{LCDBEE_DELAY, 200}, // Dis-charge capacitor power voltage  
	{0x10, 0x1690}, // SAP, BT[3:0], AP, DSTB, SLP, STB  
	{0x11, 0x0227}, // Set DC1[2:0], DC0[2:0], VC[2:0]  
	{LCDBEE_DELAY, 50}, // Delay 50ms  
	{0x12, 0x000D}, // 0012  
	{LCDBEE_DELAY, 50}, // Delay 50ms  
	{0x13, 0x1200}, // VDV[4:0] for VCOM amplitude  
	{0x29, 0x000A}, // 04  VCM[5:0] for VCOMH  
	{0x2B, 0x000D}, // Set Frame Rate  
	{LCDBEE_DELAY, 50}, // Delay 50ms  
	{0x20, 0x0000}, // GRAM horizontal Address  
	{0x21, 0x0000}, // GRAM Vertical Address  
	// ----------- Adjust the Gamma Curve ----------//  
	{0x30, 0x0000},  
	{0x31, 0x0404},  
	{0x32, 0x0003},  
	{0x35, 0x0405},  
	{0x36, 0x0808},  
	{0x37, 0x0407},  
	{0x38, 0x0303},  
	{0x39, 0x0707},  
	{0x3C, 0x0504},  
	{0x3D, 0x0808},  
	//------------------ Set GRAM area ---------------//  
	{0x50, 0x0000}, // Horizontal GRAM Start Address  
	{0x51, 0x00EF}, // Horizontal GRAM End Address  
	{0x52, 0x0000}, // Vertical GRAM Start Address  
	{0x53, 0x013F}, // Vertical GRAM Start Address  
	{0x60, 0xA700}, // Gate Scan Line  
	{0x61, 0x0001}, // NDL,VLE, REV   
	{0x6A, 0x0000}, // set scrolling line  
	//-------------- Partial Display Control ---------//  
	{0x80, 0x0000},  
	{0x81, 0x0000},  
	{0x82, 0x0000},  
	{0x83, 0x0000},  
	{0x84, 0x0000},  
	{0x85, 0x0000},  
	//-------------- Panel Control -------------------//  
	{0x90, 0x0010},  
	{0x92, 0x0000},  
	{0x07, 0x0133}, // 262K color and display ON        
	{LCDBEE_EOL, 0}
};

