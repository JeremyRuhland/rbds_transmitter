#include "includes.h"

void spiInit(void) {
    
    SPCR |= ((1<<SPE) | (1<<DORD) | (1<<MSTR) | (1<<SPR0)); // Enable, LSB first, master, 16x prescaler

    // Flush SPI buffers
    (void) spiByte(0x00);
    (void) spiByte(0x00);
    (void) spiByte(0x00);
}

uint8_t spiByte(uint8_t byte) {
    SPDR = byte; // Start SPI Tx
    while (!(SPSR & (1<<SPIF))) {} // Wait until SPI transmission is finished
    return (SPDR);
}
