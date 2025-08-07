/**********************************************************
*�ļ��� HiDrv.h
*������ ʵ�ֺ��豸ͨ��
*���ߣ� weihua.zhao	
*���ڣ� 2018-08-27
***********************************************************/
#ifndef _HIDRV_H
#define  _HIDRV_H

#include <map>
#include "HiXY2305DriverPublic.h"
#include "Public.h"

#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport) //extern "C"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAXSLOT 8
#define HR_ADDR_PCI_BAR0  0
#define HR_ADDR_PCI_BAR1  1
#define HR_ADDR_PCI_BAR2  2
#define HR_ADDR_PCI_BAR3  3
#define HR_ADDR_PCI_BAR4  4
#define HR_ADDR_PCI_BAR5  5
#define MAX_BAR_NUMBER 6

#define HR_SUCCEEDED(hr)				(((int)(hr)) >= 0)
#define HR_FAILED(hr)					(((int)(hr)) < 0)


typedef std::map<HR_DEVICE, int> CARD_MAP;
typedef std::map<unsigned int, unsigned int> MSG_MAP;


#define ONE_BYTE            1
#define TWO_BYTES           2
#define FOUR_BYTES          4

/*************************************************************
  General definitions
 *************************************************************/

typedef enum {
	HR_MODE_8_BIT = 1,
	HR_MODE_16_BIT = 2,
	HR_MODE_32_BIT = 4,
	HR_MODE_64_BIT = 8
} HR_ADDR_MODE;


#define MAX_PACKET 1024


#define CHANNELMAXNUM 1024
#define MAX_PATH_LENGTH     60
typedef struct _maped_bar_descriptor
{
    void*               raw_address;
    unsigned long       raw_size;
    unsigned char*      vir_address;
    unsigned long       phy_address;
    unsigned long       size;
}maped_bar_descriptor, *pmaped_bar_descriptor;


typedef struct _mMaster_device {  
    int                     fd;
    char                    name[MAX_PATH_LENGTH];
	IO_MEMORY			memory;
	DMA_BUFFER_ITEM     txBuffer;
	DMA_BUFFER_ITEM     rxBuffer;
    PVOID                   intParam;
	PCIE_MASTER_INT_HANDLER           funcDiagIntHandler;
    bool                    intFlag;
	
	unsigned int             channelMaxNum;//yyj
	int                     nodeMode[CHANNELMAXNUM];
	int                     RNPacNum[CHANNELMAXNUM];
	unsigned int            tableWritePos[CHANNELMAXNUM];
	unsigned int            recvVPCErrCnt[CHANNELMAXNUM];
	unsigned int            recvSTOFVPCErrCnt[CHANNELMAXNUM];
	unsigned int            rstCnt[CHANNELMAXNUM];
	unsigned int            intMask[CHANNELMAXNUM];
	int                     selfIDCount[CHANNELMAXNUM];
	/* data legnth in buffer */
	DWORD                   dataLength[CHANNELMAXNUM];
	/* write position in buffer */
	DWORD                   writePos[CHANNELMAXNUM];
	/* read position in buffer */
	DWORD                   readPos[CHANNELMAXNUM];
	DWORD                   fullFlag[CHANNELMAXNUM];
	HR_UINT                 selfIdPac[CHANNELMAXNUM][256];
	int                     stofVpcEn[CHANNELMAXNUM];
	int                     selfIDFlag[CHANNELMAXNUM];
	unsigned int            dmaCountNow[CHANNELMAXNUM];
	unsigned int            dmaCountLast[CHANNELMAXNUM];
	unsigned int            retFlag;//yyj 0714 add
	HANDLE			    	device;
	HANDLE                  hEvent;
	HR_UINT                 threadFlag;
	int                     boardNum;
	int                     selfIdThreadFlag;
	HANDLE                  selfThread;
	LPDWORD                 selfThreadId;
	HANDLE                  selfMutex;
	HANDLE                  resetMutex;
	HANDLE                  hIntThread;
	int						intEnFlag;


}HR_DEV_CTX,*PHR_DEV_CTX;

/*---------------------------------
  Function defination
-----------------------------------*/

/*Register function*/
DWORD HR_ReadReg8(PVOID hDev, HR_UINT index, HR_UINT dwOffset, BYTE* pData);
DWORD HR_ReadReg16(PVOID hDev, HR_UINT index, HR_UINT dwOffset, USHORT* pData);
DWORD HR_ReadReg32(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT32* pData);
DWORD HR_ReadReg64(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT64* pData);

DWORD HR_WriteReg8(PVOID hDev, HR_UINT index, HR_UINT dwOffset, BYTE pData);
DWORD HR_WriteReg16(PVOID hDev, HR_UINT index, HR_UINT dwOffset, USHORT pData);
DWORD HR_WriteReg32(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT32 pData);
DWORD HR_WriteReg64(PVOID hDev, HR_UINT index, HR_UINT dwOffset, UINT64 pData);



/*************************************************************
  Function prototypes
 *************************************************************/
/* -----------------------------------------------
    Device open/close
   ----------------------------------------------- */

DLL_EXPORT
HR_DEV_CTX*
HR_DeviceOpen(
const char* name
);


DLL_EXPORT
void
HR_DeviceClose(
HR_DEV_CTX* hDev
);


/* -----------------------------------------------
    Interrupts
   ----------------------------------------------- */
DLL_EXPORT DWORD HR_IntEnable(HR_DEVICE hDev, PCIE_MASTER_INT_HANDLER funcIntHandler,
    PVOID pData);
DLL_EXPORT DWORD HR_IntDisable(HR_DEVICE hDev);


DLL_EXPORT int HR_EnumDevice(PMaster_DEVICE_IDENTIFIER* idList);

DLL_EXPORT
void
HR_FreeIdentifier(
PMaster_DEVICE_IDENTIFIER idList
);

int RK_WriteDMA(void* hDev, unsigned long long dstAddr, char* pData, int length);
int RK_ReadDMA(void* hDev, unsigned long long dstAddr, char* pData, int length);
DLL_EXPORT
int
HR_GetLocation(
HR_DEV_CTX*      hDev,
PPCI_DEVICE_LOCATION location
);

DLL_EXPORT
int
HR_WriteRegister(
HR_DEV_CTX* hDev,
unsigned int index,
unsigned int		    offset,
const void*	    buffer,
unsigned int		    length
);

DLL_EXPORT
int
HR_ReadRegister(
HR_DEV_CTX* hDev,
unsigned int index,
unsigned int		    offset,
void*		    buffer,
unsigned int		    length
);


#ifdef __cplusplus
}
#endif
#endif