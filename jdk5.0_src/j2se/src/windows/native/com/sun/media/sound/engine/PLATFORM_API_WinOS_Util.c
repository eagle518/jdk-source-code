/*
 * @(#)PLATFORM_API_WinOS_Util.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_WinOS_Util.c
**
**	Overview:
**	Utility function for Windows specific code.
**
**	History	-
**	2002-04-04	$$fb Created
**      2003-01-31      $$fb clean-up
*/
/*****************************************************************************/

#define USE_ERROR
//#define USE_TRACE

#include "PLATFORM_API_WinOS_Util.h"

#if (USE_PLATFORM_MIDI_IN == TRUE) || (USE_PLATFORM_MIDI_OUT == TRUE)

/* set the startTime field in MidiDeviceHandle */
void MIDI_SetStartTime(MidiDeviceHandle* handle) {
    if (handle != NULL) {
		handle->startTime = (INT64) timeGetTime();
    }
}


/* return time stamp in microseconds */
INT64 MIDI_GetTimeStamp(MidiDeviceHandle* handle) {
    INT64 res;
    if (handle == NULL) {
		return (INT64) -1;
    }
    res = ((INT64) timeGetTime()) - handle->startTime;
    if (res < 0) {
		res *= (INT64) -1000;
    } else {
		res *= (INT64) 1000;
    }    
    return res;
}


void* MIDI_CreateLock() {
    CRITICAL_SECTION* lock = (CRITICAL_SECTION*) malloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(lock);
    TRACE0("MIDI_CreateLock\n");
    return lock;
}

void MIDI_DestroyLock(void* lock) {
    if (lock) {
	DeleteCriticalSection((CRITICAL_SECTION*) lock);
	free(lock);
	TRACE0("MIDI_DestroyLock\n");
    }
}

void MIDI_Lock(void* lock) {
    if (lock) {
	EnterCriticalSection((CRITICAL_SECTION*) lock);
    }
}

void MIDI_Unlock(void* lock) {
    if (lock) {
	LeaveCriticalSection((CRITICAL_SECTION*) lock);
    }
}
int MIDI_WinCreateEmptyLongBufferQueue(MidiDeviceHandle* handle, int count) {
    return MIDI_WinCreateLongBufferQueue(handle, count, 0, NULL);
}

int MIDI_WinCreateLongBufferQueue(MidiDeviceHandle* handle, int count, int size, UBYTE* preAllocatedMem) {
    SysExQueue* sysex;
    int i;
    UBYTE* dataPtr;
    int structSize = sizeof(SysExQueue) + ((count - 1) * sizeof(MIDIHDR));

    sysex = (SysExQueue*) malloc(structSize);
    if (!sysex) return FALSE;
    memset(sysex, 0, structSize);
    sysex->count = count;
    sysex->size = size;

    // prepare memory block which will contain the actual data
    if (!preAllocatedMem && size > 0) {
	preAllocatedMem = (UBYTE*) malloc(count*size);
	if (!preAllocatedMem) {
	    free(sysex);
	    return FALSE;
	}
	sysex->ownsLinearMem = 1;
    }
    sysex->linearMem = preAllocatedMem;
    handle->longBuffers = sysex;

    // set up headers
    dataPtr = preAllocatedMem;
    for (i=0; i<count; i++) {
	sysex->header[i].lpData = dataPtr;
	sysex->header[i].dwBufferLength = size;
	// user data is the index of the buffer
	sysex->header[i].dwUser = (DWORD) i;
	dataPtr += size;
    }
    return TRUE;
}

void MIDI_WinDestroyLongBufferQueue(MidiDeviceHandle* handle) {
    SysExQueue* sysex = (SysExQueue*) handle->longBuffers;
    if (sysex) {
	handle->longBuffers = NULL;
	if (sysex->ownsLinearMem && sysex->linearMem) {
	    free(sysex->linearMem);
	}
	free(sysex);
    }
}

#endif // USE_PLATFORM_MIDI_IN || USE_PLATFORM_MIDI_OUT
