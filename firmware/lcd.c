/********************************************************************
* Lcd.c
* 
* A set of general purpose LCD utilities. This module should not be
* used with a preemptive kernel without protection of the shared LCD.
*  
* Controller: K70 PTD1-6, 4-bit mode 
* 
* Originally the 9S12 LCD module from Andrew Pace, 2/6/99, ET 454 
* MOdified for the K70. Todd Morton, 2/24/2013 
* Modified for avr Jeremy Ruhland & Oneil Kwangwanh 5/2014 
********************************************************************
* Master Include File  
*******************************************************************/
#include "includes.h"

/*******************************************************************
* LCD Port Defines 
*******************************************************************/
#define LCD_RS_BIT     (1<<PC4)
#define LCD_E_BIT      (1<<PC5)
#define LCD_DB_MASK	   0x0f
#define LCD_PORT       PORTC
#define LCD_PORT_DIR   DDRC
#define INIT_BIT_DIR() (LCD_PORT_DIR |= (LCD_RS_BIT|LCD_E_BIT|LCD_DB_MASK))
#define LCD_SET_RS()   LCD_PORT |= LCD_RS_BIT
#define LCD_CLR_RS()   LCD_PORT &= ~(LCD_RS_BIT)
#define LCD_SET_E()    LCD_PORT |= LCD_E_BIT
#define LCD_CLR_E()    LCD_PORT &= ~(LCD_E_BIT)
#define LCD_WR_DB(nib) (LCD_PORT = (LCD_PORT & ~LCD_DB_MASK)|((nib)))

/********************************************************************
* LCD Defines                                                 *
********************************************************************/
#define NUM_CHARS     16      /* 16 character display */
#define LCD_DAT_INIT  0x28    /*Data length: 4 bit. Lines: 2. Font: 5x7 dots.*/
#define LCD_SHIFT_CUR 0x06    /*Increments cursor addr after write.*/
#define LCD_DIS_INIT  0x0C    /*Display: on. Cursor: off. Blink: off */
#define LCD_CLR_CMD   0x01    /*Clear display and move cursor home */

#define LCD_LINE1_ADDR 0x80   /* Display address for line1 column1 */
#define LCD_LINE2_ADDR 0xC0   /* Display address for line2 column1 */
#define LCD_BS_CMD     0x10   /* Move cursor left one space */
#define LCD_FS_CMD     0x14   /* Move cursor right one space */

/********************************************************************
* Public Function prototypes
********************************************************************/
void LcdInit(void);
void LcdClrDisp(void);
void LcdClrLine(uint8_t line);
void LcdDispChar(uint8_t c);
void LcdDispByte(uint8_t *b);
void LcdDispStrg(uint8_t *s);
void LcdDispStrgP(uint8_t *s);
void LcdMoveCursor(uint8_t row, uint8_t col);
void LcdDispDecByte(uint8_t *b, uint8_t lz);
void LcdDispTime(uint8_t hrs, uint8_t mins, uint8_t secs);
void LcdCursor(uint8_t on, uint8_t blink);
void LcdBSpace(void);
void LcdFSpace(void);

/* Private */
static void LcdWrCmd(uint8_t cmd);
static uint8_t i;
static const uint8_t mainCustomChars[] PROGMEM = {0x0e, 0x11, 0x04, 0x0A, 0x00, 0x04, 0x04, 0x04, 0x00, 0x02, 0x06, 0x0e, 0x06, 0x02, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

/********************************************************************
* Function Definitions
*********************************************************************
* LcdWrCmd(uint8_t cmd) - Private
*
*  PARAMETERS: cmd - Command to be sent to the LCD
*
*  DESCRIPTION: Sends a command write sequence to the LCD
*
********************************************************************/
static void LcdWrCmd(uint8_t cmd) {
      LCD_CLR_RS();				//Select command
      LCD_WR_DB(cmd>>4);		//Out most sig nibble
      LCD_SET_E();				//Pulse E. 230ns min per Seiko doc
      _delay_us(1);
      LCD_CLR_E();
      _delay_us(1);			//Wait >1us per Seiko doc
      LCD_WR_DB((cmd&0x0f));	//Out least sig nibble
      LCD_SET_E();				//Pulse E
      _delay_us(1);
      LCD_CLR_E();
      _delay_us(40);				//Wait 40us per Seiko doc
      LCD_SET_RS();				//Set back to data
}

/********************************************************************
* LcdInit()
*
*  PARAMETERS: None
*
*  DESCRIPTION: Initialises LCD ports to outputs and sends LCD reset
*               sequence per Seiko Data sheet. In this case, 4-bit
*               mode.
*
********************************************************************/
void LcdInit(void) {
   
	LCD_PORT_DIR = 0xFF;
	INIT_BIT_DIR();
    LCD_CLR_E(); 
    LCD_SET_RS();           /*Data select unless in LcdWrCmd()  */
    _delay_ms(15);  	        /* LCD requires 15ms delay at powerup */ 
   
    LCD_CLR_RS();		    /*Send first command for RESET sequence*/
    LCD_WR_DB(0x3);
    LCD_SET_E();
    _delay_us(1);
    LCD_CLR_E();
    _delay_ms(5);		    /*Wait >4.1ms */ 
  
    LCD_WR_DB(0x3);	        /*Repeat */
    LCD_SET_E();
    _delay_us(1);
    LCD_CLR_E();
    _delay_ms(1); 		    /*Wait >100us */
  
    LCD_WR_DB(0x3);	        /* Repeat */
    LCD_SET_E();
    _delay_us(1);
    LCD_CLR_E();
    _delay_us(40); 		    /*Wait >40us*/
  
    LCD_WR_DB(0x2);	        /*Send last command for RESET sequence*/
    LCD_SET_E();
    _delay_us(1);
    LCD_CLR_E();
    _delay_us(40); 
  
    LcdWrCmd(LCD_DAT_INIT);	    /*Send command for 4-bit mode */
    LcdWrCmd(LCD_SHIFT_CUR);
    LcdWrCmd(LCD_DIS_INIT);
    LcdClrDisp();

    // Create custom chars
    LcdWrCmd(0x40);
    LCD_SET_RS();
    for (i = 0; i <= 23; i++) {
        LcdDispChar(pgm_read_byte(&mainCustomChars[i]));
    }
    LcdMoveCursor(1,1);
} 

/********************************************************************
** LcdDispChar(uint8_t c) - Public
*
*  PARAMETERS: c - ASCII character to be sent to the LCD
*
*  DESCRIPTION: Displays a character at current LCD address. Assumes
*               that the LCD port is configured for a data write.
*
********************************************************************/
void LcdDispChar(uint8_t c) {
    LCD_WR_DB((c>>4));
    LCD_SET_E();
    _delay_us(1);
    LCD_CLR_E();
    _delay_us(1);
    LCD_WR_DB((c&0x0f));
    LCD_SET_E();
    _delay_us(1);
    LCD_CLR_E();
    _delay_us(40);
}

/********************************************************************
** LcdClrDisp
*
*  PARAMETERS: None
*
*  DESCRIPTION: Clears the LCD display and returns the cursor to
*               row1, col1.
*
********************************************************************/
void LcdClrDisp(void) {

    LcdWrCmd(LCD_CLR_CMD);
    _delay_ms(2);
}

/********************************************************************
** LcdClrLine
*
*  PARAMETERS: line - Line to be cleared (1 or 2).
*
*  DESCRIPTION: Writes spaces to every location in a line and then 
*               returns the cursor to column 1 of that line.
*
********************************************************************/
void LcdClrLine (uint8_t line) {
  
   uint8_t start_addr; 
   uint8_t i;

   if(line == 1){ 
      start_addr = LCD_LINE1_ADDR;  
   }else if(line == 2){ 
      start_addr = LCD_LINE2_ADDR;
   }else{
      return;
   }
   LcdWrCmd(start_addr);
   for(i = 0x0; i < (NUM_CHARS); i++) {
      LcdDispChar(' ');
   }
   LcdWrCmd(start_addr);
}
 
/********************************************************************
* LcdDispByte(uint8_t *b)
*
*  PARAMETERS: *b - pointer to the byte to be displayed.
*
*  DESCRIPTION: Displays the byte pointed to by b in hex.
*
********************************************************************/
void LcdDispByte(uint8_t *b) {

    uint8_t upnib, lonib;

    lonib = *b & 0x0F;
    upnib = (*b & 0xF0)>>4;

    if(lonib > 9){ 
        lonib = lonib + 0x37;
    }else{
        lonib = lonib + 0x30;
    }
    if(upnib > 9){
        upnib = upnib + 0x37;
    }else{
        upnib = upnib + 0x30; 
    }
    LcdDispChar(upnib);
    LcdDispChar(lonib);
}   

/********************************************************************
* LcdDispStrg(uint8_t *s)
*
*  PARAMETERS: *s - pointer to the NULL terminated string to be 
*                   displayed.
*
*  DESCRIPTION: Displays the string pointed to by s.
*
********************************************************************/
void LcdDispStrg(uint8_t *s) {

    uint8_t *sptr = s;

    while(*sptr != 0x00) {
        LcdDispChar(*sptr);
        sptr++;
    }
}

/********************************************************************
* LcdDispStrgP(uint8_t *s)
*
*  PARAMETERS: *s - pointer to the NULL terminated string to be 
*                   displayed stored in flash memory.
*
*  DESCRIPTION: Displays the string pointed to by s.
*
********************************************************************/
void LcdDispStrgP(uint8_t *s) {

    uint8_t *sptr = s;

    while(pgm_read_byte(sptr) != 0x00) {
        LcdDispChar(pgm_read_byte(sptr));
        sptr++;
    }
}

/********************************************************************
** LcdMoveCursor(uint8_t row, uint8_t col)
*
*  PARAMETERS: row - Destination row (1 or 2).
*              col - Destination column (1 - 16).
*
*  DESCRIPTION: Moves the cursor to [row,col].
*
********************************************************************/
void LcdMoveCursor(uint8_t row, uint8_t col) {

    if(row == 1) {
        LcdWrCmd(LCD_LINE1_ADDR + col - 1);
    }else{
        LcdWrCmd(LCD_LINE2_ADDR + col - 1);
    }
}

/********************************************************************
** LcdDispTime(uint8_t hrs, uint8_t min, uint8_t sec)
*
*  PARAMETERS: hrs - Hours value.
*              min - Minutes value.
*              sec - seconds value
*
*  DESCRIPTION: Displays the time in HH:MM:SS format. 
*               First converts to decimal.
********************************************************************/
void LcdDispTime(uint8_t hrs, uint8_t min, uint8_t sec) {

    uint8_t tens, ones;

    ones = (hrs % 10) + '0';
    hrs = hrs / 10;
    tens = (hrs % 10) + '0';

    LcdDispChar(tens);
    LcdDispChar(ones);
    LcdDispChar(':');
   
    ones = (min % 10) + '0';
    min = min / 10;
    tens = (min % 10) + '0';

    LcdDispChar(tens);
    LcdDispChar(ones);
    LcdDispChar(':');
   
    ones = (sec % 10) + '0';
    sec = sec / 10;
    tens = (sec % 10) + '0';

    LcdDispChar(tens);
    LcdDispChar(ones);
   
}

/********************************************************************
** LcdDispDecByte(uint8_t *b, uint8_t lz)
*
*  PARAMETERS: *b - Pointer to the byte to be displayed.
*              lz - (Binary)Display leading zeros if TRUE.
*                   Delete leading zeros if FALSE.
*
*  DESCRIPTION: Displays the byte pointed to by b in decimal. 
*               Deletes leading zeros if lz is zero. Digits are
*               right justified if leading zeros are deleted.
*
*  RETURNS: None
********************************************************************/
void LcdDispDecByte(uint8_t *b, uint8_t lz) {

    uint8_t bin = *b;
    uint8_t huns, tens, ones;
   
    ones = (bin % 10) + '0';   /* Convert to decimal digits        */
    bin    = bin / 10;
    tens = (bin % 10) + '0';
    huns = bin / 10 + '0';

    if((huns == '0') && (!lz)){
        LcdDispChar(' ');
    }else{
        lz = TRUE;
        LcdDispChar(huns);
    }
    if((tens == '0') && (!lz)){ 
        LcdDispChar(' ');
    }else{
        LcdDispChar(tens);
    }
    LcdDispChar(ones);
}

/********************************************************************
** LcdCursor(uint8_t on, uint8_t blink)
*
*  PARAMETERS: on - (Binary)Turn cursor on if TRUE, off if FALSE.
*              blink - (Binary)Cursor blinks if TRUE.
*
*  Changes LCD cursor state - four possible combinations.
*
*  RETURNS: None
********************************************************************/
void LcdCursor(uint8_t on, uint8_t blink) {

    uint8_t curcmd = 0x0C;
    
    if(on){
        curcmd |= 0x02;
    }
    if(blink) {
        curcmd |= 0x01;
    }
    LcdWrCmd(curcmd);
}

/********************************************************************
* LcdBSpace()
*	Moves cursor back one space.
*
********************************************************************/
void LcdBSpace(void) {
    LcdWrCmd(LCD_BS_CMD);
}

/********************************************************************
* LcdFSpace()
* 	Moves cursor right one space.
*
********************************************************************/
void LcdFSpace(void) {
    LcdWrCmd(LCD_FS_CMD);
}
/********************************************************************/
