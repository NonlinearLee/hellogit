#ifndef __GENERAL_TIM_H__
#define __GENERAL_TIM_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* 类型定义 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
/********************通用定时器TIM参数定义，TIM2~TIM5************/

#define GENERAL_TIMx                     TIM5
#define GENERAL_TIM_RCC_CLK_ENABLE()     __HAL_RCC_TIM5_CLK_ENABLE()
#define GENERAL_TIM_RCC_CLK_DISABLE()    __HAL_RCC_TIM5_CLK_DISABLE()
#define GENERAL_TIM_IRQ                  TIM5_IRQn
#define GENERAL_TIM_INT_FUN              TIM5_IRQHandler

#define STEPMOTOR_TIM_CHANNEL_1          TIM_CHANNEL_1
#define STEPMOTOR_TIM_CHANNEL_2          TIM_CHANNEL_2
// 定义定时器预分频，定时器实际时钟频率为：84MHz/（GENERAL_TIM_PRESCALER+1）
#define GENERAL_TIM_PRESCALER           83  // 实际时钟频率为：1MHz
// 定义定时器周期，当定时器开始计数到BASIC_TIMx_PERIOD值是更新定时器并生成对应事件和中断
#define GENERAL_TIM_PERIOD              0xFFFF 

/* 扩展变量 ------------------------------------------------------------------*/
extern TIM_HandleTypeDef htimx;

/* 函数声明 ------------------------------------------------------------------*/

void GENERAL_TIMx_Init(void);

#endif	/* __GENERAL_TIM_H__ */
/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/

