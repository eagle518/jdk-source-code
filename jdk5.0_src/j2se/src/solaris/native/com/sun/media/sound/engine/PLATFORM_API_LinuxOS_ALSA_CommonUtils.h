/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_CommonUtils.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_LinuxOS_ALSA_CommonUtils.h
**
**	Overview:
**	Platform specfic utility functions for ALSA (common for PCM and rawmidi)
**
**	History	-
**	2003-09-04	$$mp created
*/
/*****************************************************************************/

#include <alsa/asoundlib.h>
#include "Utilities.h"

#ifndef PLATFORM_API_LINUXOS_ALSA_COMMONUTILS_H_INCLUDED
#define PLATFORM_API_LINUXOS_ALSA_COMMONUTILS_H_INCLUDED

#define ALSA_VERSION_PROC_FILE "/proc/asound/version"
#define ALSA_HARDWARE "hw"
#define ALSA_HARDWARE_CARD ALSA_HARDWARE":%d"
#define ALSA_HARDWARE_DEVICE ALSA_HARDWARE_CARD",%d"
#define ALSA_HARDWARE_SUBDEVICE ALSA_HARDWARE_DEVICE",%d"

#define ALSA_PLUGHARDWARE "plughw"

#define ALSA_PCM     (0)
#define ALSA_RAWMIDI (1)

// for use in info objects
#define ALSA_VENDOR "ALSA (http://www.alsa-project.org)"

// Environment variable for inclusion of subdevices in device listing.
// If this variable is unset or "no", then subdevices are ignored, and 
// it's ALSA's choice which one to use (enables hardware mixing)
#define ENV_ENUMERATE_PCM_SUBDEVICES "ALSA_ENUMERATE_PCM_SUBDEVICES"

// if defined, subdevices are listed.
//#undef ALSA_MIDI_ENUMERATE_SUBDEVICES
#define ALSA_MIDI_ENUMERATE_SUBDEVICES

// must be called before any ALSA calls
void initAlsaSupport();

/* if true, sub devices are listed as separate mixers
   This is only used for PCM.
 */
int enumerateSubdevices();


/*
 * deviceID contains packed card, device and subdevice numbers
 * each number takes 10 bits
 */
UINT32 encodeDeviceID(int card, int device, int subdevice);

void decodeDeviceID(UINT32 deviceID, int* card, int* device, int* subdevice,
		    int isMidi);

void getDeviceString(char* buffer, int card, int device, int subdevice,
		     int usePlugHw, int isMidi);

void getDeviceStringFromDeviceID(char* buffer, UINT32 deviceID,
				 int usePlugHw, int isMidi);

void getALSAVersion(char* buffer, int len);


#endif // PLATFORM_API_LINUXOS_ALSA_COMMONUTILS_H_INCLUDED

