/******************************************************************************
  * @file    database.c
  * @author  Перминов Р.И.
  * @version v0.0.1.0
  * @date    31.12.2018
  *****************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "database.h"
#include "ErrorManager.h"
#include "retain.h"
#include "servto.h"
#include "Observer.h"



//#define CLEAR_EEPROM

//#define EEPROM_OFF

#define EEPROM_WR_CNT_MAX		5



static const u32 STATISTICS_WRITE_PERIOD = 5*60*1000*1000;



typedef union {
	float f;
	u32 d;
} float_u32_t;



typedef enum {
	TurnID_StatWr,
} TurnId;



typedef struct {
	DB_t DB;
	DBE_t copyDBE;
	to_que_t TOQueue;
	to_item_t StatWrTOItem;
	u8 State;
	u8 WrErrCnt;

	/* обозреватель изменений значений в базе параметров и сигналов */
	Observer_t *observer;
} _database_t;



static _database_t *_database = NULL;



static void _database_SetDefaultValues(void);
static void _database_SetDefaultNotResetableValues(void);
static void _database_SetMinMaxValues(void);
static void _database_CorrectMinMaxValues(void);
static void _database_CheckParams(void);
static void _database_CorrectTemp(void);
static void _database_CorrectBurnerType(void);
static void _database_SetupAlgTDeltas(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_ReadValues
 *
 * @brief: Чтение параметров из ЭНП
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
*/
Retain_RxResult _database_ReadValues(void)
{
#ifdef EEPROM_OFF
	return Retain_Empty;
#endif
    if (Retain_GetState() != Retain_Busy && Retain_ReadDB() == Retain_Busy) {
        Retain_State state = Retain_Busy;
        while ( state == Retain_Busy ) {
            state = Retain_GetState();
        }
        if (state == Retain_Complete) {
            return Retain_GetRxResult();
        } else {
            return Retain_RxError;
        }
    } else {
        return Retain_RxError;
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DB_Work
 *
 * @brief: Запись данных в ЭНП по изменению
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
void DB_Work(void)
{	
	to_item_t *turn;
	DB_t *DB = &_database->DB;

	/* Проверка изменений в базе ------------------------------------ */
	u32 *ptr = Observer_GetChanged(_database->observer);
	Burner_ChangedParam = ObservedParam_tEnd; // reset
	if (ptr == (u32*)&(Burner_CurrentState)) {
		Burner_ChangedParam = ObservedParam_Burner_CurrentState;
	}
	else if (ptr == (u32*)&(AlgPrm_WorkAlg)) {
		Burner_ChangedParam = ObservedParam_AlgPrm_WorkAlg;
	}
	else if (ptr == (u32*)&(TempPrm_ScrewCmd)) {
		Burner_ChangedParam = ObservedParam_TempPrm_ScrewCmd;
	}

	/* Проверка таймаутов ------------------------------------------- */
	while(1) {
		turn = to_check(&_database->TOQueue);
		if (!turn) break;
		to_delete(&_database->TOQueue, turn);                
		switch (turn->id) {
			case TurnID_StatWr: { 				
				Statistic_TimePUPGWorkE = Statistic_TimePUPGWork;
				Statistic_TimeBurnerWorkE = Statistic_TimeBurnerWork;
				Statistic_TimeScrewWorkE = Statistic_TimeScrewWork;
				Statistic_TimeScrewWorkLvlCtlE = Statistic_TimeScrewWorkLvlCtl;
				Statistic_TimeScrewWorkCumTtlE = Statistic_TimeScrewWorkCumTtl;
				to_append(&_database->TOQueue, &_database->StatWrTOItem, STATISTICS_WRITE_PERIOD);
			} break;
		}
	}	

    /* Команды с HMI ----------------------------------------------- */
    switch (HMI_Cmd) {
		case HMICmd_ClearErrors: {
			memset(Security_Errors, 0, ERRORS_JOURNAL_SIZE*sizeof(int));
		} break;
		case HMICmd_ClearSettings: {			
			_database_SetDefaultValues();
			_database_CorrectMinMaxValues();
		} break;
		case HMICmd_CorrectTTarget: {
			_database_CorrectTemp();
			_database_CorrectMinMaxValues();
		} break;
		case HMICmd_CorrectTDeltas: {
			_database_SetupAlgTDeltas();
		} break;
		case HMICmd_ChangeBurnerType: {
			_database_CorrectBurnerType();
		} // bo break
		case HMICmd_CorrectMinMax: {
			_database_CorrectMinMaxValues();
		} break;
	}

	/* Автомат записи в eeprom */
	switch (_database->State)
	{
		// Запись данных в EEPROM
		case 0: {			
			#ifdef EEPROM_OFF
				break;
			#endif
			if ( memcmp(&_database->copyDBE, &_database->DB.E, sizeof(DBE_t)) != 0 ) {
				if (Retain_GetState() != Retain_Busy && Retain_WriteDB(&_database->DB.E) == Retain_Busy) {
					memcpy(&_database->copyDBE, &_database->DB.E, sizeof(DBE_t));
					_database->State = 1;
				}
			}
		} break;
		// проверка состояния потока eeprom
		case 1: {
			Retain_State rs = Retain_GetState();
			switch (rs) {
				case Retain_Error: {
					_database->WrErrCnt++;
					if (_database->WrErrCnt > EEPROM_WR_CNT_MAX) {
						ErrorManager_Push(Error_RETAIN_W); // не снимается
						_database->State = 2;
					}
				} break;
				case Retain_Complete:
				case Retain_Idle: {
					_database->State = 0;
				}
			}
		} break;
		// останов попыток записи
		case 2: {
			/* nop */
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_SetDefaultNotResetableValues
 *
 * @brief: Установка значений параметров, которые не могут быть сброшены, по умолчанию. 
 * Используется при запуске с чистым EEPROM
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_SetDefaultNotResetableValues(void)
{
	DB_t *DB = &_database->DB;

	Statistic_TimePUPGWorkE = 0;
	Statistic_TimeBurnerWorkE = 0;
	Statistic_TimeScrewWorkE = 0;
	Statistic_TimeScrewWorkLvlCtlE = 0;
	Statistic_TimeScrewWorkCumTtlE = 0;
	
	// Params_charIE_16
	strcpy(Service_FactoryNumber, "000000000000000");
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_SetDefaultValues
 *
 * @brief: Установка значений по умолчанию
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_SetDefaultValues(void)
{
	DB_t *DB = &_database->DB;
	
	int Params_intHE_Def[PARAMS_INTHE_SIZE];
	float Params_floatHE_Def[PARAMS_FLOATHE_SIZE];
	
	/* ********************************** PARAMETERS DEFAULT ********************************** */
	// закоменченные корректируются в _database_CorrectBurnerType
	#define Service_Type_Def						Params_intHE_Def[0]
	#define Service_NoFlameLvl_Def					Params_intHE_Def[1]
	#define Service_StrtStpFanOffDelay_Def			Params_intHE_Def[2]
	#define Service_FanTurnChangeRate_Def			Params_intHE_Def[3]
	#define AlgPrm_WorkAlg_Def						Params_intHE_Def[4]
	#define AlgPrm_TTarget_Def						Params_intHE_Def[5]
	#define AlgPrm_TMin_Def							Params_intHE_Def[6]
	#define AlgPrm_TMax_Def							Params_intHE_Def[7]
	// #define AlgPrm_FanPMin_Def						Params_intHE_Def[8]
	// #define AlgPrm_FanPOpt_Def						Params_intHE_Def[9]
	// #define AlgPrm_FanPMax_Def						Params_intHE_Def[10]
	// #define AlgPrm_FanPFix_Def						Params_intHE_Def[11]
	#define AlgPrm_FanPWait_Def						Params_intHE_Def[12]
	#define AlgPrm_BlowTime_Def						Params_intHE_Def[13]
	#define AlgPrm_BlowPeriod_Def					Params_intHE_Def[14]
	#define AlgPrm_AutoignitionDelay_Def			Params_intHE_Def[15]
	#define Ignition_BlowTime_Def					Params_intHE_Def[16]
	// #define Ignition_FuelWeight_Def					Params_intHE_Def[17]	
	#define Ignition_SparkPlugTime_Def				Params_intHE_Def[18]
	#define Ignition_FanP_Def						Params_intHE_Def[19]
	#define Ignition_Repeat_Def						Params_intHE_Def[20]
	#define Ignition_FixingTime_Def					Params_intHE_Def[21]
	#define Ignition_FanPFixing_Def					Params_intHE_Def[22]
	#define Ignition_ToAutoignitionTime_Def			Params_intHE_Def[23]
	#define Ignition_FlameLvlBurn_Def				Params_intHE_Def[24]
	#define Ignition_FailBlowTime_Def				Params_intHE_Def[25]
	#define Service_TransModeTime_Def				Params_intHE_Def[26]
	// #define Service_PTrans_Def						Params_intHE_Def[27]
	#define Service_AutoIgnFlameHyst_Def			Params_intHE_Def[28]
	#define Security_OverheatTSupPMin_Def			Params_intHE_Def[29]
	#define Security_OverheatTSupScrewStop_Def		Params_intHE_Def[30]
	#define Security_OverheatTRoomScrewStop_Def		Params_intHE_Def[31]
	#define Security_RoomTRegMode_Def				Params_intHE_Def[32]
	#define Security_OverheatTTrayPMin_Def			Params_intHE_Def[33]
	#define Security_OverheatTTrayScrewStop_Def		Params_intHE_Def[34]
	#define Security_FueljamBurnoutTime_Def			Params_intHE_Def[35]
	#define Security_WaitModeDelay_Def				Params_intHE_Def[36]
	#define TEH_WorkPermission_Def					Params_intHE_Def[37]
	#define TEH_StateOnT_Def						Params_intHE_Def[38]
	#define GSM_Mode_Def							Params_intHE_Def[39]
	#define AlgPrm_BlowTimeRegStop_Def				Params_intHE_Def[40]
	#define GSM_Repeat_Def				            Params_intHE_Def[41]
	#define Service_TransModeFanP_Def				Params_intHE_Def[42]
	#define Service_FueljamPermission_Def			Params_intHE_Def[43]
	#define Security_OverheatTRoomPMin_Def			Params_intHE_Def[44]
	#define AlgPrm_StrtStpTTargetHyst_Def					Params_intHE_Def[45]
	#define Burner_FuelHopperCapacity_Def			Params_intHE_Def[46]
	#define Burner_FuelLowLevel_Def					Params_intHE_Def[47]
	#define Burner_FuelLowLevelNotif_Def			Params_intHE_Def[48]
	#define HMI_Locale_Def							Params_intHE_Def[49]
	#define AlgPrm_StartStopSensor_Def				Params_intHE_Def[50]

	#define Service_PelletsThermCond_Def			Params_floatHE_Def[0]
	#define Service_ScrewPerform_Def				Params_floatHE_Def[1]
	// #define Service_MaxPower_Def					Params_floatHE_Def[2]
	// #define AlgPrm_PMin_Def							Params_floatHE_Def[3]
	// #define AlgPrm_POpt_Def							Params_floatHE_Def[4]
	// #define AlgPrm_PMax_Def							Params_floatHE_Def[5]
	// #define AlgPrm_PFix_Def							Params_floatHE_Def[6]
	#define AlgPrm_PCapacity_Def					Params_floatHE_Def[7]
	#define AlgPrm_Pfixro_Def						Params_floatHE_Def[8]
	
	/* Значения по умолчанию ----------------------------------- */
	
	
	Service_Type = Service_Type_Def;

	// Signals_intIE
	Burner_CurrentState = BurnerState_Starting;	
	memset(Security_Errors, 0, sizeof(int)*ERRORS_JOURNAL_SIZE);
    
	// Params_intHE
	memcpy(DB->E.Params_intHE, Params_intHE_Def, sizeof(int)*PARAMS_INTHE_SIZE);

	// Params_floatHE
	memcpy(DB->E.Params_floatHE, Params_floatHE_Def, sizeof(float)*PARAMS_FLOATHE_SIZE);
	
	// Params_charHE_16
	for (int i = 0; i < GSM_NUMBERS_IN_SIZE; ++i) {
		strcpy(GSM_NumbersIn[i], DEFAULT_TELEPHONE_NUMBER);
	}
	for (int i = 0; i < GSM_NUMBERS_OUT_SIZE; ++i) {
		strcpy(GSM_NumbersOut[i], DEFAULT_TELEPHONE_NUMBER);
	}
	strcpy(GSM_Prefix, DEFAULT_COUNTRY_CODE);

	// вызывать после memcpy(DB, *_Def)
	_database_CorrectBurnerType();
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_SetMinMaxValues
 *
 * @brief: Установка min и max значений
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_SetMinMaxValues(void)
{	
	DB_t *DB = &_database->DB;

	/* Минимумы параметров ------------------------------------- */
	
				
	/* Максимумы параметров ------------------------------------ */
	// tag _database_SetMinMaxValues_intMax_Start
	
	// tag _database_SetMinMaxValues_intMax_End
	//

	// 
	/* Смешанные  ------------------------------------------------ */
	switch (Service_Type) {
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_SetupAlgTDeltas
 *
 * @brief: Установка допустимого отклонения от целевой температуры
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_SetupAlgTDeltas(void)
{
	DB_t *DB = &_database->DB;	
	TempPrm_TTagetToMinDelta = AlgPrm_TTarget - AlgPrm_TMin;
	TempPrm_TTagetToMaxDelta = AlgPrm_TMax - AlgPrm_TTarget;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_PresetValues
 *
 * @brief: Предустановка некоторых параметров
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_PresetValues(void)
{	
	DB_t *DB = &_database->DB;	

	Statistic_TimePUPGWork = Statistic_TimePUPGWorkE;
	Statistic_TimeBurnerWork = Statistic_TimeBurnerWorkE;
	Statistic_TimeScrewWork = Statistic_TimeScrewWorkE;
	Statistic_TimeScrewWorkLvlCtl = Statistic_TimeScrewWorkLvlCtlE;
	Statistic_TimeScrewWorkCumTtl = Statistic_TimeScrewWorkCumTtlE;
	Statistic_TimeScrewWorkUs = ((u64)Statistic_TimeScrewWork * (u64)(STATISTIC_US_DIVEDER_MIN));
	Statistic_TimeScrewWorkLvlCtlUs = ((u64)Statistic_TimeScrewWorkLvlCtl * (u64)(STATISTIC_US_DIVEDER_SEC));
	Statistic_TimeScrewWorkCumTtlUs = ((u64)Statistic_TimeScrewWorkCumTtl * (u64)(STATISTIC_US_DIVEDER_SEC));

	Burner_FuelLevelPerc = 100;
	Burner_FuelLevel = (float)Burner_FuelHopperCapacity;

	_database_SetupAlgTDeltas();
	_database_CorrectTemp();
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DB_Initialization
 *
 * @brief: Установка значений по умолчанию, минимума, максимума
 * 
 * input parameters
 * DB - указатель на базу парамтеров и сигналов
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int DB_Initialization(void)
{	
	if (!_database) return 1;
		
	DB_t *DB = &_database->DB;	
	
	/* Инициализация объектов обсулижвания таймаутов */
	to_initque(&_database->TOQueue);
	to_inititem(&_database->StatWrTOItem, 0, TurnID_StatWr);		
	to_append(&_database->TOQueue, &_database->StatWrTOItem, STATISTICS_WRITE_PERIOD);
	
	/* Инициализация ErrorManager */
	if (ErrorManager_Create() == 1) return 1;
	if (ErrorManager_Initialization(DB)) return 1;

	/* Инициализация retain */
	if (Retain_Create() == 1) return 1;
	if (Retain_Initialization(&DB->L)) return 1;
	
	/* Запись значений до чтения eeprom т.к. в eeprom используется сжатие на основе max параметров */
	_database_SetDefaultValues();
	_database_SetupAlgTDeltas();
	_database_CorrectTemp();
	_database_SetMinMaxValues();

	/* 'очистка' eeprom */
#ifdef CLEAR_EEPROM
	if (Retain_GetState() != Retain_Busy && Retain_ClearDB() == Retain_Busy) {
        Retain_State state = Retain_Busy;
        while ( state == Retain_Busy ) {
            state = Retain_GetState();
        }
	}	
#endif	

	/* Чтение с RETAIN */
	Retain_RxResult res = _database_ReadValues();
	switch (res) {
        case Retain_OK:
            Retain_CopyReadDB(&DB->E);
			// сохранить копию, если считано, иначе - запись в eeprom
			memcpy(&_database->copyDBE, &_database->DB.E, sizeof(DBE_t));
            break;
		case Retain_RxError:
		case Retain_Empty:
			_database_SetDefaultNotResetableValues();
			ErrorManager_Push(Error_RETAIN_R); // снимается в модуле HMI @ref ErrorManager_Pop(Error_RETAIN_R);
			break;
	}

	_database_PresetValues();
	_database_SetMinMaxValues();
	_database_CorrectMinMaxValues();
	_database_CheckParams();

	/* Настройка обозревателя изменений парметров базы */
	_database->observer = Observer_Create(ObservedParam_tEnd);
	if (!_database->observer) return 1;

	if ( Observer_Initialization(
				_database->observer,
				ObservedParam_tEnd,
				&(Burner_CurrentState),
				&(AlgPrm_WorkAlg),
				&(TempPrm_ScrewCmd)) 
	) {
		return 1;
	}

	return 0;
}



void _database_CheckParams(void)
{
	DB_t *DB = &_database->DB;	

	float_u32_t val = { .f = AlgPrm_PCapacity };
	Burner_isSetROParams = (val.d == 0xFFFFFFFF)? 
			0 : 1;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: If*ThenEqual*
 *
 * @brief: Макросы установки значений
 * 
*/
#define IfHigherThenEqual(a,b) { if (a > b) a = b; }
#define IfLowerThenEqual(a,b) { if (a < b) a = b; }



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_CorrectTemp
 *
 * @brief: Корректировка температур перехода на мин и макс от целевой
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_CorrectTemp(void)
{
	DB_t *DB = &_database->DB;
	AlgPrm_TMin = AlgPrm_TTarget - TempPrm_TTagetToMinDelta;
	AlgPrm_TMax = AlgPrm_TTarget + TempPrm_TTagetToMaxDelta;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_CorrectMinMaxValues
 *
 * @brief: Корректировка зависимых значений минимума и максимума
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_CorrectMinMaxValues(void)
{	
	DB_t *DB = &_database->DB;
	float BurnerPowers[3] = {0.0, 26.0, 42.0};

	if ( (Service_Type != 0 && Service_MaxPower < 1.0) || Service_Type == 0 ) {
		Service_Type = 0;
		Service_MaxPower = 0;
		AlgPrm_PMin_Max = 0;
		AlgPrm_POpt_Max = 0;
		AlgPrm_PMax_Max = 0;
		AlgPrm_PFix_Max = 0;
		return;
	}
	switch (Service_Type) {
		case 1: {
			Service_PTrans_Min = 4;
			Service_PTrans_Max = 13;
		} break;
		case 2:  {
			Service_PTrans_Min = 6;
			Service_PTrans_Max = 21;
		} break;
	}

	Service_MaxPower_Max = BurnerPowers[Service_Type];	
	IfHigherThenEqual(Service_MaxPower, Service_MaxPower_Max);
	
	AlgPrm_PFix_Max = Service_MaxPower;
	IfHigherThenEqual(AlgPrm_PFix, AlgPrm_PFix_Max);
	
	// burner
	AlgPrm_PMax_Max = Service_MaxPower;
	IfHigherThenEqual(AlgPrm_PMax, AlgPrm_PMax_Max);
	AlgPrm_POpt_Max = AlgPrm_PMax - AlgPrm_Pxxx_Step;
	IfHigherThenEqual(AlgPrm_POpt, AlgPrm_POpt_Max);
	AlgPrm_PMin_Max = AlgPrm_POpt - AlgPrm_Pxxx_Step;
	AlgPrm_PMax_Min = AlgPrm_POpt + AlgPrm_Pxxx_Step;
	IfHigherThenEqual(AlgPrm_PMin, AlgPrm_PMin_Max);
	AlgPrm_POpt_Min = AlgPrm_PMin + AlgPrm_Pxxx_Step;

	// fan
	IfHigherThenEqual(AlgPrm_FanPMax, AlgPrm_FanPMax_Max);
	AlgPrm_FanPOpt_Max = AlgPrm_FanPMax - AlgPrm_FanPxxx_Step;
	IfHigherThenEqual(AlgPrm_FanPOpt, AlgPrm_FanPOpt_Max);
	AlgPrm_FanPMin_Max = AlgPrm_FanPOpt - AlgPrm_FanPxxx_Step;
	AlgPrm_FanPMax_Min = AlgPrm_FanPOpt + AlgPrm_FanPxxx_Step;
	IfHigherThenEqual(AlgPrm_FanPMin, AlgPrm_FanPMin_Max);
	AlgPrm_FanPOpt_Min = AlgPrm_FanPMin + AlgPrm_FanPxxx_Step;
		
	// temp
	AlgPrm_TMax_Min = AlgPrm_TTarget + AlgPrm_Txxx_Step;
	AlgPrm_TMin_Max = AlgPrm_TTarget - AlgPrm_Txxx_Step;
	IfLowerThenEqual(AlgPrm_TMax, AlgPrm_TMax_Min);
	IfHigherThenEqual(AlgPrm_TMin, AlgPrm_TMin_Max);
	// supply overheat
	Security_OverheatTSupPMin_Max = (Security_OverheatTSupScrewStop - 1);
	IfHigherThenEqual(Security_OverheatTSupPMin, Security_OverheatTSupPMin_Max);
	// tray overheat
	Security_OverheatTTrayPMin_Max = (Security_OverheatTTrayScrewStop - 1);
	IfHigherThenEqual(Security_OverheatTTrayPMin, Security_OverheatTTrayPMin_Max);
	// room overheat
	Security_OverheatTRoomPMin_Max = (Security_OverheatTRoomScrewStop - 1);
	IfHigherThenEqual(Security_OverheatTRoomPMin, Security_OverheatTRoomPMin_Max);
	Security_RoomTRegMode_Max = (Security_OverheatTRoomPMin - 2);
	IfHigherThenEqual(Security_RoomTRegMode, Security_RoomTRegMode_Max);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _database_CorrectBurnerType
 *
 * @brief: Корректировка значений по умолчанию по изменению типа горелки
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _database_CorrectBurnerType(void)
{
	DB_t *DB = &_database->DB;
	switch (Service_Type) {
		case 0: {

		} break;
		case 1: {

		} break;
		case 2:  {

		} break;
		default: {
			// nop
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DB_Initialization
 *
 * @brief: Инициализация базы параметров и сигналов
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return указатель на базу
*/
DB_t *DB_Create(void)
{
	_database = (_database_t*)calloc(1, sizeof(_database_t));
	return &_database->DB;
}



