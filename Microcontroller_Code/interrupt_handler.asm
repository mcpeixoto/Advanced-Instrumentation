#include <xc.inc>

    
GLOBAL _int_handler, _Identify_NCAP_cmd  ;declare global functions
    
PSECT intcode

#define parcel_msb 0

    
_int_handler:    ;when an interrupt happens, this function is called. It is your job to verify what interrupt happened and act accordingly

    ; Clear bit 1 of RC1STA register BUG FIX RECEIVING
    BCF RC1STA, 1
    
    BANKSEL PIR3
    BTFSC PIR3, 5   ;Q: check if the EUSART1 RX flag is set. If so, go to the C function _getch. If not, skip.  
    call _Identify_NCAP_cmd

    RETFIE
    

    



