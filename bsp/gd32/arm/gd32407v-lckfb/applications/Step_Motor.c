#include "Step_Motor.h"

static Struct_StepMotor Step_Motor;

//���������ʼ��
void StepMotor_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
	//����ģʽ 1:������ģʽ 0:
	rt_pin_mode(DRV8834_CONFIG_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_CONFIG_PIN,PIN_HIGH);
	//���ó�����ģʽ 1:���� 0:����
	rt_pin_mode(DRV8834_SLEEP_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_SLEEP_PIN,PIN_HIGH);
	//����ʧ��ģ�� 1:ʧ�� 0:ʹ��
	rt_pin_mode(DRV8834_NENBL_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_NENBL_PIN,PIN_LOW);
	//���÷��� 1:��ǰ 0:���Ĭ����ǰ
	rt_pin_mode(DRV8834_DIR_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_DIR_PIN,PIN_HIGH);
	//���ò���M0:M1 00:����,01:1/2�� 10:1/4��
	rt_pin_mode(DRV8834_M0_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_M0_PIN,PIN_LOW);
	rt_pin_mode(DRV8834_M1_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_M1_PIN,PIN_LOW);
	//����step,
	rt_pin_mode(DRV8834_STEP_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_STEP_PIN,PIN_HIGH);
}

void StepMotor_Handler(void)
{
	static uint16_t i=0;
	static uint16_t time=0;
	if(time > 10)
	{
		time =0;
		if(i == 0)
		{
			i=1;
			rt_pin_write(DRV8834_STEP_PIN,PIN_LOW);
		}
		else
		{
			i=0;
			rt_pin_write(DRV8834_STEP_PIN,PIN_HIGH);
		}
	}
	else
		time++;
}