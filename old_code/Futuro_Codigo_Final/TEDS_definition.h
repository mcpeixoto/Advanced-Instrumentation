 
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  


typedef struct METATEDS_TEMPLATE {
    uint8_t TEDSID[6];
    uint8_t UUID[12];
    uint8_t MAXCHAN[4];
} METATEDS_TEMPLATE;


typedef struct TRANSDUCERCHANNEL_TEDS_TEMPLATE {
    uint8_t TEDSID[6];
    uint8_t CHANNEL_TYPE[3];
    uint8_t UNITS[12];
    uint8_t LOW_RANGE_LIMIT[3];
    uint8_t HIGH_RANGE_LIMIT[3];
    uint8_t DATA_MODEL[3];
    uint8_t DATA_MODEL_LENGTH[3];
    uint8_t MODEL_SIG_BITS[3];
} TRANSDUCERCHANNEL_TEDS_TEMPLATE;



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

