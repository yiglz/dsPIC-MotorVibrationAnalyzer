#include "_10SecTimer.h"

void Timer_10Sec_Init(void){
    // T4CON: Timer 4 Control Register
    T4CON &= ~(1<<15);

    T4CON |= ((1<<5) | (1<<4) | (1<<3));
    
    T4CON &= ~((1<<6) | (1<<1));
    
    TMR5 = 0x0000;
    TMR4 = 0x0000; 
    
    /* * TOTAL PERIOD = 1.562.500 (0x0017D784)
     Fcy (40 MIPS) 1:256 Prescaler, 10 seconds delay:
     * (40.000.000 / 256) * 10 = 1.562.500
     */
    PR5 = 0x0017; // Period Register MSW 
    PR4 = 0xD784; // Period Register LSW 
}

void Timer_10Sec_Start(void){
    /*
     * T5IF (Timer 5 Interrupt Flag)
     */
    IFS1bits.T5IF = 0; 
    
    T4CON |= (1<<15); 
}