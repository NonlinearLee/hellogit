/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "usart/bsp_usartx.h"

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
UART_HandleTypeDef husartx;
__IO uint8_t RxxBuf[FRAME_LENTH] ; // ���ջ�����
__IO uint8_t TxxBuf[FRAME_LENTH] ; // ���ͻ�����
/* ��չ���� ------------------------------------------------------------------*/
MSG_TypeDef Msg;
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/

/**
  * ��������: ����Ӳ����ʼ������
  * �������: huart�����ھ������ָ��
  * �� �� ֵ: ��
  * ˵    ��: �ú�����HAL���ڲ�����
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart == &husartx)
  {
    /* ʹ�ܴ��ڹ�������GPIOʱ�� */
    USARTx_GPIO_ClK_ENABLE();
    /* �������蹦��GPIO���� */
    GPIO_InitStruct.Pin = USARTx_Tx_GPIO_PIN|USARTx_Rx_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = USARTx_AFx;
    HAL_GPIO_Init(USARTx_Tx_GPIO, &GPIO_InitStruct);
  }
}


/**
  * ��������: NVIC����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
static void MX_NVIC_USARTx_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USARTx_IRQn, 1, 2);
  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

/**
  * ��������: ���ڲ�������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void MX_USARTx_Init(void)
{
  /* ��������ʱ��ʹ�� */
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
  /* ���ô����жϲ�ʹ�ܣ���Ҫ����HAL_UART_Init������ִ���޸Ĳ���Ч */
  HAL_UART_Receive_IT(&husartx,(uint8_t *)&RxxBuf,FRAME_LENTH); // ����ʹ�ܽ����ж�
  MX_NVIC_USARTx_Init();
}

/**
  * ��������: �ض���c�⺯��printf��DEBUG_USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&husartx, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}

/**
  * ��������: �ض���c�⺯��getchar,scanf��DEBUG_USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
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
  * ��������: ���ͷ���ֵ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ������ֵ���͵�����
  */
void Transmit_FB( __IO uint32_t *suduFeedback)
{
  uint8_t i = 0;
  for(i=0;i<FRAME_LENTH;i++)
  {
    TxxBuf[i] = FILL_VALUE;  // ������� 0x55
  } 
	
  Msg.data[0].Int = *suduFeedback;//����ֵ �ٶ�
  
  TxxBuf[0] = FRAME_START;   // ֡ͷ
  TxxBuf[1] = 0x80|CODE_SETTGT; // ָ����
	TxxBuf[2] = Msg.data[0].Ch[0];
  TxxBuf[3] = Msg.data[0].Ch[1];
  TxxBuf[4] = Msg.data[0].Ch[2];
  TxxBuf[5] = Msg.data[0].Ch[3];

  
  TxxBuf[FRAME_CHECKSUM] = CheckSum((uint8_t*)&TxxBuf[FRAME_CHECK_BEGIN],FRAME_CHECK_NUM);  // ����У���
  TxxBuf[FRAME_LENTH-1] = FRAME_END;   // ����֡β
  HAL_UART_Transmit_IT(&husartx,(uint8_t *)&TxxBuf,FRAME_LENTH); // ��������֡
}



