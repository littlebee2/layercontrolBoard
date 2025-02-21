#include "fm24cl16b.h"

static void FM24CL16B_IIC_Init(void);
static void FM24CL16B_IIC_Start(void);
static void FM24CL16B_IIC_Stop(void);
static void FM24CL16B_IIC_ACK(void);
static void FM24CL16B_IIC_NOACK(void);
static rt_uint8_t FM24CL16B_IIC_Wait_ACK(void);

/**
 * desc:IIC��ȡ����
 * return:
 * 	IIC��ȡ����ֵ
 */
static rt_uint8_t FM24CL16B_IIC_Read(void);

/**
 * desc:IICд����
 * params:
 * 	data:��Ҫͨ��IIC���͵�����
 */
static void FM24CL16B_IIC_Write(uint8_t data);

void Delay(rt_uint8_t nus)
{
	rt_uint8_t nuss = 2*nus;
	while(nuss--);
}

void Eeprom_Init(void)
{
    FM24CL16B_IIC_Init();
		FM24CL16B_ReadBytes(0, EEPROM_ADDR_ID,(rt_uint8_t *)&sys.slaveID,2);
    if (sys.slaveID==0xffff || sys.slaveID==0)
        sys.slaveID=255;
    sys.type = BOARD_TYPE;
    sys.ver = LAYERBOARD_VERSION;
}

/* ���ó��������� */
static void FM24CL16B_SDA_IN()
{
	rt_pin_mode(FM24CL16B_SDA_PIN,PIN_MODE_INPUT_PULLUP);
}
//���ó��������
static void FM24CL16B_SDA_OUT()
{
	rt_pin_mode(FM24CL16B_SDA_PIN,PIN_MODE_OUTPUT);
}

static void FM24CL16B_IIC_Init()
{
	rt_pin_mode(FM24CL16B_SDA_PIN,PIN_MODE_OUTPUT);
	rt_pin_mode(FM24CL16B_SCL_PIN,PIN_MODE_OUTPUT);
	rt_pin_mode(FM24CL16B_FW_PIN,PIN_MODE_OUTPUT);	
	FM24CL16B_SCL_HIGH;
	FM24CL16B_SDA_LOW;
	rt_pin_write(FM24CL16B_FW_PIN,PIN_LOW);
	FM24CL16B_IIC_Stop();
}

static void FM24CL16B_IIC_Start()
{
	FM24CL16B_SDA_OUT();
	FM24CL16B_SDA_HIGH;
	Delay(1);
	FM24CL16B_SCL_HIGH;
	Delay(5);
	FM24CL16B_SDA_LOW;
	Delay(5);
}

static void FM24CL16B_IIC_Stop()
{
		FM24CL16B_SDA_OUT();
		FM24CL16B_SDA_LOW;
		FM24CL16B_SCL_HIGH;
		Delay(5);
		FM24CL16B_SDA_HIGH;
		Delay(5);
}

static void FM24CL16B_IIC_ACK()
{
	FM24CL16B_SCL_LOW;
	FM24CL16B_SDA_OUT();
	FM24CL16B_SDA_LOW;
	Delay(2);
	FM24CL16B_SCL_HIGH;
	Delay(5);
	FM24CL16B_SCL_LOW;
	Delay(5);
	FM24CL16B_SDA_HIGH;
}

static void FM24CL16B_IIC_NOACK()
{
	FM24CL16B_SCL_LOW;
	FM24CL16B_SDA_OUT();
	FM24CL16B_SDA_HIGH;
	Delay(2);
	FM24CL16B_SCL_HIGH;
	Delay(5);
	FM24CL16B_SCL_LOW;
	Delay(2);
}

static rt_uint8_t FM24CL16B_IIC_Wait_ACK()
{
	FM24CL16B_SDA_HIGH;
	FM24CL16B_SCL_LOW;
	FM24CL16B_SDA_IN();

	Delay(1);
	FM24CL16B_SCL_HIGH;
	Delay(1);
	if(FM24CL16B_SDA_READ)
	{
		FM24CL16B_IIC_Stop();
		return 1;
	}
	FM24CL16B_SCL_LOW;
	return 0;
}

static rt_uint8_t FM24CL16B_IIC_Read()
{
	rt_uint8_t data=0;

	FM24CL16B_SDA_IN();
	FM24CL16B_SCL_LOW;
	Delay(2);

	for(rt_uint8_t i=0;i<8;i++)
	{
		FM24CL16B_SCL_HIGH;
		Delay(2);
		data<<=1;
		if(FM24CL16B_SDA_READ)
		{
			data|=0x01;
		}

		FM24CL16B_SCL_LOW;
		Delay(2);
	}

	return data;
}

static void FM24CL16B_IIC_Write(rt_uint8_t data)
{
	FM24CL16B_SDA_OUT();
	FM24CL16B_SCL_LOW;

	for(rt_uint8_t i=0;i<8;i++)
	{
		if(data & 0x80)
		{
			FM24CL16B_SDA_HIGH;
		}
		else
		{
			FM24CL16B_SDA_LOW;
		}
		Delay(1);
		data<<=1;
		FM24CL16B_SCL_HIGH;
		Delay(1);
		FM24CL16B_SCL_LOW;
		Delay(1);
	}
	//����λ���ߣ��ȴ�Ӧ��
	FM24CL16B_SDA_HIGH;
	Delay(1);
}

/**
 * desc:FM24CL16Bд��1���ֽ�
 * params:
 * 	page:��Ҫд�����ݵ�ҳ��ַ��0~7��һ��8ҳ
 * 	addr:��Ҫд�����ݵĶε�ַ��0~255��һ��256����ַ��
 * 	writeData:��Ҫд����ֽ�����
 * return:
 * 	0:д��ɹ���1��Ӧ��ʧ��
 */
rt_uint8_t FM24CL16B_WriteByte(rt_uint8_t page, rt_uint8_t addr, rt_uint8_t writeData)
{
	FM24CL16B_IIC_Start();
	FM24CL16B_IIC_Write(FM24CL16B_PAGE_WRITE(page));
	if(FM24CL16B_IIC_Wait_ACK()!=0) return 1;
	FM24CL16B_IIC_Write(addr);
	if(FM24CL16B_IIC_Wait_ACK()!=0) return 1;
	FM24CL16B_IIC_Write(writeData);
	if(FM24CL16B_IIC_Wait_ACK()!=0) return 1;
	FM24CL16B_IIC_Stop();
	return 0;
}

/**
 * desc:FM24CL16B��ȡ1���ֽ�
 * params:
 * 	page:��Ҫ��ȡ���ݵ�ҳ��ַ��0~7��һ��8ҳ
 * 	addr:��Ҫ��ȡ���ݵĶε�ַ��0~255��һ��256����ַ��
 * 	writeData:���ض�ȡ�����ֽ�����
 * return:
 * 	0:д��ɹ���1��Ӧ��ʧ��
 */
rt_uint8_t FM24CL16B_ReadByte(rt_uint8_t page, rt_uint8_t addr, rt_uint8_t *readData)
{
	FM24CL16B_IIC_Start();
	FM24CL16B_IIC_Write(FM24CL16B_PAGE_WRITE(page));
	if(FM24CL16B_IIC_Wait_ACK()!=0) return 1;
	FM24CL16B_IIC_Write(addr);
	if(FM24CL16B_IIC_Wait_ACK()!=0) return 1;
	FM24CL16B_IIC_Start();
	FM24CL16B_IIC_Write(FM24CL16B_PAGE_READ(page));
	if(FM24CL16B_IIC_Wait_ACK()!=0) return 1;
	*readData = FM24CL16B_IIC_Read();
	FM24CL16B_IIC_NOACK();
	FM24CL16B_IIC_Stop();
	return 0;
}

/**
 * desc:FM24CL16Bд�����ֽ�
 * params:
 * 	page:��Ҫд�����ݵ�ҳ��ַ��0~7��һ��8ҳ
 * 	addr:��Ҫд�����ݵĶε�ַ��0~255��һ��256����ַ��
 * 	writeBuff:��Ҫд����ֽ�����
 * 	len:��Ҫд������ݳ���
 * return:
 * 	0:д��ɹ���1��Ӧ��ʧ��
 */
rt_uint8_t FM24CL16B_WriteBytes(rt_uint8_t page, rt_uint8_t addr, rt_uint8_t *writeBuff, rt_uint8_t len)
{
	//char msg[16];
	//sprintf(msg, "%02X %02X %02X %02X\n",writeBuff[0], writeBuff[1], writeBuff[2], writeBuff[3]);
	//Serial_User_Print((char*)writeBuff);
	rt_base_t level;
  level = rt_hw_interrupt_disable();
	rt_pin_write(FM24CL16B_FW_PIN,PIN_LOW);
	FM24CL16B_IIC_Start();
	FM24CL16B_IIC_Write(FM24CL16B_PAGE_WRITE(page));
	if(FM24CL16B_IIC_Wait_ACK()!=0) {
		//Serial_User_Print("дpage��Ӧ��");
		rt_hw_interrupt_enable(level);
		return 1;
	}
	FM24CL16B_IIC_Write(addr);
	if(FM24CL16B_IIC_Wait_ACK()!=0)
	{
		//Serial_User_Print("дaddr��Ӧ��");
		rt_hw_interrupt_enable(level);
		return 1;
	}
	for(rt_uint8_t i=0;i<len;i++)
	{
		FM24CL16B_IIC_Write(writeBuff[i]);
		if(FM24CL16B_IIC_Wait_ACK()!=0)
		{
			//Serial_User_Print("д������Ӧ��");
			rt_hw_interrupt_enable(level);
			return 1;
		}
	}
	
	FM24CL16B_IIC_Stop();
	rt_hw_interrupt_enable(level);
	return 0;
}

/**
 * desc:FM24CL16B��ȡ����ֽ�
 * params:
 * 	page:��Ҫ��ȡ���ݵ�ҳ��ַ��0~7��һ��8ҳ
 * 	addr:��Ҫ��ȡ���ݵĶε�ַ��0~255��һ��256����ַ��
 * 	readBuff:���ض�ȡ�����ֽ�����
 * 	len:��Ҫ��ȡ�����ݳ���
 * return:
 * 	0:д��ɹ���1��Ӧ��ʧ��
 */
rt_uint8_t FM24CL16B_ReadBytes(rt_uint8_t page, rt_uint8_t addr, rt_uint8_t *readBuff, rt_uint8_t len)
{
	FM24CL16B_IIC_Start();
	FM24CL16B_IIC_Write(FM24CL16B_PAGE_WRITE(page));
	if(FM24CL16B_IIC_Wait_ACK()!=0)
	{
		//Serial_User_Print("write page no answer");
		return 1;
	}
	FM24CL16B_IIC_Write(addr);
	if(FM24CL16B_IIC_Wait_ACK()!=0)
	{
		//Serial_User_Print("write addr no answer");
		return 1;
	}
	FM24CL16B_IIC_Start();
	FM24CL16B_IIC_Write(FM24CL16B_PAGE_READ(page));
	if(FM24CL16B_IIC_Wait_ACK()!=0)
	{
		//Serial_User_Print("read pageno answer");
		return 1;
	}
	for(rt_uint8_t i=0;i<len;i++)
	{
		readBuff[i] = FM24CL16B_IIC_Read();
		if(i == len-1)
		{
			FM24CL16B_IIC_NOACK();
		}
		else
		{
			FM24CL16B_IIC_ACK();
		}
	}
	//Serial_User_Print(readBuff);
	FM24CL16B_IIC_Stop();
	return 0;
}

