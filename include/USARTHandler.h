/*******************************************************************************
  * @file    USARTHandler.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    12.10.2018
  *****************************************************************************/

#ifndef USARTHandler_H_
#define USARTHandler_H_

#include "stm32f4xx.h"
#include <stdint.h>

#define USARTHandler_USART                  USART1
#define USARTHandler_USART_CLK              RCC_APB2Periph_USART1
#define USARTHandler_USART_PORT             GPIOA
#define USARTHandler_USART_TX               GPIO_Pin_9
#define USARTHandler_USART_RX               GPIO_Pin_10
#define USARTHandler_USART_PinSource_TX     GPIO_PinSource9
#define USARTHandler_USART_PinSource_RX     GPIO_PinSource10
#define USARTHandler_USART_PORT_CLK         RCC_AHB1Periph_GPIOA
#define USARTHandler_USART_BaudRate         115200

#define USARTHandler_DMA                    DMA2
#define USARTHandler_DMA_CLK                RCC_AHB1Periph_DMA2
#define USARTHandler_DMA_ChTX               DMA_Channel_4
#define USARTHandler_DMA_ChRX               DMA_Channel_4
#define USARTHandler_DMA_STREAMRx           DMA2_Stream2
#define USARTHandler_DMA_STREAMTx           DMA2_Stream7
#define USARTHandler_DMA_RX_IT_C            DMA_IT_TCIF2
#define USARTHandler_DMA_RX_IT_E            DMA_IT_TEIF2
#define USARTHandler_DMA_IRQ_RX             DMA2_Stream2_IRQn
   
#define USARTHandler_TX_DMA_FLAG_FEIF       DMA_FLAG_FEIF7
#define USARTHandler_TX_DMA_FLAG_DMEIF      DMA_FLAG_DMEIF7
#define USARTHandler_TX_DMA_FLAG_TEIF       DMA_FLAG_TEIF7
#define USARTHandler_TX_DMA_FLAG_HTIF       DMA_FLAG_HTIF7
#define USARTHandler_TX_DMA_FLAG_TCIF       DMA_FLAG_TCIF7
#define USARTHandler_RX_DMA_FLAG_FEIF       DMA_FLAG_FEIF2
#define USARTHandler_RX_DMA_FLAG_DMEIF      DMA_FLAG_DMEIF2
#define USARTHandler_RX_DMA_FLAG_TEIF       DMA_FLAG_TEIF2
#define USARTHandler_RX_DMA_FLAG_HTIF       DMA_FLAG_HTIF2
#define USARTHandler_RX_DMA_FLAG_TCIF       DMA_FLAG_TCIF2


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USARTHandler_BUFFER_SIZE
 *
 * @brief: Size of internal two system buffers
 *
*/
#define USARTHandler_BUFFER_SIZE	255



/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: USARTHandler_RESULT_t
 *
 * @brief: Returns from USARTHandler methods after operation
 *
*/
typedef enum {
	USARTHandler_FAIL = -1,
	USARTHandler_SUCCESS = 0
} USARTHandler_RESULT_t;



/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: USARTHandler_CALL_BACK
 *
 * @brief: Call-back type for all events
 *
*/
typedef void (*USARTHandler_CALL_BACK)(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USARTHandler_NO_CallBack
 *
 * @brief: Empty callback
 *
*/
#define USARTHandler_NO_CallBack		((void *)0)



/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USART_IRQHandler
 *
 * @brief: Internal IRQHandler name
 *
*/
#define USARTHandler_IRQHandler(x) 	    DMA2_Stream2_IRQHandler(x)



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_StartRead
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 * @param buffer - array of the bytes to be received from USART
 * @param length - size of requested data in bytes
 *
 * output parameters
 *
 * return value is result of operation (described above)
*/
extern USARTHandler_RESULT_t USARTHandler_StartRead(uint8_t *buffer, uint16_t length);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_ForceReadEnd
 *
 * @brief: Turns off the receiver
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void USARTHandler_ForceReadEnd(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_ForceWriteStop
 *
 * @brief: Turns off the transmitter
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void USARTHandler_ForceWriteStop(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_GetRxBuffer
 *
 * @brief: Provides direct access to rx buffer
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern uint8_t* USARTHandler_GetRxBuffer(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_ErrorControl
 *
 * @brief: Controls peripherals errors. If errors occurred returns USARTHandler_FAIL and turns off the receiver
 *
 * NOTE: Should be called periodically or before catching the data
 *
 * input parameters
 *
 * output parameters
 *
 * return value is result of operation (described above)
*/
extern USARTHandler_RESULT_t USARTHandler_ErrorControl(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_SendByte
 *
 * @brief: Sends byte through USART
 *
 * NOTE: 
 *
 * input parameters
 * @param c - byte to be sent through USART
 *
 * output parameters
 *
 * return value is result of operation (described above)
*/
extern USARTHandler_RESULT_t USARTHandler_SendByte(const uint8_t c);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_SendBuffer
 *
 * @brief: Sends array of the bytes through USART
 *
 * NOTE: 
 *
 * input parameters
 * @param buffer - array of the bytes to be sent through USART
 * @param length - size of sended data in bytes
 *
 * output parameters
 *
 * return value is result of operation (described above)
*/
extern USARTHandler_RESULT_t USARTHandler_SendBuffer(const uint8_t *buffer, uint16_t length);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_SendString
 *
 * @brief: Sends sequence of characters through USART
 *
 * NOTE: param string should be ended with null-terminator
 *
 * input parameters
 * @param string - sequence of characters that should be ended with null-terminator
 *
 * output parameters
 *
 * return value is result of operation (described above)
*/
extern USARTHandler_RESULT_t USARTHandler_SendString(const char *string);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_Initialization
 * 
 * @brief: Инициализация USARTHandler
 *
 * NOTE: Вывод загрузочного экрана после инициализации
 * 
 * input parameters
 * rx_success_cb - callback после успешного приема
 * rx_fail_cb - callback после неудачного приема
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
extern int USARTHandler_Initialization(
	USARTHandler_CALL_BACK rx_success_cb,
	USARTHandler_CALL_BACK rx_fail_cb
);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_Create
 *
 * @brief: Создание экземпляра USARTHandler
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
extern int USARTHandler_Create(void);


#endif /* USARTHandler_H_ */