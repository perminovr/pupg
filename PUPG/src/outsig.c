/******************************************************************************
  * @file    outsig.c
  * @author  Перминов Р.И.
  * @version v0.0.1.0
  * @date    31.12.2018
  *****************************************************************************/
    
	
	
#include <stdlib.h>
#include "outsig.h"
#include "Screw.h"
#include "Fan.h"
#include "TEH.h"
#include "SparkPlug.h"
#include "servto.h"



typedef struct {
	DB_t *DB;			/* указатель на базу параметров и сигналов */
} OutSig_t;



static OutSig_t *_OutSig = 0;



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: OutSig_Work
 *
 * @brief: Основной метод OutSig
 * 
 * input parameters
 * 
 * output parameters
 * 
 * no return
*/ 
void OutSig_Work(void)
{
	Screw_Work();
	Fan_Work();
	TEH_Work();
	SparkPlug_Work();
}
  
  
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: OutSig_Initialization
 *
 * @brief: Инициализация драйверов выходных сигналов
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
int OutSig_Initialization(DB_t *DB)
{
	if (!_OutSig || !DB) return 1;
		
	_OutSig->DB = DB;	

	if (Fan_Create()) return 1;
	if (Screw_Create()) return 1;
	
	/* Инициализация драйверов */	
	Screw_Initialization(_OutSig->DB);
	Fan_Initialization(_OutSig->DB);
	TEH_Initialization(_OutSig->DB);
	SparkPlug_Initialization(_OutSig->DB);
	
	return 0;
}  



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: OutSig_Create
 *
 * @brief: Создание экземпляра OutSig
 * 
 * input parameters
 * 
 * output parameters
 * 
 * return 
 *		* 1 - провал
 *		* 0 - операция прошла успешно
*/  
int OutSig_Create(void)
{
	_OutSig = (OutSig_t*)calloc(1, sizeof(OutSig_t));	
	return (_OutSig)? 0 : 1;
} 

