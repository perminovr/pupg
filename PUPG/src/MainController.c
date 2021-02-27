/******************************************************************************
  * @file    MainController.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/
  
  

#include <float.h>  
#include <stdlib.h>
#include <math.h>
#include "servto.h"
#include "database.h"
#include "ErrorManager.h"



#define _SECOND_DELAY			((u32)1000000)
#define _MINUTE_DELAY			((u32)(60 * _SECOND_DELAY))


#define _FANP_MAX				99
#define _FLAME_HYST				Service_AutoIgnFlameHyst
#define WAIT_DELAY_FAN_DEF		10


#define SENSOR_FLAME_CONTROL_READING	Sensors_FlameAverage
#define IGNITION_COLD_CNT_MAX			4
#define IGNITION_AUTO_CNT_MAX			2


#define K_PINTEGRAL_MAX			0.00003f
#define DELTA_T_CONF_ROOMOUT	2.0f
#define DELTA_P_CFG_ROOMOUT		5.0f
#define ALG_ROOMOUT_PMIN		AlgPrm_PMin



static const u32 _MAINCONTROLLER_ROOM_OUTSIDE_CFG_TO = _MINUTE_DELAY * (u32)30;
static const u32 _MAINCONTROLLER_ROOM_POUT_FIX_TO = _SECOND_DELAY * (u32)10;
static const int _MAINCONTROLLER_ROOM_POUT_FIX_CNT_MAX = 360;
static const int _MAINCONTROLLER_ROOM_OUTSIDE_CNT_MAX = 3;



typedef struct {
	float Pint;
	int resetCnt;
} RoomOutsideWorkPrm_t;



typedef struct {
	to_item_t TOItem;
	u8 TO;
	u8 ConfigState;
	// расчет мощности 
	float Pout_sum;		/* сумма выходной мощности за период конфигурации */
	float Pout_min;		/* минимальная мощность за период */
	float Pout_max;		/* максимальная мощность за период */
	int Pout_cnt;		/* число измерений */
} RoomOutsideCfgPrm_t;



typedef struct {
	u8 TO;
	to_item_t TOItem;
	u8 State;
} StartStopPrm_t;	



typedef struct {
	int Waiting;
	int ColdStart;
	int TransMode;
	int RegularStop;
	int RegularModeFlame;
	int RegularModeFan;
	int RegularModeStart;
} States_t;



typedef enum {
	TurnID_CommonTO,
	TurnID_FanTO,
	TurnID_RoomOutsideTO,
	TurnID_StartStopTO,
} TurnID;



typedef enum {
	BurnerCmd_No,
	BurnerCmd_Start,
	BurnerCmd_Stop,
} BurnerCmd;



typedef struct {
	DB_t *DB; 	/* указатель на базу параметров и сигналов */
	States_t States; /* подсостояния горелки */
		
	/* Обслуживание таймаутов */
	to_que_t TOQueue; /* очередь таймаутов */
	/* таймаут для разных состояний горелки */
	to_item_t CommonTOItem;
	/* таймаут отдувки */
	to_item_t FanTOItem;
	
	/* флаги таймаутов */
	int CommonTO;
	int FanTO;
	
	/* Попыток розжига */
	int IgnitionCnt;
	
	/* Команда горелке */
	BurnerCmd burnerCmd;	

	/* Параметры алгоритма комната+улица */
	RoomOutsideCfgPrm_t roCfgPrm;
	RoomOutsideWorkPrm_t roWorkPrm;
	StartStopPrm_t strtStpPrm;
} MainController_t;



static MainController_t *_MainController = 0;



static void _MainController_ResetStates();
static void _MainController_Waiting();
static void _MainController_ColdStart();
static void _MainController_TransMode();
static void _MainController_RegularMode();
static void _MainController_RegularStop();
static void _MainController_GetCommand();
static void _MainController_ResetAllTO();
static void _MainController_ResetRoomOutsideCfgPrm();
static void _MainController_ResetStartStopPrm();


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
void MainController_Work(void)
{
	DB_t *DB = _MainController->DB;
	to_item_t *turn;

	switch (Burner_ChangedParam) {
		// по изменению алгоритма сбросить все подсостояния
		case ObservedParam_AlgPrm_WorkAlg: {
			_MainController_ResetStates();
		} break;
		default: { /* NOP */ } break;
	}
	
	/* проверка таймаутов */
	while (1) {				
		turn = to_check(&_MainController->TOQueue);
		if (!turn) break;
		to_delete(&_MainController->TOQueue, turn);
		/* установка флагов таймаутов */
		switch (turn->id) {
			case TurnID_CommonTO: {
				_MainController->CommonTO = 1;
			} break;
			case TurnID_FanTO: {
				_MainController->FanTO = 1;
			} break;
			case TurnID_RoomOutsideTO: {
				_MainController->roCfgPrm.TO = 1;
			} break;
			case TurnID_StartStopTO: {
				_MainController->strtStpPrm.TO = 1;
			} break;
		}			
	}
	
	/* Получение команд на старт или стоп */
	_MainController_GetCommand();	
	
	switch (Burner_CurrentState) 
	{
		case BurnerState_ColdStart: 
		case BurnerState_TransMode: 
		case BurnerState_RegularMode: 
		{			
			/* фатальная ошибка */
			if (Security_WorkBlock == (int)WorkBlock_Fatal) {
				_MainController_ResetStates();
				Burner_CurrentState = BurnerState_Error;
				break;
			} 
	
			/* команда на штатную остановку */
			if (Burner_isRequestedStop) {
				Burner_isRequestedStop = 0;
				_MainController_ResetStates();
				Burner_CurrentState = BurnerState_RegularStop;
				return;
			}
			
			/* обработка состояния */
			switch (Burner_CurrentState) 
			{
				case BurnerState_ColdStart: 
				{
					_MainController_ColdStart();
				} break;
				case BurnerState_TransMode: 
				{
					_MainController_TransMode();
				} break;
				case BurnerState_RegularMode: 
				{	
					_MainController_RegularMode();
				} break;
			}
		} break;
		default: {
			/* сброс запроса на стоп */
			Burner_isRequestedStop = 0;
			/* обработка состояния */
			switch (Burner_CurrentState) 
			{
				case BurnerState_Starting:
				{
					Burner_CurrentState = BurnerState_Waiting;
				} break;
				case BurnerState_Waiting:
				{
					_MainController_Waiting();
				} break;
				case BurnerState_RegularStop:
				{
					_MainController_RegularStop();
				} break;
				case BurnerState_Error: 		
				{
					Burner_FanPAlg = 0;
					if (Security_WorkBlock != (int)WorkBlock_Fatal)
						Burner_CurrentState = BurnerState_Waiting;
				} break;
				default: { /* nop */ } break;
			}
		} break;
	}
}



static void _MainController_ColdStart()
{
	DB_t *DB = _MainController->DB;

	switch (_MainController->States.ColdStart) {
		
	}
}



static void _MainController_TransMode()
{
	DB_t *DB = _MainController->DB;

	switch (_MainController->States.TransMode) {
		
	}
}



static void _MainController_RegularMode()
{
	DB_t *DB = _MainController->DB;

	// если горелка включилась в штатном режиме, то по отсутствию 
	//	пламени переход в холодный старт
	{
		if (Burner_isNotDefStartState) {
			_MainController->States.RegularModeStart = 1;
		}

		if (_MainController->States.RegularModeStart) {
			
			return; // no alg
		}
	}

	// в стоп-старте не проверяется
	switch (AlgPrm_WorkAlg) {
		case Algorithm_StartStop: {
			// после достижения заданного уровня температуры 
			//	потеря пламени не должна влиять на работу алгоритма
			if (_MainController->strtStpPrm.State) // == 0 => еще разжигаем
				break; // nop			
		} // no break;
		default: {
			// автомат детектирования пламени
			switch (_MainController->States.RegularModeFlame) {
				
			}
		} break;
	}

	// автомат продувки пепла
	switch (_MainController->States.RegularModeFan) {
		
	}
	
	/* основной алгоритм работы */
	switch (AlgPrm_WorkAlg)
	{
		case Algorithm_Return: 
		case Algorithm_Room:
		case Algorithm_ConfigRoomOut:
		{
			
		} break;	
		case Algorithm_FixedPower:
		{

		} break;
		case Algorithm_RoomOutside:
		{
					
		} break;
		case Algorithm_StartStop: {
			
		} break;
		default: { /* nop */ } break;
	}
}



static void _MainController_RegularStop()
{	
	DB_t *DB = _MainController->DB;
	
	switch (_MainController->States.RegularStop) {
		
	}	
}



static void _MainController_Waiting()
{	
	DB_t *DB = _MainController->DB;	
	
	switch (_MainController->burnerCmd)
	{
		case BurnerCmd_Start: {
			_MainController_ResetAllTO();
			_MainController_ResetStates();
			Burner_CurrentState = BurnerState_ColdStart;
            return;
		} break;
		default: { /* nop */ } break;
	}

	// автомат на отключение вентилятора при простое		
	switch (_MainController->States.Waiting) {
		
	}
}



static void _MainController_GetCommand()
{	
	DB_t *DB = _MainController->DB;
	
	/* HMI */
	switch (HMI_Cmd) {
		case HMICmd_Start: {
			_MainController->burnerCmd = BurnerCmd_Start;
		} break;
		case HMICmd_Stop: {
			// burnerCmd = BurnerCmd_Stop;
			Burner_isRequestedStop = 1;
		} break;
		default: {
			_MainController->burnerCmd = BurnerCmd_No;
		} break;
	}
	
	if (_MainController->burnerCmd == BurnerCmd_No) {
		/* GSM */
		switch (GSM_Cmd) {
			case GSMCmd_Start: {
				_MainController->burnerCmd = BurnerCmd_Start;
			} break;
			case GSMCmd_Stop: {
				// burnerCmd = BurnerCmd_Stop;
				Burner_isRequestedStop = 1;
			} break;
			default: {
				_MainController->burnerCmd = BurnerCmd_No;
			} break;
		}		
	}
}



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
void MainController_HandleFailDiag(void)
{		
	DB_t *DB = _MainController->DB;	
	_MainController_ResetStates();
	Burner_CurrentState = BurnerState_Error;
}



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
void MainController_HandleParamReset(void)
{		
	DB_t *DB = _MainController->DB;	
	_MainController_ResetStates();
	Burner_CurrentState = BurnerState_Waiting;
}



static void _MainController_ResetStates()
{
	DB_t *DB = _MainController->DB;
	Burner_ScrewPAlg = 0;
	Burner_ScrewCmdAlg = 0;
	memset(&_MainController->States, 0, sizeof(States_t));
	_MainController_ResetRoomOutsideCfgPrm();
	_MainController_ResetStartStopPrm();
}



static void _MainController_ResetAllTO(void)
{
	to_delete(&_MainController->TOQueue, &_MainController->CommonTOItem);
	to_delete(&_MainController->TOQueue, &_MainController->FanTOItem);
	to_delete(&_MainController->TOQueue, &_MainController->roCfgPrm.TOItem);
	to_delete(&_MainController->TOQueue, &_MainController->strtStpPrm.TOItem);
	_MainController->CommonTO = 0;
	_MainController->FanTO = 0;
	_MainController->roCfgPrm.TO = 0;
	_MainController->strtStpPrm.TO = 0;
}



static void _MainController_ResetRoomOutsideCfgPrm()
{
	to_delete(&_MainController->TOQueue, &_MainController->roCfgPrm.TOItem);
	memset(&_MainController->roCfgPrm, 0, sizeof(RoomOutsideCfgPrm_t));
	to_inititem(&_MainController->roCfgPrm.TOItem, 0, TurnID_RoomOutsideTO);
	_MainController->roCfgPrm.Pout_min = _FP4_MAX;
	_MainController->roCfgPrm.Pout_max = _FP4_MIN;
}



static void _MainController_ResetStartStopPrm()
{
	to_delete(&_MainController->TOQueue, &_MainController->strtStpPrm.TOItem);
	memset(&_MainController->strtStpPrm, 0, sizeof(StartStopPrm_t));
	to_inititem(&_MainController->strtStpPrm.TOItem, 0, TurnID_StartStopTO);
}



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
int MainController_Initialization(DB_t *pDB)
{
	if (!_MainController || !pDB) return 1;
		
	_MainController->DB = pDB;
	
	/* Инициализация объектов обсулижвания таймаутов */
	to_initque(&_MainController->TOQueue);
	to_inititem(&_MainController->CommonTOItem, 0, TurnID_CommonTO);	
	to_inititem(&_MainController->FanTOItem, 0, TurnID_FanTO);
	to_inititem(&_MainController->roCfgPrm.TOItem, 0, TurnID_RoomOutsideTO);
	to_inititem(&_MainController->strtStpPrm.TOItem, 0, TurnID_StartStopTO);

	_MainController_ResetRoomOutsideCfgPrm();

	return 0;
}



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
int MainController_Create(void)
{
	_MainController = (MainController_t*)calloc(1, sizeof(MainController_t));		
	return (_MainController)? 0 : 1;
}


