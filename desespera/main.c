#include <xc.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include "TEDS_definition.h"

// The clock frequency defined in config() must be provided here for the delay function
#define _XTAL_FREQ 32000000

// Declaration of global variables and functions
void config(void);
void __interrupt() int_handler(void); 
void define_METATEDS(void);
void define_TCTEDS(void);
void Identify_NCAP_cmd(void);
void send_METATEDS(void);
void send_TCTEDS(uint8_t);
uint8_t get_char (void);
void send_error(void);
void send_success(int length);
void send_values (uint8_t);
uint8_t main_buffer_idx = 0;   // indice do proximo array
uint8_t main_buffer[6];        // array que vai receber primeiros 6 bits da NCAP
uint8_t aux_buffer_idx= 0;   // indice do proximo array
uint8_t aux_buffer[256];     // array que recebe offset e valores da NCAP

//create the structs for the Meta TEDS and for the TransducerChannels TEDS for 3 channels
struct METATEDS_TEMPLATE METATED;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS1;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS2;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS3;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS4;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS5;
struct TRANSDUCERCHANNEL_TEDS_TEMPLATE TCTEDS6;


void main(void) {
    config();           // Configure the PIC
    define_METATEDS();  // Define the Meta TEDS
    define_TCTEDS();    // Define the TransducerChannels TEDS for 3 channels

    
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
    // This function will be called everytime there is an interrupt from the EUSART
    
    //// Start to store the values ////

    // Minimum amount of bytes
    PIR3bits.TX1IF = 0; // TODO: Try to wait for this value to be zero in the get_char function
    while (main_buffer_idx < 6){
        main_buffer[main_buffer_idx] = get_char();   
        main_buffer_idx += 1;
    }
    
    while (aux_buffer_idx < main_buffer[5] ){
        aux_buffer[aux_buffer_idx] = get_char();   // preenchemos o array value dependendo do tamanho (LSB) desse valor
        aux_buffer_idx += 1;
    }

    // Reset the indexes
    aux_buffer_idx = 0;
    main_buffer_idx = 0;

    
    // Check if we have a valid command
    // (Slide 30)
    // 2 hex - Destination Transducer MSB
    // 2 hex - Destination Transducer LSB
    // 2 hex - CMD Class (Slide 31 - 1 e 3)
    // 2 hex - CMD function (Slide 32)
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

    /////////////////////////////////////////////////////
    /////////////////////// START ///////////////////////
    /////////////////////////////////////////////////////

    // Check if we were asked information about the Meta TEDS or Transducer TEDS
    // This will check if we have the correct CMD class, CMD function, length is 2 and offset is ignored
    if( (main_buffer[2] == 1) && (main_buffer[3] == 2) &&  (main_buffer[5] == 2)){ 
        // If  command-dependant octets are 1, we have a Meta TEDS request
        if (aux_buffer[0] == 1){
            send_METATEDS(); 
            return;
        }
        // If  command-dependant octets are 3, we have a Transducer TEDS request
        if (aux_buffer[0] == 3){
            send_TCTEDS(main_buffer[1]);
            return;
        }
    }
    

    if ( (main_buffer[2] == 1) && (main_buffer[3] == 2) &&  (main_buffer[5] == 2) && (aux_buffer[0] == 3)){ // TESTA SE NCAP PEDE UMA TED
        //common command -> 1; read Teds seg ; len -> 2; TC TEDS -> 3 ;
        
        send_TCTEDS(main_buffer[1]); // ENVIA A TCTED DO CANAL PEDIDO  
        return;
    }
    
    if (main_buffer[2] == 3){             // verifica que � um transducer 
        
        if (main_buffer[3] == 1){         // ler do transdutor do canal main_buffer[1]
            send_values(main_buffer[1]); 
            return;
        }
        
            if ((main_buffer[3] == 2) && (main_buffer[1] == 4)){ //escrever no transdutor do canal 3 (unico permitido psrs escrita) 
            
            LATAbits.LATA4 = aux_buffer[1];
            send_success(0);    // enviar a NCAP mensagem de sucesso
            return;
        }
        if ((main_buffer[3] == 2) && (main_buffer[1] == 5)){ //escrever no transdutor do canal 3 (unico permitido psrs escrita) 
            LATAbits.LATA5 = aux_buffer[1];
            send_success(0);    // enviar a NCAP mensagem de sucesso
            return;
        }
        if ((main_buffer[3] == 2) && (main_buffer[1] == 6)){ //escrever no transdutor do canal 3 (unico permitido psrs escrita) 
            LATAbits.LATA6 = aux_buffer[1];
            send_success(0);    // enviar a NCAP mensagem de sucesso
            return;
        }  
    }

    // Error if we get here
    send_error();
    
    return;
    
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////// SEND FUNC //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Send success msg
void send_success(int length){
    putch(1);
    putch(0);
    putch(length);
    return;
}

// Send error msg
void send_error(void){
    for(int i = 0; i<3;i++){
        putch(0);
    }

    // Reset buffers indexes
    main_buffer_idx = 0;
    aux_buffer_idx = 0;
    
    return;
}
void send_values(uint8_t channel){
    // We will have 6 channels
    // 3 axis for the accelerometer and 3 for the potentiometer
    // Success/send_error Flag 01
    // Length (MSB) 00
    // Length (LSB) 01 (I have requested the sensor to read 1 value, which needs 1 byte)
    // Data XX

    // ADPCH
    // 001010 RB2/ ANB2 // Z
    // 001001 RB1/ ANB1 // Y
    // 001000 RB0/ANB0 // X

    // Read X axis
    if (channel == 1) {
        ADPCH = 0b00001000;
        
    // Read Y axis
    } else if (channel == 2) {
        ADPCH = 0b00001001;
        
    // Read Z axis
    } else if (channel == 3) {
        ADPCH = 0b00001010;
        
    } else {
        // send error message
        send_error();
        return;
    }
    
    // Success Flag
    send_success();
    putch(1); // Length

    // Start new conversion
    ADCON0bits.ADGO = 1;

    // Ensure ADC conversion is over 
    while (PIR1bits.ADIF == 0){
        ;
    }

    // Now send data
    putch(ADRESH);

    // Reset ADC flag
    PIR1bits.ADIF = 0;
    
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
    // We have 6 channels
    // 3 axis for the accelerometer and 3 for the 3 LEDs
    
    if (channel == 1) { // Channel 1 - Accelerometer X
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
    } else if (channel == 2) { // Channel 2 - Accelerometer Y
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
    } else if (channel == 3) { // Channel 3 - Accelerometer Z
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
    } else if (channel == 4) { // Channel 4 - LED 1
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
    } else if (channel == 5) { // Channel 5 - LED 2
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
    } else if (channel == 6) { // Channel 6 - LED 3
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
        }
    } else {
        // Send error
        send_error();
        }
    return;
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////// DEFINE FUNC ////////////////////////////////
//////////////////////////////////////////////////////////////////////////


void define_METATEDS(void) {
    uint8_t array1[] = {3, 4, 0, 1, 0, 1};
    memcpy(METATED.TEDSID, array1, 6);
    uint8_t array2[] = {4, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    memcpy(METATED.UUID, array2, 12);
    uint8_t array3[] = {13, 2, 0, 6};   // We have 6 channels. 3 sensors and 3 actuators
    memcpy(METATED.MAXCHAN, array3, 4);

    return;
}

void define_TCTEDS(void) {
    ///////////////////////////////////////////////////////////////////////////////////
    //////////////////////////// Configuring Accelerometer ////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////

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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////// CONFIGURING LEDS ////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    
    //TCTEDS4 - TEDS DO TRANSDUCER CHANNEL 4 - led
    
    uint8_t array14[] = {3, 4, 0, 3, 0, 1};
    memcpy(TCTEDS4.TEDSID, array14, 6);
    uint8_t array24[] = {11, 1, 1};
    memcpy(TCTEDS4.CHANNEL_TYPE, array24, 3);
    uint8_t array34[] = {12, 10, 0, 128, 128, 132, 130, 122, 126, 128, 128, 128};
    memcpy(TCTEDS4.UNITS, array34, 12);
    uint8_t array44[] = {13, 1, 0};
    memcpy(TCTEDS4.LOW_RANGE_LIMIT, array44, 3);
    uint8_t array54[] = {14, 1, 255};
    memcpy(TCTEDS4.HIGH_RANGE_LIMIT, array54, 3);
    uint8_t array64[] = {40, 1, 0};
    memcpy(TCTEDS4.DATA_MODEL, array64, 3);
    uint8_t array74[] = {41, 1, 1};
    memcpy(TCTEDS4.DATA_MODEL_LENGTH, array74, 3);
    uint8_t array84[] = {42, 1, 8};
    memcpy(TCTEDS4.MODEL_SIG_BITS, array84, 3);
    
    
    //TCTEDS5 - TEDS DO TRANSDUCER CHANNEL 5 - led
    memccpy(TCTEDS5.TEDSID, TCTEDS4.TEDSID, 6);
    memccpy(TCTEDS5.CHANNEL_TYPE, TCTEDS4.CHANNEL_TYPE, 3);
    memccpy(TCTEDS5.UNITS, TCTEDS4.UNITS, 12);
    memccpy(TCTEDS5.LOW_RANGE_LIMIT, TCTEDS4.LOW_RANGE_LIMIT, 3);
    memccpy(TCTEDS5.HIGH_RANGE_LIMIT, TCTEDS4.HIGH_RANGE_LIMIT, 3);
    memccpy(TCTEDS5.DATA_MODEL, TCTEDS4.DATA_MODEL, 3);
    memccpy(TCTEDS5.DATA_MODEL_LENGTH, TCTEDS4.DATA_MODEL_LENGTH, 3);
    memccpy(TCTEDS5.MODEL_SIG_BITS, TCTEDS4.MODEL_SIG_BITS, 3);

    
    
    //TCTEDS6 - TEDS DO TRANSDUCER CHANNEL 6 - led
    memccpy(TCTEDS6.TEDSID, TCTEDS4.TEDSID, 6);
    memccpy(TCTEDS6.CHANNEL_TYPE, TCTEDS4.CHANNEL_TYPE, 3);
    memccpy(TCTEDS6.UNITS, TCTEDS4.UNITS, 12);
    memccpy(TCTEDS6.LOW_RANGE_LIMIT, TCTEDS4.LOW_RANGE_LIMIT, 3);
    memccpy(TCTEDS6.HIGH_RANGE_LIMIT, TCTEDS4.HIGH_RANGE_LIMIT, 3);
    memccpy(TCTEDS6.DATA_MODEL, TCTEDS4.DATA_MODEL, 3);
    memccpy(TCTEDS6.DATA_MODEL_LENGTH, TCTEDS4.DATA_MODEL_LENGTH, 3);
    memccpy(TCTEDS6.MODEL_SIG_BITS, TCTEDS4.MODEL_SIG_BITS, 3);
    
    
    return;
}