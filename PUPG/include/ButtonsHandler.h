/******************************************************************************
  * @file    ButtonsHandler.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    05.10.2018
  *****************************************************************************/

#ifndef BUTTONSHANDLER_H_
#define BUTTONSHANDLER_H_


#include <stdint.h>


#define ENCODER_PORT_CLK            RCC_AHB1Periph_GPIOE
#define ENCODER_PORT                GPIOE
#define ENCODER_PIN_A               GPIO_Pin_9
#define ENCODER_PIN_B               GPIO_Pin_11
#define ENCODER_PIN_PinSource_A     GPIO_PinSource9
#define ENCODER_PIN_PinSource_B     GPIO_PinSource11
#define ENCODER_PINS_AF             GPIO_AF_TIM1
#define ENCODER_TIM_CLK             RCC_APB2Periph_TIM1
#define ENCODER_TIM                 TIM1

#define BUTTON_PORT_CLK             RCC_AHB1Periph_GPIOE
#define BUTTON_PORT                 GPIOE
#define BUTTON_OK                   GPIO_Pin_15
#define BUTTON_ESC                  GPIO_Pin_14
#define BUTTON_ENCODER              GPIO_Pin_10



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Button
 *
 * @brief: Список кнопок
 *
*/
typedef enum {
    Button_tStart,
    Button_OK = Button_tStart,
    Button_ESC,
    Button_ENCODER,
    Button_tRotation,
    Button_ENC_UP = Button_tRotation,
    Button_ENC_DOWN,
    Button_tEnd,
} Button;



typedef enum {
    ButtonPos_Clear = 0,
    ButtonPos_OK = 0x01,
    ButtonPos_ESC = 0x02,
    ButtonPos_ENCODER = 0x04,
    ButtonPos_B = 0x08,
    ButtonPos_A = 0x10,
} ButtonPosition;



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ButtonState
 *
 * @brief: Состояние кнопки
 *
*/
typedef enum {
    ButtonState_RESET,
    ButtonState_SET
} ButtonState;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_State
 *
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return состояние кнопки
*/ 
extern ButtonState ButtonsHandler_State(Button button);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_Event
 *
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return событие кнопки
*/ 
extern ButtonState ButtonsHandler_Event(Button button, ButtonState state);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_TimeFromLastEvent
 *
 * @brief: Время (мс) с последнего события кнопки
 * 
 * NOTE: допускается отклонение в меньшую сторону на 2.5%
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return событие кнопки
*/ 
extern uint32_t ButtonsHandler_TimeFromLastEvent(Button button);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_PressedOver
 *
 * @brief: Проверка кнопки на удержание в указанном состоянии в течение указанного времени (мс)
 * 
 * NOTE: допускается отклонение в меньшую сторону на 2.5%
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return
 *      * 1 - кнопка удерживается
 *      * 0 - состояние менялось
*/ 
extern int ButtonsHandler_PressedOver(Button button, ButtonState state, uint32_t timePressed);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_GroupState
 *
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return срез кнопок
*/ 
extern ButtonPosition ButtonsHandler_GroupState();



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_GroupEvent
 *
 * @brief: 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return группу, с отмеченным событием
*/ 
extern ButtonPosition ButtonsHandler_GroupEvent(ButtonState state);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_Work
 *
 * @brief: Основной метод ButtonsHandler
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
extern void ButtonsHandler_Work(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_Initialization
 * 
 * @brief: Инициализация ButtonsHandler (драйверов weh1602, key)
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
extern int ButtonsHandler_Initialization(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_Create
 *
 * @brief: Создание экземпляра ButtonsHandler
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
extern int ButtonsHandler_Create(void);


#endif /* ButtonsHandler_H_ */