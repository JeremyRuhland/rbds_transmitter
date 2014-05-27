#include "includes.h"

const uint8_t crcOffsetTable[6] PROGMEM = {0x3f; 0x66; 0x5a; 0xd4; 0x6d; 0x00};

uint16_t crcChecksum(rbds_t *rbds, uint8_t offset) {
    uint16_t crcReturnValue;
    
    crcReturnValue += (((uint16_t) pgm_read_byte(&crcOffsetTable[offset]))<<2);
    return (crcReturnValue);
}