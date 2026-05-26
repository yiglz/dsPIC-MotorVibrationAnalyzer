/*
 * File:   main.c
 * Author: Dev
 * Description: Main application for Motor Vibration Analyzer using dual-channel 
 * ADC, DMA Ping-Pong buffering, and in-place FFT calculation.
 */

// FBS (Boot Segment Configuration)
#pragma config BWRP = WRPROTECT_OFF     // Boot Segment Write Protect (Boot Segment may be written)
#pragma config BSS = NO_FLASH           // Boot Segment Program Flash Code Protection (No Boot program Flash segment)
#pragma config RBS = NO_RAM             // Boot Segment RAM Protection (No Boot RAM)

// FSS (Secure Segment Configuration)
#pragma config SWRP = WRPROTECT_OFF     // Secure Segment Program Write Protect (Secure segment may be written)
#pragma config SSS = NO_FLASH           // Secure Segment Program Flash Code Protection (No Secure Segment)
#pragma config RSS = NO_RAM             // Secure Segment Data RAM Protection (No Secure RAM)

// FGS (General Segment Configuration)
#pragma config GWRP = OFF               // General Code Segment Write Protect (User program memory is not write-protected)
#pragma config GSS = OFF                // General Segment Code Protection (User program memory is not code-protected)

// FOSCSEL (Oscillator Selection)
#pragma config FNOSC = FRCPLL           // Fast RC Oscillator with PLL (FRCPLL)
#pragma config IESO = ON                // Internal External Switch Over Mode enabled

// FOSC (Oscillator Configuration)
#pragma config POSCMD = NONE            // Primary Oscillator Disabled (Crystal-less operation)
#pragma config OSCIOFNC = OFF           // OSC2 pin has clock out function
#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow Only One Re-configuration)
#pragma config FCKSM = CSECMD           // Clock Switching Enabled, Clock Monitor Disabled

// FWDT (Watchdog Timer Configuration)
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler (1:32,768)
#pragma config WDTPRE = PR128           // WDT Prescaler (1:128)
#pragma config WINDIS = OFF             // Watchdog Timer in Non-Window mode
#pragma config FWDTEN = OFF             // Watchdog timer explicitly disabled

// FPOR (Power-on Reset Configuration)
#pragma config FPWRT = PWR128           // POR Timer Value (128ms)
#pragma config ALTI2C = OFF             // I2C mapped to standard SDA1/SCL1 pins
#pragma config LPOL = ON                // PWM low side active-high
#pragma config HPOL = ON                // PWM high side active-high
#pragma config PWMPIN = ON              // PWM pins controlled by PORT register at Reset

// FICD (ICD Configuration)
#pragma config ICS = PGD1               // Communicate on PGC1/EMUC1 and PGD1/EMUD1
#pragma config JTAGEN = OFF             // JTAG is Disabled

#include <xc.h>
#include <dsp.h>
#include "DMA_ADC.h"
#include "TwiddleFactors.h"
#include "Timer.h"
#include "_10SecTimer.h"
#include "UART.h"


volatile unsigned long int XAccumulator[256] = {0};
volatile unsigned long int YAccumulator[256] = {0};
volatile unsigned int fftCount = 0;
volatile int ABufferFilterFlag = 0;
volatile int BBufferFilterFlag = 0;


fractcomplex XDataBuffer[256] __attribute__((space(ymemory), aligned(1024))) = {0};
fractcomplex YDataBuffer[256] __attribute__((space(ymemory), aligned(1024))) = {0};

volatile fractional XResultBuffer[256] = {0};
volatile fractional YResultBuffer[256] = {0};

void System_Pins_Init(void) {
    __builtin_write_OSCCONL(OSCCON & 0xBF); 
    RPOR7bits.RP14R = 3;  // Map UART1 TX to RP14 pin
    __builtin_write_OSCCONL(OSCCON | 0x40); // Lock Registers
}

void Button_Init(void){
    TRISB |= (1<<7);      // Set RB7 as input
    INTCON2 |= (1<<0);    // External Interrupt 0 on negative edge
    IFS0 &= ~(1<<0);      // Clear INT0 interrupt flag
    IEC0 |= (1<<0);       // Enable INT0 interrupt
}

// DMA
void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void){
    static char whichBuffer = 0;
    
    if(whichBuffer == 0){
        ABufferFilterFlag = 1;
    }
    else {
        BBufferFilterFlag = 1;
    }
    whichBuffer ^= 1;
    
    IFS0bits.DMA0IF = 0; // Clear DMA interrupt flag
}

void LED_Init(void){
    TRISC &= ~(1<<0); // Set RC0 as output (Status LED)
    LATC &= ~(1<<0);  // Turn off LED
}

// External Interrupt 0 ISR
void __attribute__((interrupt, no_auto_psv)) _INT0Interrupt(void){
    IFS0 &= ~(1<<0);  // Clear INT0 interrupt flag
}

void main(void) {
    // Oscillator Tuning for 40 MIPS
    PLLFBD = 41; 
    CLKDIV &= ~((1<<7) | (1<<6)); 
    CLKDIV &= ~((1<<4) | (1<<3) | (1<<2) | (1<<1) | (1<<0)); 
    
    // Initiate Clock Switch to FRCPLL
    __builtin_write_OSCCONH(0x01); 
    __builtin_write_OSCCONL(OSCCON | 0x01);
    
    // Wait for Clock and PLL to lock
    while ((OSCCON & ((1<<14) | (1<<13) | (1<<12))) != (1<<12)); 
    while (!(OSCCON & (1<<5)));
    
    //Initializations
    System_Pins_Init();
    ADC_AnalogPinSet();
    ADC_Init();
    DMA_ADC_Init();
    TimerInit();
    Timer_10Sec_Init();
    Button_Init();   
    UART_Init();
    LED_Init();
    
    while(1) { 
        int i, j, k;
        unsigned int avgX, avgY;

        //Enter low-power mode and wait for external trigger
        LATC &= ~(1<<0); 
        Sleep(); 
        LATC |= (1<<0);
        
        // 2. NEW MEASUREMENT
        fftCount = 0;
        for(k = 0; k <= 255; k++){
            XAccumulator[k] = 0;
            YAccumulator[k] = 0;
        }

        // Start Hardware Peripherals
        Timer_10Sec_Start();        
        ADC_Start();
        DMA_ADC_Start();
        TimerStart(); // Starts Timer3 for ADC triggering
 
        // Process data for the 10-second duration
        while (IFS1bits.T5IF != 1) {
            
            // Process Buffer A
            if (ABufferFilterFlag){
                for (i=0; i<=255; i++) {
                    XDataBuffer[i].real = BufferA[2*i];
                    XDataBuffer[i].imag = 0;
                    YDataBuffer[i].real = BufferA[2*i+1];
                    YDataBuffer[i].imag = 0;
                }
                ABufferFilterFlag = 0;
                
                //Complex FFT
                FFTComplexIP(8, &XDataBuffer[0], 
                            (fractcomplex*)__builtin_psvoffset(&twiddleFactors[0]),
                            __builtin_psvpage(&twiddleFactors[0]));
                FFTComplexIP(8, &YDataBuffer[0], 
                            (fractcomplex*)__builtin_psvoffset(&twiddleFactors[0]),
                            __builtin_psvpage(&twiddleFactors[0]));
                
                BitReverseComplex(8, &XDataBuffer[0]);
                BitReverseComplex(8, &YDataBuffer[0]);
                SquareMagnitudeCplx(256, &XDataBuffer[0], &XResultBuffer[0]);
                SquareMagnitudeCplx(256, &YDataBuffer[0], &YResultBuffer[0]);
                
                // Accumulate results for averaging
                for (k = 0; k <= 255; k++) {
                    XAccumulator[k] += XResultBuffer[k];
                    YAccumulator[k] += YResultBuffer[k];
                }
                fftCount++; 
            }
            
            // Process Buffer B
            if (BBufferFilterFlag){
                for (j=0; j<=255; j++) {
                    XDataBuffer[j].real = BufferB[2*j];
                    XDataBuffer[j].imag = 0;
                    YDataBuffer[j].real = BufferB[2*j+1];
                    YDataBuffer[j].imag = 0;
                }
                BBufferFilterFlag = 0;   
                
                FFTComplexIP(8, &XDataBuffer[0], 
                            (fractcomplex*)__builtin_psvoffset(&twiddleFactors[0]),
                            __builtin_psvpage(&twiddleFactors[0]));
                FFTComplexIP(8, &YDataBuffer[0], 
                            (fractcomplex*)__builtin_psvoffset(&twiddleFactors[0]),
                            __builtin_psvpage(&twiddleFactors[0]));
                
                BitReverseComplex(8, &XDataBuffer[0]);
                BitReverseComplex(8, &YDataBuffer[0]);
                SquareMagnitudeCplx(256, &XDataBuffer[0], &XResultBuffer[0]);
                SquareMagnitudeCplx(256, &YDataBuffer[0], &YResultBuffer[0]);
                        
                for (k = 0; k <= 255; k++) {
                    XAccumulator[k] += XResultBuffer[k];
                    YAccumulator[k] += YResultBuffer[k];
                }
                fftCount++; 
            }
        }
        
        //Stopping peripherals
        ADC_Stop();
        T3CON &= ~(1<<15);
        T4CON &= ~(1<<15);
        IFS1bits.T5IF = 0;
        
        /*STOP DMA
         */
        DMA0CON &= ~(1<<15);
        IEC0bits.DMA0IE = 0;
        IFS0bits.DMA0IF = 0;

        // Transmit FFT Data via UART
        UART_SendString("\r\n--- RESULTS ---\r\n");
        UART_SendString("Frequency_Index, X_Avg, Y_Avg \r\n");
        
        for(k = 0; k <= 255; k++){
            avgX = 0;
            avgY = 0;
            
            if (fftCount > 0) {
                avgX = (unsigned int)(XAccumulator[k] / fftCount);
                avgY = (unsigned int)(YAccumulator[k] / fftCount);
            }
            
            UART_SendInt(k);                 
            UART_SendString(", ");           
            
            // 4. Transmit the magnitudes
            UART_SendInt(avgX);  
            UART_SendString(", ");           
            
            UART_SendInt(avgY);  
            UART_SendString("\r\n");         
        }
        
        UART_SendString("--- PRESS THE BUTTON AGAIN FOR ANOTHER SESSION  ---\r\n");
    }
}