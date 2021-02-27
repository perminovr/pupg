/******************************************************************************
  * @file    PUPGutils.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    05.10.2018
  *****************************************************************************/
 
#ifndef PUPGUTILS_H_ 
#define PUPGUTILS_H_

#include "main.h"
#include "flash.h"


/* PHY */
#define PORT_PHY_RES	GPIOC
#define GPIO_PHY_RES	GPIO_Pin_0


/* WDT */
#define PORT_WDT_RES	GPIOA
#define GPIO_WDT_RES	GPIO_Pin_6
#define PORT_WDT_STAT	GPIOA
#define GPIO_WDT_STAT	GPIO_Pin_15


/* Switch */
#define PORT_PIN0		GPIOB
#define GPIO_PIN0		GPIO_Pin_4
#define PORT_PIN1		GPIOB
#define GPIO_PIN1		GPIO_Pin_5
#define PORT_PIN2		GPIOB
#define GPIO_PIN2		GPIO_Pin_6
#define PORT_PIN3		GPIOB
#define GPIO_PIN3		GPIO_Pin_7 



extern void wdt_mode(int mode);



extern void mod_fail(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: TIM_CallBack
 *
 * @brief: Callback прерывания TIM, период = 500 мсек
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
typedef void (*TIM_CallBack)(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ModFail_CallBack
 *
 * @brief: Callback ошибки горелки
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/
typedef void (*PUPGFail_CallBack)(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: PUPGutils_setupLogger
 *
 * @brief: Создание логгера
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void PUPGutils_setupLogger(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: PUPGutils_initDevice
 *
 * @brief: Инициализация переферии @ref PUPGutils_initPeriph ; прерываний
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void PUPGutils_initDevice(TIM_CallBack tcb, PUPGFail_CallBack pfcb);  



#endif /* PUPGUTILS_H_ */