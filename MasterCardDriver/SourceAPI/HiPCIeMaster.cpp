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
//#include "samples/shared/bits.h"
//#include "status_strings.h"
#endif

//#include <unistd.h>
//#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include "HiDrv.h"
#include "HiPCIeMaterReg.h"
#include "HiPCIeMasterPublic.h"


#define RW_REG_DELAY_TIME 2 /*Waiting for read or write reg*/

CARD_MAP cardMap;
// #define DLL_PUBLIC __attribute((visibility("default")))
#define Delay(delaytime) Sleep(delaytime) /*unit:ms*/

//FILE* pFd;




char dev_name[60];

HR_ULONG hiMasterOpenCard(HR_DEVICE *pDeviceHandle, DWORD nCardNum)
{
	unsigned int device_count = 0;
	PMaster_DEVICE_IDENTIFIER pList = NULL;
	PMaster_DEVICE_IDENTIFIER identifier;
	HR_RET dwStatus = 0;
	HR_UINT cardChnNumTmp = 0;
	HR_UINT offset = 0;
	HR_DEV_CTX* device;
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


