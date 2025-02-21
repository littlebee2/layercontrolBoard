#ifndef	_EVENT_H
#define	_EVENT_H

#include <rtthread.h>
#include "rtdevice.h"
#include "main.h"

#define	DATA_ID_SELF					sys.slaveID		//定义从机地址
#define	DATA_ID_BROADCAST			0x0000    //定义广播地址

//功能码
#define	EVENT_CODE_DEVICE			0x00			//柜子信息功能码
#define	EVENT_CODE_LOCK				0x01			//锁功能码
#define	EVENT_CODE_WEIGHT			0x05      //称功能码
#define EVENT_CODE_MISCELL		0x0F		//杂项功能码
	#define EVENT_SUBCODE_WS2811				0x00	//杂项子功能，对ws2811控制
	#define EVENT_SUBCODE_DHT11					0x01	//杂项子功能，对dht11温湿度
	#define EVENT_SUBCODE_HUMANSENSOR		0x02	//杂项子功能，对人体感应的检测

#define	EVENT_CODE_DFU				0x70			//在线升级功能码
#define	EVENT_CODE_OTHER			0x90
#define	EVENT_RESPONSE_PACKET_MAX_SIZE		100	//定义响应包最大数据包大小
#define	EVENT_REPORT_PACKET_MAX_SIZE			20  //定义上报数据包最大数据包大小

#define	EVENT_SEND_TYPE_EMPTY			0x00
#define	EVENT_SEND_TYPE_REPORT		0x01
#define	EVENT_SEND_TYPE_RES				0x02

typedef struct
{
	uint8_t flag;
}Struct_Login;

typedef struct
{
	uint8_t flag;
	uint8_t code;
	uint16_t timeOut;
	uint8_t count;
}Struct_Ack;

typedef struct
{
	uint8_t len;
	uint8_t count;
	uint8_t reSendDelay;
	uint8_t dat[EVENT_RESPONSE_PACKET_MAX_SIZE];
}Struct_Response;

typedef struct
{
	uint8_t len;
	uint8_t count;
	uint8_t reSendDelay;
	uint8_t dat[EVENT_REPORT_PACKET_MAX_SIZE];
}Struct_Report;

typedef struct
{
	Struct_Login login;
	Struct_Ack ack;
	Struct_Response res;
	Struct_Report	report;
	uint8_t sendType;
	uint8_t sendPID;
	uint8_t receivePID;
}Struct_Event;

#pragma pack(1)
typedef struct
{
	uint16_t head;
	uint16_t id;
	
	uint16_t crc;
}Struct_Store;		//用于储存掉电保存信息
#pragma pack()

void Event_Init(void);
void Event_Handler(void);
void Event_DataReceive(uint8_t *dat,uint16_t len,uint32_t id);
uint8_t Event_SendReportPacket(uint8_t code,uint8_t *dat,uint8_t len);
#endif
