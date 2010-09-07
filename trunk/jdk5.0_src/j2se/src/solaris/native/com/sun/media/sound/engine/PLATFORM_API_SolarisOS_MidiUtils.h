/*
 * @(#)PLATFORM_API_SolarisOS_MidiUtils.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	Overview:
**	Functions used for both MIDI in and MIDI out.
**
**	History	-
**	2003-01-29	$$fb Created
*/
/*****************************************************************************/

#if (USE_PLATFORM_MIDI_IN == TRUE) || (USE_PLATFORM_MIDI_OUT == TRUE)

#include "PlatformMidi.h"
#include <audio/midi.h>
#include <sys/audioio.h>
/* for memcpy */
#include <string.h>
/* for malloc */
#include <stdlib.h>
/* for usleep */
#include <unistd.h>

#ifdef USE_ERROR
#include <stdio.h>
#endif

#ifdef USE_ERROR
#define MIDI_CHECK_ERROR  { if (err != MIDI_ERROR_NONE) MIDI_Utils_PrintError(err); }
#else
#define MIDI_CHECK_ERROR
#endif

typedef struct {
    MidiDeviceHandle h;      /* the real handle (must be the first field!) */
    int dir;                 /* direction of the port */
    int isStarted;           /* whether device is "started" */
    midi_msg_t* native_msg;  /* the current message */
    MidiMessage msg;         /* the copied message */
} SolMidiDeviceHandle;

extern char* MIDI_Utils_GetErrorMsg(int err);
extern void MIDI_Utils_PrintError(int err);

extern INT32 MIDI_Utils_GetNumDevices(uint_t dir);
extern INT32 MIDI_Utils_GetDeviceName(INT32 deviceID, char *name, UINT32 nameLength);
extern INT32 MIDI_Utils_GetDeviceVendor(INT32 deviceID, char *name, UINT32 nameLength);
extern INT32 MIDI_Utils_GetDeviceDescription(INT32 deviceID, char *name, UINT32 nameLength);
extern INT32 MIDI_Utils_GetDeviceVersion(INT32 deviceID, char *name, UINT32 nameLength);
extern INT32 MIDI_Utils_OpenDevice(INT32 deviceID, int dir, SolMidiDeviceHandle** handle,
				   int num_msgs, int num_long_msgs, 
				   size_t lm_size);
extern INT32 MIDI_Utils_CloseDevice(SolMidiDeviceHandle* handle);
extern INT32 MIDI_Utils_StartDevice(SolMidiDeviceHandle* handle);
extern INT32 MIDI_Utils_StopDevice(SolMidiDeviceHandle* handle);
extern INT64 MIDI_Utils_GetTimeStamp(MidiDeviceHandle* handle);


#endif // USE_PLATFORM_MIDI_IN || USE_PLATFORM_MIDI_OUT
