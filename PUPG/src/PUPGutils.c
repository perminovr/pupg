/******************************************************************************
  * @file    PUPGutils.c
  * @author  Перминов Р.И.
  * @version v0.0.1.0
  * @date    31.12.2018
  *****************************************************************************/
 

#include <stdio.h>

#include "PUPGutils.h"
#include "common.h"
#include "logserv.h"



//#define DEBUG_MSG_OUTPUT



extern unsigned short __checksum;
extern unsigned int __checksum_begin;
extern unsigned int __checksum_end;



static TIM_CallBack _TIM4_CallBack;
static PUPGFail_CallBack _PUPGFail_CallBack;



static baseprm_t BaseParams;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: msg_print
 *
 * @brief:  
 * 
 * input parameters
 * mask - маска журанала @ref logserv.h
 * msg - сообщение
 * 
 * output parameters
 * 
 * no return
*/
void msg_print(int mask, char *msg)
{
	;
#ifdef DEBUG_MSG_OUTPUT
	printf("%s\n", msg);
#endif
}



/// Функция установки режима wdt
// \param mode - режим wdt
void wdt_mode(int mode)
{
	static int wmode=wmWork;
	
	if (mode != wmCheck) wmode=mode;
	
	switch (wmode) {
		case wmWork: 
			break;
		case wmReset: 
			while(1);
		default: 
			wmode--;
	}
	
	PORT_WDT_RES->ODR ^= GPIO_WDT_RES;
}




/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: mod_fail
 *
 * @brief: Вызывается при возникновении ошибки на этапе диагностики
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
void mod_fail(void)
{
	if (_PUPGFail_CallBack)
		_PUPGFail_CallBack();
	wdt_mode(wmReset); 
}



/// Функция прерывания от таймера TIM3
void TIM3_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM3,TIM_FLAG_Update);

	/* Выдаем импульсы на WDT */
	wdt_mode(wmCheck);
}



/// Функция прерывания от таймера TIM4
void TIM4_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM4,TIM_FLAG_Update);
	
	if (_TIM4_CallBack)
		_TIM4_CallBack();
}



/// Инициализация PHY
void PHY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_PHY_RES;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_PHY_RES,&GPIO_InitStructure);
	
	PORT_PHY_RES->ODR &= ~GPIO_PHY_RES;
	for (int i=0; i<100000; i++);
	PORT_PHY_RES->ODR |= GPIO_PHY_RES;
}



/// Инициализация WDT
void WDT_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_WDT_STAT;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_WDT_STAT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_WDT_RES;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_WDT_RES,&GPIO_InitStructure);
	
	/* Устанавливаем инициализационный режим WDT */
	wdt_mode(wmInit);
}



/// Инициализация переключателя
void SWITCH_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_PIN0;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_PIN0,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_PIN1;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_PIN1,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_PIN2;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_PIN2,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_PIN3;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_PIN3,&GPIO_InitStructure);
}



/// Инициализация таймера
void TIM2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM2_TimeBaseInitStruct;

	/* Timer2 Init */
	TIM_DeInit(TIM2);
	
	/* Enable Timer3 clock and release reset */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM2,DISABLE);

	/* Set timer3 period 1 mksec */
	TIM2_TimeBaseInitStruct.TIM_Prescaler=84-1;
	TIM2_TimeBaseInitStruct.TIM_Period=0xffffffff;
	TIM2_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM2_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM2_TimeBaseInitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&TIM2_TimeBaseInitStruct);

	/* Clear update interrupt bit */
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);

	/* Enable update interrupt */
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

	/* Enable timer counting */
	TIM_Cmd(TIM2,ENABLE);
}



/// Функция инициализации TIM3
static void TIM3_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	/* Init timer */
	TIM_DeInit(TIM3);
	
	/* Enable timer clock and release reset */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3,DISABLE);
	
	/* Set timer period 500 mksec */
	TIM_TimeBaseInitStruct.TIM_Period=42000-1;
	TIM_TimeBaseInitStruct.TIM_Prescaler=0;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);
	
	/* Clear update interrupt bit */
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	
	/* Enable update interrupt */
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	/* Enable timer counting */
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=7;
	
	/* Timer IRQ enable init */
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable timer counting */
	TIM_Cmd(TIM3,ENABLE);
}  



/// Функция инициализации TIM4
static void TIM4_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	/* Init timer */
	TIM_DeInit(TIM4);
	
	/* Enable timer clock and release reset */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM4,DISABLE);
	
	/* Set timer period 500 msec */
	TIM_TimeBaseInitStruct.TIM_Period=1000;
	TIM_TimeBaseInitStruct.TIM_Prescaler=42000-1; // 500 mksec
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
	
	/* Clear update interrupt bit */
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	
	/* Enable update interrupt */
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	
	/* Enable timer counting */
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=7;
	
	/* Timer IRQ enable init */
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable timer counting */
	TIM_Cmd(TIM4,ENABLE);
}  



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: PUPGutils_initPeriph
 *
 * @brief: Инициализация TIM2,3,4 ; PHY ; WDT ; SWITCH
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void PUPGutils_initPeriph()
{	
	/* Инициализируем PHY */
	PHY_Init();
	
	/* Инициализируем WDT */
	WDT_Init();
	
	/* Инициализируем переключатели */
	SWITCH_Init();
	
	/* Инициализируем таймер для работы службы таймаутов */
	TIM2_Init();
	
	/* Инициализируем таймер для работы системы модуля */
	TIM3_Init();	
	
	/* Таймер счетчиков горелки */
	TIM4_Init();
	
	/* Конфигурирование ethernet (GPIOs, clocks, MAC, DMA) */
	// ETH_BSP_Config();
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _PUPGutils_Diagnostic
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _PUPGutils_Diagnostic(void)
{	
	int cnt;			
	cnt = (unsigned int)&__checksum_end - (unsigned int)&__checksum_begin;
	cnt = cnt + 1;

	unsigned short sum = fast_crc16(0, (unsigned char *)&__checksum_begin, cnt);
	while ( sum != (unsigned short)__checksum );
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: PUPGutils_initDevice
 *
 * @brief: Инициализация переферии @ref PUPGutils_initPeriph ; прерываний
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
void PUPGutils_initDevice(TIM_CallBack tcb, PUPGFail_CallBack pfcb)
{
	_PUPGutils_Diagnostic(); // +todo
	
	_TIM4_CallBack = tcb;
	_PUPGFail_CallBack = pfcb;
	
	/* Установка вектора прерываний */
	NVIC_SetVectorTable(ADDR_PROG,0);
	
	/* Установка значений на сброс ложных прерываний по старту  */
	EXTI_ClearFlag(0x7ffff);
	
	/* Устанавливаем приоритет на группу */
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);
	
	/* Инициализация переферии устройства */
	PUPGutils_initPeriph();	
}



void PUPGutils_setupLogger(void)
{
	/* Создание службы логгирования */
	BaseParams.debug = 7;
	log_create(&BaseParams);	
	log_write(TM_NOTE, "started");
}
 


/// Получить системные тики
//\ return - значение системных тиков
u64 __getsystick(void)
{
	u32 tick;
	u64 qtick;
	static u64 ttick=0;
	static u32 ltick=0;
	
	__disable_fault_irq();
	
	tick=TIM_GetCounter(TIM2);
	
	if (tick >= ltick) ttick += tick-ltick;
	else ttick += (0xffffffff-ltick)+tick;
	
	ltick=tick;
	qtick=ttick;
	
	__enable_fault_irq();
	
	return qtick;
}