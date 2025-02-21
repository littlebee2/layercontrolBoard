#ifndef	_LOCK_H
#define	_LOCK_H

#include <rtthread.h>
#include "rtdevice.h"
#include <board.h>
#include "main.h"

#define	LOCK_NUM								1			//定义锁数量

#define	LOCK_CONFIG_HEAD				0x5A	//配置标志位

#define	CMD_LOCK_QUERY_STATUS		0x00		//查询状态
#define	CMD_LOCK_OPEN						0x01		//开锁
#define	CMD_LOCK_CLOSE					0x02		//关锁
#define	CMD_LOCK_FORCED_OPEN		0x03		//强制开锁
#define	CMD_LOCK_CONFIG					0x04		//配置锁
#define	CMD_LOCK_READ_CONFIG		0x05		//读锁配置
#define	CMD_LOCK_ERROR_REPORT		0xFF

#define	LOCK_OUT_HIGH(G,P)				SET_GPIO_PIN(G,P)
#define	LOCK_OUT_LOW(G,P)					RESET_GPIO_PIN(G,P)
#define	LOCK_STATUS_UNLOCKED		0x00
#define	LOCK_STATUS_LOCKED			0x01


#define	LOCK_REPORT_QUEUE_SIZE	10
#define	LOCK_REPORT_BUF_SIZE		10
typedef struct
{
	uint8_t head;
	uint8_t tail;
	uint16_t delay;
	struct
	{
		uint8_t len;
		uint8_t buf[LOCK_REPORT_BUF_SIZE];
	}queue[LOCK_REPORT_QUEUE_SIZE];
	uint8_t queueFull;
}Struct_LockReport;

typedef enum
{
	LOCK_ERR_NONE = 0,
	LOCK_ERR_OPEN_FAIL,
	LOCK_ERR_CLOSE_FAIL,
	LOCK_ERR_OUT_RANGE,
	LOCK_ERR_INVALID_TYPE,
	LOCK_ERR_EEPROM
}Enum_LockError;

typedef enum
{
	LOCK_TYPE_GENERAL = 0,		//普通锁
	LOCK_TYPE_DOOR,					 //门锁
	LOCK_TYPE_MAX
}Enum_LockType;


typedef struct
{
	uint8_t head;
	uint8_t config[LOCK_NUM];
}LockConfig_Type;

#pragma anon_unions
typedef struct
{
	uint8_t lock:1;
	uint8_t lockBuf:1;
	uint8_t forcedFlag:1;
	uint16_t statusDelay;
	uint16_t delay;
	union
	{
		uint8_t config;
		struct
		{
			Enum_LockType type:4;
			uint8_t lockLevel:1;
			uint8_t openLevel:1;
		};
	};
}Lock_Type;

typedef enum
{
	LOCK_OP_NONE = 0,
	LOCK_OP_OPEN,
	LOCK_OP_FORCED_OPEN
}Lock_OP;

typedef struct
{
	Lock_OP op;
	uint8_t id;
	uint16_t delay;
}Lock_AllOP;

void Lock_Init(void);
void Lock_Handler(void);
void openDoorLock(void);
uint8_t Lock_CMDProcess(uint8_t *datIn,uint8_t lenIn,uint8_t *datOut,uint8_t *lenOut);
uint8_t Get_LockStatus(void);
#endif