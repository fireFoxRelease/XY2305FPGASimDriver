#ifndef __DEVICE_H_
#define __DEVICE_H_
/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include "Public.h"
#include "Reg9656.h"

#if 0
typedef struct _bar_descriptor
{
	unsigned char*				vir_address;
	unsigned long       phy_address;
	unsigned long		size;
	unsigned char				flag;
}bar_descriptor, *pbar_descriptor;

typedef struct _IO_MEMORY
{
	bar_descriptor bar[6];
}IO_MEMORY, *pIO_MEMORY;


typedef struct _DMA_BUFFER_ITEM
{
	void*		        vir_address;
	unsigned long		length;
	unsigned long		phy_address;
	unsigned long		bus_address;
}DMA_BUFFER_ITEM, *ptr_buffer_item;

typedef struct _register_data
{
	unsigned int		index;
	unsigned int		data;
	unsigned int		offset;
	unsigned int		length;
}REGISTER_DATA_ITEM, *pREGISTER_DATA_ITEM;
#endif
//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    ULONG PrivateDeviceData; // just a placeholder
    WDFDEVICE Device;
    ULONG Counter_i; // counter for WdfCmResourceListGetCount(ResourceListTranslated)
    union
    {
        INT_CSR bits;
        ULONG ulong;
    } IntCsr;

    union
    {
        DMA_CSR bits;
        UCHAR uchar;
    } Dma0Csr;

    union
    {
        DMA_CSR bits;
        UCHAR uchar;
    } Dma1Csr;

    ULONG OffsetAddressFromApp; // get offset address that is given by application

    // HW Resources
    PPCI9656_REGS Regs; // Registers address
    PUCHAR RegsBase;    // Registers base address
    ULONG RegsLength;   // Registers base length

    PVOID Bar0VirtualAddress;  // Bar0 start virtual address
    ULONG Bar0PhysicalAddress; // store the Bar0 start physical addrSess
    ULONG Bar0Length;
    PVOID Bar1VirtualAddress;  // Bar0 start virtual address
    ULONG Bar1PhysicalAddress; // store the Bar0 start physical address
    ULONG Bar1Length;
    PMDL pMdl0;
    PMDL pMdl1;
    IO_MEMORY Register;

    PDMA_ADAPTER DmaAdapterObject;
    PALLOCATE_COMMON_BUFFER AllocateCommonBuffer;
    PFREE_COMMON_BUFFER FreeCommonBuffer;

    PVOID TxDMABuffer;
    PHYSICAL_ADDRESS TxDMABusAddr;
    ULONG TxBufferSize;
    PMDL TxBufMdl;

    PVOID RxDMABuffer;
    PHYSICAL_ADDRESS RxDMABusAddr;
    ULONG RxBufferSize;
    PMDL RxBufMdl;

    PDEVICE_OBJECT PhysicalDeviceObject;
    WDFIOTARGET StackIoTarget;
    PDEVICE_OBJECT StackDeviceObject;

    WDFINTERRUPT Interrupt;

    ULONG BusNumber;
    USHORT FunctionNumber;
    USHORT DeviceNumber;

    // for interrupt
    PRKEVENT pEvent;
    unsigned int intCount;

    WDFDMAENABLER DmaEnabler;
    ULONG MaximumTransferLength;
    ULONG WriteTransferElements;
    WDFCOMMONBUFFER WriteCommonBuffer;
    size_t WriteCommonBufferSize;
    _Field_size_(WriteCommonBufferSize) PUCHAR WriteCommonBufferBase;
    PHYSICAL_ADDRESS WriteCommonBufferBaseLA; // Logical Address

    // Read
    ULONG ReadTransferElements;
    WDFCOMMONBUFFER ReadCommonBuffer;
    size_t ReadCommonBufferSize;
    _Field_size_(ReadCommonBufferSize) PUCHAR ReadCommonBufferBase;
    PHYSICAL_ADDRESS ReadCommonBufferBaseLA; // Logical Address

    PMDL WriteMdl;
    PMDL ReadMdl;

    WDFREQUEST Request;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

typedef struct _WORKITEM
{

    WDFREQUEST Request;     // complete this Request when done
    PDEVICE_CONTEXT DevExt; // device extension
    PVOID Buffer;
} WORKITEM, *PWORKITEM;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WORKITEM, GetWorkItem)

//
// Constants for the MDL chains we use to simulate memory fragmentation.
//
#define MDL_CHAIN_LENGTH 8
// #define MDL_BUFFER_SIZE  ((PCI9656_MAXIMUM_TRANSFER_LENGTH) / MDL_CHAIN_LENGTH)
//
//  WDFDRIVER Events including "EVT"
//
DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD MasterCardEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP MasterCardEvtDriverContextCleanup;
EVT_WDF_OBJECT_CONTEXT_CLEANUP MasterCardEvtDeviceCleanup;

EVT_WDF_DEVICE_D0_ENTRY MasterCardEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT MasterCardEvtDeviceD0Exit;
EVT_WDF_DEVICE_PREPARE_HARDWARE MasterCardEvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE MasterCardEvtDeviceReleaseHardware;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL MasterCardPCIeEvtIoDeviceControl;

EVT_WDF_INTERRUPT_ISR MasterCardEvtInterruptIsr;
EVT_WDF_INTERRUPT_DPC MasterCardEvtInterruptDpc;
EVT_WDF_INTERRUPT_ENABLE MasterCardEvtInterruptEnable;
EVT_WDF_INTERRUPT_DISABLE MasterCardEvtInterruptDisable;

EVT_WDF_WORKITEM MasterCardMapReadBuffer;
//
// Function to initialize the device and its callbacks
//
NTSTATUS
MasterCardCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit);

NTSTATUS
MasterCardInitializeDMA(
    IN PDEVICE_CONTEXT DevExt);

NTSTATUS
AllocDMABuffer(
    __in PDEVICE_CONTEXT FdoData);

VOID FreeDMABuffer(__in PDEVICE_CONTEXT FdoData);

NTSTATUS
MasterCardInterruptCreate(
    IN PDEVICE_CONTEXT DevExt);

BOOLEAN
MasterCardEvtInterruptIsr(
    _In_ WDFINTERRUPT Interrupt,
    _In_ ULONG MessageID);

VOID MasterCardEvtInterruptDpc(
    _In_ WDFINTERRUPT WdfInterrupt,
    _In_ WDFOBJECT WdfDevice);

VOID MasterCardCleanupDeviceExtension(
    _In_ PDEVICE_CONTEXT DevExt);

BOOLEAN
PciDrvReadRegistryValue(
    _In_ PDEVICE_CONTEXT FdoData,
    _In_ PWSTR Name,
    _Out_ PULONG Value);

#endif