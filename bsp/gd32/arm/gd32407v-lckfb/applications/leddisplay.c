#include "leddisplay.h"

#define UART_NAME				"uart2"/* �����豸���� */
static rt_device_t 			leddisplayserial;/* �����豸��� */
static struct rt_semaphore uart2rx_sem;

static rt_err_t uart2_rx_callback(rt_device_t dev, rt_size_t size);
static void uart2_thread_entry(void *parameter);
rt_err_t LedDisplayInit(void)
{
	rt_err_t res = RT_EOK;
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;/*��ʼ�����ò���*/
	/* step 1: ���Ҵ����豸 */
	leddisplayserial = rt_device_find(UART_NAME);
	if(!leddisplayserial)
	{
		rt_kprintf("find %s failed!\r\n", UART_NAME);
		return RT_ERROR;
	}
	
	/* step 2: �޸Ĵ������ò��� */
	config.baud_rate = BAUD_RATE_9600;
	config.data_bits = DATA_BITS_8;
	config.stop_bits = STOP_BITS_1;
	config.bufsz = 256;//�޸Ļ�������СΪ256�ֽ�
	config.parity = PARITY_NONE;
	res = rt_device_control(leddisplayserial,RT_DEVICE_CTRL_CONFIG,&config);
	
	 /* ��ʼ���ź��� */
    rt_sem_init(&uart2rx_sem, "uart2rx_sem", 0, RT_IPC_FLAG_FIFO);
	
	/* step 3: ���豸 */
	res = rt_device_open(leddisplayserial,RT_DEVICE_FLAG_INT_RX);
	if(res != RT_EOK)
	{
		rt_kprintf("open %s failed!\r\n", UART_NAME);
		return RT_ERROR;
	}
	
	/* step4: ���ûص����� */
	rt_device_set_rx_indicate(leddisplayserial,uart2_rx_callback);
	
	/* step5: ����uart2���߳� */
	rt_thread_t thread = rt_thread_create("uart2_thread",uart2_thread_entry,RT_NULL,1024,10,10);
	if(thread != RT_NULL)
	{
		rt_thread_startup(thread);
	}
	else
	{
		rt_kprintf("create uart2 thread failed!\r\n");
		return RT_ERROR;
	}
	
	return RT_EOK;
}


static rt_err_t uart2_rx_callback(rt_device_t dev, rt_size_t size)
{
	/* ���ڽ��յ����ݺ�����жϣ����ûص�������Ȼ���ͽ����ź��� */
	rt_sem_release(&uart2rx_sem);
	return RT_EOK;
}


static void uart2_thread_entry(void *parameter)
{
	char ch;

    while (1)
    {
			/* �Ӵ��ڶ�ȡһ���ֽڵ����ݣ�û�ж�ȡ����ȴ������ź��� */
			while (rt_device_read(leddisplayserial, -1, &ch, 1) != 1)
			{
            /* �����ȴ������ź������ȵ��ź������ٴζ�ȡ���� */
            rt_sem_take(&uart2rx_sem, RT_WAITING_FOREVER);
			}
        /* ��ȡ��������ͨ�����ڴ�λ��� */
			ch = ch + 1;
			rt_device_write(leddisplayserial, 0, &ch, 1);
    }
}