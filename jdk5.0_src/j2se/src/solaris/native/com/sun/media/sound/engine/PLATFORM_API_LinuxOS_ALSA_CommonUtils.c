/*
 * @(#)PLATFORM_API_LinuxOS_ALSA_CommonUtils.c	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_LinuxOS_ALSA_CommonUtils.c
**
**	Overview:
**	Utility functions for ALSA (common for PCM and rawmidi)
**
**	History	-
**	2003-08-04	$$mp created
*/
/*****************************************************************************/

//#define USE_ERROR
//#define USE_TRACE

#include "PLATFORM_API_LinuxOS_ALSA_CommonUtils.h"

static void alsaDebugOutput(const char *file, int line, const char *function, int err, const char *fmt, ...) {
#ifdef USE_ERROR
    va_list args;
    va_start(args, fmt);
    printf("%s:%d function %s: error %d: %s\n", file, line, function, err, snd_strerror(err));
    if (strlen(fmt) > 0) {
	vprintf(fmt, args);
    }
    va_end(args);
#endif
}

static int alsa_inited = 0;
static int alsa_enumerate_pcm_subdevices = FALSE; // default: no
static int alsa_enumerate_midi_subdevices = FALSE; // default: no

void initAlsaSupport() {
    char* enumerate;
    if (!alsa_inited) {
	alsa_inited = TRUE;
	snd_lib_error_set_handler(&alsaDebugOutput);

	enumerate = getenv(ENV_ENUMERATE_PCM_SUBDEVICES);
	if (enumerate != NULL && strlen(enumerate) > 0
	    && (enumerate[0] != 'f')   // false
	    && (enumerate[0] != 'F')   // False
	    && (enumerate[0] != 'n')   // no
	    && (enumerate[0] != 'N')) { // NO
	    alsa_enumerate_pcm_subdevices = TRUE;
	}
#ifdef ALSA_MIDI_ENUMERATE_SUBDEVICES
	alsa_enumerate_midi_subdevices = TRUE;
#endif
    }
}


// if true, sub devices are listed as separate mixers
int enumerateSubdevices() {
    initAlsaSupport();
    return alsa_enumerate_pcm_subdevices;
}


/*
 * deviceID contains packed card, device and subdevice numbers
 * each number takes 10 bits
 */
UINT32 encodeDeviceID(int card, int device, int subdevice) {
    return ((card & 0x3FF) << 20) | ((device & 0x3FF) << 10)
	| (subdevice & 0x3FF);
}


void decodeDeviceID(UINT32 deviceID, int* card, int* device, int* subdevice,
		    int isMidi) {
    int enumerate;
    *card = (deviceID >> 20) & 0x3FF;
    *device = (deviceID >> 10) & 0x3FF;
    if (isMidi) {
#ifdef ALSA_MIDI_ENUMERATE_SUBDEVICES
	enumerate = TRUE;
#else
	enumerate = FALSE;
#endif
    } else {
	enumerate = enumerateSubdevices();
    }
    if (enumerate) {
	*subdevice = deviceID  & 0x3FF;
    } else {
	*subdevice = -1; // ALSA will choose any subdevices
    }
}


void getDeviceString(char* buffer, int card, int device, int subdevice,
		     int usePlugHw, int isMidi) {
    int enumerate = enumerateSubdevices(isMidi);
    if (usePlugHw) {
	if (enumerate) {
	    sprintf(buffer, "%s:%d,%d,%d", ALSA_PLUGHARDWARE, card, device,
		    subdevice);
	} else {
	    sprintf(buffer, "%s:%d,%d", ALSA_PLUGHARDWARE, card, device);
	}
    } else {
	if (enumerate) {
	    sprintf(buffer, "%s:%d,%d,%d", ALSA_HARDWARE, card, device,
		    subdevice);
	} else {
	    sprintf(buffer, "%s:%d,%d", ALSA_HARDWARE, card, device);
	}
    }
}


void getDeviceStringFromDeviceID(char* buffer, UINT32 deviceID,
				 int usePlugHw, int isMidi) {
    int card, device, subdevice;

    decodeDeviceID(deviceID, &card, &device, &subdevice, isMidi);
    getDeviceString(buffer, card, device, subdevice, usePlugHw, isMidi);
}


static int hasGottenALSAVersion = FALSE;
#define ALSAVersionString_LENGTH 200
static char ALSAVersionString[ALSAVersionString_LENGTH];

void getALSAVersion(char* buffer, int len) {
    if (!hasGottenALSAVersion) {
	// get alsa version from proc interface
	FILE* file;
	int curr, len, totalLen, inVersionString;
	file = fopen(ALSA_VERSION_PROC_FILE, "r");
	ALSAVersionString[0] = 0;
	if (file) {
	    fgets(ALSAVersionString, ALSAVersionString_LENGTH, file);
	    // parse for version number
	    totalLen = strlen(ALSAVersionString);
	    inVersionString = FALSE;
	    len = 0;
	    curr = 0;
	    while (curr < totalLen) {
		if (!inVersionString) {
		    // is this char the beginning of a version string ?
		    if (ALSAVersionString[curr] >= '0'
			&& ALSAVersionString[curr] <= '9') {
			inVersionString = TRUE;
		    }
		}
		if (inVersionString) {
		    // the version string ends with white space
		    if (ALSAVersionString[curr] <= 32) {
			break;
		    }
		    if (curr != len) {
			// copy this char to the beginning of the string
			ALSAVersionString[len] = ALSAVersionString[curr];
		    }
		    len++;
		}
		curr++;
	    }
	    // remove trailing dots
	    while ((len > 0) && (ALSAVersionString[len - 1] == '.')) {
		len--;
	    }
	    // null terminate
	    ALSAVersionString[len] = 0;
	}
	hasGottenALSAVersion = TRUE;
    }
    strncpy(buffer, ALSAVersionString, len);
}


/* end */
