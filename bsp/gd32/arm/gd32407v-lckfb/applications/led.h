#ifndef	_LED_H
#define	_LED_H

#include "typedef.h"

#define	LED_NUM					3
#define	LED_RUN					1
#define	LED_CAN1_RX			2
#define	LED_CAN1_TX			3

typedef enum
{
	LED_CLOSE=0,		//LED�ƹر�
	LED_OPEN,				//LED�ƴ�
	LED_FLASH,			//LED����˸
	LED_SINGLE			//LED����һ��
}Enum_LenMode;

typedef	struct
{
	u16 delay;
	u16 space;
	u8 flash_level;
	Enum_LenMode mode;
}Struct_LedType;

void LED_Init(void);
void LED_Handler(void);
void LED_Open(u8 id);
void LED_Close(u8 id);
void LED_Flash(u8 id,u16 space);
void LED_OpenSingle(u8 id,u16 time);
#endif
