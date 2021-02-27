/******************************************************************************
  * @file    retain.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    19.10.2018
  *****************************************************************************/

#include "eeprom.h"
#include "retain.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "utils.h"
#include "servto.h"
#include <stdlib.h>



#define MEMORY_VERSION			13

#define TASK_WRITE_DELAY        2
#define TASK_READ_DELAY         5

#define TASK_WRITE_CNT_MAX		10
#define TASK_READ_CNT_MAX		10



typedef enum {
    Operation_No,
    Operation_Read,
    Operation_Write,
    Operation_Clear,
} Operation;



#define EEPROM_PAGE0_ADDR       0
#define EEPROM_PAGE1_ADDR       256

#define EEPROM_CNT0_ADDR        0
#define EEPROM_CNT1_ADDR        256
#define EEPROM_CNTX_SIZE        1

#define EEPROM_CRC0_ADDR        254
#define EEPROM_CRC1_ADDR        510
#define EEPROM_CRCX_SIZE        2

#define EEPROM_MEMVER0_ADDR		253
#define EEPROM_MEMVER1_ADDR		509
#define EEPROM_MEMVERX_SIZE		1

#define EEPROM_DATA0_START_ADDR     1
#define EEPROM_DATA1_START_ADDR     257
#define EEPROM_DATAX_SIZE           211
#define EEPROM_DATAX_MAX			252

#define EEPROM_CRCX_DATA_SIZE		254



// todo @ref EEPROM_DATAX_SIZE -->> @ref MEMORY_VERSION
// call memcnt.sh
#pragma message ("Do not forget to change EEPROM_DATAX_SIZE (FYI call memcnt.sh)")
#if (EEPROM_DATAX_SIZE > EEPROM_DATAX_MAX)
#error "EEPROM_DATAX_SIZE > EEPROM_DATAX_MAX"
#endif



typedef union {
	u16 d;
	u8 b[2];
} uint16_str;



typedef struct {
	DBE_t copyDB;
    DBL_t *DBL;
    uint8_t eeprom[EE_TOTAL_SIZE_BYTE];
	uint8_t CompressedCopyDB[EEPROM_DATAX_SIZE];
    uint8_t CNT;

    Retain_State state;
    Retain_RxResult rxResult;
    QueueHandle_t TaskQueue;
    Operation operation;
    /* Обслуживание таймаутов */
	to_que_t TOQueue;	/* очередь таймаутов */
	to_item_t CommonTOItem;
    u8 nextPageToWrite;
} Retain_t;



static Retain_t *_Retain;



static void _Retain_FromEEPROMToCopyDB(u16 start_addr);
static void _Retain_CompressDB();
static int _Retain_WaitWR();



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_WriteDB
 * 
 * @brief: Запись базы параметров и сигналов в eeprom
 * 
 * input parameters
 * DB - база параметров и сигналов
 * 
 * output parameters
 * 
 * return текущее состояние потока 
*/
extern Retain_State Retain_WriteDB(const DBE_t *DB)
{
    if (_Retain->state == Retain_Busy) return Retain_Busy;

    BaseType_t ret;  

    _Retain->operation = Operation_Write;
    _Retain->state = Retain_Busy;    
    
    ret = xQueueSendToBack(_Retain->TaskQueue, &_Retain->operation, 0);
    if (ret == pdTRUE) {
        memcpy(&_Retain->copyDB, DB, sizeof(DBE_t));
    } else {
        _Retain->operation = Operation_No;
        _Retain->state = Retain_Idle;
    }

    return _Retain->state;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_ClearDB
 * 
 * @brief: Очистка eeprom
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return текущее состояние потока 
*/
Retain_State Retain_ClearDB()
{
    if (_Retain->state == Retain_Busy) return Retain_Busy;

    BaseType_t ret;
    
    _Retain->operation = Operation_Clear;
    _Retain->state = Retain_Busy;

    ret = xQueueSendToBack(_Retain->TaskQueue, &_Retain->operation, 0);
    if (ret != pdTRUE) {
        _Retain->operation = Operation_No;
        _Retain->state = Retain_Idle;
    }

    return _Retain->state;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_ReadDB
 * 
 * @brief: Чтение данных с eeprom
 * 
 * NOTE: Завершение операции можно получить вызвав @ref Retain_GetState;
 *  после того, как операция будет завершена, можно скопировать полученные данные
 *  @ref Retain_CopyReadDB
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return текущее состояние потока 
*/
extern Retain_State Retain_ReadDB()
{
    if (_Retain->state == Retain_Busy) return Retain_Busy;

    BaseType_t ret;
    
    _Retain->operation = Operation_Read;
    _Retain->state = Retain_Busy;

    ret = xQueueSendToBack(_Retain->TaskQueue, &_Retain->operation, 0);
    if (ret != pdTRUE) {
        _Retain->operation = Operation_No;
        _Retain->state = Retain_Idle;
    }

    return _Retain->state;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain
 * 
 * @brief: Задача по чтению / записи в eeprom
 * 
 * input parameters
 * v
 * 
 * output parameters
 * v
 * 
 * no return 
*/
void Retain_Task(void *v)
{
    BaseType_t ret;
    int res;

    while (1) {
        ret = xQueueReceive(_Retain->TaskQueue, &res, portMAX_DELAY);
        if (ret == pdTRUE) {
            switch (_Retain->operation) 
            {
                // ЧТЕНИЕ С EEPROM ----------------------------------------------------------------------------
                case Operation_Read:
                {
					int cnt_read = 0;
                    int State = 0;
                    int work = 1;
                    _Retain->rxResult = Retain_RxError;

                    while (work) {
                        switch (State)
                        {
                            // установка на чтение с eeprom
                            case 0: {
                                res = EE_Read(0, _Retain->eeprom, EE_TOTAL_SIZE_BYTE);
                                if (!res) { 
                                    State = 1; 
                                } else { 
                                    work = 0; 
                                }
                            } break;
                            // проверка завершения чтения
                            case 1: {
                                if ( EE_isReadComplete() ) {                               
                                    uint16_t crc0 = fast_crc16(0, _Retain->eeprom, EEPROM_CRCX_DATA_SIZE);
                                    uint16_t crc1 = fast_crc16(0, _Retain->eeprom+EEPROM_CNT1_ADDR, EEPROM_CRCX_DATA_SIZE);
                                    uint16_t rx_crc0 = _Retain->eeprom[EEPROM_CRC0_ADDR] + (_Retain->eeprom[EEPROM_CRC0_ADDR+1] << 8);
                                    uint16_t rx_crc1 = _Retain->eeprom[EEPROM_CRC1_ADDR] + (_Retain->eeprom[EEPROM_CRC1_ADDR+1] << 8);
                                    int cnt0 = -1;
                                    int cnt1 = -1;
                                    if (crc0 == rx_crc0) {
                                        cnt0 = _Retain->eeprom[EEPROM_CNT0_ADDR];
                                    }
                                    if (crc1 == rx_crc1) {
                                        cnt1 = _Retain->eeprom[EEPROM_CNT1_ADDR];
                                    }
                                    if (cnt0 == -1 && cnt1 == -1) {			
										// u8 isEmpty = 1;					
										// // проверка памяти на заводское состояние										
										// for (u16 i = 0; i < EE_TOTAL_SIZE_BYTE; ++i) {
										// 	if ( _Retain->eeprom[i] != 0xFF) {
										// 		isEmpty = 0;
										// 		break;
										// 	}
										// }
										// // если не пуста, то ошибка crc										
                                        // _Retain->rxResult = (isEmpty)? Retain_Empty : Retain_RxError;
										_Retain->rxResult = Retain_RxError;
                                        _Retain->state = Retain_Complete;
                                        work = 0;
                                        break;
                                    }	     

                                    int start_addr = 0; // page
                                    if ( (cnt1 > cnt0) || (cnt0 == 255 && cnt1 == 0) ) {
                                        start_addr = EEPROM_DATA1_START_ADDR;
                                        _Retain->nextPageToWrite = 0;
                                        _Retain->CNT = cnt1;
                                    } else if (cnt0 != -1) {
                                        start_addr = EEPROM_DATA0_START_ADDR;
                                        _Retain->nextPageToWrite = 1;
                                        _Retain->CNT = cnt0;
                                    } else {
                                        start_addr = EEPROM_DATA1_START_ADDR;
                                        _Retain->nextPageToWrite = 0;
                                        _Retain->CNT = cnt1;
                                    }
									
									// проверка памяти на разницу версий
									if ( ((cnt0 > cnt1 && _Retain->eeprom[EEPROM_MEMVER0_ADDR] != MEMORY_VERSION) || 
										 (cnt1 > cnt0 && _Retain->eeprom[EEPROM_MEMVER1_ADDR] != MEMORY_VERSION)) 
										 ||
										 ((cnt1 == cnt0) && 
										 (_Retain->eeprom[EEPROM_MEMVER0_ADDR] != MEMORY_VERSION || _Retain->eeprom[EEPROM_MEMVER1_ADDR] != MEMORY_VERSION))
									) {
										_Retain->rxResult = Retain_Empty;
                                        _Retain->state = Retain_Complete;
                                        work = 0;
                                        break;
									}

                                    _Retain_FromEEPROMToCopyDB(start_addr);
                                    _Retain->state = Retain_Complete;
                    				_Retain->rxResult = Retain_OK;
                                    work = 0;
                                } else {
                                    vTaskDelay(TASK_READ_DELAY);
									cnt_read++;
									if (cnt_read > TASK_READ_CNT_MAX) {
										_Retain->state = Retain_Error;
										_Retain->rxResult = Retain_RxError;
										work = 0;
									}
                                }
                            } break;
                        }
                    }
                } break;


                // ЗАПИСЬ В EEPROM ----------------------------------------------------------------------------
                case Operation_Write:
                {   
					int State = 0;            
					int work = 1;         
                    int page = _Retain->nextPageToWrite;
                    uint8_t *pcopy = _Retain->CompressedCopyDB;
					u16 wrAddr, cntAddr, crcAddr, verAddr, curAddr;

					while (work) 
					{
						switch (State) 
						{
							// запись данных
							case 0: 
							{
								_Retain_CompressDB();
								if (page == 1) {
									_Retain->nextPageToWrite = 0;
									wrAddr = EEPROM_DATA1_START_ADDR;
									cntAddr = EEPROM_CNT1_ADDR;
									crcAddr = EEPROM_CRC1_ADDR;
									verAddr = EEPROM_MEMVER1_ADDR;
									curAddr = EEPROM_DATA0_START_ADDR;
								} else {
									_Retain->nextPageToWrite = 1;
									wrAddr = EEPROM_DATA0_START_ADDR;
									cntAddr = EEPROM_CNT0_ADDR;
									crcAddr = EEPROM_CRC0_ADDR;
									verAddr = EEPROM_MEMVER0_ADDR;
									curAddr = EEPROM_DATA1_START_ADDR;
								}
								State = 3; // предполагается что данные не менялись
								
								if (_Retain->eeprom[verAddr] != MEMORY_VERSION)
									State = 1;

								for (u16 i = 0; i < EEPROM_DATAX_SIZE; ++i, ++wrAddr, ++curAddr) {
									if (
										_Retain->eeprom[wrAddr] != pcopy[i]
										||
										_Retain->eeprom[curAddr] != pcopy[i]
									) {
										if ( !EE_WritePage(wrAddr, &(pcopy[i]), 1) ) {
											State = 1; // данные менялись
											_Retain->eeprom[wrAddr] = pcopy[i];
											if ( _Retain_WaitWR() ) {
												State = 2; // ошибка
												break;
											}
										} else {
											State = 2; // ошибка
											break;
										}
									}
								}
							} break;
							// запись VER, CNT и CRC
							case 1: 
							{
								// запись VER
								if (_Retain->eeprom[verAddr] != MEMORY_VERSION) {
									_Retain->eeprom[verAddr] = MEMORY_VERSION;
									if ( !EE_WritePage(verAddr, &(_Retain->eeprom[verAddr]), 1) ) {
										if ( _Retain_WaitWR() ) {
											State = 2; // ошибка
											break;
										}
									} else {
										State = 2; // ошибка
										break;
									}
								}

								// запись CNT
								_Retain->eeprom[cntAddr] = ++_Retain->CNT;
								if ( !EE_WritePage(cntAddr, &(_Retain->eeprom[cntAddr]), 1) ) {
									if ( _Retain_WaitWR() ) {
										State = 2; // ошибка
										break;
									}
								} else {
									State = 2; // ошибка
									break;
								}

								// вычисление CRC
								uint16_str crc;
								crc.d = fast_crc16(0, &(_Retain->eeprom[cntAddr]), EEPROM_CRCX_DATA_SIZE);
								memcpy(&(_Retain->eeprom[crcAddr]), crc.b, 2);
								// запись CRC
								if ( !EE_WritePage(crcAddr, &(_Retain->eeprom[crcAddr]), 2) ) {
									if ( _Retain_WaitWR() ) {
										State = 2; // ошибка
										break;
									}
								} else {
									State = 2; // ошибка
									break;
								}

								_Retain->state = Retain_Complete;
								work = 0;
							} break;
							// ошибка записи
							case 2: 
							{
								_Retain->state = Retain_Error;
								work = 0;
							} break;
							// запись не требуется
							case 3: 
							{
								_Retain->state = Retain_Complete;
								work = 0;
							}
						}
					}                    
                } break;


				// "ОЧИСТКА" EEPROM ----------------------------------------------------------------------------
               	case Operation_Clear: {
					memset(_Retain->eeprom, 0xFF, EE_TOTAL_SIZE_BYTE);

					_Retain->state = Retain_Complete;

					u16 addr = 0;
					for (int i = 0; i < 32; ++i) {
						if ( !EE_WritePage(addr, _Retain->eeprom, 16) ) {
							if ( _Retain_WaitWR() ) {
								_Retain->state = Retain_Error;
								break;
							}
						} else {
							_Retain->state = Retain_Error;
							break;
						}
						addr += 16;
					}
				} break;
                default: { /* nop */ } break;
            }
        }
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_CopyReadDB
 * 
 * @brief: копирование считанных данных
 * 
 * input parameters
 * 
 * output parameters
 * DB - база параметров и сигналов, хранимая в eeprom
 * 
 * return
 *      * 1 - ошибка при копировании
 *      * 0 - операция прошла успешно
*/
int Retain_CopyReadDB(DBE_t *DB)
{
    if (!DB) return 1;
    if (_Retain->state == Retain_Busy) return 1;
    memcpy(DB, &_Retain->copyDB, sizeof(DBE_t));
    return 0;
}



typedef union {
    float f;
    int i;
	u32 u;
} CELL;



static inline int _Retain_Shift_4(const uint8_t *dataRD)
{
    return
           (int)(((*(dataRD+0))<<0) + 
           ((*(dataRD+1))<<8) + 
           ((*(dataRD+2))<<16) + 
           ((*(dataRD+3))<<24));
}



static inline void _Retain_UnShift_4(CELL val, uint8_t *dataRD)
{
	u32 uval = val.u;
	*(dataRD+0) = (u8)(uval>>0);
	*(dataRD+1) = (u8)(uval>>8);
	*(dataRD+2) = (u8)(uval>>16);
	*(dataRD+3) = (u8)(uval>>24);
}



static void _Retain_FromEEPROMToCopyDB(u16 start_addr)
{
    DBL_t *DBL = _Retain->DBL;
    int i;
    u16 iter, size;

    // Signals_intIE
    iter = start_addr;
    for (i = 0; i < SIGNALS_INTIE_SIZE; ++i) {
        if (i >= Signals_intIE_4BYTE_START_IDX && i <= Signals_intIE_4BYTE_END_IDX) {
            size = sizeof(int);
            _Retain->copyDB.Signals_intIE[i] = _Retain_Shift_4(&(_Retain->eeprom[iter])); 
        } else {
            size = sizeof(uint8_t);
            _Retain->copyDB.Signals_intIE[i] = _Retain->eeprom[iter]; 
        }                                           
        iter += size;
    }

    // Params_intHE
    for (i = 0; i < PARAMS_INTHE_SIZE; ++i) {
        if (DBL->Params_intHE_Max[i] > 0xFF) {
            size = sizeof(int);
            _Retain->copyDB.Params_intHE[i] = _Retain_Shift_4(&(_Retain->eeprom[iter])); 
        } else {
            size = sizeof(uint8_t);
            _Retain->copyDB.Params_intHE[i] = _Retain->eeprom[iter]; 
        }                                          
        iter += size;
    }

    // Params_floatHE
    CELL cell;
    for (i = 0; i < PARAMS_FLOATHE_SIZE; ++i) { 
        cell.i = _Retain_Shift_4(&(_Retain->eeprom[iter]));    
        _Retain->copyDB.Params_floatHE[i] = cell.f;
        iter += sizeof(float);
    }

    // Params_charHE_16
    for (i = 0; i < GSM_TOTAL_NUMBERS; ++i) {
        for (int j = 0; j < TELEPHONE_NUMBER_SIZE; ++j) {
            _Retain->copyDB.Params_charHE_16[i][j] = _Retain->eeprom[iter+j];
        }
        iter += TELEPHONE_NUMBER_SIZE;
    }
	memcpy(_Retain->copyDB.Params_charHE_16[GSM_PREFIX_IDX], "+", 1);
	for (int j = 0; j < TELEPHONE_COUNTRY_CODE_SIZE; ++j) {
		_Retain->copyDB.Params_charHE_16[GSM_PREFIX_IDX][j+1] = _Retain->eeprom[iter+j];
	}
	iter += TELEPHONE_COUNTRY_CODE_SIZE;

    // Params_charIE_16
    for (int j = 0; j < FACTORY_NUMBER_SIZE; ++j) {
        _Retain->copyDB.Params_charIE_16[0][j] = _Retain->eeprom[iter+j];
    }    
}



static void _Retain_CompressDB()
{
    DBL_t *DBL = _Retain->DBL;
	uint8_t *buf = _Retain->CompressedCopyDB;
	int i;
	u32 iter, size;	
    CELL cell;

    // Signals_intIE
    iter = 0;
    for (i = 0; i < SIGNALS_INTIE_SIZE; ++i) {
        if (i >= Signals_intIE_4BYTE_START_IDX && i <= Signals_intIE_4BYTE_END_IDX) {
            size = sizeof(int);
			cell.i = _Retain->copyDB.Signals_intIE[i];
            _Retain_UnShift_4(cell, buf+iter); 
        } else {
            size = sizeof(uint8_t);
			buf[iter] = (uint8_t)_Retain->copyDB.Signals_intIE[i];
        }                                           
        iter += size;
    }

    // Params_intHE
    for (i = 0; i < PARAMS_INTHE_SIZE; ++i) {
        if (DBL->Params_intHE_Max[i] > 0xFF) {
            size = sizeof(int);
			cell.i = _Retain->copyDB.Params_intHE[i];
            _Retain_UnShift_4(cell, buf+iter); 
        } else {
            size = sizeof(uint8_t);
			buf[iter] = (uint8_t)_Retain->copyDB.Params_intHE[i];
        }                                          
        iter += size;
    }

    // Params_floatHE
    for (i = 0; i < PARAMS_FLOATHE_SIZE; ++i) { 
		cell.f = _Retain->copyDB.Params_floatHE[i];
        _Retain_UnShift_4(cell, buf+iter); 
        iter += sizeof(float);
    }

    // Params_charHE_16 
    for (i = 0; i < GSM_TOTAL_NUMBERS; ++i) {
        for (int j = 0; j < TELEPHONE_NUMBER_SIZE; ++j) {
			buf[iter+j] = (uint8_t)_Retain->copyDB.Params_charHE_16[i][j];
        }
        iter += TELEPHONE_NUMBER_SIZE;
    }
	for (int j = 0; j < TELEPHONE_COUNTRY_CODE_SIZE; ++j) {
		buf[iter+j] = (uint8_t)_Retain->copyDB.Params_charHE_16[GSM_PREFIX_IDX][j+1];
	}
	iter += TELEPHONE_COUNTRY_CODE_SIZE;

    // Params_charIE_16
    for (int j = 0; j < FACTORY_NUMBER_SIZE; ++j) {
		buf[iter+j] = (uint8_t)_Retain->copyDB.Params_charIE_16[0][j];
    }    	
}



int _Retain_WaitWR()
{
	int cnt = 0;
	while (1) {
		if ( !EE_isWriteComplete() ) {
			vTaskDelay(TASK_WRITE_DELAY);
			cnt++;
			if (cnt > TASK_WRITE_CNT_MAX)
				return cnt;
		} else {
			return 0;
		}
	} 
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_GetState
 * 
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return текущее состояние потока
*/
Retain_State Retain_GetState(void)
{
    switch (_Retain->state) {
        case Retain_Complete: {
            _Retain->state = Retain_Idle;
            return Retain_Complete;
        } break;
        case Retain_Error: {
            _Retain->state = Retain_Idle;
            return Retain_Error;
        } break;
        default: { return _Retain->state; } break;
    }
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_GetState
 * 
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return результат чтения памяти
*/
Retain_RxResult Retain_GetRxResult(void)
{
    return _Retain->rxResult;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_Initialization
 * 
 * @brief: Инициализация EE task
 * 
 * input parameters
 * DBL - лимиты базы параметров
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int Retain_Initialization(DBL_t *DBL)
{    
    if (!_Retain) return 1;
    if (!DBL) return 1;
    if (EE_Create()) return 1;
    if (EE_Initialization()) return 1;	
    _Retain->DBL = DBL;
    _Retain->TaskQueue = xQueueCreate(1, sizeof(int));
    BaseType_t res = xTaskCreate(Retain_Task, "Retain_Task", configMINIMAL_STACK_SIZE*6, NULL, Retain_PRIORITY, NULL);
    if (res != pdTRUE) return 1;
    if (!_Retain->TaskQueue) return 1;
    return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Retain_Create
 * 
 * @brief: Создание экземпляра Retain
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int Retain_Create(void)
{
	_Retain = (Retain_t *)calloc(1, sizeof(Retain_t));
	return (_Retain)? 0 : 1;
}