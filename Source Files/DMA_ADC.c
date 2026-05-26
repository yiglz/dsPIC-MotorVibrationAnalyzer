#include "DMA_ADC.h"

/* * Allocate two 512-word buffers in DMA RAM (Dual-Port SRAM).
 */
unsigned int BufferA[512] __attribute__((space(dma), aligned(1024)));
unsigned int BufferB[512] __attribute__((space(dma), aligned(1024))); 

void DMA_ADC_Init(void)
{
    /* DMA0CON Configuration:
     * Bit 15 (CHEN)  : 0 = Disable channel for now
     * Bit 14 (SIZE)  : 0 = Word (16-bit) data transfer
     * Bit 13 (DIR)   : 0 = Read from Peripheral address, write to DPSRAM address
     * Bit 12 (HALF)  : 0 = Initiate interrupt when all data has been moved
     * Bit 11 (NULLW) : 0 = Normal operation
     * Bit 5-4 (AMODE): 00 = Register Indirect with Post-Increment mode
     * Bit 1-0 (MODE) : 10 = Continuous, Ping-Pong mode enabled
     */
    DMA0CON |= (1<<1);
    DMA0CON &= ~((1<<14) | (1<<13) | (1<<12) | (1<<11) | (1<<5) | (1<<4) | (1<<0));
 
    /* DMA0REQ Configuration:
     * Bit 15 (FORCE) : 0 = Automatic transfer initiation
     * Bit 6-0 (IRQSEL): 0001101 (13) = ADC1 Convert Done Interrupt
     */
    DMA0REQ |= ((1<<3) | (1<<2) | (1<<0));
    DMA0REQ &= ~((1<<15) | (1<<6) | (1<<5) | (1<<4) | (1<<1));
 
    //DPSRAM buffer offsets
    DMA0STA = __builtin_dmaoffset(BufferA);
    DMA0STB = __builtin_dmaoffset(BufferB);
    
    // Assign ADC1 Buffer address
    DMA0PAD = (volatile unsigned int)&ADC1BUF0;         
    
    // Number of DMA requests = DMA0CNT + 1 = 511 + 1 = 512 transfers
    DMA0CNT = 511;
}

void DMA_ADC_Start(void)
{
    IFS0bits.DMA0IF = 0; // Clear the DMA interrupt flag
    IEC0bits.DMA0IE = 1; // Enable DMA interrupt
    
    DMA0CON |= (1<<15);  // CHEN = 1: Enable the DMA Channel 0
}