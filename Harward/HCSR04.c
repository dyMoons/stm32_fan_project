#include "HCSR04.h"
#include "Delay.h"

uint16_t Time;         // 保存测得的脉宽（单位 us）
uint16_t IC_Rise = 0;  // 上升沿捕获值
uint16_t IC_Fall = 0;  // 下降沿捕获值
uint8_t  Is_Captured = 0; // 标志位：1 = 已捕获完成

/**
 * @brief  HC-SR04 初始化
 */
void HCSR04_Init()
{
    // 开启 GPIO 和 TIM2 时钟（TIM2 是输入捕获常用定时器）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    // Trig = 推挽输出
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = Trig_Pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Trig_Port, &GPIO_InitStruct);

    // Echo = 浮空输入（输入捕获需要浮空）
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Pin = Echo_Pin;
    GPIO_Init( Echo_Port, &GPIO_InitStruct);

    // TIM2 配置
    TIM_InternalClockConfig(TIM2);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;  // 1us 计数
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // 输入捕获配置：通道 1
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICFilter = 0x00;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising; // 上升沿
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInit(TIM2, &TIM_ICInitStructure);

    // 允许捕获中断
    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

    // 使能定时器
    TIM_Cmd(TIM2, ENABLE);

    // NVIC 配置中断
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Trig 初始拉低
    GPIO_ResetBits(Trig_Port, Trig_Pin);
}

/**
 * @brief  HC-SR04 发射一次测距脉冲
 */
void HCSR04_Start()
{
    GPIO_SetBits(Trig_Port, Trig_Pin);  // 拉高 Trig
    Delay_us(20);                       // >10us 脉冲
    GPIO_ResetBits(Trig_Port, Trig_Pin);// 拉低 Trig

    Is_Captured = 0;   // 清标志，准备捕获
}

uint16_t HCSR04_GetValue(void)
{
    HCSR04_Start();

    uint32_t timeout = 60000; // 60ms 超时，HC-SR04 最大测 4m，大概 23ms
    while(Is_Captured == 0 && timeout--);

    if(Is_Captured == 0) 
        return 0xFFFF; // 超时，无回波

    if (IC_Fall >= IC_Rise)
    Time = IC_Fall - IC_Rise;
else
    Time = (0xFFFF - IC_Rise + IC_Fall + 1); // 处理溢出情况
    return (Time * 0.034) / 2;   // 转换为 cm
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        if (Is_Captured == 0)  // 未完成捕获
        {
            if (TIM2->CCER & TIM_CCER_CC1P) // 检测下降沿
            {
                IC_Fall = TIM_GetCapture1(TIM2); // 记录下降沿
                Is_Captured = 1;                // 标志捕获完成
                TIM_OC1PolarityConfig(TIM2, TIM_ICPolarity_Rising); // 下次捕获上升沿
            }
            else
            {
                IC_Rise = TIM_GetCapture1(TIM2); // 记录上升沿
                TIM_OC1PolarityConfig(TIM2, TIM_ICPolarity_Falling); // 下一次捕获下降沿
            }
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1); // 清中断标志
    }
}
