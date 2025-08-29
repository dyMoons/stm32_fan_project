#include "stm32f10x.h"
#include "servo.h"

#define SERVO_MIN_ANGLE  0
#define SERVO_MAX_ANGLE  150
#define PWM_PERIOD_US    20000
#define ANGLE_STEP       1.0f
#define UPDATE_MS        33   // 10秒往返（近似）

/* 状态变量 */
static volatile float servo_angle = 0.0f;
static volatile int8_t direction = 1;
static volatile uint8_t servo_running = 0;    // 0=停止，1=运行

static volatile int8_t direction1 = 1;
static volatile uint8_t servo1_running = 0;   // 0=停止，1=运行
static volatile uint16_t ms_counter1 = 0;

/* 允许 main.c 安全读取的计数器（SysTick 内更新） */
static volatile uint16_t ms_counter = 0;
static volatile uint32_t systick_total = 0;

/* 舵机1角度（超声波绑定） */
static volatile float servo1_angle = 0.0f;

/* ================== TIM4 PWM 初始化 ================== */
void Servo_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Period = PWM_PERIOD_US - 1; 
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;       
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	
	//CH1(舵机2)
    TIM_OCInitStructure.TIM_Pulse = 1500; // 中位
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	// CH2 (舵机1)
    TIM_OCInitStructure.TIM_Pulse = 1500;
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);

    Servo_SetAngle(servo_angle);   // 初始化舵机2
    Servo_SetAngle1(servo1_angle); // 初始化舵机1
}

/* 设置 PWM 脉宽 (µs) - 舵机2 */
void PWM_SetCompare1(uint16_t pulse_width)
{
    TIM_SetCompare1(TIM4, pulse_width);
}

/* 设置 PWM 脉宽 (µs) - 舵机1 */
void PWM_SetCompare2(uint16_t pulse_width)
{
    TIM_SetCompare2(TIM4, pulse_width);
}

/* 舵机2：角度 -> 脉宽 */
void Servo_SetAngle(float angle)
{
    if(angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if(angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    servo_angle = angle; // 更新内部变量（便于状态查询）

    uint16_t pulse_width = (uint16_t)( (angle * (2500u - 500u)) / SERVO_MAX_ANGLE + 500u );
    PWM_SetCompare1(pulse_width);
}

/* 舵机1：角度 -> 脉宽 */
void Servo_SetAngle1(float angle)
{
    if(angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if(angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;
    servo1_angle = angle;
    uint16_t pulse_width = (uint16_t)( (angle * (2500u - 500u)) / SERVO_MAX_ANGLE + 500u );
    PWM_SetCompare2(pulse_width);
}

/* 初始化 SysTick，返回0成功，非0表示失败 */
int Servo_SysTick_Init(void)
{
    if(SysTick_Config(SystemCoreClock / 1000)) // 1ms中断
        return 1;
    NVIC_SetPriority(SysTick_IRQn, 0);        // 优先级0
    return 0;
}

/* 启动舵机2摆头（幂等）——只在由停止 --> 运行时重置计数器 */
void Servo_Start(void)
{
    if(!servo_running)   /* 如果当前未运行，执行一次初始化动作 */
    {
        ms_counter = 0;
        /* 根据当前位置选择方向，避免“卡边” */
        if(servo_angle <= (float)SERVO_MIN_ANGLE) direction = 1;
        else if(servo_angle >= (float)SERVO_MAX_ANGLE) direction = -1;
        else direction = 1;
        servo_running = 1;
    }
    /* 如果已经运行，则什么也不做（不重置 ms_counter） */
}

/* 启动舵机1摆头 */
void Servo1_Start(void)
{
    if(!servo1_running)
    {
        ms_counter1 = 0;
        if(servo1_angle <= (float)SERVO_MIN_ANGLE) direction1 = 1;
        else if(servo1_angle >= (float)SERVO_MAX_ANGLE) direction1 = -1;
        else direction1 = 1;
        servo1_running = 1;
    }
}


/* 停止舵机摆头（不改变当前角度） */
void Servo_Stop(void)
{
    servo_running = 0;
}

/* 停止舵机1摆头（保持当前位置） */
void Servo1_Stop(void)
{
    servo1_running = 0;
}

/* 外部可读函数 */
uint16_t Servo_GetMsCounter(void) { return ms_counter; }
uint8_t  Servo_IsRunning(void)   { return servo_running ? 1 : 0; }
uint8_t Servo1_IsRunning(void) { return servo1_running ? 1 : 0; }

/* SysTick 中断：每 1ms 触发一次 */
void SysTick_Handler(void)
{
    systick_total++;

    /* 舵机2自动摆头 */
    if(servo_running)
    {
        ms_counter++;
        if(ms_counter >= UPDATE_MS)
        {
            ms_counter = 0;

            float new_angle = servo_angle + (float)direction * ANGLE_STEP;
            if(new_angle >= (float)SERVO_MAX_ANGLE)
            {
                servo_angle = (float)SERVO_MAX_ANGLE;
                direction = -1;
            }
            else if(new_angle <= (float)SERVO_MIN_ANGLE)
            {
                servo_angle = (float)SERVO_MIN_ANGLE;
                direction = 1;
            }
            else
            {
                servo_angle = new_angle;
            }

            uint16_t pulse_width = (uint16_t)((servo_angle * (2500u - 500u)) / SERVO_MAX_ANGLE + 500u);
            TIM_SetCompare1(TIM4, pulse_width);  // 舵机2
        }
    }

    /* 舵机1自动摆头 */
    if(servo1_running)
    {
        ms_counter1++;
        if(ms_counter1 >= UPDATE_MS)
        {
            ms_counter1 = 0;

            float new_angle1 = servo1_angle + (float)direction1 * ANGLE_STEP;
            if(new_angle1 >= (float)SERVO_MAX_ANGLE)
            {
                servo1_angle = (float)SERVO_MAX_ANGLE;
                direction1 = -1;
            }
            else if(new_angle1 <= (float)SERVO_MIN_ANGLE)
            {
                servo1_angle = (float)SERVO_MIN_ANGLE;
                direction1 = 1;
            }
            else
            {
                servo1_angle = new_angle1;
            }

            uint16_t pulse_width1 = (uint16_t)((servo1_angle * (2500u - 500u)) / SERVO_MAX_ANGLE + 500u);
            TIM_SetCompare2(TIM4, pulse_width1); // 舵机1
        }
    }
}


uint32_t Servo_GetSysTickTotal(void)
{
    return systick_total;
}

float Servo_GetAngle(void) { return servo_angle; }
float Servo1_GetAngle(void) { return servo1_angle; }
