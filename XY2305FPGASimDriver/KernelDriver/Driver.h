#ifndef __DRIVER_H_
#define __DRIVER_H_

/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#define INITGUID

#include <ntddk.h>
#include <wdf.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

//
// WDFDRIVER Events
//
#define _DRIVER_NAME_ "RDKPV6NVME"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD MasterCardEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP MasterCardEvtDriverContextCleanup;

#endif