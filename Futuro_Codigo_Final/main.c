/*
 * THIS PROJECT IS AN EXAMPLE OF SEVERAL FUNCTIONALITIES OF THE MICROCONTROLLER AND
 * OF HOW TO USE THEM AND ESTABLISH A CONNECTION BETWEEN FUNCTIONS WRITEN IN C AND ASM
 * 
 * THE CONFIGURATION OF ALL PERIPHERALS IS DONE IN THE CONFIG.ASM FILE. 
 * 
 * THE FUNCTION PASS_VARIABLE_BETWEEN_C_AND_ASM IS WRITTEN IN ASM AND IT IS AN EXAMPLE OF HOW 
 * TO HANDLE VARIABLES IN BOTH C AND ASM. THE INPUT ARGUMENT OF THE FUNCTION IN C (IN THIS CASE, 
 * THE NUMBER 3), IS STORED IN THE WORKING REGISTER W WHEN THE ASM FUNCTION IS CALLED. THE OUTPUT
 * OF THE FUNCTION RECEIVED IN THE C CALL OF THE FUNCTION IS WHATEVER VALUE IS STORED IN W WHEN 
 * THE RETURN HAPPENS IN THE ASM FUNCTION.
 * IN THIS EXAMPLE, THE FUNCTION RECEIVES THE INPUT VALUE (3), ADDS (5), RETURNS THE RESULT AND
 * ALSO ADDS 1 TO THE GLOBAL VARIABLE var_global.
 * 
 * AFTER THAT WE GO TO AN INFINITE LOOP WHERE EVERY 2 SECONDS WE PRINT THE GLOBAL VARIABLE AND
 * THE RESULT OF THE (3+5) OPERATION
 * 
 * APPROXIMATELY EVERY 500 MS, THE TIMER0 INTERRUPT IS TRIGGERED AND TOGGLES PORTA,5
 * ADDITIONALLY, AN ADC CONVERSION IS TRIGGERED. WHEN IT IS COMPLETED, ANOTHER INT HAPPENS WHICH
 * SENDS THE CONVERTED VALUE VIA UART.
 * 
 * WHEN THE UART RECEIVES A VALUE, IT TRIGGERS AN INTERRUPT. WE GO TO INT_HANDLER, WHICH THEN CALLS
 * FUNCTION GETCH (A FUNCTION WRITTEN IN C) THAT READS AND STORES THE RECEIVED VALUE IN VARIABLE  get_char. 
 * IN THE INFINITE LOOP, WHENEVER THE VALUE OF GET_CHAR IS NOT 0, IT PRINTS GET_CHAR+1.
 */


#include <xc.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include "TEDS_definition.h"

#define _XTAL_FREQ 32000000   //the clock frequency defined in config() must be provided here for the delay function

//declaration of global variables and functions
void send_data(uint8_t);
void send_error(void);
void define_METATEDS(void);
void define_TCTEDS(void);
void identify_NCAP_cmd(void);
void send_METATEDS(void);
void send_TCTEDS(uint8_t);
void add_to_receiv_buff(void);
void config(void);
void __interrupt() int_handler(void); //the interrupt handler routine must be declared as __interrupt(). When an INT happens, this function is called
extern unsigned char pass_variable_between_C_and_ASM(unsigned char a);

void putch(char); //function used to send data via UART. The printf uses this function too.
char var_global = 0;  
char get_char;
char comutar = 0;
char adc_result;

// Create the structs for the Meta TEDS and for the TransducerChannels TEDS for 4 channels
struct METATEDS_TEMPLATE METATED;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS1;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS2;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS3;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS4;

// Declare receive and send buffers
char rec_buffer[100];
int rec_head = 0;
char sent_buffer[100];
int sent_head = 0;

char adc_buffer[100];
int adc_head = 0;


void main(void) {
    
    config();
    define_METATEDS();
    define_TCTEDS();
    
    //We give this function an input argument (3) and it adds 5 to that value and adds 1 to the global variable var_global
    volatile unsigned char result = pass_variable_between_C_and_ASM(3); 
    
    
    
    int j = 0;
    //If the main ends, the microcontroller resets. So, a while(1) is desirable to keep it looping
    while(1){

        __delay_ms(50);
        /*print the results every 2 seconds*/
        if (0){
            printf("Global variable = ");  //we use printf to print strings
            putch(var_global);  //and we can use putch to directly put a 8bit variable in the UART TX
            printf("\n");
            printf("Output of function = ");
            putch(result);
            printf("\n");
            j = 0;
        }
        
        j=j+1;

    
        //PORTAbits.RA5 = !PORTAbits.RA5;  //this is how we modify bits in C.

        //When we receive a char from the UART RX, the interrupt calls function getch, where the received byte is stored in variable get_char.
        //Here, if we receive a value different than 0, we add 1 and return the updated value via UART.
       
        
        if(get_char != 0)
        {
            printf("Received: ");
            putch(get_char);
            printf("\n");
            printf("Transmitted: ");
            get_char = get_char+1;
            putch(get_char);
            printf("\n");
            get_char = 0;
        }
            
    } 
}

void putch(char byte) //this function is required for the printf. It tells printf where to put the data. In this case, its to TX1REG
{
    while (PIR3bits.TX1IF == 0) {  
        ;
    }
    // Q: WHERE DO WE PUT A BYTE WHEN WE WANT TO SEND IT VIA UART?
    TX1REG = byte; /* transmit a character to Serial IO */

    //wait until it is sent
    while (PIR3bits.TX1IF == 0) {
        ;
    }
}

// Add incoming comunications to the receive buffer
void add_to_receiv_buff(void){
    // BUG FIX RECEIVING
    RC1STAbits.OERR = 0; 

    rec_buffer[rec_head] = RC1REG;
    rec_head += 1;
    
    identify_NCAP_cmd();
    return;
}

void identify_NCAP_cmd(void){
    // First let's check if we have a valid command
    // First check if we have the base 6 hex values
    if (rec_head < 5){
        return;
    }

    // VERIFY WHAT COMMAND WAS RECEIVED FROM THE NCAP (COMPUTER) AND DECIDE WHAT TO DO
    // Vamos receber bytes em hexa - slide 30
    // 2 hex - Destination Transducer MSB
    // 2 hex - Destination Transducer LSB
    // 2 hex - CMD Class (slide 31 - 1 e 3)
    // 2 hex - CMD function (slide 32)
    // 2 hex - Length MSB MSB
    // 2 hex - Length MSB LSB
    // 2 hex - Length defined from above, MSB Command-dependent octets
    // 2 hex - Length defined from above, LSB Command-dependent octets
    
    
    // Notes:
    // When quering for Meta TEDS data, Destination Transducer data 
    // does not matter
    // Destination Transducer - São os canais!
    
    // CMD Class Slide 31, tab 15
    // 01 - Comunando comum do TIM  e Trandutores (Para pedir informações)
    // 03 - Tranducer operating state (Quando queremos ler dados porque o transdutor vai estar ativo)
    
    //CMD Functions 
    // Classe de Metateds Slide 32, tab 16 (1))
    // 02 - Ler TEDS
    // 03 - Escrever TEDS ?
    // Classe de Transducer operating state Slide 38 (3)
    // 01 - Read
    
    // Command dependant Slide 33, tab 17
    // 01 - METATEDS
    // 03 - Transdutores
    
    // This function needs to respond with:
    // 2 hex - Success/Fail Flag 
    // 2 hex - Length (MSB)
    // 2 hex - Length (LSB)
    // x hex - Data (x = Length)

    // let's assume we received the data on the buffer variable (array)

    // > If statement for CMD class
    // If the class is to require information (common to TIM and transducers)
    // Notes: If this is the case we can ignore the destination transducer (first 4 hex)
    if (rec_buffer[2] == 1) {
        // If statement for CMD function
        if (rec_buffer[3] == 2) {
            // length is  always 2 
            if (rec_buffer[5] == 2 ) {
                // If  command-dependant octets are 1 and offset is 0 send metateds
                if (rec_buffer[6] == 1 && rec_buffer[7] == 0) {
                    // Send Meta TEDS
                    send_METATEDS();
                    return;
                }
                // If  command-dependant octets are 3 and offset is 0 send TCTEDS
                else if (rec_buffer[6] == 3 && rec_buffer[7] == 0) {
                    // Send Transducer TEDS
                    send_TCTEDS(rec_buffer[1]);
                    return;
                }

            }
        }
    }
    
    // If the class is to read information (Transducer Operating State)
    else if (rec_buffer[2] == 3) {
        // If statement for CMD function
        // 0x01 - Read
        if (rec_buffer[4] == 1) {
            // We only support length 1 when reading info
            if (rec_buffer[4] == 0 && rec_buffer[5] == 1) {
                    // We can ignore MSB of channel (rec_buffer[0])
                    if (rec_buffer[0] == 0) {
                        send_data(rec_buffer[1]);
                    }
                }
            }
        }

    // Send error msg
    send_error();
    
    // Reset recv head
    rec_head = 0;
}



void define_METATEDS(void) {
    uint8_t array1[] = {3, 4, 0, 1, 0, 1};
    memcpy(METATED.TEDSID, array1, 6);
    uint8_t array2[] = {4, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    memcpy(METATED.UUID, array2, 12);
    uint8_t array3[] = {13, 2, 0, 2};
    memcpy(METATED.MAXCHAN, array3, 4);

    return;
}

void define_TCTEDS(void) {
    //  TCTEDS1 - TEDS DO TRANSDUCER CHANNEL 1 - Accelerometer X axis
    // TEDSID = {type, length, family, class, version, tuple length} // “TEDS Identification Header" - Slide 9
    // In our case, 
    // T type field     -> 3 (type for TEDS), 
    // L length field   -> 4 (Value field has 4 octets)
    // V value field    ->  family = 0, class = 1 (slide 10), version = 0 (slide 11, prototype), tuple length = 1 (All data blocks within a TED will have Length fields of 1 byte/1 octet, so this value is 1.)
    // Also see slide 24
    uint8_t array1[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS1.TEDSID, array1, 6);

    // TransducerChannel TEDS Channel Type - Slide 24
    // TLV structure:
    // 11: Channel type data block identifier (Table 48)
    // 01: 1 byte in the value field (Table 48)
    // 00: Sensor (Table 50)
    uint8_t array2[] = {11, 1, 0};
    memcpy(TCTEDS1.CHANNEL_TYPE, array2, 3);

    // TransducerChannel Units - Slide 25
    // TLV streucture:
    // 12: Units data block identifier (Table 48)
    // 10: 10 bytes in the value field (Table 48)
    // then the value itself
    // for this case we want to send the value "m/s^2":
    // exponent for m is 1 so we want (2*1) + 128 = 130 in the m field (4)
    // exponent for s is -2 so we want (2*-2) + 128 = 124 in the s field (6)
    uint8_t array3[] = {12, 10, 0, 128, 128, 130, 128, 124, 128, 128, 128, 128};
    memcpy(TCTEDS1.UNITS, array3, 12);

    // TransducerChannel TEDS High/Low range limits - Slide 26 TODO
    uint8_t array4[] = {13, 1, 0};
    memcpy(TCTEDS1.LOW_RANGE_LIMIT, array4, 3);
    uint8_t array5[] = {14, 1, 255};
    memcpy(TCTEDS1.HIGH_RANGE_LIMIT, array5, 3);
    
    // TransducerChannel TEDS Data model - Slide 26
    // we will work with with UInt8, so the data field is 00
    uint8_t array6[] = {40, 1, 0};
    memcpy(TCTEDS1.DATA_MODEL, array6, 3);

    // TransducerChannel TEDS Data model length
    // The sensor will only return one byte of data at a time
    uint8_t array7[] = {41, 1, 1};
    memcpy(TCTEDS1.DATA_MODEL_LENGTH, array7, 1);

    // TransducerChannel TEDS Data model significant bits:
    // Since my sensor sends 1 full byte of data, I used 8 significant bits.
    uint8_t array8[] = {42, 1, 8};
    memcpy(TCTEDS1.MODEL_SIG_BITS, array8, 3);

    // The Y and Z axis will be the same as the X axis, so we will just copy the values from the X axis
    // Y
    memcpy(TCTEDS2.TEDSID, TCTEDS1.TEDSID, 6);
    memcpy(TCTEDS2.CHANNEL_TYPE, TCTEDS1.CHANNEL_TYPE, 3);
    memcpy(TCTEDS2.UNITS, TCTEDS1.UNITS, 12);
    memcpy(TCTEDS2.LOW_RANGE_LIMIT, TCTEDS1.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS2.HIGH_RANGE_LIMIT, TCTEDS1.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS2.DATA_MODEL, TCTEDS1.DATA_MODEL, 3);
    memcpy(TCTEDS2.DATA_MODEL_LENGTH, TCTEDS1.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS2.MODEL_SIG_BITS, TCTEDS1.MODEL_SIG_BITS, 3);

    // Z
    memcpy(TCTEDS3.TEDSID, TCTEDS1.TEDSID, 6);
    memcpy(TCTEDS3.CHANNEL_TYPE, TCTEDS1.CHANNEL_TYPE, 3);
    memcpy(TCTEDS3.UNITS, TCTEDS1.UNITS, 12);
    memcpy(TCTEDS3.LOW_RANGE_LIMIT, TCTEDS1.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS3.HIGH_RANGE_LIMIT, TCTEDS1.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS3.DATA_MODEL, TCTEDS1.DATA_MODEL, 3);
    memcpy(TCTEDS3.DATA_MODEL_LENGTH, TCTEDS1.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS3.MODEL_SIG_BITS, TCTEDS1.MODEL_SIG_BITS, 3);


    // Potentiometer on channel 4
    // Is the same as the accelerometer, but the units are different
    memcpy(TCTEDS4.TEDSID, TCTEDS1.TEDSID, 6);
    memcpy(TCTEDS4.CHANNEL_TYPE, TCTEDS1.CHANNEL_TYPE, 3);
    // The units are in volts, so kg m^2 s^-3 A^-1
    uint8_t array9[] = {12, 10, 0, 128, 128, 132, 130, 122, 126, 128, 128, 128};
    memcpy(TCTEDS4.UNITS, array9, 12);
    memcpy(TCTEDS4.LOW_RANGE_LIMIT, TCTEDS1.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS4.HIGH_RANGE_LIMIT, TCTEDS1.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS4.DATA_MODEL, TCTEDS1.DATA_MODEL, 3);
    memcpy(TCTEDS4.DATA_MODEL_LENGTH, TCTEDS1.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS4.MODEL_SIG_BITS, TCTEDS1.MODEL_SIG_BITS, 3);
    
    return;
}


void send_METATEDS(void) {

    putch(1);
    putch(0);
    putch(22);
    for (int i = 0; i < 6; i++) {
        putch(METATED.TEDSID[i]);
    }
    for (int j = 0; j < 12; j++) {
        putch(METATED.UUID[j]);
    }
    for (int k = 0; k < 4; k++) {
        putch(METATED.MAXCHAN[k]);
    }
    return;
}


void send_TCTEDS(uint8_t channel) {
    // We will have 4 channels
    // 3 axis for the accelerometer and 1 for the potentiometer
    
    if (channel == 1) {
        putch(1);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS1.TEDSID[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS1.CHANNEL_TYPE[i]);
        }
        for (int i = 0; i < 12; i++) {
            putch(TCTEDS1.UNITS[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS1.LOW_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS1.HIGH_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS1.DATA_MODEL[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS1.DATA_MODEL_LENGTH[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS1.MODEL_SIG_BITS[i]);
        }
    } else if (channel == 2) {
        putch(2);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS2.TEDSID[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS2.CHANNEL_TYPE[i]);
        }
        for (int i = 0; i < 12; i++) {
            putch(TCTEDS2.UNITS[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS2.LOW_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS2.HIGH_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS2.DATA_MODEL[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS2.DATA_MODEL_LENGTH[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS2.MODEL_SIG_BITS[i]);
        }
    } else if (channel == 3) {
        putch(3);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS3.TEDSID[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS3.CHANNEL_TYPE[i]);
        }
        for (int i = 0; i < 12; i++) {
            putch(TCTEDS3.UNITS[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS3.LOW_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS3.HIGH_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS3.DATA_MODEL[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS3.DATA_MODEL_LENGTH[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS3.MODEL_SIG_BITS[i]);
        }
    } else if (channel == 4) {
        putch(4);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS4.TEDSID[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS4.CHANNEL_TYPE[i]);
        }
        for (int i = 0; i < 12; i++) {
            putch(TCTEDS4.UNITS[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS4.LOW_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS4.HIGH_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS4.DATA_MODEL[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS4.DATA_MODEL_LENGTH[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS4.MODEL_SIG_BITS[i]);
        }
    } else {
        // send error message
        send_error();
    }

    return;
}




void send_data(uint8_t channel) {
    // We will have 4 channels
    // 3 axis for the accelerometer and 1 for the potentiometer
    // Success/Fail Flag 01
    // Length (MSB) 00
    // Length (LSB) 01 (I have requested the sensor to read 1 value, which needs 1 byte)
    // Data XX

    // ADPCH
    // 001010 RB2/ ANB2 // Z
    // 001001 RB1/ ANB1 // Y
    // 001000 RB0/ANB0 // X
    // 000000 RA0/ANA0 // potenciometro
    
    // Read X axis
    if (channel == 1) {
        ADPCH = 0b00001000;
        
    // Read Y axis
    } else if (channel == 2) {
        ADPCH = 0b00001001;
        
    // Read Z axis
    } else if (channel == 3) {
        ADPCH = 0b00001010;
        
    // Read Potentiometer
    } else if (channel == 4) {
        ADPCH = 0b00000000;
        
    } else {
        // send error message
        send_error();
        return;
    }
    
    // Start new conversion
    ADCON0bits.ADGO = 1;
    // Ensure ADC conversion is over 
    while (PIR1bits.ADIF == 0){
        ;
    }
    // Reset ADC flag
    PIR1bits.ADIF = 0;
     

    // Now send data
    // Success/Fail Flag
    putch(01); // Success
    // Length  
    // MSB
    putch(0);
    // LSB
    putch(1);
    // Data
    putch(ADRESL);
    

    
    return;
}




void send_error() {
    for (int i = 0; i < 6; i++) {
            putch(0);
        }
    return;
}