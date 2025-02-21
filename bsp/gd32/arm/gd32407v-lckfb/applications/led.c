#include "led.h"
#include <rtthread.h>
#include <board.h>

static rt_base_t LedPin[LED_NUM] = {GET_PIN(C,13),GET_PIN(C,14),GET_PIN(C, 15)};						//指定每个LED灯占用的PIN
static u8 LedOpenStatus[LED_NUM] = {0,0,0};																									//指定每个LED灯开启电平状态 如 1开，则0关，如 0开，则1关

static Struct_LedType led[LED_NUM];

static void LED_SetON(u8 id);
static void LED_SetOFF(u8 id);
static void LED_Control(u8 id);

//==============================================================//
//函数名称：LED_Init					                                  //
//描述：LED初始化函数        				               	    				//
//输入：无                                                      //
//输出：无                                                      //
//返回：无                                                      //
//功能：初始化LED指示灯相关资源														      //
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
//函数名称：LED_Handler				                                 	//
//描述：LED指示灯主程序         	                             	//
//输入：无                                                      //
//输出：无                                                      //
//返回：无                                                      //
//功能：定时处理LED指示灯		                                    //
//==============================================================//
void LED_Handler(void)
{
	for(rt_uint8_t i=0;i<LED_NUM;i++)
		LED_Control(i);
}

//==============================================================//
//函数名称：LED_Open					                                 	//
//描述：LED指示灯开启程序        	                             	//
//输入：id	指定的LED灯	0:所有灯  其他:指定单个灯               //
//输出：无                                                      //
//返回：无                                                      //
//功能：设置指定的LED灯常开	                                    //
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
//函数名称：LED_Close					                                 	//
//描述：LED指示灯关闭程序        	                             	//
//输入：id	指定的LED灯	0:所有灯  其他:指定单个灯               //
//输出：无                                                      //
//返回：无                                                      //
//功能：设置指定的LED灯关闭	                                    //
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
//函数名称：LED_Flash					                                 	//
//描述：LED指示灯闪烁程序        	                             	//
//输入：id		指定的LED灯	0:所有灯  其他:指定单个灯             //
//			space	闪烁间隔	单位 ms																	//
//输出：无                                                      //
//返回：无                                                      //
//功能：设置指定的LED灯按指定时间间隔闪烁                       //
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
//函数名称：LED_OpenSingle		                                 	//
//描述：指定LED灯亮一次函数        	                            //
//输入：id		指定的LED灯	0:所有灯  其他:指定单个灯             //
//			time	时间	单位 ms																			//
//输出：无                                                      //
//返回：无                                                      //
//功能：设置指定的LED灯按指定时间亮一次			                    //
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
//函数名称：LED_Control				                                 	//
//描述：LED灯控制函数        	                            			//
//输入：id		指定的LED灯														            //
//输出：无                                                      //
//返回：无                                                      //
//功能：按当前指定LED灯模式控制其相关动作                       //
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
//函数名称：LED_SetON					                                 	//
//描述：LED点亮函数																							//
//输入：id	指定的LED灯																					//
//输出：无																											//
//返回：无                                                      //
//功能：点亮指定的LED灯																					//
//			（不同LED灯点亮电平可能不同，OPEN函数不管点亮）					//
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
//函数名称：LED_SetOFF				                                 	//
//描述：LED熄灭函数																							//
//输入：id	指定的LED灯																					//
//输出：无																											//
//返回：无                                                      //
//功能：熄灭指定的LED灯																					//
//			（不同LED灯点亮电平可能不同，CLOSE函数不管熄灭）				//
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
