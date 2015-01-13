/*
 * This is a series of functions to communicate between an ILI9320 LCD
 * controller and an STM32F1xx series ARM microcontroller. The required
 * pinout can be found in the pinout.txt file at the root of the project.
 *
 * This code borrows a lot from other similar libraries, I hope it helps
 * you as much as they have helped me.
 */

// TODO: other basic graphics methods
// TODO: handle dynamic lcd orientation

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "lcd_control.h"
#include "smallfont.h"
#include "stdlib.h"

u16 LCD_DeviceCode;

static LCD_OrientationMode_t orientation_mode = LCD_ORIENTATION_DEFAULT;

void LCD_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // Open the clock we want
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|
                         RCC_APB2Periph_GPIOB|
                         RCC_APB2Periph_GPIOC|
                         RCC_APB2Periph_GPIOD, ENABLE);

  // Configure the LCD pins for push-pull output
  // This is just the lower 8 bits of data
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|
    GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOB,&GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_Init(GPIOC,&GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOD,&GPIO_InitStructure);

}

/*
 * Name: void LCD_Initialization()
 * Function: initialize ILI9320 controller(may work with 9325 and 9328, untested)
 * Input: none
 * Output: none
 * Call: LCD_Initialization();
 */
void LCD_Initialization()
{
  u16 i;

  // Set all bits
  LCD_WriteData(0xffff);
  // Set various pins
  Set_Rst;
  Set_nWr;
  Set_Cs;
  Set_Rs;
  Set_nRd;
  Set_Rst;
  LCD_Reset();  // reset LCD

  LCD_WriteRegister(0x00e7,0x0010);
  LCD_WriteRegister(0x0000,0x0001);                     // Starts internal oscillator
  LCD_WriteRegister(0x0001,0x0100);
  LCD_WriteRegister(0x0002,0x0700);                     // Power on
  LCD_WriteRegister(0x0003,(1 << 12) | (1 << 5) | (1 << 4));            // 65K
  LCD_WriteRegister(0x0004,0x0000);
  LCD_WriteRegister(0x0008,0x0207);
  LCD_WriteRegister(0x0009,0x0000);
  LCD_WriteRegister(0x000a,0x0000);                     // Display setting
  LCD_WriteRegister(0x000c,0x0001);
  LCD_WriteRegister(0x000d,0x0000);
  LCD_WriteRegister(0x000f,0x0000);

  // Power on sequence
  LCD_WriteRegister(0x0010,0x0000);
  LCD_WriteRegister(0x0011,0x0007);
  LCD_WriteRegister(0x0012,0x0000);
  LCD_WriteRegister(0x0013,0x0000);
  for (i = 50000;i > 0;i--) __asm__("nop");
  for (i = 50000;i > 0;i--) __asm__("nop");
  LCD_WriteRegister(0x0010,0x1590);
  LCD_WriteRegister(0x0011,0x0227);
  for (i = 50000;i > 0;i--) __asm__("nop");
  for (i = 50000;i > 0;i--) __asm__("nop");
  LCD_WriteRegister(0x0012,0x009c);
  for (i = 50000;i > 0;i--) __asm__("nop");
  for (i = 50000;i > 0;i--) __asm__("nop");
  LCD_WriteRegister(0x0013,0x1900);
  LCD_WriteRegister(0x0029,0x0023);
  LCD_WriteRegister(0x002b,0x000E);
  for (i = 50000;i > 0;i--) __asm__("nop");
  for (i = 50000;i > 0;i--) __asm__("nop");
  LCD_WriteRegister(0x0020,0x0000);
  LCD_WriteRegister(0x0021,0x0000);
  for (i = 50000;i > 0;i--) __asm__("nop");
  for (i = 50000;i > 0;i--) __asm__("nop");
  LCD_WriteRegister(0x0030,0x0007);
  LCD_WriteRegister(0x0031,0x0707);
  LCD_WriteRegister(0x0032,0x0006);
  LCD_WriteRegister(0x0035,0x0704);
  LCD_WriteRegister(0x0036,0x1f04);
  LCD_WriteRegister(0x0037,0x0004);
  LCD_WriteRegister(0x0038,0x0000);
  LCD_WriteRegister(0x0039,0x0706);
  LCD_WriteRegister(0x003c,0x0701);
  LCD_WriteRegister(0x003d,0x000f);
  for (i = 50000;i > 0;i--) __asm__("nop");
  for (i = 50000;i > 0;i--) __asm__("nop");
  LCD_WriteRegister(0x0050,0x0000);
  LCD_WriteRegister(0x0051,0x00ef);
  LCD_WriteRegister(0x0052,0x0000);
  LCD_WriteRegister(0x0053,0x013f);
  LCD_WriteRegister(0x0060,0xA700);
  LCD_WriteRegister(0x0061,0x0001);
  LCD_WriteRegister(0x006a,0x0000);
  LCD_WriteRegister(0x0080,0x0000);
  LCD_WriteRegister(0x0081,0x0000);
  LCD_WriteRegister(0x0082,0x0000);
  LCD_WriteRegister(0x0083,0x0000);
  LCD_WriteRegister(0x0084,0x0000);
  LCD_WriteRegister(0x0085,0x0000);
  LCD_WriteRegister(0x0090,0x0010);
  LCD_WriteRegister(0x0092,0x0000);
  LCD_WriteRegister(0x0093,0x0003);
  LCD_WriteRegister(0x0095,0x0110);
  LCD_WriteRegister(0x0097,0x0000);
  LCD_WriteRegister(0x0098,0x0000);

  // Display on sequence
  LCD_WriteRegister(0x0007,0x0133);
  LCD_WriteRegister(0x0020,0x0000);
  LCD_WriteRegister(0x0021,0x0000);

  LCD_Light_On;
}

/*
 * Name: void LCD_SetCursor(u16 x,u16 y)
 * Function: Set the screen coordinates
 * Input: x,y-positions
 * Output: none
 * Call: LCD_SetCursor(50,50);
 */
inline void LCD_SetCursor(u16 x,u16 y)
{
#if LCD_ORIENTATION == LCD_LANDSCAPE
  LCD_WriteRegister(0x20,y);
  LCD_WriteRegister(0x21,x);
#else
  LCD_WriteRegister(0x20,x);
  LCD_WriteRegister(0x21,y);
#endif
}

/*
 * Name: void LCD_SetWindow(u16 Startx,u16 Starty,u16 Endx,u16 Endy)
 * Function: Set window region
 * Input: Start/end x,y-position of window
 * Output: none
 * Call: LCD_SetWindow(0,0,240,320);
 */
__inline void LCD_SetWindow(u16 Startx,u16 Starty,u16 Endx,u16 Endy)
{

  int temp;
  // check some sanity
  if(Endy < Starty) {
    temp = Starty;
    Starty = Endy;
    Endy = temp;
  }

  if(Endx < Startx) {
    temp = Startx;
    Startx = Endy;
    Endy = temp;
  }

#if LCD_ORIENTATION == LCD_LANDSCAPE
  LCD_WriteRegister(0x50,Starty);
  LCD_WriteRegister(0x52,Startx);
  LCD_WriteRegister(0x51,Endy);
  LCD_WriteRegister(0x53,Endx);
#else
  LCD_WriteRegister(0x50,Startx);
  LCD_WriteRegister(0x52,Starty);
  LCD_WriteRegister(0x51,Endx);
  LCD_WriteRegister(0x53,Endy);
#endif

  LCD_SetCursor(Startx,Starty);

}

/*
 * Name: void LCD_Clear(u16 Color)
 * Feature: fill screen to specified color
 * Input: fill color
 * Output: none
 * Call: LCD_Clear (0xffff);
 */
void LCD_Clear(u16 Color)
{
  u32 i;
  LCD_SetCursor(0,0);
  Clr_Cs;
  LCD_WriteIndex(0x22);         // GRAM access port
  Set_Rs;
  for (i = 0;i < 76800;i++)
    {
      LCD_WriteData(Color);
      Clr_nWr;
      Set_nWr;
    }

  Set_Cs;
}

void LCD_FillRect(u16 Startx, u16 Starty, u16 Endx, u16 Endy, u16 Color)
{
  u32 i;
  LCD_SetWindow(Startx,Starty,Endx,Endy);
  LCD_SetCursor(Startx,Starty);
  Clr_Cs;
  LCD_WriteIndex(0x22);         // GRAM access port
  Set_Rs;
  for (i=(Endx-Startx+1)*(Endy-Starty);i > 0;i--)
    {
      LCD_WriteData(Color);
      Clr_nWr;
      Set_nWr;
    }

  Set_Cs;

  LCD_WriteRegister(0x0050,0x0000);
  LCD_WriteRegister(0x0051,0x00ef);
  LCD_WriteRegister(0x0052,0x0000);
  LCD_WriteRegister(0x0053,0x013f);
}



// todo: handle rendering off the screen
void LCD_HorizontalLine(u16 Startx, u16 Endx, u16 y, u16 Color){
  u16 i;
  LCD_SetCursor(Startx,y);
  Clr_Cs;
  LCD_WriteIndex(0x22);
  Set_Rs;
  for (i=Endx; i > Startx;i--)
    {
      LCD_WriteData(Color);
      Clr_nWr;
      Set_nWr;
    }
  Set_Cs;
}

// todo: handle rendering off the screen
void LCD_VerticalLine(u16 x, u16 Starty, u16 Endy, u16 Color){

  u16 i;
  u16 oldAM = LCD_ReadRegister(0x03);
  LCD_WriteRegister(0x03,oldAM|0x0008);
  LCD_SetCursor(x,Starty);
  Clr_Cs;
  LCD_WriteIndex(0x22);
  Set_Rs;
  for (i=Endy; i > Starty;i--)
    {
      LCD_WriteData(Color);
      Clr_nWr;
      Set_nWr;
    }
  Set_Cs;
  LCD_WriteRegister(0x03,oldAM);
}

void LCD_DrawRect(u16 Startx, u16 Starty, u16 Endx, u16 Endy, u16 Color)
{
  // everything is optimized internally.
  LCD_HorizontalLine(Startx,Endx,Starty,Color);
  LCD_HorizontalLine(Startx,Endx,Endy,Color);
  LCD_VerticalLine(Startx,Starty,Endy,Color);
  LCD_VerticalLine(Endx,Starty,Endy,Color);
}


void LCD_DrawLine(u16 x0, u16 x1, u16 y0, u16 y1, u16 Color) {
  // check if we can optimize stuff.
  if(x0 == x1) {
    LCD_VerticalLine(x0, y0, y1, Color);
    return;
  }

  if(y0 == y1) {
    LCD_HorizontalLine(x0, x1, y0, Color);
    return;
  }

  // bresenham
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;

  int err = (dx>dy ? dx : -dy)/2, e2;

  while(x0 != x1 && y0 != y1) {
    LCD_SetPoint(x0,y0,Color);
    e2 = err;
    if(e2 > -dx) { err -= dy; x0 += sx;}
    if(e2 < dy) { err += dx; y0 += sy;}
  }

}


/*
 * Name: void LCD_SetPoint(u16 x,u16 y,u16 Color)
 * Function: Draw point at x,y
 * Input: x,y-position, point color
 * Output: None
 * Call: LCD_SetPoint(50,50,0x0000);
 */
void LCD_SetPoint(u16 x,u16 y,u16 Color)
{
#if LCD_ORIENTATION == LCD_LANDSCAPE
  if ((x > 320) || (y > 240))
    {
      return;
    }
#else
  if ((x > 240) || (y > 320))
    {
      return;
    }
#endif

  LCD_SetCursor(x,y);
  LCD_WR_Start();
  LCD_WriteData(Color);
  Clr_nWr; Set_nWr;
  LCD_WR_End();
}

void LCD_DrawPicture1bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy,
                         u8 *pic, u16 foreground, u16 background)
{
  u32 i;
  u8 shift;
  u32 index;
  LCD_SetWindow(Startx,Starty,Endx,Endy);
  LCD_SetCursor(Startx,Starty);
  Clr_Cs;
  LCD_WriteIndex(0x22);         // GRAM access port
  Set_Rs;
  for (i=0;i < (Endx-Startx)*(Endy-Starty);i++)
    {
      shift = i&0x07;
      index = i&0xFFFFFF80;
      LCD_WriteData(*(pic+index)&(1<<shift) ? foreground : background);
      Clr_nWr;
      Set_nWr;
    }

  Set_Cs;

  LCD_WriteRegister(0x0050,0x0000);
  LCD_WriteRegister(0x0051,0x00ef);
  LCD_WriteRegister(0x0052,0x0000);
  LCD_WriteRegister(0x0053,0x013f);
}

// I can't see myself using these anytime soon, so I'll just leave
// these here instead. Watching, waiting.
/* void LCD_DrawPicture4bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy, */
/*                          u8 *pic) */
/* { */
/*   u32 cond = (Endx-Startx)*(Endy-Starty); */
/*   LCD_SetWindow(Startx,Starty,Endx,Endy); */
/*   LCD_SetCursor(Startx,Starty); */
/*   Clr_Cs; */
/*   LCD_WriteIndex(0x22);         // GRAM access port */
/*   Set_Rs; */
/*   while(cond != 0) { */
/*     LCD_WriteData(GFX_121_TO_565(*pic>>4)); Clr_nWr; Set_nWr; */
/*     LCD_WriteData(GFX_121_TO_565(*pic&0xFF)); Clr_nWr; Set_nWr; */
/*     pic++; */
/*   } */

/*   Set_Cs; */

/*   LCD_WriteRegister(0x0050,0x0000); */
/*   LCD_WriteRegister(0x0051,0x00ef); */
/*   LCD_WriteRegister(0x0052,0x0000); */
/*   LCD_WriteRegister(0x0053,0x013f); */
/* } */

/* void LCD_DrawPicture8bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy, */
/*                          u8 *pic) */
/* { */
/*   u32 cond = (Endx-Startx)*(Endy-Starty); */
/*   LCD_SetWindow(Startx,Starty,Endx,Endy); */
/*   LCD_SetCursor(Startx,Starty); */
/*   Clr_Cs; */
/*   LCD_WriteIndex(0x22);         // GRAM access port */
/*   Set_Rs; */
/*   while(cond != 0) { */
/*     LCD_WriteData(GFX_332_TO_565(*pic)); Clr_nWr; Set_nWr; */
/*   } */

/*   Set_Cs; */

/*   LCD_WriteRegister(0x0050,0x0000); */
/*   LCD_WriteRegister(0x0051,0x00ef); */
/*   LCD_WriteRegister(0x0052,0x0000); */
/*   LCD_WriteRegister(0x0053,0x013f); */
/* } */

void LCD_DrawPicture16bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy,
                         u16 *pic)
{
  u32 cond = (Endx-Startx)*(Endy-Starty);
  LCD_SetWindow(Startx,Starty,Endx,Endy);
  LCD_SetCursor(Startx,Starty);
  Clr_Cs;
  LCD_WriteIndex(0x22);         // GRAM access port
  Set_Rs;
  while(cond != 0) {
    LCD_WriteData(*pic); Clr_nWr; Set_nWr;
  }

  Set_Cs;

  LCD_WriteRegister(0x0050,0x0000);
  LCD_WriteRegister(0x0051,0x00ef);
  LCD_WriteRegister(0x0052,0x0000);
  LCD_WriteRegister(0x0053,0x013f);
}


// TODO: make this draw chars in different orientations
// TODO: enable transparency for different background colors
// TODO: Different colors for text
// TODO: Figure out how to prevent clipping
void LCD_DrawChar(u16 Startx,u16 Starty,u8 c, u16 foreground, u16 background)
{
  if(Startx > 240 || Starty > 320) {
    return;
  }

  u16 i,j;
  u16 width = Startx+FONT_WIDTH > LCD_WIDTH ? LCD_WIDTH-Startx : FONT_WIDTH;
  u16 height = Starty+FONT_HEIGHT > LCD_HEIGHT ? LCD_HEIGHT-Starty : FONT_HEIGHT;
  LCD_SetWindow(Startx,Starty,Startx+width-1,Starty+height-1 );
  LCD_SetCursor(Startx,Starty);
  Clr_Cs;
  LCD_WriteIndex(0x22);         // GRAM access port
  Set_Rs;
  for (i = 0;i < height;i++) {
    for(j = 0; j < width; j++) {
      if (smallfont[c-32][j] & (1<<i)) {
        LCD_WriteData(foreground);
      } else {
        LCD_WriteData(background);
      }
      Clr_nWr;Set_nWr;
    }
  }
  Set_Cs;

  LCD_WriteRegister(0x0050,0x0000);
  LCD_WriteRegister(0x0051,0x00ef);
  LCD_WriteRegister(0x0052,0x0000);
  LCD_WriteRegister(0x0053,0x013f);

}

// this is kinda the same speed as the fast memory setting method.
void LCD_DrawCharTrans(u16 Startx,u16 Starty,u8 c, u16 foreground)
{
  u16 i,j;
  for (i = FONT_HEIGHT;i != 0;i--) {
    for(j = FONT_WIDTH; j != 0; j--) {
      if (smallfont[c-32][j] & (1<<i)) {
        LCD_SetPoint(Startx+j,Starty+i,foreground);
      }
    }
  }

}


// it would be nice to make this nicer for different rotations etc
// todo: figure out how to disable clipping for non-trans strings
void LCD_DrawString(u16 Startx, u16 Starty, u16 foreground,
                    u16 background, u8 trans, const char* s) {
  u16 x = Startx;
  u16 y = Starty;
  while (*s != '\0' && *s != '\n') {
    if(trans) {
      LCD_DrawCharTrans(x,y,*s, foreground);
    } else {
      LCD_DrawChar(x,y,*s,foreground,background);
    }

    s++;
    x += FONT_WIDTH;
  }
}

/*
 * Name: void LCD_Test()
 * Function: test the LCD
 * Input: none
 * Output: none
 * Call: LCD_Test();
 */
void LCD_Test()
{
  int x,y;
  u16 dat;

  for(x = 100; x >= 50; x--) {
    for(y = 0; y < 320; y++) {
      dat = LCD_GetPixel(x,y);
      dat = ~dat;
      LCD_SetPoint(x,y,dat);
    }
  }



  /* u8 R_data,G_data,B_data,i,j; */

  /* LCD_SetCursor(0,0); */
  /* LCD_WriteRegister(0x50,0);         // Set horizontal start of window at 0 */
  /* LCD_WriteRegister(0x51,239);       // Set horizontal end of window at 239 */
  /* LCD_WriteRegister(0x52,0);         // Set vertical start of window at 0 */
  /* LCD_WriteRegister(0x53,319);       // Set vertical end of window at 319 */
  /* LCD_WR_Start(); */
  /* R_data = 0;G_data = 0;B_data = 0; */
  /* /\********** RED **********\/ */
  /* for (j = 0;j < 50;j++) */
  /*   { */
  /*     for (i = 0;i < 240;i++) */
  /*       { */
  /*         R_data = i / 8; */
  /*         LCD_WriteData(R_data << 11 | G_data << 5 | B_data); */
  /*         Clr_nWr;Set_nWr; */
  /*       } */
  /*   } */
  /* R_data = 0x1f;G_data = 0x3f;B_data = 0x1f; */
  /* for (j = 0;j < 50;j++) */
  /*   { */
  /*     for (i = 0; i < 240;i++) */
  /*       { */
  /*         G_data = 0x3f - (i / 4); */
  /*         B_data = 0x1f - (i / 8); */
  /*         LCD_WriteData(R_data << 11 | G_data << 5 | B_data); */
  /*         Clr_nWr;Set_nWr; */
  /*       } */
  /*   } */
  /* /\********** GREEN **********\/ */
  /* R_data = 0;G_data = 0;B_data = 0; */
  /* for (j = 0;j < 50;j++) */
  /*   { */
  /*     for (i = 0;i < 240;i++) */
  /*       { */
  /*         G_data = i / 4; */
  /*         LCD_WriteData(R_data << 11 | G_data << 5 | B_data); */
  /*         Clr_nWr;Set_nWr; */
  /*       } */
  /*   } */
  /* R_data = 0x1f;G_data = 0x3f;B_data = 0x1f; */
  /* for (j = 0;j < 50;j++) */
  /*   { */
  /*     for (i = 0;i < 240;i++) */
  /*       { */
  /*         R_data = 0x1f - (i / 8); */
  /*         B_data = 0x1f - (i / 8); */
  /*         LCD_WriteData(R_data << 11 | G_data << 5 | B_data); */
  /*         Clr_nWr;Set_nWr; */
  /*       } */
  /*   } */
  /* /\********** BLUE **********\/ */
  /* R_data = 0;G_data = 0;B_data = 0; */
  /* for (j = 0;j < 60; j++) */
  /*   { */
  /*     for(i = 0;i < 240;i++) */
  /*       { */
  /*         B_data = i / 8; */
  /*         LCD_WriteData(R_data << 11 | G_data << 5 | B_data); */
  /*         Clr_nWr;Set_nWr; */
  /*       } */
  /*   } */
  /* R_data = 0x1f;G_data = 0x3f;B_data = 0x1f; */
  /* for (j = 0;j < 60;j++) */
  /*   { */
  /*     for (i = 0;i < 240;i++) */
  /*       { */
  /*         G_data = 0x3f - (i / 4); */
  /*         R_data = 0x1f - (i / 8); */
  /*         LCD_WriteData(R_data << 11 | G_data << 5 | B_data); */
  /*         Clr_nWr;Set_nWr; */
  /*       } */
  /*   } */
  /* LCD_WR_End(); */
}

/*
 * Name: u16 LCD_BGR2RGB(u16 color)
 * Function: RRRRRGGGGGGBBBBB to BBBBBGGGGGGRRRRR format
 * Input: BRG color
 * Output: RGB color
 * Call: LCD_BGR2RGB(0xfefefe);
 */
u16 LCD_BGR2RGB(u16 color)
{
  u16 r,g,b,rgb;
  b = (color >> 0) & 0x1f;
  g = (color >> 5) & 0x3f;
  r = (color >> 11) & 0x1f;

  rgb = (b << 11) + (g << 5) + (r << 0);
  return(rgb);
}



/*
 * Name: void LCD_WriteIndex(u16 idx)
 * Function: write register address
 * Input: register
 * Output: none
 * Call: LCD_WriteIndex(0x20);
 */
__inline void LCD_WriteIndex(u16 idx)
{
  Clr_Rs;
  Set_nRd;
  LCD_WriteData(idx);
  Clr_nWr;
  Set_nWr;
  Set_Rs;
}

/*
 * Name: void LCD_WriteData(u16 data)
 * Function: write register data
 * Input: register data
 * Output: none
 * Call: LCD_WriteData(0x0000);
 */
inline void LCD_WriteData(u16 data)
{
  // assume that cs is already low
  ((uint8_t __IO*)&GPIOA->ODR)[0] = data>>8;
  Clr_nWr;
  Set_nWr;
  ((uint8_t __IO*)&GPIOA->ODR)[0] = data;

  // have to raise cs later
}

/*
 * Name: void LCD_WR_Start(void)
 * Function: Prepare write
 * Input: none
 * Output: none
 * Call: LCD_WR_Start();
 */
inline void LCD_WR_Start(void)
{

  Clr_Cs;
  Clr_Rs;
  Set_nRd;
  LCD_WriteData(0x22);
  Clr_nWr;
  Set_nWr;
  Set_Rs;
}

/*
 * Name: void LCD_WR_End(void)
 * Function: end write
 * Input: none
 * Output: none
 * Call: LCD_WR_End();
 */
inline void LCD_WR_End(void)
{
  Set_Cs;
}

/*
 * Name: u16 LCD_ReadData(void)
 * Function: read controller data
 * Input: none
 * Output: data
 * Call: x = LCD_ReadData();
 */
__inline u16 LCD_ReadData(void)
{
  // need to make this faster somehow...
  u16 temp;
  GPIOA->CRL = (GPIOA->CRL & 0x00000000) | 0x44444444; // set data
                                                       // pins as
                                                       // input
  Clr_nRd;
  // leave the nops so that the LCD has a chance to actually write
  // before we read. Otherwise we get junk.
  // min 4 for 72MHz
  __asm__("nop");
  __asm__("nop");
  __asm__("nop");
  __asm__("nop");
  temp = ((uint8_t __IO*)&GPIOA->IDR)[0]<<8; // read high byte
  Set_nRd; Clr_nRd;
  __asm__("nop");
  __asm__("nop");
  __asm__("nop");
  __asm__("nop");
  temp += ((uint8_t __IO*)&GPIOA->IDR)[0]; // read low byte

  GPIOA->CRL = (GPIOA->CRL & 0x00000000) | 0x33333333; // set pins as
                                                       // output again

  return temp;
}

/*
  Name: LCD_GetPixel(u16 x, u16 y)
  Function: Get value of pixel
  Input: pixel coords
  Output: pixel data
  Call: x = LCD_GetPixel(0,0);
  Note: This is actually REALLY slow compared to other lcd
  methods. Please do not overuse it.
*/
u16 LCD_GetPixel(u16 x, u16 y) {
  u16 dat;
  // dummy read (copy of LCD_Register with some changes)
  LCD_SetCursor(x,y);
  Clr_Cs;
  LCD_WriteIndex(0x22);
  Clr_nRd;
  LCD_ReadData();
  LCD_WriteIndex(0x22);
  Clr_nRd;
  // proper read now that the data has been clocked into the register.
  dat = LCD_ReadData();
  Set_nRd;
  Set_Cs;

  return dat;

}

/*
 * Name: u16 LCD_ReadRegister(u16 index)
 * Function Read the value of the address register
 * Input: index of the register you want to read
 * Output: Register value
 * Call: x = LCD_ReadRegister(0x22);
 */
__inline u16 LCD_ReadRegister(u16 index)
{
  Clr_Cs;
  LCD_WriteIndex(index);
  Clr_nRd;
  index = LCD_ReadData();
  Set_nRd;
  Set_Cs;
  return index;
}

/*
 * Name: void LCD_WriteRegister(u16 index,u16 data)
 * Function: Write data to register
 * Input: index of register and data to go there
 * Output: none
 * Call: LCD_WriteRegister(0x00,0x0001);
 */
__inline void LCD_WriteRegister(u16 index,u16 data)
{
  Clr_Cs;
  Clr_Rs;
  Set_nRd;
  LCD_WriteData(index);

  Clr_nWr;Set_nWr;
  Set_Rs;
  LCD_WriteData(data);

  Clr_nWr;Set_nWr;
  Set_Cs;
}

/*
 * Name: void LCD_Reset()
 * Function: reset ili9320 controller
 * Input: none
 * Output: none
 * Call: LCD_Reset();
 */
void LCD_Reset()
{
  Set_Rst;
  LCD_Delay(50000);
  Clr_Rst;
  LCD_Delay(50000);
  Set_Rst;
  LCD_Delay(50000);
}

/*
 * Name: LCD_Backlight(u16 status)
 * Function: turn on or off backlight
 * Input: 1 for on, 0 for off
 * Output: none
 * Call: LCD_Backlight(1);
 */
void LCD_Backlight(u16 status)
{
  if(status >= 1)
    {
      LCD_Light_On;
    }
  else
    {
      LCD_Light_Off;
    }
}

/*
 * Name: void LCD_Delay(vu32 nCount)
 * Function: Delay nCount many counts
 * Input: nCount, number of counts
 * Output: none
 * Call: LCD_Delay(50000);
 */
void LCD_Delay(vu32 nCount)
{
  for(;nCount != 0;nCount--);
}

/*
 * Name: void LCD_SetOrienatation(LCD_OrientationMode_t m)
 * Function: sets the orientation of the LCD
 * Input: orientation mode
 * Output: none
 * Call: LCD_SetOrientation(mode);
 */
void LCD_SetOrientation(LCD_OrientationMode_t m)
{
  uint16_t em;
  switch (m)
    {
    case LCD_PORTRAIT_TOP_DOWN:
      em = 0x1030;
      break;
    case LCD_PORTRAIT_BOTTOM_UP:
      em = 0x1010;
      break;
    case LCD_LANDSCAPE_TOP_DOWN:
      em = 0x1028;
      break;
    case LCD_LANDSCAPE_BOTTOM_UP:
      em = 0x1008;
      break;
    default:
      em = 0x0130;
      break;
    }
  LCD_WriteRegister(0x0003,em);
  orientation_mode = m;
  LCD_SetCursor(0,0);
}

/*
 * Name: LCD_OrientationMode_t LCD_GetOrientation(void)
 * Function: returns the current orientation of the LCD
 * Input: none
 * Output: orientation
 * Call: x = LCD_GetOrientation();
 */
LCD_OrientationMode_t LCD_GetOrientation(void)
{
  return orientation_mode;
}

/*
 * Name: uint16_t LCD_GetWidth(void)
 * Function: Get the width of the LCD in pixels in the current orientation
 * Input: none
 * Output: width
 * Call: x = LCD_GetWidth();
 */
uint16_t LCD_GetWidth(void)
{
  switch (orientation_mode)
    {
    case LCD_LANDSCAPE_TOP_DOWN:
    case LCD_LANDSCAPE_BOTTOM_UP:
      return LCD_HEIGHT_HW;
    case LCD_PORTRAIT_TOP_DOWN:
    case LCD_PORTRAIT_BOTTOM_UP:
    default:
      return LCD_WIDTH_HW;
    }
}

/*
 * Name: uint16_t LCD_GetHeight(void)
 * Function: Get the height of the LCD in pixels in the current orientation
 * Input: none
 * Output: height
 * Call: y = LCD_GetHeight();
 */
uint16_t LCD_GetHeight(void)
{
  switch (orientation_mode)
    {
    case LCD_LANDSCAPE_TOP_DOWN:
    case LCD_LANDSCAPE_BOTTOM_UP:
      return LCD_WIDTH_HW;
    case LCD_PORTRAIT_TOP_DOWN:
    case LCD_PORTRAIT_BOTTOM_UP:
    default:
      return LCD_HEIGHT_HW;
    }
}
