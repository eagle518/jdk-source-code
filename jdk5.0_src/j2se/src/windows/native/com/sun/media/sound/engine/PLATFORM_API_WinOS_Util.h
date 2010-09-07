/*
 * @(#)PLATFORM_API_WinOS_Util.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>

/* for waveformat extensible */
#include <mmreg.h>
#include <ks.h>

#ifndef PLATFORM_API_WINOS_UTIL_INCLUDED
#define PLATFORM_API_WINOS_UTIL_INCLUDED

#define WIN_MAX_ERROR_LEN 200

#if (USE_PLATFORM_MIDI_IN == TRUE) || (USE_PLATFORM_MIDI_OUT == TRUE)

#include "PlatformMidi.h"

typedef struct tag_SysExQueue {
    int count;         // number of sys ex headers
    int size;          // data size per sys ex header
    int ownsLinearMem; // true when linearMem is to be disposed
    UBYTE* linearMem;  // where the actual sys ex data is, count*size bytes
    MIDIHDR header[1]; // Windows specific structure to hold meta info
} SysExQueue;

/* set the startTime field in MidiDeviceHandle */
void MIDI_SetStartTime(MidiDeviceHandle* handle);

/* return time stamp in microseconds */
INT64 MIDI_GetTimeStamp(MidiDeviceHandle* handle);

// the buffers do not contain memory
int MIDI_WinCreateEmptyLongBufferQueue(MidiDeviceHandle* handle, int count);
int MIDI_WinCreateLongBufferQueue(MidiDeviceHandle* handle, int count, int size, UBYTE* preAllocatedMem);
void MIDI_WinDestroyLongBufferQueue(MidiDeviceHandle* handle);

#endif // USE_PLATFORM_MIDI_IN || USE_PLATFORM_MIDI_OUT

#endif // PLATFORM_API_WINOS_UTIL_INCLUDED
