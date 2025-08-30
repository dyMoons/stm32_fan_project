#include <stdint.h>
#include <stdio.h>
#ifndef _USART_H
#define _USART_H

void USART_BLUE_Init(uint32_t BaudRate);
void Send_String(char* String);
void BlueTooth_Printf(char* format, ...);
void USART2_IRQHandler(void);
void Parse_Command(char* cmd);

#endif
