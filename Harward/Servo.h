#ifndef __SERVO_H
#define __SERVO_H

#include <stdint.h>

void Servo_Init(void);
void PWM_SetCompare1(uint16_t pulse_width);
void PWM_SetCompare2(uint16_t pulse_width);
void Servo_SetAngle(float angle);
void Servo_SetAngle1(float angle);
int  Servo_SysTick_Init(void);   // 返回0表示成功
void Servo_Start(void);
void Servo1_Start(void);
void Servo_Stop(void);
void Servo1_Stop(void);
uint16_t Servo_GetMsCounter(void);
uint8_t  Servo_IsRunning(void);
uint8_t Servo1_IsRunning(void);
uint32_t Servo_GetSysTickTotal(void);
float Servo_GetAngle(void);
float Servo1_GetAngle(void);

#endif
