/*******************************************************************
** Lcd.h
*
*    DESCRIPTION: LCD.c module header file
*
*    AUTHOR: Todd Morton
*
*    HISTORY: 01/29/2014    
*
*********************************************************************
* WWULCD Function prototypes                                        *
********************************************************************/
extern void LcdInit(void);          /* Initializes display. Takes  */
                                    /* ~24ms to run                */
                                    
extern void LcdClrDisp(void);       /* Clears display and returns  */
                                    /* cursor to (1,1)             */
                                    /* Takes >2ms                  */
                                    
extern void LcdClrLine(uint8_t line); /* Clears line. Legal line     */
                                    /* numbers are 1 and 2.        */
                                    /* Takes > 756us               */
                                    
extern void LcdDispChar(uint8_t c);   /* Displays ASCII character, c */
                                    /* Takes ~50us                 */
                                    
extern void LcdDispByte(uint8_t *b);  /* Displays byte, b, in hex    */
                                    /* Takes ~100us                 */
                                    
extern void LcdDispStrg(uint8_t *s);  /* Displays string pointed to  */
                                    /* by *s. Note, that the string*/
                                    /* must fit on the line. (i.e. */
                                    /* no provision is made for    */
                                    /* wrapping.                   */
                                    /* Takes 50us times the string */
                                    /* length.                     */
                                     
extern void LcdMoveCursor(uint8_t row, uint8_t col); /* Moves cursor to*/
                                    /* row row and col column. row */
                                    /* can be 1 or 2. col can be 1 */
                                    /* through 16.                 */
                                    /* Takes ~50us                 */
                                    
extern void LcdDispDecByte(uint8_t *b, uint8_t lz); /* Displays the    */
                                    /* byte pointed to by *b in    */
                                    /* decimal. If lz is one,      */
                                    /* leading zeros are displayed.*/
                                    /* If lz is 0, leading zeros   */
                                    /* are not displayed but digits*/
                                    /* remain right justified.     */
                                    /* Takes ~150us                */
                                    
extern void LcdDispTime(uint8_t hrs, uint8_t mins, uint8_t secs);
                                    /* Displays hrs:mins:secs      */
                                    /* Each is displayed as 2      */
                                    /* decimal digits.             */
                                    /* Take ~400us                 */ 
                                    
extern void LcdCursor(uint8_t on, uint8_t blink); /* Configures cursor */
                                    /* If on is TRUE cursor is on. */
                                    /* If blink is TRUE, the cursor*/
                                    /* blinks.                     */
                                    /* Takes ~50us                 */

extern void LcdBSpace(void);        /* move cursor left one space  */
                                    /* Takes ~50us                 */
extern void LcdFSpace(void);        /* move cursor right one space */
                                    /* Takes ~50us                 */

/********************************************************************/
