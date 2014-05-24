/******************************************************************************
* UART Communication Module                                                   *
*                                                                             *
* Contains functions and definitions required for 2014 ROV motherboard        *
*                                                                             *
*                                                                             *
* (void) uartInit(void)         Function initializes the UART system into     *
*                               8-bit, 1 stop bit, no parity mode.            *
* (uint8_t) uartRx(void)        Function returns byte waiting in the UART Rx  *
*                               register, returns null (0x00) if empty.       *
* (void) uartTxByte(uint8_t)    Function transmits byte if UART Tx register   *
*                               empty.                                        *
* (void) uartTxWord(uint16_t)   Function transmits word as little endian,     *
*                               blocks until UART Tx register empty.          *
* (void) uartTxStrg(uint8_t *)  Function transmits null terminated string.    *
*                               Must be passed pointer to string array.       *
*                                                                             *
******************************************************************************/

#define UART_BAUD 9600
#define UART_BAUD_CODE ((uint16_t) ((F_CPU/16/UART_BAUD)-1))

extern void uartInit(void);
extern uint8_t uartRx(void);
extern void uartTxByte(uint8_t byte);
extern void uartTxWord(uint16_t word);
