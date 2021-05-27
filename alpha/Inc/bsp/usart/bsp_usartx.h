#ifndef __BSP_USARTX_H__
#define __BSP_USARTX_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* ���Ͷ��� ------------------------------------------------------------------*/
/* ���ݸ�ʽ��ת�� */
typedef union {
  char Ch[4];
  float Float;
  uint32_t Int;
}Format_UnionTypedef;

typedef struct {
  __IO uint8_t  Code ;  	
  __IO Format_UnionTypedef data[3];//����֡��3������
}MSG_TypeDef;


  #define USARTx                                 USART1
  #define USARTx_BAUDRATE                        115200
  #define USART_RCC_CLK_ENABLE()                 __HAL_RCC_USART1_CLK_ENABLE()
  #define USART_RCC_CLK_DISABLE()                __HAL_RCC_USART1_CLK_DISABLE()

  #define USARTx_GPIO_ClK_ENABLE()               __HAL_RCC_GPIOB_CLK_ENABLE()
  #define USARTx_Tx_GPIO_PIN                     GPIO_PIN_6
  #define USARTx_Tx_GPIO                         GPIOB
  #define USARTx_Rx_GPIO_PIN                     GPIO_PIN_7   
  #define USARTx_Rx_GPIO                         GPIOB

  #define USARTx_AFx                             GPIO_AF7_USART1

  #define USARTx_IRQHANDLER                      USART1_IRQHandler
  #define USARTx_IRQn                            USART1_IRQn

// Э����ض���
#define FRAME_LENTH               16    // ָ���
#define FRAME_START               0xAA  // Э��֡��ʼ
#define FRAME_END                 '/'   // Э��֡����
#define FRAME_CHECK_BEGIN          1    // У���뿪ʼ��λ�� RxBuf[1]
#define FRAME_CHECKSUM            14    // У�����λ��   RxBuf[14]
#define FRAME_CHECK_NUM           13    // ��ҪУ����ֽ���
#define FILL_VALUE                0x55  // ���ֵ
#define CODE_SETPID               0x07  // ����PID����
#define CODE_SETTGT               0x08  // ����Ŀ��ֵ
#define CODE_RESET                0x09   // ��λ����
#define CODE_STARTMOTOR           0x0A   // �������

/* ��չ���� ------------------------------------------------------------------*/
extern UART_HandleTypeDef husartx;

/* ��ֲ��ʱ����Ҫ�޸�����ָ����ָ��ı��� */ 
extern  MSG_TypeDef Msg;
extern __IO uint8_t RxxBuf[FRAME_LENTH] ; // ���ջ�����
extern __IO uint8_t TxxBuf[FRAME_LENTH] ; // ���ջ�����
extern void (*ptr_Fun_)(void) ;//����ָ��
/* �������� ------------------------------------------------------------------*/
void MX_USARTx_Init(void);
void Transmit_FB( __IO uint32_t *suduFeedback);


#endif  /* __BSP_USARTX_H__ */

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
