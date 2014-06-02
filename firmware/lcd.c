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
* Modified for AVR Jeremy Ruhland 6/2/14
********************************************************************
* Master Include File  
*******************************************************************/
#include "includes.h"

/*******************************************************************
* LCD Port Defines 
*******************************************************************/
#define LCD_RS_BIT     (1<<PC4)
#define LCD_E_BIT      (1<<PC5)
#define LCD_DB_MASK	   0x0F
#define LCD_PORT       PORTC
#define LCD_PORT_DIR   DDRC
#define INIT_BIT_DIR() (LCD_PORT_DIR |= (LCD_RS_BIT|LCD_E_BIT|LCD_DB_MASK))
#define LCD_SET_RS()   PORTC |= LCD_RS_BIT
#define LCD_CLR_RS()   PORTC &= ~LCD_RS_BIT
#define LCD_SET_E()    PORTC |= LCD_E_BIT
#define LCD_CLR_E()    PORTC &= ~LCD_E_BIT
#define LCD_WR_DB(nib) (PORTC = (PORTC & ~LCD_DB_MASK)|((nib)<<0))


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
void LcdClrLine(INT8U line);
void LcdDispChar(INT8U c);
void LcdDispByte(INT8U *b);
void LcdDispStrg(INT8U *s);
void LcdMoveCursor(INT8U row, INT8U col);
void LcdDispDecByte(INT8U *b, INT8U lz);
void LcdDispTime(INT8U hrs, INT8U mins, INT8U secs);
void LcdCursor(INT8U on, INT8U blink);
void LcdBSpace(void);
void LcdFSpace(void);

/* Private */
static void LcdWrCmd(INT8U cmd);
//static void LcdDly500ns(void);
#define LcdDly500ns() _delay_us(0.5)
//static void LcdDly40us(void);
#define LcdDly40us _delay_us(40)
static void LcdDlyms(INT8U ms);
static const uint8_t mainCustomChars[] PROGMEM = {0x0e, 0x11, 0x04, 0x0A, 0x00, 0x04, 0x04, 0x04, 0x00, 0x02, 0x06, 0x0e, 0x06, 0x02, 0x00, 0x00};
/********************************************************************
* Function Definitions
*********************************************************************
* LcdWrCmd(INT8U cmd) - Private
*
*  PARAMETERS: cmd - Command to be sent to the LCD
*
*  DESCRIPTION: Sends a command write sequence to the LCD
*
********************************************************************/
static void LcdWrCmd(INT8U cmd) {
      LCD_CLR_RS();				//Select command
      LCD_WR_DB(cmd>>4);		//Out most sig nibble
      LCD_SET_E();				//Pulse E. 230ns min per Seiko doc
      LcdDly500ns();
      LCD_CLR_E();
      LcdDly500ns();			//Wait >1us per Seiko doc
      LcdDly500ns();
      LCD_WR_DB((cmd&0x0f));	//Out least sig nibble
      LCD_SET_E();				//Pulse E
      LcdDly500ns();
      LCD_CLR_E();
      LcdDly40us();				//Wait 40us per Seiko doc
      LCD_SET_RS();				//Set back to data
}

/********************************************************************
* Function Definitions
*********************************************************************
* LcdWrDat(INT8U cmd) - Private
*
*  PARAMETERS: cmd - Data to be sent to the LCD
*
*  DESCRIPTION: Sends a data write sequence to the LCD
*
********************************************************************/
static void LcdWrDat(INT8U cmd) {
      LCD_SET_RS();				//Select data
      LCD_WR_DB(cmd>>4);		//Out most sig nibble
      LCD_SET_E();				//Pulse E. 230ns min per Seiko doc
      LcdDly500ns();
      LCD_CLR_E();
      LcdDly500ns();			//Wait >1us per Seiko doc
      LcdDly500ns();
      LCD_WR_DB((cmd&0x0f));	//Out least sig nibble
      LCD_SET_E();				//Pulse E
      LcdDly500ns();
      LCD_CLR_E();
      LcdDly40us();				//Wait 40us per Seiko doc
      LCD_CLR_RS();				//Set back to data
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
    uint8_t i;
    
	//PORTD_PCR1=(0|PORT_PCR_MUX(1));
	//PORTD_PCR2=(0|PORT_PCR_MUX(1));
	//PORTD_PCR3=(0|PORT_PCR_MUX(1));
	//PORTD_PCR4=(0|PORT_PCR_MUX(1));
	//PORTD_PCR5=(0|PORT_PCR_MUX(1));
	//PORTD_PCR6=(0|PORT_PCR_MUX(1));
    
	INIT_BIT_DIR();
    LCD_CLR_E(); 
    LCD_SET_RS();           /*Data select unless in LcdWrCmd()  */
    LcdDlyms(15);  	        /* LCD requires 15ms delay at powerup */ 
   
    LCD_CLR_RS();		    /*Send first command for RESET sequence*/
    LCD_WR_DB(0x3);
    LCD_SET_E();
    LcdDly500ns();
    LCD_CLR_E();
    LcdDlyms(5);		    /*Wait >4.1ms */ 
  
    LCD_WR_DB(0x3);	        /*Repeat */
    LCD_SET_E();
    LcdDly500ns();
    LCD_CLR_E();
    LcdDlyms(1); 		    /*Wait >100us */
  
    LCD_WR_DB(0x3);	        /* Repeat */
    LCD_SET_E();
    LcdDly500ns();
    LCD_CLR_E();
    LcdDly40us(); 		    /*Wait >40us*/
  
    LCD_WR_DB(0x2);	        /*Send last command for RESET sequence*/
    LCD_SET_E();
    LcdDly500ns();
    LCD_CLR_E();
    LcdDly40us(); 
  
    LcdWrCmd(LCD_DAT_INIT);	    /*Send command for 4-bit mode */
    LcdWrCmd(LCD_SHIFT_CUR);
    LcdWrCmd(LCD_DIS_INIT);
    LcdClrDisp();
    
    LcdWrCmd(0x40); // Write custom chars
    for (i = 0; i < 16; i++) {
        LcdWrDat(pgm_read_byte(&mainCustomChars[i]));
    }
    
} 

/********************************************************************
** LcdDispChar(INT8U c) - Public
*
*  PARAMETERS: c - ASCII character to be sent to the LCD
*
*  DESCRIPTION: Displays a character at current LCD address. Assumes
*               that the LCD port is configured for a data write.
*
********************************************************************/
void LcdDispChar(INT8U c) {
    LCD_WR_DB((c>>4));
    LCD_SET_E();
    LcdDly500ns();
    LCD_CLR_E();
    LcdDly500ns();
    LcdDly500ns();
    LCD_WR_DB((c&0x0f));
    LCD_SET_E();
    LcdDly500ns();
    LCD_CLR_E();
    LcdDly40us();
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
    LcdDlyms(2);
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
void LcdClrLine (INT8U line) {
  
   INT8U start_addr; 
   INT8U i;

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
* LcdDispByte(INT8U *b)
*
*  PARAMETERS: *b - pointer to the byte to be displayed.
*
*  DESCRIPTION: Displays the byte pointed to by b in hex.
*
********************************************************************/
void LcdDispByte(INT8U *b) {

    INT8U upnib, lonib;

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
* LcdDispStrg(INT8U *s)
*
*  PARAMETERS: *s - pointer to the NULL terminated string to be 
*                   displayed.
*
*  DESCRIPTION: Displays the string pointed to by s.
*
********************************************************************/
void LcdDispStrg(INT8U *s) {

    INT8U *sptr = s;

    while(*sptr != 0x00) {
        LcdDispChar(*sptr);
        sptr++;
    }
}

/********************************************************************
** LcdMoveCursor(INT8U row, INT8U col)
*
*  PARAMETERS: row - Destination row (1 or 2).
*              col - Destination column (1 - 16).
*
*  DESCRIPTION: Moves the cursor to [row,col].
*
********************************************************************/
void LcdMoveCursor(INT8U row, INT8U col) {

    if(row == 1) {
        LcdWrCmd(LCD_LINE1_ADDR + col - 1);
    }else{
        LcdWrCmd(LCD_LINE2_ADDR + col - 1);
    }
}

/********************************************************************
** LcdDispTime(INT8U hrs, INT8U min, INT8U sec)
*
*  PARAMETERS: hrs - Hours value.
*              min - Minutes value.
*              sec - seconds value
*
*  DESCRIPTION: Displays the time in HH:MM:SS format. 
*               First converts to decimal.
********************************************************************/
void LcdDispTime(INT8U hrs, INT8U min, INT8U sec) {

    INT8U tens, ones;

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
** LcdDispDecByte(INT8U *b, INT8U lz)
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
void LcdDispDecByte(INT8U *b, INT8U lz) {

    INT8U bin = *b;
    INT8U huns, tens, ones;
   
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
** LcdCursor(INT8U on, INT8U blink)
*
*  PARAMETERS: on - (Binary)Turn cursor on if TRUE, off if FALSE.
*              blink - (Binary)Cursor blinks if TRUE.
*
*  Changes LCD cursor state - four possible combinations.
*
*  RETURNS: None
********************************************************************/
void LcdCursor(INT8U on, INT8U blink) {

    INT8U curcmd = 0x0C;
    
    if(on){
        curcmd |= 0x02;
    }
    if(blink) {
        curcmd |= 0x01;
    }
    LcdWrCmd(curcmd);
}

#if 0
/********************************************************************
** LcdDly500ns(void)
*  	Delays, at least, 500ns
*   Designed for 120MHz or 150MHz clock.
 * 	Tdly >= (66.5ns)i (at 150MHz)
 * Currently set to ~532ns with i=8.
 * TDM 01/20/2013
********************************************************************/
static void LcdDly500ns(void){
	INT32U i;
	for(i=0;i<8;i++){
	}
}

/********************************************************************
** LcdDly40us(void)
* 	Clock frequency independent because it uses LcdDly500ns.
********************************************************************/
static void LcdDly40us(void){
    INT8U cnt;
    for(cnt=80;cnt > 0;cnt--){
        LcdDly500ns();
    }
}
#endif

/********************************************************************
** LcdDlyms(INT8U ms)
*  	Delays, at least, ms milliseconds. Maximum ~255ms.
*  	Note, based on LcdDly500ns() so not very accurate but always
*  	greater than ms milliseconds
********************************************************************/
static void LcdDlyms(INT8U ms){
    #if 0
    INT32U cnt;
    for(cnt=ms*2000;cnt>0;cnt--){
        LcdDly500ns();
    }
    #endif
    INT8U msCnt;
    for (msCnt = ms; msCnt > 0; msCnt--) {
        _delay_ms(1);
    }
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
