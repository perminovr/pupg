/******************************************************************************
  * @file    MainController.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/

#ifndef MAINCONTROLLER_H_
#define MAINCONTROLLER_H_

#include "database.h"



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: MainController_HandleParamReset
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return 
*/ 
extern void MainController_HandleParamReset(void);




/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: MainController_HandleFailDiag
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return 
*/ 
extern void MainController_HandleFailDiag(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: MainController_Work
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
*/ 
extern void MainController_Work(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: MainController_Initialization
 *
 * @brief:
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
extern int MainController_Initialization(DB_t *pDB);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: MainController_Create
 *
 * @brief: Создание экземпляра MainController
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
extern int MainController_Create(void);



#endif /* MAINCONTROLLER_H_ */