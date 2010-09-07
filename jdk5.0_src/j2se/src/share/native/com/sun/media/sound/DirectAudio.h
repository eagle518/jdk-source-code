/*
 * @(#)DirectAudio.h	1.8 04/04/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains header information for platform port support.
 * This does not interface with the Beatnik engine at all.
 */
#ifndef DIRECT_AUDIO_INCLUDED
#define DIRECT_AUDIO_INCLUDED

// includes for types
#include "engine/X_API.h"
// for memset
#include <string.h>

#include "Utilities.h"

// the following defines should match the ones in AbstractMixer.java
#define DAUDIO_PCM  0
#define DAUDIO_ULAW 1
#define DAUDIO_ALAW 2

#define DAUDIO_STRING_LENGTH 200

typedef struct tag_DirectAudioDeviceDescription {
    // optional deviceID (complementary to deviceIndex)
    INT32 deviceID;
    INT32 maxSimulLines;
    char name[DAUDIO_STRING_LENGTH+1];
    char vendor[DAUDIO_STRING_LENGTH+1];
    char description[DAUDIO_STRING_LENGTH+1];
    char version[DAUDIO_STRING_LENGTH+1];
} DirectAudioDeviceDescription;


// method definitions

#if (USE_DAUDIO == TRUE)

// callback from GetFormats, implemented in DirectAudioDevice.c
void DAUDIO_AddAudioFormat(void* creator, int significantBits, int frameSizeInBytes,
			   int channels, float sampleRate, 
			   int encoding, int isSigned, 
			   int bigEndian);


// the following methods need to be implemented by the platform dependent code

/* returns the number of mixer devices */
INT32 DAUDIO_GetDirectAudioDeviceCount();

/* returns TRUE on success, FALSE otherwise */
INT32 DAUDIO_GetDirectAudioDeviceDescription(INT32 mixerIndex, 
                                             DirectAudioDeviceDescription* description);

// SourceDataLine and TargetDataLine

void DAUDIO_GetFormats(INT32 mixerIndex, INT32 deviceID, int isSource, void* creator);

void* DAUDIO_Open(INT32 mixerIndex, INT32 deviceID, int isSource, 
		  int encoding, float sampleRate, int sampleSizeInBits, 
		  int frameSize, int channels, 
		  int isSigned, int isBigEndian, int bufferSizeInBytes);
int DAUDIO_Start(void* id, int isSource);
int DAUDIO_Stop(void* id, int isSource);
void DAUDIO_Close(void* id, int isSource);
int DAUDIO_Write(void* id, char* data, int byteSize); // returns -1 on error
int DAUDIO_Read(void* id, char* data, int byteSize);  // returns -1 on error

int DAUDIO_GetBufferSize(void* id, int isSource);
int DAUDIO_StillDraining(void* id, int isSource);
int DAUDIO_Flush(void* id, int isSource);
/* in bytes */
int DAUDIO_GetAvailable(void* id, int isSource);
INT64 DAUDIO_GetBytePosition(void* id, int isSource, INT64 javaBytePos);
void DAUDIO_SetBytePosition(void* id, int isSource, INT64 javaBytePos);

int DAUDIO_RequiresServicing(void* id, int isSource);
void DAUDIO_Service(void* id, int isSource);

#endif // USE_DAUDIO

#endif // DIRECT_AUDIO_INCLUDED
