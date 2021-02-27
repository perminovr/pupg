/******************************************************************************
  * @file    ErrorManager.h
  * @author  Перминов Р.И.
  * @version v0.0.1.0
  * @date    31.12.2018
  *****************************************************************************/
  
#ifndef ERRORMANAGER_H_
#define ERRORMANAGER_H_
  
#include "database.h"



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Error
 *
 * @brief: Ошибка
 * 
 * NOTE: порядок важен. обработка по приоритету.
 * 
*/
typedef enum {
	Error_No = 0,
	// fatal
	Error_RETAIN_R,
	Error_TypeDsntSet,
	Error_NoFlame,
	Error_TSupDsntMeas,
	Error_TTrayDsntMeas,
	Error_TRoomDsntMeas,
	Error_TReturnDsntMeas,
	Error_TOutsideDsntMeas,
	// regular block
	Error_OverheatTSupScrewStop,
	Error_OverheatTTrayScrewStop,
	Error_OverheatTRoomScrewStop,
	Error_Fueljam,
	// info + limits
	Error_OverheatTSupPMin,
	Error_OverheatTTrayPMin,
	Error_OverheatTRoomPMin,
	// info
	Error_RETAIN_W,
	Error_FuelLowLevel,
	Error_GSMNoResponse,
	Error_tEnd
} Error;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ErrorCode
 *
 * @brief: Коды ошибок
 * 
*/
typedef enum {
	ErrorCode_No = 0,
	ErrorCode_TypeDsntSet = 1,
	ErrorCode_OverheatTSupScrewStop = 12,
	ErrorCode_OverheatTSupPMin = 13,
	ErrorCode_TSupDsntMeas = 14,
	ErrorCode_OverheatTTrayPMin = 22,
	ErrorCode_OverheatTTrayScrewStop = 23,
	ErrorCode_TTrayDsntMeas = 24,
	ErrorCode_OverheatTRoomScrewStop = 42,
	ErrorCode_OverheatTRoomPMin = 43,
	ErrorCode_TRoomDsntMeas = 44,
	ErrorCode_TReturnDsntMeas = 45,
	ErrorCode_TOutsideDsntMeas = 46,
	ErrorCode_Fueljam = 51,
	ErrorCode_GSMNoResponse = 61,
	ErrorCode_RETAIN_R = 71,
	ErrorCode_RETAIN_W = 72,
	ErrorCode_NoFlame = 81,
	ErrorCode_FuelLowLevel = 30,
} ErrorCode;
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_isPushed
 *
 * @brief: Проверка выставлена ли ошибка
 * 
 * input parameters
 * error - ошибка
 * 
 * output parameters
 * 
 * return 
 *		* 1 - ошибка выставлена
 *		* 0 - ошибка не выставлена
*/   
extern int ErrorManager_isPushed(Error error); 
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_GetLast
 *
 * @brief:
 * 
 * input parameters
 * error - ошибка
 * 
 * output parameters
 * 
 * return код ошибки
*/   
extern ErrorCode ErrorManager_GetCode(Error error); 
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_GetLast
 *
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return код последней активной ошибки
*/   
extern Error ErrorManager_GetLast(void);  
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_Push
 *
 * @brief: Добавление ошибки в очередь на обработку, добавление в журнал.
 * 
 * input parameters
 * error - ошибка
 * 
 * output parameters
 * 
 * return
*/  
extern void ErrorManager_Push(Error error); 
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_Pop
 *
 * @brief: Исключение ошибки из очереди на обработку
 * 
 * input parameters
 * error - ошибка
 * 
 * output parameters
 * 
 * return
*/   
extern void ErrorManager_Pop(Error error);  
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_ClearList
 *
 * @brief: Очистка списка ошибок
 *
 * NOTE: Журнал ошибок не очищается
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
*/   
extern void ErrorManager_ClearList(void);  
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_ForEach
 *
 * @brief: Перебор всех ошибок и применение действия
 * 
 * input parameters
 * action - действие, применяемое к каждой ошибке
 * 
 * output parameters
 * 
 * return
*/   
extern void ErrorManager_ForEach( void (*action)(Error error) );  



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_Create
 *
 * @brief: Создание экземпляра ErrorManager
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 2 - менеджер уже создан
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/ 
extern int ErrorManager_Create(void);
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_Pop
 *
 * @brief: Инициализация сервиса ошибок
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
extern int ErrorManager_Initialization(DB_t *pDB);



#endif /* ERRORMANAGER_H_ */