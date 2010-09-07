/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_MidiUtils.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_LinuxOS_ALSA_Utils.c
**
**	Overview:
**	Utility functions for ALSA MIDI
**
**	History	-
**	2003-08-04	$$mp created
*/
/*****************************************************************************/

#define USE_ERROR
#define USE_TRACE

#include "PLATFORM_API_LinuxOS_ALSA_MidiUtils.h"
#include "PLATFORM_API_LinuxOS_ALSA_CommonUtils.h"
#include <string.h>
#include <sys/time.h>

static INT64 getTimeInMicroseconds() {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000UL) + tv.tv_usec;
}


const char* getErrorStr(INT32 err) {
	return snd_strerror((int) err);
}



// callback for iteration through devices
// returns TRUE if iteration should continue
typedef int (*DeviceIteratorPtr)(char* devName, int card, int device,
				 int subdevice, snd_ctl_t *handle,
				 snd_rawmidi_info_t* rawmidi_info,
				 snd_ctl_card_info_t* info, void* userData);


// for each ALSA device, call iterator. userData is passed to the iterator
// returns total number of iterations
static int iterateRawmidiDevices(snd_rawmidi_stream_t direction,
				 DeviceIteratorPtr iterator,
				 void* userData) {
    int count = 0;
    int subdeviceCount;
    int card, dev, subDev;
    char devname[16];
    int err;
    snd_ctl_t *handle;
    snd_rawmidi_info_t* rawmidi_info;
    snd_ctl_card_info_t* card_info;
    int doContinue = TRUE;

    snd_rawmidi_info_malloc(&rawmidi_info);
    snd_ctl_card_info_malloc(&card_info);
    
    card = -1;
    TRACE0("testing for cards...\n");
    if (snd_card_next(&card) >= 0) {
	TRACE1("Found card %d\n", card);
	while (doContinue && (card >= 0)) {
	    sprintf(devname, ALSA_HARDWARE_CARD, card);
	    TRACE1("Opening control for alsa rawmidi device \"%s\"...\n", devname);
	    err = snd_ctl_open(&handle, devname, 0);
	    if (err < 0) {
		ERROR2("ERROR: snd_ctl_open, card=%d: %s\n", card, snd_strerror(err));
	    } else {
		TRACE0("snd_ctl_open() SUCCESS\n");
		err = snd_ctl_card_info(handle, card_info);
		if (err < 0) {
		    ERROR2("ERROR: snd_ctl_card_info, card=%d: %s\n", card, snd_strerror(err));
		} else {
		    TRACE0("snd_ctl_card_info() SUCCESS\n");
		    dev = -1;
		    while (doContinue) {
			if (snd_ctl_rawmidi_next_device(handle, &dev) < 0) {
			    ERROR0("snd_ctl_rawmidi_next_device\n");
			}
			TRACE0("snd_ctl_rawmidi_next_device() SUCCESS\n");
			if (dev < 0) {
			    break;
			}
			snd_rawmidi_info_set_device(rawmidi_info, dev);
			snd_rawmidi_info_set_subdevice(rawmidi_info, 0);
			snd_rawmidi_info_set_stream(rawmidi_info, direction);
			err = snd_ctl_rawmidi_info(handle, rawmidi_info);
			TRACE0("after snd_ctl_rawmidi_info()\n");
			if (err < 0) {
			    if (err != -ENOENT) {
				ERROR2("ERROR: snd_ctl_rawmidi_info, card=%d: %s", card, snd_strerror(err));
			    }
			} else {
			    TRACE0("snd_ctl_rawmidi_info() SUCCESS\n");
#ifdef ALSA_MIDI_ENUMERATE_SUBDEVICES
			    TRACE0("enumerating subdevices\n");
			    subdeviceCount = snd_rawmidi_info_get_subdevices_count(rawmidi_info);
#else
			    subdeviceCount = 1;
#endif
			    if (iterator!=NULL) {
				for (subDev = 0; subDev < subdeviceCount; subDev++) {
				    TRACE3("  Iterating %d,%d,%d\n", card, dev, subDev);
				    doContinue = (*iterator)(devname, card, dev, subDev, handle, rawmidi_info, card_info, userData);
				    TRACE0("returned from iterator\n");
				    if (!doContinue) {
					break;
				    }
				}
			    }
			    count += subdeviceCount;
			}
		    } // of while(doContinue)
		}
		snd_ctl_close(handle);
	    }
	    if (snd_card_next(&card) < 0) {
		break;
	    }
	}
    } else {
	ERROR0("No cards found!\n");
    }
    snd_ctl_card_info_free(card_info);
    snd_rawmidi_info_free(rawmidi_info);
    return count;
}



int getMidiDeviceCount(snd_rawmidi_stream_t direction) {
    int deviceCount;
    TRACE0("> getMidiDeviceCount()\n");
    initAlsaSupport();
    deviceCount = iterateRawmidiDevices(direction, NULL, NULL);
    TRACE0("< getMidiDeviceCount()\n");
    return deviceCount;
}



/*
  userData is assumed to be a pointer to ALSA_MIDIDeviceDescription.
  ALSA_MIDIDeviceDescription->index has to be set to the index of the device
  we want to get information of before this method is called the first time via
  iterateRawmidiDevices(). On each call of this method,
  ALSA_MIDIDeviceDescription->index is decremented. If it is equal to zero,
  we have reached the desired device, so action is taken.
  So after successful completion of iterateRawmidiDevices(),
  ALSA_MIDIDeviceDescription->index is zero. If it isn't, this is an
  indication of an error.
*/
static int deviceInfoIterator(char* devName, int card, int device,
			      int subdevice, snd_ctl_t *handle,
			      snd_rawmidi_info_t* rawmidi_info,
			      snd_ctl_card_info_t* info, void* userData) {
    char buffer[300];
    ALSA_MIDIDeviceDescription* desc = ( ALSA_MIDIDeviceDescription*) userData;
#ifdef ALSA_MIDI_USE_PLUGHW
    int usePlugHw = 1;
#else
    int usePlugHw = 0;
#endif

    TRACE0("deviceInfoIterator\n");
    initAlsaSupport();
    if (desc->index == 0) {

	desc->deviceID = encodeDeviceID(card, device, subdevice);

	buffer[0]=' '; buffer[1]='[';
	getDeviceString(&(buffer[2]), card, device, subdevice,
			usePlugHw, ALSA_RAWMIDI);
	strcat(buffer, "]");
	strncpy(desc->name, snd_ctl_card_info_get_id(info), desc->strLen - strlen(buffer));
	strncat(desc->name, buffer, desc->strLen - strlen(desc->name));
	strncpy(desc->description, snd_ctl_card_info_get_name(info), desc->strLen);
	strncat(desc->description, ", ", desc->strLen - strlen(desc->description));
	strncat(desc->description, snd_rawmidi_info_get_id(rawmidi_info), desc->strLen - strlen(desc->description));
	strncat(desc->description, ", ", desc->strLen - strlen(desc->description));
	strncat(desc->description, snd_rawmidi_info_get_name(rawmidi_info), desc->strLen - strlen(desc->description));
	TRACE2("Returning %s, %s\n", desc->name, desc->description);
	return FALSE; // do not continue iteration
    }
    desc->index--;
    return TRUE;
}


static int getMIDIDeviceDescriptionByIndex(snd_rawmidi_stream_t direction,
					   ALSA_MIDIDeviceDescription* desc) {
    initAlsaSupport();
    TRACE1(" getMIDIDeviceDescriptionByIndex (index = %d)\n", desc->index);
    iterateRawmidiDevices(direction, &deviceInfoIterator, desc);
    return (desc->index == 0) ? MIDI_SUCCESS : MIDI_INVALID_DEVICEID;
}



int initMIDIDeviceDescription(ALSA_MIDIDeviceDescription* desc, int index) {
    int ret = MIDI_SUCCESS;
    desc->index = index;
    desc->strLen = 200;
    desc->name = (char*) calloc(desc->strLen + 1, 1);
    desc->description = (char*) calloc(desc->strLen + 1, 1);
    if (! desc->name ||
	! desc->description) {
	ret = MIDI_OUT_OF_MEMORY;
    }
    return ret;
}


void freeMIDIDeviceDescription(ALSA_MIDIDeviceDescription* desc) {
    if (desc->name) {
	free(desc->name);
    }
    if (desc->description) {
	free(desc->description);
    }
}


int getMidiDeviceName(snd_rawmidi_stream_t direction, int index, char *name,
		      UINT32 nameLength) {
    ALSA_MIDIDeviceDescription desc;
    int ret;

    TRACE1("getMidiDeviceName: nameLength: %d\n", (int) nameLength);
    ret = initMIDIDeviceDescription(&desc, index);
    if (ret == MIDI_SUCCESS) {
	TRACE0("getMidiDeviceName: initMIDIDeviceDescription() SUCCESS\n");
	ret = getMIDIDeviceDescriptionByIndex(direction, &desc);
	if (ret == MIDI_SUCCESS) {
	    TRACE1("getMidiDeviceName: desc.name: %s\n", desc.name);
	    strncpy(name, desc.name, nameLength - 1);
	    name[nameLength - 1] = 0;
	}
    }
    freeMIDIDeviceDescription(&desc);
    return ret;
}


int getMidiDeviceVendor(int index, char *name, UINT32 nameLength) {
    strncpy(name, ALSA_VENDOR, nameLength - 1);
    name[nameLength - 1] = 0;
    return MIDI_SUCCESS;
}


int getMidiDeviceDescription(snd_rawmidi_stream_t direction,
			     int index, char *name, UINT32 nameLength) {
    ALSA_MIDIDeviceDescription desc;
    int ret;

    ret = initMIDIDeviceDescription(&desc, index);
    if (ret == MIDI_SUCCESS) {
	ret = getMIDIDeviceDescriptionByIndex(direction, &desc);
	if (ret == MIDI_SUCCESS) {
	    strncpy(name, desc.description, nameLength - 1);
	    name[nameLength - 1] = 0;
	}
    }
    freeMIDIDeviceDescription(&desc);
    return ret;
}


int getMidiDeviceVersion(int index, char *name, UINT32 nameLength) {
    getALSAVersion(name, nameLength);
    return MIDI_SUCCESS;
}


static int getMidiDeviceID(snd_rawmidi_stream_t direction, int index,
			   UINT32* deviceID) {
    ALSA_MIDIDeviceDescription desc;
    int ret;

    ret = initMIDIDeviceDescription(&desc, index);
    if (ret == MIDI_SUCCESS) {
	ret = getMIDIDeviceDescriptionByIndex(direction, &desc);
	if (ret == MIDI_SUCCESS) {
	    // TRACE1("getMidiDeviceName: desc.name: %s\n", desc.name);
	    *deviceID = desc.deviceID;
	}
    }
    freeMIDIDeviceDescription(&desc);
    return ret;
}


/*
  direction has to be either SND_RAWMIDI_STREAM_INPUT or
  SND_RAWMIDI_STREAM_OUTPUT.
  Returns 0 on success. Otherwise, MIDI_OUT_OF_MEMORY, MIDI_INVALID_ARGUMENT
   or a negative ALSA error code is returned.
*/
INT32 openMidiDevice(snd_rawmidi_stream_t direction, INT32 deviceIndex,
		     MidiDeviceHandle** handle) {
    snd_rawmidi_t* native_handle;
    snd_midi_event_t* event_parser = NULL;
    int err;
    UINT32 deviceID;
    char devicename[100];
#ifdef ALSA_MIDI_USE_PLUGHW
    int usePlugHw = 1;
#else
    int usePlugHw = 0;
#endif

    TRACE0("> openMidiDevice()\n");

    (*handle) = (MidiDeviceHandle*) calloc(sizeof(MidiDeviceHandle), 1);
    if (!(*handle)) {
	ERROR0("ERROR: openDevice: out of memory\n");
	return MIDI_OUT_OF_MEMORY;
    }

    // TODO: iterate to get dev ID from index
    err = getMidiDeviceID(direction, deviceIndex, &deviceID);
    TRACE1("  openMidiDevice(): deviceID: %d\n", (int) deviceID);
    getDeviceStringFromDeviceID(devicename, deviceID,
				usePlugHw, ALSA_RAWMIDI);
    TRACE1("  openMidiDevice(): deviceString: %s\n", devicename);

    // finally open the device
    if (direction == SND_RAWMIDI_STREAM_INPUT) {
	err = snd_rawmidi_open(&native_handle, NULL, devicename,
			       SND_RAWMIDI_NONBLOCK);
    } else if (direction == SND_RAWMIDI_STREAM_OUTPUT) {
	err = snd_rawmidi_open(NULL, &native_handle, devicename,
			       SND_RAWMIDI_NONBLOCK);
    } else {
	ERROR0("  ERROR: openMidiDevice(): direction is neither SND_RAWMIDI_STREAM_INPUT nor SND_RAWMIDI_STREAM_OUTPUT\n");
	err = MIDI_INVALID_ARGUMENT;
    }
    if (err < 0) {
	ERROR1("<  ERROR: openMidiDevice(): snd_rawmidi_open() returned %d\n", err);
	free(*handle);
	(*handle) = NULL;
	return err;
    }
    /* We opened with non-blocking behaviour to not get hung if the device
       is used by a different process. Writing, however, should
       be blocking. So we change it here. */
    if (direction == SND_RAWMIDI_STREAM_OUTPUT) {
	err = snd_rawmidi_nonblock(native_handle, 0);
	if (err < 0) {
	    ERROR1("  ERROR: openMidiDevice(): snd_rawmidi_nonblock() returned %d\n", err);
	    snd_rawmidi_close(native_handle);
	    free(*handle);
	    (*handle) = NULL;
	    return err;
	}
    }
    if (direction == SND_RAWMIDI_STREAM_INPUT) {
	err = snd_midi_event_new(EVENT_PARSER_BUFSIZE, &event_parser);
	if (err < 0) {
	    ERROR1("  ERROR: openMidiDevice(): snd_midi_event_new() returned %d\n", err);
	    snd_rawmidi_close(native_handle);
	    free(*handle);
	    (*handle) = NULL;
	    return err;
	}
    }

    (*handle)->deviceHandle = (void*) native_handle;
    (*handle)->startTime = getTimeInMicroseconds();
    (*handle)->platformData = event_parser;
    TRACE0("< openMidiDevice(): succeeded\n");
    return err;
}



INT32 closeMidiDevice(MidiDeviceHandle* handle) {
    int err;

    TRACE0("> closeMidiDevice()\n");
    if (!handle) {
	ERROR0("< ERROR: closeMidiDevice(): handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    if (!handle->deviceHandle) {
	ERROR0("< ERROR: closeMidiDevice(): native handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    err = snd_rawmidi_close((snd_rawmidi_t*) handle->deviceHandle);
    TRACE1("  snd_rawmidi_close() returns %d\n", err);
    if (handle->platformData) {
	snd_midi_event_free((snd_midi_event_t*) handle->platformData);
    }
    free(handle);
    TRACE0("< closeMidiDevice: succeeded\n");
    return err;
}


INT64 getMidiTimestamp(MidiDeviceHandle* handle) {
    if (!handle) {
	ERROR0("< ERROR: closeMidiDevice(): handle is NULL\n");
	return MIDI_INVALID_HANDLE;
    }
    return getTimeInMicroseconds() - handle->startTime;
}


/* end */
