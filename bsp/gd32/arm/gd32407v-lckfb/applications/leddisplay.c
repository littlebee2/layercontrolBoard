#include "leddisplay.h"

#define UART_NAME				"uart2"/* 串口设备名称 */
static rt_device_t 			leddisplayserial;/* 串口设备句柄 */
static struct rt_semaphore uart2rx_sem;

static rt_err_t uart2_rx_callback(rt_device_t dev, rt_size_t size);
static void uart2_thread_entry(void *parameter);
rt_err_t LedDisplayInit(void)
{
	rt_err_t res = RT_EOK;
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;/*初始化配置参数*/
	/* step 1: 查找串口设备 */
	leddisplayserial = rt_device_find(UART_NAME);
	if(!leddisplayserial)
	{
		rt_kprintf("find %s failed!\r\n", UART_NAME);
		return RT_ERROR;
	}
	
	/* step 2: 修改串口配置参数 */
	config.baud_rate = BAUD_RATE_9600;
	config.data_bits = DATA_BITS_8;
	config.stop_bits = STOP_BITS_1;
	config.bufsz = 256;//修改缓冲区大小为256字节
	config.parity = PARITY_NONE;
	res = rt_device_control(leddisplayserial,RT_DEVICE_CTRL_CONFIG,&config);
	
	 /* 初始化信号量 */
    rt_sem_init(&uart2rx_sem, "uart2rx_sem", 0, RT_IPC_FLAG_FIFO);
	
	/* step 3: 打开设备 */
	res = rt_device_open(leddisplayserial,RT_DEVICE_FLAG_INT_RX);
	if(res != RT_EOK)
	{
		rt_kprintf("open %s failed!\r\n", UART_NAME);
		return RT_ERROR;
	}
	
	/* step4: 设置回调函数 */
	rt_device_set_rx_indicate(leddisplayserial,uart2_rx_callback);
	
	/* step5: 创建uart2的线程 */
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
	/* 串口接收到数据后产生中断，调用回调函数，然后发送接收信号量 */
	rt_sem_release(&uart2rx_sem);
	return RT_EOK;
}


static void uart2_thread_entry(void *parameter)
{
	char ch;

    while (1)
    {
			/* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
			while (rt_device_read(leddisplayserial, -1, &ch, 1) != 1)
			{
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&uart2rx_sem, RT_WAITING_FOREVER);
			}
        /* 读取到的数据通过串口错位输出 */
			ch = ch + 1;
			rt_device_write(leddisplayserial, 0, &ch, 1);
    }
}