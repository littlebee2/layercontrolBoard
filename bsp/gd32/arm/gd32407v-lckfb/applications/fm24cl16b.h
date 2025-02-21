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

#define	WEIGHT_EEPROM_SINGLE_SIZE       		20      	//���嵥������Ҫ��������ݴ�С

//�ܹ�8ҳ��ÿҳ256byte
#define	EEPROM_ADDR_ID                      0x0000  	//�豸ID2�ֽ�
#define EEPROM_ADDR_ELECTRIC_CONFIG         0x0003  	//�綯�������� 				5�ֽ�

#define EEPROM_ADDR_LOCK_CONFIG             0x000A    //��������        12�ֽ�
//��һҳǰ56byteΪ�������

#define	EEPROM_ADDR_WEIGHT_CALIB1           0                                   //�궨1                  4�ֽ�
#define	EEPROM_ADDR_WEIGHT_CALIB2           EEPROM_ADDR_WEIGHT_CALIB1+4         //�궨2                  4�ֽ�
#define	EEPROM_ADDR_WEIGHT_NET              EEPROM_ADDR_WEIGHT_CALIB2+4         //����                   4�ֽ�
#define	EEPROM_ADDR_WEIGHT_SINGLE           EEPROM_ADDR_WEIGHT_NET+4            //���õĵ���             2�ֽ�
#define	EEPROM_ADDR_WEIGHT_SINGLE_CAL       EEPROM_ADDR_WEIGHT_SINGLE+2         //����ĵ���             2�ֽ�
#define EEPROM_ADDR_WEIGHT_AD               EEPROM_ADDR_WEIGHT_SINGLE_CAL+2     //����ǰ��ADֵ           4�ֽ�
#define EEPROM_ADDR_WEIGHT_NUM              EEPROM_ADDR_WEIGHT_AD+4             //����                   4�ֽ�
#define	EEPROM_ADDR_LCD_SRTING              EEPROM_ADDR_WEIGHT_NUM+2            //LCD����ʾ�ַ���ʼ��ַ  32�ֽ�



static const struct  CEaddr
{
    rt_uint8_t page;
    rt_uint16_t addr;
}s_eepromAddr[12] = 
{
    {0, 0x56 + 0*WEIGHT_EEPROM_SINGLE_SIZE}, //��1��ʼ��ַ
    {0, 0x56 + 1*WEIGHT_EEPROM_SINGLE_SIZE}, //��2��ʼ��ַ
    
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
 * desc:FM24CL16Bд��1���ֽ�
 * params:
 * 	page:��Ҫд�����ݵ�ҳ��ַ��0~7��һ��8ҳ
 * 	addr:��Ҫд�����ݵĶε�ַ��0~255��һ��256����ַ��
 * 	writeData:��Ҫд����ֽ�����
 * return:
 * 	0:д��ɹ���1��Ӧ��ʧ��
 */
rt_uint8_t FM24CL16B_WriteByte(uint8_t page, uint8_t addr, uint8_t writeData);
/**
 * desc:FM24CL16B��ȡ1���ֽ�
 * params:
 * 	page:��Ҫ��ȡ���ݵ�ҳ��ַ��0~7��һ��8ҳ
 * 	addr:��Ҫ��ȡ���ݵĶε�ַ��0~255��һ��256����ַ��
 * 	writeData:���ض�ȡ�����ֽ�����
 * return:
 * 	0:д��ɹ���1��Ӧ��ʧ��
 */
rt_uint8_t FM24CL16B_ReadByte(uint8_t page, uint8_t addr, uint8_t *readData);

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
rt_uint8_t FM24CL16B_WriteBytes(uint8_t page, uint8_t addr, uint8_t *writeBuff, uint8_t len);

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
rt_uint8_t FM24CL16B_ReadBytes(uint8_t page, uint8_t addr, uint8_t *readBuff, uint8_t len);

#endif /* _EEPROM_H_ */

