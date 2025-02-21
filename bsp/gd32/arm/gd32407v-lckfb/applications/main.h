#ifndef __MAIN_H__
#define __MAIN_H__
#include <rtthread.h>
#include "rtdevice.h"

typedef struct
{
	uint16_t slaveID;//ID号
	uint8_t type;//类型
	uint16_t ver;//版本
}Struct_System;

extern Struct_System sys;

#define	BOARD_TYPE						0x17			//层控制板
#define	LAYERBOARD_VERSION		0x0001		//版本号
#endif /*__MAIN_H__*/
