/*******************************************************************************
  * @file    Observer.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    31.12.2018
  *****************************************************************************/

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include <stdint.h>



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Observer_t
 *
 * @brief: Обозреватель изменения значений переменных
*/
typedef struct {
	uint32_t size;
	uint32_t observSize;
	uint32_t *observed;
	uint32_t *prevValues;
} Observer_t;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Observer_GetChanged
 *
 * @brief: Проверка обозреваемых переменных на изменение
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return указатель на первое изменение
*/
extern uint32_t *Observer_GetChanged(Observer_t *observer);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Observer_SetObservedValues
 *
 * @brief: Установка значений для обозреваемых переменных
 * 
 * input parameters
 * vars - число обозреваемых переменных, должно быть <= size
 * ... - список начальных значений для переменных
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int Observer_SetObservedValues(Observer_t *observer, uint32_t vars, ...);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Observer_Initialization
 *
 * @brief: Инициализация обозреваемых переменных
 * 
 * input parameters
 * vars - число обозреваемых переменных, должно быть <= size
 * ... - список адресов отслеживаемых переменных 
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int Observer_Initialization(Observer_t *observer, uint32_t vars, ...);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Observer_Create
 *
 * @brief: Создание экземпляра Observer
 * 
 * input parameters
 * size - число обозреваемых переменных
 * 
 * output parameters
 * 
 * return экземпляр обозревателя
*/
extern Observer_t *Observer_Create(uint32_t size);


#endif /* OBSERVER_H_ */