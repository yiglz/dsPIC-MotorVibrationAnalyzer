#ifndef TIMER_H
#define	TIMER_H

#include <xc.h>

#ifdef	__cplusplus
extern "C" {
#endif

void TimerInit(void);
void TimerStart(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */