/******************************************************************************
  * @file    SparkPlug.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    31.12.2018
  *****************************************************************************/
  
#ifndef SPARKPLUG_H_  
#define SPARKPLUG_H_

#include "database.h"



#define SPARKPLUG_PORT			GPIOA
#define SPARKPLUG_PIN			GPIO_Pin_5
#define SPARKPLUG_RCC			RCC_AHB1Periph_GPIOA



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
extern void SparkPlug_Work(void); 
  
  
  
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
extern int SparkPlug_Initialization(DB_t *pDB);  


#endif /* SPARKPLUG_H_ */