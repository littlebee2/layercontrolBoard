#include "device.h"
#include "main.h"
#include "eeprom.h"
uint16_t idTemp = 0xFFFF;
//==============================================================//
//函数名称：Device_CMDProcess				                              //
//描述：设备指令处理函数                                          //
//输入：*datIn	接收数据存放地址                                //
//			lenIn		接收数据长度                                    //
//输出：*datOut		待返回数据长度存放地址                        //
//			*lenOut		待返回数据长度                                //
//返回：执行结果 0 成功，非0 错误代码                           //
//功能：处理设备信息															      //
//==============================================================//

uint8_t Device_CMDProcess(uint8_t *datIn,uint8_t lenIn,uint8_t *datOut,uint8_t *lenOut)
{
	uint8_t index_test;
	switch(datIn[0])
	{
		case CMD_DEVICE_OWN_QUERY://查询ID
				datOut[(*lenOut)++] = CMD_DEVICE_OWN_QUERY;	//查询状态操作码
				datOut[(*lenOut)++] = sys.type;							//板子类型
				datOut[(*lenOut)++] = (uint8_t)(sys.ver>>8);			//版本号
				datOut[(*lenOut)++] = (uint8_t)sys.ver;
		break;
		case CMD_DEVICE_OWN_CONFIG://配置网关板ID
			if(lenIn >= 2)
			{
				idTemp = (datIn[1]<<8)|datIn[2];									//将设置的ID先存起来
				datOut[(*lenOut)++] = CMD_DEVICE_OWN_CONFIG;	//设置ID
				datOut[(*lenOut)++] = fm24clxx_write_page(eeprom_dev,FM24CLXX_ADDR_7BIT,EEPROM_ADDR_ID,\
											(uint8_t *)&idTemp,2);//获取id号
			}
		break;
	}
}