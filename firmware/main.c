/*******************************************************************************
* RBDS transmitter firmware                                                    *
* 2014 WWU Electrical engineering technology                                   *
* Jeremy Ruhland                                                               *
*                                                                              *
*******************************************************************************/

#include "includes.h"

// Function prototypes
inline void mainGpioInit(void);
inline void mainLcdInit(void);
void mainFrequencyInputTask(void);
void mainDataInputTask(void);
void mainEncodingTask(void);
void mainTransmissionTask(void);
void mainFrequencyInputLcdDisp(uint8_t msg[]);

// Global variables
#include "sintables.txt"
PROGMEM const uint8_t mainCustomRadioChar[] = {0x0e, 0x11, 0x04, 0x0A, 0x00, 0x04, 0x04, 0x04};
volatile mainSystemState_t mainSystemState = FREQUENCY_INPUT_MODE;
volatile uint16_t mainTransitFrequency;

int main(void) {
    // Initialize all functions
    mainGpioInit();
    spiInit();
    uartInit();
    mainLcdInit();

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
* Initilization routines                                                       *
*******************************************************************************/
void mainGpioInit(void) {
    PORTD |= ((1<<PD1) | (1<<PD2) | (1<<PD5) | (1<<PD6));
    DDRD |= ((1<<PD1) | (1<<PD2) | (1<<PD3) | (1<<PD5) | (1<<PD6));
    DDRB |= ((1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB5));
}

void mainLcdInit(void) {
    uint8_t i;

    lcd_init(LCD_DISP_ON);
    lcd_command(1<<LCD_CGRAM);
    for (i = 0; i<8; i++) {
        lcd_data(pgm_read_byte(&mainCustomRadioChar[i]));
    }
}

/*******************************************************************************
*
*******************************************************************************/
void mainFrequencyInputTask(void) {
    uint8_t msgBuffer[] = {' ', ' ', ' ', ' ', ' '};
    uint8_t msgIndex;
    uint8_t msgIncomingChar;
    uint32_t msgTransitFrequency;

    // Display frequency mode message
    lcd_clrscr();
    lcd_puts_P("Enter Frequency:");
    // Show current buffer on lcd
    mainFrequencyInputLcdDisp(msgBuffer);

    for (msgIndex = 0; msgIndex <= 5; msgIndex++) {
        // Enable interrupts, put cpu to sleep, will wake on uart char rx
        sei();
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();

        msgIncomingChar = uartRx(); // Grab incoming char

        // If incoming char was RETURN assume end of entry
        if (msgIncomingChar == RETURN) {
            // Fill in rest of buffer with '0' if buffer not full
            if (msgIndex <= 4) {
                msgIndex++; // Point to next element in array
                // Loop to end of array, filling each element with '0'
                for (; msgIndex <= 4; msgIndex++) {
                    msgBuffer[msgIndex] = '0';
                }
            } else {}
        } else if (msgIndex <= 4 && msgIncomingChar >= '0' && msgIncomingChar <= '9') {
            // If 0-9 input, fill element in buffer array
            msgBuffer[msgIndex] = msgIncomingChar;
        } else if (msgIncomingChar == BACKSPACE) {
            // If BACKSPACE input, go back & clear prev buffer element
            msgIndex--;
            msgBuffer[msgIndex] = ' ';
        } else {
            // Do not count illegal char entry (either not 0-9/RETURN/BACKSPACE or buffer is already full)
            msgIndex--;
        }

        // Show current buffer on lcd
        mainFrequencyInputLcdDisp(msgBuffer);
    }

    // Convert string to variable
    msgTransitFrequency = 0;
    // Shift digit over and add each element of buffer array
    for (msgIndex = 0; msgIndex <= 4; msgIndex++) {
        msgTransitFrequency *= 10;
        msgTransitFrequency += msgBuffer[msgIndex];
    }
    // If impossibly high frequency is requested, force lower frequency
    if (msgTransitFrequency > 15000) {
        msgTransitFrequency = 15000;
    // If impossibly low frequency is requested, force higher frequency
    } else if (msgTransitFrequency < 7000) {
        msgTransitFrequency = 7000;
    } else {}
    mainTransitFrequency = ((uint16_t) msgTransitFrequency);

    mainSystemState = DATA_INPUT_MODE; // Move on to next entry state
}

void mainFrequencyInputLcdDisp(uint8_t msg[]) {
    // Show current buffer on lcd
    lcd_gotoxy(5, 1);
    lcd_putc(msg[0]);
    lcd_putc(msg[1]);
    lcd_putc(msg[2]);
    lcd_putc('.');
    lcd_putc(msg[3]);
    lcd_putc(msg[4]);
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
