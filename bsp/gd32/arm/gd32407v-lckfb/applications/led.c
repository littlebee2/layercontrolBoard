#include "led.h"
#include <rtthread.h>
#include <board.h>

static rt_base_t LedPin[LED_NUM] = {GET_PIN(C,13),GET_PIN(C,14),GET_PIN(C, 15)};						//ָ��ÿ��LED��ռ�õ�PIN
static u8 LedOpenStatus[LED_NUM] = {0,0,0};																									//ָ��ÿ��LED�ƿ�����ƽ״̬ �� 1������0�أ��� 0������1��

static Struct_LedType led[LED_NUM];

static void LED_SetON(u8 id);
static void LED_SetOFF(u8 id);
static void LED_Control(u8 id);

//==============================================================//
//�������ƣ�LED_Init					                                  //
//������LED��ʼ������        				               	    				//
//���룺��                                                      //
//�������                                                      //
//���أ���                                                      //
//���ܣ���ʼ��LEDָʾ�������Դ														      //
//==============================================================//
void LED_Init(void)
{
	memset(led,0,sizeof(led));
	for(rt_uint8_t i = 0; i<LED_NUM; i++)
	{
		rt_pin_mode(LedPin[i],PIN_MODE_OUTPUT);
		rt_pin_write(LedPin[i],PIN_HIGH);
	}
		
}

//==============================================================//
//�������ƣ�LED_Handler				                                 	//
//������LEDָʾ��������         	                             	//
//���룺��                                                      //
//�������                                                      //
//���أ���                                                      //
//���ܣ���ʱ����LEDָʾ��		                                    //
//==============================================================//
void LED_Handler(void)
{
	for(rt_uint8_t i=0;i<LED_NUM;i++)
		LED_Control(i);
}

//==============================================================//
//�������ƣ�LED_Open					                                 	//
//������LEDָʾ�ƿ�������        	                             	//
//���룺id	ָ����LED��	0:���е�  ����:ָ��������               //
//�������                                                      //
//���أ���                                                      //
//���ܣ�����ָ����LED�Ƴ���	                                    //
//==============================================================//
void LED_Open(u8 id)
{
	if(id == 0)
	{
		u8 i;
		for(i=0;i<LED_NUM;i++)
		{
			led[i].mode = LED_OPEN;
			LED_SetON(i);
		}
	}
	else if(id <= LED_NUM)
	{
		led[id-1].mode = LED_OPEN;
		LED_SetON(id-1);
	}
}

//==============================================================//
//�������ƣ�LED_Close					                                 	//
//������LEDָʾ�ƹرճ���        	                             	//
//���룺id	ָ����LED��	0:���е�  ����:ָ��������               //
//�������                                                      //
//���أ���                                                      //
//���ܣ�����ָ����LED�ƹر�	                                    //
//==============================================================//
void LED_Close(u8 id)
{
	if(id == 0)
	{
		u8 i;
		for(i=0;i<LED_NUM;i++)
		{
			led[i].mode = LED_CLOSE;
			LED_SetOFF(i);
		}
	}
	else if(id <= LED_NUM)
	{
		led[id-1].mode = LED_CLOSE;
		LED_SetOFF(id-1);
	}
}

//==============================================================//
//�������ƣ�LED_Flash					                                 	//
//������LEDָʾ����˸����        	                             	//
//���룺id		ָ����LED��	0:���е�  ����:ָ��������             //
//			space	��˸���	��λ ms																	//
//�������                                                      //
//���أ���                                                      //
//���ܣ�����ָ����LED�ư�ָ��ʱ������˸                       //
//==============================================================//
void LED_Flash(u8 id,u16 space)
{
	if(id == 0)
	{
		u8 i;
		for(i=0;i<LED_NUM;i++)
		{
			led[i].mode = LED_FLASH;
			led[i].space = space;
			led[i].delay = space;
			LED_SetON(i);
		}
	}
	else if(id <= LED_NUM)
	{
		led[id-1].mode = LED_FLASH;
		led[id-1].space = space;
		led[id-1].delay = space;
		LED_SetON(id-1);
	}
}

//==============================================================//
//�������ƣ�LED_OpenSingle		                                 	//
//������ָ��LED����һ�κ���        	                            //
//���룺id		ָ����LED��	0:���е�  ����:ָ��������             //
//			time	ʱ��	��λ ms																			//
//�������                                                      //
//���أ���                                                      //
//���ܣ�����ָ����LED�ư�ָ��ʱ����һ��			                    //
//==============================================================//
void LED_OpenSingle(u8 id,u16 time)
{
	if(id == 0)
	{
		u8 i;
		for(i=0;i<LED_NUM;i++)
		{
			led[i].mode = LED_SINGLE;
			led[i].delay = time;
			LED_SetON(i);
		}
	}
	else if(id <= LED_NUM)
	{
		led[id-1].mode = LED_SINGLE;
		led[id-1].delay = time;
		LED_SetON(id-1);
	}
}

//==============================================================//
//�������ƣ�LED_Control				                                 	//
//������LED�ƿ��ƺ���        	                            			//
//���룺id		ָ����LED��														            //
//�������                                                      //
//���أ���                                                      //
//���ܣ�����ǰָ��LED��ģʽ��������ض���                       //
//==============================================================//
static void LED_Control(u8 id)
{
	if(led[id].mode == LED_FLASH)
	{
		if(led[id].delay)
		{
			led[id].delay--;
			if(!led[id].delay)
			{
				led[id].delay = led[id].space;
				if(led[id].flash_level == 0)
				{
					led[id].flash_level = 1;
					rt_pin_write(LedPin[id],PIN_HIGH);
				}		
				else
				{
					led[id].flash_level = 0;
					rt_pin_write(LedPin[id],PIN_LOW);
				}
			}
		}
	}
	else if(led[id].mode == LED_SINGLE)
	{
		if(led[id].delay)
		{
			led[id].delay--;
			if(!led[id].delay)
			{
				LED_Close(id+1);
			}
		}
	}
}

//==============================================================//
//�������ƣ�LED_SetON					                                 	//
//������LED��������																							//
//���룺id	ָ����LED��																					//
//�������																											//
//���أ���                                                      //
//���ܣ�����ָ����LED��																					//
//			����ͬLED�Ƶ�����ƽ���ܲ�ͬ��OPEN�������ܵ�����					//
//==============================================================//
static void LED_SetON(u8 id)
{
	if(LedOpenStatus[id] == 0)
	{
		led[id].flash_level = 0;
		rt_pin_write(LedPin[id], PIN_LOW);
	}
	else
	{
		led[id].flash_level = 1;
		rt_pin_write(LedPin[id], PIN_HIGH);
	}
		
}

//==============================================================//
//�������ƣ�LED_SetOFF				                                 	//
//������LEDϨ����																							//
//���룺id	ָ����LED��																					//
//�������																											//
//���أ���                                                      //
//���ܣ�Ϩ��ָ����LED��																					//
//			����ͬLED�Ƶ�����ƽ���ܲ�ͬ��CLOSE��������Ϩ��				//
//==============================================================//
static void LED_SetOFF(u8 id)
{
	if(LedOpenStatus[id] == 0)
	{
		led[id].flash_level = 1;
		rt_pin_write(LedPin[id], PIN_HIGH);
	}
	else
	{
		led[id].flash_level = 0;
		rt_pin_write(LedPin[id], PIN_LOW);
	}
		
}
