/*******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  Пискунов С.Н.
  * @version v0.0.0.1
  * @date    08.02.2016
  *****************************************************************************/

#include "stm32f4xx_it.h"
#include "main.h"
#include "stm32f4x7_eth.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"

#include "lwip/sys.h"

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
	while (1);
}

void MemManage_Handler(void)
{
}

void BusFault_Handler(void)
{
}

void UsageFault_Handler(void)
{
	while (1);
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
	/* Sheduler FreeRTOS */
	xPortSysTickHandler();
}

void EXTI15_10_IRQHandler(void)
{
}

void ETH_IRQHandler(void)
{
	static portBASE_TYPE xHigherPriorityTaskWoken;
	
	xHigherPriorityTaskWoken=pdFALSE;

	/* Frame received */
	if (ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) {
		/* Give the semaphore to wakeup LwIP task */
		xSemaphoreGiveFromISR(s_xSemaphore,&xHigherPriorityTaskWoken);
	}
	
	/* Clear the interrupt flags. */
	/* Clear the Eth DMA Rx IT pending bits */
	ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
	
	/* Switch tasks if necessary. */
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
