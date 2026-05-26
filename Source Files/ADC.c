#include "ADC.h"

void ADC_AnalogPinSet(void){
    // Set RA0 and RA1 pins as input
    TRISA |= ((1<<1) | (1<<0));
    
    // AD1CHS0: Channel 0 Input Select
    // CH0SA = 0 (AN0 selected), CH0NA = 0 (Vref- selected)
    AD1CHS0 &= ~((1<<7) | (1<<4) | (1<<3) | (1<<2) | (1<<1));
    AD1CHS0 |= (1<<0);
    
    // AD1CHS123: Channel 1,2,3 Input Select
    AD1CHS123 &= ~((1<<2) | (1<<1) | (1<<0));
    
    // AD1PCFGL: Port Configuration Register 
    // Set AN0 and AN1 pins to Analog mode
    AD1PCFGL &= ~((1<<1) | (1<<0));
}

void ADC_Init(void){
    /* AD1CON1 Configuration:
     * Bit 12 (AD12B) : 1 = 12-bit, 1-channel ADC operation
     * Bit 9-8 (FORM) : 11 = Signed Fractional
     * Bit 6 (SSRC)   : 1 = Timer / PWM interval ends sampling and starts conversion
     * Bit 3 (SIMSAM) : 1 = Simultaneous sampling enabled
     * Bit 2 (ASAM)   : 1 = Auto-sampling enabled
     */
    AD1CON1 |= ((1<<12) | (1<<9) | (1<<8) | (1<<6) | (1<<3) | (1<<2));
    AD1CON1 &= ~((1<<13) | (1<<10) | (1<<7) | (1<<5)); 

    /* AD1CON2 Configuration:
     * Bit 8 (SMPI) : Interrupt rate selection
     * Other bits are cleared to default values
     */
    AD1CON2 |= (1<<8); 
    AD1CON2 &= ~((1<<15) | (1<<14) | (1<<13) | (1<<10) | (1<<9) | (1<<5) | (1<<4) | (1<<3) | (1<<2) | (1<<1) | (1<<0));

    /* AD1CON3 Configuration:
     * Auto-Sample Time bits (SAMC) and ADC Conversion Clock (ADCS) selection
     */
    AD1CON3 |= ((1<<5) | (1<<2) | (1<<1) | (1<<0)); 
    AD1CON3 &= ~((1<<15) | (1<<12) | (1<<11) | (1<<10) | (1<<9) | (1<<8) | (1<<7) | (1<<6) | (1<<4) | (1<<3));

    /* AD1CSSL: Input Scan Select Register
     */
    AD1CSSL |= ((1<<1) | (1<<0));
}

void ADC_Start(void){
    AD1CON1 |= (1<<15); // ADON = 1: Power up and enable the ADC module
}

void ADC_Stop(void){
    AD1CON1 &= ~(1<<15); // ADON = 0: Turn off the ADC module
}