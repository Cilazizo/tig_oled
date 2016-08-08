/******************************************************************************
 * File Name   :  OLED_SSD1306.h
 * Author      :  43oh - MSP430 News Projects and Forums.
               :  (http://www.43oh.com)
 * Description :  Header file for OLED_SSD1306.c 
 * Date        :  October 21, 2011.
 *****************************************************************************/

#ifndef OLED_SSD1306_H_
#define OLED_SSD1306_H_

// manual logic trigger for analyzer
#define LOGIC_OUT	P2OUT
#define LOGIC_DIR	P2DIR
#define LOGIC_BIT	BIT5
#define TRIGGER	(LOGIC_OUT |= LOGIC_BIT)

// OLED PORTS CONFIG
#define OLED_SPI_DIR P1DIR
#define OLED_SPI_SEL P1SEL
#define OLED_SPI_SEL2 P1SEL2

#define OLED_IO_DIR	P2DIR
#define OLED_IO_OUT	P2OUT

#define OLED_CS_DIR P2DIR
#define OLED_CS_OUT	P2OUT

// OLED GPIO CONFIG
#define OLED_RES BIT1	// P2.1
#define OLED_DC	BIT2	// P2.2
#define OLED_CS	BIT0	// P2.0
#define OLED_SIMO BIT7	// P1.7
//#define OLED_SOMI BIT6	// P1.6; unused for OLED
#define OLED_SCLK BIT5	// P1.5

// OLED GPIO MACROS
#define OLED_SELECT (OLED_CS_OUT &= ~OLED_CS)
#define OLED_DESELECT	 (OLED_CS_OUT |= OLED_CS)
#define OLED_COMMAND (OLED_IO_OUT &= ~OLED_DC)
#define OLED_DATA	 (OLED_IO_OUT |= OLED_DC)
#define OLED_RES_LOW  (OLED_IO_OUT &= ~OLED_RES)
#define OLED_RES_HIGH (OLED_IO_OUT |= OLED_RES)

// SSD1306 PARAMETERS
#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 64
#define SSD1306_MAXROWS 7
#define SSD1306_MAXCONTRAST 0xFF

	// command table
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

	// scrolling commands
#define SSD1306_SCROLL_RIGHT 0x26
#define SSD1306_SCROLL_LEFT 0X27
#define SSD1306_SCROLL_VERT_RIGHT 0x29
#define SSD1306_SCROLL_VERT_LEFT 0x2A
#define SSD1306_SET_VERTICAL 0xA3

	// address setting
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDRESS 0x21
#define SSD1306_COLUMNADDRESS_MSB 0x00
#define SSD1306_COLUMNADDRESS_LSB 0x7F
#define SSD1306_PAGEADDRESS	0x22
#define SSD1306_PAGE_START_ADDRESS 0xB0

	// hardware configuration
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_SEGREMAP 0xA1
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

	// timing and driving
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_NOP 0xE3

	// power supply configuration
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x10
#define SSD1306_SWITCHCAPVCC 0x20

// PROTOTYPES
void SSD1306PinSetup( void );
void SSD1306Init( void );

void SSD1306SendCommand( char *data, int i );
void SSD1306SendData( char *data, int i );
void setAddress( char page, char column );
void clearScreen(void);
void charDraw(char row, char column, int data);
void stringDraw( char row, char column, char *word);
void pixelDraw(char x, char y);
void horizontalLine(char xStart, char xStop, char y);
void verticalLine(char x, char yStart, char yStop);
void imageDraw(const char IMAGE[], char row, char column);
//void circleDraw(char x, char y, char radius);
void circleDraw(register int x, register int y, int r);

void Set_Contrast_Control(unsigned char d);
void Set_Inverse_Display(unsigned char d);
void Set_Display_On_Off(unsigned char d);
void Set_FlipScreenVertically(unsigned char d);

void Fill_RAM( char data);
void Fill_RAM_PAGE(unsigned char page, char data);

#endif /*SSD1306_OLED_H_*/
