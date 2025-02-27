/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-13     yuanzihao    first implementation
 */

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "main.h"
#include "led.h"
#include "fm24cl16b.h"
#include "eeprom.h"
#include "mycan.h"
#include "event.h"
#include "lock.h"
#include "DC_Motor.h"
#include "leddisplay.h"
//#include "comcan.h"

/*---------------------------------------------------------*/

#define WDT_DEVICE_NAME 	"wdt"			/* 定义看门狗设备名称,句柄 */
static rt_device_t wdg_dev;

Struct_System sys;
/*-------------------------------------------------------------------*/

/* 在空闲线程回调函数喂狗 */
static void idle_hook(void)
{
	rt_device_control(wdg_dev,RT_DEVICE_CTRL_WDT_KEEPALIVE,NULL);
	//rt_kprintf("free the dog!\n");
}

int main(void)
{
	rt_err_t ret = RT_EOK;
	rt_uint32_t timeout = 10;/*溢出时间*/
	char device_name[RT_NAME_MAX];
//	/* 根据设备名称查找看门狗设备，获取设备句柄 */
//	rt_strncpy(device_name,WDT_DEVICE_NAME,RT_NAME_MAX);
//	wdg_dev = rt_device_find(device_name);
//	if(wdg_dev == RT_NULL)
//	{
//		rt_kprintf("find %s failed!\n",device_name);
//		return -RT_ERROR;
//	}
//	/*初始化看门狗*/
//	ret = rt_device_init(wdg_dev);
//	if(ret != RT_EOK)
//	{
//		rt_kprintf("initialize %s failed!\n",device_name);
//		return -RT_ERROR;
//	}
//	/*设置看门狗阈值*/
//	ret = rt_device_control(wdg_dev,RT_DEVICE_CTRL_WDT_SET_TIMEOUT,&timeout);
//	if(ret != RT_EOK)
//	{
//		rt_kprintf("start %s failed!\n",device_name);
//		return -RT_ERROR;
//	}
//	/* 设置空闲线程回调函数 */
//	rt_thread_idle_sethook(idle_hook);
	
	//初始化灯光
	LED_Init();
	LED_Flash(LED_RUN,50);
//	BSP_CANx_Init(CAN_PORT0);
//	BSP_CANx_Init(CAN_PORT1);
////	Can_SetFilter(0x00FF,0);
////	Can_SetFilter(0x0000,1);
//// 	Can_SetFilter(0xfff0,2);
	u32 res; 
	res = rcu_clock_freq_get(CK_SYS);
	rt_kprintf("sysclock %d\n",res);
	res = rcu_clock_freq_get(CK_AHB);
	rt_kprintf("AHBclock %d\n",res);
	res =	rcu_clock_freq_get(CK_APB1);
	rt_kprintf("APB1clock %d\n",res);
	res = rcu_clock_freq_get(CK_APB2);
	rt_kprintf("APB2clock %d\n",res);
	//Eeprom_Init();
	fm24cl16_init();
	Lock_Init();
	CanInit(CAN_PORT0);
	CanInit(CAN_PORT1);
	LedDisplayInit();
	//dc_init();
	//StepMotor_Init();
	while(1)
	{
		//dc_test();
		//StepMotor_Handler();
		LED_Handler();
		Event_Handler();
		rt_thread_mdelay(10);
	}
	return RT_EOK;
}
