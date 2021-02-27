#ifndef PRJCONFH
#define PRJCONFH

#include "stm32f4xx.h"
#include "lwip/sockets.h"

#define DEFTIMEVAL		0
#define DEFTIMEZONE		1

/* Работа с типами */
#define DEFU8			1
#define DEFS8			1
#define DEFU16			1
#define DEFS16			1
#define DEFU32			1
#define DEFS32			1
#define DEFU64			1
#define DEFS64			1
#define DEFF32			1
#define DEFF64			1

/* Управление памятью FREERTOS */
#define FREERTOSLOC
#define FREERTOSADDR		0x10000000

#define LOGDATALEN		1500		// Размер кадра для передачи журнала

/* Работа со службой времени */
#define FACTORTICK 1
extern unsigned long long __getsystick(void);
#define GETSYSTICK() __getsystick()

/* Работа с DMA буфером */
extern int Check_DMA_TI(void);
#define CHECKDMATX() Check_DMA_TI()

/* Работа с сообщениями */
extern void msg_print(int mask, char *msg);
#define SEND_MESSAGE(mask,msg) msg_print(mask,msg)

#define MAX_TCP_LISTEN		1
#define MAX_TCP_CLIENT		5

/* Описание режима ПЛК */
typedef enum {
	pmEnStat,						// Разрешение статистики
} plcmode_t;

/* размер стеков потоков проекта */
#define IND_MESG_QUEUE_LENGTH	16+2

/* Количество соединений Modbus */
#define MAX_TCP_CONNECTION		5

/* Поиск канала в списке */
extern int findchannel(void *vpic);

#endif
