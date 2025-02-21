#ifndef __MYCAN_H__
#define __MYCAN_H__
#include <rtthread.h>
#include "rtdevice.h"
#include "led.h"

#define CAN_PORT0 										0
#define CAN_PORT1											1
#define EVENT_CAN_RX_NEW_DATA   			0x01 //���յ��������¼�
#define	CAN_STANDARD_ID								0
#define	CAN_EXTENSION_ID							1
#define	CAN_USE_ID										CAN_EXTENSION_ID

#define	CAN_TX_BUF_SIZE								400
#define	CAN_RX_BUF_SIZE								240		//�������ݻ�������С
#define	CAN_DATA_BUF_NUM							10		//id�Ż���Ĵ�С
#define	DATA_ID_EMPTY									0xFFFFFFFF
#define	CAN_DATA_PACKET_RX_HEAD				0x5A
#define	CAN_DATA_PACKET_TX_HEAD				0xA5
#define	CAN_DATA_PACKET_MIN_SIZE			7

//������
#define	EVENT_CODE_DEVICE			0x00			//������Ϣ������
#define	EVENT_CODE_LOCK				0x01			//��������


#define	CAN_RX1_LED_INDICATE()		LED_OpenSingle(LED_CAN1_RX,2)
#define	CAN_TX1_LED_INDICATE()		LED_OpenSingle(LED_CAN1_TX,2)

typedef struct
{
	uint32_t addr;			//��¼��ǰBUF��ŵ�������Դ��ַ
	uint16_t len;						//��¼��ǰBUF�Ѵ�ŵ�����
	uint16_t index;					//��ǰ�Ѵ�����λ��
	uint16_t timeOut;					//���ڽ������ݳ�ʱ
	uint8_t dat[CAN_RX_BUF_SIZE];		//BUF��С
}CanRx_Type;

typedef struct
{
	uint32_t addr;			//��¼��ǰBUF��ŵ�������Դ��ַ
	uint16_t len;						//��¼��ǰBUF�Ѵ�ŵ�����
	uint16_t index;					//��ǰ�Ѵ�����λ��
	uint16_t timeOut;					//���ڽ������ݳ�ʱ
	uint8_t dat[CAN_RX_BUF_SIZE];		//BUF��С
}CanTx_Type;

typedef struct
{
	CanRx_Type rx[CAN_DATA_BUF_NUM];
	CanTx_Type tx;
}Can_Type;

rt_err_t CanInit(uint8_t Port);
uint8_t Can_SendPacket(uint8_t num,uint32_t addr,uint8_t *dat,uint16_t len, uint8_t headflag);

#endif /*__MYCAN_H__*/
