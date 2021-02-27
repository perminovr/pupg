/******************************************************************************
  * @file    Screw.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    26.12.2018
  *****************************************************************************/

#include <stdlib.h>
#include "Screw.h"
#include "stm32f4xx.h"
#include "utils.h"



typedef struct {
	DB_t *DB;
	u8 state;
	u64 timeStampOff;
	u64 timeStampOn;
	u64 timeStampStart;
} Screw_t;



static Screw_t *_Screw;



/*! ------------------------------------------------------------------------------------------------------------------
* @fn: Screw_Work
*
* @brief: Основной метод Screw
* 
* input parameters
* 
* output parameters
* 
* no return
*/ 
void Screw_Work(void)
{
	DB_t *DB = _Screw->DB;
	int newState = OutSig_ScrewState;

	// включить шнек без алгоритма
	if (OutSig_ScrewCmd) {
		OutSig_ScrewPOut = Service_ScrewPerform * Service_PelletsThermCond;
		newState = 1;
		_Screw->state = 0;
	} else {
		if (OutSig_ScrewP > SCREW_MINPOWER) {
			switch (_Screw->state) {	
				// включение горелки
				case 0: {					
					float koff = OutSig_ScrewP / (Service_ScrewPerform * Service_PelletsThermCond);
					u32 delay = (koff < 0.15f)? 
							2000000 : (koff > 0.5f)?
							8000000 : 4000000;	
					_Screw->timeStampOff = getsystick() + delay; // текущ + Ти	
					_Screw->timeStampOn = _Screw->timeStampOff + (u64)((float)delay * ((1.0f-koff)/koff)); // текущ + Ти + Тп (новый шаг периода)
					newState = 1;
					_Screw->state = 1;	
				} break;
				// включено
				case 1: {
					if (getsystick() > _Screw->timeStampOff) { // ожидание отключения
						newState = 0;		
						_Screw->state = 2;
					}
				} break;
				// выключено
				case 2: {
					if (getsystick() > _Screw->timeStampOn) { // ожидание включения
						_Screw->state = 0;
					}
				} break;
			}
			OutSig_ScrewPOut = OutSig_ScrewP;
		} else {
			newState = 0;
			OutSig_ScrewPOut = 0.0f;
			_Screw->state = 0;
		}
	}
	// on
	if (!OutSig_ScrewState && newState) {
		_Screw->timeStampStart = getsystick();
	}
	// off
	if (OutSig_ScrewState && !newState) {
		u64 tmp = getsystick();
		tmp = ((tmp - _Screw->timeStampStart) / STATISTIC_US_DIVIDER_200US);
		// сервисный счетчик работы шнека (мин)
		Statistic_TimeScrewWorkUs += tmp;
		Statistic_TimeScrewWork = (int)(Statistic_TimeScrewWorkUs / STATISTIC_US_DIVEDER_MIN);
		// счетчик для подсчета нарастающего итога (сек)
		Statistic_TimeScrewWorkCumTtlUs += tmp;
		Statistic_TimeScrewWorkCumTtl = (int)(Statistic_TimeScrewWorkCumTtlUs / STATISTIC_US_DIVEDER_SEC);
		// счетчик для определения уровня топлива (сек)
		Statistic_TimeScrewWorkLvlCtlUs += tmp;
		Statistic_TimeScrewWorkLvlCtl = (int)(Statistic_TimeScrewWorkLvlCtlUs / STATISTIC_US_DIVEDER_SEC);
		// счетчик для определения расхода в час/сутки (500 мсек)
		if (!Burner_DynamicMode) {
			FuelCons_SWTime_SumUs += tmp;
			FuelCons_SWTime_Sum = (int)((FuelCons_SWTime_SumUs * 2l) / STATISTIC_US_DIVEDER_SEC);
		}
	}
	// setup
	OutSig_ScrewState = newState;
	GPIO_WriteBit(SCREW_PORT, SCREW_PIN, (BitAction)OutSig_ScrewState);
}
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Screw_Initialization
 *
 * @brief: Инициализация драйвера шнека
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
int Screw_Initialization(DB_t *pDB)
{
	if (!_Screw || !pDB) return 1;

	_Screw->DB = pDB;

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(SCREW_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = SCREW_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SCREW_PORT, &GPIO_InitStructure);
		
	return 0;

}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Screw_Create
 *
 * @brief: Создание экземпляра Screw
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
int Screw_Create(void)
{
	_Screw = (Screw_t*)calloc(1, sizeof(Screw_t));	
	return (_Screw)? 0 : 1;
}