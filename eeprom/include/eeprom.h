/******************************************************************************
  * @file    eeprom.h
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date    28.09.2018
  *****************************************************************************/

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>

#define EE_I2C                          I2C1
#define EE_I2C_CLK                      RCC_APB1Periph_I2C1
#define EE_I2C_SCL_PIN                  GPIO_Pin_8
#define EE_I2C_SCL_GPIO_PORT            GPIOB
#define EE_I2C_SCL_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define EE_I2C_SCL_SOURCE               GPIO_PinSource8
#define EE_I2C_SCL_AF                   GPIO_AF_I2C1
#define EE_I2C_SDA_PIN                  GPIO_Pin_9
#define EE_I2C_SDA_GPIO_PORT            GPIOB
#define EE_I2C_SDA_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define EE_I2C_SDA_SOURCE               GPIO_PinSource9
#define EE_I2C_SDA_AF                   GPIO_AF_I2C1
#define EE_M24C64_32

#define EE_I2C_DMA                      DMA1   
#define EE_I2C_DMA_CHANNEL              DMA_Channel_1
#define EE_I2C_DMA_STREAM_TX            DMA1_Stream6
#define EE_I2C_DMA_STREAM_RX            DMA1_Stream0   
#define EE_I2C_DMA_CLK                  RCC_AHB1Periph_DMA1
#define EE_I2C_DR_Address               ((uint32_t)0x40005410)
#define EE_USE_DMA
   
#define EE_I2C_DMA_TX_IRQn              DMA1_Stream6_IRQn
#define EE_I2C_DMA_RX_IRQn              DMA1_Stream0_IRQn
#define EE_I2C_DMA_TX_IRQHandler        DMA1_Stream6_IRQHandler
#define EE_I2C_DMA_RX_IRQHandler        DMA1_Stream0_IRQHandler   
#define EE_I2C_DMA_PREPRIO              0
#define EE_I2C_DMA_SUBPRIO              0   
   
#define EE_TX_DMA_FLAG_FEIF             DMA_FLAG_FEIF6
#define EE_TX_DMA_FLAG_DMEIF            DMA_FLAG_DMEIF6
#define EE_TX_DMA_FLAG_TEIF             DMA_FLAG_TEIF6
#define EE_TX_DMA_FLAG_HTIF             DMA_FLAG_HTIF6
#define EE_TX_DMA_FLAG_TCIF             DMA_FLAG_TCIF6
#define EE_RX_DMA_FLAG_FEIF             DMA_FLAG_FEIF0
#define EE_RX_DMA_FLAG_DMEIF            DMA_FLAG_DMEIF0
#define EE_RX_DMA_FLAG_TEIF             DMA_FLAG_TEIF0
#define EE_RX_DMA_FLAG_HTIF             DMA_FLAG_HTIF0
#define EE_RX_DMA_FLAG_TCIF             DMA_FLAG_TCIF0

#define EE_I2C_SPEED                    340000     //kHz  // at24c04c sup 1 MHz
#define EE_I2C_SLAVE_ADDRESS7           0xA0

#define EE_TOTAL_SIZE_BYTE              512



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EE_isWriteComplete
 *
 * @brief: Проверка завершения записи
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - запись завершена
 *		* 0 - запись не завершена
*/ 
extern int EE_isWriteComplete(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EE_isReadComplete
 *
 * @brief: Проверка завершения чтения
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - чтение завершено
 *		* 0 - чтение не завершено
*/ 
extern int EE_isReadComplete(void);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EE_Read
 *
 * @brief: Установка на чтение из eeprom 
 * 
 * NOTE: вызвать @ref EE_isReadComplete для просмотра статуса операции
 * 
 * input parameters
 * addr - адрес слова (0-512)
 * bufSize - число читаемых слов
 * 
 * output parameters
 * buf - считанные данные
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/ 
extern int EE_Read(uint16_t addr, uint8_t* buf, uint16_t bufSize);



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EE_WritePage
 *
 * @brief: Установка данных на запись в eeprom 
 *  
 * NOTE: вызвать @ref EE_isWriteComplete для просмотра статуса операции
 * 
 * input parameters
 * addr - адрес слова (0-512)
 * buf - данные на передачу
 * bufSize - размер данных на передачу (до 16 байт)
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/ 
extern int EE_WritePage(uint16_t addr, uint8_t* buf, uint8_t bufSize);



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
extern int EE_Initialization();



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
extern int EE_Create(void);



#endif /* EEPROM_H_ */
