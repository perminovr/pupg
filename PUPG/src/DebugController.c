/******************************************************************************
  * @file    DebugController.h
  * @author  Перминов Р.И.
  * @version v0.0.1.0
  * @date    31.12.2018
  *****************************************************************************/


#include "DebugController.h"
#include "retain.h"
#include "string.h"
#include "PUPGutils.h"



/* **************************************** DEBUG ***************************************** */
#define DBG_Enable								DB->Debug_intH[0]
//#define 							DB->Debug_intH[1]
//#define 							DB->Debug_intH[2]
#define DBG_Sensors_InstantStatuses 			(DB->Debug_intH+3)  // @ref Sensor_tEnd
#define DBG_Sensors_InstantSupplyStatus 		DB->Debug_intH[Sensor_Supply+3]
#define DBG_Sensors_InstantReturnStatus 		DB->Debug_intH[Sensor_Return+3]
#define DBG_Sensors_InstantTrayStatus 			DB->Debug_intH[Sensor_Tray+3]
#define DBG_Sensors_InstantRoomStatus 			DB->Debug_intH[Sensor_Room+3]
#define DBG_Sensors_InstantOutsideStatus 		DB->Debug_intH[Sensor_Outside+3]
#define DBG_Sensors_FueljamState	 				DB->Debug_intH[8]
#define DBG_Burner_DynamicMode					DB->Debug_intH[9]
#define DBG_TempPrm_ScrewP						DB->Debug_intH[10]
#define DBG_TempPrm_FanP						DB->Debug_intH[11]
#define DBG_TempPrm_SparkPlugCmd				DB->Debug_intH[12]
#define DBG_TempPrm_TEHState					DB->Debug_intH[13]
#define DBG_GSM_ModuleState 					DB->Debug_intH[14]
#define DBG_GSM_Attached 						DB->Debug_intH[15]
#define DBG_Burner_CurrentDynamicOutput			DB->Debug_intH[16]
#define DBG_TempPrm_FanCmd						DB->Debug_intH[17]
#define DBG_Cmd									DB->Debug_intH[18]
#define DBG_TempPrm_ScrewCmd					DB->Debug_intH[19]
#define DBG_Service_FueljamPermission			DB->Debug_intH[20]


#define DBG_Sensors_Temperatures				(DB->Debug_floatH+0)  // @ref Sensor_tEnd
#define DBG_Sensors_SupplyT						DB->Debug_floatH[Sensor_Supply]
#define DBG_Sensors_ReturnT						DB->Debug_floatH[Sensor_Return]
#define DBG_Sensors_TrayT						DB->Debug_floatH[Sensor_Tray]
#define DBG_Sensors_RoomT						DB->Debug_floatH[Sensor_Room]
#define DBG_Sensors_OutsideT					DB->Debug_floatH[Sensor_Outside]
#define DBG_Sensors_FlameInstant				DB->Debug_floatH[5]
#define DBG_Sensors_FlameAverage				DB->Debug_floatH[6]
#define DBG_Sensors_FueljamInstant				DB->Debug_floatH[7]
#define DBG_Sensors_FueljamAverage				DB->Debug_floatH[8]


#define DBG_Sensors_Temperatures_R				(DB->Debug_floatI+0)  // @ref Sensor_tEnd
#define DBG_Sensors_SupplyT_R					DB->Debug_floatI[Sensor_Supply]
#define DBG_Sensors_ReturnT_R					DB->Debug_floatI[Sensor_Return]
#define DBG_Sensors_TrayT_R						DB->Debug_floatI[Sensor_Tray]
#define DBG_Sensors_RoomT_R						DB->Debug_floatI[Sensor_Room]
#define DBG_Sensors_OutsideT_R					DB->Debug_floatI[Sensor_Outside]
#define DBG_Sensors_FlameInstant_R				DB->Debug_floatI[5]
#define DBG_Sensors_FueljamInstant_R				DB->Debug_floatI[6]



typedef union {
	float f;
	u32 d;
} float_u32_t;



static DB_t *DB;



static void _DebugController_SetTestingParams(void);



#define SetParam(a,b) { \
	if (b >= 0) {		\
		a = b;			\
	}					\
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DebugController_WorkGeneral
 *
 * @brief: общие установки
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _DebugController_WorkGeneral(void)
{
	if (DBG_Enable == 222) {
		switch (DBG_Cmd) {
			case DBGCmd_WDT: {
				wdt_mode(wmReset);
			} break;
			case DBGCmd_ClearEeprom: {
				while (Retain_GetState() == Retain_Busy)
						;
				if (Retain_ClearDB() == Retain_Busy) {
					Retain_State state = Retain_Busy;
					while ( state == Retain_Busy ) {
						state = Retain_GetState();
					}
				}
			} break;
			case DBGCmd_SetTestingParams: {
				_DebugController_SetTestingParams();
			} break;
			default: {/* nop */} break;
		}
		
		if ( DBG_Cmd != DBGCmd_No ) {
			DBG_Enable = 0;
			DBG_Cmd = DBGCmd_No;
		}
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _DebugController_Read
 *
 * @brief: чтение сигналов
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _DebugController_Read(void)
{
    if (!DB) return;

	for (Sensors s = Sensor_tStart; s != Sensor_tEnd; ++s) {
		SetParam(DBG_Sensors_Temperatures_R[s], Sensors_Temperatures[s]);
	}
	SetParam(DBG_Sensors_FlameInstant_R, Sensors_FlameInstant);
	SetParam(DBG_Sensors_FueljamInstant_R, Sensors_FueljamInstant);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DebugController_WorkIn
 *
 * @brief: установки входных сигналов
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void DebugController_WorkIn(void)
{
    if (!DB) return;

	_DebugController_WorkGeneral();

	_DebugController_Read();

    if (DBG_Enable == 222) {
		for (Sensors s = Sensor_tStart; s != Sensor_tEnd; ++s) {
			SetParam(Sensors_InstantStatuses[s], DBG_Sensors_InstantStatuses[s]);
			SetParam(Sensors_Temperatures[s], DBG_Sensors_Temperatures[s]);
		}
		SetParam(Sensors_FueljamState, DBG_Sensors_FueljamState);
		SetParam(GSM_ModuleState, DBG_GSM_ModuleState);
		SetParam(GSM_Attached, DBG_GSM_Attached);
		SetParam(Sensors_FlameInstant, DBG_Sensors_FlameInstant);
		SetParam(Sensors_FlameAverage, DBG_Sensors_FlameAverage);
		SetParam(Sensors_FueljamInstant, DBG_Sensors_FueljamInstant);
		SetParam(Sensors_FueljamAverage, DBG_Sensors_FueljamAverage);
		SetParam(Service_FueljamPermission, DBG_Service_FueljamPermission);

		float_u32_t tmp;
		tmp.d = 0xFFFFFFFF;
		if (AlgPrm_PCapacity != tmp.f) {
			Burner_isSetROParams = 1;
		}
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DebugController_WorkOut
 *
 * @brief: установки выходных сигналов
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void DebugController_WorkOut(void)
{
    if (!DB) return;

    if (DBG_Enable == 222) {
		SetParam(Burner_DynamicMode, DBG_Burner_DynamicMode);
		SetParam(Burner_CurrentDynamicOutput, DBG_Burner_CurrentDynamicOutput);
		SetParam(TempPrm_ScrewP, DBG_TempPrm_ScrewP);
		SetParam(TempPrm_FanP, DBG_TempPrm_FanP);
		SetParam(TempPrm_SparkPlugCmd, DBG_TempPrm_SparkPlugCmd);
		SetParam(TempPrm_TEHState, DBG_TempPrm_TEHState);
		SetParam(TempPrm_FanCmd, DBG_TempPrm_FanCmd);
		SetParam(TempPrm_ScrewCmd, DBG_TempPrm_ScrewCmd);
    }
}



static void _DebugController_SetTestingParams(void) 
{
	int Params_intHE_Tst[PARAMS_INTHE_SIZE];
	float Params_floatHE_Tst[PARAMS_FLOATHE_SIZE];
	
	/* ********************************** PARAMETERS DEFAULT ********************************** */
	#define Service_Type_Tst						Params_intHE_Tst[0]
	#define Service_NoFlameLvl_Tst					Params_intHE_Tst[1]
	// #define 						Params_intHE_Tst[2]
	#define Service_FanTurnChangeRate_Tst			Params_intHE_Tst[3]
	#define AlgPrm_WorkAlg_Tst						Params_intHE_Tst[4]
	#define AlgPrm_TTarget_Tst						Params_intHE_Tst[5]
	#define AlgPrm_TMin_Tst							Params_intHE_Tst[6]
	#define AlgPrm_TMax_Tst							Params_intHE_Tst[7]
	#define AlgPrm_FanPMin_Tst						Params_intHE_Tst[8]
	#define AlgPrm_FanPOpt_Tst						Params_intHE_Tst[9]
	#define AlgPrm_FanPMax_Tst						Params_intHE_Tst[10]
	#define AlgPrm_FanPFix_Tst						Params_intHE_Tst[11]
	#define AlgPrm_FanPWait_Tst						Params_intHE_Tst[12]
	#define AlgPrm_BlowTime_Tst						Params_intHE_Tst[13]
	#define AlgPrm_BlowPeriod_Tst					Params_intHE_Tst[14]
	#define AlgPrm_AutoignitionDelay_Tst			Params_intHE_Tst[15]
	#define Ignition_BlowTime_Tst					Params_intHE_Tst[16]
	#define Ignition_FuelWeight_Tst					Params_intHE_Tst[17]
	#define Ignition_SparkPlugTime_Tst				Params_intHE_Tst[18]
	#define Ignition_FanP_Tst						Params_intHE_Tst[19]
	#define Ignition_Repeat_Tst						Params_intHE_Tst[20]
	#define Ignition_FixingTime_Tst					Params_intHE_Tst[21]
	#define Ignition_FanPFixing_Tst					Params_intHE_Tst[22]
	#define Ignition_ToAutoignitionTime_Tst			Params_intHE_Tst[23]
	#define Ignition_FlameLvlBurn_Tst				Params_intHE_Tst[24]
	#define Ignition_FailBlowTime_Tst				Params_intHE_Tst[25]
	#define Service_TransModeTime_Tst				Params_intHE_Tst[26]
	#define Service_PTrans_Tst						Params_intHE_Tst[27]
	// #define 						Params_intHE_Tst[28]
	#define Security_OverheatTSupPMin_Tst			Params_intHE_Tst[29]
	#define Security_OverheatTSupScrewStop_Tst		Params_intHE_Tst[30]
	#define Security_OverheatTRoomScrewStop_Tst		Params_intHE_Tst[31]
	#define Security_RoomTRegMode_Tst				Params_intHE_Tst[32]
	#define Security_OverheatTTrayPMin_Tst			Params_intHE_Tst[33]
	#define Security_OverheatTTrayScrewStop_Tst		Params_intHE_Tst[34]
	#define Security_FueljamBurnoutTime_Tst			Params_intHE_Tst[35]
	#define Security_WaitModeDelay_Tst				Params_intHE_Tst[36]
	#define TEH_WorkPermission_Tst					Params_intHE_Tst[37]
	#define TEH_StateOnT_Tst						Params_intHE_Tst[38]
	#define GSM_Mode_Tst							Params_intHE_Tst[39]
	#define AlgPrm_BlowTimeRegStop_Tst				Params_intHE_Tst[40]
	#define GSM_Repeat_Tst				            Params_intHE_Tst[41]
	#define Service_TransModeFanP_Tst				Params_intHE_Tst[42]
	#define Service_FueljamPermission_Tst			Params_intHE_Tst[43]
	#define Security_OverheatTRoomPMin_Tst			Params_intHE_Tst[44]

	#define Service_PelletsThermCond_Tst			Params_floatHE_Tst[0]
	#define Service_ScrewPerform_Tst				Params_floatHE_Tst[1]
	#define Service_MaxPower_Tst					Params_floatHE_Tst[2]	
	#define AlgPrm_PMin_Tst							Params_floatHE_Tst[3]	
	#define AlgPrm_POpt_Tst							Params_floatHE_Tst[4]	
	#define AlgPrm_PMax_Tst							Params_floatHE_Tst[5]	
	#define AlgPrm_PFix_Tst							Params_floatHE_Tst[6]	
	#define AlgPrm_PCapacity_Tst					Params_floatHE_Tst[7]
	#define AlgPrm_Pfixro_Tst						Params_floatHE_Tst[8]
	
	/* Значения для тестирования ----------------------------------- */
	Service_Type_Tst = 2; 			
	Service_NoFlameLvl_Tst = 20;
	Service_FanTurnChangeRate_Tst = 10;
	AlgPrm_WorkAlg_Tst = 1;
	AlgPrm_TTarget_Tst = 25;
	AlgPrm_TMin_Tst = 20;
	AlgPrm_TMax_Tst = 30;
	AlgPrm_FanPWait_Tst = 11;
	AlgPrm_BlowTime_Tst = 5;
	AlgPrm_BlowPeriod_Tst = 5;
	AlgPrm_AutoignitionDelay_Tst = 0;
	Ignition_BlowTime_Tst = 10;
	Ignition_SparkPlugTime_Tst = 60;
	Ignition_FanP_Tst = 70;
	Ignition_Repeat_Tst = 1;
	Ignition_FixingTime_Tst = 10;
	Ignition_FanPFixing_Tst = 75;
	Ignition_ToAutoignitionTime_Tst = 15;
	Ignition_FlameLvlBurn_Tst = 50;
	Ignition_FailBlowTime_Tst = 1;
	Service_TransModeTime_Tst = 1;
	Service_PTrans_Tst = 7;
	Security_OverheatTSupPMin_Tst = 35;
	Security_OverheatTSupScrewStop_Tst = 45;
	Security_OverheatTRoomScrewStop_Tst = 35;
	Security_RoomTRegMode_Tst = 25;
	Security_OverheatTTrayPMin_Tst = 35;
	Security_OverheatTTrayScrewStop_Tst = 45;
	Security_FueljamBurnoutTime_Tst = 30;
	Security_WaitModeDelay_Tst = 10;
	TEH_WorkPermission_Tst = 0;
	TEH_StateOnT_Tst = 15;
	GSM_Mode_Tst = 0;
	AlgPrm_BlowTimeRegStop_Tst = 2;
    GSM_Repeat_Tst = 0;
	Service_TransModeFanP_Tst = 30;
	Service_FueljamPermission_Tst = 1;
	Security_OverheatTRoomPMin_Tst = 30;
	//
	Service_PelletsThermCond_Tst = 4.0;
	Service_ScrewPerform_Tst = 16.2;

	float_u32_t tmp;
	tmp.d = 0xFFFFFFFF;
	AlgPrm_PCapacity_Tst = tmp.f;
	AlgPrm_Pfixro_Tst = tmp.f;
    
	// Params_intHE
	memcpy(DB->E.Params_intHE, Params_intHE_Tst, sizeof(int)*PARAMS_INTHE_SIZE);

	// Params_floatHE
	memcpy(DB->E.Params_floatHE, Params_floatHE_Tst, sizeof(float)*PARAMS_FLOATHE_SIZE);
	
	Service_Type = Service_Type_Tst;

	switch (Service_Type) {
		case 0: {
			Service_MaxPower = 0.0;
			Ignition_FuelWeight = 0;
			AlgPrm_PMin = 0.0;
			AlgPrm_POpt = 0.0;
			AlgPrm_PMax = 0.0;
			AlgPrm_PFix = 0.0;
			AlgPrm_FanPMin = 25;
			AlgPrm_FanPOpt = 70;
			AlgPrm_FanPMax = 99;
			AlgPrm_FanPFix = 10;
		} break;
		case 1: {
			Service_MaxPower = 26.0;
			Ignition_FuelWeight = 200;
			AlgPrm_PMin = 3.0;
			AlgPrm_POpt = 20.0;
			AlgPrm_PMax = 26.0;
			AlgPrm_PFix = 26.0;
			AlgPrm_FanPMin = 15;
			AlgPrm_FanPOpt = 20;
			AlgPrm_FanPMax = 35;
			AlgPrm_FanPFix = 20;
		} break;
		case 2:  {
			Service_MaxPower = 42.0;
			Ignition_FuelWeight = 300;
			AlgPrm_PMin = 3.0;
			AlgPrm_POpt = 37.0;
			AlgPrm_PMax = 42.0;
			AlgPrm_PFix = 21.0;
			AlgPrm_FanPMin = 15;
			AlgPrm_FanPOpt = 25;
			AlgPrm_FanPMax = 50;
			AlgPrm_FanPFix = 25;
		} break;
		default: {
			// nop
		} break;
	}

	// reset states
	Burner_CurrentState = (Burner_CurrentState != BurnerState_Error)?
			BurnerState_Waiting : BurnerState_Error;			
	Burner_FanPAlg = 0;
	Burner_ScrewPAlg = 0;
	Burner_ScrewCmdAlg = 0;
	Burner_SparkPlugCmdAlg = 0;	
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DebugController_Initialization
 * 
 * @brief: Инициализация DebugController
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
int DebugController_Initialization(DB_t *pDB)
{
    if (!pDB) return 1;
    DB = pDB;

	for (int i = 0; i < DEBUG_INTH_SIZE; ++i) {
		DB->Debug_intH[i] = -1;
	}
	for (int i = 0; i < DEBUG_FLOATH_SIZE; ++i) {
		DB->Debug_floatH[i] = -1.0;
	}

    return 0;
}