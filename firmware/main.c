/*******************************************************************************
* RBDS transmitter firmware                                                    *
* 2014 WWU Electrical engineering technology                                   *
* Jeremy Ruhland                                                               *
*                                                                              *
*******************************************************************************/

#include "includes.h"

// Function prototypes
inline void mainGpioInit(void);
void mainFrequencyInputTask(void);
void mainDataInputTask(void);
void mainEncodingTask(void);
void mainTransmissionTask(void);

// Global variables
#include "sintables.txt"
volatile mainSystemState_t mainSystemState = FREQUENCY_INPUT_MODE;
volatile uint8_t
int main(void) {
    // Initialize all functions
    mainGpioInit();
    spiInit();
    uartInit();
    lcd_init(LCD_DISP_ON);

    // Main control loop
    for (;;) {
        switch (mainSystemState) {
            case FREQUENCY_INPUT_MODE:
                mainFrequencyInputTask();
                break;
            case DATA_INPUT_MODE:
                mainDataInputTask();
                break;
            case ENCODING_MODE:
                mainEncodingTask();
                break;
            case TRANSMISSION_MODE:
                mainTransmissionTask();
                break;
            default:
                break;
        }
    }
}

/*******************************************************************************
* GPIO initilization routine                                                   *
*******************************************************************************/
void mainGpioInit(void) {
    PORTD |= ((1<<PD1) | (1<<PD2) | (1<<PD5) | (1<<PD6));
    DDRD |= ((1<<PD1) | (1<<PD2) | (1<<PD3) | (1<<PD5) | (1<<PD6));
    DDRB |= ((1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB5));
}

/*******************************************************************************
*
*******************************************************************************/
void mainFrequencyInputTask(void) {
    uint8_t msgBuffer[] = {0, 0, 0, 0, 0, 0};
    uint8_t msgIndex;

    // Display frequency mode message
    lcd_clrscr();
    lcd_puts_p();

    for (msgIndex = 0; msgIndex <= 5; msgIndex++) {
        // Enable interrupts, put cpu to sleep
        sei();
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();

        msgBuffer[msgIndex] = uartRx(); // Grab incoming char

        // Leave loop prematurely if return char
        if (msgBuffer[msgIndex] == '\r') {
            break;
        } else {}
    }

    // Convert string to variable
}

/*******************************************************************************
*
*******************************************************************************/
void mainDataInputTask(void) {
}

/*******************************************************************************
*
*******************************************************************************/
void mainEncodingTask(void) {
}

/*******************************************************************************
*
*******************************************************************************/
void mainTransmissionTask(void) {
}
