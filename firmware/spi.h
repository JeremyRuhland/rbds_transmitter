/******************************************************************************
* Serial Peripheral Interface Module                                          *
*                                                                             *
* Contains functions and definitions required for 2014 ROV motherboard        *
*                                                                             *
* (void) spiInit(void)                  Function initializes SPI system into  *
*                                       0,0 (clock idles low), LSB first      *
*                                       master mode at 8Mhz.                  *
* (void) spiUpdateDac(dac_t)            Function sends data structure to dac  *
*                                                                             *
******************************************************************************/

extern void spiInit(void);
extern void spiUpdateDac(dac_t dacdata);
