#ifndef DMA_H
#define	DMA_H

#include <xc.h>
#include "ADC.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern unsigned int BufferA[512];
extern unsigned int BufferB[512];
    
void DMA_ADC_Init(void);

void DMA_ADC_Start(void);

#ifdef	__cplusplus
}
#endif

#endif	/* DMA_H */