#ifndef __BSP_BEEP_H__
#define __BSP_BEEP_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* 宏定义 --------------------------------------------------------------------*/
#define BEEP_RCC_CLK_ENABLE()         __HAL_RCC_GPIOF_CLK_ENABLE()
#define BEEP_GPIO_PIN                 GPIO_PIN_15
#define BEEP_GPIO                     GPIOF

#define BEEP_ON                       HAL_GPIO_WritePin(BEEP_GPIO,BEEP_GPIO_PIN,GPIO_PIN_RESET)    // 输出高电平
#define BEEP_OFF                      HAL_GPIO_WritePin(BEEP_GPIO,BEEP_GPIO_PIN,GPIO_PIN_SET)  // 输出低电平
#define MOTOR_DIR_CW                            1  // 电机方向: 顺时针
#define MOTOR_DIR_CCW                         (-1) // 电机方向: 逆时针

/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/
void BEEP_GPIO_Init(void);

  
#endif  // __BSP_BEEP_H__

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
