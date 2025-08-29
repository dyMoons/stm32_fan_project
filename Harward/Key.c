#include "stm32f10x.h"                  // Device header
#include "Key.h"  
#include "Delay.h"
uint8_t Key_Mode = 0;				//模式按键标志位---PB0
uint8_t Mode = 0;					//模式切换
uint8_t Key_Start_Flag = 0;			//开机按键标志位
uint8_t Key_Down_Flag = 0;			//关机按键标志位
uint8_t Key_Speed_Flag = 0;			//风扇转速标志位
#define DEBOUNCE_TIME 20   // 消抖时间 ms

uint32_t lastKeyTime = 0;

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_1 | GPIO_Pin_11);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_8 | GPIO_Pin_10);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource8); 					//PA8外部中断初始化
	
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);					//PB1外部中断初始化
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource10);					//PA10外部中断初始化
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line10;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource11);				//PB11外部中断初始化
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStucture;
	NVIC_InitStucture.NVIC_IRQChannel = EXTI9_5_IRQn;							//PA8_NVIC初始化
	NVIC_InitStucture.NVIC_IRQChannelCmd = ENABLE;							
	NVIC_InitStucture.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStucture.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStucture);
	
	NVIC_InitStucture.NVIC_IRQChannel = EXTI1_IRQn;							//PB1_NVIC初始化
	NVIC_InitStucture.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStucture.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStucture.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStucture);
	
	NVIC_InitStucture.NVIC_IRQChannel = EXTI15_10_IRQn;						//PA10_NVIC和PB11_NVIC初始化
	NVIC_InitStucture.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStucture.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStucture.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStucture);
}

void Key_Scan(void)                 //按键扫描
{	
	if(KEY_PIN8_Status == 0)			//按键PA8触发    	模式切换
	{
		Delay_ms(20);
		while(KEY_PIN8_Status == 0);
		Delay_ms(20);
		Key_Mode = 1;
	}
	if(KEY_PIN1_Status  == 0)		//按键PB1触发     开机键
	{
		Delay_ms(20);
		while(KEY_PIN1_Status == 0);
		Delay_ms(20);
		Key_Start_Flag = 1;
	}
	if(KEY_PIN10_Status  == 0)		//按键PA10触发  	风扇转速挡位切换
	{
		Delay_ms(20);
		while(KEY_PIN10_Status == 0);
		Delay_ms(20);
		Key_Speed_Flag = 1;
	}
	if(KEY_PIN11_Status  == 0)		//按键PB11触发   关机键
	{
		Delay_ms(20);
		while(KEY_PIN11_Status == 0);
		Delay_ms(20);
		Key_Down_Flag = 1;
	}
}

void EXTI9_5_IRQHandler(void)                  //PA8中断
{
	if(EXTI_GetITStatus(EXTI_Line8) == SET)
	{
		
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}

void EXTI1_IRQHandler(void)                    //PB1中断
{
	if(EXTI_GetITStatus(EXTI_Line1) == SET)
	{
		
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

void EXTI15_10_IRQHandler(void)					//PA10和PB11中断
{
	if(EXTI_GetITStatus(EXTI_Line10) == SET)
	{
		
		EXTI_ClearITPendingBit(EXTI_Line10);
	}
	else if(EXTI_GetITStatus(EXTI_Line11) == SET)
	{
		
		EXTI_ClearITPendingBit(EXTI_Line11);
	}
	
}
