#include "event.h"
#include "mycan.h"
#include "device.h"
#include "lock.h"

static Struct_Event event;
static void Event_DataSendComplete(uint8_t err);
static uint8_t Event_DataSend(uint8_t *dat,uint8_t len);
static void Event_SendLoginPacket(void);
static void Event_SendResponsePacket(uint8_t code,uint8_t *dat,uint8_t len);
static void Event_SendAckPacket(uint8_t code,uint8_t status);
static void Event_DataPacketProcess(uint8_t *dat,uint8_t len);
static void Event_BroadcastPacketProcess(uint8_t *dat,uint8_t len);
static void Event_AckPacketProcess(uint8_t *dat,uint8_t len);

//==============================================================//
//函数名称：Event_Init			                                    //
//描述：事件初始化函数	                                        //
//输入：无                                                      //
//输出：无                                                      //
//返回：无                                                      //
//功能：初始化通讯资源以及相关寄存														  //
//==============================================================//
void Event_Init(void)
{
	memset(&event,0,sizeof(event));
}

//==============================================================//
//函数名称：Event_Handler		                                    //
//描述：事件处理主函数	                                        //
//输入：无                                                      //
//输出：无                                                      //
//返回：无                                                      //
//功能：发送登录包（第一次握手），处理发送应答等超时事件				//
//==============================================================//
void Event_Handler(void)
{
	static uint8_t time = 0;
	if(++time >= 200)
	{
		time = 0;
		if(!event.login.flag)		//未登录的时候，一直发送登录包，直到主机响应
		{
			Event_SendLoginPacket();
		}
//		else
//		{
//			if(event.ack.flag)
//			{
//				if(event.ack.timeOut)
//				{
//					event.ack.timeOut--;
//					if(!event.ack.timeOut)		//等待应答或响应超时
//					{
//						rt_kprintf("Wait Ack Time Out!\r\n");
//						if(event.ack.count < 15)
//						{
//							event.ack.count++;
//							event.report.reSendDelay = 200;		//设定发送失败后的重发延时
//							event.report.count = 1;						//发送次数计数
//							if(Event_DataSend(event.report.dat,event.report.len) == 0)
//								event.sendType = EVENT_SEND_TYPE_REPORT;
//						}
//						else
//						{
//							rt_kprintf("Send Report Not Ack!\r\n");				//此处错误是数据发送后多次未应答导致
//							event.ack.flag = 0;
//							event.sendPID++;																													//本次数据包发送失败，PID加1，如下次数据包发送成功，上位机可通过PID不连续判断丢包多少
//						}
//					}
//				}
//			}
//			if(event.res.reSendDelay)
//			{
//				event.res.reSendDelay--;
//				if(!event.res.reSendDelay)			//发送响应数据超时
//				{
//					rt_kprintf("Send Response Packet Time Out!\r\n");
//					if(event.res.count < 100)
//					{
//						event.res.reSendDelay = 20;				//设定发送失败后的重发延时
//						if(Event_DataSend(event.res.dat,event.res.len) == 0)
//						{
//							event.sendType = EVENT_SEND_TYPE_RES;
//							event.res.count++;				//发送次数计数
//						}
//					}
//					else
//					{
//						rt_kprintf("Send Response Packet Fail!\r\n");//此处错误是底层发送失败导致，一般不会出现
//					}
//				}
//			}
//			if(event.report.reSendDelay)
//			{
//				event.report.reSendDelay--;
//				if(!event.report.reSendDelay)		//发送报告超时
//				{
//					rt_kprintf("Send Report Packet Time Out!\r\n");
//					if(event.report.len && (event.report.count < 100))
//					{
//						event.report.reSendDelay = 30;		//设定发送失败后的重发延时
//						if(Event_DataSend(event.report.dat,event.report.len) == 0)
//						{
//							event.report.count++;				//发送次数计数
//							event.sendType = EVENT_SEND_TYPE_REPORT;
//						}
//					}
//					else
//					{
//						rt_kprintf("Send Report Packet Fail!\r\n");			//此处错误是底层发送失败导致，一般不会出现
//						event.ack.flag = 0;
//						event.ack.timeOut = 0;
//						event.sendPID++;																														//本次数据包发送失败，PID加1，如下次数据包发送成功，上位机可通过PID不连续判断丢包多少
//					}
//				}
//			}
//		}
	}
}

//==============================================================//
//函数名称：Event_SendReportPacket                              //
//描述：发送报告包主函数                                        //
//输入：code		报告包的功能码																	//
//			*dat		报告的数据                                      //
//			len			报告数据的长度													        //
//输出：无                                                      //
//返回：0 成功		其他:失败码	1:未登录	2:正在等待应答          //
//功能：主动发送一包数据，需上位机应答													//
//==============================================================//
uint8_t Event_SendReportPacket(uint8_t code,uint8_t *dat,uint8_t len)
{
	if(!event.login.flag)
		return 1;
	if(event.ack.flag)			//判断当前是否在等待应答
		return 2;
	event.report.dat[0] = code;
	memcpy(&event.report.dat[1],dat,len);
	event.report.dat[len+1] = event.sendPID;
	event.report.len = len+2;
	
	event.report.reSendDelay = 200;		//设定发送失败后的重发延时
	event.report.count = 1;						//发送次数计数
	if(Event_DataSend(event.report.dat,event.report.len) == 0)
	{
		event.sendType = EVENT_SEND_TYPE_REPORT;
		event.ack.flag = 1;
		event.ack.count = 1;
		event.ack.code = code;
	}
	return 0;
}

//==============================================================//
//函数名称：Event_DataReceive			                              //
//描述：收到数据后的回调函数																		//
//输入：*dat		接收的数据                                      //
//			len			接收数据的长度													        //
//			id			接收数据来源																		//
//输出：无                                                      //
//返回：无																                      //
//功能：处理接收到的数据，解析数据类型以及包是否重复						//
//			同时调用对应的处理函数处理数据													//
//==============================================================//
void Event_DataReceive(uint8_t *dat,uint16_t len,uint32_t id)
{
	rt_kprintf("Receive %d Byte Data:",len);
	if(len < 2)
	{
		rt_kprintf("Len Error!\r\n");
		return;
	}
	if(id == DATA_ID_SELF)
	{
		if(dat[0]&0x80 && dat[0] != 0x90)		//应答包/响应包
		{
			if(event.sendPID == dat[len-1])						//检测应答的PID是否正确
				Event_AckPacketProcess(dat,len-1);
			else
				rt_kprintf("Ack Packet PID Error!\r\n");
		}
		else							//数据包
		{
			if(event.login.flag)
			{
				if(event.receivePID == dat[len-1])			//检测PID
				{
					rt_kprintf("Repeat Packet!\r\n");			//接收到的包重复，则发送上次的响应包
					if(event.res.len)
					{
						event.res.reSendDelay = 10;				//设定发送失败后的重发延时
						event.res.count = 1;							//发送次数计数
						if(Event_DataSend(event.res.dat,event.res.len) == 0)
							event.sendType = EVENT_SEND_TYPE_RES;
					}
					return;
				}
				else if(event.receivePID != (dat[len-1]-1))
					rt_kprintf("Receive PID Exception!\r\n");
				event.receivePID = dat[len-1];
				
				Event_DataPacketProcess(dat,len-1);			//收到的数据长度为有效数据长度-1，减去PID域
			}
			else
				rt_kprintf("Not Login!\r\n");
		}
	}
	else if(id == DATA_ID_BROADCAST)
	{
		if(!(dat[0]&0x80))		//没有广播应答包
		{
			Event_BroadcastPacketProcess(dat,len-1);
		}
		else
			rt_kprintf("Error,Broadcast Ack!\r\n");
	}
}


//==============================================================//
//函数名称：Event_DataPacketProcess                             //
//描述：数据包处理函数																					//
//输入：*dat		接收的数据                                      //
//			len			接收数据的长度													        //
//输出：无                                                      //
//返回：无																                      //
//功能：解析数据包，并按功能码调用相关功能执行相关指令，并响应	//
//==============================================================//
static void Event_DataPacketProcess(uint8_t *dat,uint8_t len)
{
	uint8_t retDat[100];
	uint8_t retLen,ret;
	ret = 0xFF;
	retLen = 0;
	switch(dat[0])
	{
		case	EVENT_CODE_DEVICE:
			ret = Device_CMDProcess(&dat[1],len-1,retDat,&retLen);
		break;
		#ifdef	_LOCK_H
		case	EVENT_CODE_LOCK:
			ret = Lock_CMDProcess(&dat[1],len-1,retDat,&retLen);
		break;
		#endif
		#ifdef	_WEIGHT_H
		case	EVENT_CODE_WEIGHT:
			ret = Weight_CMDProcess(&dat[1],len-1,retDat,&retLen);
		break;
		#endif
		case EVENT_CODE_MISCELL:
//			if(dat[1] == EVENT_SUBCODE_WS2811)
//				ret = Ws2811_CMDProcess(&dat[2],len, retDat, &retLen);
//			else if(dat[1] ==EVENT_SUBCODE_DHT11)
//				ret = DHT11_CMDProcess(&dat[2],len, retDat, &retLen);
//			
		break;
	}
	if(retLen)
		Event_SendResponsePacket(dat[0],retDat,retLen);
}

//==============================================================//
//函数名称：Event_BroadcastPacketProcess                        //
//描述：广播包处理函数																					//
//输入：*dat		接收的数据                                      //
//			len			接收数据的长度													        //
//输出：无                                                      //
//返回：无																                      //
//功能：解析数据包，并执行广播相关功能													//
//==============================================================//
static void Event_BroadcastPacketProcess(uint8_t *dat,uint8_t len)
{
	rt_kprintf("Broadcast Data\r\n");
	switch(dat[0])
	{
		case	EVENT_CODE_DEVICE:
			if((dat[1] == 0x00)&&(len == 2))		//广播地址查询ID，则设备断开重新登录
			{
				event.login.flag = 0;
			}
		break;
	}
}

//==============================================================//
//函数名称：Event_AckPacketProcess			                        //
//描述：应答包处理函数																					//
//输入：*dat		接收的数据                                      //
//			len			接收数据的长度													        //
//输出：无                                                      //
//返回：无																                      //
//功能：判断应答是否正确，同时执行应答后的操作									//
//==============================================================//
static void Event_AckPacketProcess(uint8_t *dat,uint8_t len)
{
	rt_kprintf("Ack Packet...\r\n");
	if((event.ack.flag) && ((dat[0]&0x7F) == event.ack.code))		//当前应答或响应的操作码等于等待的操作码，清除等待应答标志
	{
		rt_kprintf("OK\r\n");
		event.ack.flag = 0;
		event.ack.timeOut = 0;
		event.sendPID++;
		if((event.ack.code == EVENT_CODE_DEVICE)&&!event.login.flag)
		{
			event.login.flag = 1;
			event.receivePID = 0xFF;		//接收PID置FF，以便于接收第一条指令的00
			rt_kprintf("Login!\r\n");
		}
	}
	else
		rt_kprintf("Invalid Ack!\r\n");
}


//==============================================================//
//函数名称：Event_SendAckPacket					                        //
//描述：应答包发送函数																					//
//输入：code		应答的功能码                                    //
//			status	应答的状态															        //
//输出：无                                                      //
//返回：无																                      //
//功能：发送一个应答包																					//
//==============================================================//
static void Event_SendAckPacket(uint8_t code,uint8_t status)
{
	Event_SendResponsePacket(code,&status,1);
}

//==============================================================//
//函数名称：Event_SendResponsePacket		                        //
//描述：响应包发送函数																					//
//输入：code		响应的功能码                                    //
//			*dat		响应的数据															        //
//			len			响应数据长度																		//
//输出：无                                                      //
//返回：无																                      //
//功能：发送一个响应包，无需上位机应答													//
//==============================================================//
static void Event_SendResponsePacket(uint8_t code,uint8_t *dat,uint8_t len)
{
	event.res.dat[0] = code|0x80;
	memcpy(&event.res.dat[1],dat,len);
	event.res.dat[len+1] = event.receivePID;
	event.res.len = len+2;
	event.res.reSendDelay = 200;			//设定发送失败后的重发延时
	event.res.count = 1;							//发送次数计数
	if(Event_DataSend(event.res.dat,event.res.len) == 0)
		event.sendType = EVENT_SEND_TYPE_RES;
}

//==============================================================//
//函数名称：Event_SendLoginPacket				                        //
//描述：登录包（第一次握手包）发送函数													//
//输入：无																											//
//输出：无                                                      //
//返回：无																                      //
//功能：上电发送的第一个数据包，用于跟上位机握手								//
//==============================================================//
static void Event_SendLoginPacket(void)
{
	uint8_t dat[6];
	dat[0] = EVENT_CODE_DEVICE;
	dat[1] = 0x00;
	dat[2] = sys.type;
	dat[3] = (uint8_t)(sys.ver>>8);				//版本号
	dat[4] = (uint8_t)sys.ver;
	event.sendPID = 0;
	dat[5] = event.sendPID;
	Can_SendPacket(0,DATA_ID_SELF,dat,6,1);
	event.ack.flag = 1;
	event.ack.code = EVENT_CODE_DEVICE;
	event.sendType = EVENT_SEND_TYPE_REPORT;
}

//==============================================================//
//函数名称：Event_DataSend							                        //
//描述：数据发送函数																						//
//输入：*dat	待发送的数据																			//
//			len		待发送数据的长度																	//
//输出：无                                                      //
//返回：发送状态 0 成功		1 失败									              //
//功能：将待发送的数据写入底层发送															//
//==============================================================//
static uint8_t Event_DataSend(uint8_t *dat,uint8_t len)
{
//	if(event.sendType == EVENT_SEND_TYPE_EMPTY)		//当前没法送数据，则立即发送
//	{
//		return Can_SendPacket(0,DATA_ID_SELF,dat,len,1);
//	}
//	else
//	{
//		return 1;
//	}
	Can_SendPacket(0,DATA_ID_SELF,dat,len,1);
}

//==============================================================//
//函数名称：Event_DataSendComplete			                        //
//描述：数据发送完成回调函数																		//
//输入：err	发送后的错误码																			//
//输出：无                                                      //
//返回：无																				              //
//功能：一包数据发送完毕后的回调																//
//			用于报告数据发送完毕，以及发送结果											//
//==============================================================//
static void Event_DataSendComplete(uint8_t err)
{
	if(err)
	{
		rt_kprintf("Data Send Fail!err=%d\r\n",err);
	}
	else
	{
		if(event.sendType == EVENT_SEND_TYPE_REPORT)
		{
			event.report.reSendDelay = 0;
			if(event.ack.flag)
			{
				event.ack.timeOut = 2000;
			}
		}
		else if(event.sendType == EVENT_SEND_TYPE_RES)
		{
			event.res.reSendDelay = 0;
		}
	}
	event.sendType = EVENT_SEND_TYPE_EMPTY;
}
