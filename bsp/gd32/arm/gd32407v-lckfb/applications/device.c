#include "device.h"
#include "main.h"
#include "eeprom.h"
uint16_t idTemp = 0xFFFF;
//==============================================================//
//�������ƣ�Device_CMDProcess				                              //
//�������豸ָ�����                                          //
//���룺*datIn	�������ݴ�ŵ�ַ                                //
//			lenIn		�������ݳ���                                    //
//�����*datOut		���������ݳ��ȴ�ŵ�ַ                        //
//			*lenOut		���������ݳ���                                //
//���أ�ִ�н�� 0 �ɹ�����0 �������                           //
//���ܣ������豸��Ϣ															      //
//==============================================================//

uint8_t Device_CMDProcess(uint8_t *datIn,uint8_t lenIn,uint8_t *datOut,uint8_t *lenOut)
{
	uint8_t index_test;
	switch(datIn[0])
	{
		case CMD_DEVICE_OWN_QUERY://��ѯID
				datOut[(*lenOut)++] = CMD_DEVICE_OWN_QUERY;	//��ѯ״̬������
				datOut[(*lenOut)++] = sys.type;							//��������
				datOut[(*lenOut)++] = (uint8_t)(sys.ver>>8);			//�汾��
				datOut[(*lenOut)++] = (uint8_t)sys.ver;
		break;
		case CMD_DEVICE_OWN_CONFIG://�������ذ�ID
			if(lenIn >= 2)
			{
				idTemp = (datIn[1]<<8)|datIn[2];									//�����õ�ID�ȴ�����
				datOut[(*lenOut)++] = CMD_DEVICE_OWN_CONFIG;	//����ID
				datOut[(*lenOut)++] = fm24clxx_write_page(eeprom_dev,FM24CLXX_ADDR_7BIT,EEPROM_ADDR_ID,\
											(uint8_t *)&idTemp,2);//��ȡid��
			}
		break;
	}
}