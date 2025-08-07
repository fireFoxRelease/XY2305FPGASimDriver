/**
 *
 *  CPCI_CAN WINDOWS 32BIT DRIVER  Version 1.2  (16/03/2016)
 *  Copyright (c) 1998-2016
 *  Hirain Technology, Inc.
 *  www.hirain.com
 *  support@hiraintech.com
 *  ALL RIGHTS RESERVED
 *
 **/

/**
 *
 *  This file includes many basic functions to control
 *  CPCI_CAN and support card. Most of functions implement
 *  basic configuration by setting register. You can get
 *  detailed register info in "CPCI-CAN user manual".
 *
 **/

#ifdef WIN32
#include <Windows.h>
// #include "samples/shared/bits.h"
// #include "status_strings.h"
#endif

// #include <unistd.h>
// #include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include "HiDrv.h"
#include "HiXY2305Reg.h"
#include "HiXY2305DriverPublic.h"

#define RW_REG_DELAY_TIME 2 /*Waiting for read or write reg*/

CARD_MAP cardMap;
// #define DLL_PUBLIC __attribute((visibility("default")))
#define Delay(delaytime) Sleep(delaytime) /*unit:ms*/

// FILE* pFd;

char dev_name[60];

HR_ULONG hiMasterOpenCard(HR_DEVICE *pDeviceHandle, DWORD nCardNum)
{
	unsigned int device_count = 0;
	PMaster_DEVICE_IDENTIFIER pList = NULL;
	PMaster_DEVICE_IDENTIFIER identifier;
	HR_RET dwStatus = 0;
	HR_UINT cardChnNumTmp = 0;
	HR_UINT offset = 0;
	HR_DEV_CTX *device;
	if (pDeviceHandle == NULL)
	{
		return ERROR_INVALID_PARAM;
	}
	CARD_MAP::iterator pos = cardMap.find(*pDeviceHandle);
	if (pos == cardMap.end())
	{
		device_count = HR_EnumDevice(&pList);
		for (identifier = pList; identifier; identifier = identifier->next)
		{
			if (identifier->index == nCardNum)
			{
				break;
			}
		}
		if (NULL == identifier)
		{
			return RET_FAIL;
		}
		device = HR_DeviceOpen(identifier->name);

		if (device == NULL)
		{
			return ERROR_OPEN_BOARD;
		}
		else
		{
			cardMap.insert(std::make_pair(device, nCardNum));
			*pDeviceHandle = device;
		}
	}
	else
	{
		return ERROR_ALREADY_OPEN;
	}
	return RET_SUCCESS;
}

HR_ULONG hiMasterCloseCard(HR_DEVICE hDeviceHandle)
{
	HR_RET dwStatus = 0;
	CARD_MAP::iterator pos = cardMap.find(hDeviceHandle);
	if (pos != cardMap.end())
	{
		HR_DeviceClose((PHR_DEV_CTX)hDeviceHandle);
		cardMap.erase(pos);
	}
	else
	{
		dwStatus = ERROR_ALREADY_CLOSE;
	}

	return dwStatus;
}

HR_ULONG hiMasterResetCard(HR_DEVICE hDeviceHandle, unsigned int cardNo)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	if (cardNo == 0)
	{
		offset = F1_BASE_ADDRESS + CARD_RESET_REG;
		dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
		if (dwStatus != 0)
		{
			return RET_FAIL;
		}
		Sleep(1000);
		dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 0);
		if (dwStatus != 0)
		{
			return RET_FAIL;
		}
	}
	else if (cardNo == 1)
	{
		offset = F2_BASE_ADDRESS + CARD_RESET_REG;
		dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
		if (dwStatus != 0)
		{
			return RET_FAIL;
		}
		Sleep(1000);
		dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 0);
		if (dwStatus != 0)
		{
			return RET_FAIL;
		}
	}
	else
	{
		return ERROR_INVALID_PARAM;
	}

	return RET_SUCCESS;
}

DLL_EXPORT HR_ULONG hiMasterGetCardVersion(HR_DEVICE hDeviceHandle, TY_CARD_INFO *pCardInfo)
{
	HR_RET dwStatus = 0;
	unsigned int regVale = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = F1_BASE_ADDRESS + CARD_TYPE_REG;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &pCardInfo->cardF1Type);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	offset = F2_BASE_ADDRESS + CARD_TYPE_REG;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &pCardInfo->cardF2Type);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	offset = F1_BASE_ADDRESS + CARD_F1_HARDWARE_REG;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &pCardInfo->hardwareVer);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	offset = F1_BASE_ADDRESS + CARD_F1_LOGIC_REG;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &pCardInfo->f1LogicVer);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	offset = F2_BASE_ADDRESS + CARD_F2_LOGIC_REG;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &pCardInfo->f2LogicVer);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return RET_SUCCESS;
}
HR_ULONG hiWriteDMA(HR_DEVICE hDeviceHandle, char *pData, unsigned int length)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	unsigned int writeLen = 0x1000000;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	unsigned long long dstAddr = 0x200000000;
	while (length)
	{
		HR_WriteReg32(hDeviceHandle, BAR1, DMA_WRITE_DEP_CTRL_REG, 0x4);
		HR_WriteReg32(hDeviceHandle, BAR1, DMA_INIT_NEXT_REG, 1);

		if (length > writeLen)
		{
			dwStatus = RK_WriteDMA(hDeviceHandle, dstAddr, pData, writeLen);
			if (dwStatus != 0)
			{
				return dwStatus;
			}
			length -= writeLen;
			pData += writeLen;
		}
		else
		{
			dwStatus = RK_WriteDMA(hDeviceHandle, dstAddr, pData, length);
			if (dwStatus != 0)
			{
				return dwStatus;
			}
			length = 0;
		}
	}
	return 0;
}
HR_ULONG hiReadDMA(HR_DEVICE hDeviceHandle, char *pData, unsigned int length)
{
	unsigned long dwStatus = 0;
	unsigned int status = 0;
	unsigned int readLen = 0x1000000;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}

	unsigned long long dstAddr = 0x200000000;
	while (length)
	{
		HR_WriteReg32(hDeviceHandle, BAR1, 0x1004, 0x4);
		HR_WriteReg32(hDeviceHandle, BAR1, DMA_INIT_NEXT_REG, 0x10);
		if (length >= readLen)
		{
			dwStatus = RK_ReadDMA(hDeviceHandle, dstAddr, pData, readLen);
			length -= readLen;
			pData += readLen;
		}
		else
		{
			dwStatus = RK_ReadDMA(hDeviceHandle, dstAddr, pData, length);
			length = 0;
		}
	}

	return dwStatus;
}

unsigned char reverseByte(unsigned char byte) {
	unsigned char result = 0;
	for (int i = 0; i < 8; i++) {
		result <<= 1;
		result |= (result >> i) & 1;
	}
	return result;
}
unsigned long long reverse_bytes(unsigned long long value) {
	unsigned long long result = 0;
	for (int i = 0; i < 8; i++) {
		result = (result << 8) | ((value >> (i * 8)) & 0xFF);
	}
	return result;
}
HR_ULONG hiReadFlash(HR_DEVICE pHandle, unsigned char* flashData, unsigned int length)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	unsigned int regValue = 0;
	int readNumber = 0;
	unsigned long long value = 0;

	offset = F1_BASE_ADDRESS + 0xA8; //F2的加载FLASH写入数据长度
	dwStatus = HR_WriteReg32(pHandle, BAR0, offset, length);
	if (dwStatus != 0)
	{
		return dwStatus;
	}
	offset = F1_BASE_ADDRESS + 0xA0; //F2的加载FLASH读出使能
	dwStatus = HR_WriteReg32(pHandle, BAR0, offset, 0x1);
	if (dwStatus != 0)
	{
		return dwStatus;
	}
	hiReadDMA(pHandle, (char*)flashData, length);
	offset = F1_BASE_ADDRESS + 0x94;
	while (1)
	{
		dwStatus = HR_ReadReg32(pHandle, BAR0, offset, &regValue);
		if (dwStatus != 0)
		{
			return RET_FAIL;
		}
		if ((regValue & 0x1) == 1)
		{
			break;
		}
	}
	for (int i = 0; i < length / 8; i++)
	{
		memcpy(&value, flashData + i * 8, 8);
		value = reverse_bytes(value);
		memcpy(flashData + i * 8, &value, 8);
		//pBuf[i] = reverseByte(pBuf[i]);
	}
	offset = F1_BASE_ADDRESS + 0x84; //F2 FLASH相关的完成标志清除寄存器。
	dwStatus = HR_WriteReg32(pHandle, BAR0, offset, 0x4);
	if (dwStatus != 0)
	{
		return dwStatus;
	}
	return 0;





	offset = F1_BASE_ADDRESS + 0xB0;
	while (1)
	{
		dwStatus = HR_ReadReg32(pHandle, BAR0, offset, &regValue);
		if (dwStatus != 0)
		{
			return RET_FAIL;
		}
		if (regValue == 0)
		{
			break;
		}
	}

	offset = F1_BASE_ADDRESS + 0xA8;//读长度下发
	dwStatus = HR_WriteReg32(pHandle, BAR0, offset, length);
	if (dwStatus != 0)
	{
		return dwStatus;
	}

	offset = F1_BASE_ADDRESS + 0xA0;//读使能寄存器
	dwStatus = HR_WriteReg32(pHandle, BAR0, offset, 0x1);
	if (dwStatus != 0)
	{
		return dwStatus;
	}
	while (1)
	{
		offset = F1_BASE_ADDRESS + 0x28;
		dwStatus = HR_ReadReg32(pHandle, BAR0, offset, &regValue);
		if (dwStatus != 0)
		{
			return RET_FAIL;
		}
		dwStatus = hiReadDMA(pHandle, (char*)flashData + readNumber, regValue * 512);
		if (dwStatus != 0)
		{
			return dwStatus;
		}
		readNumber = readNumber + regValue * 512;
		Sleep(10);
	}
	for (int i = 0; i < length / 8; i++)
	{
		memcpy(&value, flashData + i * 8, 8);
		value = reverse_bytes(value);
		memcpy(flashData + i * 8, &value, 8);
		//pBuf[i] = reverseByte(pBuf[i]);
	}
	return dwStatus;
}
HR_ULONG hiLoadFlash(HR_DEVICE hDeviceHandle, const char* fileName)
{
	unsigned char test[1024];
	for (int i = 0; i < 1024; i++)
	{
		test[i] = i;
	}
	HR_RET dwStatus = 0;
	unsigned char* pBuf = NULL;
	unsigned int fileSize = 0;
	unsigned int fileSize_1 = 0;
	unsigned int offset = 0;
	unsigned long long value = 0;
	unsigned int regValue = 0;
	unsigned int count = 0;
	FILE* fp = fopen(fileName, "ab+");
	if (fp == NULL)
	{
		return ERROR_INVALID_PARAM;
	}
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (fileSize % 512 == 0)
	{
		fileSize_1 = fileSize;
	}
	else
	{
		fileSize_1 = (fileSize / 512 + 1) * 512;
	}
	pBuf = (unsigned char*)malloc(fileSize_1);
	memset(pBuf,0, fileSize_1);
	fread(pBuf, 1, fileSize, fp);
	for (int i = 0; i < fileSize_1; i++)
	{
		//pBuf[i] = i;
	}
	for (int i = 0; i < fileSize_1 / 8; i++)
	{
		memcpy(&value, pBuf + i * 8, 8);
		value = reverse_bytes(value);
		memcpy(pBuf + i * 8, &value, 8);
		//pBuf[i] = reverseByte(pBuf[i]);
	}
	offset = F1_BASE_ADDRESS + 0xA4;//FLASH 写入数据长度 补齐512字节
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, fileSize_1);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
    
	offset = F1_BASE_ADDRESS + 0x80;//配置擦除使能
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	offset = F1_BASE_ADDRESS + 0x88;//F2 FLASH擦除完成标志
	while (1)
	{

		dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
		if (dwStatus != 0)
		{
			free(pBuf);
			return dwStatus;
		}
		if ((regValue & 0x1) == 1)
		{
			break;
		}
		Sleep(100);
		count++;
		if (count > 2000)
		{
			printf("time out\r\n");
			return 0xFF;
		}
	}
	Sleep(1000);
	offset = F1_BASE_ADDRESS + 0x84;//配置擦除完成标志位
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	offset = F1_BASE_ADDRESS + 0x9C;//0：F2加载FLASH不写入 1：F2加载FLASH写入，一次写入K7镜像11443612字节数据

	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	Sleep(1000);
	dwStatus = hiWriteDMA(hDeviceHandle, (char*)pBuf, fileSize_1);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	offset = F1_BASE_ADDRESS + 0x8C;//FLASH写入bit数据完成标志
	while (1)
	{

		dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
		if (dwStatus != 0)
		{
			free(pBuf);
			return dwStatus;
		}
		if ((regValue & 0x1) == 1)
		{
			break;
		}
		Sleep(100);
		count++;
		if (count > 2000)
		{
			printf("time out\r\n");
			return 0xFFF;
		}
	}

	offset = F1_BASE_ADDRESS + 0x84;//配置F2 FLASH相关的完成标志清除寄存器
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 2);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	return 0;
	/*offset = F1_BASE_ADDRESS + 0x84;//flash 清除
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 7);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 0);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	offset = F1_BASE_ADDRESS + 0x80;//flash 擦除
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	offset = F1_BASE_ADDRESS + 0x88;//flash 擦除完成flag
	while (1)
	{
		dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
		if (dwStatus != 0)
		{
			free(pBuf);
			return dwStatus;
		}
		if (regValue == 1)
		{
			break;
		}
	}*/
	offset = F1_BASE_ADDRESS + 0xA4;//长度下发
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, fileSize_1);
	//dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1024);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}

	offset = F1_BASE_ADDRESS + 0x24;//写数据类型寄存器
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 0x1);//1 写flash 2 对F2加载
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	offset = F1_BASE_ADDRESS + 0x9C;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	dwStatus = hiWriteDMA(hDeviceHandle, (char*)pBuf, fileSize_1);
	//dwStatus = hiWriteDMA(hDeviceHandle, (char*)test, 1024);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	free(pBuf);
	return dwStatus;
}
HR_ULONG hiLoadBit(HR_DEVICE hDeviceHandle, const char* fileName)
{
	HR_RET dwStatus = 0;
	FILE* fp = NULL;
	unsigned char* pBuf = NULL;
	unsigned int fileSize = 0;
	unsigned int fileSize_1 = 0;
	unsigned int offset = 0;
	unsigned int flashStatus;
	unsigned long long value = 0;
	FILE* fp1 = fopen(fileName, "ab+");
	if (fp1 == NULL)
	{
		return ERROR_INVALID_PARAM;
	}
	fseek(fp1, 0, SEEK_END);
	fileSize = ftell(fp1);
	fseek(fp1, 0, SEEK_SET);
	if (fileSize % 512 != 0)
	{
		fileSize_1 = (fileSize / 512 + 1) * 512;
	}
	else
	{
		fileSize_1 = fileSize;
	}
	pBuf = (unsigned char*)malloc(fileSize_1);
	memset(pBuf, 0, fileSize_1);
	fread(pBuf, 1, fileSize, fp1);
	fclose(fp1);

	for (int i = 0; i < fileSize_1 / 8; i++)
	{
		memcpy(&value,pBuf + i * 8, 8);
		value = reverse_bytes(value);
		memcpy(pBuf + i * 8, &value,8);
		//pBuf[i] = reverseByte(pBuf[i]);
	}
	offset = F1_BASE_ADDRESS + 0x20;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 0);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}


	offset = F1_BASE_ADDRESS + 0x24;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 0x2);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}

	dwStatus = hiWriteDMA(hDeviceHandle, (char*)pBuf, fileSize_1);
	if (dwStatus != 0)
	{
		free(pBuf);
		return dwStatus;
	}
	free(pBuf);
	return dwStatus;
}
HR_ULONG hiWritePulseStep(HR_DEVICE hDeviceHandle, unsigned int value)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = F2_BASE_ADDRESS + CARD_F2_PULSE_STEP_REG;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

DLL_EXPORT HR_ULONG hiRegUpLoadEnable(HR_DEVICE hDeviceHandle, unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = F2_BASE_ADDRESS + CARD_F2_UPLOAD_ENABLE_REG;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, enableFlag);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIOChnEnable(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int regValue = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = DIO_BASE_ADDRESS + CARD_IO_CHN_ENABLE;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	if (enableFlag == 1)
	{
		regValue |= (1 << chnNum);
	}
	else
	{
		regValue &= ~(1 << chnNum);
	}
	
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

DLL_EXPORT HR_ULONG hiIOChnMon(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int* pMonitor)
{
	HR_RET dwStatus = 0;
	unsigned int regValue = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = DIO_BASE_ADDRESS + CARD_IO_CHN_MON;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	*pMonitor = (regValue >> chnNum) & 0x1;
	return HR_SUCCESS;
}
DLL_EXPORT HR_ULONG hiIOChnDir(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int dir)
{
	HR_RET dwStatus = 0;
	unsigned int regValue = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = DIO_BASE_ADDRESS + CARD_IO_CHN_DIR;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	if (dir == 1)
	{
		regValue |= (1 << chnNum);
	}
	else
	{
		regValue &= ~(1 << chnNum);
	}

	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIOChnOut(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int outFlag)
{
	HR_RET dwStatus = 0;
	unsigned int regValue = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = DIO_BASE_ADDRESS + CARD_IO_CHN_DO_DATA;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	if (outFlag == 1)
	{
		regValue |= (1 << chnNum);
	}
	else
	{
		regValue &= ~(1 << chnNum);
	}

	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIOChnSetInSamp(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int samp)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = DIO_BASE_ADDRESS + CARD_IO_CHN_IN_SAMP;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, samp);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

HR_ULONG hiIOChnGetInData(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int* data)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = DIO_BASE_ADDRESS + CARD_IO_CHN_DI_DATA;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, data);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	*data = (*data >> chnNum) & 0x1;
	return HR_SUCCESS;
}
HR_ULONG hiAnalogOutEnable(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;

	}
	if (enableFlag != 1 && enableFlag!= 0)
	{
		return ERROR_INVALID_PARAM;
	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_ENABLE;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, enableFlag);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiAnalogOutSel(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int selFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;

	}
	if (selFlag != 1 && selFlag != 0)
	{
		return ERROR_INVALID_PARAM;
	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_SEL;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, selFlag);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

HR_ULONG hiAnalogOutData(HR_DEVICE hDeviceHandle, unsigned int chn, float data)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	unsigned int regValue = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;

	}

	regValue = (data + 10.0) * 65535.0 / 20.0;
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_DATA;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, regValue);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}


HR_ULONG hiAnalogOutExcSel(HR_DEVICE hDeviceHandle, unsigned int chn, int sel)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;

	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_EXC_SEL;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, sel);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiAnalogOutVpp(HR_DEVICE hDeviceHandle, unsigned int chn, int vpp)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;

	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_VPP;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, vpp);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

HR_ULONG hiAnalogOutFreq(HR_DEVICE hDeviceHandle, unsigned int chn, int freq)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;

	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_FREQ;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, freq);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
#if 0  
HR_ULONG hiAnalogAjustCalEnable(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	if (enableFlag!= 1 && enableFlag!= 0)
	{
		return ERROR_INVALID_PARAM;
	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_CAL_EN;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, enableFlag);
	if (dwStatus!= 0)
	{
		return RET_FAIL;	
	}
	return HR_SUCCESS;
}
#endif

 
HR_ULONG hiAnalogAjustCalData(HR_DEVICE hDeviceHandle, unsigned int chn, float k,short b)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_CAL_K;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, k);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	offset = AO_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_CAL_B;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, b);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
   	
}
HR_ULONG hiAnalogInEnable(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	if (enableFlag!= 1 && enableFlag!= 0)
	{
		return ERROR_INVALID_PARAM;
	}
	offset = AI_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_AI_ENABLE;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, enableFlag);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

DLL_EXPORT HR_ULONG hiAnalogInData(HR_DEVICE hDeviceHandle, unsigned int chn, float* data)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	unsigned int regValue = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = AI_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_AI_DATA;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &regValue);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	*data = (float)regValue * 40.0 / 65535.0 - 20.0;
	return HR_SUCCESS;	
}

HR_ULONG hiAnalogAjustCalInData(HR_DEVICE hDeviceHandle, unsigned int chn, int k,int b)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = AI_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_AI_K;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, k);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	offset = AI_BASE_ADDRESS + 0x1000 * chn + CARD_ANALOG_AI_B;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, b);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiHallEnable(HR_DEVICE hDeviceHandle,unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = HALL_BASE_ADDRESS + CARD_HALL_ENABLE;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, enableFlag);
	if (dwStatus!= 0)
	{
		return RET_FAIL;	
	}
	return HR_SUCCESS;
}

HR_ULONG hiHallStatus(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int* status)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = HALL_BASE_ADDRESS + CARD_HALL_STATUS;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, status);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	*status = (*status >> chn) & 0x1;
	return HR_SUCCESS;
}
HR_ULONG hiHallTime(HR_DEVICE hDeviceHandle,unsigned int* time)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = HALL_BASE_ADDRESS + CARD_HALL_TIME;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, time);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeEnable(HR_DEVICE hDeviceHandle, unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_ENABLE;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, enableFlag);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeLines(HR_DEVICE hDeviceHandle, unsigned int lines)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	if(lines <30 || lines > 65535)
	{
		return ERROR_INVALID_PARAM;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_LINES;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, lines);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeAngle(HR_DEVICE hDeviceHandle, unsigned int* angle)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_ANGLE;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, angle);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	if((*angle >> 0xF & 0x1) == 1)
	{
		*angle = *angle & 0xFF;
	}
	else
	{
		*angle = 0;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeSpeed(HR_DEVICE hDeviceHandle, unsigned int* speed)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_SPEED;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, speed);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeHigh(HR_DEVICE hDeviceHandle, unsigned int* high)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_HIGH;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, high);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeLow(HR_DEVICE hDeviceHandle, unsigned int* low)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_LOW;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, low);
	if (dwStatus!= 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeZHigh(HR_DEVICE hDeviceHandle, unsigned int* high)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODEZ_HIGH;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, high);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeZLow(HR_DEVICE hDeviceHandle, unsigned int* low)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODEZ_LOW;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, low);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeSetTime(HR_DEVICE hDeviceHandle, unsigned int time)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_CHANGE_TIME;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, time);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiIncodeAB(HR_DEVICE hDeviceHandle, unsigned int* value)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = ENCODE_BASE_ADDRESS + CARD_ENCODE_CHANGE_AB;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiPWMEnable(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int enableFlag)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = PWM_BASE_ADDRESS + chn * 0x1000 + CARD_PWM_EN;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, enableFlag);
	unsigned int value = 0;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiPWMSetSpeed(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int speed)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = PWM_BASE_ADDRESS + chn * 0x1000 + CARD_PWM_PERIOD;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, speed);
	unsigned int value = 0;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}
HR_ULONG hiPWMLow(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int low)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = PWM_BASE_ADDRESS + chn * 0x1000 + CARD_PWM_LOW;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, low);
	unsigned int value = 0;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

HR_ULONG hiPWMHigh(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int high)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = PWM_BASE_ADDRESS + chn * 0x1000 + CARD_PWM_HIGH;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, high);
	unsigned int value = 0;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

HR_ULONG hiPWMDied(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int died)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}

	offset = PWM_BASE_ADDRESS + chn * 0x1000 + CARD_PWM_DIED;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, died);
	unsigned int value = 0;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

HR_ULONG hiPWMSel(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int sel)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = PWM_BASE_ADDRESS + chn * 0x1000 + CARD_PWM_SEL;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, sel);
	unsigned int value = 0;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, &value);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	return HR_SUCCESS;
}

HR_ULONG hiGetFlashId(HR_DEVICE hDeviceHandle, unsigned int* id)
{
	HR_RET dwStatus = 0;
	unsigned int offset = 0;
	unsigned int regValue = 0;
	if (hDeviceHandle == NULL)
	{
		return ERROR_NULL_POINTER;
	}
	offset = F1_BASE_ADDRESS + 0x9C;
	dwStatus = HR_WriteReg32(hDeviceHandle, BAR0, offset, 1);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}
	Sleep(100);
	offset = F1_BASE_ADDRESS + 0xAC;
	dwStatus = HR_ReadReg32(hDeviceHandle, BAR0, offset, id);
	if (dwStatus != 0)
	{
		return RET_FAIL;
	}


	return HR_SUCCESS;
}