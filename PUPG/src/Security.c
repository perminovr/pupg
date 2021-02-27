/******************************************************************************
  * @file    Security.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/
  
  
  
#include <stdlib.h>
#include "database.h"
#include "ErrorManager.h"
#include "servto.h"



#define _Security_SENSOR_STATUS_TO	60000000

#define _Security_FANP_STOP_WORK	10



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Monitors
 * 
 * @brief: Мониторы безопасности
 * 
*/
typedef enum {
	Monitors_tStart,	
	Monitors_SensorStatus = Monitors_tStart,
	Monitors_Fueljam,
	Monitors_SupplyOverheat,
	Monitors_RoomOverheat,
	Monitors_TrayOverheat,	
	Monitors_tEnd,	
} Monitors;



typedef enum {
	TurnID_Fueljam,
} TurnID;



typedef struct {
	u8 state;
	Error error;
	to_item_t TOItem;
	u8 TO;
} SensorStatus_t;



typedef struct {
	DB_t *DB;		/* указатель на базу параметров и сигналов */
	
	/* Данные монитора статусов датчиков */
	SensorStatus_t SensorStatus[Sensor_tEnd];
	
	/* состояния работы мониторов безопасности */
	u8 FueljamMonitorState;
	u8 SupplyOverheatMonitorState;
	u8 TrayOverheatMonitorState;
	u8 RoomOverheatMonitorState;
	
	/* Обслуживание таймаутов */
	to_que_t TOQueue; 			/* очередь таймаутов */
	to_item_t FueljamTOItem;		/* таймаут на удержание после устранения затора */	
	
	/* флаги таймаутов */
	u8 FueljamTO;	
	
	/* флаг отсутствия установки типа горелки */
	u8 TypeDsntSet;

	/*  */
	u8 FatalError;

	u8 isDiagnosticFail;

	void (*monitors[Monitors_tEnd])(void);
} Security_t;



static Security_t *_Security;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_SupplyOverheatMonitor
 *
 * @brief: Мониторинг затора
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_FueljamMonitor()
{
	DB_t *DB = _Security->DB;
	if (Service_FueljamPermission == 0) {
		ErrorManager_Pop(Error_Fueljam);
		_Security->FueljamMonitorState = 0;
		return;
	}
	switch (_Security->FueljamMonitorState) {
		
	}
}


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_SupplyOverheatMonitor
 *
 * @brief: Мониторинг перегрева подачи
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_SupplyOverheatMonitor()
{
	DB_t *DB = _Security->DB;
	switch (_Security->SupplyOverheatMonitorState) {
		
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_TrayOverheatMonitor
 *
 * @brief: Мониторинг перегрева лотка
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_TrayOverheatMonitor()
{
	DB_t *DB = _Security->DB;
	switch (_Security->TrayOverheatMonitorState) {
		
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_RoomOverheatMonitor
 *
 * @brief: Мониторинг перегрева комнаты
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_RoomOverheatMonitor()
{
	DB_t *DB = _Security->DB;
	switch (_Security->RoomOverheatMonitorState) {
		
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_SensorStatusMonitor
 *
 * @brief: Мониторинг статусов датчиков
 * 
 * NOTE: push and pop:
 * 		Error_TSupDsntMeas
 * 		Error_TTrayDsntMeas
 * 		Error_TRoomDsntMeas
 * 		Error_TReturnDsntMeas
 * 		Error_TOutsideDsntMeas
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_SensorStatusMonitor()
{
	DB_t *DB = _Security->DB;
	
	for (Sensors s = Sensor_tStart; s != Sensor_tEnd; ++s) 
	{
		switch (_Security->SensorStatus[s].state) 
		{

		}
	}	
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_RegularWork
 *
 * @brief: Установки алгоритмической или тестовой мощности
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_RegularWork() 
{
	DB_t *DB = _Security->DB;
	OutSig_FanP = Burner_FanPAlg;
	OutSig_ScrewP = Burner_ScrewPAlg;
	OutSig_ScrewCmd = Burner_ScrewCmdAlg;
	OutSig_SparkPlugCmd = Burner_SparkPlugCmdAlg;	
	if (Burner_DynamicMode) {
		switch (Burner_CurrentDynamicOutput) {
			case DynamicOutput_FanP: {
				OutSig_FanP = TempPrm_FanP;
			} break;
			case DynamicOutput_FanCmd: {
				OutSig_FanP = (TempPrm_FanCmd)? 
						100 : 0;
			} break;
			case DynamicOutput_ScrewP: {
				OutSig_ScrewP = Service_MaxPower * 
						((float)TempPrm_ScrewP/100.0);
			} break;
			case DynamicOutput_ScrewCmd: {
				OutSig_ScrewCmd = TempPrm_ScrewCmd;
			} break;
			case DynamicOutput_SparkPlug: {
				OutSig_SparkPlugCmd = TempPrm_SparkPlugCmd;	
			} break;
			case DynamicOutput_TEH: {
				OutSig_TEHCmd = TempPrm_TEHState;
			} break;
		}
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_SetFatalBlock
 *
 * @brief: Блокировка работы в случае возникновения критической ошибки
 * 
 * input parameters
 * error - проверяемая ошибка
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_SetFatalBlock(Error error)
{
	DB_t *DB = _Security->DB;
	switch (error) 
	{
		case Error_TypeDsntSet:
		case Error_RETAIN_R:
		case Error_NoFlame:
		case Error_TSupDsntMeas:
		case Error_TTrayDsntMeas:
		case Error_TRoomDsntMeas:
		case Error_TReturnDsntMeas:
		case Error_TOutsideDsntMeas: {
			Security_WorkBlock = (int)WorkBlock_Fatal;
		} break;
		default: { /* nop */ } break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_SetRegularBlock
 *
 * @brief:Блокировка работы в случае возникновения штатной ошибки
 * 
 * input parameters
 * error - проверяемая ошибка
 * 
 * output parameters
 * 
 * no return
*/
static void _Security_SetRegularBlock(Error error)
{
	DB_t *DB = _Security->DB;
	switch (error) 
	{	
		case Error_Fueljam:	
		case Error_OverheatTSupScrewStop:
		case Error_OverheatTTrayScrewStop:
		case Error_OverheatTRoomScrewStop: {
			Security_WorkBlock = (int)WorkBlock_Regular;
		} break;
		default: { /* nop */ } break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_SetWorkBlock
 *
 * @brief: Блокировка работы в случае возникновения ошибок
 * 
 * input parameters
 * error - проверяемая ошибка
 * 
 * output parameters
 * 
 * no return
*/
static inline void _Security_SetWorkBlock(Error e)
{
	_Security_SetRegularBlock(e);
	_Security_SetFatalBlock(e);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_HandleFailDiag
 *
 * @brief:
 * 
 * NOTE: костылина
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static inline void _Security_HandleFailDiag(void)
{
	DB_t *DB = _Security->DB;

	OutSig_FanP = _Security_FANP_STOP_WORK;
	OutSig_ScrewP = 0;
	OutSig_ScrewCmd = 0;
	OutSig_SparkPlugCmd = 0;	
	Security_WorkBlock = (int)WorkBlock_Fatal;

	if (
		!ErrorManager_isPushed(Error_TSupDsntMeas) &&
		!ErrorManager_isPushed(Error_TTrayDsntMeas) &&
		!ErrorManager_isPushed(Error_Fueljam)
	) {
		_Security->isDiagnosticFail = 0;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _Security_SetOutputParameters
 *
 * @brief: Установка выходных параметров в зависимости от вознишеих ошибок
 * 
 * input parameters
 * error - проверяемая ошибка
 * 
 * output parameters
 * 
 * return
 * 		0 - параметры не установлены
 * 		1 - параметры установлены
*/
static int _Security_SetOutputParameters(Error error)
{
	DB_t *DB = _Security->DB;

	// установка параметров
	switch (error)
	{
		case Error_Fueljam: {
			OutSig_FanP = AlgPrm_FanPMax;
			OutSig_ScrewP = 0;
			OutSig_ScrewCmd = 0;
			OutSig_SparkPlugCmd = 0;
		} break;
		case Error_TypeDsntSet:
		case Error_RETAIN_R:
		case Error_NoFlame:
		case Error_OverheatTSupScrewStop:
		case Error_OverheatTTrayScrewStop:
		case Error_OverheatTRoomScrewStop:
		case Error_TSupDsntMeas:
		case Error_TTrayDsntMeas:
		case Error_TRoomDsntMeas:
		case Error_TReturnDsntMeas:
		case Error_TOutsideDsntMeas: {
			OutSig_FanP = _Security_FANP_STOP_WORK;
			OutSig_ScrewP = 0;
			OutSig_ScrewCmd = 0;
			OutSig_SparkPlugCmd = 0;
		} break;
		case Error_OverheatTSupPMin:
		case Error_OverheatTTrayPMin:
		case Error_OverheatTRoomPMin: {
			OutSig_FanP = AlgPrm_FanPMin;
			OutSig_ScrewP = AlgPrm_PMin;
			OutSig_ScrewCmd = 0;
			OutSig_SparkPlugCmd = 0;
		} break;
		default: {
			_Security_RegularWork();
		} break;
	}
	return 1;
}



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
void Security_Work()
{
	DB_t *DB = _Security->DB;
	to_item_t *turn;	
	
	/* Тип горелки не задан */
	if (Service_Type == 0) {
		ErrorManager_Push(Error_TypeDsntSet);
		_Security->TypeDsntSet = 1;
		return;
	} else if (_Security->TypeDsntSet) {
		ErrorManager_Pop(Error_TypeDsntSet);
		_Security->TypeDsntSet = 0; // сброс
	}
	
	/* проверка таймаутов  */
	while (1) {				
		turn = to_check(&_Security->TOQueue);
		if (!turn) break;
		to_delete(&_Security->TOQueue, turn);
		/* установка флагов таймаутов */
		switch (turn->id) {
			case TurnID_Fueljam: {
				_Security->FueljamTO = 1;
			} break;
			default: {
				for (Sensors s = Sensor_tStart; s != Sensor_tEnd; ++s) {
					_Security->SensorStatus[s].TO = 
							(turn == &_Security->SensorStatus[s].TOItem)? 
							1 : _Security->SensorStatus[s].TO;
				}
			} break;
		}			
	}	

	/* работа мониторов */
	for (Monitors m = Monitors_tStart; m != Monitors_tEnd; ++m) {
		_Security->monitors[m]();
	}

	/* нет блока работы по старту */
	if (_Security->isDiagnosticFail == 0) {
		/* по умолчанию ошибок нет */
		_Security_SetOutputParameters(Error_No);
		/* проход по ошибкам */
		switch (Burner_CurrentState) {
			case BurnerState_Error:
			case BurnerState_ColdStart:
			case BurnerState_RegularMode:
			case BurnerState_TransMode: 
			{
				Security_WorkBlock = (int)WorkBlock_No; // сброс
				for (Error e = (Error)((int)Error_No+1); e != Error_tEnd; ++e) {					
					if ( ErrorManager_isPushed(e) ) {
						/* установка выходных параметров согласно текущей ошибке */
						_Security_SetOutputParameters(e);
						/* установка блокировки работы */
						_Security_SetWorkBlock(e);
						break; // первая установленная ошибка самая приоритетная
					}					
				}
			} break;
			default: { /* nop */ } break;
		}
	} 
	/* блок работы по ошибкам с диагностики */
	else {
		_Security_HandleFailDiag();
	}
}



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
void Security_HandleFailDiag(void)
{	
	DB_t *DB = _Security->DB;

	Security_WorkBlock = (int)WorkBlock_Fatal;
	_Security->isDiagnosticFail = 1;
	if (Sensors_InstantSupplyStatus != SensorsTStatus_OK) {
		ErrorManager_Push(Error_TSupDsntMeas);
		_Security->SensorStatus[Sensor_Supply].state = 2; // ожидание восст
		Sensors_SupplyStatus = Sensors_InstantSupplyStatus;
	}
	if (Sensors_InstantTrayStatus != SensorsTStatus_OK) {
		ErrorManager_Push(Error_TTrayDsntMeas);
		_Security->SensorStatus[Sensor_Tray].state = 2; // ожидание восст
		Sensors_TrayStatus = Sensors_InstantTrayStatus;
	}
	if (Service_FueljamPermission != 0 && Sensors_FueljamState) {
		ErrorManager_Push(Error_Fueljam);
		_Security->FueljamMonitorState = 1; // ожидание восст
	}
}



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
int Security_Initialization(DB_t *pDB)
{
	if (!_Security || !pDB) return 1;
		
	DB_t *DB = _Security->DB = pDB;
	
	/* Инициализация объектов обсулижвания таймаутов */
	to_initque(&_Security->TOQueue);
	for (Sensors s = Sensor_tStart; s != Sensor_tEnd; ++s) {
		to_inititem(&_Security->SensorStatus[s].TOItem, 0, 127);		
		switch (s) {
			case Sensor_Supply:
				_Security->SensorStatus[s].error = Error_TSupDsntMeas;
				break;
			case Sensor_Return:
				_Security->SensorStatus[s].error = Error_TReturnDsntMeas;
				break;
			case Sensor_Tray:
				_Security->SensorStatus[s].error = Error_TTrayDsntMeas;
				break;
			case Sensor_Room:
				_Security->SensorStatus[s].error = Error_TRoomDsntMeas;
				break;
			case Sensor_Outside:
				_Security->SensorStatus[s].error = Error_TOutsideDsntMeas;
				break;
		}
	}			
	to_inititem(&_Security->FueljamTOItem, 0, TurnID_Fueljam);

	_Security->monitors[Monitors_SensorStatus] = _Security_SensorStatusMonitor;
	_Security->monitors[Monitors_Fueljam] = _Security_FueljamMonitor;
	_Security->monitors[Monitors_SupplyOverheat] = _Security_SupplyOverheatMonitor;
	_Security->monitors[Monitors_RoomOverheat] = _Security_RoomOverheatMonitor;
	_Security->monitors[Monitors_TrayOverheat] = _Security_TrayOverheatMonitor;

	// разрешить работу ТЭН. запрещения работы не предусмотренно
	OutSig_TEHWorkPermission = 1;
	
	return 0;
}



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
int Security_Create(void)
{
	_Security = (Security_t*)calloc(1, sizeof(Security_t));		
	return (_Security)? 0 : 1;
}


