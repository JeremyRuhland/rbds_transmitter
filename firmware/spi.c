#include "includes.h"

void spiInit(void) {
    
    SPCR |= ((1<<SPE) | (1<<DORD) | (1<<MSTR) | (1<<SPR0)); // Enable, LSB first, master, 16x prescaler

    // Flush SPI buffers
    (void) spiByte(0x00);
    (void) spiByte(0x00);
    (void) spiByte(0x00);
}

void spiPeripheralSelect(uint8_t peripheral) {
    // Disable all peripherals (drive up)
    PORTD |= (1<<PD7);
    PORTC |= (1<<PC5);
    PORTB |= ((1<<PB0) | (1<<PB1));

    // Enable proper perpheral (drive down)
    switch (peripheral) {
        case 1:
            PORTC &= ~(1<<PC5);
            break;
        case 2:
            PORTD &= ~(1<<PD7);
            break;
        case 3:
            PORTB &= ~(1<<PB0);
            break;
        case 4:
            PORTB &= ~(1<<PB1);
            break;
        default:
            break;
    }
}

uint8_t spiByte(uint8_t byte) {
    SPDR = byte; // Start SPI Tx

    while (!(SPSR & (1<<SPIF))) {} // Wait until SPI transmission is finished

    return (SPDR);
}
