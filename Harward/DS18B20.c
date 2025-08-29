#include "stm32f10x.h"
#include "Delay.h"
#include "DS18B20.h"

uint8_t temperature_flag = 1;     //0表示负温度，1表示正温度

void DS18B20_IO_OUT(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void DS18B20_IO_IN(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t DS18B20_ACK(void)
{
	uint8_t ack;
    DS18B20_IO_OUT();
    DS18B20_W(0);			//拉低总线
    Delay_us(480);          // 复位脉冲 ≥480us
    DS18B20_W(1);
    DS18B20_IO_IN();		//切换为输入模式
    Delay_us(70);           // 从机自动等待时间15~60us后把总线拉低保持60~240us的时间，之后从机自动释放总线

    ack = DS18B20_R; // 0 表示应答
    Delay_us(410);          // 等待剩余时隙时间，确保整个时序 ≥ 480us
    return ack;
}

void Send_Bit(uint8_t Bit)					//主机向从机发送数据位   （从机会在15~60us之间读取数据）
{
	DS18B20_IO_OUT(); 					//先将PB12端口置为推挽输出
	if(Bit == 1)								//主机发送1
	{
		Delay_us(2);							//在两个连续的数据之间增加间隔时间防止出现竞争与冒险
		DS18B20_W(0);							
		Delay_us(10);
		DS18B20_W(1);
		Delay_us(50);							//防止释放总线没多久被另一个Send_Bit拉低总线刚好被从机读取
	}
	else if(Bit == 0)							//主机发送0
	{
		Delay_us(2);							 //在两个连续的数据之间增加间隔时间防止出现竞争与冒险
		DS18B20_W(0);
		Delay_us(100);							//主机拉低电平后需要保持低电平60~120us
		DS18B20_W(1);
	}
}

void Send_Byte(uint8_t Byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        Send_Bit((Byte >> i) & 0x01); // 从低位到高位
    }
}

uint8_t Receive_Bit(void)					//接收位数据
{
	uint8_t Data;
	DS18B20_IO_OUT();
	DS18B20_W(0);
	Delay_us(2);
	DS18B20_W(1);
	Delay_us(12);
	DS18B20_IO_IN();
	Data = DS18B20_R;						//从机会在电平拉低15us后发送数据，主机则在此时读取数据
	Delay_us(50);							//读取1位数据的时隙最少60us并且两次读取之前最少间隔1us时隙
	return Data;
}

uint8_t Receive_Byte(void)					//接收一个字节
{
	uint8_t i , Data = 0x00;
	for(i = 0; i < 8; i++)
	{
		if(Receive_Bit())					//如果接收到的数据位1
		{
			Data |= (0x01 << i);
		}
		
	}
	return Data;
}

uint16_t Get_Temp(void)
{
    uint8_t LSB, MSB;
    uint16_t temperature;
	if (DS18B20_ACK() == 0)						//复位检测是否存在DS18B20（同时唤醒所有设备，准备通信）
	{
		Send_Byte(0xcc);						//跳过只读存储器命令（跳过逐个选定设备，直接进行广播）
		Send_Byte(0x44);						//开始转换温度命令
		Delay_ms(750);     						// 等待转换完成
	}
	else
	{
		return ERROR_CODE;						//如果没有检测到传感器这返回错误警告
	}
	if (DS18B20_ACK() == 0)						//重新复位防止之前通信干扰
	{
		DS18B20_ACK();
		Send_Byte(0xCC);
		Send_Byte(0xBE);   						// 读取暂存寄存器

		LSB = Receive_Byte();
		MSB = Receive_Byte();

		temperature = (MSB << 8) | LSB;			//温度范围为-55°C ~ +125°C    
	
		if((temperature & 0xF800) == 0xF800) 	// 判断是否为负温度，若高五位全为1，则代表检测到负温度
		{
			temperature = ~temperature + 1;		//寄存器当中的负数以补码形式存在，所以要得到负数的绝对值要将该值取反+1
			temperature_flag = 0;				//temperature_flag = 0,置温度标志位为0，表示负温度
		}
		temperature = temperature * 6.25 ;     //0.0625*100   温度=寄存器值×0.0625
		return temperature;
	}
	else
	{
		return ERROR_CODE;					//如果没有检测到传感器这返回错误警告
	}

}
