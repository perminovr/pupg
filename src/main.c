/******************************************************************************
  * @file    main.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    05.10.2018
  *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "PUPGutils.h"
#include "mbtask.h"
#include "MainController.h"
#include "HMI.h"
#include "insig.h"
#include "outsig.h"
#include "ErrorManager.h"
#include "Security.h"
#include "GSM.h"
#include "DebugController.h"



static DB_t *DB;



void AddTime(void);
void PUPGFail(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Diagnostic
 * 
 * @brief: Диагностика ПУПГ
 * 
 * NOTE: 
 * 	Первым проходит чтение EEPROM. Если возникла ошибка, то выводится специальный экран,
 * 		на котором ожидается нажатие ОК от пользователя.
 * 	Вторым проходит диагностика датчиков котла и обратки. Если возникла ошибка, то горелка переходит
 * 		в состояние Error; на экране начинает мигать ошибка. По восстановлению датчика ошибка уйдет,
 * 		горелка перейдет в режим ожидания.
 * 	Третьм проходит обнаружение пропадания питания. Только фиксация факта.
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return текущее состояние потока
*/
void Diagnostic(void)
{
	// чтения eeprom
	if (ErrorManager_isPushed(Error_RETAIN_R)) {
		MainController_HandleParamReset();
	}
	// ошибка по входным сигналам
	if (InSig_Diagnostic()) {
		Security_HandleFailDiag();
		MainController_HandleFailDiag();
		return;
	}
	// фиксация выключения горелки в работе
	switch (Burner_CurrentState) {
		case BurnerState_reserv1:
		case BurnerState_reserv2:
		case BurnerState_Starting:
		case BurnerState_Waiting:
			break;
		default:
			Burner_isNotDefStartState = 1;
			break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: PUPG_MiscWork
 *
 * @brief: Обработка дополнительных функций
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void PUPG_MiscWork(void)
{
	// сброс флага старта не из состояния по умолчанию
	Burner_isNotDefStartState = 0;

	/* Постобработка после выдачи своих команд */
	switch (PUPG_Cmd) {
		case PUPGCmd_Reboot: {
			vTaskDelay(1000);
			wdt_mode(wmReset);
		} break;
	}

	/* Сброс предыдущей команды */
	PUPG_Cmd = 0;

    /* Команды с HMI ----------------------------------------------- */
    switch (HMI_Cmd) {
        case HMICmd_ResetFuelConsCumTtl: { 
			Statistic_TimeScrewWorkCumTtl = 0;
			Statistic_TimeScrewWorkCumTtlE = 0;
			Statistic_TimeScrewWorkCumTtlUs = 0;
        } break;
		case HMICmd_HopperFull: {
			Statistic_TimeScrewWorkLvlCtl = 0;
			Statistic_TimeScrewWorkLvlCtlE = 0;
			Statistic_TimeScrewWorkLvlCtlUs = 0;
		} break;
		case HMICmd_ClearSettings:
		case HMICmd_Reboot: {
			PUPG_Cmd = PUPGCmd_Reboot;
		} break;
        default: { /* nop */ } break;
    }

	// расход топлива нарастающим итогом / текущий объем топлива
	{
		float perf_kgsec = Service_ScrewPerform / 60.0 / 60.0;
		float kg, tmp;
		kg = (float)Statistic_TimeScrewWorkCumTtl * perf_kgsec;
		Statistic_CumulatTtlFuelCons = kg;
		kg = (float)Statistic_TimeScrewWorkLvlCtl * perf_kgsec;
		tmp = (float)Burner_FuelHopperCapacity - kg;
		Burner_FuelLevel = (tmp > 0)? 
				tmp : 0.0;
		Burner_FuelLevelPerc = (int)(Burner_FuelLevel/(float)Burner_FuelHopperCapacity*100.0);
	}

	// расход топлива в час / сутки
	{
		float k = FuelCons_SWTime_Sum?
			((float)FuelCons_SWTime_Sum/(float)FuelCons_SWTime_Cnt) : 
			(OutSig_ScrewState && !Burner_DynamicMode)? 
			1.0 : 0.0;
		Statistic_FuelPerHr = k * Service_ScrewPerform;
		Statistic_FuelPerDay = Statistic_FuelPerHr * 24.0;
	}

	// проверка уровня топлива
	if (Burner_FuelLowLevelNotif && (Burner_FuelLevelPerc <= Burner_FuelLowLevel)) {
		if (!ErrorManager_isPushed(Error_FuelLowLevel)) {
			ErrorManager_Push(Error_FuelLowLevel); 
		}
	} else {
		ErrorManager_Pop(Error_FuelLowLevel);
	}
}



static void PUPGtask(void *v)
{	
	wdt_mode(wmInit);	

	PUPGutils_setupLogger();
	
	DB = DB_Create();
	
	if (!DB) mod_fail();		
	if (DB_Initialization()) mod_fail();	
	
	if (HMI_Create()) mod_fail();
	if (HMI_Initialization(DB)) mod_fail();	
	
	if (InSig_Create()) mod_fail();
	if (InSig_Initialization(DB)) mod_fail();	
	
	if (OutSig_Create()) mod_fail();
	if (OutSig_Initialization(DB)) mod_fail();	
	
	if (Security_Create()) mod_fail();
	if (Security_Initialization(DB)) mod_fail();	
	
	if (MainController_Create()) mod_fail();
	if (MainController_Initialization(DB)) mod_fail();	

	if (GSM_Create()) mod_fail();
	if (GSM_Initialization(DB)) mod_fail();

	if (HMI_CreateWorkScreens()) mod_fail();	

	if (DebugController_Initialization(DB)) mod_fail();
	
	wdt_mode(wmDiagnostic);	

	Diagnostic();
	
	if (HMI_SetMainScreen()) mod_fail();	
	
	wdt_mode(wmWork);
	
	for (;;) {
		InSig_Work();
		DebugController_WorkIn();
		GSM_Work();
		HMI_Work();		
		MainController_Work();
		DebugController_WorkOut();
		Security_Work();
		OutSig_Work(); // интеграл мощности
		DB_Work();
		PUPG_MiscWork();
		vTaskDelay(5);
	}	
}



/// Основная точка программы
/*
	SYSCLK 168 MHz
	HCLK 168 MHz
	PCLK1 42 MHz
	PLCK2 84 MHz
*/
int main(void)
{			
	// todo CRC!!!!!!!!!!!!!
	PUPGutils_initDevice(AddTime, PUPGFail);
	
	BaseType_t rbt=xTaskCreate(PUPGtask,"PUPGtask",configMINIMAL_STACK_SIZE*6,NULL,1,NULL);
	if (rbt != pdTRUE) mod_fail();
	
	vTaskStartScheduler();
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: PUPGFail
 *
 * @brief: Обработка ошибки ПО
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
void PUPGFail(void)
{
	;
}



typedef enum {
	_PUPG_CNT_TimePultOn,
	_PUPG_CNT_TimePUPGWork,
	_PUPG_CNT_TimeBurnerWork,
	_PUPG_CNT_AutoignitionDelay,
	_PUPG_CNT_ScrewPerfTestTime,
	_PUPG_CNT_tEnd
} _PUPG_CNT;



// счетчики тиков до нужных единиц
static volatile int _PUPG_COUNTERS[_PUPG_CNT_tEnd];
// предыдущие значения парамтеров для синхронизации с изменением
static volatile int _PUPG_PREV_VALS[_PUPG_CNT_tEnd];



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: AddTime
 *
 * @brief: Инкремент времени
 *
 * NOTE: из расчета на вызов раз в 500 мсек
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
void AddTime(void)
{	
	#define SEC 2
	#define MIN 120
	
	/* TimePultOn */
	_PUPG_COUNTERS[_PUPG_CNT_TimePultOn]++;
	if (_PUPG_COUNTERS[_PUPG_CNT_TimePultOn] == SEC) {
		_PUPG_COUNTERS[_PUPG_CNT_TimePultOn] = 0;
		Statistic_TimePultOn++;
	}
	
	/* TimePUPGWork */
	_PUPG_COUNTERS[_PUPG_CNT_TimePUPGWork]++;
	if (_PUPG_COUNTERS[_PUPG_CNT_TimePUPGWork] == MIN) {
		_PUPG_COUNTERS[_PUPG_CNT_TimePUPGWork] = 0;
		Statistic_TimePUPGWork++;
	}

	/* TimeBurnerWork */
	if (Sensors_FlameAverage > (float)Service_NoFlameLvl) {
		_PUPG_COUNTERS[_PUPG_CNT_TimeBurnerWork]++;
		if (_PUPG_COUNTERS[_PUPG_CNT_TimeBurnerWork] == MIN) {
			_PUPG_COUNTERS[_PUPG_CNT_TimeBurnerWork] = 0;
			Statistic_TimeBurnerWork++;
		}
	}

	/* AutoignitionDelay */
	if (TempPrm_AutoignitionDelay != 0) {
		if (_PUPG_PREV_VALS[_PUPG_CNT_AutoignitionDelay] != TempPrm_AutoignitionDelay) {
			_PUPG_PREV_VALS[_PUPG_CNT_AutoignitionDelay] = TempPrm_AutoignitionDelay;
			_PUPG_COUNTERS[_PUPG_CNT_AutoignitionDelay] = 0;
		}
		_PUPG_COUNTERS[_PUPG_CNT_AutoignitionDelay]++;
		if (_PUPG_COUNTERS[_PUPG_CNT_AutoignitionDelay] == MIN) {
			_PUPG_COUNTERS[_PUPG_CNT_AutoignitionDelay] = 0;
			TempPrm_AutoignitionDelay--;
		}		
	}

	/* ScrewPerfTestTime */
	if (TempPrm_ScrewPerfTestTime != 0) {
		_PUPG_COUNTERS[_PUPG_CNT_ScrewPerfTestTime]++;
		if (_PUPG_COUNTERS[_PUPG_CNT_ScrewPerfTestTime] == SEC) {
			_PUPG_COUNTERS[_PUPG_CNT_ScrewPerfTestTime] = 0;
			TempPrm_ScrewPerfTestTime--;
		}
	}

	/* FuelCons_SWTime_Cnt */
	switch (Burner_CurrentState) {
		case BurnerState_ColdStart: 
		case BurnerState_TransMode: 
		case BurnerState_RegularMode: {
			if (!Security_WorkBlock && !Burner_DynamicMode) {
				FuelCons_SWTime_Cnt++;
			}
		} break;
		default: { /* nop */ } break;		
	}
}
