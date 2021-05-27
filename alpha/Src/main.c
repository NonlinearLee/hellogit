/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "led/bsp_led.h"
#include "GeneralTIM/bsp_GeneralTIM.h"
#include "MB-host/bsp_MB_host.h"
#include "RS485/bsp_usartx_RS485.h"
#include "usart/bsp_usartx.h"
#include "AD5689/bsp_AD5689.h"
#include "beep/bsp_beep.h"

typedef struct 
{
  __IO float SetPoint;       // 目标值  单位:mm
  __IO int   LastError;        // 前一次误差
  __IO int 	 PrevError;        // 前两次误差
  
  __IO long SumError;        // 累计误差
  __IO float Proportion;     // Kp系数
  __IO float Integral;       // Ki系数
  
  __IO float Derivative;     // Kd系数
  __IO uint32_t feedbook;    // 速度反馈值
  __IO uint32_t feedjiaodu;  // 角度反馈值
}PID;
/*定义MODBUS协议--------------------------------------------------------------*/
typedef struct
{
	uint16_t DATA_01H;
	uint16_t DATA_02H;
    uint16_t DATA_03H;
	uint16_t DATA_04H;
	uint16_t DATA_05H;
	uint16_t DATA_06H;
	uint8_t  DATA_10H[64];
}REG_DATA;  
/* 私有宏定义 ----------------------------------------------------------------*/
#define MSG_ERR_FLAG  0xFFFF    // 接收错误 字符间超时
#define MSG_IDLE      0x0000    // 空闲状态
#define MSG_RXING     0x0001    // 正在接收数据
#define MSG_COM       0x0002    // 接收完成
#define MSG_INC       0x8000    // 数据帧不完整(两字符间的空闲间隔大于1.5个字符时间)
#define TIME_OVERRUN  100       // 定义超时时间 ms
#define abs(x)    ((x)<0?(-x):(x))
//将浮点数x四舍五入为int32_t
#define ROUND_TO_INT16(x)   ((int16_t)(x)+0.5f)>=(x)? ((int16_t)(x)):((uint16_t)(x)+1) 
/* 私有变量 ------------------------------------------------------------------*/
__IO static PID sPID,vPID; 
/* 私有变量 ------------------------------------------------------------------*/
__IO uint32_t Time_CNT = 0;//时间分段
__IO int32_t  start_flag = 0;//开始标志位
__IO int32_t  sudubuchang=2600; //速度补偿待定
__IO uint16_t Rx_MSG = MSG_IDLE;   // 接收报文状态
__IO uint8_t  rx_flag=0;
__IO int16_t sudusheding=0x8000;   // 速度给定


__IO  float Dis_Exp_Val = 0;     // PID计算出来的期望值
__IO  float Vel_Exp_Val = 0;     // PID计算出来的期望值
__IO  float Dis_Target = 0;             // 目标位置对应刻度
__IO  float Vel_Target = 0;             // 每单位采样周期内的脉冲数速度
__IO  int32_t MSF = 0;                  // 电机反馈速度
__IO  int32_t CaptureNumber=0;     // 输入捕获数
__IO  int32_t Last_CaptureNumber=0;// 上一次捕获值
__IO  int8_t Motion_Dir = 0;           // 电机运动方向
__IO  float setspeed=0;  

REG_DATA reg_value;
extern __IO uint32_t uwTick;//滴答时钟变量
__IO float *ptr_P = &sPID.Proportion;
__IO float *ptr_I = &sPID.Integral;
__IO float *ptr_D = &sPID.Derivative;
__IO float  *ptr_Target =&sPID.SetPoint;
__IO uint32_t *ptr_FB=&sPID.feedjiaodu;//反馈值         
void (*ptr_Fun_)(void) ;//函数指针  ptr_Fun_ = Analyse_Data_Callback;
uint8_t CheckSum(uint8_t *Ptr,uint8_t Num );
void WaitTimeOut(void);
void Analyse_Data_Callback(void);//解析数据
float IncPIDCalc(int NextPoint,float TargetVal,__IO PID *ptr); //PID参数计算函数
void StopMotor(void);
void StartMotor(void);

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
 
  __HAL_RCC_PWR_CLK_ENABLE();                                     //使能PWR时钟

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);  //设置调压器输出电压级别1

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;      // 外部晶振，8MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                        //打开HSE 
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                    //打开PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;            //PLL时钟源选择HSE
  RCC_OscInitStruct.PLL.PLLM = 8;                                 //8分频MHz
  RCC_OscInitStruct.PLL.PLLN = 336;                               //336倍频
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;                     //2分频，得到168MHz主时钟
  RCC_OscInitStruct.PLL.PLLQ = 7;                                 //USB/SDIO/随机数产生器等的主PLL分频系数
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                               |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;       // 系统时钟：168MHz
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;              // AHB时钟： 168MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;               // APB1时钟：42MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;               // APB2时钟：84MHz
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_RCC_EnableCSS();                                            // 使能CSS功能，优先使用外部晶振，内部时钟源为备用
 	// HAL_RCC_GetHCLKFreq()/1000    1ms中断一次
	// HAL_RCC_GetHCLKFreq()/100000	 10us中断一次
	// HAL_RCC_GetHCLKFreq()/1000000 1us中断一次
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);                // 配置并启动系统滴答定时器
  /* 系统滴答定时器时钟源 */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* 系统滴答定时器中断优先级配置 */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * 函数功能：PID参数初始化
  * 输入参数：无
  * 返 回 值：无
  * 说    明：无
*/
void Init_PIDStruct()
{
  sPID.Proportion = 0.07f;   	//比例系数
  sPID.Integral   = 0.00001f; //积分系数
  sPID.Derivative = 0;      	//微分系数
  sPID.LastError  = 0;      	//前一次的误差
  sPID.PrevError  = 0;      	//前两次的误差
  sPID.SetPoint   = 12000;    //目标值
  sPID.SumError   = 0;      	//累计误差
  
  vPID.Proportion = 0.1f;  //比例系数
  vPID.Integral   = 0.00005f;  //积分系数
  vPID.Derivative = 0;      //微分系数
  vPID.LastError  = 0;      //前一次的误差
  vPID.PrevError  = 0;      //前两次的误差
  vPID.SetPoint   = 30.0f;      //目标值
  vPID.SumError   = 0;      //累计误差 
}
/**
  * 函数功能：增量式PID速度环计算
  * 输入参数：NextPoint     由编码器得到的速度值 
  *           TargetVal    目标值
  * 返 回 值：经过PID运算得到的增量值
  * 说    明：增量式 PID 速度环控制设计,计算得到的结果仍然是速度值
  */
float IncPIDCalc(int NextPoint,float TargetVal,__IO PID *ptr)  
{
  float iError = 0,iIncpid = 0;                       // 当前误差
  iError = TargetVal - NextPoint;                     // 增量计算
  if((iError<0.5f)&&(iError>-0.5f))
    iError = 0;                                       // |e| < 0.8,不做调整
  iIncpid=(ptr->Proportion * iError)                  // E[k]项
              -(ptr->Integral * ptr->LastError)       // E[k-1]项
              +(ptr->Derivative * ptr->PrevError);    // E[k-2]项
  
  ptr->PrevError = ptr->LastError;                    // 存储误差，用于下次计算
  ptr->LastError = iError;
  return(iIncpid);                                    // 返回增量值
}

int main(void)
{
  /* 复位所有外设，初始化Flash接口和系统滴答定时器 */
  HAL_Init();
  /* 配置系统时钟 */
  SystemClock_Config();
	BEEP_GPIO_Init();   
	AD5689_Init();
  /* 定时器初始化 */
  GENERAL_TIMx_Init();
  /* LED初始化 */
  LED_GPIO_Init();	
	  /* 初始化PID参数结构体 */
  Init_PIDStruct();
  /* 设置目标值 */
	RS485_GPIO_Init();
	/* Modbus使用RS485初始化 */
	RS485_USARTx_Init();
  /* 使能定时器中断 */
	 MX_USARTx_Init();
  __HAL_TIM_ENABLE_IT(&htimx,TIM_IT_CC1);
  __HAL_TIM_ENABLE_IT(&htimx,TIM_IT_UPDATE);
  /* 设置字符间超时时间1.5个字符的时间 */
  __HAL_TIM_SET_COMPARE(&htimx,TIM_CHANNEL_1,(uint16_t)OVERTIME_15CHAR);
  /* 设置帧间超时时间3.5个字符的时间 */
  __HAL_TIM_SET_AUTORELOAD(&htimx,(uint16_t)OVERTIME_35CHAR); // 设置帧内间隔时间
  Rx_MSG = MSG_IDLE;
  ptr_Fun_ = Analyse_Data_Callback;
  /* 初始化PID参数结构体 */
  Init_PIDStruct();
	reg_value.DATA_03H=0x02;      //2个保持寄存器
 AD5689_WriteUpdate_DACREG(DAC_A,sudusheding); //启动
  /* 无限循环 */
  while (1)
  {
		  
 		if(uwTick %20== 0)
  {  
		  Rx_MSG = MSG_IDLE;
      /* 读取线圈状态 */
      MB_ReadHoldingReg_03H(MB_SLAVEADDR, HoldingReg, reg_value.DATA_03H); 
			rx_flag=1;
      /* 等待从机响应 */	
  } 		
    /* 接收到一帧的数据,对缓存提取数据 */
    if(Rx_MSG == MSG_COM)
    {      			
      /* 收到非期望的从站反馈的数据 */
      if(Rx_Buf[0] != MB_SLAVEADDR)
        continue;
			if(rx_flag==1)
			{
        vPID.feedbook=Rx_Buf[5]<<8|Rx_Buf[6];
		  	sPID.feedjiaodu=Rx_Buf[3]<<8|Rx_Buf[4];
				*ptr_FB=sPID.feedjiaodu; 
			}		
			
      static uint16_t crc_check = 0;
      crc_check = ( (Rx_Buf[RxCount-1]<<8) | Rx_Buf[RxCount-2] );
      /* CRC 校验正确 */
      if(crc_check == MB_CRC16((uint8_t*)&Rx_Buf,RxCount-2)) 
      {
        /* 通信错误 */
        if(Rx_Buf[1]&0x80)
        {
          while(1)
          {
            LED1_TOGGLE;
            HAL_Delay(500);
          }
        }
      }
      Rx_MSG = MSG_IDLE;
    }
  }
}
/**
  * 函数功能：启动电机转动
  * 输入参数：无
  * 返 回 值：无
  * 说    明：无
  */
void StartMotor(void)
{         
  start_flag = 1;
  BEEP_ON;
	//增加脉冲使能
}
/**
  * 函数功能：停止电机转动
  * 输入参数：无
  * 返 回 值：无
  * 说    明：无
  */
void StopMotor(void)
{   
  // 增加脉冲禁止
  BEEP_OFF;	
  //驱动器电源断电
  /* 复位数据 */ 
  start_flag      = 0;
  sPID.PrevError  = 0;
  sPID.LastError  = 0;
  sudusheding     = 0x8000;//速度先设置为零
  AD5689_WriteUpdate_DACREG(DAC_A,sudusheding); 
}


/** 
  * 函数功能: 超时等待函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 发送数据之后,等待从机响应,200ms之后则认为超时
  */
void WaitTimeOut(void )
{
  uint16_t TimeOut  = 0;// 通讯超时 单位:ms
  TimeOut = TIME_OVERRUN; // 定义超时时间为100ms,但实际测试时间为200ms
  while(Rx_MSG != MSG_COM)
  {
    HAL_Delay(1);
    if(TimeOut-- == 0)
    {
      if(Rx_MSG != MSG_COM)     // 200ms后还是没有接受数据，则认为超时
      {
        LED3_TOGGLE;
      }
      return ;
    }
  }
}
/** 
  * 函数功能: 串口接收中断回调函数
  * 输入参数: 串口句柄
  * 返 回 值: 无
  * 说    明: 使用一个定时器的比较中断和更新中断作为接收超时判断
  *           只要接收到数据就将定时器计数器清0,当发生比较中断的时候
  *           说明已经超时1.5个字符的时间,认定为帧错误,如果是更新中断
  *           则认为是接受完成
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart == &husartx_rs485)		 
  { switch(Rx_MSG)
    {
      /* 接收到第一个字符之后开始计时1.5/3.5个字符的时间 */
      case MSG_IDLE:
        Rx_MSG = MSG_RXING;
        RxCount = 0;
        HAL_TIM_Base_Start(&htimx);
        break;
      /* 距离上一次接收到数据已经超过1.5个字符的时间间隔,认定为数据帧不完整 */
      case MSG_ERR_FLAG:
        LED1_TOGGLE;
        Rx_MSG = MSG_INC; // 数据帧不完整
      break;
    }
    /* 使能继续接收 */
    Rx_Buf[RxCount] = tmp_Rx_Buf;
    RxCount++;
    __HAL_TIM_SET_COUNTER(&htimx,0); // 重设计数时间
    HAL_UART_Receive_IT(&husartx_rs485,(uint8_t*)&tmp_Rx_Buf,1);
  }// 重新使能接收中断  
	else if(huart == &husartx)
	{
    if(RxxBuf[0] != FRAME_START )    // 帧头正确
    {
      return;
    }
    if(RxxBuf[FRAME_LENTH-1] == FRAME_END ) // 帧尾正确
    { 
      /* 判断校验码 */
      if(CheckSum((uint8_t*)&RxxBuf[FRAME_CHECK_BEGIN],FRAME_CHECK_NUM) != RxxBuf[FRAME_CHECKSUM] )
      {
        Msg.Code = NULL;
        return;
      }
      else
      {
        /* 解析数据帧 */
        ptr_Fun_();
      }
    }
    HAL_UART_Receive_IT(huart,(uint8_t *)&RxxBuf,FRAME_LENTH); 
	}
}
		

/** 
  * 函数功能: 定时器比较中断回调函数
  * 输入参数: 定时器句柄
  * 返 回 值: 无
  * 说    明: 标记已经超时1.5个字符的时间没有接收到数据
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* 如果是第一次发生比较中断,则暂定为错误标记 */
  if(Rx_MSG != MSG_INC)
    Rx_MSG = MSG_ERR_FLAG;
  
  /* 如果是第二次进入比较中断,则认定为报文不完整 */
  else
    Rx_MSG = MSG_INC;
}
/** 
  * 函数功能: 定时器更新中断回调函数
  * 输入参数: 定时器句柄
  * 返 回 值: 无
  * 说    明: 超时3.5个字符的时间没有接收到数据,认为是空闲状态 
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* 如果已经标记了接收到不完整的数据帧,则继续标记为不完整的数据帧 */
  if(Rx_MSG == MSG_INC)
  {
    LED2_ON; 
    Rx_MSG = MSG_INC;
  }
  /* 在正常情况下时接收完成 */
  else
  {
    LED2_OFF;
    Rx_MSG = MSG_COM;
  }
}



/**
  * 函数功能: 解析数据
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 将接收到的数据帧进行解析提取.
  */
void Analyse_Data_Callback()
{
  __IO uint8_t i = 0 ;
  __IO uint8_t *Ptr = &RxxBuf[FRAME_CHECK_BEGIN+1];
  __IO char *Str = &Msg.data[0].Ch[0];
  Msg.Code = RxxBuf[FRAME_CHECK_BEGIN];         // 第二位是指令码
  /* 利用结构体和数组的内存连续性,使用指针提取数据 */
  for(i=0;i<(FRAME_CHECK_NUM-1);i++)
  {
    *Str++ = *Ptr++ ;
  }
  switch(Msg.Code)
  {
    /* 设置PID参数 */
    case CODE_SETPID:
      *ptr_P = Msg.data[0].Float; 
      *ptr_I = Msg.data[1].Float;
      *ptr_D = Msg.data[2].Float;
      break;
    /* 设置目标值 */
    case CODE_SETTGT:
      *ptr_Target = Msg.data[0].Int;
      break;
    case CODE_RESET:
      NVIC_SystemReset();      
      break;
    case CODE_STARTMOTOR:		
			StartMotor();
      break;
    default:break;
  }
}
/**
  * 函数功能: 系统滴答定时器中断回调函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 每发生一次滴答定时器中断进入该回调函数一次
  */

void HAL_SYSTICK_Callback()
{
  Time_CNT++;
  if(start_flag == 1)
  {	/* 20ms 采样周期,控制周期 */
		  Vel_Target=vPID.SetPoint*12;//每单位采样周期内的脉冲数(20ms)
		  Dis_Target=sPID.SetPoint; 
	
    if(Time_CNT % 20 == 0)
    {
			CaptureNumber =sPID.feedjiaodu;
      MSF = CaptureNumber-Last_CaptureNumber;
      Last_CaptureNumber = CaptureNumber;
      MSF = abs(MSF);
      Dis_Exp_Val= IncPIDCalc(CaptureNumber,Dis_Target,&sPID);
		 // Dis_Exp_Val =CaptureNumber-Dis_Target;
			Motion_Dir= Dis_Exp_Val<0?MOTOR_DIR_CCW:MOTOR_DIR_CW;
      Dis_Exp_Val = abs(Dis_Exp_Val);//PID计算的取绝对值	
      //位置环输出作为速度环的输入,需要限制位置环的输出不会超过速度环目标值，以20ms脉冲为单位测速			
      if(Dis_Exp_Val >= Vel_Target)
      Vel_Exp_Val = Vel_Target;
			else
				{
         Vel_Exp_Val+=IncPIDCalc(MSF,Dis_Exp_Val,&vPID);
				}
      //当到达目标位置的时候,这时候已经电机非常慢了.为了减少超调,可以直接将速度环的输出清零
        if(Vel_Exp_Val <= 10.0f)
          Vel_Exp_Val = 0;
			if(CaptureNumber <= 100.0f)
          Vel_Exp_Val =0;
				if(CaptureNumber >= 35500.0f)
          Vel_Exp_Val =0;
		 //根据电机速度，设定方向速度	
			if(Motion_Dir==MOTOR_DIR_CW)
      {     
			setspeed= Vel_Exp_Val/12;
			sudusheding=ROUND_TO_INT16(setspeed*350+sudubuchang+0x8000);//确定放大倍数及死区补偿
	    AD5689_WriteUpdate_DACREG(DAC_A,sudusheding); //启动
      }
     if(Motion_Dir==MOTOR_DIR_CCW)
      {  
			setspeed= Vel_Exp_Val/12;
			sudusheding=ROUND_TO_INT16(-setspeed*350-sudubuchang+0x8000);//确定放大倍数及死区补偿
	    AD5689_WriteUpdate_DACREG(DAC_A,sudusheding); //启动
			}		
    }
  }  
  //50ms反馈一次数据
 if(Time_CNT % 20== 0)
  {
   Transmit_FB(ptr_FB);
  }
  if(Time_CNT == 40)
    Time_CNT = 0; 
}

