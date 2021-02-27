/******************************************************************************
  * @file    insig.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/
  
#ifndef INSIG_H_  
#define INSIG_H_

  
#include "database.h"



#define INSIG_ADCx						ADC3
#define INSIG_ADCx_CLK					RCC_APB2Periph_ADC3
#define INSIG_ADCx_CONV_PERIOD_MS		5
#define INSIG_ADCx_SAMPLE_TIME       	ADC_SampleTime_480Cycles
#define INSIG_TIMx						TIM8
#define INSIG_TIMx_CLK					RCC_APB2Periph_TIM8

#define INSIG_DMA						DMA2
#define INSIG_DMA_CLK					RCC_AHB1Periph_DMA2
#define INSIG_DMA_CHANNELx				DMA_Channel_2
#define INSIG_DMA_STREAMx				DMA2_Stream0
#define INSIG_DMA_FLAG_FEIF				DMA_FLAG_FEIF0
#define INSIG_DMA_FLAG_DMEIF			DMA_FLAG_DMEIF0
#define INSIG_DMA_FLAG_TEIF				DMA_FLAG_TEIF0
#define INSIG_DMA_FLAG_HTIF				DMA_FLAG_HTIF0
#define INSIG_DMA_FLAG_TCIF				DMA_FLAG_TCIF0
#define INSIG_DMA_ERR_IT				DMA_IT_TEIF0
#define INSIG_DMA_TC_IT					DMA_IT_TCIF0
#define INSIG_DMA_IRQn					DMA2_Stream0_IRQn
#define INSIG_DMA_IRQHandler			DMA2_Stream0_IRQHandler

#define ADC_INPUT_MV_MAX			    3300UL
#define REAL_INPUT_VALUE_MV_MAX			1800
#define REAL_INPUT_VALUE_DIV			18.1f   // 0.1f взято с запасом, чтобы не превышать 99,5%

#define FLAME_SENSOR_RCC		        RCC_AHB1Periph_GPIOC
#define FLAME_SENSOR_PORT		        GPIOC
#define FLAME_SENSOR_IN			        GPIO_Pin_2	
#define FLAME_ADCx_CHANNEL		        ADC_Channel_12
#define FLAME_KAVARAGE					0.001

#define Fueljam_SENSOR_RCC				RCC_AHB1Periph_GPIOC
#define Fueljam_SENSOR_PORT				GPIOC
#define Fueljam_SENSOR_OUT				GPIO_Pin_6
#define Fueljam_SENSOR_IN				GPIO_Pin_3
#define Fueljam_SENSOR_ADCx_CHANNEL		ADC_Channel_13
#define Fueljam_SENSOR_THRESHOLD_PERC	61.8
#define Fueljam_KAVARAGE         		0.1


#define GSM_STAT_CLK            		RCC_AHB1Periph_GPIOA
#define GSM_KEY_CLK             		RCC_AHB1Periph_GPIOC
#define GSM_STAT_PORT           		GPIOA
#define GSM_KEY_PORT            		GPIOC
#define GSM_STAT_PIN            		GPIO_Pin_8
#define GSM_KEY_PIN             		GPIO_Pin_8



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
extern int InSig_Diagnostic(void);



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
extern void InSig_Work(void); 



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
extern int InSig_Create(void); 
  
  
  
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
extern int InSig_Initialization(DB_t *DB);  


#endif /* INSIG_H_ */  