#include "stm32f10x.h"                  // Device header
#include "Motor.h"  

void PWM_Init(void)                       //PWM初始化
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM3);                      //初始化内部时钟
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;                  //定时器时基单元初始化
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100-1;                      //设置ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36-1;                   //设置PSC      计时器溢出频率=72Mhz/(PSC+1)/(ARR+1)
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;                              //输出比较单元初始化
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;                 //初始化CCR   PWM频率=72Mhz/(PSC+1)/(ARR+1)  PWM占空比= CCR/(ARR+1);
	TIM_OC1Init(TIM3,&TIM_OCInitStructure);
	
	TIM_Cmd(TIM3,ENABLE);
}

void Motor_Init(void)                   //初始化电机
{
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;                 //初始化电机正反转口
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	PWM_Init();
}


void PWM_SetCompare(uint16_t Compare)                              //设置输出比较单元的CCR
{
	TIM_SetCompare1(TIM3,Compare);
}

void Motor_SetSpeed(int8_t Speed)                                   //设置电机转速（包括正反转）
{
	if(Speed >= 0)									//电机正转速度
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_4);
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
		PWM_SetCompare(Speed);
	}
	else												//电机反转速度
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);
		GPIO_SetBits(GPIOA,GPIO_Pin_5);
		PWM_SetCompare(-Speed);
	}
}

void Motor_Stop(void)                                     //电机停止旋转
{
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	GPIO_SetBits(GPIOA,GPIO_Pin_5);
	Motor_SetSpeed(0);
}

