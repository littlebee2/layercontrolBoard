#include "comcan.h"
#include "gd32f4xx.h"
#include <rtthread.h>
#include <board.h>

/* CAN0 init function */
int BSP_CANx_Init(uint8_t Port)
{
	can_parameter_struct can_parameter;
	can_filter_parameter_struct can_filter;
	if(Port == CAN_PORT0)
	{
		rcu_periph_clock_enable(RCU_CAN0);
		rcu_periph_clock_enable(RCU_GPIOB);

		/* configure CAN0 GPIO, CAN0_TX(PB9) and CAN0_RX(PB8) */
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
		gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_8);

		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
		gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_9);      

		/* initialize CAN structures */
		can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
		can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
		/* initialize CAN register */
		can_deinit(CAN0);

		/* initialize CAN */
		can_parameter.time_triggered = DISABLE;
		can_parameter.auto_bus_off_recovery = ENABLE;
		can_parameter.auto_wake_up = DISABLE;
		can_parameter.auto_retrans = DISABLE;
		can_parameter.rec_fifo_overwrite = DISABLE;
		can_parameter.trans_fifo_order = DISABLE;
		can_parameter.working_mode = CAN_NORMAL_MODE;
		can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
		can_parameter.time_segment_1 = CAN_BT_BS1_8TQ;
		can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		/* baudrate 250 bps */
		can_parameter.prescaler = 12;
		can_init(CAN0, &can_parameter);

		/* initialize filter */
		/* CAN0 filter number */
		can_filter.filter_number = 0;

		/* initialize filter */
		can_filter.filter_mode = CAN_FILTERMODE_MASK;
		can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		can_filter.filter_list_high = 0x0000;
		can_filter.filter_list_low = 0x0000;
		can_filter.filter_mask_high = 0x0000;
		can_filter.filter_mask_low = 0x0000;
		can_filter.filter_fifo_number = CAN_FIFO0;
		can_filter.filter_enable = ENABLE;
		can_filter_init(&can_filter);   
//		
		can_interrupt_enable(CAN0, CAN_INTEN_RFNEIE0);
		nvic_irq_enable(CAN0_RX0_IRQn, 1, 0);
		nvic_irq_enable(CAN0_RX1_IRQn, 1, 0);
		
		//InitCanQueue(&can_Queue,RxMessagebuf,SIZE_CNA_RX);
	}
	else if(Port == CAN_PORT1)
	{
		rcu_periph_clock_enable(RCU_CAN1);
		rcu_periph_clock_enable(RCU_GPIOB);

		/* configure CAN0 GPIO, CAN1_TX(PB6) and CAN1_RX(PB5) */
		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
		gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_6);

		gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
		gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
		gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_5);      

		/* initialize CAN structures */
		can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
		can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
		/* initialize CAN register */
		can_deinit(CAN1);

		/* initialize CAN */
		can_parameter.time_triggered = DISABLE;
		can_parameter.auto_bus_off_recovery = ENABLE;
		can_parameter.auto_wake_up = DISABLE;
		can_parameter.auto_retrans = DISABLE;
		can_parameter.rec_fifo_overwrite = DISABLE;
		can_parameter.trans_fifo_order = DISABLE;
		can_parameter.working_mode = CAN_NORMAL_MODE;
		can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
		can_parameter.time_segment_1 = CAN_BT_BS1_8TQ;
		can_parameter.time_segment_2 = CAN_BT_BS2_1TQ;
		/* baudrate 250 bps */
		can_parameter.prescaler = 12;
		can_init(CAN1, &can_parameter);

		/* initialize filter */
		/* CAN0 filter number */
		can_filter.filter_number = 15;

		/* initialize filter */
		can_filter.filter_mode = CAN_FILTERMODE_MASK;
		can_filter.filter_bits = CAN_FILTERBITS_32BIT;
		can_filter.filter_list_high = 0x0000;
		can_filter.filter_list_low = 0x0000;
		can_filter.filter_mask_high = 0x0000;
		can_filter.filter_mask_low = 0x0000;
		can_filter.filter_fifo_number = CAN_FIFO1;
		can_filter.filter_enable = ENABLE;
		can_filter_init(&can_filter);   
		
		can_interrupt_enable(CAN1, CAN_INTEN_RFNEIE1);
		nvic_irq_enable(CAN1_RX0_IRQn, 1, 0);
		nvic_irq_enable(CAN1_RX1_IRQn, 1, 0);
	}
	return 0;
}


//==============================================================//
//函数名称：Can_SetFilter		                                    //
//描述：CAN过滤器设置                                           //
//输入：id		接收数据的ID                                      //
//			bank	使用的过滤组																			//
//输出：无                                                      //
//功能：设置可接收数据的ID																			//
//==============================================================//
void Can_SetFilter(rt_uint32_t id,rt_uint8_t bank)
{
    can_filter_parameter_struct can_filter;
    can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
    rt_uint32_t idMask = 0x1fffffff;
    /* initialize filter */ 
    can_filter.filter_number = bank; //范围0-13 can0共可使用14个过滤器组，每个过滤器组有2个32位寄存器或者4个16位寄存器
    can_filter.filter_mode = CAN_FILTERMODE_MASK;//
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;//
    
    can_filter.filter_list_high = (id<<3)>>16;
    can_filter.filter_list_low = (id<<3)|(1<<2)|(0<<1);
    can_filter.filter_mask_high = (idMask<<3)>>16;
    can_filter.filter_mask_low = (idMask<<3)|(1<<2)|(1<<1);
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    
    can_filter_init(&can_filter);
}

void CAN0_RX1_IRQHandler(void)
{

    /* enter interrupt */
    rt_interrupt_enter();
		//canRecvIqr();
     if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_ERRIF))
    {
        //canError();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_ERRIF);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFO0))//缓存已满，又来新报文
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFO0);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFF0))//缓存有三条
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
    }
    /* leave interrupt */
    rt_interrupt_leave();
    
}


void CAN0_RX0_IRQHandler(void)
{

    /* enter interrupt */
    rt_interrupt_enter();
		//canRecvIqr();
     if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_ERRIF))
    {
        //canError();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_ERRIF);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFO0))//缓存已满，又来新报文
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFO0);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFF0))//缓存有三条
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
    }
    /* leave interrupt */
    rt_interrupt_leave();
    
}

void CAN1_RX1_IRQHandler(void)
{

    /* enter interrupt */
    rt_interrupt_enter();
		//canRecvIqr();
     if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_ERRIF))
    {
        //canError();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_ERRIF);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFO0))//缓存已满，又来新报文
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFO0);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFF0))//缓存有三条
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
    }
    /* leave interrupt */
    rt_interrupt_leave();
    
}


void CAN1_RX0_IRQHandler(void)
{

    /* enter interrupt */
    rt_interrupt_enter();
		//canRecvIqr();
     if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_ERRIF))
    {
        //canError();
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_ERRIF);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFO0))//缓存已满，又来新报文
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFO0);
    }
    else if (RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFF0))//缓存有三条
    {
        can_fifo_release(CAN0, CAN_FIFO0);
        can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
    }
    /* leave interrupt */
    rt_interrupt_leave();
    
}