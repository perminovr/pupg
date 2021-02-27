/******************************************************************************
  * @file    database.h
  * @author  Перминов Р.И.
  * @version v0.0.1.0
  * @date    31.12.2018
  *****************************************************************************/

#ifndef DATABASE_H_
#define DATABASE_H_



#define uVT(t, v) (*((t*)(v)))



#define ERRORS_JOURNAL_SIZE		10
#define GSM_NUMBERS_IN_SIZE		3
#define GSM_NUMBERS_OUT_SIZE	3



typedef char string_t[16];



#define TELEPHONE_NUMBER_SIZE   			10
#define TELEPHONE_COUNTRY_CODE_SIZE   		5
#define FACTORY_NUMBER_SIZE     			11



#define DEFAULT_COUNTRY_CODE  		"+7"
#define DEFAULT_TELEPHONE_NUMBER  	"0000000000"
#define SECURITY_TRAY_REGULAR_T				35



#define STATISTIC_US_DIVIDER_200US		(u64)(200)
#define STATISTIC_US_DIVEDER_SEC		(int)((u64)1000000/STATISTIC_US_DIVIDER_200US)
#define STATISTIC_US_DIVEDER_MIN		(int)(STATISTIC_US_DIVEDER_SEC*60)



/* Положение датчиков на плате upg01 (4й на плате sense01) - проверено 20.12.18 */
#define DSW_SENSOR_SUPPLY_POSITION      0
#define DSW_SENSOR_RETURN_POSITION      1
#define DSW_SENSOR_ROOM_POSITION        2
#define DSW_SENSOR_OUTSIDE_POSITION     3
#define DSW_SENSOR_TRAY_POSITION      	4



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Sensors
 *
 * @brief: Индексы датчиков температуры 
 *
*/
typedef enum {
	Sensor_tStart = 0,
	Sensor_Supply = DSW_SENSOR_SUPPLY_POSITION,
	Sensor_Return = DSW_SENSOR_RETURN_POSITION,
	Sensor_Tray = DSW_SENSOR_TRAY_POSITION,
	Sensor_Room = DSW_SENSOR_ROOM_POSITION,
	Sensor_Outside = DSW_SENSOR_OUTSIDE_POSITION,
	Sensor_tEnd = 5
} Sensors;



/* **************************************** SIGNALS **************************************** */
// --------------------------------------------------------------------------------------------
// call SII-idxes.sh
// --------------------------------------------------------------------------------------------
#define SIGNALSII_SENSORSSTAT_START 		0
#define SIGNALSII_SENSORSSTAT_SIZE			7
#define Sensors_Statuses 					(DB->Signals_intI + SIGNALSII_SENSORSSTAT_START)  // @ref Sensor_tEnd
#define Sensors_SupplyStatus 				DB->Signals_intI[Sensor_Supply + SIGNALSII_SENSORSSTAT_START]
#define Sensors_ReturnStatus 				DB->Signals_intI[Sensor_Return + SIGNALSII_SENSORSSTAT_START]
#define Sensors_TrayStatus 					DB->Signals_intI[Sensor_Tray + SIGNALSII_SENSORSSTAT_START]
#define Sensors_RoomStatus 					DB->Signals_intI[Sensor_Room + SIGNALSII_SENSORSSTAT_START]
#define Sensors_OutsideStatus 				DB->Signals_intI[Sensor_Outside + SIGNALSII_SENSORSSTAT_START]
#define Sensors_FueljamState	 			DB->Signals_intI[5 + SIGNALSII_SENSORSSTAT_START]
#define Sensors_FlameState	 				DB->Signals_intI[6 + SIGNALSII_SENSORSSTAT_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_INSTSENSORSSTAT_START 	(SIGNALSII_SENSORSSTAT_START+SIGNALSII_SENSORSSTAT_SIZE)
#define SIGNALSII_INSTSENSORSSTAT_SIZE		5
#define Sensors_InstantStatuses 			(DB->Signals_intI + SIGNALSII_INSTSENSORSSTAT_START)  // @ref Sensor_tEnd
#define Sensors_InstantSupplyStatus 		DB->Signals_intI[Sensor_Supply + SIGNALSII_INSTSENSORSSTAT_START]
#define Sensors_InstantReturnStatus 		DB->Signals_intI[Sensor_Return + SIGNALSII_INSTSENSORSSTAT_START]
#define Sensors_InstantTrayStatus 			DB->Signals_intI[Sensor_Tray + SIGNALSII_INSTSENSORSSTAT_START]
#define Sensors_InstantRoomStatus 			DB->Signals_intI[Sensor_Room + SIGNALSII_INSTSENSORSSTAT_START]
#define Sensors_InstantOutsideStatus 		DB->Signals_intI[Sensor_Outside + SIGNALSII_INSTSENSORSSTAT_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_MODCMD_START				(SIGNALSII_INSTSENSORSSTAT_START+SIGNALSII_INSTSENSORSSTAT_SIZE)
#define SIGNALSII_MODCMD_SIZE				1
#define ModuleCmds_Statuses                 (DB->Signals_intI + SIGNALSII_MODCMD_START)
#define HMI_Cmd_Status                      DB->Signals_intI[0 + SIGNALSII_MODCMD_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_BURNER_START				(SIGNALSII_MODCMD_START+SIGNALSII_MODCMD_SIZE)
#define SIGNALSII_BURNER_SIZE				9
#define Burner_UserWaiting					DB->Signals_intI[0 + SIGNALSII_BURNER_START]
#define Burner_DynamicMode					DB->Signals_intI[1 + SIGNALSII_BURNER_START]
#define Burner_ScrewCmdAlg					DB->Signals_intI[2 + SIGNALSII_BURNER_START]
#define Burner_SparkPlugCmdAlg				DB->Signals_intI[3 + SIGNALSII_BURNER_START]
#define Burner_FanPAlg						DB->Signals_intI[4 + SIGNALSII_BURNER_START]
#define Burner_isRequestedStop        		DB->Signals_intI[5 + SIGNALSII_BURNER_START]
#define Burner_isSetROParams        		DB->Signals_intI[6 + SIGNALSII_BURNER_START]
#define Burner_CurrentDynamicOutput			DB->Signals_intI[7 + SIGNALSII_BURNER_START]
#define Burner_ColdStartState				DB->Signals_intI[8 + SIGNALSII_BURNER_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_OUTSIG_START				(SIGNALSII_BURNER_START+SIGNALSII_BURNER_SIZE)
#define SIGNALSII_OUTSIG_SIZE				9
#define OutSig_FanPOut 						DB->Signals_intI[0 + SIGNALSII_OUTSIG_START] // output
#define OutSig_ScrewState 					DB->Signals_intI[1 + SIGNALSII_OUTSIG_START]
#define OutSig_SparkPlugState 				DB->Signals_intI[2 + SIGNALSII_OUTSIG_START]
#define OutSig_TEHState 					DB->Signals_intI[3 + SIGNALSII_OUTSIG_START]
#define OutSig_ScrewCmd 					DB->Signals_intI[4 + SIGNALSII_OUTSIG_START] // input
#define OutSig_SparkPlugCmd 				DB->Signals_intI[5 + SIGNALSII_OUTSIG_START]
#define OutSig_TEHWorkPermission			DB->Signals_intI[6 + SIGNALSII_OUTSIG_START]
#define OutSig_TEHCmd 						DB->Signals_intI[7 + SIGNALSII_OUTSIG_START]
#define OutSig_FanP							DB->Signals_intI[8 + SIGNALSII_OUTSIG_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_TEMPPRM_START				(SIGNALSII_OUTSIG_START+SIGNALSII_OUTSIG_SIZE)
#define SIGNALSII_TEMPPRM_SIZE				6
#define TempPrm_ScrewP						DB->Signals_intI[0 + SIGNALSII_TEMPPRM_START]
#define TempPrm_FanP						DB->Signals_intI[1 + SIGNALSII_TEMPPRM_START]
#define TempPrm_SparkPlugCmd				DB->Signals_intI[2 + SIGNALSII_TEMPPRM_START]
#define TempPrm_TEHState					DB->Signals_intI[3 + SIGNALSII_TEMPPRM_START]
#define TempPrm_FanCmd						DB->Signals_intI[4 + SIGNALSII_TEMPPRM_START]
#define TempPrm_ScrewCmd					DB->Signals_intI[5 + SIGNALSII_TEMPPRM_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_GSM_START					(SIGNALSII_TEMPPRM_START+SIGNALSII_TEMPPRM_SIZE)
#define SIGNALSII_GSM_SIZE					6
#define GSM_State 							DB->Signals_intI[0 + SIGNALSII_GSM_START]
#define GSM_ModuleState 					DB->Signals_intI[1 + SIGNALSII_GSM_START]
#define GSM_Attached 						DB->Signals_intI[2 + SIGNALSII_GSM_START]
#define GSM_SignalLevel 					DB->Signals_intI[3 + SIGNALSII_GSM_START]
#define GSM_SMSRxCnt						DB->Signals_intI[4 + SIGNALSII_GSM_START]
#define GSM_SMSTxCnt						DB->Signals_intI[5 + SIGNALSII_GSM_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_STATISTIC_START			(SIGNALSII_GSM_START+SIGNALSII_GSM_SIZE)
#define SIGNALSII_STATISTIC_SIZE			14
#define Statistic_TimePultOn 				DB->Signals_intI[0 + SIGNALSII_STATISTIC_START]
#define Statistic_TimePUPGWork				DB->Signals_intI[1 + SIGNALSII_STATISTIC_START]
#define Statistic_TimeBurnerWork			DB->Signals_intI[2 + SIGNALSII_STATISTIC_START]
#define Statistic_TimeScrewWork				DB->Signals_intI[3 + SIGNALSII_STATISTIC_START]
#define Statistic_UNUSED1Us					uVT(u64, &DB->Signals_intI[4 + SIGNALSII_STATISTIC_START])
#define Statistic_TimeScrewWorkUs			uVT(u64, &DB->Signals_intI[6 + SIGNALSII_STATISTIC_START])
#define Statistic_TimeScrewWorkLvlCtlUs		uVT(u64, &DB->Signals_intI[8 + SIGNALSII_STATISTIC_START])
#define Statistic_TimeScrewWorkLvlCtl		DB->Signals_intI[10 + SIGNALSII_STATISTIC_START]
#define Statistic_TimeScrewWorkCumTtlUs		uVT(u64, &DB->Signals_intI[11 + SIGNALSII_STATISTIC_START])
#define Statistic_TimeScrewWorkCumTtl		DB->Signals_intI[13 + SIGNALSII_STATISTIC_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_SERVICE_START				(SIGNALSII_STATISTIC_START+SIGNALSII_STATISTIC_SIZE)
#define SIGNALSII_SERVICE_SIZE				5
#define Service_1wire_FrameCnts				(DB->Signals_intI + SIGNALSII_SERVICE_START)  // @ref Sensor_tEnd
#define Service_1wire_fc_1					DB->Signals_intI[0 + SIGNALSII_SERVICE_START]
#define Service_1wire_fc_2					DB->Signals_intI[1 + SIGNALSII_SERVICE_START]
#define Service_1wire_fc_3					DB->Signals_intI[2 + SIGNALSII_SERVICE_START]
#define Service_1wire_fc_4					DB->Signals_intI[3 + SIGNALSII_SERVICE_START]
#define Service_1wire_fc_5					DB->Signals_intI[4 + SIGNALSII_SERVICE_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_FUELCONS_START			(SIGNALSII_SERVICE_START+SIGNALSII_SERVICE_SIZE)
#define SIGNALSII_FUELCONS_SIZE				4
#define FuelCons_SWTime_Cnt					DB->Signals_intI[0 + SIGNALSII_FUELCONS_START]
#define FuelCons_SWTime_SumUs				uVT(u64, &DB->Signals_intI[1 + SIGNALSII_FUELCONS_START])
#define FuelCons_SWTime_Sum					DB->Signals_intI[3 + SIGNALSII_FUELCONS_START]
// --------------------------------------------------------------------------------------------
#define SIGNALSII_OTHER_START				(SIGNALSII_FUELCONS_START+SIGNALSII_FUELCONS_SIZE)
#define SIGNALSII_OTHER_SIZE				11
#define Security_WorkBlock 					DB->Signals_intI[0 + SIGNALSII_OTHER_START]
#define TempPrm_AutoignitionDelay			DB->Signals_intI[1 + SIGNALSII_OTHER_START]
#define TempPrm_ScrewPerfTestTime			DB->Signals_intI[2 + SIGNALSII_OTHER_START]
#define Burner_isNotDefStartState			DB->Signals_intI[3 + SIGNALSII_OTHER_START]
#define TempPrm_TTagetToMaxDelta			DB->Signals_intI[4 + SIGNALSII_OTHER_START]
#define TempPrm_TTagetToMinDelta			DB->Signals_intI[5 + SIGNALSII_OTHER_START]
#define HMI_Cmd								DB->Signals_intI[6 + SIGNALSII_OTHER_START]
#define GSM_Cmd								DB->Signals_intI[7 + SIGNALSII_OTHER_START]
#define PUPG_Cmd							DB->Signals_intI[8 + SIGNALSII_OTHER_START]
#define Burner_FuelLevelPerc 				DB->Signals_intI[9 + SIGNALSII_OTHER_START]
#define Burner_ChangedParam 				DB->Signals_intI[10 + SIGNALSII_OTHER_START]
//
#define SII_SIZE (SIGNALSII_OTHER_START + SIGNALSII_OTHER_SIZE)


#define Burner_CurrentState					DB->E.Signals_intIE[0]
#define AlgPrm_Tfixout						DB->E.Signals_intIE[1]
#define Security_Errors						(DB->E.Signals_intIE+2)	// @ref ERRORS_JOURNAL_SIZE
#define Signals_intIE_1BYTE_SIZE			(ERRORS_JOURNAL_SIZE+2)
#define Statistic_TimePUPGWorkE				DB->E.Signals_intIE[Signals_intIE_1BYTE_SIZE+0]
#define Statistic_TimeBurnerWorkE			DB->E.Signals_intIE[Signals_intIE_1BYTE_SIZE+1]
#define Statistic_TimeScrewWorkE			DB->E.Signals_intIE[Signals_intIE_1BYTE_SIZE+2]
#define Statistic_TimeScrewWorkLvlCtlE		DB->E.Signals_intIE[Signals_intIE_1BYTE_SIZE+3]
#define Statistic_TimeScrewWorkCumTtlE		DB->E.Signals_intIE[Signals_intIE_1BYTE_SIZE+4]
#define Signals_intIE_4BYTE_SIZE			5
//
#define Signals_intIE_4BYTE_START_IDX		(Signals_intIE_1BYTE_SIZE)
#define Signals_intIE_4BYTE_END_IDX			(Signals_intIE_4BYTE_START_IDX+Signals_intIE_4BYTE_SIZE-1)
//
#define SIE_SIZE (Signals_intIE_1BYTE_SIZE+Signals_intIE_4BYTE_SIZE)


#define Burner_ScrewPAlg					DB->Signals_floatI[0]
#define Sensors_Temperatures				(DB->Signals_floatI+1)  // @ref Sensor_tEnd
#define Sensors_SupplyT						DB->Signals_floatI[Sensor_Supply+1]
#define Sensors_ReturnT						DB->Signals_floatI[Sensor_Return+1]
#define Sensors_TrayT						DB->Signals_floatI[Sensor_Tray+1]
#define Sensors_RoomT						DB->Signals_floatI[Sensor_Room+1]
#define Sensors_OutsideT					DB->Signals_floatI[Sensor_Outside+1]
#define Sensors_FlameInstant				DB->Signals_floatI[6]
#define Sensors_FlameAverage				DB->Signals_floatI[7]
#define OutSig_ScrewP						DB->Signals_floatI[8]
#define GSM_Balance							DB->Signals_floatI[9]
#define Sensors_FueljamInstant				DB->Signals_floatI[10]
#define Sensors_FueljamAverage				DB->Signals_floatI[11]
#define Service_1wire_ErrorRates			(DB->Signals_floatI+12)  // @ref Sensor_tEnd
#define Service_1wire_er_1					DB->Signals_floatI[12]
#define Service_1wire_er_2					DB->Signals_floatI[13]
#define Service_1wire_er_3					DB->Signals_floatI[14]
#define Service_1wire_er_4					DB->Signals_floatI[15]
#define Service_1wire_er_5					DB->Signals_floatI[16]
#define OutSig_ScrewPOut					DB->Signals_floatI[17]
#define Burner_FuelLevel 					DB->Signals_floatI[18]
#define Statistic_CumulatTtlFuelCons		DB->Signals_floatI[19]
#define Statistic_FuelPerHr					DB->Signals_floatI[20]
#define Statistic_FuelPerDay				DB->Signals_floatI[21]
//
#define SFI_SIZE 22



/* ************************************** PARAMETERS ************************************** */
#define Service_Type						DB->E.Params_intHE[0]
#define Service_NoFlameLvl					DB->E.Params_intHE[1]
#define Service_StrtStpFanOffDelay			DB->E.Params_intHE[2]
#define Service_FanTurnChangeRate			DB->E.Params_intHE[3]
#define AlgPrm_WorkAlg						DB->E.Params_intHE[4]
#define AlgPrm_TTarget						DB->E.Params_intHE[5]
#define AlgPrm_TMin							DB->E.Params_intHE[6]
#define AlgPrm_TMax							DB->E.Params_intHE[7]
#define AlgPrm_FanPMin						DB->E.Params_intHE[8]
#define AlgPrm_FanPOpt						DB->E.Params_intHE[9]
#define AlgPrm_FanPMax						DB->E.Params_intHE[10]
#define AlgPrm_FanPFix						DB->E.Params_intHE[11]
#define AlgPrm_FanPWait						DB->E.Params_intHE[12]
#define AlgPrm_BlowTime						DB->E.Params_intHE[13]
#define AlgPrm_BlowPeriod					DB->E.Params_intHE[14]
#define AlgPrm_AutoignitionDelay			DB->E.Params_intHE[15]
#define Ignition_BlowTime					DB->E.Params_intHE[16]
#define Ignition_FuelWeight					DB->E.Params_intHE[17] 
#define Ignition_SparkPlugTime				DB->E.Params_intHE[18] 
#define Ignition_FanP						DB->E.Params_intHE[19]
#define Ignition_Repeat						DB->E.Params_intHE[20]
#define Ignition_FixingTime					DB->E.Params_intHE[21]
#define Ignition_FanPFixing					DB->E.Params_intHE[22]
#define Ignition_ToAutoignitionTime			DB->E.Params_intHE[23]
#define Ignition_FlameLvlBurn				DB->E.Params_intHE[24]
#define Ignition_FailBlowTime				DB->E.Params_intHE[25]
#define Service_TransModeTime				DB->E.Params_intHE[26]
#define Service_PTrans						DB->E.Params_intHE[27]
#define Service_AutoIgnFlameHyst			DB->E.Params_intHE[28]
#define Security_OverheatTSupPMin			DB->E.Params_intHE[29]
#define Security_OverheatTSupScrewStop		DB->E.Params_intHE[30]
#define Security_OverheatTRoomScrewStop		DB->E.Params_intHE[31]
#define Security_RoomTRegMode				DB->E.Params_intHE[32]
#define Security_OverheatTTrayPMin			DB->E.Params_intHE[33]
#define Security_OverheatTTrayScrewStop		DB->E.Params_intHE[34]
#define Security_FueljamBurnoutTime			DB->E.Params_intHE[35] 
#define Security_WaitModeDelay				DB->E.Params_intHE[36]
#define TEH_WorkPermission					DB->E.Params_intHE[37]
#define TEH_StateOnT						DB->E.Params_intHE[38]
#define GSM_Mode							DB->E.Params_intHE[39]
#define AlgPrm_BlowTimeRegStop				DB->E.Params_intHE[40]
#define GSM_Repeat				            DB->E.Params_intHE[41]
#define Service_TransModeFanP				DB->E.Params_intHE[42]
#define Service_FueljamPermission			DB->E.Params_intHE[43]
#define Security_OverheatTRoomPMin			DB->E.Params_intHE[44]
#define AlgPrm_StrtStpTTargetHyst			DB->E.Params_intHE[45]
#define Burner_FuelHopperCapacity			DB->E.Params_intHE[46]
#define Burner_FuelLowLevel					DB->E.Params_intHE[47]
#define Burner_FuelLowLevelNotif			DB->E.Params_intHE[48]
#define HMI_Locale							DB->E.Params_intHE[49]
#define AlgPrm_StartStopSensor				DB->E.Params_intHE[50]
//
#define PIH_SIZE 51


#define Service_PelletsThermCond			DB->E.Params_floatHE[0]
#define Service_ScrewPerform				DB->E.Params_floatHE[1]
#define Service_MaxPower					DB->E.Params_floatHE[2]
#define AlgPrm_PMin							DB->E.Params_floatHE[3]
#define AlgPrm_POpt							DB->E.Params_floatHE[4]
#define AlgPrm_PMax							DB->E.Params_floatHE[5]
#define AlgPrm_PFix							DB->E.Params_floatHE[6]
#define AlgPrm_PCapacity					DB->E.Params_floatHE[7]
#define AlgPrm_Pfixro						DB->E.Params_floatHE[8]
//
#define PFH_SIZE 9


#define Service_FactoryNumber				DB->E.Params_charIE_16[0]
//
#define PCI_SIZE 1


#define GSM_NumbersIn						(DB->E.Params_charHE_16+0)
#define GSM_NumbersOut						(DB->E.Params_charHE_16+GSM_NUMBERS_IN_SIZE)
#define GSM_TOTAL_NUMBERS 					(GSM_NUMBERS_IN_SIZE+GSM_NUMBERS_OUT_SIZE)
#define GSM_PREFIX_IDX 						(GSM_TOTAL_NUMBERS)
#define GSM_Prefix							DB->E.Params_charHE_16[GSM_TOTAL_NUMBERS+0]
//
#define PCH_SIZE (GSM_TOTAL_NUMBERS+1)



/* ************************************ PARAMETERS MIN ************************************ */
#define Service_Type_Min					DB->L.Params_intHE_Min[0]
#define Service_NoFlameLvl_Min				DB->L.Params_intHE_Min[1]
#define Service_StrtStpFanOffDelay_Min		DB->L.Params_intHE_Min[2]
#define Service_FanTurnChangeRate_Min		DB->L.Params_intHE_Min[3]
#define AlgPrm_WorkAlg_Min					DB->L.Params_intHE_Min[4]
#define AlgPrm_TTarget_Min					DB->L.Params_intHE_Min[5]
#define AlgPrm_TMin_Min						DB->L.Params_intHE_Min[6]
#define AlgPrm_TMax_Min						DB->L.Params_intHE_Min[7]
#define AlgPrm_FanPMin_Min					DB->L.Params_intHE_Min[8]
#define AlgPrm_FanPOpt_Min					DB->L.Params_intHE_Min[9]
#define AlgPrm_FanPMax_Min					DB->L.Params_intHE_Min[10]
#define AlgPrm_FanPFix_Min					DB->L.Params_intHE_Min[11]
#define AlgPrm_FanPWait_Min					DB->L.Params_intHE_Min[12]
#define AlgPrm_BlowTime_Min					DB->L.Params_intHE_Min[13]
#define AlgPrm_BlowPeriod_Min				DB->L.Params_intHE_Min[14]
#define AlgPrm_AutoignitionDelay_Min		DB->L.Params_intHE_Min[15]
#define Ignition_BlowTime_Min				DB->L.Params_intHE_Min[16]
#define Ignition_FuelWeight_Min				DB->L.Params_intHE_Min[17] 
#define Ignition_SparkPlugTime_Min			DB->L.Params_intHE_Min[18] 
#define Ignition_FanP_Min					DB->L.Params_intHE_Min[19]
#define Ignition_Repeat_Min					DB->L.Params_intHE_Min[20]
#define Ignition_FixingTime_Min				DB->L.Params_intHE_Min[21]
#define Ignition_FanPFixing_Min				DB->L.Params_intHE_Min[22]
#define Ignition_ToAutoignitionTime_Min		DB->L.Params_intHE_Min[23]
#define Ignition_FlameLvlBurn_Min			DB->L.Params_intHE_Min[24]
#define Ignition_FailBlowTime_Min			DB->L.Params_intHE_Min[25]
#define Service_TransModeTime_Min			DB->L.Params_intHE_Min[26]
#define Service_PTrans_Min					DB->L.Params_intHE_Min[27]
#define Service_AutoIgnFlameHyst_Min		DB->L.Params_intHE_Min[28]
#define Security_OverheatTSupPMin_Min		DB->L.Params_intHE_Min[29]
#define Security_OverheatTSupScrewStop_Min		DB->L.Params_intHE_Min[30]
#define Security_OverheatTRoomScrewStop_Min		DB->L.Params_intHE_Min[31]
#define Security_RoomTRegMode_Min				DB->L.Params_intHE_Min[32]
#define Security_OverheatTTrayPMin_Min		DB->L.Params_intHE_Min[33]
#define Security_OverheatTTrayScrewStop_Min	DB->L.Params_intHE_Min[34]
#define Security_FueljamBurnoutTime_Min		DB->L.Params_intHE_Min[35] 
#define Security_WaitModeDelay_Min			DB->L.Params_intHE_Min[36]
#define TEH_WorkPermission_Min				DB->L.Params_intHE_Min[37]
#define TEH_StateOnT_Min					DB->L.Params_intHE_Min[38]
#define GSM_Mode_Min						DB->L.Params_intHE_Min[39]
#define AlgPrm_BlowTimeRegStop_Min			DB->L.Params_intHE_Min[40]
#define GSM_Repeat_Min				        DB->L.Params_intHE_Min[41]
#define Service_TransModeFanP_Min			DB->L.Params_intHE_Min[42]
#define Service_FueljamPermission_Min		DB->L.Params_intHE_Min[43]
#define Security_OverheatTRoomPMin_Min		DB->L.Params_intHE_Min[44]
#define AlgPrm_StrtStpTTargetHyst_Min				DB->L.Params_intHE_Min[45]
#define Burner_FuelHopperCapacity_Min		DB->L.Params_intHE_Min[46]
#define Burner_FuelLowLevel_Min				DB->L.Params_intHE_Min[47]
#define Burner_FuelLowLevelNotif_Min		DB->L.Params_intHE_Min[48]
#define HMI_Locale_Min						DB->L.Params_intHE_Min[49]
#define AlgPrm_StartStopSensor_Min			DB->L.Params_intHE_Min[50]


#define TempPrm_ScrewP_Min					0
#define TempPrm_FanP_Min					0
#define TempPrm_SparkPlugCmd_Min			0
#define TempPrm_TEHState_Min				0


#define Service_PelletsThermCond_Min		DB->L.Params_floatHE_Min[0]
#define Service_ScrewPerform_Min			DB->L.Params_floatHE_Min[1]
#define Service_MaxPower_Min				DB->L.Params_floatHE_Min[2]
#define AlgPrm_PMin_Min						DB->L.Params_floatHE_Min[3]
#define AlgPrm_POpt_Min						DB->L.Params_floatHE_Min[4]
#define AlgPrm_PMax_Min						DB->L.Params_floatHE_Min[5]
#define AlgPrm_PFix_Min						DB->L.Params_floatHE_Min[6]
#define AlgPrm_PCapacity_Min				DB->L.Params_floatHE_Min[7]
#define AlgPrm_Pfixro_Min					DB->L.Params_floatHE_Min[8]



/* ************************************ PARAMETERS MAX ************************************ */
#define Service_Type_Max					DB->L.Params_intHE_Max[0]
#define Service_NoFlameLvl_Max				DB->L.Params_intHE_Max[1]
#define Service_StrtStpFanOffDelay_Max 		DB->L.Params_intHE_Max[2]
#define Service_FanTurnChangeRate_Max		DB->L.Params_intHE_Max[3]
#define AlgPrm_WorkAlg_Max					DB->L.Params_intHE_Max[4]
#define AlgPrm_TTarget_Max					DB->L.Params_intHE_Max[5]
#define AlgPrm_TMin_Max						DB->L.Params_intHE_Max[6]
#define AlgPrm_TMax_Max						DB->L.Params_intHE_Max[7]
#define AlgPrm_FanPMin_Max					DB->L.Params_intHE_Max[8]
#define AlgPrm_FanPOpt_Max					DB->L.Params_intHE_Max[9]
#define AlgPrm_FanPMax_Max					DB->L.Params_intHE_Max[10]
#define AlgPrm_FanPFix_Max					DB->L.Params_intHE_Max[11]
#define AlgPrm_FanPWait_Max					DB->L.Params_intHE_Max[12]
#define AlgPrm_BlowTime_Max					DB->L.Params_intHE_Max[13]
#define AlgPrm_BlowPeriod_Max				DB->L.Params_intHE_Max[14]
#define AlgPrm_AutoignitionDelay_Max		DB->L.Params_intHE_Max[15]
#define Ignition_BlowTime_Max				DB->L.Params_intHE_Max[16]
#define Ignition_FuelWeight_Max				DB->L.Params_intHE_Max[17] 
#define Ignition_SparkPlugTime_Max			DB->L.Params_intHE_Max[18] 
#define Ignition_FanP_Max					DB->L.Params_intHE_Max[19]
#define Ignition_Repeat_Max					DB->L.Params_intHE_Max[20]
#define Ignition_FixingTime_Max				DB->L.Params_intHE_Max[21]
#define Ignition_FanPFixing_Max				DB->L.Params_intHE_Max[22]
#define Ignition_ToAutoignitionTime_Max		DB->L.Params_intHE_Max[23]
#define Ignition_FlameLvlBurn_Max			DB->L.Params_intHE_Max[24]
#define Ignition_FailBlowTime_Max			DB->L.Params_intHE_Max[25]
#define Service_TransModeTime_Max			DB->L.Params_intHE_Max[26]
#define Service_PTrans_Max					DB->L.Params_intHE_Max[27]
#define Service_AutoIgnFlameHyst_Max		DB->L.Params_intHE_Max[28]
#define Security_OverheatTSupPMin_Max		DB->L.Params_intHE_Max[29]
#define Security_OverheatTSupScrewStop_Max		DB->L.Params_intHE_Max[30]
#define Security_OverheatTRoomScrewStop_Max		DB->L.Params_intHE_Max[31]
#define Security_RoomTRegMode_Max				DB->L.Params_intHE_Max[32]
#define Security_OverheatTTrayPMin_Max		DB->L.Params_intHE_Max[33]
#define Security_OverheatTTrayScrewStop_Max	DB->L.Params_intHE_Max[34]
#define Security_FueljamBurnoutTime_Max		DB->L.Params_intHE_Max[35]
#define Security_WaitModeDelay_Max			DB->L.Params_intHE_Max[36]
#define TEH_WorkPermission_Max				DB->L.Params_intHE_Max[37]
#define TEH_StateOnT_Max					DB->L.Params_intHE_Max[38]
#define GSM_Mode_Max						DB->L.Params_intHE_Max[39]
#define AlgPrm_BlowTimeRegStop_Max			DB->L.Params_intHE_Max[40]
#define GSM_Repeat_Max				        DB->L.Params_intHE_Max[41] 
#define Service_TransModeFanP_Max			DB->L.Params_intHE_Max[42]
#define Service_FueljamPermission_Max		DB->L.Params_intHE_Max[43]
#define Security_OverheatTRoomPMin_Max		DB->L.Params_intHE_Max[44]
#define AlgPrm_StrtStpTTargetHyst_Max				DB->L.Params_intHE_Max[45]
#define Burner_FuelHopperCapacity_Max		DB->L.Params_intHE_Max[46]
#define Burner_FuelLowLevel_Max				DB->L.Params_intHE_Max[47]
#define Burner_FuelLowLevelNotif_Max		DB->L.Params_intHE_Max[48]
#define HMI_Locale_Max						DB->L.Params_intHE_Max[49]
#define AlgPrm_StartStopSensor_Max			DB->L.Params_intHE_Max[50]


#define TempPrm_ScrewP_Max					99
#define TempPrm_FanP_Max					99
#define TempPrm_SparkPlugCmd_Max			1
#define TempPrm_TEHState_Max				1


#define Service_PelletsThermCond_Max		DB->L.Params_floatHE_Max[0]
#define Service_ScrewPerform_Max			DB->L.Params_floatHE_Max[1]
#define Service_MaxPower_Max				DB->L.Params_floatHE_Max[2]
#define AlgPrm_PMin_Max						DB->L.Params_floatHE_Max[3]
#define AlgPrm_POpt_Max						DB->L.Params_floatHE_Max[4]
#define AlgPrm_PMax_Max						DB->L.Params_floatHE_Max[5]
#define AlgPrm_PFix_Max						DB->L.Params_floatHE_Max[6]
#define AlgPrm_PCapacity_Max				DB->L.Params_floatHE_Max[7]
#define AlgPrm_Pfixro_Max					DB->L.Params_floatHE_Max[8]



/* *********************************** PARAMETERS STEPS *********************************** */
//#define Service_Type_Step					1
#define Service_NoFlameLvl_Step				1
#define Service_FanTurnChangeRate_Step		1
//#define AlgPrm_WorkAlg_Step				1
#define AlgPrm_Txxx_Step					1
#define AlgPrm_TTarget_Step					AlgPrm_Txxx_Step
#define AlgPrm_TMin_Step					AlgPrm_Txxx_Step
#define AlgPrm_TMax_Step					AlgPrm_Txxx_Step
#define AlgPrm_FanPxxx_Step					1
#define AlgPrm_FanPMin_Step					AlgPrm_FanPxxx_Step
#define AlgPrm_FanPOpt_Step					AlgPrm_FanPxxx_Step
#define AlgPrm_FanPMax_Step					AlgPrm_FanPxxx_Step
#define AlgPrm_FanPFix_Step					AlgPrm_FanPxxx_Step
#define AlgPrm_FanPWait_Step				AlgPrm_FanPxxx_Step
#define AlgPrm_BlowTime_Step				1
#define AlgPrm_BlowPeriod_Step				1
#define AlgPrm_AutoignitionDelay_Step		1
#define Ignition_BlowTime_Step				1
#define Ignition_FuelWeight_Step			1
#define Ignition_SparkPlugTime_Step			1
#define Ignition_FanP_Step					AlgPrm_FanPxxx_Step
//#define Ignition_Repeat_Step				1
#define Ignition_FixingTime_Step			1
#define Ignition_FanPFixing_Step			AlgPrm_FanPxxx_Step
#define Ignition_ToAutoignitionTime_Step	1
#define Ignition_FlameLvlBurn_Step			1
#define Ignition_FailBlowTime_Step			1
#define Service_TransModeTime_Step			1
#define Service_PTrans_Step					1
#define Service_AutoIgnFlameHyst_Step		1
#define Security_OverheatTSupPMin_Step		1
#define Security_OverheatTSupScrewStop_Step		1
#define Security_OverheatTRoomScrewStop_Step	1
#define Security_RoomTRegMode_Step				1
#define Security_OverheatTTrayPMin_Step			1
#define Security_OverheatTTrayScrewStop_Step	1
#define Security_FueljamBurnoutTime_Step		1
#define Security_WaitModeDelay_Step			1
//#define TEH_WorkPermission_Step			1
#define TEH_StateOnT_Step					1
//#define GSM_Mode_Step						1
#define AlgPrm_BlowTimeRegStop_Step			1
//#define GSM_Repeat_Step                   1
#define Service_TransModeFanP_Step			1
//#define Service_FueljamPermission_Step		1
#define Security_OverheatTRoomPMin_Step		1
#define AlgPrm_StrtStpTTargetHyst_Step				1
#define Service_StrtStpFanOffDelay_Step		1
#define Burner_FuelHopperCapacity_Step		5
#define Burner_FuelLowLevel_Step			5
#define Burner_FuelLowLevelNotif_Step		1
#define HMI_Locale_Step						1
#define AlgPrm_StartStopSensor_Step			1


#define Service_PelletsThermCond_Step		0.05
#define Service_ScrewPerform_Step			0.1
#define Service_MaxPower_Step				1.0
#define AlgPrm_Pxxx_Step					0.5
#define AlgPrm_PMin_Step					AlgPrm_Pxxx_Step
#define AlgPrm_POpt_Step					AlgPrm_Pxxx_Step
#define AlgPrm_PMax_Step					AlgPrm_Pxxx_Step
#define AlgPrm_PFix_Step					AlgPrm_Pxxx_Step
//#define AlgPrm_Pfixro_Step				1


#define TempPrm_ScrewP_Step					1
#define TempPrm_FanP_Step					AlgPrm_FanPxxx_Step
//#define TempPrm_SparkPlugCmd_Step			1
//#define TempPrm_TEHState_Step				1



// -------------------------------------------------------------------------------------------------------------------
#define SIGNALS_INTI_SIZE		SII_SIZE
#define SIGNALS_INTIE_SIZE		SIE_SIZE
#define SIGNALS_FLOATI_SIZE		SFI_SIZE
#define PARAMS_INTHE_SIZE		PIH_SIZE
#define PARAMS_FLOATHE_SIZE		PFH_SIZE
#define PARAMS_CHARHE_SIZE		PCH_SIZE
#define PARAMS_CHARIE_SIZE		PCI_SIZE
//
#define DEBUG_INTH_SIZE		    40
#define DEBUG_FLOATH_SIZE       9
#define DEBUG_FLOATI_SIZE       7



typedef struct {
	int Signals_intIE[SIGNALS_INTIE_SIZE];
	int Params_intHE[PARAMS_INTHE_SIZE];
	float Params_floatHE[PARAMS_FLOATHE_SIZE];
	string_t Params_charHE_16[PARAMS_CHARHE_SIZE];
	string_t Params_charIE_16[PARAMS_CHARIE_SIZE];
} DBE_t;



typedef struct {
	int Params_intHE_Min[PARAMS_INTHE_SIZE];
	int Params_intHE_Max[PARAMS_INTHE_SIZE];
	float Params_floatHE_Min[PARAMS_FLOATHE_SIZE];
	float Params_floatHE_Max[PARAMS_FLOATHE_SIZE];
} DBL_t;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: DB_t
 *
 * @brief: База сигналов и параметров
 *
*/
typedef struct {
    DBE_t E;
    DBL_t L;
	int Signals_intI[SIGNALS_INTI_SIZE];
    int Debug_intH[DEBUG_INTH_SIZE];
	float Signals_floatI[SIGNALS_FLOATI_SIZE];
    float Debug_floatH[DEBUG_FLOATH_SIZE];
    float Debug_floatI[DEBUG_FLOATI_SIZE];
} DB_t;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: BurnerStates
 *
 * @brief: Состояния горелки
 *
*/
typedef enum {
	BurnerState_reserv1,
	BurnerState_reserv2,
	BurnerState_Starting,
	BurnerState_Error,			// 3
	BurnerState_Waiting,
	BurnerState_ColdStart,		// 5
	BurnerState_RegularStop,
	BurnerState_RegularMode,	// 8
	BurnerState_TransMode		// 9
} BurnerStates;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ColdStartStates
 *
 * @brief: Состояния холодного старта: побито на три попытки розжига для отображения на экране
 *
*/
typedef enum {
	ColdStartState_Empty,
	ColdStartState_Ignition1,
	ColdStartState_Ignition2,
	ColdStartState_Ignition3,
	ColdStartState_Ignition4,
	ColdStartState_BlowOut,
} ColdStartStates;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Algorithms
 *
 * @brief: Алгоритмы штатной работы горелки
 *
*/
typedef enum {
	Algorithm_Return = 1,
	Algorithm_Room,
	Algorithm_RoomOutside,
	Algorithm_FixedPower,
	Algorithm_ConfigRoomOut,
	Algorithm_StartStop,
} Algorithms;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: SensorsTStatuses
 *
 * @brief: Статусы датчиков 1-wire
 *
*/
typedef enum {
	SensorsTStatus_OK,		/* ОК */	
	SensorsTStatus_Off,		/* Датчик отключен */
	SensorsTStatus_PresErr,	/* Датчик отсутствует */
	SensorsTStatus_CrcErr,	/* Ошибка CRC */
	SensorsTStatus_tEnd,
} SensorsTStatuses;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ModulesCmdStatus
 *
 * @brief: Статусы команд от модулей
 *
*/
typedef enum {
	ModCmdSts_OK,
	ModCmdSts_FAIL = 1,
    ModCmdSts_RESET = 1,
} ModulesCmdStatus;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: HMICommands
 *
 * @brief: Команды от модуля HMI
 *
*/
typedef enum {
	HMICmd_No,				/* нет команды */
	HMICmd_Start,			/* старт горелки */
	HMICmd_Stop,			/* стоп горелки */
	HMICmd_ClearErrors,		/* очистка журнала ошибок */
	HMICmd_ClearSettings,	/* сброс настроек */
	HMICmd_CorrectMinMax,	/* корректировка миним и макс значений */
	HMICmd_CorrectTTarget,	/* корректировка температур перехода на мин и макс от целевой (+ отработка @ref HMICmd_CorrectMinMax) */
	HMICmd_CorrectTDeltas,	/* корректировка допустимых отклонений от целевой температуры */
	HMICmd_GetBalance,		/* запрос баланса */
	HMICmd_GetSignalLvl,	/* уровень сигнала GSM */
	HMICmd_ChangeBurnerType,	/* изменение типа горелки */
	HMICmd_ResetFuelConsCumTtl, /* сброс нарастающего итога */
	HMICmd_HopperFull, 		/* бункер загружен */
	HMICmd_Reboot, 		/* запрос перезагрузки модуля */
} HMICommands;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: PUPGCommands
 *
 * @brief: Команды модуля
 *
*/
typedef enum {
	PUPGCmd_No,				/* нет команды */
	PUPGCmd_Reboot, 		/* команда на перезагрузку */
} PUPGCommands;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GSMCommand
 *
 * @brief: Команды с модуля GSM
 * 
 * NOTE: порядок важен
 *
*/
typedef enum {    
	GSMCmd_tStart = 0,			
	GSMCmd_No = 0,		    
	GSMCmd_Start,		/* старт горелки */
	GSMCmd_Stop,		/* стоп горелки */
    GSMCmd_CorrectTTarget,	/* корректировка температур перехода на мин и макс от целевой (+ отработка @ref HMICmd_CorrectMinMax) */
	GSMCmd_tEnd,
} GSMCommand;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GSMState
 *
 * @brief: Состояния модуля GSM
 *
*/
typedef enum {
	GSMSt_Off,
	GSMSt_Initialization,
	GSMSt_Work,
} GSMState;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: DynamicOutputs
 *
 * @brief: Параметры, устанавливаемые динамически
 *
*/
typedef enum {
	DynamicOutput_FanP,			/* регулируемая мощность */
	DynamicOutput_FanCmd,		/* 100% мощность */
	DynamicOutput_ScrewP,		/* регулируемая мощность */
	DynamicOutput_ScrewCmd,		/* 100% мощность */
	DynamicOutput_SparkPlug,	/* включение лампы */
	DynamicOutput_TEH			/* включение ТЭН */
} DynamicOutputs;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: WorkBlock
 *
 * @brief: Режимы блокировки модулем безопасности
 *
*/
typedef enum {
	WorkBlock_No,		/* нет блока */
	WorkBlock_Regular,	/* штатная */
	WorkBlock_Fatal,	/* фатальная ошибка */
} WorkBlock;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Locales_e
 *
*/
typedef enum {
	Locale_Rus,
	Locale_Eng,
	Locale_tEnd,
} Locales_e;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: StartStopSensor_e
 *
 * @brief: Выбор датчика в режиме работы старт-стоп
 *
*/
typedef enum {
	StartStopSensor_Supply,
	StartStopSensor_Room,
	StartStopSensor_tEnd,
} StartStopSensor_e;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ObservedParams_e
 *
 * @brief: Отслеживаемые по изменению параметры базы
 *
*/
typedef enum {
	ObservedParam_tStart,
	ObservedParam_Burner_CurrentState = ObservedParam_tStart,
	ObservedParam_AlgPrm_WorkAlg,
	ObservedParam_TempPrm_ScrewCmd,
	ObservedParam_tEnd,
} ObservedParams_e;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: IndexOf
 *
 * @brief: Индексация в базе
 *
*/
#define IndexOfSII(x) (&(x) - &(DB->Signals_intI[0]))
#define IndexOfSFI(x) (&(x) - &(DB->Signals_floatI[0]))
#define IndexOfSIE(x) (&(x) - &(DB->E.Signals_intIE[0]))
//
#define IndexOfPFH(x) (&(x) - &(DB->E.Params_floatHE[0]))
#define IndexOfPCH(x) (&(x) - &(DB->E.Params_charHE[0]))
#define IndexOfPCI(x) (&(x) - &(DB->E.Params_charI[0]))
#define IndexOfPIH(x) (&(x) - &(DB->E.Params_intHE[0]))



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
extern void DB_Work(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DB_Initialization
 *
 * @brief: Чтение параметров из ЭНП, инициализация минимумов и максимумов
 *
 * NOTE: Cоздает и инициализирует ErrorManager
 *
 * input parameters
 *
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int DB_Initialization(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: DB_Create
 *
 * @brief: Создание экземпляра DB
 *
 * input parameters
 *
 * output parameters
 *
 * return указатель на базу сигналов и параметров
*/
extern DB_t *DB_Create(void);




#endif /* DATABASE_H_ */