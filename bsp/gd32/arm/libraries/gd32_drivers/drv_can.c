/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       	  danneil
 * 2023-01-17     RUFU	CHEN      the first version
 */
 #include "drv_can.h"
 
 static rt_err_t _can_config(struct rt_can_device *can, struct can_configure *cfg);
 static rt_err_t _can_control(struct rt_can_device *can, int cmd, void *arg);
 static rt_ssize_t _can_sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno);
 static rt_ssize_t _can_recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno);
 
 #if defined(SOC_SERIES_GD32F4xx) // APB1 30MHz
static const struct gd32_baudrate_tab can_baudrate_tab[] = 
{
    {CAN1MBaud, 		CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ, CAN_BT_BS2_2TQ, 5},
    {CAN800kBaud, 	CAN_BT_SJW_1TQ, CAN_BT_BS1_5TQ, CAN_BT_BS2_2TQ, 8},	
    {CAN500kBaud, 	CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ, CAN_BT_BS2_2TQ, 10},
    {CAN250kBaud, 	CAN_BT_SJW_1TQ, CAN_BT_BS1_8TQ, CAN_BT_BS2_1TQ, 12},
    {CAN125kBaud, 	CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ, CAN_BT_BS2_2TQ, 40},
    {CAN100kBaud, 	CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ, CAN_BT_BS2_2TQ, 50},
    {CAN50kBaud, 		CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ, CAN_BT_BS2_2TQ, 100},
    {CAN20kBaud, 		CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ, CAN_BT_BS2_2TQ, 250},
    {CAN10kBaud,		CAN_BT_SJW_1TQ, CAN_BT_BS1_7TQ, CAN_BT_BS2_2TQ, 500}
};
#endif

#ifdef BSP_USING_CAN0
static struct gd32_can drv_can0 =
{
    .devname = "can0",
    .can_periph = CAN0,
    .can_clk = RCU_CAN0,
	.ioconfig.clk = RCU_GPIOB,
	.ioconfig.port = GPIOB,
	.ioconfig.txpin = GPIO_PIN_9,
	.ioconfig.rxpin = GPIO_PIN_8,
};
#endif
    
    
#ifdef BSP_USING_CAN1
static struct gd32_can drv_can1 =
{
    .devname = "can1",
    .can_periph = CAN1,
    .can_clk = RCU_CAN1,
	.ioconfig.clk = RCU_GPIOB,
	.ioconfig.port = GPIOB,
	.ioconfig.txpin = GPIO_PIN_6,
	.ioconfig.rxpin = GPIO_PIN_5,
};
#endif

static const struct rt_can_ops gd32_can_ops =
{
    _can_config,
    _can_control,
    _can_sendmsg,
    _can_recvmsg,
};

// CAN gpio init
static void gd32_can_gpio_init(struct gd32_can *drvcan)
{
	rcu_periph_clock_enable(RCU_CAN0);		// 需要开启CAN0时钟才能使用CAN0接收中断
    
	// clock enable
	rcu_periph_clock_enable(drvcan->can_clk);
  rcu_periph_clock_enable(drvcan->ioconfig.clk);
    
	// CAN GPIO 
	gpio_output_options_set(drvcan->ioconfig.port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, drvcan->ioconfig.rxpin);
  gpio_mode_set(drvcan->ioconfig.port, GPIO_MODE_AF, GPIO_PUPD_NONE, drvcan->ioconfig.rxpin);
  gpio_af_set(drvcan->ioconfig.port, GPIO_AF_9, drvcan->ioconfig.rxpin);
    
  gpio_output_options_set(drvcan->ioconfig.port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, drvcan->ioconfig.txpin);
  gpio_mode_set(drvcan->ioconfig.port, GPIO_MODE_AF, GPIO_PUPD_NONE, drvcan->ioconfig.txpin);
  gpio_af_set(drvcan->ioconfig.port, GPIO_AF_9, drvcan->ioconfig.txpin);
}

// 获取波特率
static rt_uint32_t get_can_baud_index(rt_uint32_t baud)
{
    rt_uint32_t len, index;
    
    len = sizeof(can_baudrate_tab) / sizeof(can_baudrate_tab[0]);
    for (index = 0; index < len; index++)
    {
        if (can_baudrate_tab[index].baudrate == baud)
            return index;
    }
    
    return 0; /* default baud is CAN1MBaud */
}

// can config
static rt_err_t _can_config(struct rt_can_device *can, struct can_configure *cfg)
{
	struct gd32_can *drv_can;
	rt_uint32_t baud_index;
    
	RT_ASSERT(can);
  RT_ASSERT(cfg);
    
	drv_can = (struct gd32_can *)can->parent.user_data;
	RT_ASSERT(drv_can);
    
	gd32_can_gpio_init(drv_can);
    	
	can_deinit(drv_can->can_periph);
    
	drv_can->can_para.time_triggered = DISABLE;
	drv_can->can_para.auto_bus_off_recovery	= DISABLE;
	drv_can->can_para.auto_wake_up = DISABLE;
	drv_can->can_para.auto_retrans = DISABLE;
	drv_can->can_para.rec_fifo_overwrite = DISABLE;
	drv_can->can_para.trans_fifo_order = DISABLE;
    
	// work mode
	switch(cfg->mode)
	{
		case RT_CAN_MODE_NORMAL:
    		drv_can->can_para.working_mode = CAN_NORMAL_MODE;
    		break;
    
    	case RT_CAN_MODE_LISTEN:
    		drv_can->can_para.working_mode = CAN_SILENT_MODE;
    		break;
    
    	case RT_CAN_MODE_LOOPBACK:
    		drv_can->can_para.working_mode = CAN_LOOPBACK_MODE;
    		break;	
    
    	case RT_CAN_MODE_LOOPBACKANLISTEN:
    		drv_can->can_para.working_mode = CAN_SILENT_LOOPBACK_MODE;
    		break;				
    }
    
    // set baudrate
    baud_index = get_can_baud_index(cfg->baud_rate);
    drv_can->can_para.resync_jump_width = can_baudrate_tab[baud_index].sjw;
    drv_can->can_para.time_segment_1 = can_baudrate_tab[baud_index].bs1;
    drv_can->can_para.time_segment_2 = can_baudrate_tab[baud_index].bs2;
    drv_can->can_para.prescaler = can_baudrate_tab[baud_index].prescaler;
    
    if(can_init(drv_can->can_periph, &drv_can->can_para) != SUCCESS)
    {
    	return -RT_ERROR;
    }
    
    can_filter_init(&drv_can->filterConfig);
    
    return RT_EOK;
}

// can control
static rt_err_t _can_control(struct rt_can_device *can, int cmd, void *arg)
{
	rt_uint32_t argval;
	struct gd32_can *drv_can;
	struct rt_can_filter_config *filter_cfg;
    
	RT_ASSERT(can != RT_NULL);
	drv_can = (struct gd32_can *)can->parent.user_data;
	RT_ASSERT(drv_can != RT_NULL);
    
	switch(cmd)
	{
		// 清除中断
		case RT_DEVICE_CTRL_CLR_INT:
		{
    		argval = (rt_uint32_t) arg;
    
    		if(argval == RT_DEVICE_FLAG_INT_RX)  // 接收中断
    		{
#ifdef BSP_USING_CAN0
    			if (CAN0 == drv_can->can_periph)
    			{
    				nvic_irq_disable(CAN0_RX0_IRQn);
    				nvic_irq_disable(CAN0_RX1_IRQn);
    
    				can_interrupt_disable(drv_can->can_periph,CAN_INTEN_RFNEIE0);
    				can_interrupt_disable(drv_can->can_periph,CAN_INTEN_RFFIE0);
    				can_interrupt_disable(drv_can->can_periph,CAN_INTEN_RFOIE0);
    			}
#endif
    
				#ifdef BSP_USING_CAN1
    			if (CAN1 == drv_can->can_periph)
    			{
    				nvic_irq_disable(CAN1_RX0_IRQn);
    				nvic_irq_disable(CAN1_RX1_IRQn);
    
    				can_interrupt_disable(drv_can->can_periph,CAN_INTEN_RFNEIE1);
    				can_interrupt_disable(drv_can->can_periph,CAN_INTEN_RFFIE1);
    				can_interrupt_disable(drv_can->can_periph,CAN_INTEN_RFOIE1);
    			}
				#endif			
    
    		}
    		else if(argval == RT_DEVICE_FLAG_INT_TX)	// 发送中断
    		{
    				
				#ifdef BSP_USING_CAN0
    			if (CAN0 == drv_can->can_periph)
    			{
    				nvic_irq_disable(CAN0_TX_IRQn);
    			}
				#endif
    
				#ifdef BSP_USING_CAN1
    			if (CAN1 == drv_can->can_periph)
    			{
    				nvic_irq_disable(CAN1_TX_IRQn);
    			}		
				#endif	
    			can_interrupt_disable(drv_can->can_periph,CAN_INTEN_TMEIE);
    		}
    		else if (argval == RT_DEVICE_CAN_INT_ERR)		// ERR 中断
    		{
				#ifdef BSP_USING_CAN0
    			if (CAN0 == drv_can->can_periph)
    			{
    				nvic_irq_disable(CAN0_EWMC_IRQn);
    			}
				#endif
    
				#ifdef BSP_USING_CAN1
    			if (CAN1 == drv_can->can_periph)
    			{
    				nvic_irq_disable(CAN1_EWMC_IRQn);
    			}
				#endif				
    			can_interrupt_disable(drv_can->can_periph,CAN_INTEN_WERRIE);
    			can_interrupt_disable(drv_can->can_periph,CAN_INTEN_PERRIE);
    			can_interrupt_disable(drv_can->can_periph,CAN_INTEN_BOIE);
    			can_interrupt_disable(drv_can->can_periph,CAN_INTEN_ERRNIE);
    			can_interrupt_disable(drv_can->can_periph,CAN_INTEN_ERRIE);
    		}
    	}
		break;
    
    	// 设置中断
    	case RT_DEVICE_CTRL_SET_INT:
    	{
    		argval = (rt_uint32_t) arg;
        	if (argval == RT_DEVICE_FLAG_INT_RX)
        	{
				#ifdef BSP_USING_CAN0
    			if (CAN0 == drv_can->can_periph)
    			{
    				nvic_irq_enable(CAN0_RX0_IRQn, 1, 0);
    				nvic_irq_enable(CAN0_RX1_IRQn, 1, 0);
    
    				can_interrupt_enable(drv_can->can_periph,CAN_INTEN_RFNEIE0);// recv FIFO0 not empty
    				can_interrupt_enable(drv_can->can_periph,CAN_INTEN_RFFIE0);	// recv FIFO0 full
    				can_interrupt_enable(drv_can->can_periph,CAN_INTEN_RFOIE0);	// recv FIFO0 overfull
    			}
				#endif
    			
				#ifdef BSP_USING_CAN1
    			if (CAN1 == drv_can->can_periph)
    			{
    				nvic_irq_enable(CAN1_RX0_IRQn, 1, 0);
    				nvic_irq_enable(CAN1_RX1_IRQn, 0, 0);
    
    				can_interrupt_enable(drv_can->can_periph,CAN_INTEN_RFNEIE1);// recv FIFO0 not empty
    				can_interrupt_enable(drv_can->can_periph,CAN_INTEN_RFFIE1);	// recv FIFO0 full
    				can_interrupt_enable(drv_can->can_periph,CAN_INTEN_RFOIE1);	// recv FIFO0 overfull
    			}
				#endif	
    		}
    		else if(argval == RT_DEVICE_FLAG_INT_TX)	// 发送中断
    		{
    			can_interrupt_enable(drv_can->can_periph,CAN_INTEN_TMEIE);
    			#ifdef BSP_USING_CAN0
    			if (CAN0 == drv_can->can_periph)
    			{
    				nvic_irq_enable(CAN0_TX_IRQn, 1, 0);
    			}
				#endif
				#ifdef BSP_USING_CAN1
    			if (CAN1 == drv_can->can_periph)
    			{
    				nvic_irq_enable(CAN1_TX_IRQn, 1, 0);
    			}		
				#endif				
    		}
    		else if (argval == RT_DEVICE_CAN_INT_ERR)		// ERR 中断
    		{
				can_interrupt_enable(drv_can->can_periph,CAN_INTEN_WERRIE);
    			can_interrupt_enable(drv_can->can_periph,CAN_INTEN_PERRIE);
    			can_interrupt_enable(drv_can->can_periph,CAN_INTEN_BOIE);
    			can_interrupt_enable(drv_can->can_periph,CAN_INTEN_ERRNIE);
    			can_interrupt_enable(drv_can->can_periph,CAN_INTEN_ERRIE);
    			#ifdef BSP_USING_CAN0
    			if (CAN0 == drv_can->can_periph)
    			{
    				nvic_irq_enable(CAN0_EWMC_IRQn, 1, 0);
    			}
				#endif
				#ifdef BSP_USING_CAN1
    			if (CAN1 == drv_can->can_periph)
    			{
    				nvic_irq_enable(CAN1_EWMC_IRQn, 1, 0);
    			}
				#endif				
    		}			
    	}
		break;	
    
		// 设置filter
    	case RT_CAN_CMD_SET_FILTER:
    	{
    		rt_uint32_t id_h = 0;
    	    rt_uint32_t id_l = 0;
    	    rt_uint32_t mask_h = 0;
    	    rt_uint32_t mask_l = 0;
    	    rt_uint32_t mask_l_tail = 0;  //CAN_FxR2 bit [2:0]
    	        
    		if(RT_NULL == arg)
    		{
    			can_filter_init(&drv_can->filterConfig); // 默认filter配置			
    		}
    		else
    		{
    			filter_cfg = (struct rt_can_filter_config *)arg;
    
    			for(int i = 0; i < filter_cfg->count; i++)
    			{
    				if(filter_cfg->items[i].hdr == -1)
    				{
    					/* use default filter bank settings */
    	                if (rt_strcmp(drv_can->devname, "can0") == 0)
    	                {
    	                    /* can1 banks 0~13 */
    	                    drv_can->filterConfig.filter_number = i;
    	                }
    	                else if (rt_strcmp(drv_can->devname, "can1") == 0)
    	                {
    	                    /* can2 banks 14~27 */
    	                    drv_can->filterConfig.filter_number = i + 18;
    	                }
    				}
    				else 
    				{
						// 用户定义的filter num
    					drv_can->filterConfig.filter_number = filter_cfg->items[i].hdr; 
    				}
    					
					if (filter_cfg->items[i].mode == CAN_FILTERMODE_MASK)
                	{
    	                /* make sure the CAN_FxR1[2:0](IDE RTR) work */
    	                mask_l_tail = 0x06;
    	            }
    	            if (filter_cfg->items[i].ide == RT_CAN_STDID)
    	            {
    	                id_h = ((filter_cfg->items[i].id << 18) >> 13) & 0xFFFF;
    	                id_l = ((filter_cfg->items[i].id << 18) |
    	                        (filter_cfg->items[i].ide << 2) |
    	                        (filter_cfg->items[i].rtr << 1)) & 0xFFFF;
    	                mask_h = ((filter_cfg->items[i].mask << 21) >> 16) & 0xFFFF;
    	                mask_l = ((filter_cfg->items[i].mask << 21) | mask_l_tail) & 0xFFFF;
    	            }
    	            else if (filter_cfg->items[i].ide == RT_CAN_EXTID)
    	            {
    	                id_h = (filter_cfg->items[i].id >> 13) & 0xFFFF;
    	                id_l = ((filter_cfg->items[i].id << 3)   |
    	                        (filter_cfg->items[i].ide << 2)  |
    	                        (filter_cfg->items[i].rtr << 1)) & 0xFFFF;
    	                mask_h = ((filter_cfg->items[i].mask << 3) >> 16) & 0xFFFF;
    	                mask_l = ((filter_cfg->items[i].mask << 3) | mask_l_tail) & 0xFFFF;
    	            }
    
    				drv_can->filterConfig.filter_list_high = id_h;
    	            drv_can->filterConfig.filter_list_low = id_l;
    	            drv_can->filterConfig.filter_mask_high = mask_h;
    	            drv_can->filterConfig.filter_mask_low = mask_l;
    
    				drv_can->filterConfig.filter_mode = filter_cfg->items[i].mode;
    				drv_can->filterConfig.filter_fifo_number = filter_cfg->items[i].rxfifo;
    
    				drv_can->filterConfig.filter_enable = ENABLE;		
    				can_filter_init(&drv_can->filterConfig);
    
    			}
    		}
    	}
		break;
    			
    	// 设置CAN work mode
    	case RT_CAN_CMD_SET_MODE:
    	{
    		argval = (rt_uint32_t) arg;
    		if (argval != RT_CAN_MODE_NORMAL && argval != RT_CAN_MODE_LISTEN &&
                argval != RT_CAN_MODE_LOOPBACK && argval != RT_CAN_MODE_LOOPBACKANLISTEN)
			{
				return -RT_ERROR;
			}
    
    		if (argval != drv_can->device.config.mode)
    	    {
    	        drv_can->device.config.mode = argval;
    	        return _can_config(&drv_can->device, &drv_can->device.config);
    	    }
		}
		break;
    
    	// set baudrate
    	case RT_CAN_CMD_SET_BAUD:
    	{
				argval = (rt_uint32_t) arg;
    		if (argval != CAN1MBaud && argval != CAN800kBaud &&
                argval != CAN500kBaud && argval != CAN250kBaud &&
                argval != CAN125kBaud && argval != CAN100kBaud &&
                argval != CAN50kBaud  && argval != CAN20kBaud  &&
                argval != CAN10kBaud)
        	{
            	return -RT_ERROR;
        	}
       	 	if (argval != drv_can->device.config.baud_rate)
    	    {
    	        drv_can->device.config.baud_rate = argval;
    	        return _can_config(&drv_can->device, &drv_can->device.config);
    	    }
    	}
		break;
    
    	// 设置发送优先级
    	case RT_CAN_CMD_SET_PRIV:
    	{
    		argval = (rt_uint32_t) arg;
    		if (argval != RT_CAN_MODE_PRIV && argval != RT_CAN_MODE_NOPRIV)
    		{
    			return -RT_ERROR;
    		}
    		if (argval != drv_can->device.config.privmode)
    		{
    			drv_can->device.config.privmode = argval;
    			return _can_config(&drv_can->device, &drv_can->device.config);
    		}
    	}
		break;
    
    	// 获取CAN ERR状态
    	case RT_CAN_CMD_GET_STATUS:
    	{
    	    rt_uint32_t errtype;
    	    errtype = can_error_get(drv_can->can_periph);
    	    drv_can->device.status.rcverrcnt = errtype >> 24;
    	    drv_can->device.status.snderrcnt = (errtype >> 16 & 0xFF);
    	    drv_can->device.status.lasterrtype = errtype & 0x70;
    	    drv_can->device.status.errcode = errtype & 0x07;
    
    	    rt_memcpy(arg, &drv_can->device.status, sizeof(drv_can->device.status));
    	}
		break;
    }
    
    return RT_EOK;
}

// 发送函数
// boxno参数未使用
//rt_ssize_t (*sendmsg)(struct rt_can_device *can, const void *buf, rt_uint32_t boxno);
static rt_ssize_t _can_sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno)
{
    struct gd32_can *drv_can;
    can_trasnmit_message_struct TxMsg;
    struct rt_can_msg *pmsg = (struct rt_can_msg *) buf;
    
    RT_ASSERT(can != RT_NULL);
    drv_can = (struct gd32_can *)can->parent.user_data;
    RT_ASSERT(drv_can != RT_NULL);
    
    //RT_ASSERT(IS_CAN_DLC(pmsg->len));
    
    if (RT_CAN_STDID == pmsg->ide)		// 标准帧
    {
				TxMsg.tx_ff = CAN_FF_STANDARD;
       	//RT_ASSERT(IS_CAN_STDID(pmsg->id));
       	TxMsg.tx_sfid = pmsg->id;
    }
    else
    {
				TxMsg.tx_ff = CAN_FF_EXTENDED;
       //	RT_ASSERT(IS_CAN_EXTID(pmsg->id));
        TxMsg.tx_efid = pmsg->id;
    }
    
    if (RT_CAN_DTR == pmsg->rtr) // 数据帧或远程帧
    {
       	TxMsg.tx_ft = CAN_FT_DATA;
    }
    else
    {
      	TxMsg.tx_ft = CAN_FT_REMOTE;
    }
    
    TxMsg.tx_dlen = pmsg->len & 0x0FU; // 数据长度
    	
    for(rt_uint8_t i = 0; i < TxMsg.tx_dlen; i++)
    {
    	TxMsg.tx_data[i] = pmsg->data[i];
    }
    
    if(can_message_transmit(drv_can->can_periph, &TxMsg) <= 3) // 返回值为发送邮箱号，共3个发送邮箱
    {
    	return RT_EOK;
    }
    else
    {
    	return -RT_ERROR;
    }
}

// boxno参数未使用
static rt_ssize_t _can_recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno)
{
    struct gd32_can *drv_can;
    can_receive_message_struct RxMsg;
    struct rt_can_msg *pmsg = (struct rt_can_msg *) buf;
    
    RT_ASSERT(can != RT_NULL);
    drv_can = (struct gd32_can *)can->parent.user_data;
    RT_ASSERT(drv_can != RT_NULL);
    
    can_message_receive(drv_can->can_periph, drv_can->filterConfig.filter_fifo_number, &RxMsg);
    
    /* get id */
    if (CAN_FF_STANDARD == RxMsg.rx_ff)
    {
        pmsg->ide = RT_CAN_STDID;
        pmsg->id = RxMsg.rx_sfid;
    }
    else
    {
        pmsg->ide = RT_CAN_EXTID;
        pmsg->id = RxMsg.rx_efid;
    }
    /* get type */
    if (CAN_FT_DATA == RxMsg.rx_ft)
    {
        pmsg->rtr = RT_CAN_DTR;
    }
    else
    {
        pmsg->rtr = RT_CAN_RTR;
    }
    
    /* get len */
    pmsg->len = RxMsg.rx_dlen;
    
    // data
    for(rt_uint8_t i = 0; i < pmsg->len; i++)
    {
     	pmsg->data[i] = RxMsg.rx_data[i];
    }
    	
    return RT_EOK;	
}

static void _can_rx_isr(struct rt_can_device *can, rt_uint32_t fifo)
{
    rt_uint32_t can_periph;
    RT_ASSERT(can);
    can_periph = ((struct gd32_can *) can->parent.user_data)->can_periph;
    
    switch (fifo)
    {
		case CAN_FIFO0:
			/* save to user list */
			if (can_interrupt_flag_get(can_periph, CAN_INT_FLAG_RFL0))
			{
				rt_hw_can_isr(can, RT_CAN_EVENT_RX_IND | fifo << 8);
			}
			/* Check FULL flag for FIFO0 */
			if (can_flag_get(can_periph, CAN_FLAG_RFF0) && can_interrupt_flag_get(can_periph, CAN_INT_FLAG_RFF0))
			{
				/* Clear FIFO0 FULL Flag */
				can_flag_clear(can_periph, CAN_FLAG_RFF0);
			}
    
			/* Check Overrun flag for FIFO0 */
			if (can_flag_get(can_periph, CAN_FLAG_RFO0) && can_interrupt_flag_get(can_periph, CAN_INT_FLAG_RFO0))
			{
				/* Clear FIFO0 Overrun Flag */
				can_flag_clear(can_periph, CAN_FLAG_RFO0);
				rt_hw_can_isr(can, RT_CAN_EVENT_RXOF_IND | fifo << 8);
			}
			break;
		case CAN_FIFO1:
			/* save to user list */
			if (can_interrupt_flag_get(can_periph, CAN_INT_FLAG_RFL1))
			{
				rt_hw_can_isr(can, RT_CAN_EVENT_RX_IND | fifo << 8);
			}	
			/* Check FULL flag for FIFO1 */
			if (can_flag_get(can_periph, CAN_FLAG_RFF1) && can_interrupt_flag_get(can_periph, CAN_INT_FLAG_RFF1))
			{
				/* Clear FIFO1 FULL Flag */
				can_flag_clear(can_periph, CAN_FLAG_RFF1);
			}
    
			/* Check Overrun flag for FIFO1 */
			if (can_flag_get(can_periph, CAN_FLAG_RFO1) && can_interrupt_flag_get(can_periph, CAN_INT_FLAG_RFO1))
			{
				/* Clear FIFO1 Overrun Flag */
				can_flag_clear(can_periph, CAN_FLAG_RFO1);
				rt_hw_can_isr(can, RT_CAN_EVENT_RXOF_IND | fifo << 8);
			}
		break;
	}
}


static void _can_tx_isr(struct rt_can_device *can)
{
    rt_uint32_t can_periph;
    RT_ASSERT(can);
    can_periph = ((struct gd32_can *) can->parent.user_data)->can_periph;
    	
    if (can_flag_get(can_periph, CAN_FLAG_MTF0))
    {
       rt_hw_can_isr(can, RT_CAN_EVENT_TX_DONE | 0 << 8);
    }
    else
    {
       rt_hw_can_isr(can, RT_CAN_EVENT_TX_FAIL | 0 << 8);
    }
       
    
    if (can_flag_get(can_periph, CAN_FLAG_MTF1))
    {
       rt_hw_can_isr(can, RT_CAN_EVENT_TX_DONE | 1 << 8);
    }
    else
    {
     	rt_hw_can_isr(can, RT_CAN_EVENT_TX_FAIL | 1 << 8);
    }
    
    
    if (can_flag_get(can_periph, CAN_FLAG_MTF2))
    {
        rt_hw_can_isr(can, RT_CAN_EVENT_TX_DONE | 2 << 8);
    }
    else
    {
        rt_hw_can_isr(can, RT_CAN_EVENT_TX_FAIL | 2 << 8);
    }
}
    
#ifdef BSP_USING_CAN0
/** * @brief This function handles CAN0 TX interrupts. transmit fifo0/1/2 is empty can trigger this interrupt*/
void CAN0_TX_IRQHandler(void)
{
    rt_interrupt_enter();
    _can_tx_isr(&drv_can0.device);
    rt_interrupt_leave();
}
    
/** * @brief This function handles CAN0 RX0 interrupts.*/
void CAN0_RX0_IRQHandler(void)
{
	rt_interrupt_enter();
  _can_rx_isr(&drv_can0.device, CAN_FIFO0);
  rt_interrupt_leave();
}
    
/** * @brief This function handles CAN0 RX1 interrupts.*/
void CAN0_RX1_IRQHandler(void)
{
    rt_interrupt_enter();
    _can_rx_isr(&drv_can0.device, CAN_FIFO0);
    rt_interrupt_leave();
}    
#endif
    
#ifdef BSP_USING_CAN1
/** * @brief This function handles CAN1 TX interrupts. transmit fifo0/1/2 is empty can trigger this interrupt*/
void CAN1_TX_IRQHandler(void)
{
    rt_interrupt_enter();
    _can_tx_isr(&drv_can1.device);
    rt_interrupt_leave();
}
    
/** * @brief This function handles CAN1 RX0 interrupts.*/
void CAN1_RX0_IRQHandler(void)
{
    rt_interrupt_enter();
    _can_rx_isr(&drv_can1.device, CAN_FIFO1);
    rt_interrupt_leave();
}
    
/** * @brief This function handles CAN1 RX1 interrupts.*/
void CAN1_RX1_IRQHandler(void)
{
    rt_interrupt_enter();
    _can_rx_isr(&drv_can1.device, CAN_FIFO1);
    rt_interrupt_leave();
}
#endif

int rt_hw_can_init(void)
{
    struct can_configure config = CANDEFAULTCONFIG;
    config.privmode = RT_CAN_MODE_NOPRIV;
    config.ticks = 50;
    #ifdef RT_CAN_USING_HDR
    config.maxhdr = 14;
    #ifdef CAN1
    config.maxhdr = 28;
    #endif
    #endif
    /* config default filter */
    can_filter_parameter_struct filterConf = {0};
    filterConf.filter_list_high = 0x0000;
    filterConf.filter_list_low = 0x0000;
    filterConf.filter_mask_high = 0x0000;
    filterConf.filter_mask_low = 0x0000;
    filterConf.filter_fifo_number = CAN_FIFO0;
    filterConf.filter_number = 0;
    filterConf.filter_mode = CAN_FILTERMODE_MASK;
    filterConf.filter_bits = CAN_FILTERBITS_32BIT;
    filterConf.filter_enable = ENABLE;
    
    #ifdef BSP_USING_CAN0
    filterConf.filter_number = 0;
    
    drv_can0.filterConfig = filterConf;
    drv_can0.device.config = config;
    /* register CAN0 device */
    rt_hw_can_register(&drv_can0.device,
                       drv_can0.devname,
                       &gd32_can_ops,
                       &drv_can0);
    #endif /* BSP_USING_CAN0 */
    
    #ifdef BSP_USING_CAN1
    filterConf.filter_number = 18;
    filterConf.filter_fifo_number = CAN_FIFO1;
    
    drv_can1.filterConfig = filterConf;
    drv_can1.device.config = config;
    /* register CAN1 device */
    rt_hw_can_register(&drv_can1.device,
                       drv_can1.devname,
                       &gd32_can_ops,
                       &drv_can1);
    #endif /* BSP_USING_CAN1 */
    
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_can_init);
