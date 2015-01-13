#ifndef __LCD_CONTROLLER_H__
#define __LCD_CONTROLLER_H__

#define uint16_t unsigned int

#define Set_Cs GPIOB->BSRR = (1<<9);
#define Clr_Cs GPIOB->BRR = (1<<9);

#define Set_Rs GPIOC->BSRR = (1<<13);
#define Clr_Rs GPIOC->BRR = (1<<13);

#define Set_nWr GPIOC->BSRR = (1<<14);
#define Clr_nWr GPIOC->BRR = (1<<14);

#define Set_nRd GPIOC->BSRR = (1<<15);
#define Clr_nRd GPIOC->BRR = (1<<15);

#define Set_Rst GPIOA->BSRR = (1<<15);
#define Clr_Rst GPIOA->BRR = (1<<15);

#define LCD_Light_On GPIOD->BSRR = (1<<5);
#define LCD_Light_Off GPIOD->BRR = (1<<5);

#define u16 unsigned int
#define u8 unsigned char
#define u32 unsigned long int



/* LCD colors */
#define LCD_White                               0xFFFF
#define LCD_Black                               0x0000
#define LCD_Blue                                0x001F
#define LCD_Red                                 0xF800
#define LCD_Green                               0x07E0
#define LCD_Cyan                                0x07FF

#define LCD_Dark_Green                          0x03E0
#define LCD_Dark_Cyan                           0x03EF
#define LCD_Maroon                              0x7800
#define LCD_Purple                              0x780F
#define LCD_Olive                               0x7BE0

#define LCD_Light_Gray                          0xC618
#define LCD_Dark_Gray                           0x7BEF
#define LCD_Dark_Blue                           0x0010
#define LCD_Magenta                             0xF81F
#define LCD_Yellow                              0xFFE0

// convert 888 to 565
#define RGB_conv(r,g,b) ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3 ))


typedef enum {
  LCD_PORTRAIT_TOP_DOWN,
  LCD_PORTRAIT_BOTTOM_UP,
  LCD_LANDSCAPE_TOP_DOWN,
  LCD_LANDSCAPE_BOTTOM_UP,
} LCD_OrientationMode_t;

#define LCD_WIDTH_HW            240
#define LCD_HEIGHT_HW           320

#define LCD_PORTRAIT            0
#define LCD_LANDSCAPE           1
#define LCD_ORIENTATION LCD_PORTRAIT

#if LCD_ORIENTATION == LCD_PORTRAIT
#define LCD_WIDTH                       240
#define LCD_HEIGHT              320
#define LCD_CHARS_PER_LINE      10
#define LCD_CHAR_LINES  13
#define LCD_ENTRY_MODE_DEFAULT 0x1030
#define LCD_ORIENTATION_DEFAULT LCD_PORTRAIT_TOP_DOWN
#else
#define LCD_WIDTH                       320
#define LCD_HEIGHT              240
#define LCD_CHARS_PER_LINE      15
#define LCD_CHAR_LINES          10
#define LCD_ENTRY_MODE_DEFAULT 0x1018
#define LCD_ORIENTATION_DEFAULT LCD_LANDSCAPE_BOTTOM_UP
#endif

#define LCD_LINE_HORIZONTAL     0x00
#define LCD_LINE_VERTICAL               0x01

/* exported functions */
void LCD_Configuration(void);
void LCD_Initialization(void);
void LCD_Reset(void);
void LCD_WriteRegister(u16 index,u16 data);
inline void LCD_SetCursor(u16 x,u16 y);
void LCD_SetWindow(u16 Startx,u16 Starty,u16 Endx,u16 Endy);
void LCD_DrawPicture1bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy,
			 u8 *pic, u16 foreground, u16 background);
void LCD_DrawPicture4bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy,
			 u8 *pic);
void LCD_DrawPicture8bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy,
			 u8 *pic);
void LCD_DrawPicture16bpp(u16 Startx,u16 Starty,u16 Endx,u16 Endy,
			 u16 *pic);
void LCD_DrawChar(u16 Startx,u16 Starty,u8 c, u16 foreground, u16 background);
void LCD_DrawString(u16 Startx, u16 Starty, u16 foreground,
		    u16 background, u8 trans, const char* s);
void LCD_SetPoint(u16 x,u16 y,u16 Color);
void LCD_Clear(u16 Color);
void LCD_FillRect(u16 Startx, u16 Starty, u16 Endx, u16 Endy, u16 Color);
void LCD_DrawRect(u16 Startx, u16 Starty, u16 Endx, u16 Endy, u16 Color);
void LCD_DrawLine(u16 Startx, u16 Starty, u16 Endx, u16 Endy, u16 Color);
void LCD_Delay(u32 nCount);
void LCD_Test(void);
void LCD_WriteData(u16 data) __attribute((always_inline));
void LCD_WriteIndex(u16 idx) __attribute((always_inline));
void LCD_Backlight(u16 status);
void LCD_WR_Start(void) __attribute((always_inline));
void LCD_WR_End(void) __attribute((always_inline));

u16 LCD_BGR2RGB(u16 color);
u16 LCD_ReadData(void);
u16 LCD_ReadRegister(u16 index);
u16 LCD_GetPixel(u16 x, u16 y);

void LCD_SetOrientation(LCD_OrientationMode_t m);
LCD_OrientationMode_t LCD_GetOrientation(void);
uint16_t LCD_GetWidth(void);
uint16_t LCD_GetHeight(void);

#endif // __LCD_CONTROLLER_H__
