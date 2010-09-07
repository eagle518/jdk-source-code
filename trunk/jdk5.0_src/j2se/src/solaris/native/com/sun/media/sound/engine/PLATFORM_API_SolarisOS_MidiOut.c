/*
 * @(#)PLATFORM_API_SolarisOS_MidiOut.c	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	Implementation for external MIDI output.
**	This implementation does not interface with the HAE engine at all.
**
**	Overview:
**
**	History	-
**	2003-01-29	$$fb created
*/
/*****************************************************************************/

#define USE_ERROR
#define USE_TRACE

#if USE_PLATFORM_MIDI_OUT == TRUE

#include "PLATFORM_API_SolarisOS_MidiUtils.h"


/* 
 * implementation of the platform-dependent 
 * MIDI out functions declared in PlatformMidi.h 
 */

char* MIDI_OUT_GetErrorStr(INT32 err) {
    return MIDI_Utils_GetErrorMsg((int) err);
}


INT32 MIDI_OUT_GetNumDevices() {
    return MIDI_Utils_GetNumDevices(MIDI_OUT);
}


INT32 MIDI_OUT_GetDeviceName(INT32 deviceID, char *name, UINT32 nameLength) {
    return MIDI_Utils_GetDeviceName(deviceID, name, nameLength);
}


INT32 MIDI_OUT_GetDeviceVendor(INT32 deviceID, char *name, UINT32 nameLength) {
    return MIDI_Utils_GetDeviceVendor(deviceID, name, nameLength);
}


INT32 MIDI_OUT_GetDeviceDescription(INT32 deviceID, char *name, UINT32 nameLength) {
    /* TODO: add information about device, i.e. PORT, SYNTH, LOOPBACK, etc. ? */
    return MIDI_Utils_GetDeviceDescription(deviceID, name, nameLength);
}


INT32 MIDI_OUT_GetDeviceVersion(INT32 deviceID, char *name, UINT32 nameLength) {
    return MIDI_Utils_GetDeviceVersion(deviceID, name, nameLength);
}


/* *************************** MidiOutDevice implementation ***************************************** */

INT32 MIDI_OUT_OpenDevice(INT32 deviceID, MidiDeviceHandle** handle) {
    TRACE1("MIDI_OUT_OpenDevice: deviceID: %d\n", deviceID);
    /* queue sizes are ignored for MIDI_OUT only (uses STREAMS) */
    return MIDI_Utils_OpenDevice(deviceID, MIDI_OUT, (SolMidiDeviceHandle**) handle, 0, 0, 0);
}

INT32 MIDI_OUT_CloseDevice(MidiDeviceHandle* handle) {
    TRACE0("MIDI_OUT_CloseDevice\n");

    // issue a "SUSTAIN OFF" message to each MIDI channel, 0 to 15.
    // "CONTROL CHANGE" is 176, "SUSTAIN CONTROLLER" is 64, and the value is 0.
    // $$fb 2002-04-04: It is responsability of the application developer to
    // leave the device in a consistent state. So I put this in comments
    /*
      for (channel = 0; channel < 16; channel++)
      MIDI_OUT_SendShortMessage(deviceHandle, (unsigned char)(176 + channel), 
      (unsigned char)64, (unsigned char)0, (UINT32)-1);
    */
    return MIDI_Utils_CloseDevice((SolMidiDeviceHandle*) handle);
}


INT64 MIDI_OUT_GetTimeStamp(MidiDeviceHandle* handle) {
	return MIDI_Utils_GetTimeStamp(handle);
}


INT32 MIDI_OUT_SendShortMessage(MidiDeviceHandle* handle, UINT32 packedMsg, UINT32 timestamp) {
    int err;

    TRACE2("> MIDI_OUT_SendShortMessage %x, time: %d\n", packedMsg, timestamp);
    if (!handle) {
	ERROR0("< ERROR: MIDI_OUT_SendShortMessage: handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    err = midi_send_msg((midi_handle_t) handle->deviceHandle, 
			(uint_t) (packedMsg & 0xFF),
			(uint_t) ((packedMsg >> 8) & 0xFF),
			(uint_t) ((packedMsg >> 16) & 0xFF));
    MIDI_CHECK_ERROR;
    TRACE0("< MIDI_OUT_SendShortMessage\n");
    return err;
}


INT32 MIDI_OUT_SendLongMessage(MidiDeviceHandle* handle, UBYTE* data, UINT32 size, UINT32 timestamp) {
    int err;

    TRACE2("> MIDI_OUT_SendLongMessage size %d, time: %d\n", size, timestamp);
    if (!handle || !data) {
	ERROR0("< ERROR: MIDI_OUT_SendLongMessage: handle, or data is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    if (size == 0) {
	return MIDI_SUCCESS;
    }

    handle->isWaiting = TRUE;
    err = midi_send_long_msg((midi_handle_t) handle->deviceHandle,
			     (uchar_t*) data, (size_t) size);
    handle->isWaiting = FALSE;
    TRACE0("< MIDI_OUT_SendLongMessage\n");
    return err;
}

#endif /* USE_PLATFORM_MIDI_OUT */
