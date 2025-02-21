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
//�������ƣ�Event_Init			                                    //
//�������¼���ʼ������	                                        //
//���룺��                                                      //
//�������                                                      //
//���أ���                                                      //
//���ܣ���ʼ��ͨѶ��Դ�Լ���ؼĴ�														  //
//==============================================================//
void Event_Init(void)
{
	memset(&event,0,sizeof(event));
}

//==============================================================//
//�������ƣ�Event_Handler		                                    //
//�������¼�����������	                                        //
//���룺��                                                      //
//�������                                                      //
//���أ���                                                      //
//���ܣ����͵�¼������һ�����֣���������Ӧ��ȳ�ʱ�¼�				//
//==============================================================//
void Event_Handler(void)
{
	static uint8_t time = 0;
	if(++time >= 200)
	{
		time = 0;
		if(!event.login.flag)		//δ��¼��ʱ��һֱ���͵�¼����ֱ��������Ӧ
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
//					if(!event.ack.timeOut)		//�ȴ�Ӧ�����Ӧ��ʱ
//					{
//						rt_kprintf("Wait Ack Time Out!\r\n");
//						if(event.ack.count < 15)
//						{
//							event.ack.count++;
//							event.report.reSendDelay = 200;		//�趨����ʧ�ܺ���ط���ʱ
//							event.report.count = 1;						//���ʹ�������
//							if(Event_DataSend(event.report.dat,event.report.len) == 0)
//								event.sendType = EVENT_SEND_TYPE_REPORT;
//						}
//						else
//						{
//							rt_kprintf("Send Report Not Ack!\r\n");				//�˴����������ݷ��ͺ���δӦ����
//							event.ack.flag = 0;
//							event.sendPID++;																													//�������ݰ�����ʧ�ܣ�PID��1�����´����ݰ����ͳɹ�����λ����ͨ��PID�������ж϶�������
//						}
//					}
//				}
//			}
//			if(event.res.reSendDelay)
//			{
//				event.res.reSendDelay--;
//				if(!event.res.reSendDelay)			//������Ӧ���ݳ�ʱ
//				{
//					rt_kprintf("Send Response Packet Time Out!\r\n");
//					if(event.res.count < 100)
//					{
//						event.res.reSendDelay = 20;				//�趨����ʧ�ܺ���ط���ʱ
//						if(Event_DataSend(event.res.dat,event.res.len) == 0)
//						{
//							event.sendType = EVENT_SEND_TYPE_RES;
//							event.res.count++;				//���ʹ�������
//						}
//					}
//					else
//					{
//						rt_kprintf("Send Response Packet Fail!\r\n");//�˴������ǵײ㷢��ʧ�ܵ��£�һ�㲻�����
//					}
//				}
//			}
//			if(event.report.reSendDelay)
//			{
//				event.report.reSendDelay--;
//				if(!event.report.reSendDelay)		//���ͱ��泬ʱ
//				{
//					rt_kprintf("Send Report Packet Time Out!\r\n");
//					if(event.report.len && (event.report.count < 100))
//					{
//						event.report.reSendDelay = 30;		//�趨����ʧ�ܺ���ط���ʱ
//						if(Event_DataSend(event.report.dat,event.report.len) == 0)
//						{
//							event.report.count++;				//���ʹ�������
//							event.sendType = EVENT_SEND_TYPE_REPORT;
//						}
//					}
//					else
//					{
//						rt_kprintf("Send Report Packet Fail!\r\n");			//�˴������ǵײ㷢��ʧ�ܵ��£�һ�㲻�����
//						event.ack.flag = 0;
//						event.ack.timeOut = 0;
//						event.sendPID++;																														//�������ݰ�����ʧ�ܣ�PID��1�����´����ݰ����ͳɹ�����λ����ͨ��PID�������ж϶�������
//					}
//				}
//			}
//		}
	}
}

//==============================================================//
//�������ƣ�Event_SendReportPacket                              //
//���������ͱ����������                                        //
//���룺code		������Ĺ�����																	//
//			*dat		���������                                      //
//			len			�������ݵĳ���													        //
//�������                                                      //
//���أ�0 �ɹ�		����:ʧ����	1:δ��¼	2:���ڵȴ�Ӧ��          //
//���ܣ���������һ�����ݣ�����λ��Ӧ��													//
//==============================================================//
uint8_t Event_SendReportPacket(uint8_t code,uint8_t *dat,uint8_t len)
{
	if(!event.login.flag)
		return 1;
	if(event.ack.flag)			//�жϵ�ǰ�Ƿ��ڵȴ�Ӧ��
		return 2;
	event.report.dat[0] = code;
	memcpy(&event.report.dat[1],dat,len);
	event.report.dat[len+1] = event.sendPID;
	event.report.len = len+2;
	
	event.report.reSendDelay = 200;		//�趨����ʧ�ܺ���ط���ʱ
	event.report.count = 1;						//���ʹ�������
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
//�������ƣ�Event_DataReceive			                              //
//�������յ����ݺ�Ļص�����																		//
//���룺*dat		���յ�����                                      //
//			len			�������ݵĳ���													        //
//			id			����������Դ																		//
//�������                                                      //
//���أ���																                      //
//���ܣ�������յ������ݣ��������������Լ����Ƿ��ظ�						//
//			ͬʱ���ö�Ӧ�Ĵ�������������													//
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
		if(dat[0]&0x80 && dat[0] != 0x90)		//Ӧ���/��Ӧ��
		{
			if(event.sendPID == dat[len-1])						//���Ӧ���PID�Ƿ���ȷ
				Event_AckPacketProcess(dat,len-1);
			else
				rt_kprintf("Ack Packet PID Error!\r\n");
		}
		else							//���ݰ�
		{
			if(event.login.flag)
			{
				if(event.receivePID == dat[len-1])			//���PID
				{
					rt_kprintf("Repeat Packet!\r\n");			//���յ��İ��ظ��������ϴε���Ӧ��
					if(event.res.len)
					{
						event.res.reSendDelay = 10;				//�趨����ʧ�ܺ���ط���ʱ
						event.res.count = 1;							//���ʹ�������
						if(Event_DataSend(event.res.dat,event.res.len) == 0)
							event.sendType = EVENT_SEND_TYPE_RES;
					}
					return;
				}
				else if(event.receivePID != (dat[len-1]-1))
					rt_kprintf("Receive PID Exception!\r\n");
				event.receivePID = dat[len-1];
				
				Event_DataPacketProcess(dat,len-1);			//�յ������ݳ���Ϊ��Ч���ݳ���-1����ȥPID��
			}
			else
				rt_kprintf("Not Login!\r\n");
		}
	}
	else if(id == DATA_ID_BROADCAST)
	{
		if(!(dat[0]&0x80))		//û�й㲥Ӧ���
		{
			Event_BroadcastPacketProcess(dat,len-1);
		}
		else
			rt_kprintf("Error,Broadcast Ack!\r\n");
	}
}


//==============================================================//
//�������ƣ�Event_DataPacketProcess                             //
//���������ݰ�������																					//
//���룺*dat		���յ�����                                      //
//			len			�������ݵĳ���													        //
//�������                                                      //
//���أ���																                      //
//���ܣ��������ݰ������������������ع���ִ�����ָ�����Ӧ	//
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
//�������ƣ�Event_BroadcastPacketProcess                        //
//�������㲥��������																					//
//���룺*dat		���յ�����                                      //
//			len			�������ݵĳ���													        //
//�������                                                      //
//���أ���																                      //
//���ܣ��������ݰ�����ִ�й㲥��ع���													//
//==============================================================//
static void Event_BroadcastPacketProcess(uint8_t *dat,uint8_t len)
{
	rt_kprintf("Broadcast Data\r\n");
	switch(dat[0])
	{
		case	EVENT_CODE_DEVICE:
			if((dat[1] == 0x00)&&(len == 2))		//�㲥��ַ��ѯID�����豸�Ͽ����µ�¼
			{
				event.login.flag = 0;
			}
		break;
	}
}

//==============================================================//
//�������ƣ�Event_AckPacketProcess			                        //
//������Ӧ���������																					//
//���룺*dat		���յ�����                                      //
//			len			�������ݵĳ���													        //
//�������                                                      //
//���أ���																                      //
//���ܣ��ж�Ӧ���Ƿ���ȷ��ͬʱִ��Ӧ���Ĳ���									//
//==============================================================//
static void Event_AckPacketProcess(uint8_t *dat,uint8_t len)
{
	rt_kprintf("Ack Packet...\r\n");
	if((event.ack.flag) && ((dat[0]&0x7F) == event.ack.code))		//��ǰӦ�����Ӧ�Ĳ�������ڵȴ��Ĳ����룬����ȴ�Ӧ���־
	{
		rt_kprintf("OK\r\n");
		event.ack.flag = 0;
		event.ack.timeOut = 0;
		event.sendPID++;
		if((event.ack.code == EVENT_CODE_DEVICE)&&!event.login.flag)
		{
			event.login.flag = 1;
			event.receivePID = 0xFF;		//����PID��FF���Ա��ڽ��յ�һ��ָ���00
			rt_kprintf("Login!\r\n");
		}
	}
	else
		rt_kprintf("Invalid Ack!\r\n");
}


//==============================================================//
//�������ƣ�Event_SendAckPacket					                        //
//������Ӧ������ͺ���																					//
//���룺code		Ӧ��Ĺ�����                                    //
//			status	Ӧ���״̬															        //
//�������                                                      //
//���أ���																                      //
//���ܣ�����һ��Ӧ���																					//
//==============================================================//
static void Event_SendAckPacket(uint8_t code,uint8_t status)
{
	Event_SendResponsePacket(code,&status,1);
}

//==============================================================//
//�������ƣ�Event_SendResponsePacket		                        //
//��������Ӧ�����ͺ���																					//
//���룺code		��Ӧ�Ĺ�����                                    //
//			*dat		��Ӧ������															        //
//			len			��Ӧ���ݳ���																		//
//�������                                                      //
//���أ���																                      //
//���ܣ�����һ����Ӧ����������λ��Ӧ��													//
//==============================================================//
static void Event_SendResponsePacket(uint8_t code,uint8_t *dat,uint8_t len)
{
	event.res.dat[0] = code|0x80;
	memcpy(&event.res.dat[1],dat,len);
	event.res.dat[len+1] = event.receivePID;
	event.res.len = len+2;
	event.res.reSendDelay = 200;			//�趨����ʧ�ܺ���ط���ʱ
	event.res.count = 1;							//���ʹ�������
	if(Event_DataSend(event.res.dat,event.res.len) == 0)
		event.sendType = EVENT_SEND_TYPE_RES;
}

//==============================================================//
//�������ƣ�Event_SendLoginPacket				                        //
//��������¼������һ�����ְ������ͺ���													//
//���룺��																											//
//�������                                                      //
//���أ���																                      //
//���ܣ��ϵ緢�͵ĵ�һ�����ݰ������ڸ���λ������								//
//==============================================================//
static void Event_SendLoginPacket(void)
{
	uint8_t dat[6];
	dat[0] = EVENT_CODE_DEVICE;
	dat[1] = 0x00;
	dat[2] = sys.type;
	dat[3] = (uint8_t)(sys.ver>>8);				//�汾��
	dat[4] = (uint8_t)sys.ver;
	event.sendPID = 0;
	dat[5] = event.sendPID;
	Can_SendPacket(0,DATA_ID_SELF,dat,6,1);
	event.ack.flag = 1;
	event.ack.code = EVENT_CODE_DEVICE;
	event.sendType = EVENT_SEND_TYPE_REPORT;
}

//==============================================================//
//�������ƣ�Event_DataSend							                        //
//���������ݷ��ͺ���																						//
//���룺*dat	�����͵�����																			//
//			len		���������ݵĳ���																	//
//�������                                                      //
//���أ�����״̬ 0 �ɹ�		1 ʧ��									              //
//���ܣ��������͵�����д��ײ㷢��															//
//==============================================================//
static uint8_t Event_DataSend(uint8_t *dat,uint8_t len)
{
//	if(event.sendType == EVENT_SEND_TYPE_EMPTY)		//��ǰû�������ݣ�����������
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
//�������ƣ�Event_DataSendComplete			                        //
//���������ݷ�����ɻص�����																		//
//���룺err	���ͺ�Ĵ�����																			//
//�������                                                      //
//���أ���																				              //
//���ܣ�һ�����ݷ�����Ϻ�Ļص�																//
//			���ڱ������ݷ�����ϣ��Լ����ͽ��											//
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
