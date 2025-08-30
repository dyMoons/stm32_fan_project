#include "stm32f10x.h" // Device header
#include <string.h>
#include "USART.h" 
#include "Key.h" 
#include "stdarg.h" 

extern uint8_t Key_Mode;
extern uint8_t Key_Start_Flag;
extern uint8_t Key_Down_Flag;
extern uint8_t Key_Speed_Flag;

char RX_Buffer[100];   // 接收缓冲区
uint8_t RX_Index = 0;  // 当前存储位置

void USART_BLUE_Init(uint32_t BaudRate)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = BaudRate;					//设置波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;			//是否使用硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//发送和接收都需要
	USART_InitStructure.USART_Parity = USART_Parity_No;				//不需要奇偶校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			//一位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //八位数据位
	USART_Init(USART2,&USART_InitStructure);
	
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART2,ENABLE);
}

void Send_String(char* String)						//发送字符串
{
	uint8_t i;
	for(i = 0; String[i] != '\0'; i++ )
	{
		USART_SendData(USART2,String[i]);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
	}
}

void BlueTooth_Printf(char* format, ...)
{
	char String[100];
	va_list arg;				//定义一个参数列表变量
	va_start(arg,format);			//从format位置开始接收参数表放到arg当中
	vsprintf(String,format,arg);		//打印位置是String,格式化字符串是format,参数表是arg
	va_end(arg);				//释放参数表
	Send_String(String);
}

void Parse_Command(char* cmd) 					//蓝牙命令解析
{
	BlueTooth_Printf("Got Command: [%s]\r\n", cmd);
    if(strncmp(cmd, "ON",2) == 0)
    {
        Key_Start_Flag = 1;   // 模拟开机键按下
        Key_Down_Flag = 0;
        BlueTooth_Printf("Fan Power ON\r\n");
    }
    else if(strncmp(cmd, "OFF",3) == 0)
    {
        Key_Down_Flag = 1;    // 模拟关机键按下
        Key_Start_Flag = 0;
        BlueTooth_Printf("Fan Power OFF\r\n");
    }
    else if(strncmp(cmd, "MODE", 4) == 0)
    {
        Key_Mode = 1; // 模拟切换模式按键按下，主循环里会处理 Mode++
        BlueTooth_Printf("Change Mode\r\n");
    }
    else if(strncmp(cmd, "SPEED", 5) == 0)
    {
        Key_Speed_Flag = 1; // 模拟风扇挡位按键按下，主循环里会处理 Speed++
        BlueTooth_Printf("Change Speed\r\n");
    }
    else
    {
        BlueTooth_Printf("Unknown Command: %s\r\n", cmd);			//未知命令
    }
}


void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
	{
		char ch = USART_ReceiveData(USART2);		//接收手机蓝牙发过来的命令

        if(ch == '\r' || ch == '\n')  // 一条命令结束
        {
            RX_Buffer[RX_Index] = '\0';
            RX_Index = 0;				//清0 准备下一条
            Parse_Command(RX_Buffer); // 解析命令
        }
        else
        {
			if(ch != '\r' && ch != '\n')  // 避免存换行
			{
				RX_Buffer[RX_Index++] = ch;				//将接收到的命令放入缓冲区
				if(RX_Index >= sizeof(RX_Buffer)) RX_Index = 0;		//如果接收到的命令过大则防止溢出
			}
        }
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
	}
}
