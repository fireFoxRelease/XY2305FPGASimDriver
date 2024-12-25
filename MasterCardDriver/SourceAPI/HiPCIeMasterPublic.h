/**
 *
 *  PCIe Master Linux 64BIT DRIVER  Version 1.0  (16/01/2020)
 *  Copyright (c) 1998-2020
 *  support@hiraintech.com
 *  ALL RIGHTS RESERVED
 *
 **/

/**
 *
 *  Include file for PCIe Master Library
 *  function prototypes, defines of type,
 *  return codes, flag, struct
 *
 **/


/*right define*/
#define HR_SUCCESS 0  


#ifndef _HIPCIE_MASTER_PUBLIC_H
#define _HIPCIE_MASTER_PUBLIC_H

#ifdef WIN32
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
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
typedef void * HR_DEVICE;
typedef BOOL HR_BOOL;
#endif

#ifdef __linux
typedef void* HR_DEVICE;
typedef bool HR_BOOL;
typedef bool BOOL;
typedef void* PVOID;
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

/*Interrupt function type*/
typedef void
(*PCIE_MASTER_INT_HANDLER)(INT_PARA intPara);



/************************************************************************/
/*            return value define                                       */
/************************************************************************/


#define RET_SUCCESS                             0x00000000
#define RET_FAIL								0x00000001
#define ERROR_ALREADY_OPEN                      0x00000002
#define ERROR_OPEN_BOARD                        0x00000003
#define ERROR_INVALID_PARAM                     0x00000004
#define ERROR_NULL_POINTER                      0x00000005
#define ERROR_ALREADY_CLOSE                     0x00000006


DLL_EXPORT HR_ULONG hiMasterOpenCard(HR_DEVICE *pDeviceHandle, DWORD nCardNum);

DLL_EXPORT HR_ULONG hiMasterCloseCard(HR_DEVICE hDeviceHandle);



}
;
#endif
