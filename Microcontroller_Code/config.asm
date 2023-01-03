#include <xc.inc>

GLOBAL _config ;define the function to link it with the C definition

    
PSECT text0,local,class=CODE,reloc=2
    
CONFIG FEXTOSC=0b100 ;deactivate external oscillator (to allow write to RA7)
CONFIG CSWEN=0b1 ;allow editing NDIV and NOSC for CLK config
CONFIG WDTE=OFF ;required to avoid WDT restarting micro all the time

    
_config:
    
    ;===============
    ;CONFIGURE PORTA
    ;===============
    BANKSEL LATA
    CLRF LATA,1 ; Set all LatchA bits to zero
    
    MOVLW 0b00000000
    BANKSEL TRISA
    MOVWF TRISA,1 ;defines the pin direction. 0=out, 1=in. 
    
    MOVLW 0b00000000
    BANKSEL ANSELA
    MOVWF ANSELA,1 ;analog select. 
    
    ;===============
    ;CONFIGURE PORTB
    ;===============
    // RB0, RB1, RB2 will be the connections to Rx, Ry, Rz respectively
    // of the accelerometer
    BANKSEL LATB
    CLRF LATB,1 ; Set all LatchB bits to zero
    
    MOVLW 0b00000111 
    BANKSEL LATB
    MOVWF LATB,1 ;defines the pin direction. 0=out, 1=in. RA0 connects to potenciometer. All other are output pins
    
    MOVLW 0b00000111
    BANKSEL ANSELB
    MOVWF ANSELB,1 ; analog select. RB0, RB1, RB2  connects to accelerometer. The others are digital pins
    
    ;===============
    ;CONFIGURE PORTC
    ;===============
    BANKSEL LATC
    CLRF LATC,1 ; Set all LatchD bits to zero
    
    BANKSEL TRISC
    CLRF TRISC,1 ;All pins are output
    BSF  TRISC,7 ;except RC7, that will be used as RX - microchip specifies RX as input pin
    
    BANKSEL ANSELC
    CLRF ANSELC,1 ;All digital pins
    
    // Pag 290, 294 TODO: Check
    BANKSEL RC4PPS
    MOVLW 0b00001001  ; Q: WHAT VALUE MUST WE GIVE TO RC4PPS TO PUT THE EUSART1 TX IN PIN RC4? HINT: look in the 17.2 section of the datasheet, table 17.2
    MOVWF RC4PPS   ;place the EUSART1 (TX/CK) in RC4  
    
    BANKSEL RX1PPS
    MOVLW 0b00010111   ; Q: WHAT VALUE MUST WE GIVE TO RX1PPS TO PUT THE EUSART1 RX IN PIN RC7? HINT: look in the PPS chapter of the datasheet
    
    MOVWF RX1PPS  ;place the EUSART1 (RX) in RC7
    
    
    
    
    ;===============
    ;CONFIGURE CLOCK
    ;===============
    BANKSEL OSCCON1
    MOVLW 0b01100000  ;NOSC=0110 (internal high speed osc)
    MOVWF OSCCON1,1   ;NDIV=0000 (divider=1, clk divided by 1)
    
    BANKSEL OSCFRQ
    MOVLW 0b0000110 ; HFFRQ 0110 -> clk= 32 MHz
    MOVWF OSCFRQ,1
    
    BANKSEL OSCEN
    MOVLW 0b01000000 ;internal clock @freq=OSCFRQ ativo
    MOVWF OSCEN,1
    
    ;===============
    ;CONFIGURE USART
    ;===============
    ;BR=9600 @ CLK=32 MHz
    
    // Usefull registers: RCxREG TXxREG
    
    ; Q: WHAT VALUE SHOULD WE PUT IN SP1BRGL AND SP1BRG1H TO GET A BAUD RATE OF 9600 BPS FOR A CLOCK SPEED OF 32 MHZ? HINT: CHECK SECTION 28.2
    // 503 - We have to configure with (32000000/(64*9600)) -1 = 51 = 0b110011. We have to configure in H or L?
    BANKSEL SP1BRGL
    movlw 0b110011  
    movwf SP1BRGL
    movlw 0b00000000 // Ignoring high bc of 8 bit
    movwf SP1BRGH
    
    BANKSEL TX1STA
    movlw 0b00100000   ;8 data bits, TX enabled, ASYNC
    movwf TX1STA
    
    ; Q: HERE YOU MUST ENABLE THE USART AND THE RECEIVER WITH REGISTER RC1STA
    movlw 0b10110000 // Enable Serial Port, Put 8-bit, Single Receive Enable. USART enabled, 8 data bits / enable receiver 
    BANKSEL RC1STA
    movwf RC1STA 
    

    
    ;=============
    ;CONFIGURE ADC
    ;=============
    
    ; ADPCH Possibilities
    ; 001011 RB3/ANB3
    ; 001010 RB2/ ANB2
    ; 001001 RB1/ ANB1
    ; 001000 RB0/ANB0
    ; 000000 RA0/ANA0
    
    BANKSEL ADPCH
    MOVLW 0b00001000   ;set RA0 as ADC input
    MOVWF ADPCH,1
    
    BANKSEL ADREF
    MOVLW 0b00000000  ;Vref set to vdd and vss
    MOVWF ADREF,1
    
    // 567 - ADCfreq= FOSC/(2*(n+1)) 
    // Logo, n = (32Mhz/2*1Mhz) -1 = 15 para dar ADCfreq=1Mhz
    BANKSEL ADCLK 
    MOVLW 0b00001111   ; Q: SET THE ADC CLOCK FREQUENCY TO 1 MHZ, KNOWING THAT FOSC = 32 MHZ
    MOVWF ADCLK,1
    
    // 562 TODO: Check
    BANKSEL ADCON0
    MOVLW 0b00000000    ;Q: SET THE ADC TO: results left-justified, clock=Fosc/div, non-continuous operation
    MOVWF ADCON0,1
    
    
    
    ;================
    ;CONFIGURE TIMER0
    ;================
    ;BANKSEL T0CON0
    ;CLRF T0CON0
    
    // 303
    ;BANKSEL T0CON1
    ;MOVLW 0b01001100  //Q: SET THE PRESCALER TO ~= 16000
    ;MOVWF T0CON1
    
    //BANKSEL T0CON0
    //MOVLW 0b11000001
    //MOVWF T0CON0
   
    ;BANKSEL TMR0L
    ;CLRF TMR0L  ;clear the counter
    
    ;=================
    ;ENABLE INTERRUPTS
    ;=================
    BANKSEL PIR0
    BCF PIR0, 5 ;clear timer interrupt flag
    BANKSEL PIR1
    BCF PIR1,0  ;clear ADC interrupt flag
    BANKSEL PIR3 
    BCF PIR3,5 ;clear RX1 interrupt flag
    
    BANKSEL PIE0
    BSF PIE0, 5  ; Q: enable timer int
    BANKSEL PIE1
    BSF PIE1, 0  ; Q: enable adc int
    BANKSEL PIE3
    BSF PIE3, 5  ; Q: enable RX1 int
    
    BANKSEL T0CON0
    BSF T0CON0,7   ;start timer 0
    BANKSEL ADCON0  
    BSF ADCON0,7   ;ENABLE ADC

    
    
    ; Q: HERE YOU MUST ENABLE PERIPHERAL INTERRUPTIONS AND GLOBAL INTERRUPTIONS
    BANKSEL INTCON
    BSF INTCON, 7
    BSF INTCON, 6
    
    
    
    RETURN


