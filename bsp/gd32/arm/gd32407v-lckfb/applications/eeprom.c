#include "eeprom.h"

struct rt_i2c_bus_device *eeprom_dev = RT_NULL;


rt_err_t fm24clxx_write_page(   struct rt_i2c_bus_device *dev,
                                uint8_t page_addr_7bit,
                                uint8_t wirteAddr,          //fm24cl16内部存储地址
                                uint8_t *pBuffer, 
                                uint16_t NumByteToWrite)
{
    struct rt_i2c_msg msgs[2];
    msgs[0].addr = page_addr_7bit ;
    msgs[0].flags = RT_I2C_WR ;
    msgs[0].buf = &wirteAddr;
    msgs[0].len = 1; 
	
    msgs[1].addr = page_addr_7bit;
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;
    msgs[1].buf = pBuffer;
    msgs[1].len = NumByteToWrite;
    if(rt_i2c_transfer(dev, msgs, 2) != 2) 
    {
        rt_kprintf("i2c write data failed!");
        return RT_ERROR;
    }            
    return RT_EOK;    
}
rt_err_t fm24clxx_read_page(struct rt_i2c_bus_device *dev,
                            uint8_t page_addr_7bit, 
                            uint8_t readAddr,  //fm24cl16内部存储地址
                            uint8_t *pBuffer,  
                            uint16_t NumByteToRead)
{
    struct rt_i2c_msg msgs[2];
    msgs[0].addr = page_addr_7bit;
    msgs[0].flags = RT_I2C_WR ;
    msgs[0].buf  = &readAddr;
    msgs[0].len = 1;
    msgs[1].addr  = page_addr_7bit;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = pBuffer;
    msgs[1].len   = NumByteToRead;
    if(rt_i2c_transfer(dev, msgs, 2) != 2) 
    {
        rt_kprintf("i2c read data failed!");
        return RT_ERROR;
    }            
    return RT_EOK;
}

 
rt_err_t fm24cl16_init(void)
{
	/*使能FM24CL16B*/
		rt_base_t FM_WP = GET_PIN(C,8);
		rt_pin_mode(FM_WP,PIN_MODE_OUTPUT);
		rt_pin_write(FM_WP,PIN_LOW);
	/*查找设备*/
    struct rt_i2c_msg msgs[2];
    struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C总线设备句柄 */
    eeprom_dev = rt_i2c_bus_device_find("i2c2");
    if(eeprom_dev == RT_NULL)
    {
        rt_kprintf("cannot find i2c1 device!");
        return RT_ERROR;
    }
		//读取版本号
		fm24clxx_read_page(eeprom_dev,FM24CLXX_ADDR_7BIT,EEPROM_ADDR_ID,\
											(uint8_t *)&sys.slaveID,2);//获取id号
		if((!sys.slaveID) || (sys.slaveID >= 0xFFF0))
			sys.slaveID = 0xFF;
		sys.type = BOARD_TYPE;
		sys.ver = LAYERBOARD_VERSION;
}
MSH_CMD_EXPORT(fm24cl16_init, fm24cl16 test);

