/******************************************************************************
* Serial Peripheral Interface Module                                          *
*                                                                             *
* Contains functions and definitions required for 2014 ROV motherboard        *
*                                                                             *
* (void) spiInit(void)                  Function initializes SPI system into  *
*                                       0,0 (clock idles low), LSB first      *
*                                       master mode at 1Mhz.                  *
* (uint8_t) spiByte(uint8_t)            Function exchanges byte, returns      *
*                                       value left in SPI Rx buffer. Blocks   *
*                                       until Tx complete.                    *
*                                                                             *
******************************************************************************/

extern void spiInit(void);
extern void spiUpdateDac(dac_t dacdata);
