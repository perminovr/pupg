/******************************************************************************
  * @file    Security.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/

#ifndef SECURITY_H_
#define SECURITY_H_

#include "database.h"



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Security_HandleFailDiag
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void Security_HandleFailDiag(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Security_Create
 *
 * @brief: Создание экземпляра Security
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
extern int Security_Create(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Security_Initialization
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
extern int Security_Initialization(DB_t *pDB);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Security_Work
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
*/ 
extern void Security_Work(void);



#endif /* SECURITY_H_ */