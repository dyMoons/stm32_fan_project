#include "stm32f10x.h"                  // Device header
#include "LED.h"					//LED灯
#include "Delay.h"
#include "BEEP.h"					//蜂鸣器
#include "OLED.h"					//OLED显示屏
#include "Motor.h"					//电机
#include "Servo.h"					//舵机
#include "Key.h"					//按键
#include "DS18B20.h"				//温度检测
#include "HCSR04.h"					//超声波测距
#include "USART.h"					//蓝牙模块

extern uint8_t Mode;			//模式按键---PB0
extern uint8_t Key_Mode;			//模式标志位
extern uint8_t Key_Start_Flag;			//开机按键标志位
extern uint8_t Key_Down_Flag;			//关机按键标志位
extern uint8_t Key_Speed_Flag;			//风扇转速标志位
extern uint8_t temperature_flag;		//温度标志位
uint8_t last_mode = 0;
uint8_t KeyNum;              //按键检测
uint8_t Speed = 0;			//风扇初始速度

int main(void)
{
	LED_Init();				 //LED灯初始化     PA7
 	BEEP_Init();             //蜂鸣器初始化     PB13    
	OLED_Init();			//OLED显示屏初始化   PB8,PB9    
	Motor_Init();			//电机初始化		(TIM3,PA6),PA4,PA5         
	Servo_Init();			//舵机初始化		(TIM4,PB6)  
	Servo_SysTick_Init();
	Key_Init();				//按键初始化		PB0,PB1,PB10,PB11     
	USART_BLUE_Init(9600);	//蓝牙初始化		PA2,PA3         		
	HCSR04_Init();			//超声波测距初始化   (TIM2,PA0),PA1       
							//DS18B20温度传感器     PB12      
	
 	uint16_t Distance;
	uint16_t temperature;
	Servo_SetAngle(0);
	Servo_SetAngle1(0);
	
	while(1)
	{
		Key_Scan();					//按键扫描
		
		if(Key_Start_Flag == 1 && Key_Down_Flag == 0)			//开机键按下   开启模式0，风扇开始转动，不摆头
		{
			temperature = Get_Temp();							//计算温度
			if(temperature/100 > 50 || temperature/100 <=0)				//温度异常
				{
					OLED_Clear();
					OLED_ShowString(1,1,"Temperature Error");
					Motor_Stop();			//电机停止转动
					Servo_Stop();		//舵机停止转动
					Servo1_Stop();
					Speed = 0;
				}
			else
			{
				LED_PIN_2(1);               		//电源指示灯亮
				OLED_ShowString(1,8,"Speed:");
				OLED_ShowNum(1,14,Speed,2);
			
				Distance = HCSR04_GetValue();			
				OLED_ShowString(1,1,"Mode:");
				OLED_ShowNum(1,6,Mode,1);
				OLED_ShowString(2,1,"Distance:");
				OLED_ShowNum(2,10,Distance,3);
				OLED_ShowString(3,1,"Temp: ");
				if(temperature_flag == 0)				//判断温度正负
				{
					OLED_ShowString(3,7,"-");
				}
				else
				{
					OLED_ShowString(3,7,"+");
				}
				OLED_ShowNum(3,8,temperature/100,3);		//计算温度值
				OLED_ShowString(3,11,".");
				OLED_ShowNum(3,12,temperature%100,2);		
				BlueTooth_Printf("/Mode:%d\r\n",Mode);				//将数值发送到蓝牙主机上
				int t_int = temperature / 100;					//传输温度的整数位
				int t_dec = temperature % 100;					//传输温度的小数位
				BlueTooth_Printf("/Temperature:%d.%02d\r\n", t_int, t_dec);
				BlueTooth_Printf("/Distance:%d\r\n",Distance);
				BlueTooth_Printf("/Speed:%d\r\n",Speed);
				
				if(Key_Mode)
				{
					Mode += 1;
					if(Mode == 3) Mode = 0;
					Key_Mode = 0; //清标志位
				}
				
				
				if(Key_Speed_Flag == 1)			//风扇挡位被按下
				{
					Speed += 25;              //25，50，75 三个挡位
					if(Speed == 100)
					{
						Speed = 0;
					}
					Key_Speed_Flag = 0;
					Motor_SetSpeed(Speed);
				}
				
				if(Mode != last_mode) 
				{
					// 模式变化才处理（只调用一次 Start/Stop）
					last_mode = Mode;
					switch(Mode) 
					{
						case 0:  
							Servo_Stop();
							Servo1_Stop();
							break;
						case 1:
							   // 如需风扇也开
						Servo_Start();           // 舵机2自动摆头只调用一次（幂等）
						Servo1_Stop();
							break;
						case 2:
							Servo_Stop();      // 舵机2默认保持
							Servo1_Stop();     // 舵机1默认保持
							break;
					}
				}
				
				if(Mode == 2) 
				{
					if(Distance >= 0 && Distance <= 20)   //检测到目标
					{
						Servo1_Stop();  
						Servo_SetAngle(Servo1_GetAngle());   // 舵机2对齐舵机1
					}
				else  // ❌ 没检测到目标
					{
						Servo1_Start();   // 舵机1自动摆头
						Servo_Stop();     // 舵机2保持不动
					}
					if(temperature/100 < 20)
					{
						Motor_Stop();			//电机停止转动
						Servo_Stop();		//舵机停止转动
						Servo1_Stop();
						Speed = 0;
					}
					else if(temperature/100 >= 20 && temperature/100 <= 25)
					{
						Speed = 25;
						Motor_SetSpeed(Speed);
					}
					else if(temperature/100 > 25 && temperature/100 <= 30)
					{
						Speed = 50;
						Motor_SetSpeed(Speed);
					}
					else if(temperature/100 > 30 && temperature/100 <= 50)
					{
						Speed = 75;
						Motor_SetSpeed(Speed);
					}
					
				}
			}		
				
		}
		else 				//关机键按下 | 开机和关机键都没按 
		{
			LED_PIN_2(0);				//电源指示灯灭
			Key_Mode = 0;
			Mode = 0;
			Key_Start_Flag = 0;				//开机标志位清0
			Key_Down_Flag = 0;				//关机标志位清0
			Motor_Stop();			//电机停止转动
			Servo_Stop();		//舵机停止转动
			Servo1_Stop();
			Speed = 0;
			OLED_Clear();   		//OLED屏停止工作
			
		}
	}

	
}
