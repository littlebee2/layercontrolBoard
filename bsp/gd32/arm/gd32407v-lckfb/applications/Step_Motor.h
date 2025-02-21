#ifndef __STEP_MOTOR_H__
#define __STEP_MOTOR_H__
#include <rtthread.h>
#include <board.h>
#include "rtdevice.h"

/*********************宏定义************************/   
#define GUA_DRV8834_STEP_PER_CIRCLE							(GUA_32)(15331)   //电机转动一圈的步数
 
//引脚宏定义
#define DRV8834_SLEEP_PIN					GET_PIN(D,11)

#define DRV8834_CONFIG_PIN				GET_PIN(D,12)
 
#define DRV8834_M1_PIN						GET_PIN(D,13)

#define DRV8834_M0_PIN						GET_PIN(D,14)

#define DRV8834_NENBL_PIN					GET_PIN(D,15)

#define DRV8834_STEP_PIN					GET_PIN(C, 6)
 
#define DRV8834_DIR_PIN						GET_PIN(C, 7)
 

typedef struct
{
	uint8_t open:1;
	uint8_t dir:1;
	uint8_t brake:1;
	uint8_t delay;
	uint8_t speed;
	uint32_t step;
	uint8_t ARR;
	void (*runCompleteCallback)(void);
}Struct_StepMotor;

void StepMotor_Init(void);
void StepMotor_Handler(void);
#endif/*__STEP_MOTOR_H__*/