/**
  ******************************************************************************
  * 文件名程: bsp_MB_host.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2017-09-12
  * 功    能: Modbus协议主机底层驱动程序
  ******************************************************************************
  * 说明：
  * 本例程配套硬石stm32开发板YS-F4Pro使用。
  * 
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */
#include "MB-host/bsp_MB_host.h"

#include "RS485/bsp_usartx_RS485.h"
#include "GeneralTIM/bsp_GeneralTIM.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/

// CRC 高位字节值表
static const uint8_t auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
// CRC 低位字节值表
static const uint8_t auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};
/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

/** 
  * 函数功能: Modbus CRC16 校验计算函数
  * 输入参数: pushMsg:待计算的数据首地址,usDataLen:数据长度
  * 返 回 值: CRC16 计算结果
  * 说    明: 计算结果是高位在前,需要转换才能发送
  */
uint16_t MB_CRC16(uint8_t *pushMsg,uint8_t usDataLen)
{
  uint8_t uchCRCHi = 0xFF;
  uint8_t uchCRCLo = 0xFF;
  uint16_t uIndex;
  while(usDataLen--)
  {
    uIndex = uchCRCLo ^ *pushMsg++;
    uchCRCLo = uchCRCHi^auchCRCHi[uIndex];
    uchCRCHi = auchCRCLo[uIndex];
  }
  return (uchCRCHi<<8|uchCRCLo);
}

/** 
  * 函数功能: 读N个线圈状态(CoilStatue)
  * 输入参数: _addr:从站地址,_reg:寄存器地址,_num:待读取的数量
  * 返 回 值: 无
  * 说    明: 填充数据发送缓存区,然后发送
  */
void MB_ReadCoil_01H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	uint16_t TxCount = 0;
  uint16_t crc = 0;
	Tx_Buf[TxCount++] = _addr;		    /* 从站地址 */
	Tx_Buf[TxCount++] = 0x01;		      /* 功能码 */	
	Tx_Buf[TxCount++] = _reg >> 8;	  /* 寄存器地址 高字节 */
	Tx_Buf[TxCount++] = _reg;		      /* 寄存器地址 低字节 */
	Tx_Buf[TxCount++] = _num >> 8;	  /* 线圈(bit)个数 高字节 */
	Tx_Buf[TxCount++] = _num;		      /* 线圈(bit)个数 低字节 */

	crc = MB_CRC16((uint8_t*)&Tx_Buf,TxCount);
  Tx_Buf[TxCount++] = crc;	          /* crc 低字节 */
	Tx_Buf[TxCount++] = crc>>8;		      /* crc 高字节 */
  UART_Tx((uint8_t *)&Tx_Buf,TxCount);
}

/** 
  * 函数功能: 写单个线圈状态(CoilStatue)
  * 输入参数: _addr:从站地址,_reg:寄存器地址,_sta:待写入的线圈状态(0,1)
  * 返 回 值: 无
  * 说    明: 填充数据发送缓存区,然后发送
  */
void MB_WriteCoil_05H(uint8_t _addr, uint16_t _reg, uint16_t _sta)
{
	uint16_t TxCount = 0;
  uint16_t crc = 0;
	Tx_Buf[TxCount++] = _addr;		    /* 从站地址 */
	Tx_Buf[TxCount++] = 0x05;		      /* 功能码 */	
	Tx_Buf[TxCount++] = _reg >> 8;	  /* 寄存器地址 高字节 */
	Tx_Buf[TxCount++] = _reg;		      /* 寄存器地址 低字节 */
	Tx_Buf[TxCount++] = _sta >> 8;	  /* 线圈(bit)个数 高字节 */
	Tx_Buf[TxCount++] = _sta;		      /* 线圈(bit)个数 低字节 */

	crc = MB_CRC16((uint8_t*)&Tx_Buf,TxCount);
  Tx_Buf[TxCount++] = crc;	          /* crc 低字节 */
	Tx_Buf[TxCount++] = crc>>8;		      /* crc 高字节 */
  UART_Tx((uint8_t *)&Tx_Buf,TxCount);
}

/** 
  * 函数功能: 读输入状态状态(InputStatue)
  * 输入参数: _addr:从站地址,_reg:寄存器地址,_num:待读取的输入数量
  * 返 回 值: 无
  * 说    明: 填充数据发送缓存区,然后发送
  */
void MB_ReadInput_02H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	uint16_t TxCount = 0;
  uint16_t crc = 0;
	Tx_Buf[TxCount++] = _addr;		    /* 从站地址 */
	Tx_Buf[TxCount++] = 0x02;		      /* 功能码 */	
	Tx_Buf[TxCount++] = _reg >> 8;	  /* 寄存器地址 高字节 */
	Tx_Buf[TxCount++] = _reg;		      /* 寄存器地址 低字节 */
	Tx_Buf[TxCount++] = _num >> 8;	  /* 开关(Input)个数 高字节 */
	Tx_Buf[TxCount++] = _num;		      /* 开关(Input)个数 低字节 */

	crc = MB_CRC16((uint8_t*)&Tx_Buf,TxCount);
  Tx_Buf[TxCount++] = crc;	          /* crc 低字节 */
	Tx_Buf[TxCount++] = crc>>8;		      /* crc 高字节 */
  UART_Tx((uint8_t *)&Tx_Buf,TxCount);
}

/** 
  * 函数功能: 读保持寄存器(HoldingRegister)
  * 输入参数: _addr:从站地址,_reg:寄存器地址,_num:待读取的寄存器数量
  * 返 回 值: 无
  * 说    明: 填充数据发送缓存区,然后发送
  */
void MB_ReadHoldingReg_03H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	uint16_t TxCount = 0;
  uint16_t crc = 0;
	Tx_Buf[TxCount++] = _addr;		    /* 从站地址 */
	Tx_Buf[TxCount++] = 0x03;		      /* 功能码 */	
	Tx_Buf[TxCount++] = _reg >> 8;	  /* 寄存器地址 高字节 */
	Tx_Buf[TxCount++] = _reg;		      /* 寄存器地址 低字节 */
	Tx_Buf[TxCount++] = _num >> 8;	  /* 寄存器(16bits)个数 高字节 */
	Tx_Buf[TxCount++] = _num;		      /* 低字节 */

	crc = MB_CRC16((uint8_t*)&Tx_Buf,TxCount);
  Tx_Buf[TxCount++] = crc;	          /* crc 低字节 */
	Tx_Buf[TxCount++] = crc>>8;		      /* crc 高字节 */
  UART_Tx((uint8_t *)&Tx_Buf,TxCount);
}

/** 
  * 函数功能: 读N个输入寄存器(InputRegister)
  * 输入参数: _addr:从站地址,_reg:寄存器地址,_num:待读取的寄存器数量
  * 返 回 值: 无
  * 说    明: 填充数据发送缓存区,然后发送.
  */
void MB_ReadInputReg_04H(uint8_t _addr, uint16_t _reg, uint16_t _num)
{
	uint16_t TxCount = 0;
  uint16_t crc = 0;
	Tx_Buf[TxCount++] = _addr;		    /* 从站地址 */
	Tx_Buf[TxCount++] = 0x04;		      /* 功能码 */	
	Tx_Buf[TxCount++] = _reg >> 8;	  /* 寄存器地址 高字节 */
	Tx_Buf[TxCount++] = _reg;		      /* 寄存器地址 低字节 */
	Tx_Buf[TxCount++] = _num >> 8;	  /* 寄存器(16bits)个数 高字节 */
	Tx_Buf[TxCount++] = _num;		      /*  低字节 */

	crc = MB_CRC16((uint8_t*)&Tx_Buf,TxCount);
  Tx_Buf[TxCount++] = crc;	          /* crc 低字节 */
	Tx_Buf[TxCount++] = crc>>8;		      /* crc 高字节 */
  UART_Tx((uint8_t *)&Tx_Buf,TxCount);
}

/** 
  * 函数功能: 写单个保持寄存器(HoldingRegister)
  * 输入参数: _addr:从站地址,_reg:寄存器地址,_data:待写入的寄存器数据
  * 返 回 值: 无
  * 说    明: 填充数据发送缓存区,然后发送
  */
void MB_WriteHoldingReg_06H(uint8_t _addr, uint16_t _reg, uint16_t _data)
{
	uint16_t TxCount = 0;
  uint16_t crc = 0;
	Tx_Buf[TxCount++] = _addr;		    /* 从站地址 */
	Tx_Buf[TxCount++] = 0x06;		      /* 功能码 */	
	Tx_Buf[TxCount++] = _reg >> 8;	  /* 寄存器地址 高字节 */
	Tx_Buf[TxCount++] = _reg;		      /* 寄存器地址 低字节 */
	Tx_Buf[TxCount++] = _data >> 8;	  /* 寄存器(16bits)个数 高字节 */
	Tx_Buf[TxCount++] = _data;		    /*  低字节 */

	crc = MB_CRC16((uint8_t*)&Tx_Buf,TxCount);
  Tx_Buf[TxCount++] = crc;	          /* crc 低字节 */
	Tx_Buf[TxCount++] = crc>>8;		      /* crc 高字节 */
  UART_Tx((uint8_t *)&Tx_Buf,TxCount);
}

/** 
  * 函数功能: 写N个保持寄存器(HoldingRegister)
  * 输入参数: _addr:从站地址,_reg:寄存器地址,_num:待写入的寄存器数量,_databuf:待写入的寄存器数据
  * 返 回 值: 无
  * 说    明: 填充数据发送缓存区,然后发送._databuf的长度需 >= _num*2
  */
void MB_WriteNumHoldingReg_10H(uint8_t _addr, uint16_t _reg, uint16_t _num,uint8_t *_databuf)
{
  uint16_t i;
	uint16_t TxCount = 0;
  uint16_t crc = 0;
	Tx_Buf[TxCount++] = _addr;		    /* 从站地址 */
	Tx_Buf[TxCount++] = 0x10;		      /* 功能码 */	
	Tx_Buf[TxCount++] = _reg >> 8;	  /* 寄存器地址 高字节 */
	Tx_Buf[TxCount++] = _reg;		      /* 寄存器地址 低字节 */
	Tx_Buf[TxCount++] = _num >> 8;	  /* 寄存器(16bits)个数 高字节 */
	Tx_Buf[TxCount++] = _num;		      /* 低字节 */
  Tx_Buf[TxCount++] = _num<<1;		  /* 数据个数 */

  for (i = 0; i < 2 * _num; i++)
  {
		Tx_Buf[TxCount++]  = _databuf[i];		/* 后面的数据长度 */
	}
	crc = MB_CRC16((uint8_t*)&Tx_Buf,TxCount);
  Tx_Buf[TxCount++] = crc;	          /* crc 低字节 */
	Tx_Buf[TxCount++] = crc>>8;		      /* crc 高字节 */
  UART_Tx((uint8_t *)&Tx_Buf,TxCount);
}

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
