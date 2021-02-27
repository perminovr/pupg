/******************************************************************************
  * @file    teh.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    26.11.2018
  *****************************************************************************/


#include "teh.h"
#include "stm32f4xx.h"



static DB_t *DB;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TEH_Work
 *
 * @brief: Основной метод TEH
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void TEH_Work(void)
{
	if (OutSig_TEHWorkPermission) {
		/* Команда на включение */
		if (OutSig_TEHCmd) {
			OutSig_TEHState = 1;
		} else {
			/* Алгоритму запрещено работать или обрыв датчика */
			if ( !TEH_WorkPermission || Sensors_SupplyStatus != SensorsTStatus_OK) {
				OutSig_TEHState = 0;
			} else {
				/* Алгоритм работы */
				if (Sensors_SupplyT < TEH_StateOnT - 2.0f) {
					OutSig_TEHState = 1;
				} else if (Sensors_SupplyT > TEH_StateOnT + 2.0f) {
					OutSig_TEHState = 0;
				}
			}
		}
	}
	GPIO_WriteBit(TEH_PORT, TEH_PIN, (BitAction)OutSig_TEHState);
}
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TEH_Initialization
 *
 * @brief: Инициализация драйвера ТЭН
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
int TEH_Initialization(DB_t *pDB)
{
	if (!pDB) return 1;

	DB = pDB;

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(TEH_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = TEH_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TEH_PORT, &GPIO_InitStructure);

	return 0;
} 