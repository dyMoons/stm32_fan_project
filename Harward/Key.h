#include <stdint.h>

#ifndef _KEY_H
#define _KEY_H

#define KEY_PIN8_Status GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)   //按键PA8触发    	模式切换
#define KEY_PIN1_Status GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)		//按键PB1触发     开机键
#define KEY_PIN10_Status GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10)	//按键PA10触发  	风扇转速挡位切换
#define KEY_PIN11_Status GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)	//按键PB11触发   关机键

void Key_Init(void); 
void Key_Scan(void);


#endif
