#ifndef __BSP_USARTX_RS485_H__
#define __BSP_USARTX_RS485_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* 类型定义 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
#define RS485_USARTx                                 USART3
#define RS485_USARTx_BAUDRATE                        38400  // modbus 默认通信波特率是19.2Kbit/s 
#define RS485_USART_RCC_CLK_ENABLE()                 __HAL_RCC_USART3_CLK_ENABLE()
#define RS485_USART_RCC_CLK_DISABLE()                __HAL_RCC_USART3_CLK_DISABLE()

#define RS485_USARTx_GPIO_ClK_ENABLE()               __HAL_RCC_GPIOB_CLK_ENABLE()
#define RS485_USARTx_PORT                            GPIOB
#define RS485_USARTx_Tx_PIN                          GPIO_PIN_10
#define RS485_USARTx_Rx_PIN                          GPIO_PIN_11

/* 使用485通信的时候才会用到使能IO */
#define RS485_REDE_GPIO_ClK_ENABLE()                 __HAL_RCC_GPIOH_CLK_ENABLE()
#define RS485_REDE_PORT                              GPIOH
#define RS485_REDE_PIN                               GPIO_PIN_8
#define RS485_RX_MODE()                              HAL_GPIO_WritePin(RS485_REDE_PORT,RS485_REDE_PIN,GPIO_PIN_RESET)
#define RS485_TX_MODE()                              HAL_GPIO_WritePin(RS485_REDE_PORT,RS485_REDE_PIN,GPIO_PIN_SET)

#define RS485_USART_IRQn                             USART3_IRQn
#define RS485_USART_IRQHANDLER                       USART3_IRQHandler

  /* 波特率超过19200bit/s的情况下建议的超时时间 */
#define OVERTIME_15CHAR                750.0f    // 750us 
#define OVERTIME_35CHAR               1750.0f  // 1.75ms  
  

	
	
/* 扩展变量 ------------------------------------------------------------------*/
extern UART_HandleTypeDef husartx_rs485;
extern __IO uint8_t Rx_Buf[256];    // 接收缓存,最大256字节
extern __IO uint8_t Tx_Buf[256];    // 接收缓存,最大256字节
extern __IO uint8_t tmp_Rx_Buf;     // 接收缓存
extern __IO uint16_t RxCount;      // 接收字符计数
  
/* 函数声明 ------------------------------------------------------------------*/
void RS485_GPIO_Init(void);
void RS485_USARTx_Init(void);
void UART_Tx(uint8_t *Tx_Buf,uint16_t TxCount);



#endif  /* __BSP_USARTX_RS485_H__ */

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
