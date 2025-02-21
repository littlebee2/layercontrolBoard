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
	
	/*==========������ʱ����������APB2=60M============*/
	rcu_periph_clock_enable(RCU_TIMER0);//��ʱ��
	timer_parameter_struct timer_initparam;
	timer_initparam.alignedmode = TIMER_COUNTER_EDGE;//���ض���
	timer_initparam.counterdirection = TIMER_COUNTER_UP;//���ϼ���
	timer_initparam.clockdivision = TIMER_CKDIV_DIV1;//����Ƶ
	timer_initparam.period = 999;//�Զ�����ֵ
	timer_initparam.prescaler = 59;//Ԥ��Ƶ��ֵΪ59��Ԥ��Ƶ�����ڽ�һ�����Ͷ�ʱ��������ʱ��Ƶ��
	timer_initparam.repetitioncounter = 0;// �ظ�����������Ϊ0
	timer_init(TIMER0, &timer_initparam);
	
	//pwm������ÿ��е͵�ƽ�����Ըߵ�ƽ
	timer_oc_parameter_struct timer_oc_parm;
	timer_oc_parm.ocidlestate = TIMER_OC_IDLE_STATE_LOW;//����Ĭ�ϵ�
	timer_oc_parm.ocnidlestate = TIMER_OC_IDLE_STATE_LOW;
	timer_oc_parm.ocnpolarity = TIMER_OCN_POLARITY_HIGH;//���Կ����Ǹߵ�ƽ��Ч��ͨ����ζ�ŵ��������ﵽ�Ƚ�ֵʱ����ߵ�ƽ��
	timer_oc_parm.ocpolarity = TIMER_OCN_POLARITY_LOW;//���Կ����Ǹߵ�ƽ��Ч��ͨ����ζ�ŵ��������ﵽ�Ƚ�ֵʱ����ߵ�ƽ��
	timer_oc_parm.outputnstate = TIMER_CCX_DISABLE;
	timer_oc_parm.outputstate = TIMER_CCX_ENABLE;
	
	timer_channel_output_config(TIMER0,TIMER_CH_2,&timer_oc_parm);
	timer_channel_output_config(TIMER0,TIMER_CH_3,&timer_oc_parm);
	
	/* ����ռ�ձ� */
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_2,0);//PE13
	timer_channel_output_mode_config(TIMER0,TIMER_CH_2,TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_2,TIMER_OC_SHADOW_DISABLE);
 
	/* ����ռ�ձ� */
	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_3,0); //PE14
	timer_channel_output_mode_config(TIMER0,TIMER_CH_3,TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER0,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);
	
	timer_primary_output_config(TIMER0, ENABLE);
    /* ʹ�ܶ�ʱ�� */
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
* @brief  ����ֱ�����
* @param  None
* @retval None   
*/
static void Start()
{
    //�������
    if(DC_Motor.dir ==  Forward_Status)   //�������ķ�������ת
    {
        //HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);    //������ʱ��3ͨ��3��PWM���
    }
    else //�����������Ƿ�ת
    {
       // HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);    //������ʱ��3ͨ��4��PWM���
    }
    //���µ��״̬
    DC_Motor.status = Start_Status;
}

