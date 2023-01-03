/*Autores:
 * Maria Gabriela Jord�o Oliveira 
 * Miguel Ca�ador Peixoto
 */


#include <xc.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include "TEDS_definition.h"

#define QUEUE_IMPLEMENTATION
#define __QMA
#include "media_movel.h"

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
void send_values (uint8_t);
void enviar_valor(uint8_t canal);
char pos;
char var_global;
uint8_t info_ind = 0;   // indice do proximo array
uint8_t info[6];        // array que guarda primeiros 6 bits da NCAP
uint8_t value_ind= 0;   // indice do proximo array
uint8_t value[256];     // array para guardar offset e valores da NCAP


//create the structs for the Meta TEDS and for the TransducerChannels TEDS for 3 channels
struct METATEDS_TEMPLATE METATED;
// 3 eixos do aceler�metro
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS1;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS2;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS3;
// 3 leds
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS4;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS5;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS6;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS7;

void main(void) {


    config();
    // inicializar as queue para a m�dia m�vel
    init_queue(&Q_X);
    init_queue(&Q_Y);
    init_queue(&Q_Z);
    
    // definir a metated e a teds
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


uint8_t get_char (void){
    while ( RC1IF != 1){   //Esperar por uma interrup��o externa
    }
    return RC1REG; // Retorna o que est� no registo RC1REG! Received data. Ap�s a leitura do buffer a flag fica a 0.
}   

void Identify_NCAP_cmd(void) {
    
    // Primeiros 6 bytes
    PIR3bits.TX1IF = 0;
    while (info_ind < 6){
        info[info_ind] = get_char();   
        info_ind += 1;
    }
    // Come�ar a guardar offset e valores com base no campo 5 do array anterior 
    info_ind = 0;
    while (value_ind < info[5] ){
        value[value_ind] = get_char();   
        value_ind += 1;
    }
    value_ind = 0;
    
    // descodifica��o
    // Vamos receber bytes em hexa - slide 30
    // 2 hex - Destination Transducer MSB - info[0]
    // 2 hex - Destination Transducer LSB  - info[1]
    // 2 hex - CMD Class (slide 31 - 1 e 3) - info[2]
    // 2 hex - CMD function (slide 32)   - info[3]
    // 2 hex - Length MSB MSB - info[4]
    // 2 hex - Length MSB LSB - info[5]
    // 2 hex - Length defined from above, MSB Command-dependent octets - info[6]
    // 2 hex - Length defined from above, LSB Command-dependent octets  - info[7]
    
    
    // Nota:
    // Quando se pede a Meta Teds, o transdutor de destino n�o interessa
    
    
    // CMD Class Slide 31, tab 15   
    // 01 - Comando comum do TIM  e Trandutores (Para pedir informa��es)
    // 03 - Tranducer operating state - ler dados
    
    //CMD Functions 
    // Classe de Metateds Slide 32, tab 16 (1))
    // 02 - Ler TEDS
    // Classe de Transducer operating state Slide 38 (3)
    // 01 - Ler dos canais
    
    // Command dependant Slide 33, tab 17
    // 01 - METATEDS
    // 03 - Transdutores
    
    // This function needs to respond with:
    // 2 hex - Success/Fail Flag 
    // 2 hex - Length (MSB)
    // 2 hex - Length (LSB)
    // x hex - Data (x = Length)
    
    if( (info[2] == 1) && (info[3] == 2) &&  (info[5] == 2) && (value[0] == 1)){ // TESTA SE NCAP PEDE METATED
    
        send_METATEDS(); 
        return;
    }
    
    if( info[1] <= 7 ){ // depende de quantos canais temos, neste caso temos 3 canais 
        
        if ( (info[2] == 1) && (info[3] == 2) &&  (info[5] == 2) && (value[0] == 3)){ // TESTA SE NCAP PEDE UMA TED
            //common command -> 1; read Teds seg ; len -> 2; TC TEDS -> 3 ;
            
            send_TCTEDS(info[1]); // ENVIA A TCTED DO CANAL PEDIDO  
            return;
        }
        
        if (info[2] == 3){             // transudutor
            
            if (info[3] == 1){         // ler do transdutor do canal info[1]
                send_values(info[1]); 
                return;
            }
            //leds
             if ((info[3] == 2) && (info[1] == 4)){ //escrever no transdutor do canal 4
                
                LATAbits.LATA4 = value[1];
                write_success();    // enviar a NCAP mensagem de sucesso
                return;
            }
            if ((info[3] == 2) && (info[1] == 5)){ //escrever no transdutor do canal 5 
                LATAbits.LATA5 = value[1];
                write_success();    // enviar a NCAP mensagem de sucesso
                return;
            }
            if ((info[3] == 2) && (info[1] == 6)){ //escrever no transdutor do canal 6 
                LATAbits.LATA6 = value[1];
                write_success();    // enviar a NCAP mensagem de sucesso
                return;
            }  
             if ((info[3] == 2) && (info[1] == 7)){ //escrever no transdutor do canal 7
                LATAbits.LATA7 = value[1];
                write_success();    // enviar a NCAP mensagem de sucesso
                return;
            }  
        }
    }
    // mensagem de erro
    fail();
    
    return;
    
}

// mensagem de erro, 0 0 0
void fail (void){      // codigo de erro 
    for(int i = 0; i<3;i++){
        putch(0);
    }
    return;
}

// sucesso 1 0 0 (sem enviar mais nada)
void write_success(void){   // codigo de sucesso 
    putch(1);
    for(int i = 0; i<2;i++){
        putch(0);
    }
    return;
}

void define_METATEDS(void) {
    uint8_t array1[] = {3, 4, 0, 1, 0, 1};
    memcpy(METATED.TEDSID, array1, 6);
    uint8_t array2[] = {4, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    memcpy(METATED.UUID, array2, 12);
    uint8_t array3[] = {13, 2, 0, 7}; // 3 sensores e 4 atuador
    memcpy(METATED.MAXCHAN, array3, 4);

    return;
}

void define_TCTEDS(void) {
    //TCTEDS1 - TEDS DO TRANSDUCER CHANNEL 1 - SENSOR
    uint8_t array11[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS1.TEDSID, array11, 6);
    uint8_t array21[] = {11, 1, 0};
    memcpy(TCTEDS1.CHANNEL_TYPE, array21, 3);
    uint8_t array31[] = {12, 10, 0, 128, 128, 130, 128, 124, 128, 128, 128, 128};
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
    memcpy(TCTEDS2.TEDSID, TCTEDS1.TEDSID, 6);
    memcpy(TCTEDS2.CHANNEL_TYPE, TCTEDS1.CHANNEL_TYPE, 3);
    memcpy(TCTEDS2.UNITS, TCTEDS1.UNITS, 12);
    memcpy(TCTEDS2.LOW_RANGE_LIMIT, TCTEDS1.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS2.HIGH_RANGE_LIMIT, TCTEDS1.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS2.DATA_MODEL, TCTEDS1.DATA_MODEL, 3);
    memcpy(TCTEDS2.DATA_MODEL_LENGTH, TCTEDS1.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS2.MODEL_SIG_BITS, TCTEDS1.MODEL_SIG_BITS, 3);

    //TCTEDS3 - TEDS DO TRANSDUCER CHANNEL 3 - Sensor
    memcpy(TCTEDS3.TEDSID, TCTEDS1.TEDSID, 6);
    memcpy(TCTEDS3.CHANNEL_TYPE, TCTEDS1.CHANNEL_TYPE, 3);
    memcpy(TCTEDS3.UNITS, TCTEDS1.UNITS, 12);
    memcpy(TCTEDS3.LOW_RANGE_LIMIT, TCTEDS1.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS3.HIGH_RANGE_LIMIT, TCTEDS1.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS3.DATA_MODEL, TCTEDS1.DATA_MODEL, 3);
    memcpy(TCTEDS3.DATA_MODEL_LENGTH, TCTEDS1.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS3.MODEL_SIG_BITS, TCTEDS1.MODEL_SIG_BITS, 3);
    
    //TCTEDS4 - TEDS DO TRANSDUCER CHANNEL 4 - led
    uint8_t array14[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS4.TEDSID, array14, 6);
    uint8_t array24[] = {11, 1, 1};
    memcpy(TCTEDS4.CHANNEL_TYPE, array24, 3);
    uint8_t array34[] = {12, 10, 0, 128, 128, 132, 130, 122, 126, 128, 128, 128};
    memcpy(TCTEDS4.UNITS, array34, 12);
    uint8_t array44[] = {13, 1, 0};
    memcpy(TCTEDS4.LOW_RANGE_LIMIT, array44, 3);
    uint8_t array54[] = {14, 1, 1};
    memcpy(TCTEDS4.HIGH_RANGE_LIMIT, array54, 3);
    uint8_t array64[] = {40, 1, 0};
    memcpy(TCTEDS4.DATA_MODEL, array64, 3);
    uint8_t array74[] = {41, 1, 1};
    memcpy(TCTEDS4.DATA_MODEL_LENGTH, array74, 3);
    uint8_t array84[] = {42, 1, 8};
    memcpy(TCTEDS4.MODEL_SIG_BITS, array84, 3);
    
    
   //TCTEDS5 - TEDS DO TRANSDUCER CHANNEL 5 - led
    memcpy(TCTEDS5.TEDSID, TCTEDS4.TEDSID, 6);
    memcpy(TCTEDS5.CHANNEL_TYPE, TCTEDS4.CHANNEL_TYPE, 3);
    memcpy(TCTEDS5.UNITS, TCTEDS4.UNITS, 12);
    memcpy(TCTEDS5.LOW_RANGE_LIMIT, TCTEDS4.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS5.HIGH_RANGE_LIMIT, TCTEDS4.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS5.DATA_MODEL, TCTEDS4.DATA_MODEL, 3);
    memcpy(TCTEDS5.DATA_MODEL_LENGTH, TCTEDS4.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS5.MODEL_SIG_BITS, TCTEDS4.MODEL_SIG_BITS, 3);

    
    
    //TCTEDS6 - TEDS DO TRANSDUCER CHANNEL 6 - led
    memcpy(TCTEDS6.TEDSID, TCTEDS4.TEDSID, 6);
    memcpy(TCTEDS6.CHANNEL_TYPE, TCTEDS4.CHANNEL_TYPE, 3);
    memcpy(TCTEDS6.UNITS, TCTEDS4.UNITS, 12);
    memcpy(TCTEDS6.LOW_RANGE_LIMIT, TCTEDS4.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS6.HIGH_RANGE_LIMIT, TCTEDS4.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS6.DATA_MODEL, TCTEDS4.DATA_MODEL, 3);
    memcpy(TCTEDS6.DATA_MODEL_LENGTH, TCTEDS4.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS6.MODEL_SIG_BITS, TCTEDS4.MODEL_SIG_BITS, 3);
    
    //TCTEDS7 - LED
    memcpy(TCTEDS7.TEDSID, TCTEDS4.TEDSID, 6);
    memcpy(TCTEDS7.CHANNEL_TYPE, TCTEDS4.CHANNEL_TYPE, 3);
    memcpy(TCTEDS7.UNITS, TCTEDS4.UNITS, 12);
    memcpy(TCTEDS7.LOW_RANGE_LIMIT, TCTEDS4.LOW_RANGE_LIMIT, 3);
    memcpy(TCTEDS7.HIGH_RANGE_LIMIT, TCTEDS4.HIGH_RANGE_LIMIT, 3);
    memcpy(TCTEDS7.DATA_MODEL, TCTEDS4.DATA_MODEL, 3);
    memcpy(TCTEDS7.DATA_MODEL_LENGTH, TCTEDS4.DATA_MODEL_LENGTH, 3);
    memcpy(TCTEDS7.MODEL_SIG_BITS, TCTEDS4.MODEL_SIG_BITS, 3);
    
    
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
        if (channel == 4) {            // enviar TED do canal 4
        putch(1);
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
    } 
    
    //channel2 TEDS
    
        if (channel == 2) {         // enviar TED do canal 2
        putch(1);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS2.TEDSID[i]);    // Now send data
    //putch(ADRESL);
    //putch(channel);
    
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
    if (channel == 5) {            // enviar TED do canal 5
        putch(1);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS5.TEDSID[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS5.CHANNEL_TYPE[i]);
        }
        for (int i = 0; i < 12; i++) {
            putch(TCTEDS5.UNITS[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS5.LOW_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS5.HIGH_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS5.DATA_MODEL[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS5.DATA_MODEL_LENGTH[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS5.MODEL_SIG_BITS[i]);
        }
    }
    if (channel == 6) {            // enviar TED do canal 6
        putch(1);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS6.TEDSID[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS6.CHANNEL_TYPE[i]);
        }
        for (int i = 0; i < 12; i++) {
            putch(TCTEDS6.UNITS[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS6.LOW_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS6.HIGH_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS6.DATA_MODEL[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS6.DATA_MODEL_LENGTH[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS6.MODEL_SIG_BITS[i]);
        }}
     if (channel == 7) {            // enviar TED do canal 7
        putch(1);
        putch(0);
        putch(36);

        for (int i = 0; i < 6; i++) {
            putch(TCTEDS7.TEDSID[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS7.CHANNEL_TYPE[i]);
        }
        for (int i = 0; i < 12; i++) {
            putch(TCTEDS7.UNITS[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS7.LOW_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS7.HIGH_RANGE_LIMIT[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS7.DATA_MODEL[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS7.DATA_MODEL_LENGTH[i]);
        }
        for (int i = 0; i < 3; i++) {
            putch(TCTEDS7.MODEL_SIG_BITS[i]);
        }}
    
    
    return;
}

void send_values(uint8_t channel){        
     //Vamos ter 3 canais (aceler�metro)
    
    // Success/Fail Flag 01
    // Length (MSB) 00
    // Length (LSB) 01 (I have requested the sensor to read 1 value, which needs 1 byte)
    // Data XX

    // ADPCH
    // 001010 RB2/ ANB2 // Z
    // 001001 RB1/ ANB1 // Y
    // 001000 RB0/ANB0 // X
   
    Queue*q=NULL;
    // Read X axis
    if (channel == 1) {
        ADPCH = 0b00001000;
        q=&Q_X;
    // Read Y axis
    } else if (channel == 2) {
        ADPCH = 0b00001001;
        q=&Q_Y;
    // Read Z axis
    } else if (channel == 3) {
        ADPCH = 0b00001010;
        q=&Q_Z;
    // Read Potentiometer
    } else if (channel == 4) {
        ADPCH = 0b00000000;
        
    } else {
        // send error message
        fail();
        return;
    }
    
    // Success/Fail Flag
    putch(1); // Success

    // Length  
    // MSB
    putch(0);
    // LSB
    putch(1);

    // Start new conversion
    ADCON0bits.ADGO = 1;

    // Ensure ADC conversion is over 
    while (PIR1bits.ADIF == 0){
        ;
    }
    
    // Send data
    putch(get_next_value(q,ADRESH));

    // Reset ADC flag
    PIR1bits.ADIF = 0;
     
    return;
}
