#ifndef _LED_H
#define _LED_H

#define LED_PIN2 GPIO_Pin_7

#define LED_PIN_2(x) GPIO_WriteBit(GPIOA,LED_PIN2,(BitAction)(x))

void LED_Init(void); 


#endif
