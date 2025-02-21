#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <rtthread.h>
#include "rtdevice.h"

#define	CMD_DEVICE_OWN_QUERY			0x00		//��ѯ��������Ϣ
#define	CMD_DEVICE_OWN_CONFIG			0x01		//���ÿ�����ID

uint8_t Device_CMDProcess(uint8_t *datIn,uint8_t lenIn,uint8_t *datOut,uint8_t *lenOut);
#endif /*__DEVICE_H__*/
