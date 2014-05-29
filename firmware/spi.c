#include "includes.h"

void spiInit(void);
static uint8_t spiByte(uint8_t byte);
void spiUpdateDac(dac_t dacdata);

void spiInit(void) {
    SPCR |= ((1<<SPE) | (1<<MSTR)); // Enable, MSB first, master, mode 0,0
    SPSR |= (1<<SPI2X); // 2x prescaler

    // Flush SPI buffers
    (void) spiByte(0x00);
}

static uint8_t spiByte(uint8_t byte) {
    SPDR = byte; // Start SPI Tx
    while (!(SPSR & (1<<SPIF))) {} // Wait until SPI transmission is finished
    return (SPDR);
}

void spiUpdateDac(dac_t dacdata) {
    PORTD &= ~(1<<PD5); // Assert chip select

    // Transmit new shift register data
    (void) spiByte((uint8_t) dacdata.spi);
    (void) spiByte((uint8_t) (dacdata.spi>>8));

    PORTD |= (1<<PD5); // Deassert chip select

    // Rising edge on latch, delay and drop
    _delay_us(0.04); // Delay 40nS
    PORTB &= ~(1<<PB1);
    _delay_us(0.1); // Delay 100nS
    PORTB |= (1<<PB1);
}
