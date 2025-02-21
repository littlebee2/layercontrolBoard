#ifndef __DC_MOTOR_H__
#define __DC_MOTOR_H__

#include <rtthread.h>
#include <board.h>
#include "rtdevice.h"

#define DC_MO_PIN		GET_PIN(E,13)
#define DC_M1_PIN 	GET_PIN(E,14)

typedef enum
{
	Stop_Status	=0,	//关闭状态
	Start_Status	 //开始状态
}Enum_Mode;

typedef enum
{
	Forward_Status = 0,	//正向转动
	Reverse_Status	 //反向转动
}Enum_Direction ;

typedef enum
{
	Speed_10 = 0,
	Speed_20,
	Speed_30,
	Speed_40,
	Speed_50,
	Speed_60,
	Speed_70,
	Speed_80,
	Speed_90,
	Speed_100
}Enum_Speed;

typedef	struct
{
	Enum_Mode status;
	Enum_Direction dir;
	Enum_Speed speed;
}Struct_DcMotor;

void dc_test(void);

#endif/*__DC_MOTOR_H__*/

