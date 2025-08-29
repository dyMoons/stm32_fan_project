#ifndef __HCSR04_H
#define __HCSR04_H

#include "stm32f10x.h"

// HC-SR04 Trig / Echo 引脚宏定义
#define Trig_Port GPIOA       // 你实际连接的端口
#define Trig_Pin  GPIO_Pin_1  // 你实际连接的引脚
#define Echo_Port GPIOA
#define Echo_Pin  GPIO_Pin_0

extern uint16_t Time;       // 测得的脉宽（us）

// 初始化 HC-SR04
void HCSR04_Init(void);

// 发射一次测距脉冲
void HCSR04_Start(void);

// 获取距离值（cm）
uint16_t HCSR04_GetValue(void);

// TIM2 输入捕获中断服务函数
void TIM2_IRQHandler(void);

#endif
