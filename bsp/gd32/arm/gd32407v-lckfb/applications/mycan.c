#include "mycan.h"
#include "crc.h"
#include "device.h"
#include "main.h"
#include "event.h"

#define	CAN_NUM     2
static struct rt_semaphore can0rx_sem,can1rx_sem;	// 用于接收消息的信号量
static rt_event_t evt_can_rx;//解析数据事件对象

static rt_device_t can0_dev,can1_dev;
static Can_Type can[CAN_NUM]; 

static void Can_Handler(void);
static void Can_DataCheck(uint8_t num,CanRx_Type *buf);
static void DataReceiveProcess(uint8_t *dat,uint16_t len,uint32_t id);
static void Event_DataPacketProcess(uint8_t *dat,uint8_t len);
static void Event_BroadcastPacketProcess(uint8_t *dat,uint8_t len);
static void saveCanData(uint8_t canIdx, struct rt_can_msg *rxmsg);
/*==============================================================
****函数名称：can_rx_call
****描述：接收数据回调函数
****输入：无
****输出：无
****返回：无
****功能：回调函数
==============================================================*/
static rt_err_t can0_rx_call(rt_device_t dev, rt_size_t size)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	/* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
	rt_sem_release(&can0rx_sem);
    
	return RT_EOK;
}

/*==============================================================
****函数名称：can_rx_call
****描述：接收数据回调函数
****输入：无
****输出：无
****返回：无
****功能：回调函数
==============================================================*/
static rt_err_t can1_rx_call(rt_device_t dev, rt_size_t size)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	/* CAN 接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
	rt_sem_release(&can1rx_sem);
    
	return RT_EOK;
}

/*==============================================================
****函数名称：can0_rx_thread
****描述：can接收线程
****输入：无
****输出：无
****返回：无
****功能：接收can原始数据
==============================================================*/        
static void can0_rx_thread(void *parameter)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	int i;
	struct rt_can_msg rxmsg = {0};
    
	/* 设置接收回调函数 */
	rt_device_set_rx_indicate(can0_dev, can0_rx_call);
    	
	while (1)
	{
		/* hdr 值为 - 1，表示直接从 uselist 链表读取数据 */
		rxmsg.hdr_index = -1;
		/* 阻塞等待接收信号量 */
		rt_sem_take(&can0rx_sem, RT_WAITING_FOREVER);
		/* 从 CAN 读取一帧数据 */
		rt_device_read(can0_dev, 0, &rxmsg, sizeof(rxmsg));
		
		/*闪烁灯光+保存数据*/
		CAN_RX1_LED_INDICATE();//设置灯光闪烁20ms
		if(rxmsg.id == DATA_ID_SELF || rxmsg.id == DATA_ID_BROADCAST)//收到本层的ID的数据或者广播数据
		{
			saveCanData(0,&rxmsg);
		}
		
		/* 打印数据 ID 及内容 */
		rt_kprintf("CAN0 recv:ID:%x ", rxmsg.id);
		for (i = 0; i < 8; i++)
		{
				rt_kprintf("%02x ", rxmsg.data[i]);
		}
		rt_kprintf("\r\n");
	}
}

/*==============================================================
****函数名称：can1_rx_thread
****描述：can接收线程
****输入：无
****输出：无
****返回：无
****功能：接收can原始数据
==============================================================*/        
static void can1_rx_thread(void *parameter)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	int i;
	struct rt_can_msg rxmsg = {0};
    
	/* 设置接收回调函数 */
	rt_device_set_rx_indicate(can1_dev, can1_rx_call);
    	
	while (1)
	{
		/* hdr 值为 - 1，表示直接从 uselist 链表读取数据 */
		rxmsg.hdr_index = -1;
		/* 阻塞等待接收信号量 */
		rt_sem_take(&can1rx_sem, RT_WAITING_FOREVER);
		/* 从 CAN 读取一帧数据 */
		rt_device_read(can1_dev, 0, &rxmsg, sizeof(rxmsg));
		
		/*闪烁灯光+保存数据*/
		CAN_RX1_LED_INDICATE();//设置灯光闪烁20ms
		if(rxmsg.id == DATA_ID_SELF || rxmsg.id == DATA_ID_BROADCAST)//收到本层的ID的数据或者广播数据
		{
			saveCanData(1,&rxmsg);
			// 接收到数据后发送事件
			//rt_event_send(evt_can_rx, EVENT_CAN_RX_NEW_DATA);
		}
		/* 打印数据 ID 及内容 */
		rt_kprintf("CAN1 recv:ID:%x ", rxmsg.id);
		for (i = 0; i < 8; i++)
		{
				rt_kprintf("%02x ", rxmsg.data[i]);
		}
		rt_kprintf("\r\n");
	}
}

/*==============================================================
****函数名称：can_handler_thread
****描述：can数据解析线程
****输入：无
****输出：无
****返回：无
****功能：解析数据
==============================================================*/
static void can0_handler_thread(void *parameter)
{
	rt_tick_t timeout_ticks = rt_tick_from_millisecond(500); // 1秒超时
	rt_uint32_t recv_event;
	while(1)
	{
		Can_Handler();
		rt_thread_mdelay(10);
	}
}

/*==============================================================
****函数名称：can_handler_thread
****描述：can数据解析线程
****输入：无
****输出：无
****返回：无
****功能：解析数据
==============================================================*/
static void can1_handler_thread(void *parameter)
{
	rt_tick_t timeout_ticks = rt_tick_from_millisecond(500); // 1秒超时
	rt_uint32_t recv_event;
	while(1)
	{
		Can_Handler();
		rt_thread_mdelay(10);
	}
}
/*==============================================================
****函数名称：CanInit
****描述：can初始化函数
****输入：无
****输出：无
****返回：无
****功能：can初始化函数
==============================================================*/ 
rt_err_t CanInit(uint8_t Port)
{
	struct rt_can_msg msg = {0};
	rt_err_t res;
	rt_size_t  size;
	rt_thread_t thread;
	char can_name[RT_NAME_MAX];
	
	if(Port == CAN_PORT0)
	{
		rt_strncpy(can_name, "can0", RT_NAME_MAX);
		// 查找设备
		can0_dev = rt_device_find(can_name);
		if (!can0_dev)
		{
			rt_kprintf("find %s failed!\r\n", can_name);
			return RT_ERROR;
		}
		// init 接收信号量
		res = rt_sem_init(&can0rx_sem, "can0rx_sem", 0, RT_IPC_FLAG_FIFO);
		RT_ASSERT(res == RT_EOK);
		// set 250Kbps 
		res = rt_device_control(can0_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN250kBaud);
		RT_ASSERT(res == RT_EOK);
		//set filter
		struct rt_can_filter_item filter;
		filter.id = sys.slaveID;	// ID
		filter.ide = 1;
		filter.rtr = 0;
		filter.mode = 1;
		filter.mask = 0x00000000;// Mask
		filter.hdr = -1;
		filter.rxfifo = 0;  // CAN_FIFO0
		
		struct rt_can_filter_config cfg = {1, 1, &filter}; /* 一共有 1 个过滤表 */
		
		res = rt_device_control(can0_dev, RT_CAN_CMD_SET_FILTER, &cfg);
		RT_ASSERT(res == RT_EOK);
		// 中断方式打开设备
		res = rt_device_open(can0_dev, RT_DEVICE_FLAG_INT_RX);
		RT_ASSERT(res == RT_EOK);
	
		// 创建接收线程
		thread = rt_thread_create("can_rx", can0_rx_thread, RT_NULL, 512, 10, 10);
		if (thread != RT_NULL)
		{	
			rt_thread_startup(thread);
		}
		else
		{
			rt_kprintf("create can_rx thread failed!\r\n");
		}
		//创建解析线程
		thread = rt_thread_create("can0_handler", can0_handler_thread, RT_NULL, 1024, 10, 10);
		if (thread != RT_NULL)
		{
			rt_thread_startup(thread);
		}
		else
		{
			rt_kprintf("create can0_handler failed!\r\n");
		}
		//创建can解析事件对象
		evt_can_rx = rt_event_create("evt_can_rx", RT_IPC_FLAG_FIFO);
	}
	else if(Port == CAN_PORT1)
	{
		rt_strncpy(can_name, "can1", RT_NAME_MAX);
		// 查找设备can1
		can1_dev = rt_device_find(can_name);
		if (!can1_dev)
		{
			rt_kprintf("find %s failed!\r\n", can_name);
			return RT_ERROR;
		}
		// init 接收信号量
		res = rt_sem_init(&can1rx_sem, "can1rx_sem", 0, RT_IPC_FLAG_FIFO);
		RT_ASSERT(res == RT_EOK);
		// set 250Kbps 
		res = rt_device_control(can1_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN250kBaud);
		RT_ASSERT(res == RT_EOK);
		//set filter
		struct rt_can_filter_item filter;
		filter.id = sys.slaveID;	// ID
		filter.ide = 1;
		filter.rtr = 0;
		filter.mode = 1;
		filter.mask = 0x00000000;// Mask
		filter.hdr = -1;
		filter.rxfifo = 1;  // CAN_FIFO0
		
		struct rt_can_filter_config cfg = {1, 1, &filter}; /* 一共有 1 个过滤表 */
		
		res = rt_device_control(can1_dev, RT_CAN_CMD_SET_FILTER, &cfg);
		RT_ASSERT(res == RT_EOK);
		// 中断方式打开设备
		res = rt_device_open(can1_dev, RT_DEVICE_FLAG_INT_RX);
		RT_ASSERT(res == RT_EOK);
	
		// 创建接收线程
		thread = rt_thread_create("can1_rx_thread", can1_rx_thread, RT_NULL, 512, 10, 10);
		if (thread != RT_NULL)
		{	
			rt_thread_startup(thread);
		}
		else
		{
			rt_kprintf("create can1_rx_thread thread failed!\r\n");
		}
		//创建解析线程
		thread = rt_thread_create("can1_handler", can1_handler_thread, RT_NULL, 1024, 10, 10);
		if (thread != RT_NULL)
		{
			rt_thread_startup(thread);
		}
		else
		{
			rt_kprintf("create can1_handler failed!\r\n");
		}
	}
	return res;
}

static void Can_Handler(void)
{
	uint8_t i;
	for(i=0;i<CAN_DATA_BUF_NUM;i++)
	{
		Can_DataCheck(0,&can[0].rx[i]);
	}
}

//==============================================================//
//函数名称：Can_DataCheck		                                    //
//描述：CAN数据包检测函数																				//
//输入：num		CAN号
//			*buf	待发送数据结构体		                              //
//输出：无																								    	//
//功能：从结构体重检测是否含有有效数据包												//
//			检测数据头，数据长度以及CRC，检测通过后调用处理函数处理 //
//==============================================================//
static void Can_DataCheck(uint8_t num,CanRx_Type *buf)
{
	if(buf->addr != DATA_ID_EMPTY)
	{
		if(buf->timeOut)
		{
			buf->timeOut--;
			if(buf->dat[buf->index] == CAN_DATA_PACKET_RX_HEAD || buf->dat[buf->index] == CAN_DATA_PACKET_TX_HEAD)
			{
				if((buf->len - buf->index) >= CAN_DATA_PACKET_MIN_SIZE)
				 {
					if((buf->len - buf->index) >= buf->dat[buf->index+1])
					{
						uint16_t crc = CRC16_Calculate(&buf->dat[buf->index],buf->dat[buf->index+1]-2);
						if(crc == ((buf->dat[buf->dat[buf->index+1]-2]<<8) | (buf->dat[buf->dat[buf->index+1]-1])))	//校验CRC
						{
							Event_DataReceive(&buf->dat[buf->index+2],buf->dat[buf->index+1]-4,buf->addr);
						}
						else
							rt_kprintf("CAN %d CRC Error!CRC Cal = 0x%04X,Receive = 0x%04X\r\n",\
								num,crc,(buf->dat[buf->dat[buf->index+1]-2]<<8) | (buf->dat[buf->dat[buf->index+1]-1]));
						buf->index += buf->dat[buf->index+1];
						if(buf->index == buf->len)      //当前处理的数据等于数据长度，说明数据处理完毕
						{
							buf->addr = DATA_ID_EMPTY;
							buf->len = 0;
							buf->index = 0;
						}
					}
					else
						rt_kprintf("CAN %d Data Receive Time Out,But not Receive full\r\n");
				}
				else
					rt_kprintf("CAN %d Data Receive Len Less than MiniLen\r\n");
			}
			else
			{
				rt_kprintf("Data Head Error!\r\n");
				buf->addr = DATA_ID_EMPTY;
				buf->len = 0;
				buf->index = 0;
			}
		}
		else
		{
			rt_kprintf("%d Data Receive Time Out!\r\n",num);
			buf->addr = DATA_ID_EMPTY;
			buf->len = 0;
			buf->index = 0;
		}
	}
}


//==============================================================//
//函数名称：saveCanData                   //
//描述：CAN接收保存原始数据																					//
//输入：*hcan	接收到数据的CAN			                              //
//输出：无																						          //
//功能：将CAN接收到的数据组合存入BUF											      //
//==============================================================//
static void saveCanData(uint8_t canIdx, struct rt_can_msg *rxmsg)
{
    uint8_t i;
    for(i=0;i<CAN_DATA_BUF_NUM;i++)
    {
			#if	CAN_USE_ID == CAN_STANDARD_ID
			if(rxmsg.id == can.rxBuf[i].addr)              //判断之前是否有接收到当前地址发送的数据
			#elif	CAN_USE_ID == CAN_EXTENSION_ID
			if(rxmsg->id == can[canIdx].rx[i].addr)         //判断之前是否有接收到当前地址发送的数据
			#endif
			{
				if((can[canIdx].rx[i].len + rxmsg->len) <= CAN_RX_BUF_SIZE)
				{
						//如之前有接收到数据，则将当前数据存放到之前的数据之后
					memcpy(&can[canIdx].rx[i].dat[can[canIdx].rx[i].len],&rxmsg->data[0],rxmsg->len); 
					can[canIdx].rx[i].len += rxmsg->len;
					can[canIdx].rx[i].timeOut = 20;
				}
				break;
			}
    }

    if(i >= CAN_DATA_BUF_NUM)//如成立，表示之前未收到此地址的数据
    {
			for(i=0;i<CAN_DATA_BUF_NUM;i++)//查找空BUF，将数据存入
			{
				if(can[canIdx].rx[i].addr == DATA_ID_EMPTY)
				{
					#if	CAN_USE_ID == CAN_STANDARD_ID
					can.rxBuf[i].addr = canRxHeader.StdId;                                              //如有空BUF，记录当前存放的地址以及数据
					#elif	CAN_USE_ID == CAN_EXTENSION_ID
					can[canIdx].rx[i].addr = rxmsg->id;                                         //如有空BUF，记录当前存放的地址以及数据
					#endif
					memcpy(&can[canIdx].rx[i].dat[0],&rxmsg->data[0],rxmsg->len);
					can[canIdx].rx[i].len = rxmsg->len;
					can[canIdx].rx[i].timeOut = 20;
					break;
				}
			}
    }
}

//==============================================================//
//函数名称：Can_SendPacket	                                    //
//描述：CAN发送一包数据																					//
//输入：num		CAN号																							//
//			*dat	待发送数据所在地址                                //
//			len		待发送数据长度																		//
			
//输出：发送状态	0 成功，1 发送长度不在范围，2 CAN正在发送     //
//功能：将一包数据通过CAN分段发送													      //
//==============================================================//
uint8_t Can_SendPacket(uint8_t num,uint32_t addr,uint8_t *dat,uint16_t len, uint8_t headflag)
{
	struct rt_can_msg msg = {0};
	rt_size_t	size = 0;
	uint8_t err;
	u16 crc;
	if(headflag == 1)
		can[num].tx.dat[0] = CAN_DATA_PACKET_TX_HEAD;
	else
		can[num].tx.dat[0] = CAN_DATA_PACKET_RX_HEAD;
	can[num].tx.dat[1] = len + 4;
	memcpy(&can[num].tx.dat[2],dat,len);
	crc = CRC16_Calculate(&can[num].tx.dat[0],len+2);
	can[num].tx.dat[len+2] = (u8)(crc>>8);
	can[num].tx.dat[len+3] = (u8)crc;
	can[num].tx.len= len+4;
	rt_kprintf("%d Send Data:",num);
	#if	CAN_USE_ID == CAN_STANDARD_ID
	canTxHeader.StdId = addr;
	canTxHeader.IDE = CAN_ID_STD;
	#elif	CAN_USE_ID == CAN_EXTENSION_ID
	msg.id = addr; /* 扩展ID */
	msg.ide = RT_CAN_EXTID;/* 扩展帧ID */
	#endif
	msg.rtr = RT_CAN_DTR;       /* 数据帧 */
	
	if(can[num].tx.len > 8)
	{
		uint8_t count = can[num].tx.len / 8;//完整帧数
		uint8_t lastlen = can[num].tx.len % 8;//最后一帧长度
		uint8_t index;
		for(index=0; index < count; index++)
		{
			msg.len = 8;
			memcpy(&msg.data,&can[num].tx.dat[8*index],8);
			/* 发送一帧 CAN 数据 */
			if(num == 0)
				size = rt_device_write(can0_dev, 0, &msg, sizeof(msg));
			else
				size = rt_device_write(can1_dev, 0, &msg, sizeof(msg));
			if (size == 0)
			{
				rt_kprintf("can %d dev write data failed!\r\n",num);
				return 1;
			}
			else
			{
				CAN_TX1_LED_INDICATE();
				rt_kprintf("can %d dev write data success!\r\n",num);
			}
		}
		if(lastlen > 0)
		{
			msg.len = lastlen;
			memcpy(&msg.data,&can[num].tx.dat[8*count],lastlen);
			/* 发送一帧 CAN 数据 */
			if(num == 0)
				size = rt_device_write(can0_dev, 0, &msg, sizeof(msg));
			else
				size = rt_device_write(can1_dev, 0, &msg, sizeof(msg));
			if (size == 0)
			{
				rt_kprintf("can %d dev write data failed!\r\n",num);
				return 1;
			}
			else
			{
				CAN_TX1_LED_INDICATE();
				rt_kprintf("can %d dev write data success!\r\n",num);
			}
		}
	}
	else
	{
		msg.len = can[num].tx.len;
		memcpy(&msg.data,&can[num].tx.dat,can[num].tx.len);
		/* 发送一帧 CAN 数据 */
		if(num == 0)
			size = rt_device_write(can0_dev, 0, &msg, sizeof(msg));
		else
			size = rt_device_write(can1_dev, 0, &msg, sizeof(msg));
		if (size == 0)
		{
			rt_kprintf("can %d dev write data failed!\r\n",num);
			return 1;
		}
		else
		{
			CAN_TX1_LED_INDICATE();
			rt_kprintf("can %d dev write data success!\r\n",num);
		}
	}
	return err;
}

       