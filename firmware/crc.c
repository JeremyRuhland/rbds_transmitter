#include "includes.h"

// g(x) = x^10 + x^8 + x^7 + x^5 + x^4 + x^3 + 1
static const uint16_t crcGenTable[16] PROGMEM = {0x01b9, 0x0372, 0x035d, 0x0303, 0x03bf, 0x02c7, 0x0037, 0x006e, 0x00dc, 0x01b8, 0x0370, 0x0359, 0x030b, 0x03af, 0x02e7, 0x0077};

static const uint16_t crcOffsetTable[6] PROGMEM = {0x00fc, 0x0198, 0x0168, 0x0350, 0x01b4, 0x0000};

uint16_t crcChecksum(rbds_t *rbds, uint8_t offset) {
    uint16_t crcReturnValue = 0;
    uint16_t crcControlValue;
    uint8_t i;

    crcControlValue = ((rbds->hex) >> 16); // Get message from rbds packet
    for (i = 0; i <= 15; i++) {
        if (crcControlValue & (1<<i)) {
            crcReturnValue ^= pgm_read_word(&crcGenTable[i]); // xor with gen table element
        } else {}
    }
    crcReturnValue ^= pgm_read_word(&crcOffsetTable[offset]); // xor with group offset
    crcReturnValue &= ~(0xfc00); // Strip off excess high bites
    return (crcReturnValue);
}
