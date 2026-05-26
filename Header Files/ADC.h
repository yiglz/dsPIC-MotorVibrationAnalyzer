#ifndef ADC_H
#define	ADC_H

#include <xc.h>

#ifdef	__cplusplus
extern "C" {
#endif

void ADC_AnalogPinSet(void);

void ADC_Init(void);

void ADC_Start(void);

void ADC_Stop(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

