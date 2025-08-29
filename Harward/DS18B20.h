#include <stdint.h>

#ifndef _DS18B20_H
#define _DS18B20_H

extern uint8_t temperature_flag;

void DS18B20_IO_OUT (void);                
void DS18B20_IO_IN (void); 
uint8_t DS18B20_ACK(void);
void Send_Bit(uint8_t Bit);
void Send_Byte(uint8_t Byte);;
uint8_t Receive_Bit(void);
uint8_t Receive_Byte(void);
uint16_t Get_Temp(void);

#define DS18B20_R   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12);                    //读取PB12口的电平

#define DS18B20_W(x)   GPIO_WriteBit(GPIOB,GPIO_Pin_12,(BitAction)(x));            //向PB12口写入电平

#define ERROR_CODE  1000

#endif
