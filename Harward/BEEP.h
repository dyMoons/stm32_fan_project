#ifndef _BEEP_H
#define _BEEP_H

#define BEEP_PIN GPIO_Pin_13

#define BEEP_PIN_13(x) GPIO_WriteBit(GPIOB,BEEP_PIN,(BitAction)(x))

void BEEP_Init(void); 

#endif
