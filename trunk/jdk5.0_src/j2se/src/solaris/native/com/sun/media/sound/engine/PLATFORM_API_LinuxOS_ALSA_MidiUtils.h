/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_MidiUtils.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_LinuxOS_ALSA_MidiUtils.h
**
**	Overview:
**	Platform specfic utility functions for ALSA MIDI
**
**	History	-
**	2003-09-04	$$mp created
*/
/*****************************************************************************/

#include <alsa/asoundlib.h>
#include "Utilities.h"
#include "PlatformMidi.h"


#ifndef PLATFORM_API_LINUXOS_ALSA_MIDIUTILS_H_INCLUDED
#define PLATFORM_API_LINUXOS_ALSA_MIDIUTILS_H_INCLUDED

#define EVENT_PARSER_BUFSIZE (2048)

// if this is defined, use plughw: devices
//#define ALSA_MIDI_USE_PLUGHW
#undef ALSA_MIDI_USE_PLUGHW

typedef struct tag_ALSA_MIDIDeviceDescription {
	int index;          // in
	int strLen;         // in
	INT32 deviceID;    // out
	char* name;         // out
	char* description;  // out
} ALSA_MIDIDeviceDescription;


const char* getErrorStr(INT32 err);

/* Returns the number of devices. */
/* direction is either SND_RAWMIDI_STREAM_OUTPUT or
   SND_RAWMIDI_STREAM_INPUT. */
int getMidiDeviceCount(snd_rawmidi_stream_t direction);

/* Returns MIDI_SUCCESS or MIDI_INVALID_DEVICEID */
/* direction is either SND_RAWMIDI_STREAM_OUTPUT or
   SND_RAWMIDI_STREAM_INPUT. */
int getMidiDeviceName(snd_rawmidi_stream_t direction, int index,
		      char *name, UINT32 nameLength);

/* Returns MIDI_SUCCESS or MIDI_INVALID_DEVICEID */
int getMidiDeviceVendor(int index, char *name, UINT32 nameLength);

/* Returns MIDI_SUCCESS or MIDI_INVALID_DEVICEID */
/* direction is either SND_RAWMIDI_STREAM_OUTPUT or
   SND_RAWMIDI_STREAM_INPUT. */
int getMidiDeviceDescription(snd_rawmidi_stream_t direction, int index,
			     char *name, UINT32 nameLength);

/* Returns MIDI_SUCCESS or MIDI_INVALID_DEVICEID */
int getMidiDeviceVersion(int index, char *name, UINT32 nameLength);

// returns 0 on success, otherwise MIDI_OUT_OF_MEMORY or ALSA error code
/* direction is either SND_RAWMIDI_STREAM_OUTPUT or
   SND_RAWMIDI_STREAM_INPUT. */
INT32 openMidiDevice(snd_rawmidi_stream_t direction, INT32 deviceIndex,
		     MidiDeviceHandle** handle);

// returns 0 on success, otherwise a (negative) ALSA error code
INT32 closeMidiDevice(MidiDeviceHandle* handle);

INT64 getMidiTimestamp(MidiDeviceHandle* handle);

#endif // PLATFORM_API_LINUXOS_ALSA_MIDIUTILS_H_INCLUDED

