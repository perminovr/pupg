/*******************************************************************************
  * @file    UART_DMA.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    12.10.2018
  *****************************************************************************/



#include "USARTHandler.h"
#include <stdlib.h>
#include <string.h>



typedef struct {
	USARTHandler_CALL_BACK RX_Success_CallBack;
	USARTHandler_CALL_BACK RX_Fail_CallBack;
} USARTHandler_cb_t;



typedef enum {
    State_RXOFF,
    State_RXON,
} RXState;



typedef struct {
    RXState rxState;
    uint8_t TXBuffer[USARTHandler_BUFFER_SIZE];
    uint8_t RXBuffer[USARTHandler_BUFFER_SIZE];
    uint16_t RequestedLength;
    uint8_t *RequestedBuffer;
    USARTHandler_cb_t CallBacks;
} USARTHandler_t;



static USARTHandler_t *_USARTHandler = 0;
static const uint32_t _USARTHandler_ERROR_FLAGS = USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_ORE | USART_FLAG_PE;
static const uint32_t _USARTHandler_TX_FLAGS = USARTHandler_TX_DMA_FLAG_FEIF | USARTHandler_TX_DMA_FLAG_DMEIF | USARTHandler_TX_DMA_FLAG_TEIF | USARTHandler_TX_DMA_FLAG_HTIF | USARTHandler_TX_DMA_FLAG_TCIF;
static const uint32_t _USARTHandler_RX_FLAGS = USARTHandler_RX_DMA_FLAG_FEIF | USARTHandler_RX_DMA_FLAG_DMEIF | USARTHandler_RX_DMA_FLAG_TEIF | USARTHandler_RX_DMA_FLAG_HTIF | USARTHandler_RX_DMA_FLAG_TCIF;


static USARTHandler_RESULT_t _USARTHandler_SendByDMA(uint16_t size);
USARTHandler_RESULT_t USARTHandler_ErrorControl(void);



USARTHandler_RESULT_t USARTHandler_StartRead(uint8_t *buffer, uint16_t length)
{
    if (length > USARTHandler_BUFFER_SIZE)
        return USARTHandler_FAIL;

    if (_USARTHandler->rxState != State_RXON) {
        _USARTHandler->rxState = State_RXON;    

        USARTHandler_ErrorControl();    

        _USARTHandler->RequestedBuffer = buffer;
        _USARTHandler->RequestedLength = length;

        DMA_ClearFlag(USARTHandler_DMA_STREAMRx, _USARTHandler_RX_FLAGS);
        USARTHandler_DMA_STREAMRx->NDTR = (uint32_t)length;
        USARTHandler_DMA_STREAMRx->CR |= DMA_SxCR_EN;

	    return USARTHandler_SUCCESS;
    } else {
	    return USARTHandler_FAIL;
    }
}



void USARTHandler_ForceReadEnd(void)
{
	USARTHandler_DMA_STREAMRx->CR &= ~DMA_SxCR_EN;    
    _USARTHandler->rxState = State_RXOFF;
}



void USARTHandler_ForceWriteStop(void)
{
    USARTHandler_DMA_STREAMTx->CR &= ~DMA_SxCR_EN;
}



uint8_t* USARTHandler_GetRxBuffer(void)
{
    return _USARTHandler->RXBuffer;
}



USARTHandler_RESULT_t USARTHandler_ErrorControl(void)
{
    if ( USARTHandler_USART->SR & _USARTHandler_ERROR_FLAGS ) {
        USARTHandler_DMA_STREAMRx->CR &= ~DMA_SxCR_EN;
        __IO uint16_t dummy = (uint16_t)(USARTHandler_USART->DR & (uint16_t)0x01FF);
		return USARTHandler_FAIL;
    }
	return USARTHandler_SUCCESS;
}



void USARTHandler_IRQHandler(void)
{
	USARTHandler_DMA_STREAMRx->CR &= ~DMA_SxCR_EN;

    // RECEIVE COMPLETE
    if ( DMA_GetITStatus(USARTHandler_DMA_STREAMRx, USARTHandler_DMA_RX_IT_C) ) {
        DMA_ClearFlag(USARTHandler_DMA_STREAMRx, USARTHandler_DMA_RX_IT_C);
        if (_USARTHandler->RequestedBuffer)
		    memcpy((void*)_USARTHandler->RequestedBuffer, &(_USARTHandler->RXBuffer[0]), _USARTHandler->RequestedLength);
        _USARTHandler->rxState = State_RXOFF;  
		_USARTHandler->CallBacks.RX_Success_CallBack();		
    }
    // ERROR OCCURRE
    else if ( DMA_GetITStatus(USARTHandler_DMA_STREAMRx, USARTHandler_DMA_RX_IT_E) ) {
        DMA_ClearFlag(USARTHandler_DMA_STREAMRx, USARTHandler_DMA_RX_IT_E);
        _USARTHandler->rxState = State_RXOFF;  
		_USARTHandler->CallBacks.RX_Fail_CallBack();
    }
}



static USARTHandler_RESULT_t _USARTHandler_SendByDMA(uint16_t size)
{   
    USARTHandler_DMA_STREAMTx->CR &= ~DMA_SxCR_EN;
    DMA_ClearFlag(USARTHandler_DMA_STREAMTx, _USARTHandler_TX_FLAGS);
    USARTHandler_DMA_STREAMTx->NDTR = size;
    USARTHandler_DMA_STREAMTx->CR |= DMA_SxCR_EN;
	return USARTHandler_SUCCESS;
}



USARTHandler_RESULT_t USARTHandler_SendByte(const uint8_t c)
{
	memcpy((void*)_USARTHandler->TXBuffer, &c, 1);
	return _USARTHandler_SendByDMA(1);
}



USARTHandler_RESULT_t USARTHandler_SendBuffer(const uint8_t *buffer, uint16_t length)
{
	memcpy((void*)_USARTHandler->TXBuffer, buffer, length);
	return _USARTHandler_SendByDMA(length);
}



USARTHandler_RESULT_t USARTHandler_SendString(const char *string)
{	
	uint16_t i = strlen(string);
	memcpy((void*)_USARTHandler->TXBuffer, string, i);
	return _USARTHandler_SendByDMA(i);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_Initialization
 * 
 * @brief: Инициализация USARTHandler
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
int USARTHandler_Initialization(USARTHandler_CALL_BACK rx_success_cb, USARTHandler_CALL_BACK rx_fail_cb)
{
    USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	// RCC ENABLE ---------------------------------------------------
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_APB2PeriphClockCmd(USARTHandler_USART_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(USARTHandler_DMA_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(USARTHandler_USART_PORT_CLK, ENABLE);
	
	// GPIO CONFIG --------------------------------------------------   
	GPIO_InitStructure.GPIO_Pin = USARTHandler_USART_TX | USARTHandler_USART_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(USARTHandler_USART_PORT, &GPIO_InitStructure); 
	//
    GPIO_PinAFConfig(USARTHandler_USART_PORT, USARTHandler_USART_PinSource_TX, GPIO_AF_USART1);
	GPIO_PinAFConfig(USARTHandler_USART_PORT, USARTHandler_USART_PinSource_RX, GPIO_AF_USART1);  
	
	// USART CONFIG --------------------------------------------------
	USART_InitStructure.USART_BaudRate = USARTHandler_USART_BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USARTHandler_USART, &USART_InitStructure);
    
	USART_DMACmd(USARTHandler_USART, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
	USART_Cmd(USARTHandler_USART, ENABLE);
	
	// DMA CONFIG ---------------------------------------------------	
	DMA_DeInit(USARTHandler_DMA_STREAMTx);
	DMA_DeInit(USARTHandler_DMA_STREAMRx);
	
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;			
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;	
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    
    DMA_InitStructure.DMA_Channel = USARTHandler_DMA_ChTX;  
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USARTHandler_USART->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)_USARTHandler->TXBuffer;
    DMA_Init(USARTHandler_DMA_STREAMTx, &DMA_InitStructure);
    
    DMA_InitStructure.DMA_Channel = USARTHandler_DMA_ChRX;  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USARTHandler_USART->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)_USARTHandler->RXBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_Init(USARTHandler_DMA_STREAMRx, &DMA_InitStructure);
	
	// NVIC CONFIG --------------------------------------------------
	NVIC_InitStructure.NVIC_IRQChannel = USARTHandler_DMA_IRQ_RX;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	
	DMA_ITConfig(USARTHandler_DMA_STREAMRx, DMA_IT_TC | DMA_IT_TE, ENABLE);
	
	_USARTHandler->CallBacks.RX_Success_CallBack = rx_success_cb;
	_USARTHandler->CallBacks.RX_Fail_CallBack = rx_fail_cb;
    _USARTHandler->rxState = State_RXOFF;
	
	return 0;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_Create
 *
 * @brief: Создание экземпляра USART_DMA
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
int USARTHandler_Create(void)
{
	_USARTHandler = (USARTHandler_t *)calloc(1, sizeof(USARTHandler_t));
	return (_USARTHandler)? 0 : 1;    
}