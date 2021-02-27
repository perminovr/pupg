/*******************************************************************************
  * @file    DebugController.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/

#ifndef DEBUG_CONTROLLER_H_
#define DEBUG_CONTROLLER_H_

#include "database.h"



typedef enum {
	DBGCmd_No,
	DBGCmd_WDT,
	DBGCmd_ClearEeprom,
	DBGCmd_SetTestingParams,
} DBGCommands;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DebugController_Work
 *
 * @brief: Основной метод DebugController
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void DebugController_WorkIn(void);
extern void DebugController_WorkOut(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DebugController_Initialization
 * 
 * @brief: Инициализация DebugController
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
extern int DebugController_Initialization(DB_t *pDB);


#endif /* DEBUG_CONTROLLER_H_ */