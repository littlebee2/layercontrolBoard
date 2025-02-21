#include "Step_Motor.h"

static Struct_StepMotor Step_Motor;

//步进电机初始化
void StepMotor_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
	//配置模式 1:索引器模式 0:
	rt_pin_mode(DRV8834_CONFIG_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_CONFIG_PIN,PIN_HIGH);
	//设置成休眠模式 1:工作 0:休眠
	rt_pin_mode(DRV8834_SLEEP_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_SLEEP_PIN,PIN_HIGH);
	//设置失能模块 1:失能 0:使能
	rt_pin_mode(DRV8834_NENBL_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_NENBL_PIN,PIN_LOW);
	//设置方向 1:向前 0:向后，默认向前
	rt_pin_mode(DRV8834_DIR_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_DIR_PIN,PIN_HIGH);
	//配置步幅M0:M1 00:满步,01:1/2步 10:1/4步
	rt_pin_mode(DRV8834_M0_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_M0_PIN,PIN_LOW);
	rt_pin_mode(DRV8834_M1_PIN,PIN_MODE_OUTPUT);
	rt_pin_write(DRV8834_M1_PIN,PIN_LOW);
	//配置step,
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