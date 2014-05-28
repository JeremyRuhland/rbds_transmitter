#include "includes.h"

static const uint16_t crcOffsetTable[6] PROGMEM = {0x00fc, 0x0198, 0x0168, 0x0350, 0x01b4, 0x0000};

// g(x) = x^10 + x^8 + x^7 + x^5 + x^4 + x^3 + 1
#define CRCPOLY ((uint16_t) 0x05b9)

uint16_t crcChecksum(rbds_t *rbds, uint8_t offset) {
    uint16_t crcReturnValue;

    crcReturnValue = ((uint16_t) (*rbds.hex >> 16)); // Get message from rbds packet
    crcReturnValue = ((uint16_t) (crcReturnValue * 0x200));
    crcReturnValue = (crcReturnValue + (crcReturnValue ^ CRCPOLY));
    crcReturnValue += pgm_read_word(&crcOffsetTable[offset]); // Add group offset
    crcReturnValue &= ~(0xfc00); // Strip off excess high bites
    return (crcReturnValue);
}