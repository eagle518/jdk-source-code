/*
 * Copyright (c) 2004, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */


#ifdef AMD64
typedef unsigned char UBYTE;

#define UNW_FLAG_EHANDLER  0x01
#define UNW_FLAG_UHANDLER  0x02
#define UNW_FLAG_CHAININFO 0x04

// This structure is used to define an UNWIND_INFO that
// only has an ExceptionHandler.  There are no UnwindCodes
// declared.
typedef struct _UNWIND_INFO_EH_ONLY {
    UBYTE Version       : 3;
    UBYTE Flags         : 5;
    UBYTE SizeOfProlog;
    UBYTE CountOfCodes;
    UBYTE FrameRegister : 4;
    UBYTE FrameOffset   : 4;
    union {
       OPTIONAL ULONG ExceptionHandler;
       OPTIONAL ULONG FunctionEntry;
    };
    OPTIONAL ULONG ExceptionData[1];
} UNWIND_INFO_EH_ONLY, *PUNWIND_INFO_EH_ONLY;


/*
typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;
*/

typedef struct _DISPATCHER_CONTEXT {
    ULONG64 ControlPc;
    ULONG64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG64 EstablisherFrame;
    ULONG64 TargetIp;
    PCONTEXT ContextRecord;
//    PEXCEPTION_ROUTINE LanguageHandler;
    char * LanguageHandler; // double dependency problem
    PVOID HandlerData;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;

#if MSC_VER < 1500

/* Not needed for VS2008 compiler, comes from winnt.h. */
typedef EXCEPTION_DISPOSITION (*PEXCEPTION_ROUTINE) (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONG64 EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
);

#endif

#endif // AMD64
