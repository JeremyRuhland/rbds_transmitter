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
