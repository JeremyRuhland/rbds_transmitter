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

#include "spi.h"
#include "uart.h"
#include "lcd.h"

typedef enum {FREQUENCY_INPUT_MODE, DATA_INPUT_MODE, ENCODING_MODE, TRANSMISSION_MODE} mainSystemState_t;
