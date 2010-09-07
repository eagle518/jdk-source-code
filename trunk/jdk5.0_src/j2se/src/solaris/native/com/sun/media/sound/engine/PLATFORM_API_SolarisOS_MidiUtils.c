/*
 * @(#)PLATFORM_API_SolarisOS_MidiUtils.c	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	Overview:
**      Implementation of the functions used both for MIDI in and MIDI out.
**
**	History	-
**	2003-01-29	$$fb Created
*/
/*****************************************************************************/

#define USE_ERROR
#define USE_TRACE

#if (USE_PLATFORM_MIDI_IN == TRUE) || (USE_PLATFORM_MIDI_OUT == TRUE)

#include "PLATFORM_API_SolarisOS_MidiUtils.h"

char* MIDI_Utils_GetErrorMsg(int err) {
    return midi_strerror(err);
}


void MIDI_Utils_PrintError(int err) {
#ifdef USE_ERROR
    char* s = MIDI_Utils_GetErrorMsg(err);
    if (s!=NULL) {
	printf("%s\n", s);
    }
#endif
}


/* cache (not thread safe!) */
INT32 lastDeviceID=-1;
audio_device_t lastDeviceInfo;
midi_info_t lastMidiInfo;

INT32 refreshMidiDeviceInfoCache(INT32 deviceID) {
    int err = MIDI_ERROR_NONE;

    if (deviceID < 0) {
	return MIDI_INVALID_DEVICEID;
    }

    if (lastDeviceID != deviceID) {
	err = midi_get_dev_info((int) deviceID, &lastDeviceInfo);
	if (err != MIDI_ERROR_NONE) {
	    lastDeviceID = -1;
	    MIDI_CHECK_ERROR;
	} else {
	    err = midi_get_port_info((int) deviceID, &lastMidiInfo);
	    if (err != MIDI_ERROR_NONE) {
		lastDeviceID = -1;
		MIDI_CHECK_ERROR;
	    } else {
		lastDeviceID = deviceID;
	    }
	}
    }
    return err;
}


INT32 MIDI_Utils_GetNumDevices(uint_t dir) {
    int err;
    int num_ports;
    
    err = midi_get_num_ports(&num_ports);
    if (err != MIDI_ERROR_NONE) {
	num_ports = 0;
	MIDI_CHECK_ERROR;
    }
    return (INT32) num_ports;
}


INT32 MIDI_Utils_GetDeviceName(INT32 deviceID, char *name, UINT32 nameLength) {
    int err = refreshMidiDeviceInfoCache(deviceID);
    if (err == MIDI_ERROR_NONE) {
	strncpy(name, lastMidiInfo.midi_name, nameLength-1);
	name[nameLength-1] = 0;
    }
    return err;
}


INT32 MIDI_Utils_GetDeviceVendor(INT32 deviceID, char *name, UINT32 nameLength) {
    int err = refreshMidiDeviceInfoCache(deviceID);
    if (err == MIDI_ERROR_NONE) {
	strncpy(name, lastDeviceInfo.name, nameLength-1);
	name[nameLength-1] = 0;
    }
    return err;
}


INT32 MIDI_Utils_GetDeviceDescription(INT32 deviceID, char *name, UINT32 nameLength) {
    int err = refreshMidiDeviceInfoCache(deviceID);
    if (err == MIDI_ERROR_NONE) {
	strncpy(name, lastDeviceInfo.config, nameLength-1);
	name[nameLength-1] = 0;
    }
    return err;
}


INT32 MIDI_Utils_GetDeviceVersion(INT32 deviceID, char *name, UINT32 nameLength) {
    int err = refreshMidiDeviceInfoCache(deviceID);
    if (err == MIDI_ERROR_NONE) {
	strncpy(name, lastDeviceInfo.version, nameLength-1);
	name[nameLength-1] = 0;
    }
    return err;
}


INT32 MIDI_Utils_OpenDevice(INT32 deviceID, int dir, SolMidiDeviceHandle** handle, 
			    int num_msgs, int num_long_msgs, 
			    size_t lm_size) {
    midi_handle_t native_handle;
    int err;

    TRACE0("MIDI_Utils_OpenDevice\n");

    (*handle) = (SolMidiDeviceHandle*) malloc(sizeof(SolMidiDeviceHandle));
    if (!(*handle)) {
	ERROR0("ERROR: MIDI_Utils_OpenDevice: out of memory\n");
	return MIDI_OUT_OF_MEMORY;
    }
    memset(*handle, 0, sizeof(SolMidiDeviceHandle));

    // don't need to create queue -> MIDI subsystem is taking care of it

    // finally open the device
    native_handle = midi_bind_port((int) deviceID, dir, num_msgs, num_long_msgs, lm_size, &err);

    if (native_handle == NULL) {
	MIDI_CHECK_ERROR;
	free(*handle);
	return err;
    }
    (*handle)->h.deviceHandle = (void*) native_handle;
    (*handle)->dir = dir;

    TRACE0("MIDI_Utils_OpenDevice: succeeded\n");
    return err;
}


INT32 MIDI_Utils_CloseDevice(SolMidiDeviceHandle* handle) {
    int err;

    TRACE0("> MIDI_Utils_CloseDevice\n");
    if (!handle) {
		ERROR0("< ERROR: MIDI_Utils_CloseDevice: handle is NULL\n");
		return MIDI_INVALID_HANDLE;
    }
    if (!handle->h.deviceHandle) {
		ERROR0("< ERROR: MIDI_Utils_CloseDevice: native handle is NULL\n");
		return MIDI_INVALID_HANDLE;
    }
    handle->isStarted = FALSE;
    err = midi_unbind_port((midi_handle_t) handle->h.deviceHandle);
    handle->h.deviceHandle = NULL;
    MIDI_CHECK_ERROR;
    // wait until the Java thread has exited
    while (handle->h.isWaiting) usleep(1000);
    free(handle);

    TRACE0("< MIDI_Utils_CloseDevice: succeeded\n");
    return MIDI_SUCCESS;
}


INT32 MIDI_Utils_StartDevice(SolMidiDeviceHandle* handle) {
    int err;
    long long new_ts;
    
    if (!handle || !handle->h.deviceHandle) {
	ERROR0("ERROR: MIDI_Utils_StartDevice: handle or native is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    /* reset the time stamp */
    err = midi_set_timestamp((midi_handle_t) handle->h.deviceHandle, (long long) 0);
    MIDI_CHECK_ERROR;
    /* WORKAROUND for a bug in the first beta version of the MIDI drivers */
    /* TODO: remove this workaround */
    new_ts = (long long) MIDI_Utils_GetTimeStamp((MidiDeviceHandle*) handle);
    if ((new_ts < -1) || (new_ts > 100000)) {
	TRACE0("Needed to use workaround for setting timestamp to 0.\n");
	new_ts = gethrtime() / 1000;
	err = midi_set_timestamp((midi_handle_t) handle->h.deviceHandle, new_ts);
	MIDI_CHECK_ERROR;
    }
    /* END WORKAROUND */

    if (!handle->isStarted) {
	/* set the flag that we can now receive messages */
	handle->isStarted = TRUE;
	
	/* flush event queue */
	err = midi_flush_bound_data((midi_handle_t) handle->h.deviceHandle, 
				    handle->dir);
	MIDI_CHECK_ERROR;
    }
    return MIDI_SUCCESS; /* don't fail */
}


INT32 MIDI_Utils_StopDevice(SolMidiDeviceHandle* handle) {
    int err;
    
    if (!handle || !handle->h.deviceHandle) {
	ERROR0("ERROR: MIDI_Utils_StopDevice: handle or native handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }

    if (handle->isStarted) {
	/* set the flag that we don't want to receive messages anymore */
	handle->isStarted = FALSE;
	
	/* flush event queue */
	/* platformData contains the direction */
	err = midi_flush_bound_data((midi_handle_t) handle->h.deviceHandle, 
				    handle->dir);
	MIDI_CHECK_ERROR;
    }
    return MIDI_SUCCESS;
}


INT64 MIDI_Utils_GetTimeStamp(MidiDeviceHandle* handle) {
    int err;
    long long ts = 0;
    
    if (!handle || !handle->deviceHandle) {
		ERROR0("ERROR: MIDI_Utils_GetTimeStamp: handle or native handle is NULL\n");
		return (INT64) -1; /* failure */
    }
    err = midi_get_timestamp((midi_handle_t) handle->deviceHandle, &ts);
    if (err != MIDI_ERROR_NONE) {
		MIDI_CHECK_ERROR;
		return (INT64) -1; /* failure */
    }
    return (INT64) ts;
}


#endif // USE_PLATFORM_MIDI_IN || USE_PLATFORM_MIDI_OUT
