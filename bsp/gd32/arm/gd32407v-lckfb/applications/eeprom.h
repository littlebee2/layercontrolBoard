#ifndef __EEPROM_H__
#define __EEPROM_H__
#include <rtthread.h>
#include <board.h>
#include "rtdevice.h"
#include "main.h"
extern struct rt_i2c_bus_device *eeprom_dev;

#define FM24CLXX_ADDR_7BIT (0xA0 >> 1) 
#define	EEPROM_SIZE			2048

#define	EEPROM_ADDR_ID									0x0000		//�豸ID				2�ֽ�
#define	EEPROM_ADDR_LOCK_CONFIG					0x0002		//��������Ϣ		1�ֽ�*8


rt_err_t fm24cl16_init(void);

rt_err_t fm24clxx_write_page(		struct rt_i2c_bus_device *dev,
                                uint8_t page_addr_7bit,
                                uint8_t wirteAddr,          //fm24cl16�ڲ��洢��ַ
                                uint8_t *pBuffer, 
                                uint16_t NumByteToWrite);
rt_err_t fm24clxx_read_page(struct rt_i2c_bus_device *dev,
                            uint8_t page_addr_7bit, 
                            uint8_t readAddr,  //fm24cl16�ڲ��洢��ַ
                            uint8_t *pBuffer,  
                            uint16_t NumByteToRead);


#endif /*__EEPROM_H__*/
