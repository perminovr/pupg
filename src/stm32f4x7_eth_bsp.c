/*******************************************************************************
  * @file    stm32f4x7_eth_bsp.c
  * @author  Пискунов С.Н.
  * @version v0.0.0.2
  * @date    21.09.2016
  *****************************************************************************/

#include "main.h"
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"

/// Настройка PHY RMII
static void ETH_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
		RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	  
	/* Включаем SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	
	/* Устанавливаем режим RMII */
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);
	SYSCFG_CompensationCellCmd(ENABLE);
	
	/* Конфигурируем PHY1_RST PORTC_Pin0 */
	GPIO_InitStructure.GPIO_Pin=GPIO_PHY1_RST;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_PHY1_RST,&GPIO_InitStructure);
	
	GPIO_ResetBits(PORT_PHY1_RST,GPIO_PHY1_RST);

	/* Конфигурируем RMII1_REFCLK PORTA_Pin1 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_REFCLK;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_REFCLK,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_REFCLK,SOURCE_RMII1_REFCLK,GPIO_AF_ETH);

	/* Конфигурируем RMII1_MDIO PORTA_Pin2 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_MDIO;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_MDIO,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_MDIO,SOURCE_RMII1_MDIO,GPIO_AF_ETH);

	/* Конфигурируем RMII1_CRS_DV PORTA_Pin7 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_CRS_DV;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_CRS_DV,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_CRS_DV,SOURCE_RMII1_CRS_DV,GPIO_AF_ETH);

	/* Конфигурируем RMII1_RXER PORTB_Pin10 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_RXER;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_RXER,&GPIO_InitStructure);

	/* Конфигурируем RMII1_TX_EN PORTB_Pin11 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_TX_EN;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_TX_EN,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_TX_EN,SOURCE_RMII1_TX_EN,GPIO_AF_ETH);
	
	/* Конфигурируем RMII1_TXD0 PORTB_Pin12 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_TXD0;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_TXD0,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_TXD0,SOURCE_RMII1_TXD0,GPIO_AF_ETH);
	
	/* Конфигурируем RMII1_TXD1 PORTB_Pin13 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_TXD1;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_TXD1,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_TXD1,SOURCE_RMII1_TXD1,GPIO_AF_ETH);
	
	/* Конфигурируем PHY1_MDC PORTC_Pin1 */
	GPIO_InitStructure.GPIO_Pin=GPIO_PHY1_MDC;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_PHY1_MDC,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_PHY1_MDC,SOURCE_PHY1_MDC,GPIO_AF_ETH);
	
	/* Конфигурируем RMII1_RXD0 PORTC_Pin4 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_RXD0;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_RXD0,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_RXD0,SOURCE_RMII1_RXD0,GPIO_AF_ETH);
	
	/* Конфигурируем RMII1_RXD1 PORTC_Pin5 */
	GPIO_InitStructure.GPIO_Pin=GPIO_RMII1_RXD1;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(PORT_RMII1_RXD1,&GPIO_InitStructure);
	GPIO_PinAFConfig(PORT_RMII1_RXD1,SOURCE_RMII1_RXD1,GPIO_AF_ETH);
	
	GPIO_SetBits(PORT_PHY1_RST,GPIO_PHY1_RST);
}

/// Настройка прерывания Ethernet
static void ETH_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel=ETH_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=11;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/// Настройка параметров Ethernet
static void ETH_MACDMA_Config(void)
{
	ETH_InitTypeDef ETH_InitStructure;

	/* Enable Ethernet clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC |
		RCC_AHB1Periph_ETH_MAC_Tx | RCC_AHB1Periph_ETH_MAC_Rx,ENABLE);

	/* Reset Ethernet on AHB Bus */
	ETH_DeInit();

	/* Software reset */
	ETH_SoftwareReset();

	/* Wait for software reset */
	while (ETH_GetSoftwareResetStatus() == SET);

	/* Ethernet Configuration */
	ETH_StructInit(&ETH_InitStructure);

	/* Fill ETH_InitStructure parametrs */
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;

	/* Configure Ethernet */
	while (ETH_Init(&ETH_InitStructure,DP83848_PHY_ADDRESS) == ETH_ERROR);

	/* Enable the Ethernet Rx Interrupt */
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R,ENABLE);
}

/// Инициализация Ethernet
void ETH_BSP_Config(void)
{
	/* Configure the GPIO ports for ethernet pins */
	ETH_GPIO_Config();

	/* Config NVIC for Ethernet */
	ETH_NVIC_Config();

	/* Configure the Ethernet MAC/DMA */
	ETH_MACDMA_Config();
}

/// Проверка возможности передачи
//\ return = 0 - элемент не может быть передан, нет буферов на передачу
//\ return = 1 - удача, можно передавать.
int Check_DMA_TI(void)
{
	extern __IO ETH_DMADESCTypeDef *DMATxDescToSet;
	
	if ((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET) return 0;
	return 1;
}

/// Получение МАК адреса
uint8_t get_mac(u8 num)
{
	modprm_t *pmod=pgmod;
	return pmod->curnet.MAC_Addr[num];
}
