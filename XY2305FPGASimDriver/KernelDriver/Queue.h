#ifndef __QUEUE_H_
#define __QUEUE_H_
/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    Kernel-mode Driver Framework

--*/

//
// This is the context that can be placed per queue
// and would contain per queue information.
//

typedef struct _QUEUE_CONTEXT
{

  ULONG PrivateDeviceData; // just a placeholder

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

typedef struct _pci_device_location
{
  unsigned int bus;
  unsigned short function;
  unsigned short device;
} pci_device_location, *ppci_device_location;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
MasterCardQueueInitialize(
    _In_ WDFDEVICE hDevice);

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL MasterCardEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_STOP MasterCardEvtIoStop;

#endif