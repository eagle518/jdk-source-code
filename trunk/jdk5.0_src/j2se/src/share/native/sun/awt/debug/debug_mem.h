/*
 * @(#)debug_mem.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Debug Memory Manager
 * 
 * - inits allocated memory to predefined byte to expose uninitialized variables
 * - fills freed memory with predefined byte to expose dangling pointers
 * - catches under/overwrites with 'guard' bytes around allocated blocks
 * - tags blocks with the file name and line number where they were allocated
 * - reports unfreed blocks to help find memory leaks
 *
 */

#if !defined(_DEBUGMEM_H)
#define _DEBUGMEM_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(DEBUG)

#include "debug_util.h"

/* prototype for allocation callback function */
typedef void * (*DMEM_ALLOCFN)(size_t size);

/* prototype for deallocation callback function */
typedef void (*DMEM_FREEFN)(void * pointer);

/* prototype for pointer validation function */
typedef dbool_t (*DMEM_CHECKPTRFN)(void * ptr, size_t size);

/* Debug memory manager global state */
/* DO NOT REFERENCE this structure in code, it is only exported */
/* to ease it's use inside a source level debugger */
typedef struct DMemState {
    DMEM_ALLOCFN	pfnAlloc;	/* block allocate callback */
    DMEM_FREEFN		pfnFree;	/* block free callback */
    DMEM_CHECKPTRFN	pfnCheckPtr;	/* pointer validation callback */
    size_t		biggestBlock;	/* largest block allocated so far */
    size_t		maxHeap;	/* maximum size of the debug heap */
    size_t		totalHeapUsed;	/* total memory allocated so far */
    dbool_t		failNextAlloc;	/* whether the next allocation fails (automatically resets)*/
    int			totalAllocs;	/* total number of allocations so far */
} DMemState;

/* Exported global var so you can view/change settings in the debugger */
extern const DMemState	* DMemStatePtr;

/* General memory manager functions */
extern void DMem_Initialize();
extern void DMem_Shutdown();
extern void * DMem_AllocateBlock(size_t size, const char * filename, int linenumber);
extern void DMem_FreeBlock(void *ptr);
extern void DMem_ReportLeaks();

/* Routines to customize behaviour with callbacks */
extern void DMem_SetAllocCallback( DMEM_ALLOCFN pfn );
extern void DMem_SetFreeCallback( DMEM_FREEFN pfn );
extern void DMem_SetCheckPtrCallback( DMEM_CHECKPTRFN pfn );
extern void DMem_DisableMutex();

#endif /* defined(DEBUG) */

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* _DEBUGMEM_H */
