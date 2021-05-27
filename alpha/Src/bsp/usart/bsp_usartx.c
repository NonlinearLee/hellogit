/* 包含头文件 ----------------------------------------------------------------*/
#include "usart/bsp_usartx.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
UART_HandleTypeDef husartx;
__IO uint8_t RxxBuf[FRAME_LENTH] ; // 接收缓存区
__IO uint8_t TxxBuf[FRAME_LENTH] ; // 发送缓存区
/* 扩展变量 ------------------------------------------------------------------*/
MSG_TypeDef Msg;
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

/**
  * 函数功能: 串口硬件初始化配置
  * 输入参数: huart：串口句柄类型指针
  * 返 回 值: 无
  * 说    明: 该函数被HAL库内部调用
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart == &husartx)
  {
    /* 使能串口功能引脚GPIO时钟 */
    USARTx_GPIO_ClK_ENABLE();
    /* 串口外设功能GPIO配置 */
    GPIO_InitStruct.Pin = USARTx_Tx_GPIO_PIN|USARTx_Rx_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = USARTx_AFx;
    HAL_GPIO_Init(USARTx_Tx_GPIO, &GPIO_InitStruct);
  }
}


/**
  * 函数功能: NVIC配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
static void MX_NVIC_USARTx_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USARTx_IRQn, 1, 2);
  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

/**
  * 函数功能: 串口参数配置.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void MX_USARTx_Init(void)
{
  /* 串口外设时钟使能 */
  USART_RCC_CLK_ENABLE();
	
  husartx.Instance = USARTx;
  husartx.Init.BaudRate = USARTx_BAUDRATE;
  husartx.Init.WordLength = UART_WORDLENGTH_8B;
  husartx.Init.StopBits = UART_STOPBITS_1;
  husartx.Init.Parity = UART_PARITY_NONE;
  husartx.Init.Mode = UART_MODE_TX_RX;
  husartx.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  husartx.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&husartx);
  /* 配置串口中断并使能，需要放在HAL_UART_Init函数后执行修改才有效 */
  HAL_UART_Receive_IT(&husartx,(uint8_t *)&RxxBuf,FRAME_LENTH); // 重新使能接收中断
  MX_NVIC_USARTx_Init();
}

/**
  * 函数功能: 重定向c库函数printf到DEBUG_USARTx
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&husartx, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}

/**
  * 函数功能: 重定向c库函数getchar,scanf到DEBUG_USARTx
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
int fgetc(FILE * f)
{
  uint8_t ch = 0;
  while(HAL_UART_Receive(&husartx,&ch, 1, 0xffff)!=HAL_OK);
  return ch;
}

uint8_t CheckSum(uint8_t *Ptr,uint8_t Num )
{
  uint8_t Sum = 0;
  while(Num--)
  {
    Sum += *Ptr;
    Ptr++;
  }
  return Sum;
}
/**
  * 函数功能: 发送反馈值
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 将反馈值发送到串口
  */
void Transmit_FB( __IO uint32_t *suduFeedback)
{
  uint8_t i = 0;
  for(i=0;i<FRAME_LENTH;i++)
  {
    TxxBuf[i] = FILL_VALUE;  // 参数填充 0x55
  } 
	
  Msg.data[0].Int = *suduFeedback;//反馈值 速度
  
  TxxBuf[0] = FRAME_START;   // 帧头
  TxxBuf[1] = 0x80|CODE_SETTGT; // 指令码
	TxxBuf[2] = Msg.data[0].Ch[0];
  TxxBuf[3] = Msg.data[0].Ch[1];
  TxxBuf[4] = Msg.data[0].Ch[2];
  TxxBuf[5] = Msg.data[0].Ch[3];

  
  TxxBuf[FRAME_CHECKSUM] = CheckSum((uint8_t*)&TxxBuf[FRAME_CHECK_BEGIN],FRAME_CHECK_NUM);  // 计算校验和
  TxxBuf[FRAME_LENTH-1] = FRAME_END;   // 加入帧尾
  HAL_UART_Transmit_IT(&husartx,(uint8_t *)&TxxBuf,FRAME_LENTH); // 发送数据帧
}



