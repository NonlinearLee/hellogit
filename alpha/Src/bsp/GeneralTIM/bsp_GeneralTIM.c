/**
  ******************************************************************************
  * 文件名程: bsp_BasicTIM.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2017-03-30
  * 功    能: 基本定时器TIM6 & TIM7底层驱动程序
  ******************************************************************************
  * 说明：
  * 本例程配套硬石stm32开发板YS-F4Pro使用。
  * 
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */
/* 包含头文件 ----------------------------------------------------------------*/
#include "GeneralTIM/bsp_GeneralTIM.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
TIM_HandleTypeDef htimx;

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

/**
  * 函数功能: 基本定时器初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void GENERAL_TIMx_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_OC_InitTypeDef sOCConfig;

  htimx.Instance = GENERAL_TIMx;
  htimx.Init.Prescaler = GENERAL_TIM_PRESCALER;
  htimx.Init.CounterMode = TIM_COUNTERMODE_UP;
  htimx.Init.Period = GENERAL_TIM_PERIOD;
  htimx.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_OnePulse_Init(&htimx,TIM_OPMODE_SINGLE);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htimx, &sClockSourceConfig);

  /* 定时器比较输出配置 */
  sOCConfig.OCMode = TIM_OCMODE_TOGGLE;                // 比较输出模式：反转输出
  sOCConfig.Pulse = 0xFFFF;                      // 脉冲数
  sOCConfig.OCPolarity = TIM_OCPOLARITY_HIGH;          // 输出极性
  sOCConfig.OCNPolarity = TIM_OCNPOLARITY_LOW;         // 互补通道输出极性
  sOCConfig.OCFastMode = TIM_OCFAST_DISABLE;           // 快速模式
  sOCConfig.OCIdleState = TIM_OCIDLESTATE_RESET;       // 空闲电平
  sOCConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;     // 互补通道空闲电平
  HAL_TIM_OC_ConfigChannel(&htimx, &sOCConfig, STEPMOTOR_TIM_CHANNEL_1);
  HAL_TIM_OC_ConfigChannel(&htimx, &sOCConfig, STEPMOTOR_TIM_CHANNEL_2);
  
}

/**
  * 函数功能: 基本定时器硬件初始化配置
  * 输入参数: htim_base：基本定时器句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
void HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==GENERAL_TIMx)
  {
    /* 基本定时器外设时钟使能 */
    GENERAL_TIM_RCC_CLK_ENABLE();
    /* 外设中断配置 */
    HAL_NVIC_SetPriority(GENERAL_TIM_IRQ, 0, 1);
    HAL_NVIC_EnableIRQ(GENERAL_TIM_IRQ);
  }
}

/**
  * 函数功能: 基本定时器硬件反初始化配置
  * 输入参数: htim_base：基本定时器句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==GENERAL_TIMx)
  {
    /* 基本定时器外设时钟禁用 */
    GENERAL_TIM_RCC_CLK_DISABLE();
    /* 关闭外设中断 */
    HAL_NVIC_DisableIRQ(GENERAL_TIM_IRQ);
  }
} 

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
