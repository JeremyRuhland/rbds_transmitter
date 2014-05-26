#include "includes.h"

void spiInit(void);
static uint8_t spiByte(uint8_t byte);
void spiUpdateDac(dac_t dacdata);

void spiInit(void) {
    
    SPCR |= ((1<<SPE) | (1<<DORD) | (1<<MSTR) | (1<<SPR0)); // Enable, LSB first, master, 16x prescaler

    // Flush SPI buffers
    (void) spiByte(0x00);
    (void) spiByte(0x00);
    (void) spiByte(0x00);
}

static uint8_t spiByte(uint8_t byte) {
    SPDR = byte; // Start SPI Tx
    while (!(SPSR & (1<<SPIF))) {} // Wait until SPI transmission is finished
    return (SPDR);
}

void spiUpdateDac(dac_t dacdata) {
     // Transmit new shift register data
    (void) spiByte((uint8_t) dacdata.spi);
    (void) spiByte((uint8_t) (dacdata.spi>>8));

    _delay_us(0.025); // Delay 25nS for setup time of shift register

    // Rising edge on latch, delay and drop
    PORTD &= ~(1<<PD6);
    _delay_us(0.025); // Delay 25nS for setup time of shift register
    PORTD |= (1<<PD6);
}
