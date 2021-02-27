/*******************************************************************************
  * @file    stm32f4x7_eth_bsp.h
  * @author  Пискунов С.Н.
  * @version v0.0.0.1
  * @date    08.02.2016
  *****************************************************************************/

#ifndef __STM32F4x7_ETH_BSP_H
#define __STM32F4x7_ETH_BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Relative to STM324xG-EVAL Board */
#define DP83848_PHY_ADDRESS			0x03

/* PHY AF GPIO */
#define PORT_RMII1_REFCLK			GPIOA
#define GPIO_RMII1_REFCLK			GPIO_Pin_1
#define SOURCE_RMII1_REFCLK			GPIO_PinSource1
#define PORT_RMII1_MDIO				GPIOA
#define GPIO_RMII1_MDIO				GPIO_Pin_2
#define SOURCE_RMII1_MDIO			GPIO_PinSource2
#define PORT_RMII1_CRS_DV			GPIOA
#define GPIO_RMII1_CRS_DV			GPIO_Pin_7
#define SOURCE_RMII1_CRS_DV			GPIO_PinSource7
#define PORT_RMII1_RXER				GPIOB
#define GPIO_RMII1_RXER				GPIO_Pin_10
#define SOURCE_RMII1_RXER			GPIO_PinSource10
#define PORT_RMII1_TX_EN			GPIOB
#define GPIO_RMII1_TX_EN			GPIO_Pin_11
#define SOURCE_RMII1_TX_EN			GPIO_PinSource11
#define PORT_RMII1_TXD0				GPIOB
#define GPIO_RMII1_TXD0				GPIO_Pin_12
#define SOURCE_RMII1_TXD0			GPIO_PinSource12
#define PORT_RMII1_TXD1				GPIOB
#define GPIO_RMII1_TXD1				GPIO_Pin_13
#define SOURCE_RMII1_TXD1			GPIO_PinSource13
#define PORT_PHY1_RST				GPIOC
#define GPIO_PHY1_RST				GPIO_Pin_0
#define SOURCE_PHY1_RST				GPIO_PinSource0
#define PORT_PHY1_MDC				GPIOC
#define GPIO_PHY1_MDC				GPIO_Pin_1
#define SOURCE_PHY1_MDC				GPIO_PinSource1
#define PORT_RMII1_RXD0				GPIOC
#define GPIO_RMII1_RXD0				GPIO_Pin_4
#define SOURCE_RMII1_RXD0			GPIO_PinSource4
#define PORT_RMII1_RXD1				GPIOC
#define GPIO_RMII1_RXD1				GPIO_Pin_5
#define SOURCE_RMII1_RXD1			GPIO_PinSource5

void Eth_Link_EXTIConfig(void);
void Eth_Link_ITHandler(uint16_t PHYAddress);
uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4x7_ETH_BSP_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
