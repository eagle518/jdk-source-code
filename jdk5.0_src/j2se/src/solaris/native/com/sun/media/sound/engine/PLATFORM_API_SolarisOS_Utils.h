/*
 * @(#)PLATFORM_API_SolarisOS_Utils.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	PLATFORM_API_SolarisOS_Utils.h
**
**	Overview:
**	Platform specfic utility functions for Solaris.
**
**	History	-
**	2002-08-06	$$fb created
*/
/*****************************************************************************/

#include <Utilities.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
/* does not work on Solaris 2.7 */
#include <sys/audio.h> 
#include <sys/mixer.h> 
#include <sys/types.h>
#include <stropts.h>
#include <sys/conf.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PLATFORM_API_SOLARISOS_UTILS_H_INCLUDED
#define PLATFORM_API_SOLARISOS_UTILS_H_INCLUDED

/* defines for Solaris 2.7
   #ifndef AUDIO_AUX1_OUT
   #define AUDIO_AUX1_OUT   (0x08)  // output to aux1 out
   #define AUDIO_AUX2_OUT   (0x10)  // output to aux2 out
   #define AUDIO_SPDIF_OUT  (0x20)  // output to SPDIF port
   #define AUDIO_AUX1_IN    (0x08)    // input from aux1 in
   #define AUDIO_AUX2_IN    (0x10)    // input from aux2 in
   #define AUDIO_SPDIF_IN   (0x20)    // input from SPDIF port
   #endif
*/

/* input from Codec inter. loopback */
#ifndef AUDIO_CODEC_LOOPB_IN
#define AUDIO_CODEC_LOOPB_IN       (0x40)
#endif


#define MAX_NAME_LENGTH 300

typedef struct tag_AudioDevicePath {
    char path[MAX_NAME_LENGTH];
    ino_t st_ino; // inode number to detect duplicate devices
    dev_t st_dev; // device ID to detect duplicate audio devices
} AudioDevicePath;

typedef struct tag_AudioDeviceDescription {
    INT32 maxSimulLines;
    char path[MAX_NAME_LENGTH+1];
    char pathctl[MAX_NAME_LENGTH+4];
    char name[MAX_NAME_LENGTH+1];
    char vendor[MAX_NAME_LENGTH+1];
    char version[MAX_NAME_LENGTH+1];
    char description[MAX_NAME_LENGTH+1];
} AudioDeviceDescription;

int getAudioDeviceCount();

/*
 * adPath is an array of AudioDevicePath structures
 * count contains initially the number of elements in adPath
 *       and will be set to the returned number of paths.
 */
void getAudioDevices(AudioDevicePath* adPath, int* count);

/*
 * fills adDesc from the audio device given in path
 * returns 0 if an error occured
 * if getNames is 0, only path and pathctl are filled
 */
int getAudioDeviceDescription(char* path, AudioDeviceDescription* adDesc, int getNames);
int getAudioDeviceDescriptionByIndex(int index, AudioDeviceDescription* adDesc, int getNames);


#endif // PLATFORM_API_SOLARISOS_UTILS_H_INCLUDED

