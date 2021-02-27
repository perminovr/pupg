/******************************************************************************
  * @file    Fan.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    26.11.2018
  *****************************************************************************/

#include <stdlib.h>
#include "Fan.h"
#include "stm32f4xx.h"
#include "servto.h"


#define FAN_PERIOD 			2000
#define FAN_RESET_PERIOD	1000000



typedef struct {
	DB_t *DB;
	
	/* Обслуживание таймаутов */
	to_que_t TOQueue;	/* очередь таймаутов */
	/* таймауты на запуск ШИМ */
	to_item_t FanTOItem;
} Fan_t;



static Fan_t *_Fan;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Fan_Work
 *
 * @brief: Основной метод Fan
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void Fan_Work(void)
{
	DB_t *DB = _Fan->DB;

	to_item_t *item = to_check(&_Fan->TOQueue);
	if (!item) return;
	to_reappend(&_Fan->TOQueue, &_Fan->FanTOItem, FAN_RESET_PERIOD);	

	int delta = OutSig_FanP - OutSig_FanPOut;	
	if (delta > 0) {
		if (delta > Service_FanTurnChangeRate) {
			OutSig_FanPOut += Service_FanTurnChangeRate;
		} else {
			OutSig_FanPOut = OutSig_FanP;
		}
	} else {
		if (-delta > Service_FanTurnChangeRate) {
			OutSig_FanPOut -= Service_FanTurnChangeRate;
		} else {
			OutSig_FanPOut = OutSig_FanP;
		}
	}

	if (delta != 0) {
		TIM_OCInitTypeDef TIM_OCStruct;
		TIM_OCStructInit(&TIM_OCStruct);
		TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
		TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
		TIM_OCStruct.TIM_Pulse = (uint32_t)(OutSig_FanPOut * FAN_PERIOD / 100);
		TIM_OC1Init(FAN_TIMx, &TIM_OCStruct);
		TIM_OC1PreloadConfig(FAN_TIMx, TIM_OCPreload_Enable);
	}
}
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Fan_Initialization
 *
 * @brief: Инициализация драйвера вентилятора
 * 
 * input parameters
 * pDB - указатель на базу параметров и сигналов
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/ 
int Fan_Initialization(DB_t *pDB)
{
	if (!_Fan || !pDB) return 1;

	_Fan->DB = pDB;
		
	/* Инициализация объектов обсулижвания таймаутов */
	to_initque(&_Fan->TOQueue);
	to_inititem(&_Fan->FanTOItem, 0, 0);	
	
	/* Запуск таймаута */
	to_append(&_Fan->TOQueue, &_Fan->FanTOItem, 0);	

	/* Инициализация переферии */
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseStruct;

	RCC_AHB1PeriphClockCmd(FAN_RCC, ENABLE);

	GPIO_PinAFConfig(FAN_PORT, FAN_PIN_SRC, FAN_AF);

	GPIO_InitStructure.GPIO_Pin = FAN_PIN;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(FAN_PORT, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(FAN_TIMx_CLK, ENABLE);
	TIM_BaseStruct.TIM_Prescaler = 8400-1;
	TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_BaseStruct.TIM_Period = FAN_PERIOD;
	TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_BaseStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(FAN_TIMx, &TIM_BaseStruct);
	TIM_Cmd(FAN_TIMx, ENABLE);	

	return 0;
} 



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Fan_Create
 *
 * @brief: Создание экземпляра Fan
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
int Fan_Create(void)
{
	_Fan = (Fan_t*)calloc(1, sizeof(Fan_t));	
	return (_Fan)? 0 : 1;
} 