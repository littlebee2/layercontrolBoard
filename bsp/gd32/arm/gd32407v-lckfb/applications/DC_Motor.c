#include "DC_Motor.h"

static Struct_DcMotor DC_Motor;

void dc_init(void)
{
	rcu_periph_clock_enable(RCU_GPIOE);
	gpio_output_options_set(GPIOE,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_13);
	gpio_mode_set(GPIOE,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO_PIN_13);
	gpio_af_set(GPIOE,GPIO_AF_1,GPIO_PIN_13);
	
	gpio_output_options_set(GPIOE,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_14);
	gpio_mode_set(GPIOE,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO_PIN_14);
	gpio_af_set(GPIOE,GPIO_AF_1,GPIO_PIN_14);
	
	/*==========基本定时器参数设置APB2=60M============*/
	rcu_periph_clock_enable(RCU_TIMER0);//打开时钟
	timer_parameter_struct timer_initparam;
	timer_initparam.alignedmode = TIMER_COUNTER_EDGE;//边沿对齐
	timer_initparam.counterdirection = TIMER_COUNTER_UP;//向上计数
	timer_initparam.clockdivision = TIMER_CKDIV_DIV1;//不分频
	timer_initparam.period = 999;//自动重载值
	timer_initparam.prescaler = 59;//预分频器值为59。预分频器用于进一步降低定时器的输入时钟频率
	timer_initparam.repetitioncounter = 0;// 重复计数器设置为0
	timer_init(TIMER0, &timer_initparam);
	
	//pwm输出配置空闲低电平，极性高电平
	timer_oc_parameter_struct timer_oc_parm;
	timer_oc_parm.ocidlestate = TIMER_OC_IDLE_STATE_LOW;//空闲默认低
	timer_oc_parm.ocnidlestate = TIMER_OC_IDLE_STATE_LOW;
	timer_oc_parm.ocnpolarity = TIMER_OCN_POLARITY_HIGH;//极性可以是高电平有效（通常意味着当计数器达到比较值时输出高电平）
	timer_oc_parm.ocpolarity = TIMER_OCN_POLARITY_LOW;//极性可以是高电平有效（通常意味着当计数器达到比较值时输出高电平）
	timer_oc_parm.outputnstate = TIMER_CCX_DISABLE;
	timer_oc_parm.outputstate = TIMER_CCX_ENABLE;
	
	timer_channel_output_config(TIMER0,TIMER_CH_2,&timer_oc_parm);
	timer_channel_output_config(TIMER0,TIMER_CH_3,&timer_oc_parm);
	
	/* 设置占空比 */
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,0);//PE13
	timer_channel_output_mode_config(TIMER0,TIMER_CH_2,TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);
 
	/* 设置占空比 */
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_3,0); //PE14
	timer_channel_output_mode_config(TIMER0,TIMER_CH_3,TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);
	
	timer_primary_output_config(TIMER0, ENABLE);
    /* 使能定时器 */
	timer_enable(TIMER0);
}

void dc_test(void)
{
	static uint16_t i=0;
	static uint16_t time=0;
	if(time > 50)
	{
		time = 0;
		if(i<1000)
		{
			i+=10;
			if(i>980)
				i=0;
			timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,i);
		}
	}
	else
		time++;
}

/*
* @name   Start
* @brief  启动直流电机
* @param  None
* @retval None   
*/
static void Start()
{
    //启动电机
    if(DC_Motor.dir ==  Forward_Status)   //如果电机的方向是正转
    {
        //HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);    //启动定时器3通道3的PWM输出
    }
    else //如果电机方向是反转
    {
       // HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);    //启动定时器3通道4的PWM输出
    }
    //更新电机状态
    DC_Motor.status = Start_Status;
}

