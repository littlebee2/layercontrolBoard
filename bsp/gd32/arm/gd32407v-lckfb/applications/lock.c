#include "lock.h"
#include "eeprom.h"

static Lock_Type lock[LOCK_NUM];//操作结构体
static LockConfig_Type lockConfig;//配置结构体
static uint8_t lockConfigUpdate;//配置锁控标志位

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
****函数名称：lock_thread
****描述：can数据解析线程
****输入：无
****输出：无
****返回：无
****功能：解析数据
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
//函数名称：Lock_Init					                                  //
//描述：锁控制程序初始化函数        				               	    //
//输入：无                                                      //
//输出：无                                                      //
//返回：无                                                      //
//功能：初始化与锁控制相关资源														      //
//==============================================================//
void Lock_Init(void)
{
	rt_thread_t thread;
	fm24clxx_read_page(eeprom_dev,FM24CLXX_ADDR_7BIT,EEPROM_ADDR_LOCK_CONFIG,(uint8_t*)&lockConfig,sizeof(lockConfig));//读取配置参数
	if(LOCK_CONFIG_HEAD == lockConfig.head)//已经配置过第一次
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
	
	//开线程
	// 创建接收线程
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
//函数名称：Lock_Handler			                                 	//
//描述：锁控制主程序         	                                	//
//输入：无                                                      //
//输出：无                                                      //
//返回：无                                                      //
//功能：处理锁相关事件                                    			//
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
//函数名称：Lock_CMDProcess				                              //
//描述：锁指令处理函数                                          //
//输入：*datIn	接收数据存放地址                                //
//			lenIn		接收数据长度                                    //
//输出：*datOut		待返回数据长度存放地址                        //
//			*lenOut		待返回数据长度                                //
//返回：执行结果 0 成功，非0 错误代码                           //
//功能：处理上位机锁控制指令															      //
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
//函数名称：Lock_Open							                              //
//描述：开锁程序			                                          //
//输入：id	开启的锁的ID				                                //
//输出：无                                                      //
//返回：0 成功	1 指定ID超出锁数		                           	//
//功能：开启指定ID的锁，当ID=0时，开启所有锁							      //
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
				lock[id].delay = 50;		//设置开锁限制时间，如锁长时间未打开，为防止损坏在此时间后强制断电
				if(lock[id].openLevel)
					rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
				else
					rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
			}
		}
		else if(lock[id].type == LOCK_TYPE_DOOR)
		{
			lock[id].delay = 2000;		//设定保险柜开锁限制时间，时间到后还未开锁则自动关闭
			if(lock[id].openLevel)
				rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
			else
				rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
		}
	}
	return 0;
}

//==============================================================//
//函数名称：Lock_OpenForced				                              //
//描述：强制开锁程序	                                          //
//输入：id	开启的锁的ID				                                //
//输出：无                                                      //
//返回：0 成功	1 指定ID超出锁数		                           	//
//功能：开启指定ID的锁，当ID=0时，开启所有锁							      //
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
			lock[id].delay = 50;		//设置开锁限制时间，如锁长时间未打开，为防止损坏在此时间后强制断电
			lock[id].forcedFlag = 1;
			if(lock[id].openLevel)
				rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
			else
				rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
		}
		else if(lock[id].type == LOCK_TYPE_DOOR)
		{
			lock[id].delay = 2000;		//设定保险柜开锁限制时间，时间到后还未开锁则自动关闭
			if(lock[id].openLevel)
					rt_pin_write(LOCK_PWR_Pin[id],PIN_HIGH);
				else
					rt_pin_write(LOCK_PWR_Pin[id],PIN_LOW);
		}
	}
	return 0;
}

//==============================================================//
//函数名称：Lock_Close						                              //
//描述：关锁程序			                                          //
//输入：id	关闭的锁的ID				                                //
//输出：无																											//
//返回：0 成功	1 指定ID超出锁数		                           	//
//功能：关闭指定ID的锁，当ID=0时，关闭所有锁							      //
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
//函数名称：Lock_Scan							                              //
//描述：扫描锁状态函数                                          //
//输入：id	扫描的锁的ID				                                //
//输出：无																                      //
//返回：无                                                      //
//功能：扫描锁状态，并更新																      //
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
		if(temp != lock[id].lockBuf)  //防抖
		{
			lock[id].lockBuf = temp;
			if(lock[id].lockBuf)
				lock[id].statusDelay = 50;				//检测关锁延时500ms
			else
				lock[id].statusDelay = 10;				//检测关锁延时100ms
		}
		else if(!lock[id].statusDelay)
		{
			lock[id].lock = lock[id].lockBuf;
			if(lock[id].lock)
			{
				if(lock[id].type == LOCK_TYPE_DOOR)
				{
					//如为保险柜锁上，则关闭开锁
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
					lock[id].delay = 6000;		//保险柜开锁后，重新设定自动关锁
					temp = rt_pin_read(LOCK_DET_Pin[id])? 1:0;
					if (temp  == lock[id].lockLevel)
						Lock_StatusReport(id+1,LOCK_STATUS_LOCKED);
					else
						Lock_StatusReport(id+1,LOCK_STATUS_UNLOCKED); 
				}
				else
				{
					Lock_StatusReport(id+1,LOCK_STATUS_UNLOCKED);
					if(!lock[id].forcedFlag)		//不为强制开锁，则检测打开后关锁
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
//函数名称：Lock_QueryStatus			                              //
//描述：查询锁状态函数                                          //
//输入：id	查询的锁的ID				                                //
//输出：无																											//
//返回：0 已打开，1 已锁，0xFF 指定ID错误		                    //
//功能：获取当前锁的状态																	      //
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
