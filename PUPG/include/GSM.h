/*******************************************************************************
  * @file    GSM.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    12.10.2018
  *****************************************************************************/

#ifndef GSM_H_
#define GSM_H_

#include "database.h"


#define GSM_PWR_CONT_CLK    RCC_AHB1Periph_GPIOB
#define GSM_PWR_KEY_CLK     RCC_AHB1Periph_GPIOB
#define GSM_PWR_CONT_PORT   GPIOB
#define GSM_PWR_KEY_PORT    GPIOB
#define GSM_PWR_CONT_PIN    GPIO_Pin_14
#define GSM_PWR_KEY_PIN     GPIO_Pin_15



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GSM_Work
 *
 * @brief: Основной метод GSM
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void GSM_Work(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GSM_Initialization
 * 
 * @brief: Инициализация GSM 
 * 
 * input parameters
 * DB - указатель на базу параметров и сигналов
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int GSM_Initialization(DB_t *DB);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GSM_Create
 *
 * @brief: Создание экземпляра GSM
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
extern int GSM_Create(void);



#endif /* GSM_H_ */