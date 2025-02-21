#ifndef __MYCAN_H__
#define __MYCAN_H__
#include <rtthread.h>
#include "rtdevice.h"
#include "led.h"

#define CAN_PORT0 										0
#define CAN_PORT1											1
#define EVENT_CAN_RX_NEW_DATA   			0x01 //接收到新数据事件
#define	CAN_STANDARD_ID								0
#define	CAN_EXTENSION_ID							1
#define	CAN_USE_ID										CAN_EXTENSION_ID

#define	CAN_TX_BUF_SIZE								400
#define	CAN_RX_BUF_SIZE								240		//接收数据缓存区大小
#define	CAN_DATA_BUF_NUM							10		//id号缓存的大小
#define	DATA_ID_EMPTY									0xFFFFFFFF
#define	CAN_DATA_PACKET_RX_HEAD				0x5A
#define	CAN_DATA_PACKET_TX_HEAD				0xA5
#define	CAN_DATA_PACKET_MIN_SIZE			7

//功能码
#define	EVENT_CODE_DEVICE			0x00			//柜子信息功能码
#define	EVENT_CODE_LOCK				0x01			//锁功能码


#define	CAN_RX1_LED_INDICATE()		LED_OpenSingle(LED_CAN1_RX,2)
#define	CAN_TX1_LED_INDICATE()		LED_OpenSingle(LED_CAN1_TX,2)

typedef struct
{
	uint32_t addr;			//记录当前BUF存放的数据来源地址
	uint16_t len;						//记录当前BUF已存放的数据
	uint16_t index;					//当前已处理到的位置
	uint16_t timeOut;					//用于接收数据超时
	uint8_t dat[CAN_RX_BUF_SIZE];		//BUF大小
}CanRx_Type;

typedef struct
{
	uint32_t addr;			//记录当前BUF存放的数据来源地址
	uint16_t len;						//记录当前BUF已存放的数据
	uint16_t index;					//当前已处理到的位置
	uint16_t timeOut;					//用于接收数据超时
	uint8_t dat[CAN_RX_BUF_SIZE];		//BUF大小
}CanTx_Type;

typedef struct
{
	CanRx_Type rx[CAN_DATA_BUF_NUM];
	CanTx_Type tx;
}Can_Type;

rt_err_t CanInit(uint8_t Port);
uint8_t Can_SendPacket(uint8_t num,uint32_t addr,uint8_t *dat,uint16_t len, uint8_t headflag);

#endif /*__MYCAN_H__*/
