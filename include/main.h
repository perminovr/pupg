/*******************************************************************************
  * @file    main.h
  * @author  Перминов Р.И.
  * @version v0.1.0.4
  * @date    04.10.2018
  *****************************************************************************/

#ifndef MAINH
#define MAINH

/******************************************************************************/
// #define IDSOFT				0				// Программный идентификатор
// #define IDHARD				0				// Аппаратный идентификатор

// extern const char version[][16];

// #define MODNAME			""	// Имя модуля
// #define SOFTNAME			""		// Имя прошивки
// #define VERSION				version[0]		// Версия прошивки
// #define REALDATA			version[1]		// Дата последнего изменения

// #define MSYSREALNAME		"msys"			// Имя канала
// #define MSYSVERSION			version[0]		// Версия канала
// #define MSYSREALDATA		version[1]		// Дата последнего изменения
/******************************************************************************/

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"

#include "task.h"
#include "tcpip.h"
#include "systime.h"

/* MAC адрес модуля */
#define MAC_ADDR0		(get_mac(0))
#define MAC_ADDR1		(get_mac(1))
#define MAC_ADDR2		(get_mac(2))
#define MAC_ADDR3		(get_mac(3))
#define MAC_ADDR4		(get_mac(4))
#define MAC_ADDR5		(get_mac(5))

/* Максимальое количествово сообщений */
// #define MSYSCNTMSG		10

// /* Структура IP адреса */
// typedef union {
	// struct {
		// u8 o1;
		// u8 o2;
		// u8 o3;
		// u8 o4;
	// } okt;
	// u32 all;
// } uip_t;

/* Структура сетевых параметров */
typedef struct {
	ip_addr_t Mask;			/* Маска сети модуля */
	ip_addr_t IP_Addr;		/* IP адрес модуля */
	ip_addr_t Gateway;		/* Шлюз для удаленной работы */
	char MAC_Addr[6];		/* МАК адрес модуля */
} netdata_t;

/* Структура flash параметров модуля */
typedef struct {
	u16 crc;				/* Контрольная сумма */
	netdata_t data;			/* Сетевые параметры */
} netprm_t;

/* Структура данных сервера */
typedef struct {
	netprm_t netprm;		/* Flash параметры модуля */
	netdata_t curnet;		/* Текущие сетевые параметры */
	struct netif xnetif;	/* Дескриптор для работы с LwIP */
} modprm_t;

/* Список режимов wdt */
typedef enum {
	wmCheck,
	wmWork,
	wmReset,
	wmInit=10000,
	wmParam=240000,
	wmDiagnostic=5010000
} wdtmode_e;

/* для работы с флеш */
// {
	typedef struct {
		//unsigned short usReserv;
		unsigned short usTOReset;
		unsigned short usUnitId;
	} mprm_t;

	// Структура параметров модуля в режиме mbtcp
	typedef struct {
		mprm_t mprm;
		unsigned int ucrc;
	} mbmodprm_t;
// }

extern modprm_t *pgmod;

/* Семафор для CortexLib */
extern xSemaphoreHandle s_xSemaphore;

/* Семафор службы тайм-аутов */
extern xSemaphoreHandle to_xSemaphore;

/* Проверка разрешения передатчика */
extern int Check_DMA_TI(void);

/* Чтение MAC адресов из pgmod байтами */
extern uint8_t get_mac(u8 num);

/* Инициализация Ethernet */
extern void ETH_BSP_Config(void);

/* Загрузка конфигурации */
// extern int setconfig(modprm_t *pmod);

/* Sheduler FreeRTOS */
extern void xPortSysTickHandler(void);

/// Функция установки режима wdt
// \param mode - режим wdt
void wdt_mode(int mode);

#endif
