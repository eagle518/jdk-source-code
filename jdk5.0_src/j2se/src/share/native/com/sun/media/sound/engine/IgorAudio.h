/*
 * @(#)IgorAudio.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "IgorAudio.h"
**
** Modification History:
**
**	9/1/97		Created
*/
/*****************************************************************************/

#include "HAE.h"
#include "HAEMidiExternal.h"

#ifndef IGOR_AUDIO
#define IGOR_AUDIO

#define	IGOR_DROP_SAMPLE				HAE_DROP_SAMPLE
#define	IGOR_2_POINT_INTERPOLATION		HAE_2_POINT_INTERPOLATION
#define	IGOR_LINEAR_INTERPOLATION		HAE_LINEAR_INTERPOLATION
#define IgorTerpMode					HAETerpMode

#define IGOR_8K							HAE_8K
#define	IGOR_11K						HAE_11K
#define IGOR_11K_TERP_22K				HAE_11K_TERP_22K
#define	IGOR_22K						HAE_22K
#define	IGOR_24K						HAE_24K
#define	IGOR_44K						HAE_44K
#define	IGOR_48K						HAE_48K
#define	IgorQuality						HAEQuality

#define IGOR_NONE						HAE_NONE
#define IGOR_USE_16						HAE_USE_16
#define IGOR_USE_STEREO					HAE_USE_STEREO
#define IGOR_DISABLE_REVERB				HAE_DISABLE_REVERB
#define IGOR_STEREO_FILTER				HAE_STEREO_FILTER
#define	IgorAudioModifiers				HAEAudioModifiers

#define	IGOR_REVERB_NO_CHANGE			HAE_REVERB_NO_CHANGE
#define	IGOR_REVERB_NONE				HAE_REVERB_NONE
#define	IGOR_REVERB_TYPE_1				HAE_REVERB_TYPE_1
#define	IGOR_REVERB_TYPE_2 				HAE_REVERB_TYPE_2
#define	IGOR_REVERB_TYPE_3 				HAE_REVERB_TYPE_3
#define	IGOR_REVERB_TYPE_4 				HAE_REVERB_TYPE_4
#define	IGOR_REVERB_TYPE_5 				HAE_REVERB_TYPE_5
#define	IGOR_REVERB_TYPE_6 				HAE_REVERB_TYPE_6
#define	IGOR_REVERB_TYPE_7 				HAE_REVERB_TYPE_7
#define	IGOR_REVERB_TYPE_8 				HAE_REVERB_TYPE_8
#define	IgorReverbMode					HAEReverbMode

#define	IgorPathName					HAEPathName

#define	IGOR_NO_ERROR					HAE_NO_ERROR
#define	IGOR_PARAM_ERR					HAE_PARAM_ERR
#define	IGOR_MEMORY_ERR					HAE_MEMORY_ERR
#define	IGOR_BAD_INSTRUMENT				HAE_BAD_INSTRUMENT
#define	IGOR_BAD_MIDI_DATA				HAE_BAD_MIDI_DATA
#define	IGOR_ALREADY_PAUSED				HAE_ALREADY_PAUSED
#define	IGOR_ALREADY_RESUMED			HAE_ALREADY_RESUMED
#define	IGOR_DEVICE_UNAVAILABLE			HAE_DEVICE_UNAVAILABLE
#define	IGOR_NO_SONG_PLAYING			HAE_NO_SONG_PLAYING
#define	IGOR_STILL_PLAYING				HAE_STILL_PLAYING
#define	IGOR_TOO_MANY_SONGS_PLAYING		HAE_TOO_MANY_SONGS_PLAYING
#define	IGOR_NO_VOLUME					HAE_NO_VOLUME
#define	IGOR_GENERAL_ERR				HAE_GENERAL_ERR
#define	IGOR_NOT_SETUP					HAE_NOT_SETUP
#define	IGOR_NO_FREE_VOICES				HAE_NO_FREE_VOICES
#define	IGOR_STREAM_STOP_PLAY			HAE_STREAM_STOP_PLAY
#define	IGOR_BAD_FILE_TYPE				HAE_BAD_FILE_TYPE
#define	IGOR_GENERAL_BAD				HAE_GENERAL_BAD
#define	IGOR_BAD_FILE					HAE_BAD_FILE
#define	IGOR_NOT_REENTERANT				HAE_NOT_REENTERANT
#define	IGOR_BAD_SAMPLE					HAE_BAD_SAMPLE
#define	IGOR_BUFFER_TO_SMALL			HAE_BUFFER_TO_SMALL
#define	IgorErr							HAEErr

#define IgorInfoTypes					HAEInfoTypes
#define IgorMetaTypes					HAEMetaTypes

#define IGOR_AIFF_TYPE					HAE_AIFF_TYPE
#define IGOR_WAVE_TYPE					HAE_WAVE_TYPE
#define IGOR_AU_TYPE					HAE_AU_TYPE
#define IgorFileType					HAEFileType

#define	IGOR_BOOL						HAE_BOOL
#define	IGOR_INSTRUMENT					HAE_INSTRUMENT
#define	IGOR_FIXED						HAE_FIXED

#define IgorControlerCallbackPtr 		HAEControlerCallbackPtr
#define IgorTaskCallbackPtr				HAETaskCallbackPtr
#define	IgorTimeCallbackPtr				HAETimeCallbackPtr
#define	IgorMetaEventCallbackPtr		HAEMetaEventCallbackPtr
#define IgorDoneCallbackPtr				HAEDoneCallbackPtr
#define	IgorLoopDoneCallbackPtr			HAELoopDoneCallbackPtr
#define	IgorOutputCallbackPtr			HAEOutputCallbackPtr
#define IgorSampleFrameCallbackPtr		HAESampleFrameCallbackPtr

#define IGOR_MAX_VOICES					HAE_MAX_VOICES
#define IGOR_MAX_MIDI_VOLUME			HAE_MAX_MIDI_VOLUME

#define IGOR_MIN_STREAM_BUFFER_SIZE		HAE_MIN_STREAM_BUFFER_SIZE

#define IGOR_FULL_LEFT_PAN				HAE_FULL_LEFT_PAN
#define IGOR_CENTER_PAN					HAE_CENTER_PAN
#define IGOR_FULL_RIGHT_PAN				HAE_FULL_RIGHT_PAN

#define	IgorAudioInfo					HAEAudioInfo
#define	IgorSampleInfo					HAESampleInfo

#define	IgorAudioMixer					HAEAudioMixer
#define IgorRMFFile						HAERMFFile

#define IgorMidiFile					HAEMidiFile
#define IgorMidiDirect					HAEMidiDirect
#define IgorSound						HAESound
#define IgorSoundStream					HAESoundStream
#define IgorAudioNoise					HAEAudioNoise
#define	IgorMod							HAEMod

#define IgorStreamMessage				HAEStreamMessage
#define IGOR_STREAM_NULL				HAE_STREAM_NULL
#define IGOR_STREAM_CREATE				HAE_STREAM_CREATE
#define IGOR_STREAM_DESTROY				HAE_STREAM_DESTROY
#define IGOR_STREAM_GET_DATA			HAE_STREAM_GET_DATA
#define IGOR_STREAM_GET_SPECIFIC_DATA	HAE_STREAM_GET_SPECIFIC_DATA

#define IgorStreamData					HAEStreamData
#define IgorStreamObjectProc			HAEStreamObjectProc

#define IgorMidiInputDevice				HAEMidiExternal
#define IgorConnection					HAEConnection
#define IGOR_MIDI_MANAGER_FAILED		HAE_MIDI_MANAGER_FAILED
#define IGOR_OMS_FAILED					HAE_OMS_FAILED
#define IGOR_MIDI_MANGER_NOT_THERE		HAE_MIDI_MANGER_NOT_THERE
#define IGOR_OMS_CONNECT				HAE_OMS_CONNECT
#define IGOR_OMS_NAME_CONNECT			HAE_OMS_NAME_CONNECT
#define IGOR_MIDI_MANAGER_CONNECT		HAE_MIDI_MANAGER_CONNECT
	
#endif	// IGOR_AUDIO
