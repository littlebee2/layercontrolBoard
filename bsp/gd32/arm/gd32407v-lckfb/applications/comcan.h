#ifndef __COMCAN_H__
#define __COMCAN_H__

#include "mycan.h"
#include <rtthread.h>
#include "rtdevice.h"
#include "gd32f4xx_can.h"

#define CAN_PORT0 0
#define CAN_PORT1	1
int BSP_CANx_Init(uint8_t Port);
void Can_SetFilter(rt_uint32_t id,rt_uint8_t bank);

#endif /*__COMCAN_H__*/
