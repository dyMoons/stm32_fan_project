#include <stdint.h>

#ifndef _MOTOR_H
#define _MOTOR_H

void PWM_Init(void);
void Motor_Init(void); 
void PWM_SetCompare(uint16_t Compare);
void Motor_SetSpeed(int8_t Speed);
void Motor_Stop(void);

#endif
