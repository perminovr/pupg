/******************************************************************************
  * @file    retain.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/

#ifndef RETAIN_H_
#define RETAIN_H_

#include "database.h"


#define Retain_PRIORITY     1



typedef enum {
    Retain_Idle,        /* поток в ожидании команды */
    Retain_Busy,        /* поток занят исполнением команды */
    Retain_Complete,    /* исполнение последней команды завершено */
    Retain_Error,       /* исполнение последней команды завершено с ошибкой */
} Retain_State;



typedef enum {
    Retain_OK,          /* Чтение выполнено */
    Retain_Empty,       /* память пуста */
    Retain_RxError,       /* Ошибка при чтении */
} Retain_RxResult;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_GetState
 * 
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return результат чтения памяти
*/
extern Retain_RxResult Retain_GetRxResult(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_GetState
 * 
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return текущее состояние потока
*/
extern Retain_State Retain_GetState(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_WriteDB
 * 
 * @brief: Запись базы параметров и сигналов
 * 
 * NOTE: Завершение операции можно получить вызвав @ref Retain_GetState
 * 
 * input parameters
 * DB - база параметров и сигналов для записи в eeprom
 * 
 * output parameters
 * 
 * return текущее состояние потока 
*/
extern Retain_State Retain_WriteDB(const DBE_t *DB);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_ClearDB
 * 
 * @brief: Очистка eeprom
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return текущее состояние потока 
*/
extern Retain_State Retain_ClearDB();



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_CopyReadDB
 * 
 * @brief: копирование считанных данных
 * 
 * input parameters
 * 
 * output parameters
 * DB - база параметров и сигналов, хранимая в eeprom
 * 
 * return
 *      * 1 - ошибка при копировании
 *      * 0 - операция прошла успешно
*/
extern int Retain_CopyReadDB(DBE_t *DB);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_ReadDB
 * 
 * @brief: Чтение данных с eeprom
 * 
 * NOTE: Завершение операции можно получить вызвав @ref Retain_GetState;
 *  после того, как операция будет завершена, можно скопировать полученные данные
 *  @ref Retain_CopyReadDB
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return текущее состояние потока 
*/
extern Retain_State Retain_ReadDB();



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_Initialization
 * 
 * @brief: Инициализация EE task
 * 
 * input parameters
 * DBL - лимиты базы параметров
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int Retain_Initialization(DBL_t *DBL);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_Create
 * 
 * @brief: Создание экземпляра Retain
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int Retain_Create(void);



#endif /* RETAIN_H_ */
