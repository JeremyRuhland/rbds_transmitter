#include "includes.h"

void uartInit(void) {
    // Flush buffer
    UDR0 = 0x00;

    // Set baudrate
    UBRR0H = (uint8_t) (UART_BAUD_CODE>>8);
    UBRR0L = (uint8_t) (UART_BAUD_CODE);

    UCSR0B = ((1<<RXEN0) | (1<<TXEN0)); // Enable tx/rx

    UCSR0C = ((1<<UCSZ01) | (1<<UCSZ00)); // 8-bit mode
}

uint8_t uartRx(void) {
    uint8_t byte;

    if (UCSR0A & (1<<RXC0)) { // Check for data waiting
        byte = UDR0;
    } else {
        byte = 0x00;
    }

    return (byte);
}

void uartTxByte(uint8_t byte) {
    while(!(UCSR0A & (1<<UDRE0))) {} // Wail for buffer empty
    UDR0 = byte;
}

void uartTxWord(uint16_t word) {
    while(!(UCSR0A & (1<<TXC0))) {} // Wail for buffer & Tx empty

    // Write upper & lower byte
    uartTxByte((uint8_t) (word>>8));
    uartTxByte((uint8_t) word);
}

#ifdef TEXT_DEBUG
void uartTxStrg(char strg[]) {
    uint8_t i = 0;

    for (; strg[i] != 0x00; i++) {
        uartTxByte((uint8_t) strg[i]);
    }
}
#endif
