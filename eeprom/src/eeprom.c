/******************************************************************************
  * @file    eeprom.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/


#include "eeprom.h"
#include "stm32f4xx.h"
#include <stdlib.h>



#define EE_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define EE_LONG_TIMEOUT         ((uint32_t)(30 * EE_FLAG_TIMEOUT))
#define EE_ACK_WAIT_REPEAT_MAX  300



typedef enum {
    EEDir_RX,
    EEDir_TX,
} EE_Direction;
 


static const uint32_t TX_FLAGS = EE_TX_DMA_FLAG_FEIF | EE_TX_DMA_FLAG_DMEIF | EE_TX_DMA_FLAG_TEIF | 
                                       EE_TX_DMA_FLAG_HTIF | EE_TX_DMA_FLAG_TCIF;
static const uint32_t RX_FLAGS = EE_RX_DMA_FLAG_FEIF | EE_RX_DMA_FLAG_DMEIF | EE_RX_DMA_FLAG_TEIF |
                                       EE_RX_DMA_FLAG_HTIF | EE_RX_DMA_FLAG_TCIF;
                                       


typedef struct {
    DMA_InitTypeDef DMA_InitStructure;        
    uint8_t isRxComplete;   
	uint8_t wrCompleteState;
	uint8_t DMA_TX_Complete; 
} EE_t;

                                                                 

static EE_t *_EE;



void _EE_DMAConfig(uint8_t *buf, uint32_t bufSize, EE_Direction direction);



int EE_isReadComplete(void)
{
    return _EE->isRxComplete;
}



int EE_isWriteComplete(void)
{  	
	uint32_t timeout;
	uint16_t tmpSR1;

	while (1) { // чтобы перейти 0 -> 1
		switch (_EE->wrCompleteState) {
			case 0: {
				if (_EE->DMA_TX_Complete) {
					while ( !I2C_GetFlagStatus(EE_I2C, I2C_FLAG_BTF) )
						;
					// STOP    
					I2C_GenerateSTOP(EE_I2C, ENABLE);
					_EE->wrCompleteState = 1;
				}
			} break;
			case 1: {
				timeout = EE_LONG_TIMEOUT;
				while ( I2C_GetFlagStatus(EE_I2C, I2C_FLAG_BUSY) ) {
					if ((timeout--) == 0) return 0;
				}
				
				// START
				I2C_GenerateSTART(EE_I2C, ENABLE);
				timeout = EE_FLAG_TIMEOUT;
				while( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_MODE_SELECT) ) {
					if ((timeout--) == 0) return 0;
				}    

				I2C_Send7bitAddress(EE_I2C, EE_I2C_SLAVE_ADDRESS7, I2C_Direction_Transmitter);
				timeout = EE_LONG_TIMEOUT;
				do
				{     
					tmpSR1 = EE_I2C->SR1;
					if ((timeout--) == 0) return 0;
				} while ( (tmpSR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) == 0 );


				// STOP 
				I2C_GenerateSTOP(EE_I2C, ENABLE);
				if (tmpSR1 & I2C_SR1_ADDR)
				{
					tmpSR1 = EE_I2C->SR2;
					_EE->wrCompleteState = 2;
					return 1;
				} else {
					I2C_ClearFlag(EE_I2C, I2C_FLAG_AF);                  
				}

				return 0;
			} break;
			case 2: {
				return 1;
			} break;
		}
	}
}



int EE_Read(uint16_t addr, uint8_t* buf, uint16_t bufSize)
{  
    if (!buf || !bufSize)
        return 1;

    const uint8_t EEAddress = EE_I2C_SLAVE_ADDRESS7 + ((addr>255)? 0x02 : 0x00); // P0
    uint32_t timeout;

    // reset
    _EE->isRxComplete = 0;

    // не было STOP
    if ( I2C_GetFlagStatus(EE_I2C, I2C_FLAG_BUSY) )
        return 1;

    // START
    I2C_GenerateSTART(EE_I2C, ENABLE);

    // ожидание выдачи START
    timeout = EE_FLAG_TIMEOUT;
    while( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_MODE_SELECT) ) {
        if ((timeout--) == 0) return 1;
    }

    // DEV ADDRESS
    I2C_Send7bitAddress(EE_I2C, EEAddress, I2C_Direction_Transmitter);

    // ожидание передачи адреса eeprom
    timeout = EE_FLAG_TIMEOUT;
    while( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) ) {
        if ((timeout--) == 0) return 1;
    } 

    // WORD ADDRESS
    I2C_SendData(EE_I2C, addr);  

    // ожидание передачи адреса слова
    timeout = EE_FLAG_TIMEOUT;
    while( I2C_GetFlagStatus(EE_I2C, I2C_FLAG_BTF) == RESET )
    {
        if ((timeout--) == 0) return 1;
    }

    // START
    I2C_GenerateSTART(EE_I2C, ENABLE);

    // ожидание выдачи START
    timeout = EE_FLAG_TIMEOUT;
    while( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_MODE_SELECT) ) {
        if ((timeout--) == 0) return 1;
    } 

    // DEV ADDRESS
    I2C_Send7bitAddress(EE_I2C, EEAddress, I2C_Direction_Receiver);  

    // чтение одного байта без DMA
    if (bufSize == 1)
    {    
        /* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
        timeout = EE_FLAG_TIMEOUT;
        while (I2C_GetFlagStatus(EE_I2C, I2C_FLAG_ADDR) == RESET) {
            if ((timeout--) == 0) return 1;
        } 

        // ACK OFF
        I2C_AcknowledgeConfig(EE_I2C, DISABLE);   

        // Clear ADDR 
        (void)EE_I2C->SR2;

        // STOP
        I2C_GenerateSTOP(EE_I2C, ENABLE);

        // ожидание завершения передачи
        timeout = EE_FLAG_TIMEOUT;
        while (I2C_GetFlagStatus(EE_I2C, I2C_FLAG_RXNE) == RESET) {
            if ((timeout--) == 0) return 1;
        }

        *buf = I2C_ReceiveData(EE_I2C);
        bufSize--;        

        /* Wait to make sure that STOP control bit has been cleared */
        timeout = EE_FLAG_TIMEOUT;
        while (EE_I2C->CR1 & I2C_CR1_STOP) {
            if ((timeout--) == 0) return 1;
        }  

        // ACK ON
        I2C_AcknowledgeConfig(EE_I2C, ENABLE);  

        _EE->isRxComplete = 1;  
    }
    // чтение более 1 байта
    else
    {
        // ожидание передачи адреса eeprom
        timeout = EE_FLAG_TIMEOUT;
        while ( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) ) {
            if ((timeout--) == 0) return 1;
        }  

        /* Configure the DMA Rx Channel with the buffer address and the buffer size */
        _EE_DMAConfig(buf, bufSize, EEDir_RX);

        /* Inform the DMA that the next End Of Transfer Signal will be the last one */
        I2C_DMALastTransferCmd(EE_I2C, ENABLE); 

    	DMA_ClearFlag(EE_I2C_DMA_STREAM_RX, RX_FLAGS);

        /* Enable the DMA Rx Stream */
        DMA_Cmd(EE_I2C_DMA_STREAM_RX, ENABLE);    

        /* Enable the EE_I2C peripheral DMA requests */
        I2C_DMACmd(EE_I2C, ENABLE);      
    }
    return 0;
}



int EE_WritePage(uint16_t addr, uint8_t* buf, uint8_t bufSize)
{ 
    if (!buf || !bufSize || bufSize > 16)
        return 1;

    const uint8_t EEAddress = EE_I2C_SLAVE_ADDRESS7 + ((addr>255)? 0x02 : 0x00); // P0
    uint32_t timeout;

	_EE->DMA_TX_Complete = 0; // reset
	_EE->wrCompleteState = 0;

    /*!< While the bus is busy */
    timeout = EE_LONG_TIMEOUT;
    while ( I2C_GetFlagStatus(EE_I2C, I2C_FLAG_BUSY) ) {
        if ((timeout--) == 0) return 1;
    }

    // START
    I2C_GenerateSTART(EE_I2C, ENABLE);

    /*!< Test on EV5 and clear it */
    timeout = EE_FLAG_TIMEOUT;
    while( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_MODE_SELECT) ) {
        if ((timeout--) == 0) return 1;
    }

    // DEV ADDRESS
    I2C_Send7bitAddress(EE_I2C, EEAddress, I2C_Direction_Transmitter);

    /*!< Test on EV6 and clear it */
    timeout = EE_FLAG_TIMEOUT;
    while ( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) ) {
        if ((timeout--) == 0) return 1;
    }

    // WORD ADDRESS
    I2C_SendData(EE_I2C, addr);

    /*!< Test on EV8 and clear it */
    timeout = EE_FLAG_TIMEOUT; 
    while ( !I2C_CheckEvent(EE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING) ) {
        if ((timeout--) == 0) return 1;
    }  

    /* Configure the DMA Tx Channel with the buffer address and the buffer size */
    _EE_DMAConfig(buf, bufSize, EEDir_TX);	
	
    DMA_ClearFlag(EE_I2C_DMA_STREAM_TX, TX_FLAGS);

    /* Enable the DMA Tx Stream */
    DMA_Cmd(EE_I2C_DMA_STREAM_TX, ENABLE);

    /* Enable the EE_I2C peripheral DMA requests */
    I2C_DMACmd(EE_I2C, ENABLE);

    return 0;
}



void _EE_DMAConfig(uint8_t *buf, uint32_t bufSize, EE_Direction direction)
{ 
    if (direction == EEDir_TX) {
		DMA_Cmd(EE_I2C_DMA_STREAM_TX, DISABLE);
        _EE->DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buf;
        _EE->DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;    
        _EE->DMA_InitStructure.DMA_BufferSize = bufSize;  
        DMA_Init(EE_I2C_DMA_STREAM_TX, &_EE->DMA_InitStructure);  
    } else { 
		DMA_Cmd(EE_I2C_DMA_STREAM_RX, DISABLE);
        _EE->DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buf;
        _EE->DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        _EE->DMA_InitStructure.DMA_BufferSize = bufSize;      
        DMA_Init(EE_I2C_DMA_STREAM_RX, &_EE->DMA_InitStructure);    
    }
}



void EE_I2C_DMA_TX_IRQHandler(void)
{
    // COMPLETE
    if ( DMA_GetFlagStatus(EE_I2C_DMA_STREAM_TX, EE_TX_DMA_FLAG_TCIF) ) {  
        DMA_Cmd(EE_I2C_DMA_STREAM_TX, DISABLE);
        DMA_ClearFlag(EE_I2C_DMA_STREAM_TX, EE_TX_DMA_FLAG_TCIF);
		_EE->DMA_TX_Complete = 1;
    }

	// ERROR
	if ( DMA_GetFlagStatus(EE_I2C_DMA_STREAM_TX, EE_TX_DMA_FLAG_TEIF) ) {
		I2C_GenerateSTOP(EE_I2C, ENABLE); // reset BUSY
	}
}



void EE_I2C_DMA_RX_IRQHandler(void)
{
    // COMPLETE
    if ( DMA_GetFlagStatus(EE_I2C_DMA_STREAM_RX, EE_RX_DMA_FLAG_TCIF) ) {     
        // STOP
        I2C_GenerateSTOP(EE_I2C, ENABLE);    
        DMA_Cmd(EE_I2C_DMA_STREAM_RX, DISABLE);
        DMA_ClearFlag(EE_I2C_DMA_STREAM_RX, EE_RX_DMA_FLAG_TCIF);
        _EE->isRxComplete = 1;
    }

	// ERROR
	if ( DMA_GetFlagStatus(EE_I2C_DMA_STREAM_RX, EE_RX_DMA_FLAG_TEIF) ) {
        I2C_GenerateSTOP(EE_I2C, ENABLE);   
	}
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EE_Initialization
 * 
 * @brief: Инициализация EE 
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/
int EE_Initialization(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; 
    I2C_InitTypeDef  I2C_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;         
    
    RCC_APB1PeriphClockCmd(EE_I2C_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(EE_I2C_SCL_GPIO_CLK | EE_I2C_SDA_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(EE_I2C_DMA_CLK, ENABLE); 
    
    GPIO_PinAFConfig(EE_I2C_SCL_GPIO_PORT, EE_I2C_SCL_SOURCE, EE_I2C_SCL_AF);
    GPIO_PinAFConfig(EE_I2C_SDA_GPIO_PORT, EE_I2C_SDA_SOURCE, EE_I2C_SDA_AF);  
    
    GPIO_InitStructure.GPIO_Pin = EE_I2C_SCL_PIN | EE_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(EE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);
    
    /* Configure and enable I2C DMA TX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EE_I2C_DMA_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EE_I2C_DMA_PREPRIO;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EE_I2C_DMA_SUBPRIO;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Configure and enable I2C DMA RX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EE_I2C_DMA_RX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EE_I2C_DMA_PREPRIO;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = EE_I2C_DMA_SUBPRIO;
    NVIC_Init(&NVIC_InitStructure);  
    
    DMA_ClearFlag(EE_I2C_DMA_STREAM_TX, TX_FLAGS);
    DMA_Cmd(EE_I2C_DMA_STREAM_TX, DISABLE);
    DMA_DeInit(EE_I2C_DMA_STREAM_TX);
    
    _EE->DMA_InitStructure.DMA_Channel = EE_I2C_DMA_CHANNEL;
    _EE->DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)EE_I2C_DR_Address;
    _EE->DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    _EE->DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    _EE->DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    _EE->DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    _EE->DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    _EE->DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    _EE->DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    _EE->DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    _EE->DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    _EE->DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    
    // конфигурируется при транзакции
    _EE->DMA_InitStructure.DMA_Memory0BaseAddr = 0;
    _EE->DMA_InitStructure.DMA_DIR = 0;
    _EE->DMA_InitStructure.DMA_BufferSize = 0;
    DMA_Init(EE_I2C_DMA_STREAM_TX, &_EE->DMA_InitStructure);
    
    DMA_ClearFlag(EE_I2C_DMA_STREAM_RX, RX_FLAGS);
    DMA_Cmd(EE_I2C_DMA_STREAM_RX, DISABLE);
    DMA_DeInit(EE_I2C_DMA_STREAM_RX);
    DMA_Init(EE_I2C_DMA_STREAM_RX, &_EE->DMA_InitStructure);
    
    /* Enable the DMA Channels Interrupts */
    DMA_ITConfig(EE_I2C_DMA_STREAM_TX, DMA_IT_TC | DMA_IT_TE, ENABLE);
    DMA_ITConfig(EE_I2C_DMA_STREAM_RX, DMA_IT_TC | DMA_IT_TE, ENABLE);  
    
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = EE_I2C_SPEED;
    
    I2C_Init(EE_I2C, &I2C_InitStructure);
    I2C_Cmd(EE_I2C, ENABLE);

    _EE->isRxComplete = 0;

    return 0;
}




/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EE_Create
 *
 * @brief: Создание экземпляра EE
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
int EE_Create(void)
{
	_EE = (EE_t *)calloc(1, sizeof(EE_t));
	return (_EE)? 0 : 1;
}


















