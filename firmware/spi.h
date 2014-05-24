/******************************************************************************
* Serial Peripheral Interface Module                                          *
*                                                                             *
* Contains functions and definitions required for 2014 ROV motherboard        *
*                                                                             *
* (void) spiInit(void)                  Function initializes SPI system into  *
*                                       0,0 (clock idles low), LSB first      *
*                                       master mode at 1Mhz.                  *
* (void) spiPeripheralSelect(uint8_t)   Function activates proper chip select *
*                                       line for peripheral port. Accepts     *
*                                       8-bit value ranging from 1-4          *
*                                       (0x01-0x04). Out of range argument    *
*                                       will disable all peripherals.         *
* (uint8_t) spiByte(uint8_t)            Function exchanges byte, returns      *
*                                       value left in SPI Rx buffer. Blocks   *
*                                       until Tx complete.                    *
*                                                                             *
******************************************************************************/

extern void spiInit(void);
extern void spiPeripheralSelect(uint8_t peripheral);
extern uint8_t spiByte(uint8_t byte);
