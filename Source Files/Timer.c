#include "Timer.h"

void TimerInit(void){
    // Clear Timer3 counte
    TMR3 = 0x00;    
    
    /* T3CON Configuration:
     * Bit 13 (TSIDL) : 1 = Discontinue module operation when device enters Idle mode
     */
    T3CON |= (1<<13);  
    
    /* * Bit 6 (TGATE)   : 0 = Gated time accumulation disabled
     * Bit 5-4 (TCKPS) : 00 = 1:1 Prescale value
     * Bit 1 (TCS)     : 0 = Internal clock (Fosc/2)
     */
    T3CON &= ~((1<<6) | (1<<5) | (1<<4) | (1<<1));
    
    /* * Period Register 3 (PR3)
     * Fcy = 40 MIPS and 1:1 Prescaler:
     * 10,000 cycles = 4 kHz
     */
    PR3 = 9999;
}

void TimerStart(void){
    // TON = 1: Starts 16-bit Timer3
    T3CON |= (1<<15);
}