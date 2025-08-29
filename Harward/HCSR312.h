#ifndef _HC_SR312_H
#define _HC_SR312_H

#define HC_SR312_PIN GPIO_Pin_14

#define HC_SR312_Status GPIO_ReadInputDataBit(GPIOB,HC_SR312_PIN)

void HC_SR312_Init(void); 


#endif
