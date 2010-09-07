/*
 * @(#)PLATFORM_API_SolarisOS_MidiIn.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	Overview:
**	This provides platform specfic MIDI input functions for Solaris.
**	This implementation does not interface with the HAE engine at all.
**
**	History	-
**	2003-01-28	$$fb Created
*/
/*****************************************************************************/

#define USE_ERROR
#define USE_TRACE

#if USE_PLATFORM_MIDI_IN == TRUE

#include "PLATFORM_API_SolarisOS_MidiUtils.h"

/* 
 * implementation of the platform-dependent 
 * MIDI in functions declared in PlatformMidi.h 
 */

char* MIDI_IN_GetErrorStr(INT32 err) {
    return MIDI_Utils_GetErrorMsg((int) err);
}


INT32 MIDI_IN_GetNumDevices() {
    return MIDI_Utils_GetNumDevices(MIDI_IN);
}


INT32 MIDI_IN_GetDeviceName(INT32 deviceID, char *name, UINT32 nameLength) {
    return MIDI_Utils_GetDeviceName(deviceID, name, nameLength);
}


INT32 MIDI_IN_GetDeviceVendor(INT32 deviceID, char *name, UINT32 nameLength) {
    return MIDI_Utils_GetDeviceVendor(deviceID, name, nameLength);
}


INT32 MIDI_IN_GetDeviceDescription(INT32 deviceID, char *name, UINT32 nameLength) {
    return MIDI_Utils_GetDeviceDescription(deviceID, name, nameLength);
}


INT32 MIDI_IN_GetDeviceVersion(INT32 deviceID, char *name, UINT32 nameLength) {
    return MIDI_Utils_GetDeviceVersion(deviceID, name, nameLength);
}


INT32 MIDI_IN_OpenDevice(INT32 deviceID, MidiDeviceHandle** handle) {
    TRACE0("MIDI_IN_OpenDevice\n");
    return 
	MIDI_Utils_OpenDevice(deviceID, MIDI_IN, (SolMidiDeviceHandle**) handle,
			      MIDI_IN_MESSAGE_QUEUE_SIZE, 
			      MIDI_IN_LONG_QUEUE_SIZE, 
			      MIDI_IN_LONG_MESSAGE_SIZE);
}


INT32 MIDI_IN_CloseDevice(MidiDeviceHandle* handle) {
    TRACE0("MIDI_IN_CloseDevice\n");
    return MIDI_Utils_CloseDevice((SolMidiDeviceHandle*) handle);
}


INT32 MIDI_IN_StartDevice(MidiDeviceHandle* handle) {
    TRACE0("MIDI_IN_StartDevice\n");
    return MIDI_Utils_StartDevice((SolMidiDeviceHandle*) handle);
}


INT32 MIDI_IN_StopDevice(MidiDeviceHandle* handle) {
    TRACE0("MIDI_IN_StopDevice\n");
    return MIDI_Utils_StopDevice((SolMidiDeviceHandle*) handle);
}

INT64 MIDI_IN_GetTimeStamp(MidiDeviceHandle* handle) {
    return MIDI_Utils_GetTimeStamp(handle);
}


/* read the next message from the queue */
MidiMessage* MIDI_IN_GetMessage(MidiDeviceHandle* handle) {
    SolMidiDeviceHandle* h = (SolMidiDeviceHandle*) handle;
    int err;

    if (h && h->isStarted && handle->deviceHandle) {
	/* wait a maximum of 1/2 second to receive a message */
	handle->isWaiting = TRUE;
	h->native_msg = midi_get_msg((midi_handle_t) handle->deviceHandle, 500, &err);
	if (h->isStarted && (h->native_msg != NULL)) {
	    /* copy the message to the internal representation */
	    h->msg.timestamp = (INT64) h->native_msg->msg_timestamp;
	    TRACE1("Received message. Timestamp=%d millis\n", (int) (h->native_msg->msg_timestamp / 1000));
	    /* don't need to care for locked field */
	    switch (h->native_msg->msg_type) {
	    case MIDI_MSG_STD: 
		h->msg.type = SHORT_MESSAGE;
		h->msg.data.s.packedMsg = 
		    (UINT32) (h->native_msg->msg_status
			      | (h->native_msg->msg_data1 << 8)
			      | (h->native_msg->msg_data2 << 16));
		break;
	    case MIDI_MSG_LONG: /* fall through */
	    case MIDI_MSG_LONG_CONT:
		h->msg.type = LONG_MESSAGE;
		h->msg.data.l.size = (UINT32) h->native_msg->msg_data.msg_long.msg_size;
		h->msg.data.l.data = (UBYTE*) h->native_msg->msg_data.msg_long.msg_data;
		break;
	    }
	    handle->isWaiting = FALSE;
	    return &(h->msg);
	} else {
	    MIDI_CHECK_ERROR;
	}
	handle->isWaiting = FALSE;
	/* TRACE0("MIDI_IN_GetMessage: waiting finished\n") */
    }
    return NULL;
}


void MIDI_IN_ReleaseMessage(MidiDeviceHandle* handle, MidiMessage* msg) {
    SolMidiDeviceHandle* h = (SolMidiDeviceHandle*) handle;
    int err;

    if (h && handle->deviceHandle && (msg == &(h->msg))) {
	err = midi_return_msg((midi_handle_t) handle->deviceHandle, h->native_msg);
	MIDI_CHECK_ERROR;
    }
}

#endif /* USE_PLATFORM_MIDI_IN */
