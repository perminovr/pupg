/*******************************************************************************
  * @file    common.h
  * @author  Пискунов С.Н.
  * @version v0.0.0.1
  * @date    17.07.2018
  *****************************************************************************/

#ifndef COMMONH
#define COMMONH

#include "utils.h"

/* Глобальная структура */
typedef struct  {
	u8 debug;			// Общий флаг отладки
} baseprm_t;

/* Раскладка приоритетов */
extern int prio[];

/***************************************************/
/*****************  WEH1602  ***********************/
/***************************************************/
#define LCD_EN_Num		0
#define LCD_RS_Num		1
#define LCD_RW_Num		2
#define LCD_DB0_Num		3
#define LCD_DB1_Num		4
#define LCD_DB2_Num		5
#define LCD_DB3_Num		6
#define LCD_DB4_Num		7
#define LCD_DB5_Num		8
#define LCD_DB6_Num		9
#define LCD_DB7_Num		10

#define LCD_EN_Port		GPIOE
#define LCD_RS_Port		GPIOE
#define LCD_RW_Port		GPIOE
#define LCD_DB4_Port	GPIOE
#define LCD_DB5_Port	GPIOE
#define LCD_DB6_Port	GPIOE
#define LCD_DB7_Port	GPIOE

#define LCD_DB4_Pin		GPIO_Pin_0
#define LCD_DB5_Pin		GPIO_Pin_1
#define LCD_DB6_Pin		GPIO_Pin_2
#define LCD_DB7_Pin		GPIO_Pin_3
#define LCD_EN_Pin		GPIO_Pin_4
#define LCD_RS_Pin		GPIO_Pin_5
#define LCD_RW_Pin		GPIO_Pin_6

/* Получить тики, мкс */
extern u32 weh_gettick(void);

/* Функция инициализации линий */
extern void weh_initline(void);

/* Функция установки бита на линии */
extern void weh_setline(u8 npin);

/* Функция снятия бита на линии */
extern void weh_clrline(u8 npin);

/* Функция установки/снятия бита на линии */
extern void weh_dataline(u8 npin, u8 data);

/***************************************************/
/*****************  DSW18S20  **********************/
/***************************************************/

/* Период сброса */
#define DSWTIMERES		600
/* Период обнаружения импульса */
#define DSWTIMEIMP		600
/* Время до чтения импульса */
#define DSWRESTIME_T1	80
/* Период тайм слота */
#define DSWTIMESLOT		80
/* Время удержания линии в 0, если запись 1 */
#define DSWWRTIME_T1	2
/* Время удержания линии в 1, завершение слота записи */
#define DSWWRTIME_T2	15
/* Время удержания линии в 0, начало слота чтения */
#define DSWRDTIME_T1	5
/* Время до чтения линии */
#define DSWRDTIME_T2	10
/* Время удержания линии в 1, завершение слота чтения */
#define DSWRDTIME_T3	15

/* Время конверсии измерительного канала */
#define DSWTIMEMEAS		750000

/* Получить тики, мкс */
extern u32 dsw_gettick(void);

/* Функция для чтения линии приема */
extern u8 dsw_getline(u8 num);

/* Функция для установки линии передачи в 1 */
extern void dsw_setline(u8 num);

/* Функция для установки линии передачи в 0 */
extern void dsw_clrline(u8 num);

/* Функция инициализации линий */
extern void dsw_initline(u8 num);

/* Функция для установки подтягивающей линии в 1 */
extern void dsw_setresist(u8 num);

/* Функция для установки подтягивающей линии в 0 */
extern void dsw_clrresist(u8 num);

#endif
