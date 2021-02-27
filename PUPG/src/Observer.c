/*******************************************************************************
  * @file    Observer.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    31.12.2018
  *****************************************************************************/

#include "Observer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



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
uint32_t *Observer_GetChanged(Observer_t *observer)
{
	uint32_t *ptr1;
	for (uint32_t i = 0; i < observer->observSize; ++i) {
		ptr1 = (uint32_t*)observer->observed[i];
		if ( observer->prevValues[i] != (*ptr1) ) {
			observer->prevValues[i] = (*ptr1);
			return ptr1;
		}
	}
	return 0;
}



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
int Observer_SetObservedValues(Observer_t *observer, uint32_t vars, ...)
{
	if (!observer || vars > observer->size || !observer->observed || !observer->prevValues) 
		return 1;

	va_list argptr;
	va_start (argptr, vars);

	uint32_t it = 0;
	for (; vars; --vars) {
		observer->prevValues[it++] = va_arg(argptr, uint32_t);
	}

	va_end(argptr);
	return 0;
}



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
int Observer_Initialization(Observer_t *observer, uint32_t vars, ...)
{
	if (!observer || vars > observer->size || !observer->observed || !observer->prevValues) 
		return 1;

	observer->observSize = vars;

	va_list argptr;
	va_start (argptr, vars);

	uint32_t it = 0;
	for (; vars; vars--) {
		observer->observed[it++] = va_arg(argptr, uint32_t);
	}

	va_end(argptr);
	return 0;
}



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
Observer_t *Observer_Create(uint32_t size)
{
	Observer_t *observer;
	if (size == 0) return 0;

	observer = (Observer_t*)calloc(1, sizeof(Observer_t));
	if (!observer) return 0;
	observer->size = size;

	observer->observed = (uint32_t*)calloc(size, sizeof(uint32_t));
	if (!observer->observed) return 0;
	observer->prevValues = (uint32_t*)calloc(size, sizeof(uint32_t));
	if (!observer->prevValues) return 0;

	return observer;
}