#ifndef __BSP_BEEP_H__
#define __BSP_BEEP_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* �궨�� --------------------------------------------------------------------*/
#define BEEP_RCC_CLK_ENABLE()         __HAL_RCC_GPIOF_CLK_ENABLE()
#define BEEP_GPIO_PIN                 GPIO_PIN_15
#define BEEP_GPIO                     GPIOF

#define BEEP_ON                       HAL_GPIO_WritePin(BEEP_GPIO,BEEP_GPIO_PIN,GPIO_PIN_RESET)    // ����ߵ�ƽ
#define BEEP_OFF                      HAL_GPIO_WritePin(BEEP_GPIO,BEEP_GPIO_PIN,GPIO_PIN_SET)  // ����͵�ƽ
#define MOTOR_DIR_CW                            1  // �������: ˳ʱ��
#define MOTOR_DIR_CCW                         (-1) // �������: ��ʱ��

/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
void BEEP_GPIO_Init(void);

  
#endif  // __BSP_BEEP_H__

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
