#include "mycan.h"
#include "crc.h"
#include "device.h"
#include "main.h"
#include "event.h"

#define	CAN_NUM     2
static struct rt_semaphore can0rx_sem,can1rx_sem;	// ���ڽ�����Ϣ���ź���
static rt_event_t evt_can_rx;//���������¼�����

static rt_device_t can0_dev,can1_dev;
static Can_Type can[CAN_NUM]; 

static void Can_Handler(void);
static void Can_DataCheck(uint8_t num,CanRx_Type *buf);
static void DataReceiveProcess(uint8_t *dat,uint16_t len,uint32_t id);
static void Event_DataPacketProcess(uint8_t *dat,uint8_t len);
static void Event_BroadcastPacketProcess(uint8_t *dat,uint8_t len);
static void saveCanData(uint8_t canIdx, struct rt_can_msg *rxmsg);
/*==============================================================
****�������ƣ�can_rx_call
****�������������ݻص�����
****���룺��
****�������
****���أ���
****���ܣ��ص�����
==============================================================*/
static rt_err_t can0_rx_call(rt_device_t dev, rt_size_t size)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	/* CAN ���յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
	rt_sem_release(&can0rx_sem);
    
	return RT_EOK;
}

/*==============================================================
****�������ƣ�can_rx_call
****�������������ݻص�����
****���룺��
****�������
****���أ���
****���ܣ��ص�����
==============================================================*/
static rt_err_t can1_rx_call(rt_device_t dev, rt_size_t size)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	/* CAN ���յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
	rt_sem_release(&can1rx_sem);
    
	return RT_EOK;
}

/*==============================================================
****�������ƣ�can0_rx_thread
****������can�����߳�
****���룺��
****�������
****���أ���
****���ܣ�����canԭʼ����
==============================================================*/        
static void can0_rx_thread(void *parameter)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	int i;
	struct rt_can_msg rxmsg = {0};
    
	/* ���ý��ջص����� */
	rt_device_set_rx_indicate(can0_dev, can0_rx_call);
    	
	while (1)
	{
		/* hdr ֵΪ - 1����ʾֱ�Ӵ� uselist �����ȡ���� */
		rxmsg.hdr_index = -1;
		/* �����ȴ������ź��� */
		rt_sem_take(&can0rx_sem, RT_WAITING_FOREVER);
		/* �� CAN ��ȡһ֡���� */
		rt_device_read(can0_dev, 0, &rxmsg, sizeof(rxmsg));
		
		/*��˸�ƹ�+��������*/
		CAN_RX1_LED_INDICATE();//���õƹ���˸20ms
		if(rxmsg.id == DATA_ID_SELF || rxmsg.id == DATA_ID_BROADCAST)//�յ������ID�����ݻ��߹㲥����
		{
			saveCanData(0,&rxmsg);
		}
		
		/* ��ӡ���� ID ������ */
		rt_kprintf("CAN0 recv:ID:%x ", rxmsg.id);
		for (i = 0; i < 8; i++)
		{
				rt_kprintf("%02x ", rxmsg.data[i]);
		}
		rt_kprintf("\r\n");
	}
}

/*==============================================================
****�������ƣ�can1_rx_thread
****������can�����߳�
****���룺��
****�������
****���أ���
****���ܣ�����canԭʼ����
==============================================================*/        
static void can1_rx_thread(void *parameter)
{
	rt_kprintf("%s \r\n", __FUNCTION__);
	int i;
	struct rt_can_msg rxmsg = {0};
    
	/* ���ý��ջص����� */
	rt_device_set_rx_indicate(can1_dev, can1_rx_call);
    	
	while (1)
	{
		/* hdr ֵΪ - 1����ʾֱ�Ӵ� uselist �����ȡ���� */
		rxmsg.hdr_index = -1;
		/* �����ȴ������ź��� */
		rt_sem_take(&can1rx_sem, RT_WAITING_FOREVER);
		/* �� CAN ��ȡһ֡���� */
		rt_device_read(can1_dev, 0, &rxmsg, sizeof(rxmsg));
		
		/*��˸�ƹ�+��������*/
		CAN_RX1_LED_INDICATE();//���õƹ���˸20ms
		if(rxmsg.id == DATA_ID_SELF || rxmsg.id == DATA_ID_BROADCAST)//�յ������ID�����ݻ��߹㲥����
		{
			saveCanData(1,&rxmsg);
			// ���յ����ݺ����¼�
			//rt_event_send(evt_can_rx, EVENT_CAN_RX_NEW_DATA);
		}
		/* ��ӡ���� ID ������ */
		rt_kprintf("CAN1 recv:ID:%x ", rxmsg.id);
		for (i = 0; i < 8; i++)
		{
				rt_kprintf("%02x ", rxmsg.data[i]);
		}
		rt_kprintf("\r\n");
	}
}

/*==============================================================
****�������ƣ�can_handler_thread
****������can���ݽ����߳�
****���룺��
****�������
****���أ���
****���ܣ���������
==============================================================*/
static void can0_handler_thread(void *parameter)
{
	rt_tick_t timeout_ticks = rt_tick_from_millisecond(500); // 1�볬ʱ
	rt_uint32_t recv_event;
	while(1)
	{
		Can_Handler();
		rt_thread_mdelay(10);
	}
}

/*==============================================================
****�������ƣ�can_handler_thread
****������can���ݽ����߳�
****���룺��
****�������
****���أ���
****���ܣ���������
==============================================================*/
static void can1_handler_thread(void *parameter)
{
	rt_tick_t timeout_ticks = rt_tick_from_millisecond(500); // 1�볬ʱ
	rt_uint32_t recv_event;
	while(1)
	{
		Can_Handler();
		rt_thread_mdelay(10);
	}
}
/*==============================================================
****�������ƣ�CanInit
****������can��ʼ������
****���룺��
****�������
****���أ���
****���ܣ�can��ʼ������
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
		// �����豸
		can0_dev = rt_device_find(can_name);
		if (!can0_dev)
		{
			rt_kprintf("find %s failed!\r\n", can_name);
			return RT_ERROR;
		}
		// init �����ź���
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
		
		struct rt_can_filter_config cfg = {1, 1, &filter}; /* һ���� 1 �����˱� */
		
		res = rt_device_control(can0_dev, RT_CAN_CMD_SET_FILTER, &cfg);
		RT_ASSERT(res == RT_EOK);
		// �жϷ�ʽ���豸
		res = rt_device_open(can0_dev, RT_DEVICE_FLAG_INT_RX);
		RT_ASSERT(res == RT_EOK);
	
		// ���������߳�
		thread = rt_thread_create("can_rx", can0_rx_thread, RT_NULL, 512, 10, 10);
		if (thread != RT_NULL)
		{	
			rt_thread_startup(thread);
		}
		else
		{
			rt_kprintf("create can_rx thread failed!\r\n");
		}
		//���������߳�
		thread = rt_thread_create("can0_handler", can0_handler_thread, RT_NULL, 1024, 10, 10);
		if (thread != RT_NULL)
		{
			rt_thread_startup(thread);
		}
		else
		{
			rt_kprintf("create can0_handler failed!\r\n");
		}
		//����can�����¼�����
		evt_can_rx = rt_event_create("evt_can_rx", RT_IPC_FLAG_FIFO);
	}
	else if(Port == CAN_PORT1)
	{
		rt_strncpy(can_name, "can1", RT_NAME_MAX);
		// �����豸can1
		can1_dev = rt_device_find(can_name);
		if (!can1_dev)
		{
			rt_kprintf("find %s failed!\r\n", can_name);
			return RT_ERROR;
		}
		// init �����ź���
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
		
		struct rt_can_filter_config cfg = {1, 1, &filter}; /* һ���� 1 �����˱� */
		
		res = rt_device_control(can1_dev, RT_CAN_CMD_SET_FILTER, &cfg);
		RT_ASSERT(res == RT_EOK);
		// �жϷ�ʽ���豸
		res = rt_device_open(can1_dev, RT_DEVICE_FLAG_INT_RX);
		RT_ASSERT(res == RT_EOK);
	
		// ���������߳�
		thread = rt_thread_create("can1_rx_thread", can1_rx_thread, RT_NULL, 512, 10, 10);
		if (thread != RT_NULL)
		{	
			rt_thread_startup(thread);
		}
		else
		{
			rt_kprintf("create can1_rx_thread thread failed!\r\n");
		}
		//���������߳�
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
//�������ƣ�Can_DataCheck		                                    //
//������CAN���ݰ���⺯��																				//
//���룺num		CAN��
//			*buf	���������ݽṹ��		                              //
//�������																								    	//
//���ܣ��ӽṹ���ؼ���Ƿ�����Ч���ݰ�												//
//			�������ͷ�����ݳ����Լ�CRC�����ͨ������ô��������� //
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
						if(crc == ((buf->dat[buf->dat[buf->index+1]-2]<<8) | (buf->dat[buf->dat[buf->index+1]-1])))	//У��CRC
						{
							Event_DataReceive(&buf->dat[buf->index+2],buf->dat[buf->index+1]-4,buf->addr);
						}
						else
							rt_kprintf("CAN %d CRC Error!CRC Cal = 0x%04X,Receive = 0x%04X\r\n",\
								num,crc,(buf->dat[buf->dat[buf->index+1]-2]<<8) | (buf->dat[buf->dat[buf->index+1]-1]));
						buf->index += buf->dat[buf->index+1];
						if(buf->index == buf->len)      //��ǰ��������ݵ������ݳ��ȣ�˵�����ݴ������
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
//�������ƣ�saveCanData                   //
//������CAN���ձ���ԭʼ����																					//
//���룺*hcan	���յ����ݵ�CAN			                              //
//�������																						          //
//���ܣ���CAN���յ���������ϴ���BUF											      //
//==============================================================//
static void saveCanData(uint8_t canIdx, struct rt_can_msg *rxmsg)
{
    uint8_t i;
    for(i=0;i<CAN_DATA_BUF_NUM;i++)
    {
			#if	CAN_USE_ID == CAN_STANDARD_ID
			if(rxmsg.id == can.rxBuf[i].addr)              //�ж�֮ǰ�Ƿ��н��յ���ǰ��ַ���͵�����
			#elif	CAN_USE_ID == CAN_EXTENSION_ID
			if(rxmsg->id == can[canIdx].rx[i].addr)         //�ж�֮ǰ�Ƿ��н��յ���ǰ��ַ���͵�����
			#endif
			{
				if((can[canIdx].rx[i].len + rxmsg->len) <= CAN_RX_BUF_SIZE)
				{
						//��֮ǰ�н��յ����ݣ��򽫵�ǰ���ݴ�ŵ�֮ǰ������֮��
					memcpy(&can[canIdx].rx[i].dat[can[canIdx].rx[i].len],&rxmsg->data[0],rxmsg->len); 
					can[canIdx].rx[i].len += rxmsg->len;
					can[canIdx].rx[i].timeOut = 20;
				}
				break;
			}
    }

    if(i >= CAN_DATA_BUF_NUM)//���������ʾ֮ǰδ�յ��˵�ַ������
    {
			for(i=0;i<CAN_DATA_BUF_NUM;i++)//���ҿ�BUF�������ݴ���
			{
				if(can[canIdx].rx[i].addr == DATA_ID_EMPTY)
				{
					#if	CAN_USE_ID == CAN_STANDARD_ID
					can.rxBuf[i].addr = canRxHeader.StdId;                                              //���п�BUF����¼��ǰ��ŵĵ�ַ�Լ�����
					#elif	CAN_USE_ID == CAN_EXTENSION_ID
					can[canIdx].rx[i].addr = rxmsg->id;                                         //���п�BUF����¼��ǰ��ŵĵ�ַ�Լ�����
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
//�������ƣ�Can_SendPacket	                                    //
//������CAN����һ������																					//
//���룺num		CAN��																							//
//			*dat	�������������ڵ�ַ                                //
//			len		���������ݳ���																		//
			
//���������״̬	0 �ɹ���1 ���ͳ��Ȳ��ڷ�Χ��2 CAN���ڷ���     //
//���ܣ���һ������ͨ��CAN�ֶη���													      //
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
	msg.id = addr; /* ��չID */
	msg.ide = RT_CAN_EXTID;/* ��չ֡ID */
	#endif
	msg.rtr = RT_CAN_DTR;       /* ����֡ */
	
	if(can[num].tx.len > 8)
	{
		uint8_t count = can[num].tx.len / 8;//����֡��
		uint8_t lastlen = can[num].tx.len % 8;//���һ֡����
		uint8_t index;
		for(index=0; index < count; index++)
		{
			msg.len = 8;
			memcpy(&msg.data,&can[num].tx.dat[8*index],8);
			/* ����һ֡ CAN ���� */
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
			/* ����һ֡ CAN ���� */
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
		/* ����һ֡ CAN ���� */
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

       