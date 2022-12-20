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
void config(void);
void __interrupt() int_handler(void); 


void define_METATEDS(void);
void define_TCTEDS(void);
void Identify_NCAP_cmd(void);
void send_METATEDS(void);
void send_TCTEDS(uint8_t);
uint8_t get_char (void);
void fail (void);
void write_success(void);
void enviar_valor(uint8_t canal);
char pos;
char var_global;
uint8_t info_ind = 0;   // indice do proximo array
uint8_t info[6];        // array que vai receber primeiros 6 bits da NCAP
uint8_t value_ind= 0;   // indice do proximo array
uint8_t value[256];     // array que recebe offset e valores da NCAP
void echo (void);       // fun��o de teste da comunica��o 
uint8_t test;           // variavel de teste 

//create the structs for the Meta TEDS and for the TransducerChannels TEDS for 3 channels
struct METATEDS_TEMPLATE METATED;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS1;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS2;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS3;


void main(void) {


    config();

    define_METATEDS();
    define_TCTEDS();

    
    while (1) {
        ;
        }
    
}

void putch(char byte) 
{
    while (PIR3bits.TX1IF == 0) {
        ;
    }
    TX1REG = byte; /* transmit a character to Serial IO */

    //wait until it is sent
    while (PIR3bits.TX1IF == 0) {
        ;
    }
}
// C�DIGO FEITO EM CASA, SABENDO QUE N�O � NECESS�RIO UMA INTERRUP��O PARA GUARDAR O VALOR QUE RECEBEMOS


// SEMPRE QUE ACONTECE UMA INTERRUP��O ESTA FUN��O � CHAMADA E S�O LIDOS TODOS OS BYTES NECESS�RIOS E CONSECUTIVOS,  
//    DE SEGUIDA � FEITA A DESCODIFICA��OE E ENVIO DOS VALORES E MENSAGNES DE SUCESSO. QUANDO TUDO FOR ENVIADO
//    INCLUINDO A MENSAGEM DE ERRO, FAZEMOS RETURN.
//    E PODEREMOS ENT�O RECEBER A PROXIMA INTERRUP��O DE RECEP��O PROVENIENTE DO PROXIMO PEDIDO DA NCAP. 



// !! DE NOTAR QUE ENQUANTO ESTA INTERRUP��O EST� A CORRER, OUTRAS N�O SER�O OUVIDAS, LOGO SE FOR NECESS�RIO USAR O ADC DENTRO 
//      DESTA FUN��O, N�O PODEMOS USAR NENHUMA INTERRUP��O PARA IR PERGAR NUMA CONVERS�O ( N�O HAVER INTERRUP��O N�O QUER DIZER QUE A
//      FLAG DO ADC TER FEITO UMA CONVERS�O N�O � POSTA A 1, APENAS N�O H� NENHUMA UM INTERRUPT HANDLER PARA O ADC) DESTA 
//      FORMA TERMOS QUE VERIFICAR A FLAG E PEGAR NA CONVERS�O DO ADC TUDO FEITO NA MAIN, EM C.!!


uint8_t get_char (void){
    while ( RC1IF != 1){   // enquanto que o RC1REG n�o tiver nada, n�o faz nada;
    }
    return RC1REG; // se tiver devolve o que est� l�, e segundo datasheet (pag), a flag RC1IF vai a 0 at� o RC1REG estiver cheio outra vez
}   

void Identify_NCAP_cmd(void) {
    
    // GUARDAR OS BYTES EM ARRAYS DIFERENTES
    PIR3bits.TX1IF = 0;
    while (info_ind < 6){
        info[info_ind] = get_char();   // preenchemos o array info com os 6 bytes da instru��o
        info_ind += 1;
    }
    info_ind = 0;
    while (value_ind < info[5] ){
        value[value_ind] = get_char();   // preenchemos o array value dependendo do tamanho (LSB) desse valor
        value_ind += 1;
    }
    value_ind = 0;
    
    // A SEQUIR VAMOS DESCODIFICAR OS BYTES E ENVIAR O QUE FOR NECESS�RIO
    
    
    if( (info[2] == 1) && (info[3] == 2) &&  (info[5] == 2) && (value[0] == 1)){ // TESTA SE NCAP PEDE METATED
        //common command -> 1; read Teds seg ; len -> 2; Meta TEDS -> 01 ;
    
        send_METATEDS(); 
        return;
    }
    
    if( info[1] <= 3 ){ // depende de quantos canais temos, neste caso temos 3 canais 
        
        if ( (info[2] == 1) && (info[3] == 2) &&  (info[5] == 2) && (value[0] == 3)){ // TESTA SE NCAP PEDE UMA TED
            //common command -> 1; read Teds seg ; len -> 2; TC TEDS -> 3 ;
            
            send_TCTEDS(info[1]); // ENVIA A TCTED DO CANAL PEDIDO  
            return;
        }
        
        if (info[2] == 3){             // verifica que � um transducer 
            
            if (info[3] == 1){         // ler do transdutor do canal info[1]
                enviar_valor(info[1]); 
                return;
            }
            
            if ((info[3] == 2) && (info[1] == 3)){ //escrever no transdutor do canal 3 (unico permitido psrs escrita) 
                DAC1CON1 = value[1];
                write_success();    // enviar a NCAP mensagem de sucesso
                return;
            } 
        }
    }
    // se n�o foi nenhuma das anteriores d� erro
    fail();
    
    return;
    
}
void fail (void){      // codigo de erro 
    for(int i = 0; i<3;i++){
        putch(0);
    }
    return;
}
void write_success(void){   // codigo de sucesso 
    putch(1);
    for(int i = 0; i<2;i++){
        putch(0);
    }
    return;
}
 void enviar_valor(uint8_t canal){
      
      putch(1);
      putch(0);      // mensagem de sucesso
      putch(1);
      
 
      if(canal==1) {
        ADPCH = 0B00000001 ;        // O input do ADC � o pin RA1 para o valor do x
        ADCON0bits.ADGO = 1 ;       // fazer uma convers�o
        while(PIR1bits.ADIF == 0){
            ;
        }
        putch(ADRESL);              // mandar valor 
        PIR1bits.ADIF = 0;          // reset da flag do ADC
      }
      if(canal==2) {
        ADPCH = 0B00000010 ;        // O input do ADC � o pin RA2 para o valor do y
        ADCON0bits.ADGO = 1 ;       // fazer uma convers�o
        while(PIR1bits.ADIF == 0){
            ;
        }
        putch(ADRESL);              // mandar valor
        PIR1bits.ADIF = 0;          // reset da flag do AD
      }
      if(canal==3) {
        putch(DAC1CON1);            // se quiser ler do transducer 3 escrever o valor do DAC 
      }
      
 }





void define_METATEDS(void) {
    uint8_t array1[] = {3, 4, 0, 1, 0, 1};
    memcpy(METATED.TEDSID, array1, 6);
    uint8_t array2[] = {4, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    memcpy(METATED.UUID, array2, 12);
    uint8_t array3[] = {13, 2, 0, 3}; // 2 sensores e 1 atuador
    memcpy(METATED.MAXCHAN, array3, 4);

    return;
}

void define_TCTEDS(void) {
    //TCTEDS1 - TEDS DO TRANSDUCER CHANNEL 1 - SENSOR
    uint8_t array11[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS1.TEDSID, array11, 6);
    uint8_t array21[] = {11, 1, 0};
    memcpy(TCTEDS1.CHANNEL_TYPE, array21, 3);
    uint8_t array31[] = {12, 10, 0, 128, 128, 132, 130, 122, 126, 128, 128, 128};
    memcpy(TCTEDS1.UNITS, array31, 12);
    uint8_t array41[] = {13, 1, 0};
    memcpy(TCTEDS1.LOW_RANGE_LIMIT, array41, 3);
    uint8_t array51[] = {14, 1, 255};
    memcpy(TCTEDS1.HIGH_RANGE_LIMIT, array51, 3);
    uint8_t array61[] = {40, 1, 0};
    memcpy(TCTEDS1.DATA_MODEL, array61, 3);
    uint8_t array71[] = {41, 1, 1};
    memcpy(TCTEDS1.DATA_MODEL_LENGTH, array71, 3);
    uint8_t array81[] = {42, 1, 8};
    memcpy(TCTEDS1.MODEL_SIG_BITS, array81, 3);

    
    
    ///TCTEDS2 - TEDS DO TRANSDUCER CHANNEL 2 - SENSOR
    
    
    uint8_t array12[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS2.TEDSID, array12, 6);
    uint8_t array22[] = {11, 1, 0};
    memcpy(TCTEDS2.CHANNEL_TYPE, array22, 3);
    uint8_t array32[] = {12, 10, 0, 128, 128, 132, 130, 122, 126, 128, 128, 128};
    memcpy(TCTEDS2.UNITS, array32, 12);
    uint8_t array42[] = {13, 1, 0};
    memcpy(TCTEDS2.LOW_RANGE_LIMIT, array42, 3);
    uint8_t array52[] = {14, 1, 255};
    memcpy(TCTEDS2.HIGH_RANGE_LIMIT, array52, 3);
    uint8_t array62[] = {40, 1, 0};
    memcpy(TCTEDS2.DATA_MODEL, array62, 3);
    uint8_t array72[] = {41, 1, 1};
    memcpy(TCTEDS2.DATA_MODEL_LENGTH, array72, 3);
    uint8_t array82[] = {42, 1, 8};
    memcpy(TCTEDS2.MODEL_SIG_BITS, array82, 3);

    
    //TCTEDS3 - TEDS DO TRANSDUCER CHANNEL 3 - ACTUATOR
    
    uint8_t array13[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS3.TEDSID, array13, 6);
    uint8_t array23[] = {11, 1, 1};
    memcpy(TCTEDS3.CHANNEL_TYPE, array23, 3);
    uint8_t array33[] = {12, 10, 0, 128, 128, 132, 130, 122, 126, 128, 128, 128};
    memcpy(TCTEDS3.UNITS, array33, 12);
    uint8_t array43[] = {13, 1, 0};
    memcpy(TCTEDS3.LOW_RANGE_LIMIT, array43, 3);
    uint8_t array53[] = {14, 1, 255};
    memcpy(TCTEDS3.HIGH_RANGE_LIMIT, array53, 3);
    uint8_t array63[] = {40, 1, 0};
    memcpy(TCTEDS3.DATA_MODEL, array63, 3);
    uint8_t array73[] = {41, 1, 1};
    memcpy(TCTEDS3.DATA_MODEL_LENGTH, array73, 3);
    uint8_t array83[] = {42, 1, 8};
    memcpy(TCTEDS3.MODEL_SIG_BITS, array83, 3);
    
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
//void echo (void){           // fun��o teste que cospe valores recebidos pela NCAP
  //  putch (get_char());
    //putch (get_char());
   // putch (get_char());
   // putch (get_char());
   // putch (get_char());
   // test = get_char();      // o tamanhp do que vem para a frente depende do sexto byte 
   // putch (test);
   // for(int i =0; i < test; i++){
    //    putch (get_char());
   // }
   // return;
//}

void send_TCTEDS(uint8_t channel) {
    
    //channel1 TEDS
    
    if (channel == 1) {         // enviar TED do canal 1 
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
    } 
    
    //channel2 TEDS
    
        if (channel == 2) {         // enviar TED do canal 2
        putch(1);
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
    } 
    //channel3 TEDS
    
     if (channel == 3) {            // enviar TED do canal 3
        putch(1);
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
    } 
    return;
}

void send_values(uint8_t canal){        // fun��od de teste 
    if (canal == 1){
        putch(0);
        putch(1);
        
    }
}