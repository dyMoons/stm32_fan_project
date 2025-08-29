#include "delay.h"

/**
  * @brief  简单空循环延时 (阻塞式)
  * @note   精度不高，但可用于 STM32F103
  */
void Delay_us(uint32_t us)
{
    uint32_t i;
    while(us--)
    {
        for(i = 0; i < 8; i++); // 按72MHz大约空循环1us，测试可微调
    }
}

void Delay_ms(uint32_t ms)
{
    while(ms--)
        Delay_us(1000);
}

void Delay_s(uint32_t s)
{
    while(s--)
        Delay_ms(1000);
}
