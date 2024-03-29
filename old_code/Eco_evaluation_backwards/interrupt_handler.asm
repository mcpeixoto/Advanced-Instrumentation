#include <xc.inc>

    
GLOBAL _int_handler, _getch, _adc_result, _show_adc_result, _comutar, _envia_bit     ;declare global functions
    
PSECT intcode

#define parcel_msb 0

    
_int_handler:    ;when an interrupt happens, this function is called. It is your job to verify what interrupt happened and act accordingly
    BANKSEL PIR0
    BTFSC PIR0, 5 ; Q:check if the timer0 interrupt flag is set. If so, go to timer0_int_handler. If not, skip.
    goto _timer0_int_handler
    
    BANKSEL PIR1
    BTFSC PIR1, 0   ;Q: check if the ADC interrupt flag is set. If so, go to adc_int_handler. If not, skip.
    goto _adc_int_handler
    
    BANKSEL PIR3
    BTFSC PIR3, 5   ;Q: check if the EUSART1 RX flag is set. If so, go to the C function _getch. If not, skip.  
    call _getch
    
    RETFIE
    

_timer0_int_handler:

    BANKSEL ADPCH
    MOVLW 0b00001000   ;set RA0 as ADC input
    MOVWF ADPCH,1
    // Pag 562
    // This initiates a new conversion
    BANKSEL ADCON0
    BSF ADCON0, 0
    
    BANKSEL PORTA
    BTG PORTA,5  ;TOGGLE LED ON PORTA,5
    BANKSEL PIR0
    BCF PIR0,5 ;clear timer_int flag
    
    call _envia_bit;
    
    RETFIE  ;return from interruption
    
    
_adc_int_handler:
    BANKSEL ADRESH
    MOVFF ADRESH, _adc_result  ;put the highest 8 bits of the conversion result in parcel_msb
     
    ;here I am just changing the value of the byte read by the ADC to a value we know for debug purposes
    ;MOVLW 0b11111111
    ;MOVWF parcel_msb
        
    ; Sends data to computer
    ;BANKSEL TX1REG
    ;MOVFF parcel_msb, TX1REG  ;Q: put the 8 MSB of the ADC conversion in the UART TX.
    
    BANKSEL PIR1
    BCF PIR1,0   ;clear the ADC interrupt flag
    
    ;call _show_adc_result
    RETFIE  ;return from interruption
    

    



