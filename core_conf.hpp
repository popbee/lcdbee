/*
  LcdBee - Arduino open source CPP library support for small color LCD Boards.
  
  http://code.google.com/p/lcdbee/
  
  Copyright (c)2011 Bernard Poulin. All rights reserved 

------------------------------------------------------------------------------
  
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
#ifndef LCDBEE_CORECONF_H
#define LCDBEE_CORECONF_H

#include "lcd_config.txt"

#ifndef RD_IS_NOT_USED
 #define LCDBEE_SUPPORT_READING_BACK
#else
 #ifdef LCDBEE_INTERNAL_LCD_BUFFER
  #define LCDBEE_SUPPORT_READING_BACK
 #endif
#endif

#endif