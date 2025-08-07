#define HR_SUCCESS 0

#ifndef _HIPCIE_MASTER_PUBLIC_H
#define _HIPCIE_MASTER_PUBLIC_H

#ifdef WIN32
#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllexport)
#endif
#else
#ifndef HR_DLL_EXPORT
#define HR_DLL_EXPORT
#endif
#endif

extern "C"
{

#ifdef WIN32
#include <stdio.h>
#include <Windows.h>
    typedef void *HR_DEVICE;
    typedef BOOL HR_BOOL;
#endif

#ifdef __linux
    typedef void *HR_DEVICE;
    typedef bool HR_BOOL;
    typedef bool BOOL;
    typedef void *PVOID;
    typedef unsigned long DWORD;
    typedef unsigned short WORD;
    typedef unsigned char BYTE;
    typedef unsigned short USHORT;
    typedef unsigned int UINT32;
    typedef unsigned long UINT64;
#define __int64 long long
#endif

    typedef unsigned int HR_UINT;
    typedef unsigned char HR_UCHAR;
    typedef unsigned char HR_USHORT;
    typedef unsigned long HR_RET;
    typedef unsigned long HR_ULONG;

    typedef int HR_INT;
    typedef long HR_LONG;
    typedef unsigned char HR_BYTE;

    typedef unsigned int HR_32BIT;
    typedef unsigned short HR_16BIT;
    typedef unsigned char HR_8BIT;

    typedef struct
    {
        HR_DEVICE pDev;
        HR_UINT chn;
        PVOID pUserData; /*user define*/
    } INT_PARA;

    typedef void (*PCIE_MASTER_INT_HANDLER)(INT_PARA intPara);

    /*Interrupt function type*/

    /************************************************************************/
    /*            return value define                                       */
    /************************************************************************/

#define RET_SUCCESS 0x00000000
#define RET_FAIL 0x00000001
#define ERROR_ALREADY_OPEN 0x00000002
#define ERROR_OPEN_BOARD 0x00000003
#define ERROR_INVALID_PARAM 0x00000004
#define ERROR_NULL_POINTER 0x00000005
#define ERROR_ALREADY_CLOSE 0x00000006
#define ERROR_DMA_TIME_OUT 0x00000007
    typedef struct
    {
        unsigned int cardF1Type;
        unsigned int cardF2Type;
        unsigned int hardwareVer;
        unsigned int f1LogicVer;
        unsigned int f2LogicVer;
    } TY_CARD_INFO;

    DLL_EXPORT HR_ULONG hiMasterOpenCard(HR_DEVICE *pDeviceHandle, DWORD nCardNum);

    DLL_EXPORT HR_ULONG hiMasterCloseCard(HR_DEVICE hDeviceHandle);

    DLL_EXPORT HR_ULONG hiMasterResetCard(HR_DEVICE hDeviceHandle, unsigned int cardNo);

    DLL_EXPORT HR_ULONG hiMasterGetCardVersion(HR_DEVICE hDeviceHandle, TY_CARD_INFO *pCardInfo);

    DLL_EXPORT HR_ULONG hiWriteDMA(HR_DEVICE hDeviceHandle, char *pData, unsigned int length);

    DLL_EXPORT HR_ULONG hiReadDMA(HR_DEVICE hDeviceHandle, char *pData, unsigned int length);

    DLL_EXPORT HR_ULONG hiLoadBit(HR_DEVICE hDeviceHandle, const char* fileName);

    DLL_EXPORT HR_ULONG hiLoadFlash(HR_DEVICE hDeviceHandle, const char* fileName);
    DLL_EXPORT HR_ULONG hiReadFlash(HR_DEVICE pHandle, unsigned char* flashData, unsigned int length);


    DLL_EXPORT HR_ULONG hiWritePulseStep(HR_DEVICE hDeviceHandle, unsigned int value);

    DLL_EXPORT HR_ULONG hiRegUpLoadEnable(HR_DEVICE hDeviceHandle, unsigned int enableFlag);

    DLL_EXPORT HR_ULONG hiIOChnEnable(HR_DEVICE hDeviceHandle, unsigned int chnNum,unsigned int enableFlag);

    DLL_EXPORT HR_ULONG hiIOChnMon(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int* pMonitor);

    DLL_EXPORT HR_ULONG hiIOChnDir(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int dir);

    DLL_EXPORT HR_ULONG hiIOChnOut(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int outFlag);

    DLL_EXPORT HR_ULONG hiIOChnSetInSamp(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int samp);

    DLL_EXPORT HR_ULONG hiIOChnGetInData(HR_DEVICE hDeviceHandle, unsigned int chnNum, unsigned int* data);
    
    DLL_EXPORT HR_ULONG hiAnalogOutEnable(HR_DEVICE hDeviceHandle, unsigned int chn,unsigned int enableFlag);
    
    DLL_EXPORT HR_ULONG hiAnalogOutSel(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int selFlag);
    
    DLL_EXPORT HR_ULONG hiAnalogOutData(HR_DEVICE hDeviceHandle, unsigned int chn, float data);

    DLL_EXPORT HR_ULONG hiAnalogOutExcSel(HR_DEVICE hDeviceHandle, unsigned int chn, int sel);

    DLL_EXPORT HR_ULONG hiAnalogOutVpp(HR_DEVICE hDeviceHandle, unsigned int chn, int vpp);

    DLL_EXPORT HR_ULONG hiAnalogOutFreq(HR_DEVICE hDeviceHandle, unsigned int chn, int freq);

    
    DLL_EXPORT HR_ULONG hiAnalogAjustCalEnable(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int enableFlag);
 
    DLL_EXPORT HR_ULONG hiAnalogAjustCalData(HR_DEVICE hDeviceHandle, unsigned int chn, float k,short b);
    
    DLL_EXPORT HR_ULONG hiAnalogInEnable(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int enableFlag);

    DLL_EXPORT HR_ULONG hiAnalogInData(HR_DEVICE hDeviceHandle, unsigned int chn, float* data);

    DLL_EXPORT HR_ULONG hiAnalogAjustCalInData(HR_DEVICE hDeviceHandle, unsigned int chn, int k,int b);

    DLL_EXPORT HR_ULONG hiHallEnable(HR_DEVICE hDeviceHandle,unsigned int enableFlag);

    DLL_EXPORT HR_ULONG hiHallStatus(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int* status);
   
    DLL_EXPORT HR_ULONG hiHallTime(HR_DEVICE hDeviceHandle,unsigned int* time);
   
    DLL_EXPORT HR_ULONG hiIncodeEnable(HR_DEVICE hDeviceHandle, unsigned int enableFlag);
    
    DLL_EXPORT HR_ULONG hiIncodeLines(HR_DEVICE hDeviceHandle, unsigned int lines);
    
    DLL_EXPORT HR_ULONG hiIncodeAngle(HR_DEVICE hDeviceHandle, unsigned int* angle);

    DLL_EXPORT HR_ULONG hiIncodeSpeed(HR_DEVICE hDeviceHandle, unsigned int* speed);

    DLL_EXPORT HR_ULONG hiIncodeHigh(HR_DEVICE hDeviceHandle, unsigned int* high);

    DLL_EXPORT HR_ULONG hiIncodeLow(HR_DEVICE hDeviceHandle, unsigned int* low);

    DLL_EXPORT HR_ULONG hiIncodeZHigh(HR_DEVICE hDeviceHandle, unsigned int* high);

    DLL_EXPORT HR_ULONG hiIncodeZLow(HR_DEVICE hDeviceHandle, unsigned int* low);

    DLL_EXPORT HR_ULONG hiIncodeSetTime(HR_DEVICE hDeviceHandle, unsigned int time);

    DLL_EXPORT HR_ULONG hiIncodeAB(HR_DEVICE hDeviceHandle, unsigned int* value);

    DLL_EXPORT HR_ULONG hiPWMEnable(HR_DEVICE hDeviceHandle, unsigned int chn,unsigned int enableFlag);

    DLL_EXPORT HR_ULONG hiPWMSetSpeed(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int speed);

    DLL_EXPORT HR_ULONG hiPWMLow(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int low);

    DLL_EXPORT HR_ULONG hiPWMHigh(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int high);

    DLL_EXPORT HR_ULONG hiPWMDied(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int died);

    DLL_EXPORT HR_ULONG hiPWMSel(HR_DEVICE hDeviceHandle, unsigned int chn, unsigned int sel);

    DLL_EXPORT HR_ULONG hiGetFlashId(HR_DEVICE hDeviceHandle,unsigned int* id);
};
#endif
