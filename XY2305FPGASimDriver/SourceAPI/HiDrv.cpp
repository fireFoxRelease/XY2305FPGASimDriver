#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <Windows.h>
#include <Setupapi.h>
#include <WinIoCtl.h>
#include <initguid.h>
#include <process.h>
#include "HiDrv.h"
#include "HiXY2305Reg.h"
#include "HiXY2305DriverPublic.h"
#include <Public.h>
//#include "ntddk.h"


extern CARD_MAP cardMap;




int intcount;
int recvCountTmp = 0;
int dmaCount = 0;
int chnNo = 0;
int parsePackage(HR_DEVICE hDev)
{
	
	return 0;

}
static void HR_IntHandlerDma(PHR_DEV_CTX pDev)
{
	
	HR_UINT intStatus = 0;
	INT_PARA* intParaTmp;

	HR_DEV_CTX* pDevCtx;
	CARD_MAP::iterator pos = cardMap.find(pDev);

	if (pDev == NULL)
	{
		return;
	}

	intParaTmp = (INT_PARA*)pDev->intParam;
	intParaTmp->pDev = (HR_DEVICE)pDev;
	int ret = parsePackage((HR_DEVICE)pDev);
	if (ret == 0)
	{
		if (pDev->funcDiagIntHandler)
		{
			pDev->funcDiagIntHandler(*((INT_PARA*)pDev->intParam));
		}
	}
}

static DWORD IntEnableDma(HR_DEVICE pDev, PCIE_MASTER_INT_HANDLER funcDiagIntHandler, PVOID pData)
{
	HR_UINT u32DMAMODE;
	HR_UINT u32INTCSR;
	HR_UINT pAddr = 0;
	HR_UINT read;
	HR_UINT offset = 0;
	HR_DEV_CTX* handle = (HR_DEV_CTX*)pDev;
	char writeDataTmp[8] = { 0 };
	//handle->intParam = new INT_PARA;
	if (pData)
	{
		memcpy(handle->intParam, pData, sizeof(INT_PARA));
	}
	else
	{
		memset(handle->intParam, 0, sizeof(INT_PARA));
	}

	if (funcDiagIntHandler)
	{
		handle->funcDiagIntHandler = funcDiagIntHandler;
	}

	return HR_SUCCESS;
}
UINT WINAPI intThread(LPVOID para)
{
	PHR_DEV_CTX pDevCtx = (PHR_DEV_CTX)para;
	//HANDLE *phEvent = (HANDLE *)para;

	LARGE_INTEGER litmp;
	LARGE_INTEGER nLastTime2;
	LARGE_INTEGER nFreq, nFreq1;
	long long qt1, qt2, qt3;
	double dfm, dff, dft;
	double dfm1, dff1, dft1;

	while (pDevCtx->threadFlag)
	{
		WaitForSingleObject(pDevCtx->selfMutex, INFINITE);

		WaitForSingleObject(pDevCtx->hEvent, INFINITE);


		HR_IntHandlerDma(pDevCtx);

		ReleaseMutex(pDevCtx->selfMutex);
		//	printf("ReleaseMutex out \n");

	}
	//printf("intThread out \n");
	return 0;
}

DWORD HR_IntEnable(HR_DEVICE hDev, PCIE_MASTER_INT_HANDLER funcDiagIntHandler, PVOID pData)
{
	DWORD dwStatus = 0;
	HR_UINT dmaBusy = 0;
	OVERLAPPED overlap;
	PHR_DEV_CTX pDevCtx = (PHR_DEV_CTX)hDev;
	HR_UINT intEnable, offset;
	ZeroMemory(&overlap, sizeof(OVERLAPPED));

	overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!overlap.hEvent)
	{
		return -1;
	}


	//
	//	1. 创建用户层的等待事件，传入内核 
	//	2. 创建线程，用于监测内核事件的到来
	//
	pDevCtx->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	pDevCtx->threadFlag = 1;
	pDevCtx->hIntThread = (HANDLE)_beginthreadex(NULL, 0, intThread, pDevCtx, 0, NULL);

	//先将用户层的等待Event传入内核
	DeviceIoControl(pDevCtx->device, IOCTL_SET_EVENT, &pDevCtx->hEvent, sizeof(HANDLE), NULL, 0, NULL, &overlap);

	dwStatus = IntEnableDma(hDev, funcDiagIntHandler, pData);


	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}
	return dwStatus;
}


DWORD HR_IntDisable(HR_DEVICE hDev)
{
	DWORD dwStatus = 0;
	PHR_DEV_CTX pDevCtx = (PHR_DEV_CTX)hDev;
	OVERLAPPED overlap;

	ZeroMemory(&overlap, sizeof(OVERLAPPED));

	//clear kernel event
	overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!overlap.hEvent)
	{
		return -1;
	}

	/* Physically disable the interrupts on the board */
	HR_UINT offset;

	DeviceIoControl(pDevCtx->device, IOCTL_CLEAR_EVENT, NULL, sizeof(HANDLE), NULL, 0, NULL, &overlap);
	pDevCtx->threadFlag = 0;
	pDevCtx->intFlag = 0;

	if (pDevCtx->hIntThread)
	{
		//TerminateThread(pDevCtx->hIntThread, 0);
		::CloseHandle(pDevCtx->hIntThread);
	}

	if (overlap.hEvent)
	{

		::CloseHandle(overlap.hEvent);
	}

	if (pDevCtx->hEvent)
	{
		SetEvent(pDevCtx->hEvent);
		Sleep(500);
		::CloseHandle(pDevCtx->hEvent);
	}

	Sleep(200);

	return dwStatus;
}


#if 1
int
HR_EnumDevice(
PMaster_DEVICE_IDENTIFIER* id_list)
{
	if (id_list == NULL)
	{
		return ERROR_NULL_POINTER;
	}

	HDEVINFO                   deviceInfo;
	SP_DEVICE_INTERFACE_DATA   deviceInfoData;
	PMaster_DEVICE_IDENTIFIER   identifierHeader = NULL;
	//const GUID *GUID_DEVINTERFACE_MasterCard;

	*id_list = NULL;
	deviceInfo = ::SetupDiGetClassDevs(
		(LPGUID)&GUID_DEVINTERFACE_MasterCard,
		//NULL,
		NULL,
		NULL,
		(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)
		);
	if (INVALID_HANDLE_VALUE == deviceInfo)
	{
		return 0;
	}

	deviceInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	for (ULONG index = 0;
		::SetupDiEnumDeviceInterfaces(deviceInfo,
		0,
		(LPGUID)&GUID_DEVINTERFACE_MasterCard,
		index,
		&deviceInfoData);
	index++
		)
	{
		PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData;
		ULONG                            requiredLength;

		::SetupDiGetDeviceInterfaceDetail(
			deviceInfo,
			&deviceInfoData,
			NULL,
			0,
			&requiredLength,
			NULL);
		deviceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(
			GPTR, requiredLength
			);

		if (!deviceDetailData)
		{
			continue;
		}

		deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		::SetupDiGetDeviceInterfaceDetail(
			deviceInfo,
			&deviceInfoData,
			deviceDetailData,
			requiredLength,
			&requiredLength,
			NULL);

		HANDLE device = ::CreateFile(deviceDetailData->DevicePath,
			GENERIC_READ,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
			);

		if (INVALID_HANDLE_VALUE != device)
		{
			PMaster_DEVICE_IDENTIFIER idNode = new Master_DEVICE_IDENTIFIER;

			if (idNode)
			{
				ZeroMemory(idNode, sizeof(Master_DEVICE_IDENTIFIER));
				size_t nNameLen = _tcslen(deviceDetailData->DevicePath) + 1;
				idNode->name = new char[nNameLen];
				ZeroMemory(idNode->name, sizeof(char)* nNameLen);

#ifdef UNICODE
				::WideCharToMultiByte(
					CP_ACP,
					0,
					deviceDetailData->DevicePath,
					nNameLen,
					idNode->name,
					nNameLen,
					NULL,
					NULL
					);
#else
				strcpy_s(idNode->name, nNameLen, deviceDetailData->DevicePath);
#endif // UNICODE

				DWORD dwReturned = 0;
				if (::DeviceIoControl(device,
					IOCTL_GET_DEVICE_IDENTIFICATION,
					NULL,
					0,
					&(idNode->location),
					sizeof(PCI_DEVICE_LOCATION),
					&dwReturned,
					NULL))
				{
					idNode->next = identifierHeader;
					identifierHeader = idNode;
				}

				else
				{
					delete idNode->name;
					delete idNode;
				}
				int errorcode = GetLastError();

			}

			::CloseHandle(device);
		}

		::GlobalFree(deviceDetailData);
	}

	::SetupDiDestroyDeviceInfoList(deviceInfo);

	while (identifierHeader)
	{
		PMaster_DEVICE_IDENTIFIER pDevIdentifier = identifierHeader;
		identifierHeader = identifierHeader->next;
		pDevIdentifier->next = *id_list;
		*id_list = pDevIdentifier;
	}

	unsigned int counter = 0;

	identifierHeader = *id_list;
	while (identifierHeader)
	{
		identifierHeader->index = counter++;
		identifierHeader = identifierHeader->next;
	}

	return counter;
}

void HR_FreeIdentifier(PMaster_DEVICE_IDENTIFIER id_list)
{
	while (id_list)
	{
		PMaster_DEVICE_IDENTIFIER idNode = id_list;
		id_list = id_list->next;

		delete idNode->name;
		delete idNode;
	}
}

int HR_GetLocation(HR_DEV_CTX* handle, PPCI_DEVICE_LOCATION location)
{
	int status;
	if (!handle || !location)
	{
		return ERROR_NULL_POINTER;
	}

	OVERLAPPED overlap;
	ZeroMemory(&overlap, sizeof(overlap));

	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		status = ::DeviceIoControl(
			handle->device,
			IOCTL_GET_DEVICE_IDENTIFICATION,
			NULL,
			0,
			location,
			sizeof(PCI_DEVICE_LOCATION),
			NULL,
			&overlap
			);

		::WaitForSingleObject(overlap.hEvent, INFINITE);

		status = 0;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return status;
}


int Map_IOMemory(HR_DEV_CTX* handle)
{
	int status = 0;
	OVERLAPPED overlap;

	if (handle == NULL)
	{
		return ERROR_INVALID_HANDLE;
	}

	ZeroMemory(&overlap, sizeof(overlap));

	do
	{

		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		status = ::DeviceIoControl(
			handle->device,
			IOCTL_GET_BAR_POINTER,
			NULL,
			0,
			&handle->memory,
			sizeof(handle->memory),
			NULL,
			&overlap
			);

		int errorcode;
		if (status == 0)
		{
			errorcode = GetLastError();
			::WaitForSingleObject(overlap.hEvent, INFINITE);
			//break;
		}

		status = HR_SUCCESS;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return status;
}

int UnMap_IOMemory(HR_DEV_CTX* handle)
{
	int status = 0;
	OVERLAPPED overlap;

	if (handle == NULL)
	{
		return ERROR_INVALID_HANDLE;
	}

	ZeroMemory(&overlap, sizeof(overlap));

	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		status = ::DeviceIoControl(
			handle->device,
			IOCTL_RELEASE_BAR_POINTER,
			&handle->memory,
			sizeof(handle->memory),
			NULL,
			0,
			NULL,
			&overlap
			);

		if (status == 0)
		{
			::WaitForSingleObject(overlap.hEvent, INFINITE);
		}

		status = HR_SUCCESS;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return status;
}

int Map_RxBuffer(HR_DEV_CTX* handle)
{
	int status = 0;
	OVERLAPPED overlap;

	if (NULL == handle)
	{
		return ERROR_INVALID_HANDLE;
	}

	ZeroMemory(&overlap, sizeof(overlap));

	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		/*if (!::DeviceIoControl(
			handle->device,
			IOCTL_MAP_RX_DMA_BUFFER,
			NULL,
			0,
			&handle->rxBuffer,
			sizeof(handle->rxBuffer),
			NULL,
			&overlap
			))
			break;*/

		status = ::DeviceIoControl(
			handle->device,
			IOCTL_MAP_READ_BUFFER,
			NULL,
			0,
			&handle->rxBuffer,
			sizeof(handle->rxBuffer),
			NULL,
			&overlap
			);
		memset(handle->rxBuffer.virAddress, 0, handle->rxBuffer.length);
		if (status == 0)
		{
			int errorcode = GetLastError();
			::WaitForSingleObject(overlap.hEvent, INFINITE);
		}

		status = HR_SUCCESS;
	} while (FALSE);
	//printf("handle->rxBuffer.vir=%x\n", handle->rxBuffer.virAddress);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return status;
}

int UnMap_RxBuffer(HR_DEV_CTX* handle)
{
	int status = 0;
	OVERLAPPED overlap;

	if (NULL == handle)
	{
		return ERROR_INVALID_HANDLE;
	}

	if (NULL == handle->rxBuffer.virAddress)
	{
		return HR_SUCCESS;
	}

	ZeroMemory(&overlap, sizeof(overlap));

	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		/*status = ::DeviceIoControl(
			handle->device,
			IOCTL_RELEASE_RX_POINTER,
			&(handle->rxBuffer.virAddress),
			sizeof(handle->rxBuffer.virAddress),
			NULL,
			0,
			NULL,
			&overlap
			);*/

		status = ::DeviceIoControl(
			handle->device,
			IOCTL_RELEASE_READ_POINTER,
			&(handle->rxBuffer.virAddress),
			sizeof(handle->rxBuffer.virAddress),
			NULL,
			0,
			NULL,
			&overlap
			);

		if (status == 0)
		{
			::WaitForSingleObject(overlap.hEvent, INFINITE);
		}

		handle->rxBuffer.virAddress = NULL;

		status = HR_SUCCESS;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return status;
}

int Map_TxBuffer(HR_DEV_CTX* handle)
{
	int status = 0;
	OVERLAPPED overlap;

	if (NULL == handle)
	{
		return ERROR_INVALID_HANDLE;
	}

	ZeroMemory(&overlap, sizeof(overlap));

	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		status = ::DeviceIoControl(
			handle->device,
			IOCTL_MAP_WRITE_BUFFER,
			NULL,
			0,
			&handle->txBuffer,
			sizeof(handle->txBuffer),
			NULL,
			&overlap
			);

		if (status == 0)
		{
			int errorcode = GetLastError();
			::WaitForSingleObject(overlap.hEvent, INFINITE);
		}

		status = HR_SUCCESS;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return status;
}

int UnMap_TxBuffer(HR_DEV_CTX* handle)
{
	int status = 0;
	OVERLAPPED overlap;

	if (NULL == handle)
	{
		return ERROR_INVALID_HANDLE;
	}

	if (NULL == handle->txBuffer.virAddress)
	{
		return HR_SUCCESS;
	}

	ZeroMemory(&overlap, sizeof(overlap));

	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		status = ::DeviceIoControl(
			handle->device,
			IOCTL_RELEASE_WRITE_POINTER,
			&(handle->txBuffer.virAddress),
			sizeof(handle->txBuffer.virAddress),
			NULL,
			0,
			NULL,
			&overlap
			);

		if (status == 0)
		{
			::WaitForSingleObject(overlap.hEvent, INFINITE);
		}

		handle->txBuffer.virAddress = NULL;

		status = HR_SUCCESS;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return status;
}

void UnMap_Resouce(HR_DEV_CTX* hDev)
{
	UnMap_TxBuffer(hDev);
	UnMap_RxBuffer(hDev);
	//UnMap_IOMemory(hDev);
}

int
HR_WriteRegister(
HR_DEV_CTX* hDev,
unsigned int index,
unsigned int		    offset,
const void*	    buffer,
unsigned int		    length
)
{
	if (!hDev || !buffer)
	{
		return ERROR_NULL_POINTER;
	}

	if (index >= MAX_BAR_NUMBER)
	{
		return 0;
	}

#if 0
	if (NULL == hDev->memory.bar[0].virAddress)
	{
		return HR_FAIL;
	}


	if (offset + length >= hDev->memory.bar[0].size)
		return -1;

	memcpy(hDev->memory.bar[0].virAddress + offset, buffer, length);

	return HR_SUCCESS;
#endif

	int status = 0;
	OVERLAPPED overlap;
	unsigned int data = 0;

	REGISTER_DATA_ITEM regData;
	ZeroMemory(&overlap, sizeof(OVERLAPPED));
	//ZeroMemory(&regData, sizeof(REGISTER_DATA_ITEM));

	regData.index = 0;
	regData.length = length;
	regData.offset = offset;
	memcpy(&regData.data, buffer, length);
	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}
		if (index == 0)
		{
			status = ::DeviceIoControl(
				hDev->device,
				IOCTL_IOCTL_WRITE_REGISTER0,
				&regData,
				sizeof(REGISTER_DATA_ITEM),
				NULL,
				0,
				NULL,
				&overlap
				);
		}
		else if (index == 1)
		{
			status = ::DeviceIoControl(
				hDev->device,
				IOCTL_IOCTL_WRITE_REGISTER1,
				&regData,
				sizeof(REGISTER_DATA_ITEM),
				NULL,
				0,
				NULL,
				&overlap
				);
		}


		if (status == 0)
		{
			int errorcode = GetLastError();
			::WaitForSingleObject(overlap.hEvent, INFINITE);
		}

		status = HR_SUCCESS;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
	}

	return HR_SUCCESS;
}

int
HR_ReadRegister(
HR_DEV_CTX* hDev,
unsigned int index,
unsigned int		    offset,
void*		    buffer,
unsigned int		    length
)
{
	if (!hDev || !buffer)
	{
		return ERROR_NULL_POINTER;
	}

	if (index >= MAX_BAR_NUMBER)
	{
		return 0;
	}

#if 0
	if (NULL == hDev->memory.bar[0].virAddress)
	{
		return RET_FAIL;
	}

	if (offset + length >= hDev->memory.bar[0].size)
		return RET_FAIL;

	memcpy(buffer, hDev->memory.bar[0].virAddress + offset, length);

	return HR_SUCCESS;
#endif

	int status = 0;
	OVERLAPPED overlap;
	unsigned int data = 0;

	REGISTER_DATA_ITEM regData;
	ZeroMemory(&overlap, sizeof(OVERLAPPED));
	ZeroMemory(&regData, sizeof(REGISTER_DATA_ITEM));

	regData.index = 0;
	regData.length = length;
	regData.offset = offset;

	do
	{
		overlap.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlap.hEvent)
		{
			break;
		}

		if (index == 0)
		{
			status = ::DeviceIoControl(
				hDev->device,
				IOCTL_IOCTL_READ_REGISTER0,
				&regData,
				sizeof(REGISTER_DATA_ITEM),
				&data,
				sizeof(unsigned int),
				NULL,
				&overlap
				);
		}
		else if (index == 1)
		{
			status = ::DeviceIoControl(
				hDev->device,
				IOCTL_IOCTL_READ_REGISTER1,
				&regData,
				sizeof(REGISTER_DATA_ITEM),
				&data,
				sizeof(unsigned int),
				NULL,
				&overlap
				);
		}
		else
		{
			return 1;
		}

		int errorcode = GetLastError();

		if (status == 0)
		{
			::WaitForSingleObject(overlap.hEvent, INFINITE);
		}

		memcpy(buffer, &data, length);
		status = HR_SUCCESS;
	} while (FALSE);

	if (overlap.hEvent)
	{
		::CloseHandle(overlap.hEvent);
		overlap.hEvent = NULL;
	}

	return status;
}

DWORD HR_WriteReg8(PVOID hDev, HR_UINT index, HR_UINT dwOffset, BYTE pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_WriteRegister(handle, index, dwOffset, &pData, 1);
	return status;
}

DWORD HR_WriteReg16(PVOID hDev, HR_UINT index, HR_UINT dwOffset, USHORT pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_WriteRegister(handle, index, dwOffset, &pData, 2);
	return status;
}

DWORD HR_WriteReg32(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT32 pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_WriteRegister(handle, index, dwOffset, &pData, 4);
	return status;
}

DWORD HR_WriteReg64(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT64 pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_WriteRegister(handle, index, dwOffset, &pData, 8);
	return status;
}

DWORD HR_ReadReg8(PVOID hDev, HR_UINT index, HR_UINT dwOffset, BYTE* pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_ReadRegister(handle, index, dwOffset, pData, 1);
	return status;
}

DWORD HR_ReadReg16(PVOID hDev, HR_UINT index, HR_UINT dwOffset, USHORT* pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_ReadRegister(handle, index, dwOffset, pData, 2);
	return status;
}
DWORD HR_ReadReg32(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT32* pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_ReadRegister(handle, index, dwOffset, pData, 4);
	return status;
}

DWORD HR_ReadReg64(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT64* pData)
{
	DWORD status;
	HR_DEV_CTX* handle;

	handle = (HR_DEV_CTX*)hDev;
	status = HR_ReadRegister(handle, index, dwOffset, pData, 8);
	return status;
}

DWORD HR_WriteBlock32(PVOID hDev, HR_UINT index, HR_UINT length, HR_UINT dwOffset, UINT32* pData)
{
	DWORD status;
	HR_DEV_CTX* handle;
	HR_UINT pData32;

	handle = (HR_DEV_CTX*)hDev;
	for (int i = 0; i < length / 4; i++)
	{
		pData32 = pData[i];
		status = HR_WriteRegister(handle, index, dwOffset, &pData32, 4);
		dwOffset += 4;
	}

	return status;
}

HR_DEV_CTX* HR_DeviceOpen(const char* name)
{
	HR_DEV_CTX* handle = NULL;

	do
	{
		if (name == NULL)
		{
			break;
		}

		handle = new HR_DEV_CTX;
		if (handle == NULL)
		{
			break;
		}

		ZeroMemory(handle, sizeof(HR_DEV_CTX));

		TCHAR szName[_MAX_PATH];
		ZeroMemory(szName, sizeof(szName));

#ifdef _UNICODE
		MultiByteToWideChar(
			CP_ACP,
			0,
			name,
			strlen(name) + 1,
			szName,
			_countof(szName)
			);
#else
		strcpy_s(szName, _countof(szName), name);
#endif // _UNICODE

		handle->device = ::CreateFile(
			szName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);

		if (!handle->device)
		{

			break;
		}

		TCHAR szSyncName[MAX_PATH];
		_stprintf_s(szSyncName, _countof(szSyncName), _T("Global\\WCORE-TX-SYNC-%s"), handle->name);

		/*
		if (HR_FAILED(Map_IOMemory(handle)))
		{
		break;
		}
		*/
		if (HR_FAILED(Map_TxBuffer(handle)))
		{
			break;
		}
		if (HR_FAILED(Map_RxBuffer(handle)))
		{
			break;
		}

		return handle;
	} while (FALSE);

	if (handle)
	{
		UnMap_Resouce(handle);

		if (handle->device)
		{
			::CloseHandle(handle->device);
		}
	}

	return NULL;


}

void HR_DeviceClose(HR_DEV_CTX* hDev)
{
	UnMap_IOMemory(hDev);
	if (hDev->intParam != NULL)
	{
		delete hDev->intParam;
		hDev->intParam = NULL;
	}
	if (hDev->selfThread != NULL)
	{

		hDev->selfIdThreadFlag = 0;
		Sleep(200);
	}

	UnMap_Resouce(hDev);
	CloseHandle(hDev->device);
	hDev->device = NULL;
	hDev = NULL;
	delete hDev;

}
unsigned long IntEnableDma(void* hDev)
{
	unsigned int pAddr = 0;
	int offset = 0;
	HR_DEV_CTX* handle = (HR_DEV_CTX*)hDev;
	int status = 0;
	unsigned int writeValue = 0;

	HR_WriteReg32(hDev, BAR1, DMA_WRITE_DEP_INT_REG, 0x4);
	Sleep(100);
	HR_WriteReg32(hDev, BAR1, 0x2010, 0x11);
	Sleep(100);

	HR_WriteReg32(hDev, BAR1, DMA_WRITE_DEP_ADDRH32_REG, handle->rxBuffer.phyAddressH);
	// printf("rxBuffer.phyAddressH>>>%x\r\n", handle->rxBuffer.phyAddressH);
	Sleep(100);
	// writeValue = handle->rxBuffer.phyAddress & 0xffffffff;
	HR_WriteReg32(hDev, BAR1, DMA_WRITE_DEP_ADDRL32_REG, handle->rxBuffer.phyAddress);
	// printf("rxBuffer.phyAddressH>>>%x handle->rxBuffer.phyAddress>>>%x\r\n", handle->rxBuffer.phyAddressH,handle->rxBuffer.phyAddress);
	Sleep(100);
	HR_WriteReg32(hDev, BAR1, DMA_WRITE_DEP_NUM_REG, 0x0);
	Sleep(100);
	//HR_WriteReg32(hDev, BAR1, DMA_WRITE_DEP_CTRL_REG, 0x5);
	HR_WriteReg32(hDev, BAR1, DMA_WRITE_DEP_CTRL_REG, 0x1);
	return 0;
}
unsigned long IntEnableDmaRead(void* hDev)
{
	unsigned int pAddr = 0;
	int offset = 0;
	HR_DEV_CTX* handle = (HR_DEV_CTX*)hDev;
	int status = 0;
	unsigned int writeValue = 0;

	HR_WriteReg32(hDev, BAR1, DMA_WRITE_DEP_INT_REG, 0x4);
	HR_WriteReg32(hDev, BAR1, 0x2010, 0x11);

	HR_WriteReg32(hDev, BAR1, 0x5084, handle->rxBuffer.phyAddressH);
	// printf("rxBuffer.phyAddressH>>>%x\r\n", handle->rxBuffer.phyAddressH);

	writeValue = handle->rxBuffer.phyAddress & 0xffffffff;
	HR_WriteReg32(hDev, BAR1, 0x5080, handle->rxBuffer.phyAddress);
	// printf("handle->rxBuffer.phyAddress>>>%x\r\n", handle->rxBuffer.phyAddress);

	HR_WriteReg32(hDev, BAR1, 0x5088, 0);

	HR_WriteReg32(hDev, BAR1, 0x1004, 5);

	return 0;
}
int RK_WriteDMA(void* hDev, unsigned long long dstAddr, char* pData, int length)
{
	if (length <= 0)
	{
		return 1;
	}
	HR_DEV_CTX* handle;
	int count = 0;
	handle = (HR_DEV_CTX*)hDev;
	unsigned int* writeDesc = NULL;
	unsigned int writeValue = 0;
	unsigned int dmaStatus = 0;
	writeDesc = (unsigned int*)handle->rxBuffer.virAddress;
	writeDesc[0] = 0xAD4B0113;
	writeDesc[1] = (length + 511) / 512 * 512;
	writeDesc[2] = handle->txBuffer.phyAddress;
	writeDesc[3] = handle->txBuffer.phyAddressH;
	//writeDesc[4] = dstAddr & 0xffffffff;
	//writeDesc[5] = (dstAddr >> 32) & 0xffffffff;
	memcpy(handle->txBuffer.virAddress, pData, length);
	IntEnableDma(hDev);
	while (1)
	{
		/* get status reg */
		HR_ReadReg32(handle, BAR1, 0x0044, &dmaStatus);
		// printf("dmaStatus:%x \n", dmaStatus);
		if (0 == (dmaStatus & 1))
		{
			printf("<HR_WriteDMA> DEP_STAUTS = 0x%x\n", dmaStatus);
			HR_WriteReg32(handle, BAR1, 0x0044, dmaStatus);
			break;
		}
		Sleep(100);
		count++;
		if (count > 10000)
		{
			return ERROR_DMA_TIME_OUT;
		}
	}
	/* stop dma ctrl */
	HR_WriteReg32(handle, BAR1, DMA_WRITE_DEP_CTRL_REG, 0);
	//memset(handle->txBuffer.virAddress, 0, length);
	return 0;
}
int RK_ReadDMA(void* hDev, unsigned long long dstAddr, char* pData, int length)
{
	if (length <= 0)
	{
		return 1;
	}
	HR_DEV_CTX* handle = (HR_DEV_CTX*)hDev;
	unsigned int* writeDesc = NULL;
	unsigned int writeValue = 0;
	unsigned int dmaStatus = 0;
	unsigned int nvmeStatus = 0;
	writeDesc = (unsigned int*)handle->rxBuffer.virAddress;
	writeDesc[0] = 0xAD4B0113;
	writeDesc[1] = (length + 511) / 512 * 512;
	writeDesc[2] = dstAddr & 0xffffffff;
    writeDesc[3] = (dstAddr >> 32) & 0xffffffff;
	writeDesc[4] = handle->txBuffer.phyAddress;
	writeDesc[5] = handle->txBuffer.phyAddressH;
	//memset(handle->txBuffer.virAddress, 0, length);
	IntEnableDmaRead(hDev);
	// RK_WriteReg32(handle, RK_ADDR_PCI_BAR1, 0x1004, 5);
	int count = 0;
#if 1
	while (1)
	{
		HR_ReadReg32(handle, BAR1, 0x1044, &dmaStatus);
		// printf("dmaStatus:%x \n", dmaStatus);
		if (0 == (dmaStatus & 1))
		{
			printf("<RK_ReadDMA> DEP_STAUTS = 0x%x\n", dmaStatus); ////////////////
			HR_WriteReg32(handle, BAR0, 0x1044, dmaStatus);
			break;
		}
		Sleep(100);
		count++;
		if (count > 10000)
		{
			return ERROR_DMA_TIME_OUT;
		}
	}
	/* stop dma ctrl */

	HR_WriteReg32(handle, BAR1, 0x1004, 0);
#endif

	memcpy(pData, handle->txBuffer.virAddress, length);
	return 0;
}

#endif


