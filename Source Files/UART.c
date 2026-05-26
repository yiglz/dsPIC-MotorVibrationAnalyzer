#include "UART.h"
#include <xc.h>
#include <stdio.h>

void UART_Init(void){
    // UARTEN = 0: Disable UART during configuration
    U1MODE &= ~(1<<15); 
    
    // BRGH = 1: High-Speed mode enabled
    U1MODE |= (1<<3);   
    
    // PDSEL<1:0> = 00 (8-bit data, no parity), STSEL = 0 (1 Stop bit)
    U1MODE &= ~((1<<2) | (1<<1) | (1<<0)); 
    
    /* --- CRYSTAL-LESS SYSTEM CONFIGURATION ---
     * To compensate for internal oscillator (FRC) drift and ensure
     * 9600 Baud communication at 40 MIPS, High-Speed mode is used.
     * U1BRG = (Fcy / (4 * Baud Rate)) - 1
     * U1BRG = (40,000,000 / (4 * 9600)) - 1 = 1041
     */
    U1BRG = 1041; 
    
    // UARTEN = 1: Enable the UART module
    U1MODE |= (1<<15);
    
    // UTXEN = 1: Enable Transmit
    U1STA |= (1<<10);
}

void UART_SendChar(char c){
    /* UTXBF (Transmit Buffer Full Status bit)
     * Wait while the transmit buffer is full to prevent data loss.
     */
    while(U1STA & (1<<9)); 
    
    U1TXREG = c; 
}

void UART_SendString(const char *str){
    while(*str){
        UART_SendChar(*str++);
    }
}

void UART_SendInt(int number){
    char buffer[15];
    sprintf(buffer, "%d", number); 
    UART_SendString(buffer);
}