/*******************************************************************************
  * @file    common.c
  * @author  Пискунов С.Н.
  * @version v0.0.0.1
  * @date    17.07.2018
  *****************************************************************************/

#include "common.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

/* Раскладка приоритетов, 5 - высший приоритет */
int prio[3]={5,4,3};

/***************************************************/
/*****************  1-WIRE  ************************/
/***************************************************/

/// Функция инициализации линий
//\ param num - номер канала
void dsw_initline(u8 num)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	switch (num) {
	case 0:
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		break;
	case 1:
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		break;
	case 2:
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);		
		break;
	case 3:
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);		
		break;	
	case 4:
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
		GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		// GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
		// GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
		// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
		// GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
		// GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		// GPIO_Init(GPIOD,&GPIO_InitStructure);		
		break;
	}
}

/// Функция для установки линии передачи в 1
//\ param num - номер канала
void dsw_setline(u8 num)
{
	switch (num) {
	case 0: GPIOD->ODR &= ~GPIO_Pin_4; break;
	case 1: GPIOD->ODR &= ~GPIO_Pin_7; break;
	case 2: GPIOD->ODR &= ~GPIO_Pin_10; break;
	case 3: GPIOD->ODR &= ~GPIO_Pin_13; break;
	case 4: GPIOD->ODR &= ~GPIO_Pin_15; break;
	}
}

/// Функция для установки линии передачи в 0
//\ param num - номер канала
void dsw_clrline(u8 num)
{
	switch (num) {
	case 0: GPIOD->ODR |= GPIO_Pin_4; break;
	case 1: GPIOD->ODR |= GPIO_Pin_7; break;
	case 2: GPIOD->ODR |= GPIO_Pin_10; break;
	case 3: GPIOD->ODR |= GPIO_Pin_13; break;
	case 4: GPIOD->ODR |= GPIO_Pin_15; break;
	}
}

/// Функция для чтения линии приема
//\ param num - номер канала
u8 dsw_getline(u8 num)
{
	u32 idr=0;
	
	switch (num) {
	case 0: idr=(GPIOD->IDR & GPIO_Pin_3)? 1:0; break;
	case 1: idr=(GPIOD->IDR & GPIO_Pin_6)? 1:0; break;
	case 2: idr=(GPIOD->IDR & GPIO_Pin_9)? 1:0; break;
	case 3: idr=(GPIOD->IDR & GPIO_Pin_12)? 1:0; break;
	case 4: idr=(GPIOD->IDR & GPIO_Pin_14)? 1:0; break;
	}
	
	return idr;
}

/// Функция для установки подтягивающей линии в 1
//\ param num - номер канала
void dsw_setresist(u8 num)
{
}

/// Функция для установки подтягивающей линии в 0
//\ param num - номер канала
void dsw_clrresist(u8 num)
{
}

/// Функция для получения тиков
//\ return тики с периодом 1 мкс
u32 dsw_gettick(void)
{
	return TIM_GetCounter(TIM2);
}

/***************************************************/
/*****************  WEH1602  ***********************/
/***************************************************/

/// Функция для получения тиков
//\ return тики с периодом 1 мкс
u32 weh_gettick(void)
{
	return TIM_GetCounter(TIM2);
}

/// Функция инициализации линий
void weh_initline(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=LCD_RS_Pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(LCD_RS_Port,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=LCD_EN_Pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(LCD_EN_Port,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=LCD_RW_Pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(LCD_RW_Port,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=LCD_DB4_Pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB4_Port,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=LCD_DB5_Pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB5_Port,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=LCD_DB6_Pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB6_Port,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=LCD_DB7_Pin;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(LCD_DB7_Port,&GPIO_InitStructure);
}

/// Функция установки бита на линии
//\ param npin - номер линии
void weh_setline(u8 npin)
{
	switch (npin) {
	case LCD_EN_Num: GPIO_WriteBit(LCD_EN_Port,LCD_EN_Pin,Bit_SET); break;
	case LCD_RS_Num: GPIO_WriteBit(LCD_RS_Port,LCD_RS_Pin,Bit_SET); break;
	case LCD_RW_Num: GPIO_WriteBit(LCD_RW_Port,LCD_RW_Pin,Bit_SET); break;
	case LCD_DB4_Num: GPIO_WriteBit(LCD_DB4_Port,LCD_DB4_Pin,Bit_SET); break;
	case LCD_DB5_Num: GPIO_WriteBit(LCD_DB5_Port,LCD_DB5_Pin,Bit_SET); break;
	case LCD_DB6_Num: GPIO_WriteBit(LCD_DB6_Port,LCD_DB6_Pin,Bit_SET); break;
	case LCD_DB7_Num: GPIO_WriteBit(LCD_DB7_Port,LCD_DB7_Pin,Bit_SET); break;
	}
}

/// Функция снятия бита на линии
//\ param npin - номер линии
void weh_clrline(u8 npin)
{
	switch (npin) {
	case LCD_EN_Num: GPIO_WriteBit(LCD_EN_Port,LCD_EN_Pin,Bit_RESET); break;
	case LCD_RS_Num: GPIO_WriteBit(LCD_RS_Port,LCD_RS_Pin,Bit_RESET); break;
	case LCD_RW_Num: GPIO_WriteBit(LCD_RW_Port,LCD_RW_Pin,Bit_RESET); break;
	case LCD_DB4_Num: GPIO_WriteBit(LCD_DB4_Port,LCD_DB4_Pin,Bit_RESET); break;
	case LCD_DB5_Num: GPIO_WriteBit(LCD_DB5_Port,LCD_DB5_Pin,Bit_RESET); break;
	case LCD_DB6_Num: GPIO_WriteBit(LCD_DB6_Port,LCD_DB6_Pin,Bit_RESET); break;
	case LCD_DB7_Num: GPIO_WriteBit(LCD_DB7_Port,LCD_DB7_Pin,Bit_RESET); break;
	}
}

/// Функция установки/снятия бита на линии по маске
//\ param npin - номер линии
//\ param data - данные
void weh_dataline(u8 npin, u8 data)
{
	switch (npin) {
	case LCD_DB4_Num: GPIO_WriteBit(LCD_DB4_Port,LCD_DB4_Pin,(BitAction)mChkBit(data,4)); break;
	case LCD_DB5_Num: GPIO_WriteBit(LCD_DB5_Port,LCD_DB5_Pin,(BitAction)mChkBit(data,5)); break;
	case LCD_DB6_Num: GPIO_WriteBit(LCD_DB6_Port,LCD_DB6_Pin,(BitAction)mChkBit(data,6)); break;
	case LCD_DB7_Num: GPIO_WriteBit(LCD_DB7_Port,LCD_DB7_Pin,(BitAction)mChkBit(data,7)); break;
	}
}
