/******************************************************************************
  * @file    insig.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/
  
  
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "insig.h"
#include "servto.h"
#include "dsw18s20.h"



#define _INSIG_ADC_CONVERTION_TYPE 2



#define Fueljam_MEASURE_PERIOD	1000000
#define Fueljam_CONTROL_POINTS	3



#define POLL_PERIOD				2
#define REPEAT_DELAY			2



static const u32 DIAGNOSTIC_SENSORS_TIMEOUT = 2 * 1000000;



typedef enum {
	TurnID_Fueljam,
	TurnID_Diagnostic,
} TurnID;



typedef struct {
	to_item_t TOItem;
	to_que_t TOQueue;
} Diagnostic;



typedef struct {
	float instant;
	float average;
} SensorReadings;



typedef enum {
	ADCTurn_tStart,
	ADCTurn_Fueljam = ADCTurn_tStart,
	ADCTurn_Flame,
	ADCTurn_tEnd
} ADCTurn;



typedef struct {
	DB_t *DB;
	dsw_drv_t *dsw;
	uint8_t eventT[Sensor_tEnd];	
	uint8_t errorT[Sensor_tEnd];	
	
	/* Обслуживание таймаутов */
	to_que_t TOQueue;	/* очередь таймаутов */
	/* таймаут на измерение пламени */
	to_item_t FueljamTOItem;

	uint16_t ADCOriginalValues[2];	// попугаи
	uint16_t ADCConvertedValues[2];	// мВ

	uint8_t Fueljam_Cnt;

	/* диагностика датчиков */
	Diagnostic diag;

	/* показания датчиков */
	SensorReadings Fueljam;
	SensorReadings flame;

	ADCTurn ADC_turn;
	uint8_t ADCChannel_turn[ADCTurn_tEnd];
} InSig_t;



static const uint32_t _INSIG_DMA_FLAGS = INSIG_DMA_FLAG_FEIF | INSIG_DMA_FLAG_DMEIF | INSIG_DMA_FLAG_TEIF | INSIG_DMA_FLAG_HTIF | INSIG_DMA_FLAG_TCIF;



static InSig_t *_InSig = 0;




void _InSig_dswevent(u8 nloc, dsw_event_e nevt);
static void _InSig_InitFlameSensor(void);
static void _InSig_InitFueljamSensor(void);
static void _InSig_InitADC(void);
static void _InSig_InitGSM(void);
 



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: InSig_Work
 *
 * @brief: Основной метод InSig
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void InSig_Work(void)
{
	DB_t *DB = _InSig->DB;
	dsw_data_t data;
	dsw_info_t info;
	to_item_t *turn;

	/* Датчик затора -------------------------------------------------- */
	Sensors_FueljamInstant = _InSig->Fueljam.instant;
	Sensors_FueljamAverage = floorf(_InSig->Fueljam.average);
	/* Датчик пламени ------------------------------------------------- */
	Sensors_FlameInstant = _InSig->flame.instant;
	Sensors_FlameAverage = floorf(_InSig->flame.average);

	/* Проверка таймаутов --------------------------------------------- */
	while (1) {
		turn = to_check(&_InSig->TOQueue);
		if (!turn) break;
		to_delete(&_InSig->TOQueue, turn);

		switch (turn->id) {
			case TurnID_Fueljam: {
				if (Sensors_FueljamAverage < Fueljam_SENSOR_THRESHOLD_PERC) {
					_InSig->Fueljam_Cnt = 0;
					Sensors_FueljamState = 0;
				} else if (_InSig->Fueljam_Cnt >= Fueljam_CONTROL_POINTS) {
					Sensors_FueljamState = 1;
				} else {
					_InSig->Fueljam_Cnt++;
				}
				to_append(&_InSig->TOQueue, &_InSig->FueljamTOItem, Fueljam_MEASURE_PERIOD);
			}
		}
	}

	/* Датчики температуры -------------------------------------------- */
	for (Sensors i = Sensor_tStart; i < Sensor_tEnd; ++i) {
		if (_InSig->eventT[i]) {
			_InSig->eventT[i] = 0;
			// data
			if ( dsw_getdata(i, &data) ) {
				_InSig->errorT[i]++;
			} else {
				Sensors_InstantStatuses[i] = (int)data.state;
				if (Sensors_InstantStatuses[i] == SensorsTStatus_OK) {
					Sensors_Temperatures[i] = data.value;
				}
			}
			// info
			if ( !dsw_getinfo(i, &info) ) {		
				int err = info.cntcrcerr + info.cntpreserr;
				Service_1wire_FrameCnts[i] = info.cntcrcok + err;
				Service_1wire_ErrorRates[i] = (float)err / (float)Service_1wire_FrameCnts[i] * 100.0;
			}
		}
	}

    /* GSM ------------------------------------------------------------ */    
    GSM_Attached = !GPIO_ReadInputDataBit(GSM_KEY_PORT, GSM_KEY_PIN);       
    GSM_ModuleState = GPIO_ReadInputDataBit(GSM_STAT_PORT, GSM_STAT_PIN);   
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: InSig_Diagnostic
 *
 * @brief:
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 * 		0 - положительный результат диагностики
 * 		1 - провал диагностики
*/ 
int InSig_Diagnostic(void)
{
	if (!_InSig || !_InSig->DB) return 1;

	DB_t *DB = _InSig->DB;
	to_item_t *turn;
	to_append(&_InSig->diag.TOQueue, &_InSig->diag.TOItem, DIAGNOSTIC_SENSORS_TIMEOUT);

	while (1) {
		InSig_Work();
		turn = to_check(&_InSig->diag.TOQueue);
		if (!turn) continue;
		to_delete(&_InSig->diag.TOQueue, turn);
		if (Sensors_FueljamState) return 1;
		return (Sensors_InstantSupplyStatus != SensorsTStatus_OK || Sensors_InstantTrayStatus != SensorsTStatus_OK);
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: InSig_Initialization
 *
 * @brief: Инициализация драйверов входных сигналов
 * 
 * input parameters
 * pDB - указатель на базу параметров и сигналов
 * 
 * output parameters
 * 
 * return
*/
void _InSig_dswevent(u8 dev, dsw_event_e eventT)
{	
	if (dev == 255) 
		return;
	
	_InSig->eventT[dev]++;
	if (eventT == dswerr)
		_InSig->errorT[dev]++;
}



static inline void _InSig_ReloadConvertions()
{
	TIM_Cmd(INSIG_TIMx, DISABLE);
	INSIG_DMA_STREAMx->CR &= ~DMA_SxCR_EN;
	DMA_ClearFlag(INSIG_DMA_STREAMx, _INSIG_DMA_FLAGS);
	INSIG_DMA_STREAMx->NDTR = 2;
	INSIG_DMA_STREAMx->CR |= DMA_SxCR_EN;
	TIM_Cmd(INSIG_TIMx, ENABLE);
}



void ADC_IRQHandler(void)
{
	// if ( ADC_GetITStatus(INSIG_ADCx, ADC_IT_AWD) ) {
	// 	ADC_ClearITPendingBit(INSIG_ADCx, ADC_IT_AWD);
	// }

	if ( ADC_GetITStatus(INSIG_ADCx, ADC_IT_OVR) ) {
		ADC_ClearITPendingBit(INSIG_ADCx, ADC_IT_OVR);
		ADC_ClearFlag(INSIG_ADCx, ADC_FLAG_OVR);
		_InSig_ReloadConvertions();
	}
}



void INSIG_DMA_IRQHandler(void)
{
	if ( DMA_GetITStatus(INSIG_DMA_STREAMx, INSIG_DMA_TC_IT) ) {
		DMA_ClearITPendingBit(INSIG_DMA_STREAMx, INSIG_DMA_TC_IT);
		DB_t *DB = _InSig->DB;	

		// К усиления входного сигнала, в зависимости от температуры
		float T1, k1, k2;
		if (Sensors_TrayT < 0.0f) {
			T1 = -10.0f;
			k1 = 0.86879f;
			k2 = 0.004358485f;
		} else if (Sensors_TrayT >= 0.0f || Sensors_TrayT <= 25.0) {
			T1 = 0.0f;
			k1 = 0.912374582f;
			k2 = 0.003505017f;
		} else {
			T1 = 25.0f;
			k1 = 1.0f;
			k2 = 0.00254739f;
		}
		const float k = k1 + k2 * (Sensors_TrayT - T1);		
		_InSig->ADCConvertedValues[0] = (uint16_t)(((uint32_t)_InSig->ADCOriginalValues[0] * ADC_INPUT_MV_MAX) >> 12);
		_InSig->ADCConvertedValues[1] = (uint16_t)(((uint32_t)_InSig->ADCOriginalValues[1] * ADC_INPUT_MV_MAX) >> 12);
		_InSig->ADCConvertedValues[0] = (uint16_t)(k * (float)(_InSig->ADCConvertedValues[0]));
		_InSig->ADCConvertedValues[1] = (uint16_t)(k * (float)(_InSig->ADCConvertedValues[1]));

		// реальные значения не должны превышать @ref REAL_INPUT_VALUE_MV_MAX
		if (_InSig->ADCConvertedValues[0] > REAL_INPUT_VALUE_MV_MAX)
			_InSig->ADCConvertedValues[0] = REAL_INPUT_VALUE_MV_MAX;
		if (_InSig->ADCConvertedValues[1] > REAL_INPUT_VALUE_MV_MAX)
			_InSig->ADCConvertedValues[1] = REAL_INPUT_VALUE_MV_MAX;

#if (_INSIG_ADC_CONVERTION_TYPE == 1)	
		_InSig->Fueljam.instant = 100.0 - ((float)(_InSig->ADCConvertedValues[0]) / REAL_INPUT_VALUE_DIV);
		_InSig->Fueljam.average = _InSig->Fueljam.average * (1.0-Fueljam_KAVARAGE) + 
				_InSig->Fueljam.instant * Fueljam_KAVARAGE;

		_InSig->flame.instant = ((float)(_InSig->ADCConvertedValues[1]) / REAL_INPUT_VALUE_DIV);
		_InSig->flame.average = _InSig->flame.average * (1.0-FLAME_KAVARAGE) + 
				_InSig->flame.instant * FLAME_KAVARAGE;
#endif		

#if (_INSIG_ADC_CONVERTION_TYPE == 2)	
		switch (_InSig->ADC_turn) {
			case ADCTurn_Fueljam: {		
				_InSig->Fueljam.instant = 100.0 - ((float)(_InSig->ADCConvertedValues[0]) / REAL_INPUT_VALUE_DIV);
				_InSig->Fueljam.average = _InSig->Fueljam.average * (1.0-Fueljam_KAVARAGE) + 
						_InSig->Fueljam.instant * Fueljam_KAVARAGE;	
			} break;
			case ADCTurn_Flame: {
				_InSig->flame.instant = (float)(_InSig->ADCConvertedValues[0]) / REAL_INPUT_VALUE_DIV;
				_InSig->flame.average = _InSig->flame.average * (1.0-FLAME_KAVARAGE) + 
						_InSig->flame.instant * FLAME_KAVARAGE;
			} break;
		}
		_InSig->ADC_turn++;
		if (_InSig->ADC_turn == ADCTurn_tEnd)
			_InSig->ADC_turn = ADCTurn_tStart;
		ADC_RegularChannelConfig(INSIG_ADCx, _InSig->ADCChannel_turn[_InSig->ADC_turn], 1, INSIG_ADCx_SAMPLE_TIME);	
#endif	

	}

	if ( DMA_GetITStatus(INSIG_DMA_STREAMx, INSIG_DMA_ERR_IT) ) {
		DMA_ClearITPendingBit(INSIG_DMA_STREAMx, INSIG_DMA_ERR_IT);
		_InSig_ReloadConvertions();
	}	
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _InSig_InitADC
 *
 * @brief: Инициализация АЦП для датчика пламени и затора
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static void _InSig_InitADC(void)
{		
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	// Enable clocks
	RCC_AHB1PeriphClockCmd(INSIG_DMA_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(INSIG_ADCx_CLK, ENABLE);	
	RCC_APB2PeriphClockCmd(INSIG_TIMx_CLK, ENABLE);

	// DMA configuration
	DMA_InitStructure.DMA_Channel = INSIG_DMA_CHANNELx;  		
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
#if (_INSIG_ADC_CONVERTION_TYPE == 1)		
	DMA_InitStructure.DMA_BufferSize = 2;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
#endif
#if (_INSIG_ADC_CONVERTION_TYPE == 2)		
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
#endif
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(INSIG_ADCx->DR);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;	
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(_InSig->ADCOriginalValues);
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;	
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_Init(INSIG_DMA_STREAMx, &DMA_InitStructure);

	DMA_ClearITPendingBit(INSIG_DMA_STREAMx, DMA_IT_TE | DMA_IT_TC);	
	DMA_ITConfig(INSIG_DMA_STREAMx, DMA_IT_TE | DMA_IT_TC, ENABLE);

	/* Init timer */
	TIM_DeInit(INSIG_TIMx);	
	/* Set timer period msec */
	TIM_TimeBaseInitStruct.TIM_Period = INSIG_ADCx_CONV_PERIOD_MS;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 42000-1; // 1 ms
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV2;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(INSIG_TIMx, &TIM_TimeBaseInitStruct);
	
	TIM_SelectOutputTrigger(INSIG_TIMx, TIM_TRGOSource_Update);
	
	TIM_Cmd(INSIG_TIMx, ENABLE);		
	
	/* ADCx regular channel12 configuration **************************************/
#if (_INSIG_ADC_CONVERTION_TYPE == 1)		
	ADC_RegularChannelConfig(INSIG_ADCx, Fueljam_SENSOR_ADCx_CHANNEL, 1, INSIG_ADCx_SAMPLE_TIME);
	ADC_RegularChannelConfig(INSIG_ADCx, FLAME_ADCx_CHANNEL, 2, INSIG_ADCx_SAMPLE_TIME);
#endif
#if (_INSIG_ADC_CONVERTION_TYPE == 2)		
	ADC_RegularChannelConfig(INSIG_ADCx, Fueljam_SENSOR_ADCx_CHANNEL, 1, INSIG_ADCx_SAMPLE_TIME);
#endif
	ADC_EOCOnEachRegularChannelCmd(INSIG_ADCx, ENABLE);

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(INSIG_ADCx, ENABLE);

	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;	
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;	// 10.5 MHz					
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;			
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles; //ADC_TwoSamplingDelay_5Cycles;	
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADCx Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;							
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	
#if (_INSIG_ADC_CONVERTION_TYPE == 1)		
	ADC_InitStructure.ADC_NbrOfConversion = 2;
#endif
#if (_INSIG_ADC_CONVERTION_TYPE == 2)		
	ADC_InitStructure.ADC_NbrOfConversion = 1;
#endif															
	ADC_Init(INSIG_ADCx, &ADC_InitStructure);	

	/* Clear update interrupt bit */
	// ADC_ClearITPendingBit(INSIG_ADCx, ADC_IT_EOC);
	// ADC_ClearITPendingBit(INSIG_ADCx, ADC_IT_AWD);
	ADC_ClearITPendingBit(INSIG_ADCx, ADC_IT_OVR);
	
	/* Enable update interrupt */
	// ADC_ITConfig(INSIG_ADCx, ADC_IT_EOC, ENABLE);
	// ADC_ITConfig(INSIG_ADCx, ADC_IT_AWD, ENABLE);
	ADC_ITConfig(INSIG_ADCx, ADC_IT_OVR, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;	
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = INSIG_DMA_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	// Enable ADC & DMA
	ADC_DMACmd(INSIG_ADCx, ENABLE);
	DMA_Cmd(INSIG_DMA_STREAMx, ENABLE);
	ADC_Cmd(INSIG_ADCx, ENABLE);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _InSig_InitFlameSensor
 *
 * @brief: Инициализация переферии датчика пламени
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/  
static void _InSig_InitFlameSensor(void)
{   	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(FLAME_SENSOR_RCC, ENABLE);  

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = FLAME_SENSOR_IN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	// GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(FLAME_SENSOR_PORT, &GPIO_InitStructure);	
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _InSig_InitFueljamSensor
 *
 * @brief: Инициализация переферии датчика затора
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/  
static void _InSig_InitFueljamSensor(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(Fueljam_SENSOR_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = Fueljam_SENSOR_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(Fueljam_SENSOR_PORT, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = Fueljam_SENSOR_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_Init(Fueljam_SENSOR_PORT, &GPIO_InitStructure);		
	
	Fueljam_SENSOR_PORT->ODR &= ~Fueljam_SENSOR_OUT;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _InSig_InitGSM
 *
 * @brief: Инициализация входных сигналов GSM
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/  
static void _InSig_InitGSM(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(GSM_STAT_CLK, ENABLE);  
	RCC_AHB1PeriphClockCmd(GSM_KEY_CLK, ENABLE);  
    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

    GPIO_InitStructure.GPIO_Pin = GSM_STAT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GSM_STAT_PORT, &GPIO_InitStructure); 

    GPIO_InitStructure.GPIO_Pin = GSM_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GSM_KEY_PORT, &GPIO_InitStructure); 
}
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: InSig_Initialization
 *
 * @brief: Инициализация драйверов входных сигналов
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
int InSig_Initialization(DB_t *DB)
{
	if (!_InSig || !DB) return 1;
		
	_InSig->DB = DB;	
		
	/* Инициализация объектов обсулижвания таймаутов */
	to_initque(&_InSig->TOQueue);
	to_inititem(&_InSig->FueljamTOItem, 0, TurnID_Fueljam);	

	/* Инициализация объектов обсулижвания таймаутов диагностики */
	to_initque(&_InSig->diag.TOQueue);
	to_inititem(&_InSig->diag.TOItem, 0, TurnID_Diagnostic);	
	
	/* Запуск таймаута на измерение */
	to_append(&_InSig->TOQueue, &_InSig->FueljamTOItem, 0);	
	
	_InSig->dsw = dsw_create();
	if (!_InSig->dsw) return 1;
		
	_InSig->dsw->event = _InSig_dswevent;
	
	for (Sensors i = Sensor_tStart; i != Sensor_tEnd; ++i) {
		_InSig->dsw->chan[i].permeas = POLL_PERIOD;
		_InSig->dsw->chan[i].tarepeat = REPEAT_DELAY;
		_InSig->dsw->chan[i].mode.bit.work = 1;	
		// по старту отключены
		Sensors_Statuses[i] = SensorsTStatus_PresErr;	
		Sensors_InstantStatuses[i] = SensorsTStatus_PresErr;	
	}
	
	if ( dsw_start() ) return 1;		

	_InSig_InitFlameSensor();
	_InSig_InitFueljamSensor();
    _InSig_InitGSM();
	_InSig_InitADC();

	_InSig->ADCChannel_turn[ADCTurn_Fueljam] = Fueljam_SENSOR_ADCx_CHANNEL;
	_InSig->ADCChannel_turn[ADCTurn_Flame] = FLAME_ADCx_CHANNEL;

	/* Установка для диагностики. Будет сразу сброшен, если затора нет */
	Sensors_FueljamState = 1;
	
	return 0;
}  




/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: InSig_Create
 *
 * @brief: Создание экземпляра InSig
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
int InSig_Create(void)
{
	_InSig = (InSig_t*)calloc(1, sizeof(InSig_t));
	return (_InSig)? 0 : 1;	
} 
  