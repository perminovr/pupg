/******************************************************************************
  * @file    HMI.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/
  
  
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include "HMI.h"  
#include "ErrorManager.h"
#include "servto.h"
#include "ButtonsHandler.h"
#include "weh1602.h"



#define TEST_ERROR_CODE				99
#define TEST_KEYGROUP_IN_CNT		2
#define TEST_KEYGROUP_OUT_CNT		3


#define _HMI_TIME_TO_HANDLE_PRESSED_BUTTON		2000


#define _HMI_SECOND_DELAY			1000000
#define _HMI_MINUTE_DELAY			(60 * _HMI_SECOND_DELAY)
#define _HMI_KEYBOARD_TO			10000
#define _HMI_GETSIGNALLVL_TO        (10 * _HMI_SECOND_DELAY)
#define _HMI_CMDSYMBLINK_TO			300000
#define _HMI_SCREWPERFTEST_TO		(360)


#define _HMI_BUTTON_CMD 			Button_OK
#define _HMI_BUTTON_CMD_POS			ButtonPos_OK



static const int _HMI_ERROR_BLINK_ON = (1 * _HMI_SECOND_DELAY);
static const int _HMI_ERROR_BLINK_OFF = (1 * _HMI_SECOND_DELAY);
static const int _HMI_BUTTON_TO_ERROR_BLINK = (2 * _HMI_SECOND_DELAY);
static const int _HMI_CNT_START_TO_DISPLAY_OFF = (_HMI_MINUTE_DELAY / _HMI_KEYBOARD_TO);



enum {
    TreeID_Load,
    TreeID_Main,
    TreeID_Service,
    TreeID_Alert,
	TreeID_eepromError,
};
 
 
typedef enum {
    ScrID_Load,    
    ScrID_Main1,
    ScrID_Main2,
    ScrID_Main3,
    ScrID_Main4,
	ScrID_Cmd,    
    ScrID_Test,
	ScrID_Test_CurrentState,
	ScrID_Test_ScrewCurrentPower,
	ScrID_Test_FanCurrentPower,
    ScrID_Test_SparkPlugCurrentState,
	ScrID_Test_Flame,
    ScrID_Test_Fueljam,
    ScrID_Test_TSup,
    ScrID_Test_TRet,
    ScrID_Test_TRoom,
    ScrID_Test_TOutside,
    ScrID_Test_TTray,
    ScrID_Test_TEHCurrentState,
	ScrID_Test_Control,	
	ScrID_Test_ScrewCtrl,
	ScrID_Test_FanCtrl,
	ScrID_Test_SparkPlugCtrl,
	ScrID_Test_TEHCtrl,
	ScrID_Test_ErrorCtrl,
	ScrID_Test_ScrewPerf,
	ScrID_Test_FuelLevel,    
    ScrID_AlgPrm,
    ScrID_AlgPrm_WorkAlg,
    ScrID_AlgPrm_AlgPrm,
    ScrID_AlgPrm_TTarget,
    ScrID_AlgPrm_TMin,
    ScrID_AlgPrm_TMax,
    ScrID_AlgPrm_PMin,
    ScrID_AlgPrm_FanPMin,
    ScrID_AlgPrm_POpt,
    ScrID_AlgPrm_FanPOpt,
    ScrID_AlgPrm_PMax,
    ScrID_AlgPrm_FanPMax,
    ScrID_AlgPrm_PFix,
    ScrID_AlgPrm_FanPFix,
    ScrID_AlgPrm_FanPWait,
    ScrID_AlgPrm_BlowTime,
    ScrID_AlgPrm_BlowPeriod,
    ScrID_AlgPrm_Autoignition,
	ScrID_AlgPrm_AutoignitionDelay,
	ScrID_AlgPrm_AutoignitionCnt,
	ScrID_AlgPrm_AutoignitionFan,
	ScrID_AlgPrm_AutoignitionFlameAnalizeTime,
	ScrID_AlgPrm_BlowTimeRegStop,
	ScrID_AlgPrm_StrtStpTTargetHyst,
	ScrID_AlgPrm_StartStopSensor,
    ScrID_Ignition,
    ScrID_Ignition_BlowTime,
    ScrID_Ignition_FuelWeight,			
    ScrID_Ignition_SparkPlugTime,
    ScrID_Ignition_FanP,
    ScrID_Ignition_Repeat,
    ScrID_Ignition_FixingTime,
    ScrID_Ignition_FanPFixing,
    ScrID_Ignition_ToAutoignitionTime,
    ScrID_Ignition_FlameLvlBurn,
    ScrID_Ignition_FailBlowTime,    
    ScrID_Security,
    ScrID_Security_OverheatTSupPMin,
    ScrID_Security_OverheatTSupScrewStop,
    ScrID_Security_OverheatTRoomScrewStop,
	ScrID_Security_OverheatTRoomPMin,
    ScrID_Security_RoomTRegMode,
    ScrID_Security_OverheatTTrayPMin,
    ScrID_Security_OverheatTTrayScrewStop,
    ScrID_Security_FueljamBurnoutTime,
    ScrID_Security_WaitModeDelay,
	ScrID_CleanParams, 
	ScrID_CommonParams,    
    ScrID_CommonParams_Language,
	ScrID_CommonParams_SensorsControl,
	ScrID_CommonParams_SensorsControl_Room,
	ScrID_CommonParams_SensorsControl_Outside,
    ScrID_TEH,
    ScrID_TEH_WorkPermission,
    ScrID_TEH_StateOnT,
    ScrID_GSM,
    ScrID_GSM_Mode,
    ScrID_GSM_State,					
    ScrID_GSM_Balance,					
    ScrID_GSM_SignalLevel,				
    ScrID_GSM_Repeat,
    ScrID_GSM_NumbersIn,
    ScrID_GSM_NumberIn1,
    ScrID_GSM_NumberIn2,
    ScrID_GSM_NumberIn3,
    ScrID_GSM_NumberIn4,
    ScrID_GSM_NumberIn5,
    ScrID_GSM_NumbersOut,
    ScrID_GSM_NumberOut1,
    ScrID_GSM_NumberOut2,
    ScrID_GSM_NumberOut3,
    ScrID_GSM_NumberOut4,
    ScrID_GSM_NumberOut5,
    ScrID_GSM_Prefix,
    ScrID_Statistic,
    ScrID_Statistic_TimePultOn,
    ScrID_Statistic_FuelCons,
    ScrID_Statistic_FuelConsHr,
    ScrID_Statistic_FuelConsDay,
    ScrID_Statistic_FuelConsCumTtl,
    ScrID_Statistic_ResetFuelConsCumTtl,
    ScrID_Statistic_Errors,			
    ScrID_Statistic_Error1,			
    ScrID_Statistic_Error2,			
    ScrID_Statistic_Error3,			
    ScrID_Statistic_Error4,			
    ScrID_Statistic_Error5,			
    ScrID_Statistic_Error6,			
    ScrID_Statistic_Error7,		
    ScrID_Statistic_Error8,			
    ScrID_Statistic_Error9,			
    ScrID_Statistic_Error10,					
    ScrID_Statistic_ClearErrors,
	ScrID_FuelCons,
	ScrID_FuelCons_HopperCapacity,
	ScrID_FuelCons_FuelLowLevel,
	ScrID_FuelCons_HopperFullCmd,
	ScrID_FuelCons_NotifCtl,    
    ScrID_Service, 						
    ScrID_Service_KeyGroupTest,
    ScrID_Service_TransMode,
	ScrID_Service_TModeTime,
    ScrID_Service_TModeFanPMin,
    ScrID_Service_PTrans,
    ScrID_Service_AutoIgnFlameHyst,	
	ScrID_Service_StrtStpFanOffDelay,	
    ScrID_Service_ResetSettings,			
    ScrID_Service_Type,		
    ScrID_Service_FueljamPermission,
    ScrID_Service_FactoryNumber,
    ScrID_Service_PelletsThermCond,
    ScrID_Service_ScrewPerform,
    ScrID_Service_MaxPower,
    ScrID_Service_NoFlameLvl,
    ScrID_Service_TimeCounters,			
    ScrID_Service_TimePUPGWork,			
    ScrID_Service_TimeBurnerWork,
    ScrID_Service_TimeScrewWork,			
    ScrID_Service_FanTurnChangeRate,
    ScrID_Service_DebugInfo,	
    ScrID_Service_Statistic_1wire,	
    ScrID_Service_Statistic_1wire_1,	
    ScrID_Service_Statistic_1wire_2,	
    ScrID_Service_Statistic_1wire_3,	
    ScrID_Service_Statistic_1wire_4,
    ScrID_Service_Statistic_1wire_5,		
    ScrID_Service_IdleTime,
    ScrID_Alert_Error,
    ScrID_Alert_Cooling,    
    ScrID_EEPROM_Error,    
} ScreenID;
 



typedef enum {
    ButtonGr_Service = ButtonPos_ESC | ButtonPos_A | ButtonPos_ENCODER,
    ButtonGr_MovingGroup = ButtonPos_A | ButtonPos_B | ButtonPos_OK | ButtonPos_ESC,
} ButtonGroups;



typedef enum {
	TurnID_Display,
	TurnID_Keyboard,
	TurnID_ScreenErrorSwitch,
	TurnID_AutoignitionDelay,
	TurnID_DisplayOff,
	TurnID_CmdSymBlink,
	TurnID_ScrewPerfTest,
} TurnID;



typedef enum {
	SrcSwitch_tStart,
	SrcSwitch_Test_KeyGroup = SrcSwitch_tStart,
	SrcSwitch_tEnd,
} SrcBlockSwitch;



typedef enum {
	SrcBlink_tStart,
	SrcBlink_Test_KeyGroup = SrcBlink_tStart,
	SrcBlink_EEPROM_Error,
	SrcBlink_Edit_Mode,
	SrcBlink_tEnd,
} SrcBlockBlink;
 
 
 
typedef enum {
	OpRes_Success = 0,
	OpRes_Fail = 1
} OperationRes;	



typedef enum {
	AutoIgnState_Reset,
	AutoIgnState_Set,
	AutoIgnState_Wait,
	AutoIgnState_Start,
} AutoignitionState;



typedef struct tm tm_t;


 
typedef struct {
	char sSensors[8];					/* показание етмператур на основном экране */
	char sSparkPlugScrewState[2];		/* состояние Ш/Л */
	char sTempFact[5];					/* Тфх главного экрана */
	char sTempTarget[4];				/* Тц главного экрана */			
} ScrMainEntities;



typedef struct {
	char sState[17];		/* Состояние */
	char sCmd[6];			/* Нажатие -> действие */
} ScrCmdEntities;



typedef struct {
	char sTSup[6];			/* строка с показаниями темп. подачи */	
	char sTRet[6];			/* строка с показаниями темп. обратки */		
	char sTRoom[6];			/* строка с показаниями темп. комнаты */		
	char sTOutside[6];		/* строка с показаниями темп. улицы */	
	char sTTray[6];			/* строка с показаниями темп. лотка */		
	char sKeyGroup[17];		/* строка с нажатыми кнопками */
	char sSPerfTime[4];		/* строка с оставшегося времени теста произв. шнека в сек */	
	u8 uError;				/* значение текущей ошибки на экране */	
	u8 uKeyPressed;			/* текущие надатые кнопки */									
} ScrTestEntities;



typedef struct {
	char sTimePultOn[17];			/* строка содержит время с момента включения пульта
											DDDD HH:MM:SS */		
} ScrStatisticEntities;



typedef struct {
	char sTimePUPGWork[17];			/* строка содержит общее время работы горелки
											YYл DDDд HHч MMм */	
	char sTimeBurnerWork[17];		/* строка содержит общее время горения
											YYл DDDд HHч MMм */		
	char sTimeScrewWork[17];		/* строка содержит общее время работы шнека
											YYл DDDд HHч MMм */													
} ScrServiceEntities;



typedef struct {
	char sDescription[17];		/* строка содержит описание ошибки */													
} ScrAlertEntities;
 


typedef struct {
	char sBalance[7];		/* строка содержит баланс */		
	char sSignalLvl[7];		/* строка содержит ур. сигнала */												
} ScrGSMEntities;

 
 
typedef struct {
	DB_t *DB;			/* указатель на базу параметров и сигналов */
	weh_drv_t *pweh;	/* указатель на дисплей */
	
	ScrMainEntities MainEntities;			/* формируемые строки главного экрана */
	ScrCmdEntities CmdEntities;				/* формируемые строки экрана команд */
	ScrTestEntities TestEntities;			/* формируемые строки экранов проверки */	
	ScrStatisticEntities StatisticEntities;	/* формируемые строки экранов статистики */
	ScrServiceEntities ServiceEntities;		/* формируемые строки экранов сервиса */
	ScrAlertEntities AlertEntities;			/* формируемые строки экранов сигнализации */
    ScrGSMEntities GSMEntities;             /* формируемые строки экранов GSM */
	char sCmdSymBlink[2];					/* мограющий символ во время удержания командной кнопки  */
	
	ButtonPosition KeyPressed;		/* события кнопок по нажатию */
	ButtonPosition KeyReleased;	    /* события кнопок по освобождению */
	ButtonPosition KeyGroup;		/* срез кнопок */
	
	/* Указатели на переключаемые экраны */
	weh_item_t *CurrentScreen;	/* текущий экран */
	weh_item_t *AlertScreen;	/* текущий экран сигналов */
	
	/* последняя ошибка */
	Error lastError;
	/* Код последней ошибки */
	u8 lastErrorCode;
			
	/* Обслуживание таймаутов */
	to_que_t TOQueue;	/* очередь таймаутов */
	to_item_t DisplayTOItem;
	to_item_t KeyboardTOItem;	
	/* таймаут на переключение экранов: ошибка / рабочий экран */
	to_item_t ScreenErrorSwitchTOItem;	
	/* таймаут на команду включения горелки */
	to_item_t AutoignitionDelayTOItem;
	/* таймаут на выключение экрана */
	to_item_t DisplayOffTOItem;
	/* таймаут мигания символа удржания командной кнпоки */
	to_item_t CmdSymBlinkTOItem;
	/* таймаут выключения теста шнека */
	to_item_t ScrewPerfTestTOItem;
	
	/* флаг запуска таймаута на старт горелки */
	AutoignitionState Autoignition;
	
	/* мощности горелок = 0, 26, 42 */
	u8 BurnerPowers[3];		
	
	/* счетчик времени остутствия команд от пользователя */
	int noUserCmdCnt;
	
	/* источники блокирования мигания ошибкой */
	u8 sourcesBlockErrorBlink[SrcBlink_tEnd];
	
	/* источники блокирования мигания ошибкой */
	u8 sourcesBlockScreenSwitch[SrcSwitch_tEnd];

	/* проверка кнопок */
	u8 testKeyGroupCntCmd; // счетчик нажатий для входа/выхода из режима

	/* нажата кнопка команд */
	u8 isPushedDownButtonCmd;
} HMI_t;



static HMI_t *_HMI;
 
 
 
weh_error_e _HMI_wehEvent(weh_item_t *pitem, weh_event_e nevt);
static OperationRes _HMI_CreateDisplays(void);
static void _HMI_SetErrorScreen(void);
static void _HMI_ControlPressed(const weh_item_t *pitem, ButtonPosition cmd);
static void _HMI_ControlGroup(ButtonPosition cmd);
static int _HMI_GroupPressed(ButtonPosition cmd);
static u8 _HMI_CntPressed(ButtonPosition cmd);
static void _HMI_ControlClick(const weh_item_t *pitem, ButtonPosition cmd);
static void _HMI_ControlScreenSwitch(const weh_item_t *prevScr, const weh_item_t *nextScr);
static void _HMI_LED_ON(void);
static void _HMI_LED_OFF(void);
static void _HMI_InitLED(void);
static void _HMI_wehControl(weh_move_state_e key);
static void _HMI_wehStepTree(weh_tree_t *ptree, u8 mode);
static void _HMI_wehStepItem(weh_item_t *pitem);
static void _HMI_DisplayOn(void);
static int _HMI_GetBlockErrorBlink(void);
static int _HMI_GetBlockScreenSwitch(void);
static void _HMI_CorrectRanges(void);
static void _HMI_SetCurrentDynamicPrm(void);
static void _HMI_SlowCommandControl(const weh_item_t *pitem);
static void _HMI_SetCmdSym(void);
static void _HMI_ResetCmdSym(void);
static void _HMI_ToggleCmdSym(void);
static void _HMI_HideAlgPrmScreens(void);
static u32 _HMI_HideMainScreens(void);
static void _HMI_HandleParamsChange(void);
static void _HMI_PostControlOwnLastCmd(void);
static void _HMI_StepToMainScreens(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: locale_strcpy
 *
 * @brief: Копирует строку в буфер в зависимости от выбранного языка 
 * 		@ref Locales_e @ref HMI_Locale
 * 
 * input parameters
 * buf - буфер для копирования
 * localesize - число локализаций
 * args - строки в локализации
 * 
 * output parameters
 * 
 * return длина строки
*/ 
static int locale_strcpy(char *buf, int localesize, ...)
{
	DB_t *DB = _HMI->DB;

	if (!buf || localesize <= 0)
		return 0;

	int i;
	int size = 0;
	char *str = 0;
	va_list args;

	va_start(args, &localesize);
	for (i = 0; i < localesize; ++i) {
		str = va_arg(args, char*);
		if (!str)
			break;
		if (i != HMI_Locale)
			continue;
		strcpy(buf, weh_conv(str));
		size = strlen(buf);
		break;
	}
	va_end(args);

	return size;
}
#define localestrcpy(buf,...) locale_strcpy(buf,Locale_tEnd,##__VA_ARGS__)



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: locale_setpos
 *
 * @brief: Установка значения позиции от выбранного языка 
 * 		@ref Locales_e @ref HMI_Locale
 * 
 * input parameters
 * pos - переменная позиции
 * localesize - число локализаций
 * args - строки в локализации
 * 
 * output parameters
 * 
 * no return
*/ 
static void locale_setpos(u8 *pos, int localesize, ...)
{
	DB_t *DB = _HMI->DB;

	if (!pos || localesize <= 0)
		return;

	int i;
	u8 p = 0;
	va_list args;

	va_start(args, &localesize);
	for (i = 0; i < localesize; ++i) {
		p = va_arg(args, u8);
		if (i != HMI_Locale)
			continue;
		*pos = p;
		break;
	}
	va_end(args);
}
#define localesetpos(pos,...) locale_setpos(pos,Locale_tEnd,##__VA_ARGS__)
 
 
 
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_Work
 *
 * @brief: Основной метод HMI
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void HMI_Work(void)
{		
	DB_t *DB = _HMI->DB;
	to_item_t *turn;	

	/* Постобработка после выдачи своих команд */
	_HMI_PostControlOwnLastCmd();
	
	/* Сброс предыдущей команды */
	HMI_Cmd = HMICmd_No;

	/* Обработка по изменению параметров (например, скрытие экранов) */
	_HMI_HandleParamsChange();

	/* обработка команд GSM */
	switch (GSM_Cmd) {
		case GSMCmd_CorrectTTarget: {
			HMI_Cmd = HMICmd_CorrectTTarget;
			return; // чтобы не потерять команду
		} break;
		default: { /* nop */ } break;
	}
	
	/* Установка команды на режим проверки датчиков */
	if (_HMI->pweh->srw == wmrw_Dynamic) {
		Burner_DynamicMode = 1;
		_HMI_SetCurrentDynamicPrm();
	} else {
		Burner_DynamicMode = 0;
	}
	
	/* Включение горелки по таймауту */
	if (_HMI->Autoignition == AutoIgnState_Set) {
		_HMI->Autoignition = AutoIgnState_Reset;
		TempPrm_AutoignitionDelay = AlgPrm_AutoignitionDelay;
		if (AlgPrm_AutoignitionDelay) {
			_HMI->Autoignition = AutoIgnState_Wait;
			/* Возведение таймаута на включение горелки */
			to_reappend(
					&_HMI->TOQueue, 
					&_HMI->AutoignitionDelayTOItem, 
					AlgPrm_AutoignitionDelay * _HMI_MINUTE_DELAY);			
		} else {
			to_delete(
					&_HMI->TOQueue, 
					&_HMI->AutoignitionDelayTOItem);
		}
	}
	
	/* Проверка ошибок --------------------------------------------- */
	Error error = ErrorManager_GetLast();
	if (error != Error_No) {
		/* Включение светодиода */
		_HMI_LED_ON();
		/* Данная ошибка уже обрабатывается */
		if (_HMI->lastError != error) {
			_HMI->lastError = error;
			/* Формирование экрана ошибки */
			_HMI_SetErrorScreen();
			/* Возведение таймаута на мигание по ошибке */
			to_append(&_HMI->TOQueue, &_HMI->ScreenErrorSwitchTOItem, 0);
		}
	} else {
		_HMI->lastError = Error_No;
	}

	/* Включить дисплей по возникновению ошибки */
	if (error != Error_No && Burner_UserWaiting) {
		_HMI_DisplayOn();
	}
	
	/* Проверка таймаутов ---------------------------------------------------------------- */
	while (1) {
		turn = to_check(&_HMI->TOQueue);
		if (!turn) break;
		to_delete(&_HMI->TOQueue, turn);
		
		switch (turn->id) 
		{
			/* Дисплей ------------------------------------------------------------------- */
			case TurnID_Display: 
			{
				/* Отрисовка дисплея */
				u32 to = weh_work();
				/* Возведение таймаута */
				to_append(&_HMI->TOQueue, &_HMI->DisplayTOItem, to);					
			} break;
			
			/* Клавиатура ---------------------------------------------------------------- */
			case TurnID_Keyboard: 
			{
				/* Работа с кнопками */
				ButtonsHandler_Work();
				ButtonPosition group = ButtonsHandler_GroupState();					
				
				if (!group) {
					/* ТА на выключение дисплея */
					if (Security_WaitModeDelay != 0) {
						_HMI->noUserCmdCnt++;
						if (_HMI->noUserCmdCnt == _HMI_CNT_START_TO_DISPLAY_OFF) {
							/* возведение ТА на отключение дисплея. -1, т.к. минуту уже подождали */
							to_reappend(
									&_HMI->TOQueue, 
									&_HMI->DisplayOffTOItem, 
									(Security_WaitModeDelay-1)*_HMI_MINUTE_DELAY);	
						}
					}
				} else {
					_HMI->noUserCmdCnt = 0;
					/* включить дисплей */
					if (Burner_UserWaiting) {
						_HMI_DisplayOn();
					}
					/* перемещение по дисплею => переключится с экрана ошибки */
					if (error != Error_No) {
						if (_HMI->pweh->ptcur->id == TreeID_Alert)
							_HMI_wehStepTree(_HMI->CurrentScreen->pown, 0);	
						to_reappend(&_HMI->TOQueue, &_HMI->ScreenErrorSwitchTOItem, _HMI_BUTTON_TO_ERROR_BLINK);
					}
				}
				
				/* Нажато более 1 кнопки */
				if (_HMI_GroupPressed(group)) { 
					_HMI_ControlGroup(group);
				} else {
					/* Проверка команд пользователя HMI, для которых требуется удержание ввода */
					if (_HMI->isPushedDownButtonCmd && ButtonsHandler_PressedOver(_HMI_BUTTON_CMD, ButtonState_SET, _HMI_TIME_TO_HANDLE_PRESSED_BUTTON)) {
						_HMI->isPushedDownButtonCmd = 0;
						_HMI_SetCmdSym();
						_HMI_SlowCommandControl(_HMI->pweh->ptcur->picur);
					}
					/* Чтение событий кнопок */
					_HMI->KeyPressed = ButtonsHandler_GroupEvent(ButtonState_SET);
					_HMI->KeyReleased = ButtonsHandler_GroupEvent(ButtonState_RESET);
					/* Исполнение команд с клавиатуры */
					if (_HMI->KeyPressed != ButtonPos_Clear) {
						weh_item_t *curId = _HMI->pweh->ptcur->picur;
						_HMI_ControlPressed(curId, _HMI->KeyPressed);
						/* Обработка переключения экрана */
						_HMI_ControlScreenSwitch(curId, _HMI->pweh->ptcur->picur);
					}
					/* Обратботка клика: KeyGroup с пред. опроса */
					ButtonPosition click = (ButtonPosition)(_HMI->KeyGroup & _HMI->KeyReleased);
					if (click != ButtonPos_Clear) {
						_HMI_ControlClick(_HMI->pweh->ptcur->picur, click);
					}
					/* Проверка отпускания командной кнопки */
					if (_HMI->KeyReleased & _HMI_BUTTON_CMD_POS) {
						_HMI->isPushedDownButtonCmd = 0;	
						_HMI_SetCmdSym();
					}
				}
				
				/* Запись текущей группы */
				_HMI->KeyGroup = group;
				/* Фиксация текущей рабочей области */
				if (_HMI->pweh->ptcur->id != TreeID_Alert) {
					_HMI->CurrentScreen = _HMI->pweh->ptcur->picur;
				}
				/* Блокирование мигания по редактированию */
				_HMI->sourcesBlockErrorBlink[SrcBlink_Edit_Mode] = (_HMI->pweh->srw != wmrw_Show)? 
							1 : 0;
				/* Возведение таймаута */
				to_append(&_HMI->TOQueue, &_HMI->KeyboardTOItem, _HMI_KEYBOARD_TO);		
			} break;
			
			/* Индикация ошибки ---------------------------------------------------------- */
			case TurnID_ScreenErrorSwitch: 
			{				
				if (error != Error_No) {
					uint32_t blink_delay = _HMI_ERROR_BLINK_ON;
					/* Можно мигать */
					if ( !_HMI_GetBlockErrorBlink() ) {
						/* Переключение окна */
						if (_HMI->pweh->ptcur->id != TreeID_Alert) {
							_HMI_wehStepTree(_HMI->AlertScreen->pown, 1);
							_HMI_wehStepItem(_HMI->AlertScreen);
							blink_delay = _HMI_ERROR_BLINK_ON;
						} else {
							_HMI_wehStepTree(_HMI->CurrentScreen->pown, 0);
							blink_delay = _HMI_ERROR_BLINK_OFF;
						}
					}
					to_append(&_HMI->TOQueue, &_HMI->ScreenErrorSwitchTOItem, blink_delay);
				} else {
					/* Выключение светодиода */
					_HMI_LED_OFF();		
					/* Переход в рабочее дерево */
					_HMI_wehStepTree(_HMI->CurrentScreen->pown, 0);					
				}				
			} break;
			
			/* Старт горелки по таймауту ------------------------------------------------- */
			case TurnID_AutoignitionDelay:
			{
				if (_HMI->Autoignition == AutoIgnState_Wait) {
					_HMI->Autoignition = AutoIgnState_Start;
					HMI_Cmd = HMICmd_Start;
					return;
				}
			} break;
			
			/* Выключение дисплея -------------------------------------------------------- */
			case TurnID_DisplayOff:
			{
				if (_HMI->noUserCmdCnt >= _HMI_CNT_START_TO_DISPLAY_OFF && error == Error_No ) {
					/* выход из режима редактирования */
					if (_HMI->pweh->srw != wmrw_Show)
						_HMI_wehControl(wmsEsc);
					/* выключение дисплея */
					weh_block_display();
					Burner_UserWaiting = 1;
				}
			} break;

			/* Моргание символом по удержанию командной кнопки --------------------------- */
			case TurnID_CmdSymBlink:
			{
				if (_HMI->isPushedDownButtonCmd) {
					_HMI_ToggleCmdSym();
				}
				to_append(&_HMI->TOQueue, &_HMI->CmdSymBlinkTOItem, _HMI_CMDSYMBLINK_TO);
			} break;

			/* Выключение теста производительности шнека --------------------------------- */
			case TurnID_ScrewPerfTest:
			{
				TempPrm_ScrewCmd = 0;
			} break;
		}		
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_EditEventControl
 *
 * @brief: Обработка события редактирования
 * 
 * input parameters
 * pitem - Структура экрана
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_EditEventControl(const weh_item_t *pitem) 
{
	DB_t *DB = _HMI->DB;
	
	// по завершению редактирования
	if (_HMI->pweh->srw == wmrw_Show) { 
		switch (pitem->id) {
			case ScrID_Service_Type: {
				HMI_Cmd = HMICmd_ChangeBurnerType;
			} break;
			case ScrID_Service_MaxPower: 
			case ScrID_AlgPrm_PMin:
			case ScrID_AlgPrm_POpt:
			case ScrID_AlgPrm_PMax:
			case ScrID_AlgPrm_FanPMin:
			case ScrID_AlgPrm_FanPOpt:
			case ScrID_AlgPrm_FanPMax:
			// case ScrID_Security_OverheatTSupPMin:
			case ScrID_Security_OverheatTSupScrewStop:
			// case ScrID_Security_OverheatTTrayPMin:
			case ScrID_Security_OverheatTTrayScrewStop:
			case ScrID_Security_OverheatTRoomPMin:
			case ScrID_Security_OverheatTRoomScrewStop:
			// case ScrID_Security_RoomTRegMode:
			{
				HMI_Cmd = HMICmd_CorrectMinMax;
			} break;
			case ScrID_AlgPrm_TTarget:
			{
				HMI_Cmd = HMICmd_CorrectTTarget;
			} break;
			case ScrID_AlgPrm_TMax:
			case ScrID_AlgPrm_TMin:
			{
				HMI_Cmd = HMICmd_CorrectTDeltas;
			} break;
			case ScrID_AlgPrm_AutoignitionDelay: {
				_HMI->Autoignition = AutoIgnState_Set;
			} break;
			case ScrID_CommonParams_Language: {
				HMI_Cmd = HMICmd_Reboot;
			} break;	
			default: { /* nop */ } break;
		}
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_FastCommandControl
 *
 * @brief: Обработка команд пользователя HMI
 * 
 * input parameters
 * pitem - Структура экрана
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_FastCommandControl(const weh_item_t *pitem) 
{
	DB_t *DB = _HMI->DB;
	
	switch (pitem->id) {		
		// обработка ----------------------------------------------------------------
		case ScrID_Test_ErrorCtrl: {
			_HMI->TestEntities.uError = TEST_ERROR_CODE;
			_HMI_LED_ON();
		} break;
		case ScrID_EEPROM_Error: {
			_HMI->sourcesBlockErrorBlink[SrcBlink_EEPROM_Error] = 0;
			ErrorManager_Pop(Error_RETAIN_R);
			_HMI_StepToMainScreens();
		} break;
		// установка на обработку ---------------------------------------------------
		// список соответствует экранам из @ref _HMI_SlowCommandControl
		case ScrID_Cmd: {
			switch (Burner_CurrentState) {
				case BurnerState_ColdStart: 
				case BurnerState_TransMode: 
				case BurnerState_RegularMode: 
				case BurnerState_RegularStop: 
				{ /* nop */ } break;
				// выдача старта горелке
				default: 
				{
					ErrorManager_Pop(Error_NoFlame);
				} break;
			}
		} // no break
		case ScrID_GSM_Balance:
		case ScrID_GSM_SignalLevel:
		case ScrID_Service_ResetSettings:
		case ScrID_Statistic_ClearErrors:
		case ScrID_Statistic_ResetFuelConsCumTtl:
		case ScrID_FuelCons_HopperFullCmd: {	
			_HMI->isPushedDownButtonCmd = 1;
		} break;
		default: { /* nop */ } break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_SlowCommandControl
 *
 * @brief: Обработка команд пользователя HMI, для которых требуется удержание ввода
 * 
 * input parameters
 * pitem - Структура экрана
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_SlowCommandControl(const weh_item_t *pitem)
{
	DB_t *DB = _HMI->DB;
	
	switch (pitem->id) {
		case ScrID_Cmd: {	
			switch (Burner_CurrentState) {
				case BurnerState_ColdStart: 
				case BurnerState_TransMode: 
				case BurnerState_RegularMode: 
				case BurnerState_RegularStop: 
				{
					HMI_Cmd = HMICmd_Stop;
				} break;
				default: 
				{
					HMI_Cmd = HMICmd_Start;
				} break;
			}
			_HMI_StepToMainScreens();
		} break;
		case ScrID_GSM_Balance: {	
			HMI_Cmd = HMICmd_GetBalance;
		} break;
		case ScrID_GSM_SignalLevel: {	
			HMI_Cmd = HMICmd_GetSignalLvl;
		} break;
		case ScrID_Service_ResetSettings: {	
			HMI_Cmd = HMICmd_ClearSettings;
		} break;
		case ScrID_Statistic_ClearErrors: {	
			HMI_Cmd = HMICmd_ClearErrors;
		} break;
		case ScrID_Statistic_ResetFuelConsCumTtl: {	
			HMI_Cmd = HMICmd_ResetFuelConsCumTtl;
		} break;
		case ScrID_FuelCons_HopperFullCmd: {
			HMI_Cmd = HMICmd_HopperFull;
		} break;
		default: { /* nop */ } break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_ControlPressed
 *
 * @brief: Обработка нажатий дисплеем
 * 
 * input parameters
 * pitem - Структура экрана
 * cmd - Команда пользователя (нажатые кнопки)
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_ControlPressed(const weh_item_t *pitem, ButtonPosition cmd)
{
	switch (cmd) {
        case ButtonPos_OK: {
            // режим экрана: только просмотр -> переход вправо, иначе -> переход в редактирвоание
            if (pitem->frw == wmrw_Show) {
                _HMI_wehControl(wmsRight);
            } else {
                _HMI_wehControl(wmsEnter);
            }
			_HMI_FastCommandControl(pitem);
			_HMI_EditEventControl(pitem);				
        } break;
        case ButtonPos_ESC: {
            // текущий режим экрана: просмотр -> переход влево, иначе -> выход из редактирования
            if (_HMI->pweh->srw == wmrw_Show) {
                _HMI_wehControl(wmsLeft);
            } else {
                _HMI_wehControl(wmsEsc);
            }
        } break;
        case ButtonPos_ENCODER: {
            // режим экрана -> подекадное перемещение
            if (pitem->frw != wmrw_Show) {
                _HMI_wehControl(wmsRight);
            }
        } break;
        case ButtonPos_A: {
            if (_HMI->pweh->srw == wmrw_Show) {
                _HMI_wehControl(wmsDown);
            } else {
                _HMI_wehControl(wmsUp);
            }
        } break;
        case ButtonPos_B: {
            if (_HMI->pweh->srw == wmrw_Show) {
                _HMI_wehControl(wmsUp);
            } else {
                _HMI_wehControl(wmsDown);
            }
        } break;
		default: {
			// nop
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_ControlClick
 *
 * @brief: Обработка кликов
 * 
 * input parameters
 * pitem - Структура экрана
 * cmd - Команда пользователя (нажатые кнопки)
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_ControlClick(const weh_item_t *pitem, ButtonPosition cmd)
{
	switch (cmd) {
		case ButtonPos_OK: {
			switch (pitem->id) {
				case ScrID_Test_ErrorCtrl: {	
					_HMI->TestEntities.uError = ErrorManager_GetCode(
							ErrorManager_GetLast()
					);
					_HMI_LED_OFF();
				} break;
				case ScrID_Service_KeyGroupTest: {
					_HMI->testKeyGroupCntCmd++;
					if (!_HMI->sourcesBlockScreenSwitch[SrcSwitch_Test_KeyGroup] && _HMI->testKeyGroupCntCmd == TEST_KEYGROUP_IN_CNT) {
						_HMI->testKeyGroupCntCmd = 0;
						_HMI->sourcesBlockScreenSwitch[SrcSwitch_Test_KeyGroup] = 1;
						_HMI->sourcesBlockErrorBlink[SrcBlink_Test_KeyGroup] = 1;
					}
					if (_HMI->sourcesBlockScreenSwitch[SrcSwitch_Test_KeyGroup] && _HMI->testKeyGroupCntCmd == TEST_KEYGROUP_OUT_CNT) {
						_HMI->testKeyGroupCntCmd = 0;
						_HMI->sourcesBlockScreenSwitch[SrcSwitch_Test_KeyGroup] = 0;
						_HMI->sourcesBlockErrorBlink[SrcBlink_Test_KeyGroup] = 0;
					}
				} break;
			}
		} break;
		default: {
			/* сброс для теста кнопок */
			_HMI->testKeyGroupCntCmd = 0;
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_ControlGroup
 *
 * @brief: Обработка нескольких одновременных нажатий 
 * 
 * input parameters
 * pitem - Структура экрана
 * cmd - Команда пользователя (нажатые кнопки)
 * 
 * output parameters
 * 
 * no return
*/
void _HMI_ControlGroup(ButtonPosition cmd)
{
    ButtonGroups bgr = (ButtonGroups)cmd;
	switch (bgr) {
		case ButtonGr_Service: {		
			weh_tree_t *pt;		
			/* Переход в сервисное дерево */
			if (_HMI->pweh->ptcur->id == TreeID_Main) {
				pt = weh_search_tree(TreeID_Service);
				_HMI_wehStepTree(pt, 1);
			} else /* Переход к рабочему дереву */ {	
				pt = weh_search_tree(TreeID_Main);
				_HMI_wehStepTree(pt, 0);
			}
		} break;
		default: {
			// nop
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_ControlScreenSwitch
 *
 * @brief: Обработка переключения экрана
 * 
 * input parameters
 * prevScr - Экран, с которого переключились
 * nextScr - Экран, на который переключились
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_ControlScreenSwitch(const weh_item_t *prevScr, const weh_item_t *nextScr)
{
	DB_t *DB = _HMI->DB;

	if ( prevScr != nextScr ) {
		switch (nextScr->id) {
			case ScrID_GSM_Balance:
			case ScrID_GSM_SignalLevel: 
			{
				HMI_Cmd_Status = ModCmdSts_RESET;
			} break;		
			default: { /* nop */ } break;
		}
		HMI_Cmd_Status = ModCmdSts_OK; // сброс, для отображения данных разных команд
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_PostControlOwnLastCmd
 *
 * @brief: Постобработка после выдачи своих команд
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_PostControlOwnLastCmd(void)
{
	DB_t *DB = _HMI->DB;
	switch (HMI_Cmd) {
		case HMICmd_CorrectMinMax:
		case HMICmd_CorrectTTarget:
		case HMICmd_ChangeBurnerType: {
			_HMI_CorrectRanges();
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_wehEvent
 *
 * @brief: Callback события с драйвера weh1602
 *
 * NOTE: работает по UPDATE (weh_update)
 * 
 * input parameters
 * pitem - Структура экрана
 * nevt - Сообщение
 * 
 * output parameters
 * 
 * return результат обработки события
*/
weh_error_e _HMI_wehEvent(weh_item_t *pitem, weh_event_e nevt)
{
	DB_t *DB = _HMI->DB;

	if (pitem) {
		switch (nevt) {
			case weUpdate: {
				switch (pitem->id) {
					// MAIN 1,2
					case ScrID_Main1:
					case ScrID_Main2: {
						u8 it = 0;
						switch (Sensors_ReturnStatus) {
							case SensorsTStatus_OK: {
								if (Sensors_ReturnT > -9.49f)
									sprintf(_HMI->MainEntities.sSensors, "%02.00f", Sensors_ReturnT);
								else
									sprintf(_HMI->MainEntities.sSensors, "**");	
								it += 2;
							} break;
							case SensorsTStatus_PresErr: {
								localestrcpy(_HMI->MainEntities.sSensors, 
										"нет",
										"no ");
								it += 3;
							} break;
							default: {
								strcpy(_HMI->MainEntities.sSensors, "crc");			
								it += 3;				
							} break;
						}
						strcpy(_HMI->MainEntities.sSensors+it, weh_conv("→"));		
						it += 1;					
						switch (Sensors_SupplyStatus) {
							case SensorsTStatus_OK: {
								if (Sensors_SupplyT > -9.49f)
									sprintf(_HMI->MainEntities.sSensors+it, "%02.00f", Sensors_SupplyT);
								else
									sprintf(_HMI->MainEntities.sSensors+it, "**");	
								strcpy(_HMI->MainEntities.sSensors+it+2, weh_conv("°"));
							} break;
							case SensorsTStatus_PresErr: {
								localestrcpy(_HMI->MainEntities.sSensors+it,
										"нет",
										"no ");
							} break;
							default: {
								strcpy(_HMI->MainEntities.sSensors+it, "crc");								
							} break;
						}
						if (it == 3) // 1 темп + стрелка заняли 3 места -> 7 символ свободен
							strcpy(_HMI->MainEntities.sSensors+6, " ");
						if (OutSig_ScrewState != 0)
							localestrcpy(_HMI->MainEntities.sSparkPlugScrewState,
									"Ш",
									"S");
						else if (OutSig_SparkPlugState)
							localestrcpy(_HMI->MainEntities.sSparkPlugScrewState,
									"Л",
									"S");
						else
							strcpy(_HMI->MainEntities.sSparkPlugScrewState, weh_conv(" "));
					} break;
					// MAIN 3,4
					case ScrID_Main3:
					case ScrID_Main4: {
						u8 it = 0;
						switch (AlgPrm_WorkAlg) {
							// RETURN FIXINGPOWER
							case Algorithm_Return:
							case Algorithm_FixedPower: {
								switch (Sensors_ReturnStatus) {
									case SensorsTStatus_OK: {
										localestrcpy(_HMI->MainEntities.sTempFact+it,
												"о",
												"r");
										it += 1;
										if (Sensors_ReturnT > -9.49f)
											sprintf(_HMI->MainEntities.sTempFact+it, "%02.00f", Sensors_ReturnT);
										else
											sprintf(_HMI->MainEntities.sTempFact+it, "**");	
										it += 2;
										strcpy(_HMI->MainEntities.sTempFact+it, weh_conv("°"));
									} break;
									case SensorsTStatus_PresErr: {
										localestrcpy(_HMI->MainEntities.sTempFact+it,
												":нет",
												":no ");
									} break;
									default: {
										strcpy(_HMI->MainEntities.sTempFact+it, ":crc");	
									} break;
								}
							} break;
							// START-STOP
							case Algorithm_StartStop: {
								switch (AlgPrm_StartStopSensor) {
									// SUPPLY
									case StartStopSensor_Supply: {
										switch (Sensors_SupplyStatus) {
											case SensorsTStatus_OK: {
												localestrcpy(_HMI->MainEntities.sTempFact+it,
														"п",
														"s");			
												it += 1;
												if (Sensors_SupplyT > -9.49f)
													sprintf(_HMI->MainEntities.sTempFact+it, "%02.00f", Sensors_SupplyT);
												else
													sprintf(_HMI->MainEntities.sTempFact+it, "**");	
												it += 2;
												strcpy(_HMI->MainEntities.sTempFact+it, weh_conv("°"));
											} break;
											case SensorsTStatus_PresErr: {
												localestrcpy(_HMI->MainEntities.sTempFact+it,
														":нет",
														":no ");	
											} break;
											default: {
												strcpy(_HMI->MainEntities.sTempFact+it, ":crc");	
											} break;
										}
									} break;
									// ROOM
									case StartStopSensor_Room: {
										switch (Sensors_RoomStatus) {
											case SensorsTStatus_OK: {
												localestrcpy(_HMI->MainEntities.sTempFact+it,
														"к",
														"r");			
												it += 1;
												if (Sensors_RoomT > -9.49f)
													sprintf(_HMI->MainEntities.sTempFact+it, "%02.00f", Sensors_RoomT);
												else
													sprintf(_HMI->MainEntities.sTempFact+it, "**");	
												it += 2;
												strcpy(_HMI->MainEntities.sTempFact+it, weh_conv("°"));
											} break;
											case SensorsTStatus_PresErr: {
												localestrcpy(_HMI->MainEntities.sTempFact+it,
														":нет",
														":no ");	
											} break;
											default: {
												strcpy(_HMI->MainEntities.sTempFact+it, ":crc");	
											} break;
										}
									} break;
								}
							} break;
							// OTHER
							default: {
								switch (Sensors_RoomStatus) {
									case SensorsTStatus_OK: {
										localestrcpy(_HMI->MainEntities.sTempFact+it,
												"к",
												"r");				
										it += 1;
										if (Sensors_RoomT > -9.49f)
											sprintf(_HMI->MainEntities.sTempFact+it, "%02.00f", Sensors_RoomT);
										else
											sprintf(_HMI->MainEntities.sTempFact+it, "**");	
										it += 2;
										strcpy(_HMI->MainEntities.sTempFact+it, weh_conv("°"));
									} break;
									case SensorsTStatus_PresErr: {
										localestrcpy(_HMI->MainEntities.sTempFact+it,
												":нет",
												":no ");
									} break;
									default: {
										strcpy(_HMI->MainEntities.sTempFact+it, ":crc");	
									} break;
								}
							} break;
						}
					} break;
					case ScrID_Cmd: {
						switch (Burner_CurrentState) {
							case BurnerState_ColdStart: 
							case BurnerState_TransMode: 
							case BurnerState_RegularMode: 
							case BurnerState_RegularStop: 
							{
								if ( !Burner_isRequestedStop ) {
									localestrcpy(_HMI->CmdEntities.sState,
											"В работе        ",
											"Working         ");
								} else {
									localestrcpy(_HMI->CmdEntities.sState,
											"В работе (стоп) ",
											"Working (stop)  ");
								}
								localestrcpy(_HMI->CmdEntities.sCmd,
										"стоп ",
										"stop ");
							} break;
							default: 
							{
								localestrcpy(_HMI->CmdEntities.sState,
										"Ожидание        ",
										"Waiting         ");
								localestrcpy(_HMI->CmdEntities.sCmd,
										"пуск ",
										"start");
							} break;
						}
					} break;
					case ScrID_Statistic_TimePultOn: {
						tm_t tms;
						int days, years;
						time_t timer = (time_t)Statistic_TimePultOn;						
						memcpy(&tms, localtime(&timer), sizeof(tm_t));
						years = tms.tm_year - 70;
						days = (years)? years*365 + tms.tm_yday : tms.tm_yday;
						sprintf(_HMI->StatisticEntities.sTimePultOn, 
								"%.4d %.2d:%.2d:%.2d", // DDDD HH:MM:SS
								days, tms.tm_hour, tms.tm_min, tms.tm_sec);
					} break;
					case ScrID_Service_TimePUPGWork:
					case ScrID_Service_TimeBurnerWork:
					case ScrID_Service_TimeScrewWork: {
						char *DTFstr;
						time_t timer;
						switch (pitem->id) {
							case ScrID_Service_TimePUPGWork: 
								DTFstr = _HMI->ServiceEntities.sTimePUPGWork;
								timer = (time_t)Statistic_TimePUPGWork;
								break;
							case ScrID_Service_TimeBurnerWork: 
								DTFstr = _HMI->ServiceEntities.sTimeBurnerWork;
								timer = (time_t)Statistic_TimeBurnerWork;
								break;
							case ScrID_Service_TimeScrewWork: 
								DTFstr = _HMI->ServiceEntities.sTimeScrewWork;
								timer = (time_t)Statistic_TimeScrewWork;
								break;
						}
						tm_t tms;
						int n = 0;
						timer *= 60;
						memcpy(&tms, localtime(&timer), sizeof(tm_t));	
						sprintf(DTFstr, "%.2d", tms.tm_year - 70);	n+=2;
						n+=localestrcpy(DTFstr+n,
								"л ",
								"y ");
						sprintf(DTFstr+n, "%.3d", tms.tm_yday);		n+=3;
						n+=localestrcpy(DTFstr+n,
								"д ",
								"d ");
						sprintf(DTFstr+n, "%.2d", tms.tm_hour);		n+=2;
						n+=localestrcpy(DTFstr+n,
								"ч ",
								"h ");	
						sprintf(DTFstr+n, "%.2d", tms.tm_min);		n+=2;
						n+=localestrcpy(DTFstr+n,
								"м ",
								"m ");
					} break;
					case ScrID_Test_TSup: {
						switch (Sensors_SupplyStatus) {
							case SensorsTStatus_OK: {								
								sprintf(_HMI->TestEntities.sTSup, "%02.00f", Sensors_SupplyT);
								int it = (Sensors_SupplyT <= -9.0f)?
										3 : 2;
								strcpy(_HMI->TestEntities.sTSup+it, weh_conv("° "));
							} break;
							case SensorsTStatus_PresErr: {
								localestrcpy(_HMI->TestEntities.sTSup,
										"нет  ",
										"no   ");
							} break;
							default: {
								strcpy(_HMI->TestEntities.sTSup, "crc  ");				
							} break;
						}							
					} break;
					case ScrID_Test_TRet: {
						switch (Sensors_ReturnStatus) {
							case SensorsTStatus_OK: {
								sprintf(_HMI->TestEntities.sTRet, "%02.00f", Sensors_ReturnT);
								int it = (Sensors_ReturnT <= -9.0f)?
										3 : 2;
								strcpy(_HMI->TestEntities.sTRet+it, weh_conv("° "));
							} break;
							case SensorsTStatus_PresErr: {
								localestrcpy(_HMI->TestEntities.sTRet,
										"нет  ",
										"no   ");
							} break;
							default: {
								strcpy(_HMI->TestEntities.sTRet, "crc  ");	
							} break;
						}						
					} break;
					case ScrID_Test_TRoom: {
						switch (Sensors_RoomStatus) {
							case SensorsTStatus_OK: {
								sprintf(_HMI->TestEntities.sTRoom, "%02.00f", Sensors_RoomT);
								int it = (Sensors_RoomT <= -9.0f)?
										3 : 2;
								strcpy(_HMI->TestEntities.sTRoom+it, weh_conv("° "));
							} break;
							case SensorsTStatus_PresErr: {
								localestrcpy(_HMI->TestEntities.sTRoom,
										"нет  ",
										"no   ");
							} break;
							default: {
								strcpy(_HMI->TestEntities.sTRoom, "crc  ");	
							} break;
						}							
					} break;
					case ScrID_Test_TOutside: {
						switch (Sensors_OutsideStatus) {
							case SensorsTStatus_OK: {
								sprintf(_HMI->TestEntities.sTOutside, "%02.00f", Sensors_OutsideT);
								int it = (Sensors_OutsideT <= -9.0f)?
										3 : 2;
								strcpy(_HMI->TestEntities.sTOutside+it, weh_conv("° "));
							} break;
							case SensorsTStatus_PresErr: {
								localestrcpy(_HMI->TestEntities.sTOutside,
										"нет  ",
										"no   ");
							} break;
							default: {
								strcpy(_HMI->TestEntities.sTOutside, "crc  ");	
							} break;
						}
					} break;
					case ScrID_Test_TTray: {
						switch (Sensors_TrayStatus) {
							case SensorsTStatus_OK: {
								sprintf(_HMI->TestEntities.sTTray, "%02.00f", Sensors_TrayT);
								int it = (Sensors_TrayT <= -9.0f)?
										3 : 2;
								strcpy(_HMI->TestEntities.sTTray+it, weh_conv("° "));
							} break;
							case SensorsTStatus_PresErr: {
								localestrcpy(_HMI->TestEntities.sTTray,
										"нет  ",
										"no   ");
							} break;
							default: {
								strcpy(_HMI->TestEntities.sTTray, "crc  ");	
							} break;
						}
					} break;
					case ScrID_Test_ScrewPerf: {
						if (TempPrm_ScrewPerfTestTime == 0)
							strcpy(_HMI->TestEntities.sSPerfTime, "   ");
						else if (TempPrm_ScrewPerfTestTime < 1000)
							sprintf(_HMI->TestEntities.sSPerfTime, "%d", TempPrm_ScrewPerfTestTime);
						else
							strcpy(_HMI->TestEntities.sSPerfTime, "***");
					} break;
					case ScrID_Service_KeyGroupTest: {
						_HMI->TestEntities.uKeyPressed = _HMI_CntPressed(_HMI->KeyGroup);
						// ВВ НАЗ ПР ЛВ М
						u8 it = 0;
						if (_HMI->KeyGroup & ButtonPos_OK) {
							strcpy(_HMI->TestEntities.sKeyGroup+it, weh_conv("ВВ "));
						} else {
							strcpy(_HMI->TestEntities.sKeyGroup+it, "   ");
						}
						it += 3;
						if (_HMI->KeyGroup & ButtonPos_ESC) {
							strcpy(_HMI->TestEntities.sKeyGroup+it, weh_conv("НАЗ "));
						} else {
							strcpy(_HMI->TestEntities.sKeyGroup+it, "    ");
						}
						it += 4;
						if (_HMI->KeyGroup & ButtonPos_A) {
							strcpy(_HMI->TestEntities.sKeyGroup+it,  weh_conv("ПР "));
						} else {
							strcpy(_HMI->TestEntities.sKeyGroup+it, "   ");
						}
						it += 3;
						if (_HMI->KeyGroup & ButtonPos_B) {
							strcpy(_HMI->TestEntities.sKeyGroup+it, weh_conv("ЛВ "));
						} else {
							strcpy(_HMI->TestEntities.sKeyGroup+it, "   ");
						}
						it += 3;
						if (_HMI->KeyGroup & ButtonPos_ENCODER) {
							strcpy(_HMI->TestEntities.sKeyGroup+it, weh_conv("М  "));
						} else {
							strcpy(_HMI->TestEntities.sKeyGroup+it, "   ");
						}
					} break;	
					case ScrID_Test_ErrorCtrl: {	
						if (_HMI->TestEntities.uError != TEST_ERROR_CODE) {
							_HMI->TestEntities.uError = ErrorManager_GetCode(
									ErrorManager_GetLast()
							);							
						}
					} break;
                    case ScrID_GSM_Balance: {
                        if (HMI_Cmd_Status == ModCmdSts_OK) {
                            sprintf(_HMI->GSMEntities.sBalance, "%04.01f CU", GSM_Balance);
                        } else {
							localestrcpy(_HMI->GSMEntities.sBalance,
									"н.д.  ",
									"n/a   ");
                        }
                    } break;
                    case ScrID_GSM_SignalLevel: {
                        if (HMI_Cmd_Status == ModCmdSts_OK) {
                            sprintf(_HMI->GSMEntities.sSignalLvl, "%.2d", GSM_SignalLevel);
                        } else {
							localestrcpy(_HMI->GSMEntities.sSignalLvl,
									"н.д.  ",
									"n/a   ");
                        }
                    } break;
					default: {
						// nop
					} break;
				}
			} break;
			default: {
				// nop
			} break;			
		}		
	}	
	return weOk;
}
 

 
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_SetLoadScreen
 * 
 * @brief: Создание загрузчного дерева и экрана
 *
 * NOTE: Вывод загрузочного экрана после инициализации
 * 
 * input parameters
 * pDB - указатель на базу параметров и сигналов
 * 
 * output parameters
 * 
 * return результат создания окон
*/
static OperationRes _HMI_SetLoadScreen(void)
{
	DB_t *DB = _HMI->DB;
	weh_tree_t *pt;
	weh_item_t *pi;
		 
	// загрузочное дерево
	{
		pt = weh_create_tree(NULL, wdDown, TreeID_Load);
		if (!pt) return OpRes_Fail;
		 
		pi = weh_create_item(pt, NULL, wdDown, ScrID_Load);		
		if (!pi) return OpRes_Fail;
				 
		const char version[][16]= {
			#include "version"
		};
				 
		pi->frw = wmrw_Show;
		sprintf(pi->line[0].led, "%s%d", weh_conv("Теплодар АПГ-"), _HMI->BurnerPowers[Service_Type]);
		sprintf(pi->line[1].led, "%s%s", weh_conv("АПГ_"), version[0]);
	}
	return OpRes_Success;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_CntPressed
 *
 * @brief: Проверка нажатия группы кнопок
 * 
 * input parameters
 * cmd - текущие нажатые кнопки
 * 
 * output parameters
 * 
 * return число нажатых кнопок
*/ 
static u8 _HMI_CntPressed(ButtonPosition cmd)
{	
	u8 cnt = 0;	
	for (u8 i = 0; i < 8; ++i)
		if (cmd & (1 << i))
			cnt++;
	return cnt;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_GroupPressed
 *
 * @brief: Проверка нажатия группы кнопок
 * 
 * input parameters
 * cmd - текущие нажатые кнопки
 * 
 * output parameters
 * 
 * return:
 * 		* 1 - если нажато более 1 кнопки 
 * 		* 0 - иначе 
*/ 
static int _HMI_GroupPressed(ButtonPosition cmd)
{	
	u8 cnt = 0;	
	
	/* Если группа нажатых кнопок изменилась с прошлого раза */
	if (_HMI->KeyGroup != cmd) {	
		cnt = _HMI_CntPressed(cmd);
	}
	return (cnt > 1)? 1 : 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_CreateWorkScreens
 * 
 * @brief: Создание рабочих окон
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int HMI_CreateWorkScreens(void)
{
	/* Создание экранов */ 
	if (_HMI_CreateDisplays() != OpRes_Success)
		return 1;	
		
	/* Проверка экранов */	
	if ( weh_start() )
		return 1;

	return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_SetMainScreen
 * 
 * @brief: Переход к основному меню
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int HMI_SetMainScreen(void)
{	
	if ( ErrorManager_isPushed(Error_RETAIN_R) ) {		
		weh_tree_t *pt = weh_search_tree(TreeID_eepromError);
		_HMI_wehStepTree(pt, 1);
		_HMI->sourcesBlockErrorBlink[SrcBlink_EEPROM_Error] = 1;
		return 0;
	}	

	_HMI_StepToMainScreens();
	return 0;
} 



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_SetErrorScreen
 *
 * @brief: Записывает пояснение к ошибке в буфер sDescription для вывода на экран
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _HMI_SetErrorScreen(void)
{		
	weh_tree_t *pt = weh_search_tree(TreeID_Alert);
	switch ( _HMI->lastError ) {
		case Error_TypeDsntSet:
		case Error_TSupDsntMeas:
		case Error_TTrayDsntMeas:
		case Error_TRoomDsntMeas:
		case Error_TReturnDsntMeas:
		case Error_TOutsideDsntMeas:
		case Error_Fueljam:
		case Error_GSMNoResponse:
		case Error_RETAIN_R:
		case Error_RETAIN_W:
		case Error_NoFlame:
		case Error_FuelLowLevel: {
			_HMI->AlertScreen = weh_search_item(pt, ScrID_Alert_Error);
			_HMI->lastErrorCode = ErrorManager_GetCode(_HMI->lastError);
			switch ( _HMI->lastError ) {
				case Error_TypeDsntSet: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Тип не задан    ",
							"Type not set    ");
				} break;
				case Error_TSupDsntMeas: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Т подачи не изм.",
							"Tsup isn't meas.");
				} break;
				case Error_TTrayDsntMeas: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Т лотка не изм. ",
							"Ttray isn't meas");
				} break;
				case Error_TRoomDsntMeas: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Т комнаты не изм",
							"Troom isn't meas");
				} break;
				case Error_TReturnDsntMeas: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Т обратки не изм",
							"Tret isn't meas.");
				} break;
				case Error_TOutsideDsntMeas: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Т улицы не измер",
							"Tout isn't meas.");
				} break;
				case Error_Fueljam: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Затор подачи    ",
							"Fuel jam        ");
				} break;
				case Error_GSMNoResponse: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"GSM модул не отв",
							"GSM isnt respond");
				} break;
				case Error_RETAIN_R: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Чтения ЭНП      ",
							"Reading from NVM");
				} break;
				case Error_RETAIN_W: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Записи ЭНП      ",
							"Writing to NVM  ");
				} break;
				case Error_NoFlame: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Нет пламени     ",
							"No flame        ");
				} break;
				case Error_FuelLowLevel: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Низк.ур.топлива ",
							"Low fuel level  ");
				} break;
			}
		} break;
		case Error_OverheatTSupScrewStop:
		case Error_OverheatTSupPMin:
		case Error_OverheatTTrayPMin:
		case Error_OverheatTTrayScrewStop:
		case Error_OverheatTRoomScrewStop:
		case Error_OverheatTRoomPMin: {
			_HMI->AlertScreen = weh_search_item(pt, ScrID_Alert_Cooling);
			switch ( _HMI->lastError ) {
				case Error_OverheatTSupPMin: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Перегрев подачи1",
							"Supply overheat1");
				} break;
				case Error_OverheatTSupScrewStop: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Перегрев подачи2",
							"Supply overheat2");
				} break;
				case Error_OverheatTTrayPMin: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Перегрев лотка 1",
							"Tray overheat 1 ");
				} break;
				case Error_OverheatTTrayScrewStop: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Перегрев лотка 2",
							"Tray overheat 2 ");
				} break;
				case Error_OverheatTRoomPMin: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Перегрев комн 1 ",
							"Room overheat 1 ");
				} break;
				case Error_OverheatTRoomScrewStop: {
					localestrcpy(_HMI->AlertEntities.sDescription, 
							"Перегрев комн 2 ",
							"Room overheat 2 ");
				} break;
			}
		} break;
	}
}



inline static void _HMI_HideAndStepToMainScr(void)
{
	u32 addr = _HMI_HideMainScreens();
	switch (_HMI->CurrentScreen->id) {
		case ScrID_Main1:
		case ScrID_Main2:
		case ScrID_Main3:
		case ScrID_Main4: {
			_HMI_wehStepItem( (weh_item_t *)(addr) );
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_HandleParamsChange
 *
 * @brief: Обработка изменений параметров базы
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _HMI_HandleParamsChange(void)
{
	DB_t *DB = _HMI->DB;

	switch (Burner_ChangedParam) {
		// по изменению текущего состояния скрывать 3 из 4 главных экрана и переключаться на новый по необходимости
		case ObservedParam_Burner_CurrentState: {
			_HMI_HideAndStepToMainScr();
		} break;
		// по изменению алгоритма - скрывать параметры, не относящиеся к алгоритму
		case ObservedParam_AlgPrm_WorkAlg: {
			_HMI_HideAlgPrmScreens();
			_HMI_HideMainScreens();
		} break;
		// по изменению запускать обратный отсчет
		case ObservedParam_TempPrm_ScrewCmd: {
			if (_HMI->pweh->ptcur->picur->id == ScrID_Test_ScrewPerf) {
				if (TempPrm_ScrewCmd) { // включили
					TempPrm_ScrewPerfTestTime = _HMI_SCREWPERFTEST_TO;
					to_reappend(&_HMI->TOQueue, 
							&_HMI->ScrewPerfTestTOItem, 
							_HMI_SCREWPERFTEST_TO * _HMI_SECOND_DELAY);
				} else { // выключили / время истекло
					TempPrm_ScrewPerfTestTime = 0;
					to_delete(&_HMI->TOQueue, &_HMI->ScrewPerfTestTOItem);
				}
			}
		} break;
		default: { /* NOP */ } break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_HideMainScreens
 *
 * @brief: Скрыть некоторые экраны в зависмости от параметров
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return адрес экрана, на который необходимо переключиться
*/ 
static u32 _HMI_HideMainScreens(void)
{
	DB_t *DB = _HMI->DB;
	weh_tree_t *pt = weh_search_tree(TreeID_Main);
	weh_item_t *pi_m1 = weh_search_item(pt, ScrID_Main1); 
	weh_item_t *pi_m2 = weh_search_item(pt, ScrID_Main2);
	weh_item_t *pi_m3 = weh_search_item(pt, ScrID_Main3);
	weh_item_t *pi_m4 = weh_search_item(pt, ScrID_Main4);
	u32 ret;

	switch (Burner_CurrentState) {
		default: {
			pi_m1->param.hide = 0;
			pi_m2->param.hide = 1;
			pi_m3->param.hide = 1;
			pi_m4->param.hide = 1;
			ret = (u32)&(*pi_m1);
		} break;
		case BurnerState_ColdStart: {
			pi_m1->param.hide = 1;
			pi_m2->param.hide = 0;
			pi_m3->param.hide = 1;
			pi_m4->param.hide = 1;
			ret = (u32)&(*pi_m2);
		} break;
		case BurnerState_RegularMode: {
			switch (AlgPrm_WorkAlg) {
				case Algorithm_ConfigRoomOut:
				case Algorithm_FixedPower: {
					pi_m1->param.hide = 1;
					pi_m2->param.hide = 1;
					pi_m3->param.hide = 0;
					pi_m4->param.hide = 1;
					ret = (u32)&(*pi_m3);
				} break;
				default: {
					pi_m1->param.hide = 1;
					pi_m2->param.hide = 1;
					pi_m3->param.hide = 1;
					pi_m4->param.hide = 0;
					ret = (u32)&(*pi_m4);
				} break;
			}
		} break;
	}
	return ret;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_HideAlgPrmScreens
 *
 * @brief: Скрыть некотоыре экраны в зависмости от параметров
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _HMI_HideAlgPrmScreens(void)
{
	DB_t *DB = _HMI->DB;
	weh_tree_t *pt = weh_search_tree(TreeID_Main);

	switch (AlgPrm_WorkAlg) {
		case Algorithm_ConfigRoomOut:
		case Algorithm_Return: 
		case Algorithm_Room: {
			(weh_search_item(pt, ScrID_AlgPrm_PFix))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_FanPFix))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_StrtStpTTargetHyst))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_StartStopSensor))->param.hide = 1;
			//			
			(weh_search_item(pt, ScrID_AlgPrm_TTarget))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_TMin))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_TMax))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_POpt))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_PMin))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_PMax))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPOpt))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMin))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMax))->param.hide = 0;
		} break;
		case Algorithm_RoomOutside: {
			(weh_search_item(pt, ScrID_AlgPrm_PFix))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_FanPFix))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_TMax))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_POpt))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_StrtStpTTargetHyst))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_StartStopSensor))->param.hide = 1;
			//			
			(weh_search_item(pt, ScrID_AlgPrm_TTarget))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_TMin))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_PMax))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_PMin))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPOpt))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMin))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMax))->param.hide = 0;
		} break;
		case Algorithm_FixedPower: {
			(weh_search_item(pt, ScrID_AlgPrm_TTarget))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_TMin))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_TMax))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_POpt))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_PMin))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_PMax))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_FanPOpt))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMin))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMax))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_StrtStpTTargetHyst))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_StartStopSensor))->param.hide = 1;
			//
			(weh_search_item(pt, ScrID_AlgPrm_PFix))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPFix))->param.hide = 0;
		} break;
		case Algorithm_StartStop: {
			(weh_search_item(pt, ScrID_AlgPrm_TMax))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_PMin))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMin))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_PFix))->param.hide = 1;
			(weh_search_item(pt, ScrID_AlgPrm_FanPFix))->param.hide = 1;
			//
			(weh_search_item(pt, ScrID_AlgPrm_TTarget))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_TMin))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_POpt))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPOpt))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_PMax))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_FanPMax))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_StrtStpTTargetHyst))->param.hide = 0;
			(weh_search_item(pt, ScrID_AlgPrm_StartStopSensor))->param.hide = 0;
		} break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_StepToMainScreens
 *
 * @brief: Переход к главному, не скрытому экрану
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _HMI_StepToMainScreens(void)
{
	weh_tree_t *pt = weh_search_tree(TreeID_Main);
	weh_item_t *pi_m1 = weh_search_item(pt, ScrID_Main1); 
	weh_item_t *pi_m2 = weh_search_item(pt, ScrID_Main2);
	weh_item_t *pi_m3 = weh_search_item(pt, ScrID_Main3);
	weh_item_t *pi_m4 = weh_search_item(pt, ScrID_Main4);	

	_HMI->CurrentScreen = 	(!pi_m1->param.hide)? 	pi_m1 :
							(!pi_m2->param.hide)? 	pi_m2 :
							(!pi_m3->param.hide)? 	pi_m3 :
													pi_m4;	
	_HMI_wehStepItem(_HMI->CurrentScreen);
}



#define CR_SETMIN(t,name,l,f,v) {				\
	pi = weh_search_item(t, ScrID_##name);		\
	memcpy(&pi->line[l].pfields[f].min,&v,4);	\
}
#define CR_SETMAX(t,name,l,f,v) {				\
	pi = weh_search_item(t, ScrID_##name);		\
	memcpy(&pi->line[l].pfields[f].max,&v,4);	\
}
#define CR_MIN_BYNAME(t,name,l,f) {						\
	pi = weh_search_item(t, ScrID_##name);				\
	memcpy(&pi->line[l].pfields[f].min,&name##_Min,4);	\
}
#define CR_MAX_BYNAME(t,name,l,f) {						\
	pi = weh_search_item(t, ScrID_##name);				\
	memcpy(&pi->line[l].pfields[f].max,&name##_Max,4);	\
}
#define CR_MINMAX_BYNAME(t,name,l,f) {					\
	pi = weh_search_item(t, ScrID_##name);				\
	memcpy(&pi->line[l].pfields[f].min,&name##_Min,4);	\
	memcpy(&pi->line[l].pfields[f].max,&name##_Max,4);	\
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_CorrectRanges
 *
 * @brief: Корректировка значений макс и мин параметров
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_CorrectRanges(void)
{	
	DB_t *DB = _HMI->DB;
	weh_tree_t *tree_main = weh_search_tree(TreeID_Main);
	weh_tree_t *tree_serv = weh_search_tree(TreeID_Service);
	weh_item_t *pi;

	CR_MAX_BYNAME(tree_main,AlgPrm_PFix,1,0);

	CR_MINMAX_BYNAME(tree_main,AlgPrm_PMax,1,0);
	CR_MINMAX_BYNAME(tree_main,AlgPrm_POpt,1,0);
	CR_MAX_BYNAME(tree_main,AlgPrm_PMin,1,0);

	CR_MINMAX_BYNAME(tree_main,AlgPrm_FanPMax,1,0);
	CR_MINMAX_BYNAME(tree_main,AlgPrm_FanPOpt,1,0);
	CR_MAX_BYNAME(tree_main,AlgPrm_FanPMin,1,0);
	
	CR_MIN_BYNAME(tree_main,AlgPrm_TMax,1,0);
	CR_MAX_BYNAME(tree_main,AlgPrm_TMin,1,0);
	
	CR_MAX_BYNAME(tree_main,Security_OverheatTSupPMin,1,0);
	CR_MAX_BYNAME(tree_main,Security_OverheatTTrayPMin,1,0);

	CR_MAX_BYNAME(tree_main,Security_OverheatTRoomPMin,1,0);
	CR_MAX_BYNAME(tree_main,Security_RoomTRegMode,1,0);

	CR_MAX_BYNAME(tree_serv,Service_MaxPower,1,0);
	
	CR_MINMAX_BYNAME(tree_serv,Service_PTrans,1,0);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_GetBlockScreenSwitch
 *
 * @brief: Просмотр активных блокировок переключения экрана
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 * 		* 1 - если кто-то блокирует переключение
 * 		* 0 - можно переключать
*/ 
static int _HMI_GetBlockScreenSwitch()
{
	for (SrcBlockSwitch s = SrcSwitch_tStart; s != SrcSwitch_tEnd; ++s)
		if (_HMI->sourcesBlockScreenSwitch[s])
			return 1;
	return 0;		
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_GetBlockErrorBlink
 *
 * @brief: Просмотр активных блокировок моргания экрана
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 * 		* 1 - если кто-то блокирует моргание
 * 		* 0 - можно переключать
*/ 
static int _HMI_GetBlockErrorBlink()
{	
#ifdef HMI_BLINK_OFF
	return 1;
#endif
	for (SrcBlockBlink s = SrcBlink_tStart; s != SrcBlink_tEnd; ++s)
		if (_HMI->sourcesBlockErrorBlink[s])
			return 1;
	return 0;		
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_SetCurrentDynamicPrm
 *
 * @brief: Установка активного динамически изменяющегося параметра в зависимости от экрана
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_SetCurrentDynamicPrm()
{
	DB_t *DB = _HMI->DB;
	switch (_HMI->CurrentScreen->id) {
		//case ScrID_Test_ScrewCtrlPower: { Burner_CurrentDynamicOutput = DynamicOutput_ScrewP; } break;
		case ScrID_Test_ScrewCtrl: { Burner_CurrentDynamicOutput = DynamicOutput_ScrewCmd; } break;
		//case ScrID_Test_FanCtrlPower: { Burner_CurrentDynamicOutput = DynamicOutput_FanP; } break;
		case ScrID_Test_FanCtrl: { Burner_CurrentDynamicOutput = DynamicOutput_FanCmd; } break;
		case ScrID_Test_SparkPlugCtrl: { Burner_CurrentDynamicOutput = DynamicOutput_SparkPlug; } break;
		case ScrID_Test_TEHCtrl: { Burner_CurrentDynamicOutput = DynamicOutput_TEH; } break;
		case ScrID_Test_ScrewPerf: { Burner_CurrentDynamicOutput = DynamicOutput_ScrewCmd; } break;
		default: { /* nop */ } break;
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_DisplayOn
 *
 * @brief: Включить дисплей и перейти на главное дерево
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_DisplayOn(void)
{
	DB_t *DB = _HMI->DB;
	Burner_UserWaiting = 0;
	weh_unblock_display();
	_HMI_StepToMainScreens();
}



static void _HMI_SetCmdSym(void)
{
	strcpy(_HMI->sCmdSymBlink, weh_conv("→"));
}
static void _HMI_ResetCmdSym(void)
{
	strcpy(_HMI->sCmdSymBlink, " ");
}
static void _HMI_ToggleCmdSym(void)
{
	if (_HMI->sCmdSymBlink[0] == ' ') {
		_HMI_SetCmdSym();
	} else {
		_HMI_ResetCmdSym();
	}
}



static void _HMI_wehControl(weh_move_state_e key)
{
	// при заблокированном экране в режиме просмотра команды дисплею не передаются
	if ( !(_HMI_GetBlockScreenSwitch() && _HMI->pweh->srw == wmrw_Show) )
		weh_control(key);
}
static void _HMI_wehStepTree(weh_tree_t *ptree, u8 mode)
{
	if (!_HMI_GetBlockScreenSwitch())
		weh_step_tree(ptree, mode);
}
static void _HMI_wehStepItem(weh_item_t *pitem)
{
	if (!_HMI_GetBlockScreenSwitch())
		weh_step_item(pitem);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_Initialization
 * 
 * @brief: Инициализация HMI (драйверов weh1602, key)
 *
 * NOTE: Вывод загрузочного экрана после инициализации
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
int HMI_Initialization(DB_t *pDB)
{
	if (!_HMI || !pDB) return 1;
		
	_HMI->DB = pDB;
	
	/* Мощность горелок (для формирования строк) */
	_HMI->BurnerPowers[0] = 0;
	_HMI->BurnerPowers[1] = 26;
	_HMI->BurnerPowers[2] = 42;

	/* по старту запуск отсчета */
	_HMI->Autoignition = AutoIgnState_Set;
	
	/* Инициализация объектов обсулижвания таймаутов */
	to_initque(&_HMI->TOQueue);
	to_inititem(&_HMI->DisplayTOItem, 0, TurnID_Display);	
	to_inititem(&_HMI->KeyboardTOItem, 0, TurnID_Keyboard);	
	to_inititem(&_HMI->ScreenErrorSwitchTOItem, 0, TurnID_ScreenErrorSwitch);	
	to_inititem(&_HMI->AutoignitionDelayTOItem, 0, TurnID_AutoignitionDelay);	
	to_inititem(&_HMI->DisplayOffTOItem, 0, TurnID_DisplayOff);	
	to_inititem(&_HMI->CmdSymBlinkTOItem, 0, TurnID_CmdSymBlink);
	to_inititem(&_HMI->ScrewPerfTestTOItem, 0, TurnID_ScrewPerfTest);	
	
	/* Запуск на обработку дисплея и клавиатуры */
	to_append(&_HMI->TOQueue, &_HMI->DisplayTOItem, 0);
	to_append(&_HMI->TOQueue, &_HMI->KeyboardTOItem, 0);
	/* Запуск моргания спецсимвола */
	to_append(&_HMI->TOQueue, &_HMI->CmdSymBlinkTOItem, 0);
	 
	/* Драйвер для работы с weh1602 */
	_HMI->pweh = weh_create();
	if (!_HMI->pweh) return 1;
	/* Callback на обработку событий */	
	_HMI->pweh->event = _HMI_wehEvent;
	/* Период обновления индикации, мкс */
	_HMI->pweh->toshow = 100000;
	
	/* Выставить экран загрузки */	 
	if (_HMI_SetLoadScreen() != OpRes_Success) return 1;
	
	/* Запуск драйвера */
	if ( weh_start() ) return 1;

    /* инициализация обработчика кнопок */
    if ( ButtonsHandler_Create() ) return 1;
    if ( ButtonsHandler_Initialization() ) return 1;
	
    /* инициализация лампочки ошибки */
	_HMI_InitLED();

	/* Установка символа командной кнопки */
	_HMI_SetCmdSym();

	return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: HMI_Create
 *
 * @brief: Создание экземпляра HMI
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/ 
int HMI_Create(void)
{
	_HMI = (HMI_t *)calloc(1, sizeof(HMI_t));
	return (_HMI)? 0 : 1;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_LED
 *
 * @brief: Включение / выключение светодиода
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _HMI_LED_ON(void)
{	
	HMI_LED_PORT->ODR &= ~HMI_LED_OUT;	
}
static void _HMI_LED_OFF(void)
{
	HMI_LED_PORT->ODR |= HMI_LED_OUT;	
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_InitLED
 *
 * @brief: Инициализация переферии светодиода
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/  
static void _HMI_InitLED(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(HMI_LED_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_InitStructure.GPIO_Pin = HMI_LED_OUT;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(HMI_LED_PORT, &GPIO_InitStructure);	
	
	_HMI_LED_OFF();
}


 
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _HMI_CreateDisplays
 * 
 * @brief: Создание пунктов меню
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return результат создания окон
*/
static OperationRes _HMI_CreateDisplays(void)
{
	DB_t *DB = _HMI->DB;
	weh_tree_t *pt;
	weh_item_t *pi;
	weh_item_t *pi1;
	weh_item_t *pi11;
	weh_item_t *pi111;
	weh_field_t *pf;
	weh_shift_t *ps;
	
	pt = weh_search_tree(TreeID_Load);
	if (!pt) return OpRes_Fail;
	 
	// основное дерево
	{
		pt = weh_create_tree(pt, wdDown, TreeID_Main);
		if (!pt) return OpRes_Fail;
		
		// ГЛАВНЫЙ ЭКРАН ---------------------------------------------------------------------
		{
			// ЭКРАН 1
			{
				
			}
			
			// ЭКРАН 2
			{
				
			}

			// ЭКРАН 3
			{
				
			}

			// ЭКРАН 4
			{
				
			}
		}

		// ПУСК/ОСТАНОВ ГОРЕЛКИ --------------------------------------------------------------
		{
			
		}
		
		// МЕНЮ ПАРАМЕТРОВ системы -----------------------------------------------------------
		{
			
		}
		
		// МЕНЮ ПРОВЕРКИ Ф.ЧАСТЕЙ ------------------------------------------------------------
		{
			
		}
		
		// МЕНЮ СТАТИСТИКИ -------------------------------------------------------------------
		{
			
	}
	 
	// сервисное дерево
	{
		pt = weh_create_tree(pt, wdDown, TreeID_Service);
		if (!pt) return OpRes_Fail;
		
		// СЕРВИСНОЕ МЕНЮ --------------------------------------------------------------------
		{
			
	}
	
	// дерево сигнализаций
	{
		pt = weh_create_tree(pt, wdDown, TreeID_Alert);
		if (!pt) return OpRes_Fail;
		
		
	}
	
	return OpRes_Success;
}
