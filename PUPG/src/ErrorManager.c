/******************************************************************************
  * @file    ErrorManager.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/
  
  
  
#include <stdlib.h>
#include "ErrorManager.h"  



typedef struct ErrorNode_s ErrorNode;



struct ErrorNode_s {
    ErrorCode code;
    Error error;
    ErrorNode *down;
    ErrorNode *up;
};



typedef struct {
	DB_t *DB;
    ErrorNode nodes[Error_tEnd];
    ErrorNode *last;
} ErrorManager_t;



static ErrorManager_t *_ErrorManager = NULL;
  
  
  
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
int ErrorManager_isPushed(Error error)
{
	return (_ErrorManager->nodes[error].down || _ErrorManager->nodes[error].up);
}
  
  
  
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
ErrorCode ErrorManager_GetCode(Error error)
{
    return _ErrorManager->nodes[error].code;
}
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ErrorManager_GetLast
 *
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return код последней не разрешенной ошибки
*/   
Error ErrorManager_GetLast(void)
{
    return _ErrorManager->last->error;
}
  
  
  
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
void ErrorManager_Push(Error error) {	
    /* Нет ошибки - нет добавления */
    if (error == Error_No)
        return;
	
    /* Добавление в журнал */
	{
		DB_t *DB = _ErrorManager->DB;
		for (int i = ERRORS_JOURNAL_SIZE-1; i != 0; --i) {
			Security_Errors[i] = Security_Errors[i-1];
		}
		Security_Errors[0] = _ErrorManager->nodes[error].code;
	}
    
    /* Ошибка уже стоит последней */
    if (_ErrorManager->last == &_ErrorManager->nodes[error]) 
        return;
    
    /* Ошибка выдергивается из текущей позиции */
    if (_ErrorManager->nodes[error].up)  
        _ErrorManager->nodes[error].up->down =  _ErrorManager->nodes[error].down;
    if (_ErrorManager->nodes[error].down)    
        _ErrorManager->nodes[error].down->up = _ErrorManager->nodes[error].up;
        	
    /* Ошибка устанавливается последней */    
	if (_ErrorManager->last)	
		_ErrorManager->last->up = &_ErrorManager->nodes[error];
    _ErrorManager->nodes[error].up = 0;
    _ErrorManager->nodes[error].down = _ErrorManager->last;
    _ErrorManager->last = &_ErrorManager->nodes[error];
}
  
  
  
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
*/ 
void ErrorManager_Pop(Error error) {
    /* Нет ошибки - нет удаления */
    if (error == Error_No)
        return;
	
	/* т.к. всегда есть Error_No, то такого элемента в списке не было */
	if (!_ErrorManager->nodes[error].up && !_ErrorManager->nodes[error].down)
		return;
    
    /* Элемент сверху опускается */
    if (_ErrorManager->nodes[error].up) 
        _ErrorManager->nodes[error].up->down =  _ErrorManager->nodes[error].down;
    else /* или становится последним */
        _ErrorManager->last = _ErrorManager->nodes[error].down;
    
    /* Элемент снизу поднимается */  
    if (_ErrorManager->nodes[error].down) 
        _ErrorManager->nodes[error].down->up = _ErrorManager->nodes[error].up;
	
	/* Удаление связей */
	_ErrorManager->nodes[error].up = 0;
	_ErrorManager->nodes[error].down = 0;
}
  
  
  
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
void ErrorManager_ClearList(void)
{    
    for (Error e = (Error)((int)Error_No+1); e != Error_tEnd; ++e) {
        _ErrorManager->nodes[e].up = 0;
        _ErrorManager->nodes[e].down = 0;
    }	
	_ErrorManager->last = &_ErrorManager->nodes[Error_No];
}
  
  
  
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
void ErrorManager_ForEach( void (*action)(Error error) )
{
	for (Error e = (Error)((int)Error_No+1); e != Error_tEnd; ++e) {
		action(e);
	}
}
  
  
  
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
int ErrorManager_Initialization(DB_t *pDB)
{
	if (!_ErrorManager || !pDB) return 1;
	
	_ErrorManager->DB = pDB;
	
	_ErrorManager->nodes[Error_No].code = ErrorCode_No;
	_ErrorManager->nodes[Error_TypeDsntSet].code = ErrorCode_TypeDsntSet;
	_ErrorManager->nodes[Error_OverheatTSupScrewStop].code = ErrorCode_OverheatTSupScrewStop;
	_ErrorManager->nodes[Error_OverheatTSupPMin].code = ErrorCode_OverheatTSupPMin;
	_ErrorManager->nodes[Error_TSupDsntMeas].code = ErrorCode_TSupDsntMeas;
	_ErrorManager->nodes[Error_OverheatTTrayPMin].code = ErrorCode_OverheatTTrayPMin;
	_ErrorManager->nodes[Error_OverheatTTrayScrewStop].code = ErrorCode_OverheatTTrayScrewStop;
	_ErrorManager->nodes[Error_TTrayDsntMeas].code = ErrorCode_TTrayDsntMeas;
	_ErrorManager->nodes[Error_OverheatTRoomScrewStop].code = ErrorCode_OverheatTRoomScrewStop;
	_ErrorManager->nodes[Error_OverheatTRoomPMin].code = ErrorCode_OverheatTRoomPMin;
	_ErrorManager->nodes[Error_TRoomDsntMeas].code = ErrorCode_TRoomDsntMeas;
	_ErrorManager->nodes[Error_TReturnDsntMeas].code = ErrorCode_TReturnDsntMeas;
	_ErrorManager->nodes[Error_TOutsideDsntMeas].code = ErrorCode_TOutsideDsntMeas;
	_ErrorManager->nodes[Error_Fueljam].code = ErrorCode_Fueljam;
	_ErrorManager->nodes[Error_GSMNoResponse].code = ErrorCode_GSMNoResponse;
	_ErrorManager->nodes[Error_RETAIN_R].code = ErrorCode_RETAIN_R;
	_ErrorManager->nodes[Error_RETAIN_W].code = ErrorCode_RETAIN_W;
	_ErrorManager->nodes[Error_NoFlame].code = ErrorCode_NoFlame;
	_ErrorManager->nodes[Error_FuelLowLevel].code = ErrorCode_FuelLowLevel;
    
    for (Error e = Error_No; e != Error_tEnd; ++e) {
        _ErrorManager->nodes[e].error = e;
    }
    
    /* Всегда есть last */    
    _ErrorManager->last = &_ErrorManager->nodes[Error_No];
	
	return 0;
}



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
int ErrorManager_Create(void)
{
	if (_ErrorManager)
		return 2;
	
	_ErrorManager = (ErrorManager_t*)calloc(1, sizeof(ErrorManager_t));	
	return (_ErrorManager)? 0 : 1;
}



