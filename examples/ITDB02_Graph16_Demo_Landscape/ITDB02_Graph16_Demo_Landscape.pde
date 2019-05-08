#include <LcdBee.h>

// ITDB02_Graph16_Demo (C)2011 Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
//
// This program is a demo of how to use most of the functions
// of the library with a supported 16bit display module.
//
// This demo was not made for the widescreen modules. Use
// ITDB02_Graph16_Demo_Widescreen instead.
//
// This program requires the ITDB02_Graph16 library.
//

// Declare which fonts we will be using
extern uint8_t SmallFont[];

extern prog_uint16_t tux[0x400];

extern PROGMEM unsigned char Tahoma_data[];
extern PROGMEM unsigned char ment_data[];
extern PROGMEM unsigned char WizardsMagic_data[];   //TODO: make a #define for this  LCDBEE_DECLAREFONT(WizardsMagic_data)

void setup()
{
  randomSeed(analogRead(0));
  
// reset the LCD
  lcdReset();
  setFont(SmallFont);
}

void loop()
{
  int buf[318];
  int x, x2;
  int y, y2;
  int r;

  setAlphaMethod(SINGLE_COLOR_SHAPE);

  setBackColor(BLACK);
  clearScreen();
  setColor(255,255,255);

  textAlign(CENTER);
  textFont(Tahoma_data);
  text("Allô Jérémie, Ô Mentaliste! :)", 160, 20);

/*  textFont(ment_data);
  setColor(0,0,255);
  textAlign(LEFT);
  text("The", 10, 35);
  textAlign(RIGHT);
  text("iMentalist", 310, 90);
*/
  textFont(WizardsMagic_data);
  setColor(90,51,245);
  setColor(255,255,255);
  textAlign(LEFT);
  text("The", 10, 35);
  textAlign(RIGHT);
  text("iMentalist", 310, 90);
delay(5000);

  setBackColor(WHITE);
  clearScreen();
  setColor(PURPLE);
  textAlign(LEFT);
  text("The", 10, 35);
  textAlign(RIGHT);
  text("iMentalist", 310, 90);
delay(5000);
  
/*
  setColor(255,255,255);
  for (int x=5; x<316; x++)
  {
    for(int y=5; y<236; y++)
      setPixel(x,y);
  }


delay(1500);
  setColor(128,0,128);
  for (int x=5; x<316; x++)
  {
    for(int y=5; y<236; y++)
      setPixel(x,y);
  }
delay(1500);

 
  setBackColor(BLACK);
  setColor(PURPLE);
  print("   Two different icons in scale 1 to 4  ", CENTER, 228);
   x=0;
  for (int s=0; s<4; s++)
  {
    x+=(s*32);
    drawBitmap (x, 0, 32, 32, tux, s+1);
  }

  setColor(AQUA);
  rect(0,0,319,239); 

  setColor(255, 0, 0);
  fillRect(0, 0, 319, 13);
  setColor(64, 64, 64);
  fillRect(0, 226, 319, 239);
  setColor(255, 255, 255);
  setBackColor(255, 0, 0);
  print("*** QVGA TFT 240x320 Color Display ***", CENTER, 1);
  setBackColor(64, 64, 64);
  setColor(255,255,0);
  print("/ lcdbee!!! \\", CENTER, 227);

delay(2000);

  setColor(0, 0, 255);
  rect(0, 14, 319, 225);

// Draw crosshairs
  setColor(0, 0, 255);
  setBackColor(0, 0, 0);
  line(159, 15, 159, 224);
  line(1, 119, 318, 119);
  for (int i=9; i<310; i+=10)
    line(i, 117, i, 121);
  for (int i=19; i<220; i+=10)
    line(157, i, 161, i);

// Draw sin-, cos- and tan-lines  
  setColor(0,255,255);
  print("Sin", 5, 15);
  for (int i=1; i<318; i++)
  {
    setPixel(i,119+(sin(((i*1.13)*3.14)/180)*95));
  }
  
  setColor(255,0,0);
  print("Cos", 5, 27);
  for (int i=1; i<318; i++)
  {
    setPixel(i,119+(cos(((i*1.13)*3.14)/180)*95));
  }

  setColor(255,255,0);
  print("Tan", 5, 39);
  for (int i=1; i<318; i++)
  {
    setPixel(i,119+(tan(((i*1.13)*3.14)/180)));
  }

  delay(2000);

  setColor(0,0,0);
  fillRect(1,15,318,224);
  setColor(0, 0, 255);
  setBackColor(0, 0, 0);
  line(159, 15, 159, 224);
  line(1, 119, 318, 119);

// Draw a moving sinewave
  x=1;
  for (int i=1; i<(318*20); i++) 
  {
    x++;
    if (x==319)
      x=1;
    if (i>319)
    {
      if ((x==159)||(buf[x-1]==119))
        setColor(0,0,255);
      else
        setColor(0,0,0);
      setPixel(x,buf[x-1]);
    }
    setColor(0,255,255);
    y=119+(sin(((i*1.1)*3.14)/180)*(90-(i / 100)));
    setPixel(x,y);
    buf[x-1]=y;
  }

  delay(2000);
  
  setColor(0,0,0);
  fillRect(1,15,318,224);

// Draw some filled rectangles
  for (int i=1; i<6; i++)
  {
    switch (i)
    {
      case 1:
        setColor(255,0,255);
        break;
      case 2:
        setColor(255,0,0);
        break;
      case 3:
        setColor(0,255,0);
        break;
      case 4:
        setColor(0,0,255);
        break;
      case 5:
        setColor(255,255,0);
        break;
    }
    fillRect(70+(i*20), 30+(i*20), 130+(i*20), 90+(i*20));
  }

  delay(2000);
  
  setColor(0,0,0);
  fillRect(1,15,318,224);

// Draw some filled, rounded rectangles
  for (int i=1; i<6; i++)
  {
    switch (i)
    {
      case 1:
        setColor(255,0,255);
        break;
      case 2:
        setColor(255,0,0);
        break;
      case 3:
        setColor(0,255,0);
        break;
      case 4:
        setColor(0,0,255);
        break;
      case 5:
        setColor(255,255,0);
        break;
    }
    fillRoundRect(190-(i*20), 30+(i*20), 250-(i*20), 90+(i*20));
  }
  
  delay(2000);
  
  setColor(0,0,0);
  fillRect(1,15,318,224);

// Draw some filled circles
  for (int i=1; i<6; i++)
  {
    switch (i)
    {
      case 1:
        setColor(255,0,255);
        break;
      case 2:
        setColor(255,0,0);
        break;
      case 3:
        setColor(0,255,0);
        break;
      case 4:
        setColor(0,0,255);
        break;
      case 5:
        setColor(255,255,0);
        break;
    }
    fillCircle(100+(i*20),60+(i*20), 30);
  }
  
  delay(2000);
  */
  /*
  setColor(0,0,0);
  fillRect(1,15,318,224);

// Draw some lines in a pattern
  setColor (255,0,0);
  setBackColor(0,0,0);
  for (int i=15; i<224; i+=5)
  {
    smoothLine(1, i, (i*1.44)-10, 224);
  }
  setColor (255,0,0);
  for (int i=224; i>15; i-=5)
  {
    smoothLine(318, i, (i*1.44)-11, 15);
  }
  setColor (0,255,255);
  for (int i=224; i>15; i-=5)
  {
    smoothLine(1, i, 331-(i*1.44), 15);
  }
  setColor (0,255,255);
  for (int i=15; i<224; i+=5)
  {
    smoothLine(318, i, 330-(i*1.44), 224);
  }
  
  delay(15000);
  */
/*  
  setColor(0,0,0);
  fillRect(1,15,318,225);

// Draw some random circles
  for (int i=0; i<100; i++)
  {
    setColor(random(255), random(255), random(255));
    x=32+random(256);
    y=45+random(146);
    r=random(30);
    circle(x, y, r);
  }

  delay(2000);
  
  setColor(0,0,0);
  fillRect(1,15,318,224);

// Draw some random rectangles
  for (int i=0; i<100; i++)
  {
    setColor(random(255), random(255), random(255));
    x=2+random(316);
    y=16+random(207);
    x2=2+random(316);
    y2=16+random(207);
    rect(x, y, x2, y2);
  }

  delay(2000);
  
  setColor(0,0,0);
  fillRect(1,15,318,224);

// Draw some random rounded rectangles
  for (int i=0; i<100; i++)
  {
    setColor(random(255), random(255), random(255));
    x=2+random(316);
    y=16+random(207);
    x2=2+random(316);
    y2=16+random(207);
    roundRect(x, y, x2, y2);
  }

  delay(2000);
*/  /*
  setBackColor(0,0,0);
  clearScreen();

    setColor(255, 255, 0); // full yellow

int lastx = 150;
int lasty = 150;

#define DIST  30

  for (int i=0; i<500; i++)
  {
    x=lastx + random(DIST*2) - DIST;
    y=lasty + random(DIST*2) - DIST;
    
    if(x < 0)
      x = 0;
      else if(x > 319)
      x = 319;

    if(y < 0)
      y = 0;
      else if(y > 239)
      y = 239;
      
    smoothLine(lastx, lasty, x, y);
    
    lastx = x;
    lasty = y;
  }

  delay(15000);
  /*
  setColor(0,0,0);
  fillRect(1,15,318,224);

  for (int i=0; i<10000; i++)
  {
    setColor(random(255), random(255), random(255));
    setPixel(2+random(316), 16+random(209));
  }

  delay(2000);
  setColor(0, 0, 255);
  fillScreen();
  setColor(255, 0, 0);
  fillRoundRect(80, 70, 239, 169);
  
  setColor(255, 255, 255);
  setBackColor(255, 0, 0);
  print("Et voila!", CENTER, 93);
  print("Restarting in a", CENTER, 119);
  print("few seconds...", CENTER, 132);

  setColor(0, 255, 0);
  setBackColor(0, 0, 255);
  print("Runtime: (msecs)", CENTER, 210);
  printNumI(millis(), CENTER, 225);
 
  delay (5000);
  */
}

