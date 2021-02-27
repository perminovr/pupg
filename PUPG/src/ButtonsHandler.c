/******************************************************************************
  * @file    ButtonsHandler.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    05.10.2018
  *****************************************************************************/



#include "ButtonsHandler.h"
#include "stm32f4xx.h"
#include "utils.h"
#include <stdlib.h>
#include <stdint.h>



#define BUTTONS_HANDLER_TO_EVENT_CNT        8
#define BUTTONS_HANDLER_TO_ROTATION_CNT     13
#define ENCODER_DEFAULT_CNT                 32768



typedef struct {
	ButtonState state;
	ButtonState event;
	uint8_t sum;
	u64 timeStamp;
} Button_t;


typedef struct {
    uint16_t currentCnt;
    int event;
} Encoder_t;



typedef struct {
    Encoder_t encoder;
    Button_t buttons[Button_tEnd];
} ButtonsHandler_t;



static ButtonsHandler_t *_ButtonsHandler;



static ButtonState _ButtonsHandler_ReadPort(Button button);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _ButtonsHandler_SetDefaultCNT
 *
 * @brief: Установка счетчика таймера
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static inline void _ButtonsHandler_SetDefaultCNT(void)
{    
    TIM_Cmd(ENCODER_TIM, DISABLE);
    _ButtonsHandler->encoder.currentCnt = ENCODER_DEFAULT_CNT;
    ENCODER_TIM->CNT = _ButtonsHandler->encoder.currentCnt;
    TIM_Cmd(ENCODER_TIM, ENABLE);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _ButtonsHandler_ChangeState
 *
 * @brief: Изменение состояния кнопки / обновление событий / счетчика
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
static inline void _ButtonsHandler_ChangeState(Button b, ButtonState state)
{
    _ButtonsHandler->buttons[b].state = state;
    _ButtonsHandler->buttons[b].event = ButtonState_SET;
	_ButtonsHandler->buttons[b].timeStamp = getsystick();
    if (b == Button_ENC_UP || b == Button_ENC_DOWN) {
        _ButtonsHandler_SetDefaultCNT();
    }
}



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
void ButtonsHandler_Work(void)
{
    /* ENCODER */
    uint16_t tmp = ENCODER_TIM->CNT;
	_ButtonsHandler->encoder.event = _ButtonsHandler->encoder.currentCnt - tmp;
	for (Button b = Button_tRotation; b != Button_tEnd; ++b) {
        if ( _ButtonsHandler_ReadPort(b) ) {
            if (_ButtonsHandler->buttons[b].sum < BUTTONS_HANDLER_TO_ROTATION_CNT) {
                _ButtonsHandler->buttons[b].sum++;
            }
            if (_ButtonsHandler->buttons[b].sum == BUTTONS_HANDLER_TO_ROTATION_CNT) {
                _ButtonsHandler_ChangeState(b, ButtonState_SET);
            }
        } else {
			if (_ButtonsHandler->buttons[b].sum > 0) {
				if (--_ButtonsHandler->buttons[b].sum == 0) {
                    _ButtonsHandler_ChangeState(b, ButtonState_RESET);
				}
			}
		}
    }
    
    /* BUTTONS */
    for (Button b = Button_tStart; b != Button_tRotation; ++b) {
		if ( _ButtonsHandler_ReadPort(b) ) {
			if (_ButtonsHandler->buttons[b].sum < BUTTONS_HANDLER_TO_EVENT_CNT) {
				if (++_ButtonsHandler->buttons[b].sum == BUTTONS_HANDLER_TO_EVENT_CNT) {
					_ButtonsHandler_ChangeState(b, ButtonState_SET);
				}
			}
		} else {
			if (_ButtonsHandler->buttons[b].sum > 0) {
				if (--_ButtonsHandler->buttons[b].sum == 0) {
                    _ButtonsHandler_ChangeState(b, ButtonState_RESET);
				}
			}
		}
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: _ButtonsHandler_ReadPort
 *
 * @brief: Чтение порта / значения счетчика
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return состояние кнопки
*/ 
static ButtonState _ButtonsHandler_ReadPort(Button button)
{
	switch (button) {
        case Button_OK: return (BUTTON_PORT->IDR & BUTTON_OK)? 
                ButtonState_RESET : ButtonState_SET;
        case Button_ESC: return (BUTTON_PORT->IDR & BUTTON_ESC)?
                ButtonState_RESET : ButtonState_SET;
        case Button_ENCODER: return (BUTTON_PORT->IDR & BUTTON_ENCODER)?
                ButtonState_RESET : ButtonState_SET;
        case Button_ENC_UP: return (_ButtonsHandler->encoder.event > 0)?
                ButtonState_SET : ButtonState_RESET;
        case Button_ENC_DOWN: return (_ButtonsHandler->encoder.event < 0)?
                ButtonState_SET : ButtonState_RESET;
        default: return ButtonState_RESET;        
	}	
}



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
ButtonState ButtonsHandler_State(Button button)
{
    return _ButtonsHandler->buttons[button].state;
}



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
uint32_t ButtonsHandler_TimeFromLastEvent(Button button)
{
	return (uint32_t)((getsystick() - _ButtonsHandler->buttons[button].timeStamp) >> 10); // уменьшается точность, чтобы не делить
}



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
int ButtonsHandler_PressedOver(Button button, ButtonState state, uint32_t timePressed)
{
	return (_ButtonsHandler->buttons[button].state == state && ButtonsHandler_TimeFromLastEvent(button) >= timePressed)? 
			1 : 0;
}



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
ButtonState ButtonsHandler_Event(Button button, ButtonState state)
{
    if (_ButtonsHandler->buttons[button].event) {
		if (_ButtonsHandler->buttons[button].state == state) {
			_ButtonsHandler->buttons[button].event = ButtonState_RESET;
			return ButtonState_SET;
		}
	}	
	return ButtonState_RESET;
}



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
ButtonPosition ButtonsHandler_GroupState()
{
	ButtonPosition ret = (ButtonPosition)0;
	for (Button b = Button_tStart; b != Button_tEnd; ++b) {
		if (_ButtonsHandler->buttons[b].state)
			ret |= (ButtonPosition)(1 << b);
	}
	return ret;    
}



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
ButtonPosition ButtonsHandler_GroupEvent(ButtonState state)
{
    ButtonPosition ret = (ButtonPosition)0;
	for (Button b = Button_tStart; b != Button_tEnd; ++b) {
		if (_ButtonsHandler->buttons[b].event && _ButtonsHandler->buttons[b].state == state) {
			_ButtonsHandler->buttons[b].event = ButtonState_RESET;
			ret |= (ButtonPosition)(1 << b);
		}
	}
	return ret;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ButtonsHandler_Initialization
 * 
 * @brief: Инициализация ButtonsHandler (драйверов weh1602, key)
 *
 * NOTE: Вывод загрузочного экрана после инициализации
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int ButtonsHandler_Initialization(void)
{
    if (!_ButtonsHandler) return 1; 

    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
 
    RCC_AHB1PeriphClockCmd(ENCODER_PORT_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(BUTTON_PORT_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(ENCODER_TIM_CLK, ENABLE);
 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = ENCODER_PIN_A | ENCODER_PIN_B;
    GPIO_Init(ENCODER_PORT, &GPIO_InitStructure);
 
    GPIO_PinAFConfig(ENCODER_PORT, ENCODER_PIN_PinSource_A, ENCODER_PINS_AF);
    GPIO_PinAFConfig(ENCODER_PORT, ENCODER_PIN_PinSource_B, ENCODER_PINS_AF);
    
    TIM_TimeBaseStructInit(&TIM_InitStructure);
    TIM_InitStructure.TIM_Period = 0xffff; // max encoder cnt 
    TIM_InitStructure.TIM_Prescaler = 0;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(ENCODER_TIM, &TIM_InitStructure);
    
    TIM_EncoderInterfaceConfig(ENCODER_TIM, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
    // начальное значение
    _ButtonsHandler_SetDefaultCNT();
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = BUTTON_OK | BUTTON_ESC | BUTTON_ENCODER;
    GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);
	
	return 0;
}



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
int ButtonsHandler_Create(void)
{
	_ButtonsHandler = (ButtonsHandler_t *)calloc(1, sizeof(ButtonsHandler_t));
	return (_ButtonsHandler)? 0 : 1;
}


