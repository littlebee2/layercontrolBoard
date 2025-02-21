#ifndef _FM24CL16B_H_
#define _FM24CL16B_H_
#include <rtthread.h>
#include "rtdevice.h"
#include <drv_gpio.h>
#include "main.h"

#define FM24CL16B_SDA_PIN			GET_PIN(A,8)
#define FM24CL16B_SCL_PIN			GET_PIN(C,9)
#define	FM24CL16B_FW_PIN			GET_PIN(C,8)

#define FM24CL16B_SCL_HIGH 		rt_pin_write(FM24CL16B_SCL_PIN,PIN_HIGH)
#define FM24CL16B_SCL_LOW 		rt_pin_write(FM24CL16B_SCL_PIN,PIN_LOW)

#define FM24CL16B_SDA_HIGH 		rt_pin_write(FM24CL16B_SDA_PIN,PIN_HIGH)
#define FM24CL16B_SDA_LOW 		rt_pin_write(FM24CL16B_SDA_PIN,PIN_LOW)

#define FM24CL16B_SDA_READ 		rt_pin_read(FM24CL16B_SDA_PIN)

#define FM24CL16B_SLAVEID			0xA0
#define FM24CL16B_READ_BIT		0x01
#define FM24CL16B_WRITE_BIT		0x00

#define FM24CL16B_PAGE_READ(page)		(FM24CL16B_SLAVEID | (page << 1) | FM24CL16B_READ_BIT)
#define FM24CL16B_PAGE_WRITE(page)		(FM24CL16B_SLAVEID | (page << 1) | FM24CL16B_WRITE_BIT)

#define	EEPROM_SIZE			2048

#define	WEIGHT_EEPROM_SINGLE_SIZE       		20      	//定义单个称需要保存的数据大小

//总共8页，每页256byte
#define	EEPROM_ADDR_ID                      0x0000  	//设备ID2字节
#define EEPROM_ADDR_ELECTRIC_CONFIG         0x0003  	//电动滑轨配置 				5字节

#define EEPROM_ADDR_LOCK_CONFIG             0x000A    //锁控配置        12字节
//第一页前56byte为配置相关

#define	EEPROM_ADDR_WEIGHT_CALIB1           0                                   //标定1                  4字节
#define	EEPROM_ADDR_WEIGHT_CALIB2           EEPROM_ADDR_WEIGHT_CALIB1+4         //标定2                  4字节
#define	EEPROM_ADDR_WEIGHT_NET              EEPROM_ADDR_WEIGHT_CALIB2+4         //重量                   4字节
#define	EEPROM_ADDR_WEIGHT_SINGLE           EEPROM_ADDR_WEIGHT_NET+4            //设置的单重             2字节
#define	EEPROM_ADDR_WEIGHT_SINGLE_CAL       EEPROM_ADDR_WEIGHT_SINGLE+2         //计算的单重             2字节
#define EEPROM_ADDR_WEIGHT_AD               EEPROM_ADDR_WEIGHT_SINGLE_CAL+2     //掉电前的AD值           4字节
#define EEPROM_ADDR_WEIGHT_NUM              EEPROM_ADDR_WEIGHT_AD+4             //数量                   4字节
#define	EEPROM_ADDR_LCD_SRTING              EEPROM_ADDR_WEIGHT_NUM+2            //LCD屏显示字符起始地址  32字节



static const struct  CEaddr
{
    rt_uint8_t page;
    rt_uint16_t addr;
}s_eepromAddr[12] = 
{
    {0, 0x56 + 0*WEIGHT_EEPROM_SINGLE_SIZE}, //秤1起始地址
    {0, 0x56 + 1*WEIGHT_EEPROM_SINGLE_SIZE}, //秤2起始地址
    
    {1, 0*WEIGHT_EEPROM_SINGLE_SIZE},
    {1, 1*WEIGHT_EEPROM_SINGLE_SIZE},
    {1, 2*WEIGHT_EEPROM_SINGLE_SIZE},
    
    {2, 0*WEIGHT_EEPROM_SINGLE_SIZE},
    {2, 1*WEIGHT_EEPROM_SINGLE_SIZE},
    {2, 2*WEIGHT_EEPROM_SINGLE_SIZE},

    {3, 0*WEIGHT_EEPROM_SINGLE_SIZE},
    {3, 1*WEIGHT_EEPROM_SINGLE_SIZE},
    {3, 2*WEIGHT_EEPROM_SINGLE_SIZE},
    
    {4, 0*WEIGHT_EEPROM_SINGLE_SIZE},
};

void Eeprom_Init(void);


/**
 * desc:FM24CL16B写入1个字节
 * params:
 * 	page:需要写入数据的页地址：0~7，一共8页
 * 	addr:需要写入数据的段地址：0~255，一共256个地址段
 * 	writeData:需要写入的字节数据
 * return:
 * 	0:写入成功；1：应答失败
 */
rt_uint8_t FM24CL16B_WriteByte(uint8_t page, uint8_t addr, uint8_t writeData);
/**
 * desc:FM24CL16B读取1个字节
 * params:
 * 	page:需要读取数据的页地址：0~7，一共8页
 * 	addr:需要读取数据的段地址：0~255，一共256个地址段
 * 	writeData:返回读取到的字节数据
 * return:
 * 	0:写入成功；1：应答失败
 */
rt_uint8_t FM24CL16B_ReadByte(uint8_t page, uint8_t addr, uint8_t *readData);

/**
 * desc:FM24CL16B写入多个字节
 * params:
 * 	page:需要写入数据的页地址：0~7，一共8页
 * 	addr:需要写入数据的段地址：0~255，一共256个地址段
 * 	writeBuff:需要写入的字节数据
 * 	len:需要写入的数据长度
 * return:
 * 	0:写入成功；1：应答失败
 */
rt_uint8_t FM24CL16B_WriteBytes(uint8_t page, uint8_t addr, uint8_t *writeBuff, uint8_t len);

/**
 * desc:FM24CL16B读取多个字节
 * params:
 * 	page:需要读取数据的页地址：0~7，一共8页
 * 	addr:需要读取数据的段地址：0~255，一共256个地址段
 * 	readBuff:返回读取到的字节数据
 * 	len:需要读取的数据长度
 * return:
 * 	0:写入成功；1：应答失败
 */
rt_uint8_t FM24CL16B_ReadBytes(uint8_t page, uint8_t addr, uint8_t *readBuff, uint8_t len);

#endif /* _EEPROM_H_ */

