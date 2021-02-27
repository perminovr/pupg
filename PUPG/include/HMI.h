/******************************************************************************
  * @file    HMI.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/
  
#ifndef HMI_H_
#define HMI_H_

#include "database.h"



//#define HMI_BLINK_OFF



#define HMI_LED_RCC		RCC_AHB1Periph_GPIOC
#define HMI_LED_PORT	GPIOC
#define HMI_LED_OUT		GPIO_Pin_7



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_CreateWorkScreens
 * 
 * @brief: Создание рабочих окон
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int HMI_CreateWorkScreens(void);




/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_SetMainScreen
 * 
 * @brief: Переход к основному меню
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int HMI_SetMainScreen(void); 



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_Work
 *
 * @brief: Основной метод HMI
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void HMI_Work(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_Initialization
 * 
 * @brief: Инициализация HMI (драйверов weh1602, key)
 *
 * NOTE: Вывод загрузочного экрана после инициализации
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
extern int HMI_Initialization(DB_t *pDB);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_Create
 *
 * @brief: Создание экземпляра HMI
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
extern int HMI_Create(void);



#endif /* HMI_H_ */