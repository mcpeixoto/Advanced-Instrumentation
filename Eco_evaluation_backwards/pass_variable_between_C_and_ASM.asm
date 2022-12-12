#include <xc.inc>
    
GLOBAL _pass_variable_between_C_and_ASM, _var_global  ;we must declare the global variable

    
PSECT text3,local,class=CODE,reloc=2
    
#define temp_variable  3

;This function does 2 things:
; 1 - receives an input, adds 5 and returns the result
; 2 - adds 1 to the global variable

;the argument we give the function (in this example, 3) when we call it in C is stored in the working register W
;when the function ends, it returns the value of W to the variable "result" in C
    
_pass_variable_between_C_and_ASM:
    ;the argument (3) is stored in W when we call the function.
    ADDLW 0x05  ;Now we add "5" to W.
    
    ;now we are going to add a value to the global variable var_global, and we will need to use the W register. Since the function returns the value of W, we must save it.
    MOVWF temp_variable  ;store W in a variable
    
    ;add 1 to the global variable
    BANKSEL _var_global
    MOVLW 0x01   
    ADDWF _var_global,1   ;the "1" in this instruction tells the compiler to save the result of the ADD to _var_global. If it was "0" it would be saved in "W".
   
    ;now we need to recover the W to return the result of adding (3) with (5).
    ANDLW 0x00  ;set W as 0
    ADDWF temp_variable, 0   ;put the variable value back into W for the return. Here it is "0" so that the ADD is saved into W.
    
    return   ;when the function returns, the _var_global value is already updated and it will return the W value back to C for it to put in "result"




