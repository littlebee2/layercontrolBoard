#include "lock.h"
#include "eeprom.h"

static Lock_Type lock[LOCK_NUM];//�����ṹ��
static LockConfig_Type lockConfig;//���ýṹ��
static uint8_t lockConfigUpdate;//�������ر�־λ

rt_base_t LOCK_PWR_Pin[LOCK_NUM] = {GET_PIN(C,0)};
rt_base_t LOCK_DET_Pin[LOCK_NUM] = {GET_PIN(C,1)};

static uint8_t Lock_Open(uint8_t id);
static uint8_t Lock_Close(uint8_t id);
static void Lock_Scan(uint8_t id);
static uint8_t Lock_QueryStatus(uint8_t id);
uint8_t Lock_OpenForced(uint8_t id);
//static void Lock_Report(uint8_t *dat,uint8_t len);
static void Lock_StatusReport(uint8_t num,uint8_t status);
static void Lock_ErrorReport(uint8_t num,uint8_t err);

/*==============================================================
****�������ƣ�lock_thread
****������can���ݽ����߳�
****���룺��
****�������
****���أ���
****���ܣ���������
==============================================================*/
static void lockHandler_thread(void *parameter)
{
	while(1)
	{
		Lock_Handler();
		rt_thread_mdelay(10);
	}
}

//==============================================================//
//�������ƣ�Lock_Init					                                  //
//�����������Ƴ����ʼ������        				               	    //
//���룺��                                                      //
//�������                                                      //
//���أ���                                                      //
//���ܣ���ʼ���������������Դ														      //
//==============================================================//
void Lock_Init(void)
{
	rt_thread_t thread;
	fm24clxx_read_page(eeprom_dev,FM24CLXX_ADDR_7BIT,EEPROM_ADDR_LOCK_CONFIG,(uint8_t*)&lockConfig,sizeof(lockConfig));//��ȡ���ò���
	if(LOCK_CONFIG_HEAD == lockConfig.head)//�Ѿ����ù���һ��
	{
		
	}
	else
	{
		lock[0].type = LOCK_TYPE_GENERAL;
		lock[0].lockLevel = 0,
		lock[0].openLevel = 1;
		memset(lockConfig.config,lock[0].config,sizeof(lockConfig.config));
		lockConfigUpdate = 1;
	}
	for(uint8_t i=0;i<LOCK_NUM;i++)
	{
		rt_pin_mode(LOCK_PWR_Pin[i],PIN_MODE_OUTPUT);
		rt_pin_mode(LOCK_DET_Pin[i],PIN_MODE_INPUT);
		lock[i].lock = 1;
		lock[i].lockBuf = lock[i].lock;
		lock[i].config = lockConfig.config[i];
		if(lock[i].type >= LOCK_TYPE_MAX)
		{
			lock[i].type = LOCK_TYPE_GENERAL;
			lock[i].lockLevel = 0,
			lock[i].openLevel = 1;
		}
	}
	Lock_Close(0);
	
	//���߳�
	// ���������߳�
	thread = rt_thread_create("lock_thread", lockHandler_thread, RT_NULL, 512, 25, 10);
	if (thread != RT_NULL)
	{
		rt_thread_startup(thread);
	}
	else
	{
		rt_kprintf("create can_rx thread failed!\r\n");
	}
}

//==============================================================//
//�������ƣ�Lock_Handler			                                 	//
//������������������         	                                	//
//���룺��                                                      //
//�������                                                      //
//���أ���                                                      //
//���ܣ�����������¼�                                    			//
//==============================================================//
void Lock_Handler(void)
{
	for(uint8_t i=0;i < LOCK_NUM;i++)
	{
		if(lock[i].delay)
		{
			lock[i].delay--;
			if(!lock[i].delay)
			{
				if(lock[i].type == LOCK_TYPE_GENERAL)
				{
					Lock_Close(i+1);
					if(!lock[i].forcedFlag)
					{
						rt_kprintf("Lock %d Open Fail!\r\n",i+1);
						Lock_ErrorReport(i+1,LOCK_ERR_OPEN_FAIL);
					}
					lock[i].forcedFlag = 0;
				}
				else //if(lock[i].config.type == LOCK_TYPE_DOOR)
				{
					Lock_Close(i+1);
				}
			}
		}
		Lock_Scan(i);
	}
	//Lock_ReportCheck();
	if(lockConfigUpdate)
	{
		lockConfigUpdate = 0;
		lockConfig.head = LOCK_CONFIG_HEAD;
		fm24clxx_write_page(eeprom_dev,FM24CLXX_ADDR_7BIT,EEPROM_ADDR_LOCK_CONFIG,(uint8_t*)&lockConfig,sizeof(lockConfig));
	}
}

//==============================================================//
//�������ƣ�Lock_CMDProcess				                              //
//��������ָ�����                                          //
//���룺*datIn	�������ݴ�ŵ�ַ                                //
//			lenIn		�������ݳ���                                    //
//�����*datOut		���������ݳ��ȴ�ŵ�ַ                        //
//			*lenOut		���������ݳ���                                //
//���أ�ִ�н�� 0 �ɹ�����0 �������                           //
//���ܣ�������λ��������ָ��															      //
//==============================================================//
uint8_t Lock_CMDProcess(uint8_t *datIn,uint8_t lenIn,uint8_t *datOut,uint8_t *lenOut)
{
	uint8_t res;
	res = 0xFF;
	switch(datIn[0])
	{
		case	CMD_LOCK_QUERY_STATUS:
			if((lenIn >= 2)&&(datIn[1] <= LOCK_NUM))
			{
				datOut[(*lenOut)++] = CMD_LOCK_QUERY_STATUS;
				datOut[(*lenOut)++] = datIn[1];
				if(datIn[1] == 0)
				{
					uint8_t i;
					for(i=0;i<LOCK_NUM;i++)
					{
						datOut[(*lenOut)++] = Lock_QueryStatus(i+1);
					}
				}
				else
				{
					datOut[(*lenOut)++] = Lock_QueryStatus(datIn[1]);
				}
			}
		break;
		case	CMD_LOCK_OPEN:
			if(lenIn >= 2)
			{
				datOut[(*lenOut)++] = CMD_LOCK_OPEN;
				datOut[(*lenOut)++] = Lock_Open(datIn[1]);
			}
		break;
		case	CMD_LOCK_CLOSE:
			if(lenIn >= 2)
			{
				datOut[(*lenOut)++] = CMD_LOCK_CLOSE;
				datOut[(*lenOut)++] = Lock_Close(datIn[1]);
			}
		break;
		case	CMD_LOCK_FORCED_OPEN:
			if(lenIn >= 2)
			{
				datOut[(*lenOut)++] = CMD_LOCK_FORCED_OPEN;
				datOut[(*lenOut)++] = Lock_OpenForced(datIn[1]);
			}
		break;
		case	CMD_LOCK_CONFIG:
			if(lenIn >= 5)
			{
				datOut[(*lenOut)++] = CMD_LOCK_CONFIG;
				if(datIn[2] < LOCK_TYPE_MAX)
				{
					if(datIn[1] <= LOCK_NUM)
					{
						if(datIn[1] == 0)
						{
							for(uint8_t i=0;i<LOCK_NUM;i++)
							{
								lock[i].type = (Enum_LockType)datIn[2];
								lock[i].openLevel = datIn[3];
								lock[i].lockLevel = datIn[4];
								lockConfig.config[i] = lock[i].config;
								Lock_Close(i+1);
							}
						}
						else// if(datIn[1] <= LOCK_NUM)
						{
							uint8_t num = datIn[1]-1;;
							lock[num].config = datIn[2];
							lock[num].openLevel = datIn[3];
							lock[num].lockLevel = datIn[4];
							lockConfig.config[num] = lock[num].config;
							Lock_Close(num+1);
						}
						datOut[(*lenOut)++] = 0;
						lockConfigUpdate = 1;
					}
					else
						datOut[(*lenOut)++] = LOCK_ERR_OUT_RANGE;
				}
				else
					datOut[(*lenOut)++] = LOCK_ERR_INVALID_TYPE;
			}
		break;
		case	CMD_LOCK_READ_CONFIG:
			if(lenIn >= 2)
			{
				datOut[(*lenOut)++] = CMD_LOCK_READ_CONFIG;
				datOut[(*lenOut)++] = datIn[1];
				if(datIn[1] == 0)
				{
					for(uint8_t i=0;i<LOCK_NUM;i++)
					{
						datOut[(*lenOut)++] = lock[i].type;
						datOut[(*lenOut)++] = lock[i].openLevel;
						datOut[(*lenOut)++] = lock[i].lockLevel;
					}
				}
				else if(datIn[1] <= LOCK_NUM)
				{
					datOut[(*lenOut)++] = lock[datIn[1]-1].type;
					datOut[(*lenOut)++] = lock[datIn[1]-1].openLevel;
					datOut[(*lenOut)++] = lock[datIn[1]-1].lockLevel;
				}
				else
				{
					datOut[(*lenOut)++] = 0xFF;
					datOut[(*lenOut)++] = 0xFF;
					datOut[(*lenOut)++] = 0xFF;
				}
			}
		break;
		default:
			rt_kprintf("Unknown CMD!\r\n");
		break;
	}
	return res;
}

static void Lock_ErrorReport(uint8_t num,uint8_t err)
{
	uint8_t dat[3];
	dat[0] = CMD_LOCK_ERROR_REPORT;
	dat[1] = num;
	dat[2] = err;
	//Lock_Report(dat,3);
}

static void Lock_StatusReport(uint8_t num,uint8_t status)
{
	uint8_t dat[3];
	dat[0] = CMD_LOCK_QUERY_STATUS;
	dat[1] = num;
	dat[2] = status;
	//Lock_Report(dat,3);
}

//==============================================================//
//�������ƣ�Lock_Open							                              //
//��������������			                                          //
//���룺id	����������ID				                                //
//�������                                                      //
//���أ�0 �ɹ�	1 ָ��ID��������		                           	//
//���ܣ�����ָ��ID��������ID=0ʱ������������							      //
//==============================================================//
static uint8_t Lock_Open(uint8_t id)
{
	if(id > LOCK_NUM)
		return 1;
	if(id)
	{
		id--;
		if(lock[id].type == LOCK_TYPE_GENERAL)
		{
			if(lock[id].lock)
			{
				lock[id].delay = 50;		//���ÿ�������ʱ�䣬������ʱ��δ�򿪣�Ϊ��ֹ���ڴ�ʱ���ǿ�ƶϵ�
				if(lock[id].openLevel)
					rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
				else
					rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
			}
		}
		else if(lock[id].type == LOCK_TYPE_DOOR)
		{
			lock[id].delay = 2000;		//�趨���չ�������ʱ�䣬ʱ�䵽��δ�������Զ��ر�
			if(lock[id].openLevel)
				rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
			else
				rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
		}
	}
	return 0;
}

//==============================================================//
//�������ƣ�Lock_OpenForced				                              //
//������ǿ�ƿ�������	                                          //
//���룺id	����������ID				                                //
//�������                                                      //
//���أ�0 �ɹ�	1 ָ��ID��������		                           	//
//���ܣ�����ָ��ID��������ID=0ʱ������������							      //
//==============================================================//
uint8_t Lock_OpenForced(uint8_t id)
{
	if(id > LOCK_NUM)
		return 1;
	if(id)
	{
		id--;
		if(lock[id].type == LOCK_TYPE_GENERAL)
		{
			lock[id].delay = 50;		//���ÿ�������ʱ�䣬������ʱ��δ�򿪣�Ϊ��ֹ���ڴ�ʱ���ǿ�ƶϵ�
			lock[id].forcedFlag = 1;
			if(lock[id].openLevel)
				rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
			else
				rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
		}
		else if(lock[id].type == LOCK_TYPE_DOOR)
		{
			lock[id].delay = 2000;		//�趨���չ�������ʱ�䣬ʱ�䵽��δ�������Զ��ر�
			if(lock[id].openLevel)
					rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
				else
					rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
		}
	}
	return 0;
}

//==============================================================//
//�������ƣ�Lock_Close						                              //
//��������������			                                          //
//���룺id	�رյ�����ID				                                //
//�������																											//
//���أ�0 �ɹ�	1 ָ��ID��������		                           	//
//���ܣ��ر�ָ��ID��������ID=0ʱ���ر�������							      //
//==============================================================//
static uint8_t Lock_Close(uint8_t id)
{
	if(id > LOCK_NUM)
		return 1;
	if(id)
	{
		id--;
		lock[id].delay = 0;
		if(lock[id].openLevel)
			rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
		else
			rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
	}
	else
	{
		uint8_t i;
		for(i=0;i < LOCK_NUM;i++)
		{
			lock[i].delay = 0;
			if(lock[i].openLevel)
				rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
			else
				rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
		}
	}
	return 0;
}

//==============================================================//
//�������ƣ�Lock_Scan							                              //
//������ɨ����״̬����                                          //
//���룺id	ɨ�������ID				                                //
//�������																                      //
//���أ���                                                      //
//���ܣ�ɨ����״̬��������																      //
//==============================================================//
static void Lock_Scan(uint8_t id)
{
	uint8_t temp;
	if(lock[id].statusDelay)
	{
		lock[id].statusDelay--;
	}
	
	temp = rt_pin_read(LOCK_DET_Pin[id])? 1:0;
	if(temp == lock[id].lockLevel)
		temp = 1;
	else
		temp = 0; 
	if(temp != lock[id].lock) 
	{
		if(temp != lock[id].lockBuf)  //����
		{
			lock[id].lockBuf = temp;
			if(lock[id].lockBuf)
				lock[id].statusDelay = 50;				//��������ʱ500ms
			else
				lock[id].statusDelay = 10;				//��������ʱ100ms
		}
		else if(!lock[id].statusDelay)
		{
			lock[id].lock = lock[id].lockBuf;
			if(lock[id].lock)
			{
				if(lock[id].type == LOCK_TYPE_DOOR)
				{
					//��Ϊ���չ����ϣ���رտ���
					Lock_Close(id+1);
				}
				rt_kprintf("Lock %d Locked!\r\n",id+1);
				Lock_StatusReport(id+1,LOCK_STATUS_LOCKED);
			}
			else
			{
				rt_kprintf("Lock %d Unlocked!\r\n",id+1);
				if(lock[id].type == LOCK_TYPE_DOOR)
				{
					lock[id].delay = 6000;		//���չ����������趨�Զ�����
					temp = rt_pin_read(LOCK_DET_Pin[id])? 1:0;
					if (temp  == lock[id].lockLevel)
						Lock_StatusReport(id+1,LOCK_STATUS_LOCKED);
					else
						Lock_StatusReport(id+1,LOCK_STATUS_UNLOCKED); 
				}
				else
				{
					Lock_StatusReport(id+1,LOCK_STATUS_UNLOCKED);
					if(!lock[id].forcedFlag)		//��Ϊǿ�ƿ���������򿪺����
					{
						lock[id].delay = 0;
						Lock_Close(id+1);							
					}
				}
			}                                   
		}
	}
	else
		lock[id].lockBuf = lock[id].lock;
}

//==============================================================//
//�������ƣ�Lock_QueryStatus			                              //
//��������ѯ��״̬����                                          //
//���룺id	��ѯ������ID				                                //
//�������																											//
//���أ�0 �Ѵ򿪣�1 ������0xFF ָ��ID����		                    //
//���ܣ���ȡ��ǰ����״̬																	      //
//==============================================================//
static uint8_t Lock_QueryStatus(uint8_t id)
{
	if((id > LOCK_NUM) || (id == 0))
		return 0xFF;
	return lock[id-1].lock;
}

uint8_t Get_LockStatus(void)
{
	return lock[0].lock;
}
