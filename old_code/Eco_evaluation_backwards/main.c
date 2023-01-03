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

#define _XTAL_FREQ 32000000   //the clock frequency defined in config() must be provided here for the delay function

//declaration of global variables and functions
void config(void);
void __interrupt() int_handler(void); //the interrupt handler routine must be declared as __interrupt(). When an INT happens, this function is called
extern unsigned char pass_variable_between_C_and_ASM(unsigned char a);
void putch(char); //function used to send data via UART. The printf uses this function too.
char var_global = 0;  
char get_char;
char comutar = 0;
char adc_result;


void main(void) {
    
    config();  //this ASM function configures the microcontroller
    
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
            putch(get_char);
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


void show_adc_result(void){
    
    // 001011 RB3/ANB3
    // 001010 RB2/ ANB2
    // 001001 RB1/ ANB1
    // 001000 RB0/ANB0
    // 000000 RA0/ANA0
    

    if (comutar == 0){
        printf("X:");
        putchar(adc_result);
        printf("\n");
        
        // Comutar para o Y RB1
        ADPCH = 0b00001001;
    }
    if (comutar == 1){
        printf("Y:");
        putchar(adc_result);
        printf("\n");
        
        // Comutar para o Z RB2
        ADPCH = 0b00001010;
    }
    if (comutar == 2){
        printf("Z:");
        putchar(adc_result);
        printf("\n");
        
        // Comutar para o X RB0
        ADPCH = 0b00001000;
    }
    
    
    
    comutar += 1;
    
    if (comutar == 3){
        comutar = 0;
    }

    
    return;
}


void envia_bit(void){
    adc_result=42;
    putchar(adc_result);
    return;
}

void getch(void)   //this function gets the received char from XXXXXX
{
    get_char = RC1REG;  //Q: WHEN UART RECEIVES A BYTE, WHERE DOES IT GO TO?
        
    return;
}
