/*******************************************************************************
* Main Includes File                                                           *
*                                                                              *
*******************************************************************************/

/******************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#define FALSE 0
#define TRUE 1

#define RETURN ((uint8_t) '\r')
#define BACKSPACE ((uint8_t) '\b')

#define CHA ((uint8_t) 0)
#define CHB ((uint8_t) 1)
#define VREF ((uint8_t) 1)
#define TWOVREF ((uint8_t) 0)
#define STARTUP ((uint8_t) 1)
#define SHUTDOWN ((uint8_t) 0)
#define HZPERMV ((uint16_t) 5225)
#define FREERUNNINGFREQUENCY ((uint32_t) 40000000)

#define PICODE ((uint16_t) 0x54a8)
#define PICODECHECKWORD ((uint16_t) 0x0000)

#define OFFSETA ((uint8_t) 0x00)
#define OFFSETB ((uint8_t) 0x01)
#define OFFSETC ((uint8_t) 0x02)
#define OFFSETC2 ((uint8_t) 0x03)
#define OFFSETD ((uint8_t) 0x04)
#define OFFSETE ((uint8_t) 0x05)

#define GROUP2A ((uint8_t) 0x04)
#define NOPROGRAMTYPE ((uint8_t) 0x00)
#define A ((uint8_t) 0x00)

#define STARTTHEMUSIC ((uint8_t) 0x01)
#define STOPTHEMUSIC ((uint8_t) 0x00)

#define PERIODDELAY 26.315
#define COMPUTATIONTIME 2.14

typedef enum {FREQUENCY_INPUT_MODE, DATA_INPUT_MODE, ENCODING_MODE, TRANSMISSION_MODE} mainSystemState_t;

typedef struct {
    uint8_t channel   : 1;
    uint8_t           : 1;
    uint8_t gainstage : 1;
    uint8_t shutdown  : 1;
    uint16_t data     : 12;
} bit_t;

typedef union dac_t {
    uint16_t spi;
    bit_t bit;
} dac_t;

typedef struct {
    uint16_t picode    : 16;
    uint16_t checkword : 10;
    uint8_t            : 6;
} groupa_t;

typedef struct {
    uint8_t grouptype  : 5;
    uint8_t tp         : 1;
    uint8_t pty        : 5;
    uint8_t ta         : 1;
    uint8_t ms         : 1;
    uint8_t di         : 1;
    uint8_t c          : 2;
    uint16_t checkword : 10;
    uint8_t            : 6;
} type0groupb_t;

typedef struct {
    uint8_t grouptype      : 5;
    uint8_t tp             : 1;
    uint8_t pty            : 5;
    uint8_t textab         : 1;
    uint8_t segmentaddress : 4;
    uint16_t checkword     : 10;
    uint8_t                : 6;
} type2groupb_t;

typedef struct {
    uint8_t hichar     : 8;
    uint8_t lowchar    : 8;
    uint16_t checkword : 10;
    uint8_t            : 6;
} type2groupcd_t;

typedef union rbds_t {
    groupa_t groupa;
    type0groupb_t type0groupb;
    type2groupb_t type2groupb;
    type2groupcd_t type2groupcd;
    uint32_t hex;
} rbds_t;

// These includes require some structs defined above
#include "spi.h"
#include "uart.h"
#include "LCD.h"
#include "crc.h"