/*******************************************************************************
* RBDS transmitter firmware                                                    *
* 2014 WWU Electrical engineering technology                                   *
* Jeremy Ruhland                                                               *
*                                                                              *
*******************************************************************************/

#include "includes.h"

// Function prototypes
void mainGpioInit(void);
void mainPwmInit(void);
void mainLcdInit(void);
void mainFrequencyInputTask(void);
void mainDataInputTask(void);
void mainEncodingTask(void);
void mainTransmissionTask(void);
uint16_t mainFrequencyConverter(uint16_t frequency);
void mainFrequencyInputLcdDisp(void);
void mainDelayOneSec(void);
void mainDataInputLcdDisp(void);
void mainPwmControl(uint8_t command);

// Global variables
#include "sintables.txt"
const uint8_t mainCustomChars[] PROGMEM = {0x0e, 0x11, 0x04, 0x0A, 0x00, 0x04, 0x04, 0x04, 0x00, 0x02, 0x06, 0x0e, 0x06, 0x02, 0x00, 0x00};
volatile mainSystemState_t mainSystemState = FREQUENCY_INPUT_MODE;
volatile uint8_t mainFrequencyBuffer[5];
volatile uint16_t mainTransitFrequency;
volatile uint8_t mainDataBuffer[65];
volatile uint8_t mainRbdsPacketBuffer[208];
volatile uint8_t mainRbdsPacketLength;

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

void mainPwmInit(void) {
    PRR |= ((1<<PRTWI)|(1<<PRTIM2)|(1<<PRTIM0)|(1<<PRTIM1)|(1<<PRADC)); // Power down unused modules, put pwm to sleep
        
    // 57khz 0c0a, 8, 0x23
    TCCR0A |= ((1<<COM0A0)|(1<<WGM01)); // Toggle 0c0a on cmp match, ctc mode
    TCCR0B |= (1<<CS00); // prescaler 1
    OCR0A = 139;
    
    // 19khz 0c1b
    TCCR1A |= (1<<COM1B0); // Toggle 0c1b on cmp match, ctc mode
    TCCR1B |= ((1<<WGM12)|(1<<CS10)); // ctc mode, prescaler 1
    OCR1AH = ((uint8_t) (420>>8));
    OCR1AL = ((uint8_t) 420);
    
    // ~26khz 0c2b
    TCCR2A |= ((1<<COM2B0)|(1<<WGM21)); // Toggle 0c2b on cmp match, ctc mode
    TCCR2B |= (1<<CS21); // prescaler 8
    OCR2A = 37;
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
        // Let the user see their entry has been corrected
        mainFrequencyInputLcdDisp();
        mainDelayOneSec();
    // If impossibly low frequency is requested, force higher frequency
    } else if (msgTransitFrequency < 7000) {
        msgTransitFrequency = 7000;
        mainFrequencyBuffer[0] = ' ';
        mainFrequencyBuffer[1] = '7';
        mainFrequencyBuffer[2] = '0';
        mainFrequencyBuffer[3] = '0';
        mainFrequencyBuffer[4] = '0';
        // Let the user see their entry has been corrected
        mainFrequencyInputLcdDisp();
        mainDelayOneSec();
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

void mainDelayOneSec(void) {
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
* Modifies global variable mainSystemState & mainDataBuffer &                  *
* mainRbdsPacketLength                                                         *
*******************************************************************************/
void mainDataInputTask(void) {
    uint8_t msgIndex;
    uint8_t msgIncomingChar;
    uint8_t msgRevertToPrevState = FALSE;
    uint8_t msgPadBuffer;

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
        // Check length of buffer
        for (msgIndex = 0; mainDataBuffer[msgIndex] != 0x00; msgIndex++) {}
        msgIndex--;
        // msgIndex now contains length of buffer
        // If message length not divisible by 4, add extra spaces at end of buffer until it is, end with \r
        msgPadBuffer = (msgIndex % 4);
        if (msgPadBuffer != 0) {
            for (; msgPadBuffer <= 2; msgPadBuffer++) {
                mainDataBuffer[msgIndex] = ' ';
                msgIndex++;
            }
            mainDataBuffer[msgIndex] = RETURN;
            msgPadBuffer++;
            mainRbdsPacketLength = ((msgIndex+msgPadBuffer)/4);
        // If zero length buffer or buffer divisible by 4 but less than 64 chars, make 3 space chars, end with \r
        } else if (msgIndex != 64) {
            mainDataBuffer[0] = ' ';
            mainDataBuffer[1] = ' ';
            mainDataBuffer[3] = ' ';
            mainDataBuffer[4] = RETURN;
            mainRbdsPacketLength = ((msgIndex+4)/4);
        // If buffer already full, clobber last char with \r
        } else {
            mainRbdsPacketBuffer[msgIndex-1] = RETURN;
            mainRbdsPacketLength = (msgIndex/4);
        }
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
        lcd_data(1); // Print left arrow char
        // Check length of buffer
        for (i = 0; mainDataBuffer[i] != 0x00; i++) {}
        // i now contains length of buffer
        lcd_puts(&mainDataBuffer[i-15]); // Print to end of string from i
    }
}

/*******************************************************************************
* Encoding task, takes text data and formats rbds packets for transmission.    *
*                                                                              *
* Modifies global variable mainSystemState & mainRbdsPacketBuffer              *
*******************************************************************************/
void mainEncodingTask(void) {
    rbds_t encRbdsBuffer[mainRbdsPacketLength];
    uint8_t encLoopLength;
    uint8_t i, j;
    uint8_t encBitLength;
    uint8_t encCurrentPacket;
    uint8_t encCurrentSegment = 0;
    uint8_t encCurrentBitOffset;
    uint8_t encLastBit = 0;
    uint8_t encCurrentBit;
    uint8_t encWorkingBit;
    
    encLoopLength = ((mainRbdsPacketLength*4)-1);
    
    // Stuff fields
    for (i = 0; i <= encLoopLength; i++) {
        switch (i % 4) {
            case 0:
                // Fill each group A field with premade data
                encRbdsBuffer[i].groupa.picode = PICODE;
                encRbdsBuffer[i].groupa.checkword = PICODECHECKWORD;
                break;
            case 1:
                // Fill each group B field with appropriate data
                encRbdsBuffer[i].type2groupb.grouptype = GROUP2A; // group type 2A, radiotext
                encRbdsBuffer[i].type2groupb.tp = FALSE; // No traffic announcements
                encRbdsBuffer[i].type2groupb.pty = NOPROGRAMTYPE; // No PTY sent to receiver
                encRbdsBuffer[i].type2groupb.textab = A; // Group type A
                encRbdsBuffer[i].type2groupb.segmentaddress = encCurrentSegment;
                // Compute group checksum
                encRbdsBuffer[i].type2groupb.checkword = crcChecksum(&encRbdsBuffer[i], OFFSETB);
                break;
            case 2:
                // Fill group c with two chars
                encRbdsBuffer[i].type2groupcd.hichar = mainDataBuffer[encCurrentSegment];
                encCurrentSegment++;
                encRbdsBuffer[i].type2groupcd.lowchar = mainDataBuffer[encCurrentSegment];
                encCurrentSegment++;
                // Compute group checksum
                encRbdsBuffer[i].type2groupcd.checkword = crcChecksum(&encRbdsBuffer[i], OFFSETC);
                break;
            case 3:
                // Fill group d with two chars
                encRbdsBuffer[i].type2groupcd.hichar = mainDataBuffer[encCurrentSegment];
                encCurrentSegment++;
                encRbdsBuffer[i].type2groupcd.lowchar = mainDataBuffer[encCurrentSegment];
                encCurrentSegment++;
                // Compute group checksum
                encRbdsBuffer[i].type2groupcd.checkword = crcChecksum(&encRbdsBuffer[i], OFFSETD);
                break;
            default :
                break;
        }
    }
    
    // Differentially encode encRbdsBuffer and place into mainRbdsPacketBuffer
    encBitLength = (mainRbdsPacketLength-1);
    i = 0;
    j = 0;
    // Go through each packet
    for (encCurrentPacket = 0; encCurrentPacket <= mainRbdsPacketLength; encCurrentPacket++) {
        // Go through each bit in each packet, MSB to LSB
        for (encCurrentBitOffset = 31; encCurrentBitOffset <= 6; encCurrentBitOffset--) {
            encCurrentBit = ((uint8_t) ((encRbdsBuffer[encCurrentPacket].hex >> encCurrentBitOffset) & ((uint32_t) 0x01))); // get current bit, 0 or 1
            encWorkingBit = (encCurrentBit ^ encLastBit);
            encLastBit = encCurrentBit; // Set new last bit

            // Clear or set appropriate bit in correct element of mainRbdsPacketBuffer
            if (encWorkingBit == 0) {
                mainRbdsPacketBuffer[i] = (mainRbdsPacketBuffer[i] & ~(1<<j));
            } else {
                mainRbdsPacketBuffer[i] = (mainRbdsPacketBuffer[i] | (1<<j));
            }

            // Advance to next bit position in element i of mainRbdsPacketBuffer
            if (j == 7) {
                j = 0;
                i++; // After 8 bits, move to next element in mainRbdsPacketBuffer array
            } else {
                j++;
            }
        }
    }
    
    mainSystemState = TRANSMISSION_MODE;
}

/*******************************************************************************
* Transmission task, turns on transmission hardware and loops through packet   *
* buffer, checking each bit in turn. Positive or negative sin waves are        *
* generated and sent to the transmission hardware.                             *
*                                                                              *
* Modifies global variable mainSystemState                                     *
*******************************************************************************/
void mainTransmissionTask(void) {
    dac_t trxDac;
    uint8_t trxIncomingChar = 0x00;
    uint16_t trxCurrentBufferBit;
    uint8_t trxCurrentBufferByte;
    uint16_t trxBufferBitLength;
    uint8_t trxCurrentBitInByte;
    uint8_t trxSinType;
    uint8_t i;
    
    mainPwmControl(STARTTHEMUSIC); // Start the music

    // Display frequency mode message
    lcd_clrscr();
    lcd_puts_P("Transmitting ");
    lcd_data(0); // Print custom transmission char
    lcd_gotoxy(0,1);
    lcd_puts_P("Freq");
    mainFrequencyInputLcdDisp();

    // Set transmission frequency
    trxDac.bit.channel = CHA;
    trxDac.bit.gainstage = TWOVREF;
    trxDac.bit.shutdown = STARTUP;
    trxDac.bit.data = mainFrequencyConverter(mainTransitFrequency);
    spiUpdateDac(trxDac);
    
    trxDac.bit.channel = CHB; // Get ready for transmitting data
    
    trxBufferBitLength = (((uint16_t) mainRbdsPacketLength) * 104); // Calculate how many bits we must transmit
    trxCurrentBitInByte = 0;
    trxCurrentBufferByte = 0;
    
    while (trxIncomingChar != BACKSPACE) {
        trxIncomingChar = uartRx(); // Check if we need to exit
        // Loop through for each bit we must transmit
        for (trxCurrentBufferBit = 0; trxCurrentBufferBit <= trxBufferBitLength; trxCurrentBufferBit++) {
            // Determine if we must transmit a 1 or 0
            trxSinType = (mainRbdsPacketBuffer[trxCurrentBufferByte] & (0x80>>trxCurrentBitInByte));
            
            // Transmit all 31 bits of the sin wave, either positive or negative
            // Positive wave
            if (trxSinType) {
                for (i = 0; i <= 30; i++) {
                    trxDac.bit.data = pgm_read_word(&mainSinTable[i]);
                    spiUpdateDac(trxDac); // Send new data to DAC
                    _delay_us(PERIODDELAY-COMPUTATIONTIME); // Delay until next sin wave segment is needed, adjust COMPUTATIONTIME as needed
                }
            // Negative wave
            } else {
                for (i = 32; i >= 1; i--) {
                    trxDac.bit.data = pgm_read_word(&mainSinTable[i]);
                    spiUpdateDac(trxDac); // Send new data to DAC
                    _delay_us(PERIODDELAY-COMPUTATIONTIME); // Delay until next sin wave segment is needed, adjust COMPUTATIONTIME as needed
                }
            }
                        
            // Keep track of which byte and bit position each bit is stored in
            if (trxCurrentBitInByte == 7) {
                trxCurrentBitInByte = 0;
                trxCurrentBufferByte++;
            } else {
                trxCurrentBitInByte++;
            }
        }
    }

    mainPwmControl(STOPTHEMUSIC);
    mainSystemState = FREQUENCY_INPUT_MODE;
}

uint16_t mainFrequencyConverter(uint16_t frequency) {
    uint32_t frequencyHertz;

    // Convert binary frequency number into setting for dac
    frequencyHertz = (((uint32_t) frequency) * 10000);
    frequencyHertz -= FREERUNNINGFREQUENCY; // Take difference from freerunning freq of vco
    frequencyHertz /= HZPERMV; // Calculate how many mV are needed to alter freq by this many hz
    
    return ((uint16_t) frequencyHertz);
}

void mainPwmControl(uint8_t command) {
    if (command == STARTTHEMUSIC) {
        DDRD &= ~(1<<PD1); // Turn on transmission circuits
        PRR &= ~((1<<PRTIM0)|(1<<PRTIM1)|(1<<PRTIM2)); // Power up pwm modules
    } else {
        DDRD |= (1<<PD1); // Turn off transmission circuits
        PRR |= ((1<<PRTIM0)|(1<<PRTIM1)|(1<<PRTIM2)); // Power down pwm modules
    }
}
