/******************************************************************************
  * @file    teh.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    31.12.2018
  *****************************************************************************/
  
#ifndef TEH_H_  
#define TEH_H_

#include "database.h"



#define TEH_PORT		GPIOA
#define TEH_PIN			GPIO_Pin_3
#define TEH_RCC			RCC_AHB1Periph_GPIOA



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
extern void TEH_Work(void); 
  
  
  
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
extern int TEH_Initialization(DB_t *pDB);  




#endif /* TEH_H_ */