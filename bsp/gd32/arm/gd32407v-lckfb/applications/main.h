#ifndef __MAIN_H__
#define __MAIN_H__
#include <rtthread.h>
#include "rtdevice.h"

typedef struct
{
	uint16_t slaveID;//ID��
	uint8_t type;//����
	uint16_t ver;//�汾
}Struct_System;

extern Struct_System sys;

#define	BOARD_TYPE						0x17			//����ư�
#define	LAYERBOARD_VERSION		0x0001		//�汾��
#endif /*__MAIN_H__*/
