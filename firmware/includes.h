/*******************************************************************************
* Main Includes File                                                           *
*                                                                              *
*******************************************************************************/

/******************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
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

#define PICODE 0
#define PICODECHECKWORD 0

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
} block1_t;

typedef struct {
    uint8_t grouptype       : 4;
    uint8_t version         : 1;
    uint8_t trafficprogcode : 1;
    uint8_t pty             : 5;
    uint8_t ta              : 1;
    uint8_t ms              : 1;
    uint8_t di              : 1;
    uint8_t c               : 2;
    uint16_t checkword      : 10;
    uint8_t                 : 6;
} type0group_t;

typedef struct {
    uint8_t grouptype  : 4;
    uint8_t bo         : 1;
    uint8_t tp         : 1;
    uint8_t pty        : 5;
    uint8_t textabflag : 1;
    uint8_t c          : 4;
    uint16_t checkword : 10;
    uint8_t            : 6;
} type2group_t;

typedef struct {
    uint8_t hichar     : 8;
    uint8_t lowchar    : 8;
    uint16_t checkword : 10;
    uint8_t            : 6;
} radiotext_t;

typedef union rbds_t {
    block1_t block1;
    type0group_t type0group;
    type2group_t type2group;
    radiotext_t radiotext;
} rbds_t;

#include "spi.h"
#include "uart.h"
#include "lcd.h"
