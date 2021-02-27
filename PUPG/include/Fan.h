/******************************************************************************
  * @file    Fan.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    31.12.2018
  *****************************************************************************/
  
#ifndef FAN_H_  
#define FAN_H_

#include "database.h"



#define FAN_PORT		GPIOA
#define FAN_PIN			GPIO_Pin_0
#define FAN_PIN_SRC		GPIO_PinSource0
#define FAN_RCC			RCC_AHB1Periph_GPIOA
#define FAN_AF			GPIO_AF_TIM5
#define FAN_TIMx		TIM5
#define FAN_TIMx_CLK	RCC_APB1Periph_TIM5



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
extern void Fan_Work(void); 
  
  
  
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
extern int Fan_Initialization(DB_t *pDB);  



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
extern int Fan_Create(void);




#endif /* Fan_H_ */