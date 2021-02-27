#include "SparkPlug.h"
#include "stm32f4xx.h"
#include <stdlib.h>
#include "utils.h"



static DB_t *DB;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: SparkPlug_Work
 *
 * @brief: Основной метод SparkPlug
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void SparkPlug_Work(void)
{
	OutSig_SparkPlugState = OutSig_SparkPlugCmd;
	GPIO_WriteBit(SPARKPLUG_PORT, SPARKPLUG_PIN, (BitAction)OutSig_SparkPlugState);
}
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: SparkPlug_Initialization
 *
 * @brief: Инициализация драйвера свечи розжига
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
int SparkPlug_Initialization(DB_t *pDB)
{
	if (!pDB) return 1;

	DB = pDB;

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(SPARKPLUG_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = SPARKPLUG_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPARKPLUG_PORT, &GPIO_InitStructure);

	return 0;
}