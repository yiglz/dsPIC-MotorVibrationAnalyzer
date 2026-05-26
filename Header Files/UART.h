#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif

void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char *str);
void UART_SendInt(int number);

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */