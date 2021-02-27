/*******************************************************************************
  * @file    GSM.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    12.10.2018
  *****************************************************************************/


#include "GSM.h"
#include "ErrorManager.h"
#include "servto.h"
#include "USARTHandler.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


#define _GSM_MSEC_DELAY			            1000
#define _GSM_SECOND_DELAY			        1000000

#define _GSM_DELAY_BETWEEN_2_USART_MSG_US   20000
#define _GSM_DEFAULT_ANS_DELAY              40000
#define _GSM_SMS_SEND_DELAY			        (60 * _GSM_SECOND_DELAY)

#define _GSM_MAX_ERRORS_PER_MESSAGE         2
#define _GSM_HEADER_BUFFER_SIZE             32
#define _GSM_MESSAGE_BUFFER_SIZE            256
#define _GSM_REG_NUM_MAX_SIZE               2
#define _GSM_REG_VAL_MAX_SIZE               17

#define _GSM_SEND_ATTEMPT_MAX               3
#define _GSM_TEST_CLI_MAX               	3



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GSMIntCmd
 *
 * @brief: Команды GSM от дургих программных модулей
 * 
 * NOTE: порядок важен
 *
*/
typedef enum {    
    GSMIntCmd_tStart = 0,  
	GSMIntCmd_GetBalance = 0,		/* запрос баланса */
    GSMIntCmd_GetSignalLvl,         /* запрос уровня сигнала */
    GSMIntCmd_tEnd,

    GSMIntCmd_No,	                /* нет команды */ 
} GSMIntCmd;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GSMExtCmd
 *
 * @brief: Команды GSM от пользователя через SMS
 *
*/
typedef enum {    
    GSMExtCmd_tStart = 0,
	GSMExtCmd_Start = 0,		    /* старт горелки */
	GSMExtCmd_Stop,                 /* стоп горелки */   
	GSMExtCmd_GetInfo,              /* состояние горелки */
    GSMExtCmd_SetREG,               /* Запись регистра */
    GSMExtCmd_GetREG,               /* Чтение регистра */
    GSMExtCmd_tEnd,

    GSMExtCmd_No,                   /* нет команды */ 
} GSMExtCmd;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GSMNotif
 *
 * @brief: Уведомления по наступлению события
 *
*/
typedef enum {
    GSMNotif_Start,     /* по переходу в штатный режим*/
    GSMNotif_Stop,      /* по переходу в режим ожидания */
    GSMNotif_Error,     /* по ошибке */
    GSMNotif_DeniedStart,   /* по фатальной ошибке в ожидании старта */
    GSMNotif_DeniedStop,    /* по фатальной ошибке в ожидании останова */
    GSMNotif_No         /* нет уведомлений */
} GSMNotif;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GSMNotif
 *
 * @brief: События
 *
*/
typedef enum {
    GSMEvent_Start,
    GSMEvent_Stop,
    GSMEvent_tEnd,
} GSMEvent;



typedef enum {
	TurnID_Common,
} TurnID;



typedef struct {    
    uint8_t buffer[_GSM_HEADER_BUFFER_SIZE];
    uint8_t curPos;
} Header_t;



typedef struct {    
    uint8_t buffer[_GSM_MESSAGE_BUFFER_SIZE];
    uint8_t curPos;
} Message_t;



typedef struct {    
    uint8_t data[USARTHandler_BUFFER_SIZE];
    char *it;
} CommonBuf_t;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: REGCmd
 *
 * @brief: Команда над регистром
 *
*/
typedef enum {    
    REGCmd_SetREG,               /* Запись регистра */
    REGCmd_GetREG,               /* Чтение регистра */
    REGCmd_Denied,               /* отклонение команды */
} REGCmd;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: REGType
 *
 * @brief: Тип регистра
 *
*/
typedef enum {
    REG_PFH,	// Params_floatHE
    REG_PCH,	// Params_charHE_16
    REG_PCI,	// Params_charIE_16
    REG_PIH,	// Params_intHE
    
    REG_SII,	// Signals_intI
    REG_SFI,	// Signals_floatI
    REG_SIE,	// Signals_intIE

    REG_DIH,	// Debug_intH
    REG_DFH,	// Debug_floatH
    REG_DFI,	// Debug_floatI
} REGType;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: REG_t
 *
 * @brief: Регистр GSM
 *
*/
typedef struct {
    int n;
    REGType t;
    union {
        float f;
        int d;
        char c[_GSM_REG_VAL_MAX_SIZE];
    } VAL;
} REG_t;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: CommandParams_t
 *
 * @brief: Параметры команды над регистром
 *
*/
typedef struct {
    int valid;
    REG_t REG;
} CommandParams_t;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GSMModuleCmd
 *
 * @brief: Команды GSM модуля
 *
*/
typedef enum {   
    GSMModuleCmd_No,                /* нет команды */
    GSMModuleCmd_SMSSendCmd,        /* команда отправки смс */
	GSMModuleCmd_SMSSendContent, /* отправка сообщения */
    GSMModuleCmd_SMSRead,           /* прочитать смс */
    GSMModuleCmd_GetBalance,        /* запрос баланса */
    GSMModuleCmd_GetSignalLvl,      /* запрос уровня сигнала */
	GSMModuleCmd_TestCLI, 			/* проверка доступа CLI */

    GSMModuleCmd_tInitStart,        /* блок команд инициализации */
	GSMModuleCmd_PowerOn = GSMModuleCmd_tInitStart, 	/* по включению модуля */
    GSMModuleCmd_AT,            	/*  */
    GSMModuleCmd_ATE0,              /* выключить эхо */
    GSMModuleCmd_GSMBUSY,           /* запретить вх. звонки */
    GSMModuleCmd_CMGF,              /* выбор формата сообщений */
    GSMModuleCmd_CREG,             	/* регистрация в сети */
    GSMModuleCmd_DELALL,            /* удалить все смс */
    GSMModuleCmd_CPAS,             	/* готовность к работе */
    GSMModuleCmd_tInitEnd,
} GSMModuleCmd;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ModAnsRes
 *
 * @brief: Результат ответа модуля
 *
*/
typedef enum {
	ModAnsRes_No,			/* нет ответа */
	ModAnsRes_OK,			/* верный ответ */
	ModAnsRes_Timeout,		/* таймаут на ответ */
} ModAnsRes;



typedef struct {
    int State;    
} USARTMode_t;



#define TELNUMB_BUF_SIZE (TELEPHONE_NUMBER_SIZE+TELEPHONE_COUNTRY_CODE_SIZE+2)



typedef struct {
    int State;
    char ExpectedStr[_GSM_MESSAGE_BUFFER_SIZE];
    CommonBuf_t CommonBuffer;
    GSMModuleCmd modCmd;
    GSMNotif notif;
    u32 ExpectedTO;
    char currTelNumber[TELNUMB_BUF_SIZE];
    uint8_t currTelNumOutIdx;  
    uint8_t nSendAttempt;
	uint8_t softInitFailCnt;
	uint8_t testCLICnt;
	int TxCnt;
	int RxCnt;
} SMSMode_t;



typedef struct {	
	DB_t *DB;			/* указатель на базу параметров и сигналов */    
    int MainState;      /* основное состояние GSM */
    /* Подсостояния автомата */
    USARTMode_t USARTMode;
    SMSMode_t SMSMode;

    /* команды GSM от других модулей */
    int IntCommands[GSMIntCmd_tEnd];

    /* ожидаемые уведомления по событиям */
    int Notifications[GSMEvent_tEnd];

    /* Предыдущее фиксирвоанное состояние */
    int prevIsAttached;

    /* Буфер приема */
    uint8_t *RxBuffer;
    /* Сообщение на отправку */
    Header_t header;
    Message_t message;
    
    /* Обслуживание таймаутов */
	to_que_t TOQueue;	/* очередь таймаутов */
	to_item_t CommonTOItem;
    int CommonTO;
	
	/* Последние фиксированные ошибки */
	int lastErrors[Error_tEnd];
    /* Ошибки, которые нужно обработать */
    int errorsToHandle[Error_tEnd];

    /* индексы номеров, на которые нужно отвечать */
    int telNumForRespIdxs[GSMExtCmd_tEnd];

    /* указатель на статус, соотнесенный с командой */
    int *IntCommandStatus;

} GSM_t;



static GSM_t *_GSM = 0;


static GSMIntCmd _GSM_GetNextInternalCommand(void);
static GSMModuleCmd _GSM_HandleInternalCommand(GSMIntCmd command);

static GSMModuleCmd _GSM_HandleExternalCommand(void);

static GSMNotif _GSM_GetNextNotification(void);
static GSMModuleCmd _GSM_HandleNotification(GSMNotif notif);

static REGCmd _GSM_CheckExtCmdParams(GSMExtCmd command, const CommandParams_t *params);
static void _GSM_SetREGValue(const REG_t *REG);
static void _GSM_ReadREGValue(CommandParams_t *params);
static int _GSM_ReadREGParams(const char *buf, REGCmd act, CommandParams_t *params);
static void _GSM_ConvertREGValueToString(const REG_t *REG, char *str);

static void _GSM_PushStringToHeader(const char *str);

static void _GSM_PushBurnerInfoToMessage(void);
static void _GSM_PushStringToMessage(const char *str);
static void _GSM_PushErrorToMessage(Error error);
static int _GSM_PushErrorsToMessage(void);
static void _GSM_PushTailToMessage(void);

static int _GSM_GetIdxOfTelNumberIn(const char *telNum);
static int _GSM_isCorrectTelNumber(const char *telNum);
static int _GSM_SetNotifTelNumber(GSMNotif notif);

static void _GSM_SMSModeWork(void);
static void _GSM_USARTModeWork(void);

static void _GSM_SendModuleCmd(void);
static void _GSM_SetModuleAnsWaitParams(void);
static void _GSM_SendSMSContent(void);
static int _GSM_SetModuleCmdParams(GSMModuleCmd cmd);
static int _GSM_isCorrectModuleAns(void);
static int _GSM_isUnreadSMS(void);
static int _GSM_HandleModuleCmdRes(GSMModuleCmd cmd);
static ModAnsRes _GSM_TestModuleAns(void);

static char* _GSM_strstr(const char *_S, uint16_t _S_SIZE, const char *_P);

#define _GSM_POWER_CONT_ON(x)	GPIO_ResetBits(GSM_PWR_CONT_PORT, GSM_PWR_CONT_PIN)
#define _GSM_POWER_CONT_OFF(x)	GPIO_SetBits(GSM_PWR_CONT_PORT, GSM_PWR_CONT_PIN)
#define _GSM_POWER_KEY_UP(x)	GPIO_SetBits(GSM_PWR_KEY_PORT, GSM_PWR_KEY_PIN)
#define _GSM_POWER_KEY_DOWN(x)	GPIO_ResetBits(GSM_PWR_KEY_PORT, GSM_PWR_KEY_PIN)
  


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GSM_Work
 *
 * @brief: Основной метод GSM
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void GSM_Work(void)
{
    to_item_t *turn;
    DB_t *DB = _GSM->DB;

    /* Сброс предыдущей команды ------------------------------------ */
    GSM_Cmd = GSMCmd_No;

    /* Команды с HMI ----------------------------------------------- */
    switch (HMI_Cmd) {
        case HMICmd_GetBalance: { 
            _GSM->IntCommands[GSMIntCmd_GetBalance] = 1; 
            _GSM->IntCommandStatus = &HMI_Cmd_Status;
        } break;
        case HMICmd_GetSignalLvl: { 
            _GSM->IntCommands[GSMIntCmd_GetSignalLvl] = 1; 
            _GSM->IntCommandStatus = &HMI_Cmd_Status;
        } break;
        default: { /* nop */ } break;
    }

    /* Основной автомат -------------------------------------------- */
    switch (_GSM->MainState) 
    {
        // общий сброс
        case 0: {
			_GSM->SMSMode.TxCnt = GSM_SMSTxCnt;
			_GSM->SMSMode.RxCnt = GSM_SMSRxCnt;
            _GSM->SMSMode.State = 0;
            _GSM->USARTMode.State = 0;
            _GSM->CommonTO = 0;
            to_delete(&_GSM->TOQueue, &_GSM->CommonTOItem);
            USARTHandler_ForceReadEnd();
            USARTHandler_ForceWriteStop();
            memset(_GSM->RxBuffer, 0, USARTHandler_BUFFER_SIZE);
            memset(&_GSM->message, 0, sizeof(Message_t));
            memset(&_GSM->header, 0, sizeof(Header_t));
            _GSM->MainState = 1;
        } break;
        // проверка ТА и выбор автомата
        case 1: {
            while (1) {
                turn = to_check(&_GSM->TOQueue);
                if (!turn) break;
                to_delete(&_GSM->TOQueue, turn);                
                switch (turn->id) {
                    case TurnID_Common: { _GSM->CommonTO = 1; } break;
                }
            }
            // состяние GSM модуля поменялось
            if (_GSM->prevIsAttached != GSM_Attached) {
                _GSM->prevIsAttached = GSM_Attached;
                _GSM->MainState = 0;
                break;
            }
            // выбор автомата
            if (GSM_Attached) {
                _GSM_SMSModeWork();
            } else {
                _GSM_USARTModeWork();
            }
        }
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_USARTModeWork
 *
 * @brief: Автомат работы модуля в режиме USART (чтение и вывод в порт)
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_USARTModeWork(void)
{
    switch (_GSM->USARTMode.State) 
    {
        // сброс
        case 0: 
        {
            memset(&_GSM->message, 0, sizeof(Message_t));
            USARTHandler_StartRead(0, USARTHandler_BUFFER_SIZE); // данные берутся напрямую, перекладывать не следует
            _GSM->USARTMode.State = 1;
        } break;
        // проверка команд от других модулей
        case 1: 
        {
            // получение приоритетной команды
            GSMIntCmd icmd = _GSM_GetNextInternalCommand();
            GSMModuleCmd gcmd = _GSM_HandleInternalCommand(icmd);
            if ( gcmd != GSMModuleCmd_No ) {
                _GSM->USARTMode.State = 11;
                break;       
            }  
            _GSM->USARTMode.State = 2;            
        } break;
        // проверка уведомлений
        case 2: 
        {            
            GSMNotif notif = _GSM_GetNextNotification();
            if ( _GSM_HandleNotification(notif) ) {
                _GSM->USARTMode.State = 10;
                break;
            }
            _GSM->USARTMode.State = 3;  
        } break;
        // проверка принятых команд
        case 3: 
		{
            if ( _GSM_HandleExternalCommand() != GSMModuleCmd_No ) {
                _GSM->USARTMode.State = 10;
                break;                 
            }
            // нет команд / нечего отвечать
            _GSM->USARTMode.State = 1;            
        } break;
        // отправка ответов или уведомлений
        case 10: 
		{
            USARTHandler_SendBuffer(_GSM->message.buffer, _GSM->message.curPos);
            to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, _GSM_DELAY_BETWEEN_2_USART_MSG_US);
            _GSM->USARTMode.State = 12;
        } break;
        // отправка команд
        case 11: 
		{
            USARTHandler_SendBuffer(_GSM->header.buffer, _GSM->header.curPos);
            to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, _GSM_DELAY_BETWEEN_2_USART_MSG_US);
            _GSM->USARTMode.State = 12;
        } break;
        // пауза перед новой отправкой
        case 12: 
		{
            if (_GSM->CommonTO) {
                _GSM->CommonTO = 0; // сброс
                _GSM->USARTMode.State = 0;
            }
        } break;
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_SMSModeWork
 *
 * @brief: Автомат работы модуля в режиме SMS (работа с GSM модулем)
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_SMSModeWork(void)
{
    DB_t *DB = _GSM->DB;

	if ( !GSM_Mode ) {
		GSM_State = GSMSt_Off;
		_GSM_POWER_CONT_OFF();
		_GSM->SMSMode.State = 0;
		ErrorManager_Pop(Error_GSMNoResponse);
		return;
	}

    switch (_GSM->SMSMode.State) 
    {
		// ошибка работы с модулем на этапе инициализации
		case -1: 
		{
			ErrorManager_Push(Error_GSMNoResponse);
			_GSM->SMSMode.State = 0;
		}
        // сброс
		// pc = power_control (питание на модуль) = 0
		// pk = power_key (спец. ножка) = 1
        case 0: 
        {
			GSM_State = GSMSt_Initialization;
			_GSM_POWER_CONT_OFF();
			_GSM_POWER_KEY_UP();
            memset(&_GSM->message, 0, sizeof(Message_t));
            memset(&_GSM->header, 0, sizeof(Header_t));
            for (GSMExtCmd c = GSMExtCmd_tStart; c != GSMExtCmd_tEnd; ++c)
                _GSM->telNumForRespIdxs[c] = -1;
			to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, 2 * _GSM_SECOND_DELAY);	
            _GSM->SMSMode.softInitFailCnt = 0;
			_GSM->SMSMode.State = 1;
        } break;
        // Инициализация GSM 1: pk = pc = 1
        case 1: 
        {
            if (_GSM->CommonTO) {
                _GSM->CommonTO = 0;
				_GSM_POWER_CONT_ON();
				_GSM_POWER_KEY_UP();
				to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, 2 * _GSM_SECOND_DELAY);
				_GSM->SMSMode.State = 2;
			}
        } break;
        // Инициализация GSM 2: pc = 1; pk = 0
        case 2: 
        {
            if (_GSM->CommonTO) {
                _GSM->CommonTO = 0;	
				_GSM_POWER_KEY_DOWN();	
                to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, 2 * _GSM_SECOND_DELAY);
                _GSM->SMSMode.State = 3;
            }
        } break;
        // Инициализация GSM 3: pk = pc = 1
        case 3: 
        {
            if (_GSM->CommonTO) {
                _GSM->CommonTO = 0;		
				_GSM_POWER_KEY_UP();	
                to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, 2 * _GSM_SECOND_DELAY);
                _GSM->SMSMode.State = 4;
            }
        } break;
        // Ожидание готовности GSM
        case 4: 
        {
            if (_GSM->CommonTO) {
                _GSM->CommonTO = 0;
                if ( GSM_ModuleState ) {
					_GSM->SMSMode.modCmd = GSMModuleCmd_tInitStart;
                    _GSM->SMSMode.State = 5;
                } else {
					_GSM->SMSMode.State = -1;
                }                    
            }
        } break;
        // настройка модуля
        case 5: 
        {
            _GSM_SetModuleAnsWaitParams();
            _GSM_SetModuleCmdParams(_GSM->SMSMode.modCmd);
            _GSM_SendModuleCmd();
            _GSM->SMSMode.State = 7;
        } break;
        // ожидание ответа на команду
        case 7: 
        {
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: {
					_GSM->SMSMode.modCmd++;
					if ( _GSM->SMSMode.modCmd == GSMModuleCmd_tInitEnd) {
						_GSM->SMSMode.State = 8;
					} else {
						_GSM->SMSMode.State = 5;
					}
				} break;
				case ModAnsRes_Timeout: {
					_GSM->SMSMode.softInitFailCnt++;
					if (_GSM->SMSMode.softInitFailCnt > 10) {				
						_GSM->SMSMode.State = -1;
					} else {
						_GSM->SMSMode.modCmd = (GSMModuleCmd)((int)GSMModuleCmd_tInitStart + 1);	
						_GSM->SMSMode.State = 5;
					}
				} break;
				default: { /* nop */ } break;
			}
        } break;
		// удаление ошибки
		case 8:
		{			
			GSM_State = GSMSt_Work;
			ErrorManager_Pop(Error_GSMNoResponse);
			_GSM->SMSMode.State = 200;
		} break;


        // проверка команд от других модулей
        case 200: 
        {
            // получение приоритетной команды
            GSMIntCmd icmd = _GSM_GetNextInternalCommand();
            _GSM->SMSMode.modCmd = _GSM_HandleInternalCommand(icmd);
            if ( _GSM->SMSMode.modCmd != GSMModuleCmd_No ) {
                // есть что отправлять
                _GSM_SetModuleAnsWaitParams();
                _GSM_SendModuleCmd();
                _GSM->SMSMode.State = 201;
                break;       
            }  
            _GSM->SMSMode.State = 300;            
        } break;
        // ожидание ответа модуля на команду
        case 201:
        {
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: {
                    _GSM_HandleModuleCmdRes(_GSM->SMSMode.modCmd);	
				} // no break;			
				case ModAnsRes_Timeout: {
                	_GSM->SMSMode.State = 200;
				} break;
				default: { /* nop */ } break;
			}
        } break;


        // проверка уведомлений
        case 300: 
        {            
            _GSM->SMSMode.notif = _GSM_GetNextNotification();
            if ( _GSM_HandleNotification(_GSM->SMSMode.notif) != GSMModuleCmd_No ) {
				_GSM->SMSMode.currTelNumOutIdx = 0; // сброс
                _GSM->SMSMode.State = 301;
                break;
            }
            // нечего отправлять
            _GSM->SMSMode.State = 400;  
        } break;
        // установка команды модулю на отправку sms
        case 301:
        {
            _GSM->SMSMode.nSendAttempt = 0;
            if ( _GSM_SetNotifTelNumber(_GSM->SMSMode.notif) ) {              
                // формирование команды модулю
                if ( _GSM_SetModuleCmdParams(GSMModuleCmd_SMSSendCmd) ) {
                    // есть что отправлять
                    _GSM->SMSMode.State = 302;
                    break;
                } 
            }
            // нет номеров на отправку
            _GSM->SMSMode.State = 400;
        } break;
        // отправка команды
        case 302: 
        {
            _GSM_SetModuleAnsWaitParams();
            _GSM_SendModuleCmd();
            _GSM->SMSMode.State = 303;
        } break;
        // ожидание предложения для ввода SMS
        case 303:
        {
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: {
                    // отправка сообщения
					_GSM_SetModuleCmdParams(GSMModuleCmd_SMSSendContent);
					_GSM_SetModuleAnsWaitParams();
					_GSM_SendSMSContent();
					_GSM->SMSMode.State = 304;            
				} break;			
				case ModAnsRes_Timeout: {
					_GSM->SMSMode.testCLICnt++;
					// превышение числа попыток отправки
					if (_GSM->SMSMode.testCLICnt > _GSM_TEST_CLI_MAX) {
						_GSM->SMSMode.State = -1;
						break;
					}
					// тест обмена данными с модулем
					_GSM_SetModuleCmdParams(GSMModuleCmd_TestCLI);
					_GSM_SetModuleAnsWaitParams();
					_GSM_SendModuleCmd();
					_GSM->SMSMode.State = 305;
				} break;
				default: { /* nop */ } break;
			}
        } break;
        // ожидание ответа от модуля на команду отправки
        case 304:
        {
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: {
                    GSM_SMSTxCnt++;
                    _GSM->SMSMode.State = 301; // следующий номер           
				} break;			
				case ModAnsRes_Timeout: {
					// попробовать отправить на следующий номер
					_GSM->SMSMode.State = 301;
					// или повтор на тотже номер
                	_GSM->SMSMode.nSendAttempt++;
                    if (GSM_Repeat && _GSM->SMSMode.nSendAttempt <= _GSM_SEND_ATTEMPT_MAX) {
                        _GSM->SMSMode.State = 302;
                    }
				} break;
				default: { /* nop */ } break;
			}
        } break;
		// ожидание результата теста CLI
		case 305: 
		{
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				// повтор команды передачи смс
				case ModAnsRes_OK: {     
					_GSM->SMSMode.State = 302;
				} break;		
				// что-то поломалось	
				case ModAnsRes_Timeout: {  
					_GSM->SMSMode.State = -1;
				} break;
				default: { /* nop */ } break;
			}
		} break;


        // запрос не прочитанных смс
        case 400: 
        {      
            _GSM_SetModuleCmdParams(GSMModuleCmd_SMSRead);
            _GSM_SetModuleAnsWaitParams();
            _GSM_SendModuleCmd();
            _GSM->SMSMode.State = 401;            
        } break;
        // ожидание ответа от модуля
        case 401: 
        {
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: { 
					if ( _GSM_isUnreadSMS() ) {
						_GSM->SMSMode.nSendAttempt = 0;
						if ( _GSM_HandleExternalCommand() != GSMModuleCmd_No ) {    
							// формирование команды модулю
							if ( _GSM_SetModuleCmdParams(GSMModuleCmd_SMSSendCmd) ) {
								_GSM->SMSMode.State = 403;
								break;
							}                
						}
					}                  
				} // no break;			
				case ModAnsRes_Timeout: {         
					// нет команд / нечего отвечать
					_GSM->SMSMode.State = 500;  
				} break;
				default: { /* nop */ } break;
			}
        } break;
        // отправка команды
        case 403:
        {
            _GSM_SetModuleAnsWaitParams();
            _GSM_SendModuleCmd();
            _GSM->SMSMode.State = 404;
        } break;
        // ожидание предложения для ввода SMS
        case 404: 
		{
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: { 
					// отправка сообщения
					_GSM_SetModuleCmdParams(GSMModuleCmd_SMSSendContent);
					_GSM_SetModuleAnsWaitParams();
					_GSM_SendSMSContent();
					_GSM->SMSMode.State = 405;                          
				} break;			
				case ModAnsRes_Timeout: {       
					_GSM->SMSMode.testCLICnt++;
					// превышение числа попыток отправки
					if (_GSM->SMSMode.testCLICnt > _GSM_TEST_CLI_MAX) {
						_GSM->SMSMode.State = -1;
						break;
					}
					// тест обмена данными с модулем
					_GSM_SetModuleCmdParams(GSMModuleCmd_TestCLI);
					_GSM_SetModuleAnsWaitParams();
					_GSM_SendModuleCmd();
					_GSM->SMSMode.State = 406;
				} break;
				default: { /* nop */ } break;
			}
        } break;
        // ожидание ответа от модуля на команду отправки
        case 405: 
		{   
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: {        
                    GSM_SMSTxCnt++;   
                	_GSM->SMSMode.State = 500;                   
				} break;			
				case ModAnsRes_Timeout: {
                	_GSM->SMSMode.State = 500;
                    _GSM->SMSMode.nSendAttempt++;
                    if (GSM_Repeat && _GSM->SMSMode.nSendAttempt <= _GSM_SEND_ATTEMPT_MAX) {
                        // повтор
                        _GSM->SMSMode.State = 403;
                        break;
                    }
				} break;
				default: { /* nop */ } break;
			}
        } break;
		// ожидание результата теста CLI
		case 406: 
		{
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				// повтор команды передачи смс
				case ModAnsRes_OK: {     
					_GSM->SMSMode.State = 403;
				} break;		
				// что-то поломалось	
				case ModAnsRes_Timeout: {  
					_GSM->SMSMode.State = -1;
				} break;
				default: { /* nop */ } break;
			}
		} break;


        // удаление всех смс
        case 500: 
		{
			if ( _GSM->SMSMode.TxCnt != GSM_SMSTxCnt || _GSM->SMSMode.RxCnt != GSM_SMSRxCnt ) {				
				_GSM->SMSMode.TxCnt = GSM_SMSTxCnt;
				_GSM->SMSMode.RxCnt = GSM_SMSRxCnt;         
				_GSM_SetModuleCmdParams(GSMModuleCmd_DELALL);
				_GSM_SetModuleAnsWaitParams();
				_GSM_SendModuleCmd();
				_GSM->SMSMode.State = 501;
			} else {
				_GSM->SMSMode.State = 200;
			}
        } break;
        // ожидание таймаута на удаление
        case 501: 
		{
			ModAnsRes res = _GSM_TestModuleAns();
			switch (res) {
				case ModAnsRes_OK: {                        
				} // no break;			
				case ModAnsRes_Timeout: {     
                	_GSM->SMSMode.State = 200;
				} break;
				default: { /* nop */ } break;
			}
        } break;
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_SetModuleCmdParams
 *
 * @brief: Установка параметров _GSM->SMSMode
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 *      1 - есть что отправлять
 *      0 - нечего отправлять
*/ 
static int _GSM_SetModuleCmdParams(GSMModuleCmd cmd)
{
    DB_t *DB = _GSM->DB;
    memset(&_GSM->header, 0, sizeof(Header_t));

    switch (cmd) 
    {
        case GSMModuleCmd_SMSSendCmd: {
            if ( !_GSM_isCorrectTelNumber(_GSM->SMSMode.currTelNumber) )
                return 0;
            _GSM_PushStringToHeader("AT+CMGS=\"");
            _GSM_PushStringToHeader(GSM_Prefix);
            _GSM_PushStringToHeader(_GSM->SMSMode.currTelNumber);
            _GSM_PushStringToHeader("\"\r");
            strcpy(_GSM->SMSMode.ExpectedStr, ">");
            _GSM->SMSMode.ExpectedTO = 5 * _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;            
        } break;
		case GSMModuleCmd_SMSSendContent: {
			 _GSM_PushStringToHeader("");
			strcpy(_GSM->SMSMode.ExpectedStr, "+CMGS:");
			_GSM->SMSMode.ExpectedTO = 60 * _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;
		} break;	
        case GSMModuleCmd_SMSRead: {
            _GSM_PushStringToHeader("AT+CMGL=\"REC UNREAD\"\r\n");
            // strcpy(_GSM->SMSMode.ExpectedStr, "+CMGL");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = 20 * _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;
        } break;
        case GSMModuleCmd_GetBalance: {
			_GSM_PushStringToHeader("AT+CUSD=1,\"#100#\"\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = 5 * _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;
        } break;
        case GSMModuleCmd_GetSignalLvl: {
            _GSM_PushStringToHeader("AT+CSQ\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "+CSQ: ");
            _GSM->SMSMode.ExpectedTO = _GSM_DEFAULT_ANS_DELAY / _GSM_DEFAULT_ANS_DELAY;
        } break;
		case GSMModuleCmd_PowerOn: {
			_GSM_PushStringToHeader("AT\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "SMS Ready");
            _GSM->SMSMode.ExpectedTO = 10 * _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;
		} break;
		case GSMModuleCmd_AT: {
            _GSM_PushStringToHeader("AT\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = _GSM_DEFAULT_ANS_DELAY / _GSM_DEFAULT_ANS_DELAY;
		} break;
        case GSMModuleCmd_ATE0: {
            _GSM_PushStringToHeader("ATE0\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = _GSM_DEFAULT_ANS_DELAY / _GSM_DEFAULT_ANS_DELAY;
        } break;
        case GSMModuleCmd_CREG: {
            _GSM_PushStringToHeader("AT+CREG?\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "CREG: 0,1");
            _GSM->SMSMode.ExpectedTO = _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;
        } break;
		case GSMModuleCmd_CPAS: {
		    _GSM_PushStringToHeader("AT+CPAS\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "CPAS: 0");
            _GSM->SMSMode.ExpectedTO = _GSM_DEFAULT_ANS_DELAY / _GSM_DEFAULT_ANS_DELAY;		
		} break;
        case GSMModuleCmd_DELALL: {
            // _GSM_PushStringToHeader("AT+CMGDA=\"DEL ALL\"\r\n");
			_GSM_PushStringToHeader("AT+CMGD=1,4\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = 25 * _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;
        } break;
        case GSMModuleCmd_GSMBUSY: {
            _GSM_PushStringToHeader("AT+GSMBUSY=1\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = _GSM_DEFAULT_ANS_DELAY;
        } break;
        case GSMModuleCmd_CMGF: {
            _GSM_PushStringToHeader("AT+CMGF=1\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = _GSM_DEFAULT_ANS_DELAY / _GSM_DEFAULT_ANS_DELAY;
        } break;
		case GSMModuleCmd_TestCLI: {
            _GSM_PushStringToHeader("\r\n\x1B\r\nAT\r\n");
            strcpy(_GSM->SMSMode.ExpectedStr, "OK");
            _GSM->SMSMode.ExpectedTO = 5 * _GSM_SECOND_DELAY / _GSM_DEFAULT_ANS_DELAY;
		} break;
        default: {
            return 0;
        } break;
    }
    return 1;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_TestModuleAns
 *
 * @brief: проверка состояния ответа модуля
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return результат ответа
*/ 
static ModAnsRes _GSM_TestModuleAns(void)
{
	ModAnsRes res = ModAnsRes_No;

	if (_GSM_isCorrectModuleAns()) {
		to_delete(&_GSM->TOQueue, &_GSM->CommonTOItem);
		res = ModAnsRes_OK;
	} else if (_GSM->CommonTO) {
		if ( _GSM->SMSMode.ExpectedTO-- ) {
			to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, _GSM_DEFAULT_ANS_DELAY); 
		} else {
			res = ModAnsRes_Timeout;
		}		
	}
	_GSM->CommonTO = 0;

	return res;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_isCorrectModuleAns
 *
 * @brief: проверка ответа модуля GSM
 * 
 * NOTE: выключает чтение и очищает буфер приема. Устанавливает _GSM->SMSMode.CommonBuffer.it 
 *  на начало ожидаемой строки
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 *      1 - ответ совпадает с ожидаемым
 *      0 - другой ответ (или его нет)
*/ 
static int _GSM_isCorrectModuleAns(void)
{        
    memcpy(_GSM->SMSMode.CommonBuffer.data, _GSM->RxBuffer, USARTHandler_BUFFER_SIZE);

    _GSM->SMSMode.CommonBuffer.it = _GSM_strstr(
			(char*)_GSM->SMSMode.CommonBuffer.data, 
			USARTHandler_BUFFER_SIZE, 
			_GSM->SMSMode.ExpectedStr
	);
    if (_GSM->SMSMode.CommonBuffer.it) {
        return 1;
    }
    return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_isUnreadSMS
 *
 * @brief: Проверка наличия непрочитанных SMS
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 *      1 - есть непрочитанные
 *      0 - нет
*/ 
static int _GSM_isUnreadSMS(void)
{
	if ( _GSM_strstr((char*)_GSM->SMSMode.CommonBuffer.data, USARTHandler_BUFFER_SIZE, "REC UNREAD") ) {
		 _GSM->SMSMode.CommonBuffer.it = _GSM_strstr(
			(char*)_GSM->SMSMode.CommonBuffer.data, 
			USARTHandler_BUFFER_SIZE, 
			"+CMGL"
		);
		if (_GSM->SMSMode.CommonBuffer.it) {
			return 1;
		}
	}	
	return 0;	
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_HandleModuleCmdRes
 *
 * @brief: Чтение значений из ответа модуля на команду и их обработка
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 *      1 - ошибка чтения
 *      0 - значения считаны
*/ 
static int _GSM_HandleModuleCmdRes(GSMModuleCmd cmd)
{
    DB_t *DB = _GSM->DB;
    if ( !_GSM->SMSMode.CommonBuffer.it )
        return 1;
        
    switch (cmd)
    {
        case GSMModuleCmd_GetBalance: {
            float tmp;
            int res = sscanf(_GSM->SMSMode.CommonBuffer.it, "Balance:%fr", &tmp);
            if (res != 0) {
                GSM_Balance = tmp;
                *(_GSM->IntCommandStatus) = ModCmdSts_OK;
                return 0;
            }
        } break;
        case GSMModuleCmd_GetSignalLvl: {
            float tmp;
            int res = sscanf(_GSM->SMSMode.CommonBuffer.it, "+CSQ: %f", &tmp);
            if (res != 0) {
                GSM_SignalLevel = (int)tmp;
                *(_GSM->IntCommandStatus) = ModCmdSts_OK;
                return 0;
            }
        } break;
        default: {} break;
    }
    *(_GSM->IntCommandStatus) = ModCmdSts_FAIL;
    return 1;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_GetNextInternalCommand
 *
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return приоритетная команда на обработку
*/ 
static GSMIntCmd _GSM_GetNextInternalCommand(void)
{
    for (GSMIntCmd c = GSMIntCmd_tStart; c != GSMIntCmd_tEnd; ++c) {
        if (_GSM->IntCommands[c]) {
			_GSM->IntCommands[c] = 0;
            return c;
        }
    }
    return GSMIntCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_GetNextNotification
 *
 * @brief: просмотр событий
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 *      1 - телефон установлен
 *      0 - нет телефона на отправку
*/ 
static int _GSM_SetNotifTelNumber(GSMNotif notif)
{
    DB_t *DB = _GSM->DB;
    int idx = -1;

    switch (notif) {
        case GSMNotif_Error: 
        {
            while (
                _GSM->SMSMode.currTelNumOutIdx != GSM_NUMBERS_OUT_SIZE &&
                !_GSM_isCorrectTelNumber(GSM_NumbersOut[_GSM->SMSMode.currTelNumOutIdx])                
            ) {
                _GSM->SMSMode.currTelNumOutIdx++;
            }
            // больше нет номеров 
            if (_GSM->SMSMode.currTelNumOutIdx == GSM_NUMBERS_OUT_SIZE) {
                // _GSM->SMSMode.currTelNumOutIdx = 0; // сброс
                return 0;
            }            
            // установка номера на отправку
            strcpy(_GSM->SMSMode.currTelNumber, GSM_NumbersOut[_GSM->SMSMode.currTelNumOutIdx]);
			_GSM->SMSMode.currTelNumOutIdx++;
            return 1;
        } break;
        case GSMNotif_Start:
        case GSMNotif_Stop:
        case GSMNotif_DeniedStart:
        case GSMNotif_DeniedStop:
        {
            switch (notif) {
                case GSMNotif_DeniedStart:
                case GSMNotif_Start: 
                { 
                    idx = _GSM->telNumForRespIdxs[GSMExtCmd_Start]; 
                    _GSM->telNumForRespIdxs[GSMExtCmd_Start] = -1;
                } break;
                case GSMNotif_DeniedStop: 
                case GSMNotif_Stop: 
                { 
                    idx = _GSM->telNumForRespIdxs[GSMExtCmd_Stop]; 
                    _GSM->telNumForRespIdxs[GSMExtCmd_Stop] = -1;
                } break;
            }            
            if (idx != -1) {
                // установка номера на отправку
                strcpy(_GSM->SMSMode.currTelNumber, GSM_NumbersIn[idx]);
                return 1;
            }
        } break;
        default: { /* nop */ } break;
    }
    return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_GetNextNotification
 *
 * @brief: просмотр событий
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return приоритетное событие на уведомление
*/ 
static GSMNotif _GSM_GetNextNotification(void)
{
    DB_t *DB = _GSM->DB;

    int errors = 0;
    // Проверка ошибок
    for (Error e = (Error)((int)Error_No+1); e != Error_tEnd; ++e) {
        // ошибка в списке
        if (ErrorManager_isPushed(e)) {
            // ошибка не фиксировалась ранее
            if (!_GSM->lastErrors[e]) {
                _GSM->lastErrors[e] = 1;
                _GSM->errorsToHandle[e] = 1;
                errors = 1;
            }
        } else {
            _GSM->lastErrors[e] = 0;
            _GSM->errorsToHandle[e] = 0;
        }
        // есть что обслуживать
        if (_GSM->errorsToHandle[e])
            errors = 1;
    }
    // есть ошибка
    if (errors) 
        return GSMNotif_Error;

    // ожидание завершения старта
    if (_GSM->Notifications[GSMNotif_Start]) {
        if (Burner_CurrentState == BurnerState_RegularMode) {  
            _GSM->Notifications[GSMNotif_Start] = 0;
            return GSMNotif_Start;
        } else if (Security_WorkBlock == (int)WorkBlock_Fatal) {
            _GSM->Notifications[GSMNotif_Start] = 0;
            return GSMNotif_DeniedStart;
        }   
    }

    // ожидание завершения останова
    if (_GSM->Notifications[GSMNotif_Stop]) {
        if (Burner_CurrentState == BurnerState_Waiting) {
            _GSM->Notifications[GSMNotif_Stop] = 0;
            return GSMNotif_Stop;
        } else if (Security_WorkBlock == (int)WorkBlock_Fatal) {
            _GSM->Notifications[GSMNotif_Stop] = 0;
            return GSMNotif_DeniedStop;
        }  
    }

    return GSMNotif_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_HandleInternalCommand
 *
 * @brief:
 * 
 * input parameters
 * command - команда на обработку
 * 
 * output parameters
 * 
 * return команда GSM модулю
*/ 
static GSMModuleCmd _GSM_HandleInternalCommand(GSMIntCmd command)
{
    switch (command) {
        case GSMIntCmd_GetBalance: {
            *(_GSM->IntCommandStatus) = ModCmdSts_RESET; 
            return _GSM_SetModuleCmdParams(GSMModuleCmd_GetBalance)? 
                    GSMModuleCmd_GetBalance : GSMModuleCmd_No;
        } break;
        case GSMIntCmd_GetSignalLvl: {
            *(_GSM->IntCommandStatus) = ModCmdSts_RESET;   
            return _GSM_SetModuleCmdParams(GSMModuleCmd_GetSignalLvl)? 
                    GSMModuleCmd_GetSignalLvl : GSMModuleCmd_No;
        }
        default: { return GSMModuleCmd_No; } break;
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_ReadTelNum
 *
 * @brief: Чтение телефона из смс
 * 
 * input parameters
 * buf - буфер приема
 * 
 * output parameters
 * 
 * return индекс номера телефона из списка разрешенных
 *      * -1, если телефон не найден
*/
static int _GSM_ExternalCmd_ReadTelNum(const char *buf)
{
    // чтение тел номера, если есть
	const int telPos = 14;
	// +CMGR: "REC READ","+8613918186089","","02/01/30,20:40:31+00" 
    char *recUnread = _GSM_strstr(buf, USARTHandler_BUFFER_SIZE, ",\"REC UNREAD\",\"+");
    if (recUnread) {
        char telNumber[TELNUMB_BUF_SIZE];
		char endSym;
		if (sscanf(recUnread+telPos, "\"%[^\"]%c", telNumber, &endSym) == 2) {
			if (endSym == '\"' && telNumber[0] == '+') {
				// получение индекса тел номера		
				return _GSM_GetIdxOfTelNumberIn(telNumber);
			}
		}
                      
    }
	return -1;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_Start
 *
 * @brief: Обработка команды старт
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_Start()
{
    DB_t *DB = _GSM->DB;
	GSM_Cmd = GSMCmd_Start;
	_GSM->Notifications[GSMNotif_Start] = 1; // отслеживание старта
	return GSMModuleCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_Stop
 *
 * @brief: Обработка команды стоп
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_Stop()
{
    DB_t *DB = _GSM->DB;
	GSM_Cmd = GSMCmd_Stop;
	_GSM->Notifications[GSMNotif_Stop] = 1; // отслеживание стопа
	return GSMModuleCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_SetupREG
 *
 * @brief: Проверка и установка регистра
 * 
 * input parameters
 * REG - параметры регистра
 * 
 * output parameters
 * 
 * return	1 - успешно
 * 			0 - проверка не прошла
*/
static int _GSM_ExternalCmd_SetupREG(const REG_t *REG)
{
	CommandParams_t params;

	return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_TEHWorkPerm
 *
 * @brief: Обработка команды для ТЭН и формирование ответа
 * 		установка параметра TEH_WorkPermission
 * 
 * input parameters
 * startOfCmd - указатель на буфер с началом текста команды
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_TEHWorkPerm(const char *startOfCmd)
{
    DB_t *DB = _GSM->DB;

	return GSMModuleCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_TEHStateOnT
 *
 * @brief: Обработка команды установки температуры срабатывания ТЭН и формирование ответа
 * 		установка параметра TEH_StateOnT
 * 
 * input parameters
 * startOfCmd - указатель на буфер с началом текста команды
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_TEHStateOnT(const char *startOfCmd)
{
    DB_t *DB = _GSM->DB;
	// Teh t XX
	int t;
	char c;

	return GSMModuleCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_TTarget
 *
 * @brief: Обработка команды установки целевой температуры и формирование ответа
 * 		установка параметра AlgPrm_TTarget
 * 
 * input parameters
 * startOfCmd - указатель на буфер с началом текста команды
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_TTarget(const char *startOfCmd)
{
    DB_t *DB = _GSM->DB;
	// Target t XX
	int t;
	char c;

	return GSMModuleCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_FixPower
 *
 * @brief: Обработка команды установки фиксированной мощности и формирование ответа
 * 		установка параметра AlgPrm_FanPFix, AlgPrm_PFix
 * 
 * input parameters
 * startOfCmd - указатель на буфер с началом текста команды
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_FixPower(const char *startOfCmd)
{
    DB_t *DB = _GSM->DB;
	// Fix power p XX f YY
	int fan;
	float pwr;
	char c1, c2;

	return GSMModuleCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_GetInfo
 *
 * @brief: Обработка команды запроса информации о горелке
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_GetInfo()
{
	_GSM_PushBurnerInfoToMessage();
	_GSM_PushTailToMessage();
	return GSMModuleCmd_SMSSendCmd;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_SetREG
 *
 * @brief: Обработка команды установки регистра
 * 
 * input parameters
 * startOfCmd - указатель на буфер с началом текста команды
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_SetREG(const char *startOfCmd)
{
	CommandParams_t params;
	params.valid = _GSM_ReadREGParams(startOfCmd, REGCmd_SetREG, &params);
	REGCmd ans = _GSM_CheckExtCmdParams(GSMExtCmd_SetREG, &params);
	switch (ans) {
		case REGCmd_SetREG:  {
			_GSM_SetREGValue(&params.REG);
			_GSM_PushStringToMessage("Set OK");
		} break;
		default: {
			_GSM_PushStringToMessage("Denied Set");
		} break;
	}
	_GSM_PushTailToMessage();
	return GSMModuleCmd_SMSSendCmd;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ExternalCmd_GetREG
 *
 * @brief: Обработка команды чтения регистра
 * 
 * input parameters
 * startOfCmd - указатель на буфер с началом текста команды
 * 
 * output parameters
 * 
 * return команда модулю GSM
*/
static GSMModuleCmd _GSM_ExternalCmd_GetREG(const char *startOfCmd)
{
	CommandParams_t params;
	params.valid = _GSM_ReadREGParams(startOfCmd, REGCmd_GetREG, &params);
	REGCmd ans = _GSM_CheckExtCmdParams(GSMExtCmd_GetREG, &params);
	switch (ans) {
		case REGCmd_GetREG:  {
			char tmp[_GSM_REG_VAL_MAX_SIZE];
			_GSM_ReadREGValue(&params);
			_GSM_ConvertREGValueToString(&params.REG, tmp);
			_GSM_PushStringToMessage("Value: ");            
			_GSM_PushStringToMessage(tmp);
		} break;
		default: {
			_GSM_PushStringToMessage("Denied Get");
		} break;
	}
	_GSM_PushTailToMessage();
	return GSMModuleCmd_SMSSendCmd;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_HandleExternalCommand
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return команда GSM модулю
*/ 
static GSMModuleCmd _GSM_HandleExternalCommand(void)
{
    DB_t *DB = _GSM->DB;
	const char endSeq[] = "\r\n";
    char buffer[USARTHandler_BUFFER_SIZE];
    char *sCmd;
	char *eCmd;
	GSMModuleCmd cmd = GSMModuleCmd_No;

	/* копия буфера приема */
    memcpy(buffer, _GSM->RxBuffer, USARTHandler_BUFFER_SIZE);
	buffer[USARTHandler_BUFFER_SIZE-1] = 0;

	/* по приоритету (первый - высший), остальное выбрасывается */
	enum {
		_SMSCmd_Stop,
		_SMSCmd_Start,
		_SMSCmd_TEHWorkPerm,
		_SMSCmd_TEHStateOnT,
		_SMSCmd_TTarget,
		_SMSCmd_FixPowerSF,
		_SMSCmd_GetInfo,
		_SMSCmd_GetReg,
		_SMSCmd_SetReg,
	};
	const char *commands[] = {

	};
	
	return GSMModuleCmd_No;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_HandleNotification
 *
 * @brief:
 * 
 * input parameters
 * notif - уведомление на обработку
 * 
 * output parameters
 * 
 * return команда GSM модулю
*/ 
static GSMModuleCmd _GSM_HandleNotification(GSMNotif notif)
{
    memset(&_GSM->message, 0, sizeof(Message_t));

    switch (notif) {
        case GSMNotif_Error: {
            _GSM_PushErrorsToMessage();            
        } break;
        case GSMNotif_Start: {
            _GSM_PushStringToMessage("Started"); 
        } break;
        case GSMNotif_Stop: {
            _GSM_PushStringToMessage("Stopped"); 
        } break;
        case GSMNotif_DeniedStart: {
            _GSM_PushStringToMessage("Denied start"); 
        } break;
        case GSMNotif_DeniedStop: {
            _GSM_PushStringToMessage("Denied stop"); 
        } break;
        default: { return GSMModuleCmd_No; } break;
    }
    _GSM_PushTailToMessage();
    return GSMModuleCmd_SMSSendCmd; 
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_GetIdxOfTelNumberIn
 *
 * @brief: Проверка тел номера на совпадение из списка
 * 
 * input parameters
 * telNum - номер телефона "+..." с кодом страны
 * 
 * output parameters
 * 
 * return
 *      * индекс номера из GSM_NumbersIn, если телефон найден
 *      * -1, если телефон не найден
*/ 
static int _GSM_GetIdxOfTelNumberIn(const char *telNum)
{
    DB_t *DB = _GSM->DB;

	char fullTelNum[TELNUMB_BUF_SIZE];
    for (int i = 0; i < GSM_NUMBERS_IN_SIZE; ++i) {
		strcpy(fullTelNum, GSM_Prefix);
		strcpy(fullTelNum + strlen(fullTelNum), GSM_NumbersIn[i]);
		if ( strlen(telNum) != strlen(fullTelNum) )
			continue;
        if ( strcmp(telNum, fullTelNum) == 0 )
            return i;
	}
    return -1;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_isCorrectTelNumber
 *
 * @brief: Проверка строки на корректное содержание тел. номера
 * 
 * input parameters
 * telNum - номер телефона без кода страны
 * 
 * output parameters
 * 
 * return
 *      * 0 - номер не корректен
 *      * 1 - корректный номер
*/ 
static int _GSM_isCorrectTelNumber(const char *telNum)
{
	int size = strlen(telNum);
	if (size) {
		for (int i = 0; i < size; ++i) {
			if (telNum[i] != ' ' && telNum[i] != '0')
				return 1;
		}
	}
	return 0;
}




/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_SetREGValue
 *
 * @brief: Установка значения регистра
 * 
 * input parameters
 * REG - регистр для установки
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_SetREGValue(const REG_t *REG)
{
    DB_t *DB = _GSM->DB;
    switch (REG->t) {
        case REG_PFH: { DB->E.Params_floatHE[REG->n] = REG->VAL.f; } break;
        case REG_PIH: { DB->E.Params_intHE[REG->n] = REG->VAL.d; } break;
        case REG_PCH: { strcpy( DB->E.Params_charHE_16[REG->n], REG->VAL.c); } break;
        case REG_PCI: { strcpy( DB->E.Params_charIE_16[REG->n], REG->VAL.c); } break;
		case REG_SII: { DB->Signals_intI[REG->n] = REG->VAL.d; } break; 
		case REG_SIE: { DB->E.Signals_intIE[REG->n] = REG->VAL.d; } break;
		case REG_SFI: { DB->Signals_floatI[REG->n] = REG->VAL.f; } break;
        case REG_DFH: { DB->Debug_floatH[REG->n] = REG->VAL.f; } break;
        case REG_DFI: { DB->Debug_floatI[REG->n] = REG->VAL.f; } break;
        case REG_DIH: { DB->Debug_intH[REG->n] = REG->VAL.d; } break;
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ReadREGValue
 *
 * @brief: Установка значения регистра
 * 
 * input parameters
 * 
 * output parameters
 * params - параметры регистра для установки
 * 
 * no return
*/ 
static void _GSM_ReadREGValue(CommandParams_t *params)
{     
    DB_t *DB = _GSM->DB;
    switch (params->REG.t) {
        case REG_SII: params->REG.VAL.d = DB->Signals_intI[params->REG.n]; break;
        case REG_SIE: params->REG.VAL.d = DB->E.Signals_intIE[params->REG.n]; break;
        case REG_SFI: params->REG.VAL.f = DB->Signals_floatI[params->REG.n]; break;
        case REG_PIH: params->REG.VAL.d = DB->E.Params_intHE[params->REG.n]; break;
        case REG_PFH: params->REG.VAL.f = DB->E.Params_floatHE[params->REG.n]; break;
        case REG_PCH: strcpy(params->REG.VAL.c, DB->E.Params_charHE_16[params->REG.n]); break;
        case REG_PCI: strcpy(params->REG.VAL.c, DB->E.Params_charIE_16[params->REG.n]); break;
        case REG_DIH: params->REG.VAL.d = DB->Debug_intH[params->REG.n]; break;
        case REG_DFH: params->REG.VAL.f = DB->Debug_floatH[params->REG.n]; break;
        case REG_DFI: params->REG.VAL.f = DB->Debug_floatI[params->REG.n]; break;
    }                    
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_CheckExtCmdParams
 *
 * @brief: Проверка полученных параметров
 * 
 * input parameters
 * command - поступившая команда (чтение или запись)
 * params - параметры
 * 
 * output parameters
 * 
 * return результат проверки: установка, чтение параметров или отказ
*/ 
static REGCmd _GSM_CheckExtCmdParams(GSMExtCmd command, const CommandParams_t *params)
{
    DB_t *DB = _GSM->DB;

    switch (command) {
        case GSMExtCmd_SetREG: 
        {
            if (!params || !params->valid) return REGCmd_Denied;

            switch (params->REG.t) {
                case REG_PFH: {
                    if (params->REG.n >= PARAMS_FLOATHE_SIZE)
                        return REGCmd_Denied;
                    if (params->REG.VAL.f > DB->L.Params_floatHE_Max[params->REG.n] || params->REG.VAL.f < DB->L.Params_floatHE_Min[params->REG.n])
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
                } break;
                case REG_PIH: {
                    if (params->REG.n >= PARAMS_INTHE_SIZE)
                        return REGCmd_Denied;
                    if (params->REG.VAL.d > DB->L.Params_intHE_Max[params->REG.n] || params->REG.VAL.d < DB->L.Params_intHE_Min[params->REG.n])
                        return REGCmd_Denied;                    
                    return REGCmd_SetREG;
                } break;
                case REG_PCH: {
                    if (params->REG.n >= PARAMS_CHARHE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
                } break;
                case REG_PCI: {
                    if (params->REG.n >= PARAMS_CHARIE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
                } break;

                case REG_SII: {
                    if (params->REG.n >= SIGNALS_INTI_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
                } break;
                case REG_SIE: {
                    if (params->REG.n >= SIGNALS_INTIE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
                } break;
                case REG_SFI: {
                    if (params->REG.n >= SIGNALS_FLOATI_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
                } break;
				
				case REG_DFH: {
                    if (params->REG.n >= DEBUG_FLOATH_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
				} break;
				case REG_DIH: {
                    if (params->REG.n >= DEBUG_INTH_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
				} break;
				case REG_DFI: {
                    if (params->REG.n >= DEBUG_FLOATI_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_SetREG;
				} break;

                default: { 
                    return REGCmd_Denied; 
                } break;
            }
        } break;
        case GSMExtCmd_GetREG: 
        {
            if (!params || !params->valid) return REGCmd_Denied;

            switch (params->REG.t) {
                case REG_PFH: {
                    if (params->REG.n >= PARAMS_FLOATHE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
                } break;
                case REG_PIH: {
                    if (params->REG.n >= PARAMS_INTHE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
                } break;
                case REG_PCH: {
                    if (params->REG.n >= PARAMS_CHARHE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
                } break;
                case REG_PCI: {
                    if (params->REG.n >= PARAMS_CHARIE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
                } break;

                case REG_SII: {
                    if (params->REG.n >= SIGNALS_INTI_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
                } break;
                case REG_SFI: {
                    if (params->REG.n >= SIGNALS_FLOATI_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
                } break;
                case REG_SIE: {
                    if (params->REG.n >= SIGNALS_INTIE_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
                } break;
				
				case REG_DFH: {
                    if (params->REG.n >= DEBUG_FLOATH_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
				} break;
				case REG_DIH: {
                    if (params->REG.n >= DEBUG_INTH_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
				} break;
				case REG_DFI: {
                    if (params->REG.n >= DEBUG_FLOATI_SIZE)
                        return REGCmd_Denied;
                    return REGCmd_GetREG;
				} break;

                default: { 
                    return REGCmd_Denied; 
                } break;
            }
        } break;
        default: 
        {
            return REGCmd_Denied;
        } break;
    }
}



#define ShiftStrPtrAndCheck(p, start, max, out) {	\
	p += strlen(p);									\
	if ((p-start) > max)							\
		goto out;									\
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_PushBurnerInfoToMessage
 *
 * @brief: Заполнение сообщения информацией по горелке
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_PushBurnerInfoToMessage(void) 
{
	#define sTmp_size 256
    DB_t *DB = _GSM->DB;
    char sTmp[sTmp_size];
	memset(sTmp, 0, sTmp_size);
	char *it = sTmp;

	sprintf(it, "P=%02.00fkW\nTr:", OutSig_ScrewPOut);
	ShiftStrPtrAndCheck(it, sTmp, sTmp_size, exit);

	
exit:	
	if (it)
    	_GSM_PushStringToMessage(sTmp);
    _GSM_PushTailToMessage();
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_PushStringToHeader
 *
 * @brief: Добавление строки в заголовок SMS
 * 
 * input parameters
 * str - отправляемая строка
 * 
 * output parameters
 * 
 * return число обработаных ошибок
*/ 
static void _GSM_PushStringToHeader(const char *str)
{
    uint8_t len = strlen(str);
	if (len+_GSM->header.curPos >= _GSM_HEADER_BUFFER_SIZE)
		return;
    memcpy(
        _GSM->header.buffer + _GSM->header.curPos,
        str,
        len
    );
    _GSM->header.curPos += len;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_PushErrorsToMessage
 *
 * @brief: Добавление ошибок в сообщение
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return число обработаных ошибок
*/ 
static int _GSM_PushErrorsToMessage(void)
{
    int cnt = 0;
    for (Error e = (Error)((int)Error_No+1); e != Error_tEnd; ++e) {
        if (_GSM->errorsToHandle[e]) {
            if (cnt < _GSM_MAX_ERRORS_PER_MESSAGE) {
                _GSM->errorsToHandle[e] = 0;
                _GSM_PushErrorToMessage(e);
                cnt++;
            } else {
                break;
            }
        }
    }    
    return cnt;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_PushStringToMessage
 *
 * @brief: Добавление строки в сообщение
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_PushStringToMessage(const char *str)
{
    uint8_t len = strlen(str);
	if (len+_GSM->message.curPos >= _GSM_MESSAGE_BUFFER_SIZE)
		return;
    memcpy(
        _GSM->message.buffer + _GSM->message.curPos,
        str,
        len
    );
    _GSM->message.curPos += len;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_PushErroCodeToMessage
 *
 * @brief: Добавление кода ошибки в сообщение
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_PushErroCodeToMessage(ErrorCode code)
{
    char *buf = (char*)_GSM->message.buffer;
	if (2+_GSM->message.curPos >= _GSM_MESSAGE_BUFFER_SIZE)
		return;
    sprintf(
        buf + _GSM->message.curPos,
        "%.2d",
        code
    );
    _GSM->message.curPos = strlen(buf);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ConvertREGValueToString
 *
 * @brief: Преобразование занчение регистра в строку
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_ConvertREGValueToString(const REG_t *REG, char *str)
{
	switch (REG->t) {
        case REG_PFH: 
        case REG_SFI:
		case REG_DFH:
		case REG_DFI:
        {
            sprintf(str, "%4.2f", REG->VAL.f);
        } break;
        case REG_PIH: 
        case REG_SII:
        case REG_SIE:
		case REG_DIH:
        {
            sprintf(str, "%d", REG->VAL.d);
        } break;
        case REG_PCH: 
        case REG_PCI: 
        {
            strcpy(str, REG->VAL.c);
        } break;
        default: { /* nop */ } break;
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_PushErrorToMessage
 *
 * @brief: Добавление кода ошибки и описание в сообщение
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_PushErrorToMessage(Error error)
{			
    _GSM_PushStringToMessage("ERROR ");
    _GSM_PushErroCodeToMessage( ErrorManager_GetCode(error) );
    _GSM_PushStringToMessage(": ");

	switch ( error ) {

	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_ReadREGParams
 *
 * @brief: Чтение регистровых параметров команды из входящего сообщения
 * 
 * input parameters
 * buf - начало команды ("Get" / "Set")
 * act - действие с регистром
 * 
 * output parameters
 * params - параметры команды
 * 
 * return
 *      * 1 - успешное чтение
 *      * 0 - ошибка в параметрах
*/ 
static int _GSM_ReadREGParams(const char *buf, REGCmd act, CommandParams_t *params)
{
    if (!buf || !params) return 0;
    if (strlen(buf) < 8) return 0; // как минимум '*et XXXY' * = S/G

    int state = 0;
    int it = 4; // 'set'/'get' + ' '

    while (1) {
        switch (state) {
            
        }
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_PushTailToMessage
 *
 * @brief: Добавление завершающей последовательности в сообщение
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _GSM_PushTailToMessage(void)
{
    if (_GSM->GSM_Attached) {
        _GSM_PushStringToMessage("\x1A");
    } else {
        _GSM_PushStringToMessage("\r\n");
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_SendModuleCmd
 *
 * @brief: Отправка команды модулю GSM (header)
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _GSM_SendModuleCmd(void)
{
    USARTHandler_SendBuffer(_GSM->header.buffer, _GSM->header.curPos);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_SetModuleAnsWaitParams
 *
 * @brief: Установка параметров на ожидание ответа от модуля GSM (вкл. чтение, уст. ТА)
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _GSM_SetModuleAnsWaitParams()
{            
    USARTHandler_ForceReadEnd();
    memset(_GSM->RxBuffer, 0, USARTHandler_BUFFER_SIZE);
    USARTHandler_StartRead(0, USARTHandler_BUFFER_SIZE);
    to_append(&_GSM->TOQueue, &_GSM->CommonTOItem, 0); // в @ref _GSM_TestModuleAns должно зайти по первому запросу
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_SendSMSContent
 *
 * @brief: Отправка содержания сообщения
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
static void _GSM_SendSMSContent(void)
{
	USARTHandler_SendBuffer(_GSM->message.buffer, _GSM->message.curPos);    
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _GSM_UHCB
 *
 * @brief: USART handler Callback завершения приема / ошибки прием
 * 
 * NOTE: успешное завершение говорит о переполнении буфера.
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void _GSM_UHCB(void)
{
    USARTHandler_ForceReadEnd();
    memset(_GSM->RxBuffer, 0, USARTHandler_BUFFER_SIZE);
    USARTHandler_StartRead(0, USARTHandler_BUFFER_SIZE); // данные берутся напрямую, перекладывать не следует
}



static char* _GSM_strstr(const char *_S, uint16_t _S_SIZE, const char *_P)
{	
	const char *start = 0;
	for (uint16_t i = 0; i < _S_SIZE; ++i) {
		if (_S[i] != 0) {
			start = _S + i;
			break;
		}
	}
	return start? strstr(start, _P) : 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GSM_Initialization
 * 
 * @brief: Инициализация GSM (драйверов weh1602, key)
 *
 * NOTE: Вывод загрузочного экрана после инициализации
 * 
 * input parameters
 * DB - указатель на базу параметров и сигналов
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int GSM_Initialization(DB_t *DB)
{
	if (!_GSM || !DB) return 1;
		
	_GSM->DB = DB;
	
	/* Инициализация объектов обсулижвания таймаутов */
	to_initque(&_GSM->TOQueue);
	to_inititem(&_GSM->CommonTOItem, 0, TurnID_Common);		

    if (USARTHandler_Create()) return 1;
    if (USARTHandler_Initialization(_GSM_UHCB, _GSM_UHCB)) return 1;

    _GSM->RxBuffer = USARTHandler_GetRxBuffer();
    
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(GSM_PWR_CONT_CLK, ENABLE);  
	RCC_AHB1PeriphClockCmd(GSM_PWR_KEY_CLK, ENABLE);  
    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // OUT
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Pin = GSM_PWR_CONT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GSM_PWR_CONT_PORT, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GSM_PWR_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GSM_PWR_KEY_PORT, &GPIO_InitStructure); 

	// module off
	_GSM_POWER_CONT_OFF();
	_GSM_POWER_KEY_UP();

    return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GSM_Create
 *
 * @brief: Создание экземпляра GSM
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
int GSM_Create(void)
{
	_GSM = (GSM_t *)calloc(1, sizeof(GSM_t));
	return (_GSM)? 0 : 1;
}
