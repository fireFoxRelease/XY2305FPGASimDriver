/*++

Module Name:

    Trace.h

Abstract:

    Header file for the debug tracing related function defintions and macros.

Environment:

    Kernel mode

--*/

//
// Define the tracing flags.
//
// Tracing GUID - 75d92351-633c-4de6-8a28-f61110868a43
//

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        MasterCardTraceGuid, (75d92351,633c,4de6,8a28,f61110868a43),\
                                                                            \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                              \
        WPP_DEFINE_BIT(TRACE_DRIVER)                                   \
        WPP_DEFINE_BIT(TRACE_DEVICE)                                   \
        WPP_DEFINE_BIT(TRACE_QUEUE)                                    \
        )                             

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

// Define debug levels
//
#define     NONE      0 //  Tracing is not on
#define     FATAL      1 // Abnormal exit or termination
#define     ERROR      2 // Severe errors that need logging
#define     WARNING    3  // Warnings such as allocation failure
#define     INFO       4  // Includes non-error cases such as Entry-Exit
#define     TRACE2      5 // Detailed traces from intermediate steps
#define     LOUD       6  // Detailed trace from every step

//
// Define Debug Flags
//
#define DBG_INIT             0x00000001
#define DBG_PNP    0x00000002
#define DBG_POWER              0x00000004
#define DBG_WMI            0x00000008
#define DBG_CREATE_CLOSE              0x00000010
#define DBG_IOCTLS             0x00000020
#define DBG_WRITE            0x00000040
#define DBG_READ        0x00000080
#define DBG_DPC              0x00000100
#define DBG_INTERRUPT           0x00000200
#define DBG_LOCKS            0x00000400
#define DBG_QUEUEING         0x00000800
#define DBG_HW_ACCESS        0x00001000

VOID
DebugPrint(
__in ULONG   DebugPrintLevel,
__in PCCHAR  DebugMessage,
...
);
//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
//
