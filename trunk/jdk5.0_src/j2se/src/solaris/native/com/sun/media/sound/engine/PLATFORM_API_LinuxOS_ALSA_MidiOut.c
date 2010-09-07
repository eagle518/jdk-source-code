/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_MidiOut.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	Implementation for external MIDI output, using ALSA.
**	This implementation does not interface with the HAE engine at all.
**
**	Overview:
**
**	History	-
**	2003-07-28	$$fb created
*/
/*****************************************************************************/

#define USE_ERROR
#define USE_TRACE

#if USE_PLATFORM_MIDI_OUT == TRUE

#include <alsa/asoundlib.h>
#include "PlatformMidi.h"
#include "PLATFORM_API_LinuxOS_ALSA_MidiUtils.h"



static int CHANNEL_MESSAGE_LENGTH[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, 3, 3, 3, 3, 2, 2, 3 };
/*                                 8x 9x Ax Bx Cx Dx Ex */

static int SYSTEM_MESSAGE_LENGTH[] = {
    -1, 2, 3, 2, -1, -1, 1, 1, 1, -1, 1, 1, 1, -1, 1, 1 };
/*  F0 F1 F2 F3  F4  F5 F6 F7 F8  F9 FA FB FC  FD FE FF */


// the returned length includes the status byte.
// for illegal messages, -1 is returned.
static int getShortMessageLength(int status) {
	int	dataLength = 0;
	if (status < 0xF0) { // channel voice message
		dataLength = CHANNEL_MESSAGE_LENGTH[(status >> 4) & 0xF];
	} else {
		dataLength = SYSTEM_MESSAGE_LENGTH[status & 0xF];
	}
	return dataLength;
}


/* 
 * implementation of the platform-dependent 
 * MIDI out functions declared in PlatformMidi.h 
 */
char* MIDI_OUT_GetErrorStr(INT32 err) {
    return (char*) getErrorStr(err);
}


INT32 MIDI_OUT_GetNumDevices() {
    TRACE0("MIDI_OUT_GetNumDevices()\n");
    return getMidiDeviceCount(SND_RAWMIDI_STREAM_OUTPUT);
}


INT32 MIDI_OUT_GetDeviceName(INT32 deviceIndex, char *name, UINT32 nameLength) {
    TRACE0("MIDI_OUT_GetDeviceName()\n");
    return getMidiDeviceName(SND_RAWMIDI_STREAM_OUTPUT, deviceIndex,
			     name, nameLength);
}


INT32 MIDI_OUT_GetDeviceVendor(INT32 deviceIndex, char *name, UINT32 nameLength) {
    TRACE0("MIDI_OUT_GetDeviceVendor()\n");
    return getMidiDeviceVendor(deviceIndex, name, nameLength);
}


INT32 MIDI_OUT_GetDeviceDescription(INT32 deviceIndex, char *name, UINT32 nameLength) {
    TRACE0("MIDI_OUT_GetDeviceDescription()\n");
    return getMidiDeviceDescription(SND_RAWMIDI_STREAM_OUTPUT, deviceIndex,
				    name, nameLength);
}


INT32 MIDI_OUT_GetDeviceVersion(INT32 deviceIndex, char *name, UINT32 nameLength) {
    TRACE0("MIDI_OUT_GetDeviceVersion()\n");
    return getMidiDeviceVersion(deviceIndex, name, nameLength);
}


/* *************************** MidiOutDevice implementation *************** */

INT32 MIDI_OUT_OpenDevice(INT32 deviceIndex, MidiDeviceHandle** handle) {
    TRACE1("MIDI_OUT_OpenDevice(): deviceIndex: %d\n", (int) deviceIndex);
    return openMidiDevice(SND_RAWMIDI_STREAM_OUTPUT, deviceIndex, handle);
}


INT32 MIDI_OUT_CloseDevice(MidiDeviceHandle* handle) {
    TRACE0("MIDI_OUT_CloseDevice()\n");
    return closeMidiDevice(handle);
}


INT64 MIDI_OUT_GetTimeStamp(MidiDeviceHandle* handle) {
    return getMidiTimestamp(handle);
}


INT32 MIDI_OUT_SendShortMessage(MidiDeviceHandle* handle, UINT32 packedMsg,
				UINT32 timestamp) {
    int err;
    int status;
    int data1;
    int data2;
    char buffer[3];

    TRACE2("> MIDI_OUT_SendShortMessage() %x, time: %u\n", packedMsg, (unsigned int) timestamp);
    if (!handle) {
	ERROR0("< ERROR: MIDI_OUT_SendShortMessage(): handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    if (!handle->deviceHandle) {
	ERROR0("< ERROR: MIDI_OUT_SendLongMessage(): native handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    status = (packedMsg & 0xFF);
    buffer[0] = (char) status;
    buffer[1]  = (char) ((packedMsg >> 8) & 0xFF);
    buffer[2]  = (char) ((packedMsg >> 16) & 0xFF);
    TRACE4("status: %d, data1: %d, data2: %d, length: %d\n", (int) buffer[0], (int) buffer[1], (int) buffer[2], getShortMessageLength(status));
    err = snd_rawmidi_write((snd_rawmidi_t*) handle->deviceHandle, buffer, getShortMessageLength(status));
    if (err < 0) {
	ERROR1("  ERROR: MIDI_OUT_SendShortMessage(): snd_rawmidi_write() returned %d\n", err);
    }

    TRACE0("< MIDI_OUT_SendShortMessage()\n");
    return err;
}


INT32 MIDI_OUT_SendLongMessage(MidiDeviceHandle* handle, UBYTE* data,
			       UINT32 size, UINT32 timestamp) {
    int err;

    TRACE2("> MIDI_OUT_SendLongMessage() size %u, time: %u\n", (unsigned int) size, (unsigned int) timestamp);
    if (!handle) {
	ERROR0("< ERROR: MIDI_OUT_SendLongMessage(): handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    if (!handle->deviceHandle) {
	ERROR0("< ERROR: MIDI_OUT_SendLongMessage(): native handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    if (!data) {
	ERROR0("< ERROR: MIDI_OUT_SendLongMessage(): data is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    err = snd_rawmidi_write((snd_rawmidi_t*) handle->deviceHandle,
			    data, size);
    if (err < 0) {
	ERROR1("  ERROR: MIDI_OUT_SendLongMessage(): snd_rawmidi_write() returned %d\n", err);
    }

    TRACE0("< MIDI_OUT_SendLongMessage()\n");
    return err;
}


#endif /* USE_PLATFORM_MIDI_OUT */
