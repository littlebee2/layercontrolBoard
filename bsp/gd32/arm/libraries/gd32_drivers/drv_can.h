/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       	  danneil
 * 2018-08-05     RUFU	CHEN      the first version
 */
#ifndef __DRV_CAN_H__
#define __DRV_CAN_H__
 #include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "gd32f4xx_rcu.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined SOC_SERIES_GD32F10x
#include "gd32f10x_can.h"
#elif defined SOC_SERIES_GD32F20x
#include "gd32f20x_can.h"
#elif defined SOC_SERIES_GD32F30x
#include "gd32f30x_can.h"
#elif defined SOC_SERIES_GD32F4xx
#include "gd32f4xx_can.h"
//#include "can.h"
#endif
 struct gd32_baudrate_tab
{
    rt_uint32_t baudrate;		// 波特率
    rt_uint8_t sjw;				// 配置参数
    rt_uint8_t bs1;
		rt_uint8_t bs2;
		rt_uint16_t prescaler;
};

struct can_gpio_info
{
	rcu_periph_enum clk;	// RCU_GPIOx  
	uint32_t port;			// GPIOx	 
	uint32_t txpin;			// GPIO_PIN_x
	uint32_t rxpin;			// GPIO_PIN_x
};

// CAN设备结构体
struct gd32_can
{
    char *devname;								// 设备名称
    rt_uint32_t can_periph;						// CAN0/1
    rcu_periph_enum can_clk;					// CAN外设时钟：RCU_CAN0 / RCU_CAN1
    struct can_gpio_info ioconfig;				// CAN Tx/Rx io 配置
    can_parameter_struct can_para;				// CAN配置参数
    can_filter_parameter_struct filterConfig; 	// CAN Filter配置参数
    	
    struct rt_can_device device;				// can设备基类
};
#endif /*__DRV_CAN_H__*/
