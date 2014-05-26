/*******************************************************************************
* RBDS transmitter firmware                                                    *
* 2014 WWU Electrical engineering technology                                   *
* Jeremy Ruhland                                                               *
*                                                                              *
*******************************************************************************/

#include "includes.h"

// Function prototypes
void mainGpioInit(void);
void mainLcdInit(void);
void mainFrequencyInputTask(void);
void mainDataInputTask(void);
void mainEncodingTask(void);
void mainTransmissionTask(void);
void mainFrequencyInputLcdDisp(void);
void delayOneSec(void);
void mainDataInputLcdDisp(void);

// Global variables
#include "sintables.txt"
PROGMEM const uint8_t mainCustomChars[] = {0x0e, 0x11, 0x04, 0x0A, 0x00, 0x04, 0x04, 0x04, 0x00, 0x02, 0x06, 0x0e, 0x06, 0x02, 0x00, 0x00};
volatile mainSystemState_t mainSystemState = FREQUENCY_INPUT_MODE;
volatile uint8_t mainFrequencyBuffer[5];
volatile uint16_t mainTransitFrequency;
volatile uint8_t mainDataBuffer[65];
PROGMEM const rbds_t group1 = {.block1 = {.picode = PICODE, .checkword = PICODECHECKWORD}};

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
* Initialization routines                                                      *
*******************************************************************************/
void mainGpioInit(void) {
    PORTD |= ((1<<PD3) | (1<<PD5) | (1<<PD6));
    DDRD |= ((1<<PD1) | (1<<PD3) | (1<<PD5) | (1<<PD6));
    DDRB |= ((1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB5));
}

void mainLcdInit(void) {
    uint8_t i;

    lcd_init(LCD_DISP_ON);
    lcd_command(1<<LCD_CGRAM);
    for (i = 0; i < 16; i++) {
        lcd_data(pgm_read_byte(&mainCustomChars[i]));
    }
}

/*******************************************************************************
* Frequency input task, sleeps CPU until UART Rx, places char into buffer.     *
* When buffer fills all keys but RETURN & BACKSPACE are ignored. If at any     *
* time RETURN is received the rest of the buffer is filled with 0 and the task *
* converts the string into a binary variable. In the event of under/overflow   *
* the variable is floor/ceilinged to maintain a sane value.                    *
*                                                                              *
* Modifies global variable mainSystemState & mainTransitFrequency              *
*******************************************************************************/
void mainFrequencyInputTask(void) {
    uint8_t msgIndex;
    uint8_t msgIncomingChar;
    uint32_t msgTransitFrequency;

    // Clear mainFrequencyBuffer
    for (msgIndex = 0; msgIndex <= 4; msgIndex++) {
        mainFrequencyBuffer[msgIndex] = ' ';
    }

    // Display frequency mode message
    lcd_clrscr();
    lcd_puts_P("Enter Frequency:");
    // Show current buffer on lcd
    mainFrequencyInputLcdDisp();

    for (msgIndex = 0; msgIndex <= 5; msgIndex++) {
        // Enable interrupts, put cpu to sleep, will wake on uart char rx
        sei();
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
        // Cpu wakes up here on keypress

        msgIncomingChar = uartRx(); // Grab incoming char

        // If incoming char was RETURN assume end of entry
        if (msgIncomingChar == RETURN) {
            // Fill in rest of buffer with '0' if buffer not full
            if (msgIndex <= 4) {
                // Loop to end of array, filling each element with '0'
                for (; msgIndex <= 4; msgIndex++) {
                    mainFrequencyBuffer[msgIndex] = '0';
                }
                // msgIndex now at 5, for loop will exit and convert string to binary
            } else {}
        } else if (msgIndex <= 4 && msgIncomingChar >= '0' && msgIncomingChar <= '9') {
            // If 0-9 input, fill element in buffer array
            mainFrequencyBuffer[msgIndex] = msgIncomingChar;
        } else if (msgIndex > 0 && msgIncomingChar == BACKSPACE) {
            // If BACKSPACE input & enough room in buffer go back & clear prev buffer element
            msgIndex--;
            mainFrequencyBuffer[msgIndex] = ' ';
            msgIndex--;
        } else {
            // Do not count illegal char entry (either not 0-9/RETURN/BACKSPACE or buffer is already full)
            msgIndex--;
        }

        // Show current buffer on lcd
        mainFrequencyInputLcdDisp();
    }

    // Convert string to binary
    msgTransitFrequency = 0;
    // Shift digit over and add each element of buffer array
    for (msgIndex = 0; msgIndex <= 4; msgIndex++) {
        msgTransitFrequency *= 10;
        msgTransitFrequency += (mainFrequencyBuffer[msgIndex]);
    }
    // If impossibly high frequency is requested, force lower frequency
    if (msgTransitFrequency > 15000) {
        msgTransitFrequency = 15000;
        mainFrequencyBuffer[0] = '1';
        mainFrequencyBuffer[1] = '5';
        mainFrequencyBuffer[2] = '0';
        mainFrequencyBuffer[3] = '0';
        mainFrequencyBuffer[4] = '0';
        mainFrequencyInputLcdDisp();
        delayOneSec();
    // If impossibly low frequency is requested, force higher frequency
    } else if (msgTransitFrequency < 7000) {
        msgTransitFrequency = 7000;
        mainFrequencyBuffer[0] = ' ';
        mainFrequencyBuffer[1] = '7';
        mainFrequencyBuffer[2] = '0';
        mainFrequencyBuffer[3] = '0';
        mainFrequencyBuffer[4] = '0';
        mainFrequencyInputLcdDisp();
        delayOneSec();
    } else {}
    mainTransitFrequency = ((uint16_t) msgTransitFrequency);

    mainSystemState = DATA_INPUT_MODE; // Move on to next entry state
}

void mainFrequencyInputLcdDisp(void) {
    // Show current buffer on lcd
    lcd_gotoxy(5, 1);
    lcd_putc(mainFrequencyBuffer[0]);
    lcd_putc(mainFrequencyBuffer[1]);
    lcd_putc(mainFrequencyBuffer[2]);
    lcd_putc('.');
    lcd_putc(mainFrequencyBuffer[3]);
    lcd_putc(mainFrequencyBuffer[4]);
}

void delayOneSec(void) {
    uint8_t i;
    
    for (i = 0; i <= 9; i++) {
        _delay_ms(100);
    }
}

/*******************************************************************************
* Data input task, sleeps CPU until UART Rx, places char into buffer.          *
* When buffer fills all keys but RETURN & BACKSPACE are ignored. If at any     *
* time RETURN is received the entry is assumed to be finished and the system   *
* moves to the next state.                                                     *
*                                                                              *
* Modifies global variable mainSystemState & mainDataBuffer                    *
*******************************************************************************/
void mainDataInputTask(void) {
    uint8_t msgIndex;
    uint8_t msgIncomingChar;
    uint8_t msgRevertToPrevState = FALSE;

    // Clear mainDataBuffer
    for (msgIndex = 0; msgIndex <= 64; msgIndex++) {
        mainDataBuffer[msgIndex] = 0x00;
    }
    
    // Display frequency mode message
    lcd_clrscr();
    lcd_puts_P("Enter a message:");
    // Show current buffer on lcd
    mainDataInputLcdDisp();

    for (msgIndex = 0; msgIndex <= 64; msgIndex++) {
        // Enable interrupts, put cpu to sleep, will wake on uart char rx
        sei();
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
        // Cpu wakes up here on keypress

        msgIncomingChar = uartRx(); // Grab incoming char

        // If incoming char was RETURN assume end of entry
        if (msgIncomingChar == RETURN) {
            msgIndex = 64; // Skip msgIndex to end of buffer, for loop will exit
        } else if (msgIndex <= 63 && msgIncomingChar >= ' ' && msgIncomingChar <= '~') {
            // If printing char, fill element in buffer array
            mainDataBuffer[msgIndex] = msgIncomingChar;
        } else if (msgIncomingChar == BACKSPACE) {
            // Check if anything exists in buffer
            if (msgIndex > 0) {
                // If BACKSPACE input, go back & clear prev buffer element
                msgIndex--;
                mainDataBuffer[msgIndex] = 0x00;
                msgIndex--;
            } else {
                // Nothing exists in buffer to erase, skip back to previous state
                msgRevertToPrevState = TRUE;
                msgIndex = 64;
            }
        } else {
            // Do not count illegal char entry (either not printing ascii/RETURN/BACKSPACE or buffer is already full)
            msgIndex--;
        }

        // Show current buffer on lcd
        mainDataInputLcdDisp();
    }
    
    // Figure out how to exit state
    if (msgRevertToPrevState) {
        mainSystemState = FREQUENCY_INPUT_MODE;
    } else {
        mainSystemState = ENCODING_MODE;
    }
}

void mainDataInputLcdDisp(void) {
    // Show latest 15 chars of buffer on display
    uint8_t i;
    
    lcd_gotoxy(0, 1); // Move cursor to beginning of second line
    // Check if msg exceeds 16 chars
    if (mainDataBuffer[16] == 0x00) {
        lcd_puts(&mainDataBuffer[0]); // Print entire buffer onto lcd
    } else {
        // Check length of buffer
        for (i = 0; mainDataBuffer[i] == 0x00; i++) {}
        // i now contains length of buffer
        lcd_data(1); // Print left arrow char
        lcd_puts(&mainDataBuffer[i-15]); // Print to end of string from i
    }
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
    dac_t encDac;
    uint16_t encFrequency;
    
    PORTD &= ~(1<<PD1); // Turn on transmission circuits
    _delay_ms(100);

    encFrequency = mainTransitFrequency;

    // Set transmission frequency
    encDac.bit.channel = CHA;
    encDac.bit.gainstage = VREF;
    encDac.bit.shutdown = STARTUP;
    encDac.bit.data = encFrequency;
    spiUpdateDac(encDac);
}
