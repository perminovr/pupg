/******************************************************************************
  * @file    Screw.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    31.12.2018
  *****************************************************************************/
   
#ifndef SCREW_H_  
#define SCREW_H_

#include "database.h"



#define SCREW_PORT		GPIOA
#define SCREW_PIN		GPIO_Pin_4
#define SCREW_CLK		RCC_AHB1Periph_GPIOA
#define SCREW_MINPOWER	1.0f



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Screw_Work
 *
 * @brief: Основной метод Screw
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void Screw_Work(void); 
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Screw_Initialization
 *
 * @brief: Инициализация драйвера шнека
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
extern int Screw_Initialization(DB_t *pDB);  



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Screw_Create
 *
 * @brief: Создание экземпляра Screw
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
extern int Screw_Create(void);


#endif /* SCREW_H_ */