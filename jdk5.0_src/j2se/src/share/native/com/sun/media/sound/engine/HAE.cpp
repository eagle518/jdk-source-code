/*
 * @(#)HAE.cpp	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "HAE.cpp"
**
**	Generalized Audio Synthesis package presented in an oop fashion
**
** Modification History:
**
**	7/8/96		Created
**	11/1/96		Added HAEStreamSound class
**	11/15/96	Fixed StopMidiSong to end the sequencer
**				Fixed FadeMidiSong to restore the volume after the song ends
**	11/18/96	Added LoadResourceSample
**	11/19/96	Changed char to HAE_BOOL in cases of boolean use
**	11/21/96	Changed LoadResourceSample to pass in size of data
**	11/26/96	Changed ServiceStreams to ServiceIdle
**				Added FadeTo and async for all fades to the HAEMidiFile
**				and HAESoundStream, and HAESound
**	12/1/96		Changed GetCurrentProgramBank to GetProgramBank
**	12/2/96		Added Mod code
**	12/3/96		Fixed bug with HAESound::Start
**	12/10/96	Added HAEAudioMixer::ChangeAudioFileToMemory
**	12/16/96	Fixed bug in destructors that hung in a loop while trying to remove
**				objects from fade links
**	12/16/96	Fixed bug in HAEMidiFile::SetTempoInBeatsPerMinute.Used the wrong API
**	12/30/96	Changed copyright
**	1/2/97		Added GetSongNameFromAudioFile & GetSampleNameFromAudioFile &
**				GetInstrumentNameFromAudioFile
**	1/7/97		Added HAEMidiFile::LoadFromMemory
**	1/10/97		Added some const
**	1/15/97		Changed LoadFromBank and LoadBankSample to support native MacOS resources
**	1/16/97		Added GetInfoSize
**	1/22/97		Added HAEAudioMixer::GetTick
**				Fixed callbacks and added SetDoneCallback in HAESound and HAEMod
**	1/24/97		Fixed a bug with HAEAudioMixer::Close that could make my counters
**				get out of sync
**				Added HAEMod::SetLoopFlag & HAEMod::SetTempoInBeatsPerMinute &
**				HAEMod::GetTempoInBeatsPerMinute(void)
**				Rebuilt fade system. Completely intergrated with low level mixer
**	1/27/97		Changed Fade to not stop the audio object once it reaches zero
**	1/28/97		Added a HAERMFFile class. Its pretty much the same as a HAEMidiFile
**				object, except it knows how to read an RMF file
**	2/1/97		Added HAEMidiDirect::AllowChannelPitchOffset & 
**				HAEMidiDirect::DoesChannelAllowPitchOffset
**	2/5/97		Added HAESound::LoadMemorySample
**				Changed MOD handling not to keep MOD loaded into memory once parsed
**	2/8/97		Modified HAEMixer to change modifiers for stereo support
**	2/19/97		Added support for platform specific data when creating an HAEAudioMixer
**	2/28/97		Added NoteOnWithLoad & IsInstrumentLoaded & TranslateBankProgramToInstrument & 
**				TranslateCurrentProgramToInstrument
**	3/3/97		Changed NoteOnWithLoad to get the current time after loading instrument
**	3/10/97		Removed extra reverbmode get, and typed the SetReverb correctly
**	3/18/97		Fixed the pause/resume methods. They were reversed
**	3/20/97		Changed ChangeAudioModes to remap modes to C API
**				Added GetMicrosecondLength & SetMicrosecondPosition & GetMicrosecondPosition
**				in the HAEMidiFile class
**				Fixed a bug with HAEAudioMixer::SetReverbType. Forgot to set reverb type
**				for later retreival
**	4/1/97		Fixed a bug with ProgramBankChange. Forgot to include the controller change
**				for the non queued version
**				Changed GetTick & GetAudioLatency to pull the tick count from the new mixer
**				syncCount rather than XMicroseconds
**	4/18/97		Removed extra linkage and unused variables
**	4/20/97		Added HAEMidiFile::SetMetaEventCallback
**	5/1/97		Changed HAESound::Start to accecpt an frame offset when starting the sample
**	5/3/97		Fixed a few potential problems that the MOT compiler found
**				Added HAESound::StartCustomSample
**	5/5/97		Fixed a problem with HAESound::StartCustomSample and HAESound::Start
**				in which the stereoPosition was scaled wrong. Now the value is -63 to 63.
**	5/7/97		Added to HAESound::StartCustomSample & HAESound::Start new error
**				messages for a voice that is still playing, a way to stop the sound
**				and an error when the volume level is zero
**	5/12/97		Fixed memory leak when failing with HAEMidiFile::LoadFromFile & 
**				HAEMidiFile::LoadFromMemory
**	5/13/97		Added HAEAudioMixer::GetVersionFromAudioFile
**	5/21/97		Added HAESound::GetPlaybackPosition
**				Changed method names for HAESoundStream. To stream a normal file,
**				you'll now call SetupFileStream then Start. To do a custom stream
**				call SetupCustomStream, then start.
**				Added HAESoundStream::GetInfo to return information about the
**				file once it has been setup.
**				Added HAESound::GetInfo
**	5/23/97		Added HAESound::GetSamplePointer & HAESound::GetSampleLoopPoints &
**				HAESound::SetSampleLoopPoints
**	6/4/97		Restricted 68k codebase to 11k, mono, drop sample
**	6/25/97		Changed an unsigned to an unsigned long in HAESound::SetSampleLoopPoints
**				Fixed HAESound::SetSampleLoopPoints to actually set the playing samples
**				loop points
**	6/27/97		Changed HAEMidiDirect::Open to set the song to use the current
**				mixer settings
**				Changed HAEAudioMixer::GetMixLevel & HAEAudioMixer::GetSoundVoices & 
**				HAEAudioMixer::GetMidiVoice & HAEAudioMixer::GetModifiers update to
**				the real values
**	7/9/97		Added HAEMidiDirect::ParseMidiData to parse midi data and disburse it
**				to the various functions
**	7/15/97		Added HAEAudioNoise base class to all HAEAudio objects
**				Added HAEAudioMixer::StartOutputToFile & HAEAudioMixer::StopOutputToFile & 
**				HAEAudioMixer::ServiceAudioOutputToFile
**	7/21/97		Changed HAEMod::GetTempoInBeatsPerMinute and HAEMidi::GetTempoInBeatsPerMinute to
**				to unsigned long
**	7/22/97		Changed SYNC_BUFFER_TIME to BUFFER_SLICE_TIME
**	7/28/97		Put USE_STREAM_API around the stream class
**	8/8/97		Modified PV_ProcessSongMetaEventCallbacks to support extra parameter
**	8/18/97		Seperated GM_StartHardwareSoundManager from being called inside of 
**				GM_InitGeneralSound. Now its called in HAEAudioMixer::Open. Likewise
**				GM_StopHardwareSoundManager is no longer being called within GM_FinisGeneralSound,
**				it nows is called at HAEAudioMixer::Close
**	8/20/97		Changed HAESoundStream::SetupCustomStream & HAESoundStream::SetupFileStream
**				to use HAE_MIN_STREAM_BUFFER_SIZE to control minimumn size of buffer
**	8/25/97		Changed parameter order in PV_ProcessSongControllerCallbacks
**	9/9/97		Fixed bug with PV_ProcessSongTimeCallbacks that used the wrong callback !
**	9/11/97		Added HAEMidiFile::IsPaused && HAEMod::IsDone && HAEMidiFile::IsPlaying &&
**				HAESoundStream::IsDone && HAESound::IsDone
**	9/15/97		Changed HAEMidiFile::Unload to call HAEMidiDirect::Close instead of duplicating
**				code.
**	9/30/97		Changed references to GM_AudioStreamError to pass in the stream reference
**				Changed HAEAudioMixer::GetInstrumentNameFromAudioFileFromID to allow for ID
**				being 0
**				Fixed a bug in which HAEAudioMixer::ChangeAudioFileToMemory && 
**				HAEAudioMixer::ChangeAudioFile would leave a bad patch file open if it
**				failed to open the passed file. Now it reverts back to the previously
**				open patch file if failure occurs.
**	9/30/97		Added HAEMidiFile::GetEmbeddedMidiVoices & HAEMidiFile::GetEmbeddedMixLevel & 
**				HAEMidiFile::GetEmbeddedSoundVoices
**	10/3/97		Added HAEMidiFile::SetEmbeddedMidiVoices & HAEMidiFile::SetEmbeddedMixLevel & 
**				HAEMidiFile::SetEmbeddedSoundVoices
**				Added HAEMidiFile::GetEmbeddedReverbType & HAEMidiFile::SetEmbeddedReverbType
**				Added HAEAudioMixer::GetQuality & HAEAudioMixer::GetTerpMode
**	10/12/97	Added HAERMFFile::LoadFromBank
**	10/15/97	Changed the HAERMFFile class to always copy the RMF data
**	10/16/97	Changed HAEMidiFile::Start to allow for an optional reconfigure of
**				the mixer when starting a song
**				Modified HAEMidiFile::LoadFromID & HAEMidiFile::LoadFromBank & HAEMidiFile::LoadFromFile &
**				HAEMidiFile::LoadFromMemory & HAERMFFile::LoadFromFile & HAERMFFile::LoadFromMemory &
**				HAERMFFile::LoadFromBank to allow for optional reporting of failure to load instruments
**				Removed HAEMidiDirect::FlushInstrumentCache. Not required or used.
**				Renamed GetPatches to GetInstruments and changed the array passed to
**				be of type HAE_INSTRUMENT
**	11/6/97		Added HAERMFFile::LoadFromID
**	11/10/97	Wrapped some conditional code around HAEMidiFile::GetInstruments
**	11/11/97	Cleaned up some garbage code from HAEAudioMixer::HAEAudioMixer
**				Added GetMaxDeviceCount & SetCurrentDevice & GetCurrentDevice & GetDeviceName
**	11/24/97	Added HAESoundStream::Flush
**	12/18/97	Cleaned up some warnings and added some rounding devices
**	1/18/98		Fixed up IsPlaying/IsDone pairs to return BOOLEANS correctly
**	1/20/98		Fixed callback problem with HAEMidiFile::Start
**				Fixed memory allocation problem with HAEMidiDirect::Close
**	1/22/98		Changed name of pRMFDataBlock to m_pRMFDataBlock
**				Added HAERMFFile::IsCompressed HAERMFFile::IsEncrypted
**	1/27/98		Added a parameter to HAEMidiFile::SetTimeCallback and changed the way the callbacks
**				are handled
**	2/10/98		In HAEMidiDirect::Open enabled sample caching, and HAEMidiDirect::Close
**				disables sample caching
**				Fixed HAEMidiFile::LoadFromMemory to return a memory error in case it
**				can't duplicate the memory block
**				Fixed HAERMFFile::LoadFromMemory to return a memory error in case it
**				can't duplicate the memory block, and fixed the function to actaully
**				copy the data correctly.
**	2/11/98		Changed HAESound::SetSampleLoopPoints to accept 0 as a valid start
**				loop point
**				Added HAE_8K, HAE_48K, HAE_11K_TERP_22K, HAE_22K_TERP_44K, HAE_24K
**	2/18/98		Added HAESound::StartDoubleBuffer
**	2/19/98		Added HAEAudioMixer::GetURLFromAudioFile & HAEAudioMixer::GetNameFromAudioFile
**				Added code wrappers around GetURLFromAudioFile & GetNameFromAudioFile
**	2/24/98		Fixed a problem with the way the SONG resource and memory based files handle
**				retriving the size of memory blocks inside of a memory file
**	3/2/98		Fixed HAEAudioMixer::Open to Close down the mixer correctly when failing
**	3/5/98		Fixed HAESound::StartCustomSample & HAESound::Start to return an error if
**				trying to play samples larger than 1MB
**	3/9/98		Modified open to allow an optional not connect to audio hardware. If you call
**				Open without connecting to hardware, you'll need to call HAEAudioMixer::ReengageAudio
**				Added new method HAEAudioMixer::IsAudioEngaged
**	3/11/98		Grr. Fixed a problem with HAEAudioMixer::Open. Didn't connect to audio hardware
**				if you passed NULL as the audiofile.
**	3/12/98		Added HAEMidiFile::SetEmbeddedVolume & HAEMidiFile::GetEmbeddedVolume
**				Changed HAEAudioMixer::ReengageAudio & HAEAudioMixer::DisengageAudio
**				to change the audioSetup flag correctly.
**				Fixed HAEMidiDirect::Open to return an error code if memory fails
**	3/16/98		Added new verb types
**	4/30/98		Added SUB_GENRE_INFO & GENRE_INFO
**	5/5/98		Fixed HAEAudioMixer::GetNameFromAudioFile & HAEAudioMixer::GetURLFromAudioFile
**				to clear the name in case there is no information
**	5/7/98		Fixed HAESound::LoadFileSample & HAESound::LoadMemorySample to handle
**				error codes better
**				Created HAEPrivate.h to allow access to certain functions.
**				Renamed PV_TranslateOPErr to HAE_TranslateOPErr
**	5/26/98		Removed from HAEAudioMixer::Open the auto close and deallocate of the mixer upon
**				error. Deallocation will now happen when HAEAudioMixer object is deleted.
**	6/18/98		Added SetCacheStatusForAudioFile & GetCacheStatusForAudioFile
**	6/20/98		Renamed PV_CustomStreamCallback to PV_CustomOutputStreamCallback
**	7/6/98		Changed HAEMidiDirect::IsInstrumentLoaded to use direct GM_ API
**	7/17/98		Added HAE_UseThisFile
**	7/20/98		Changed ChangeAudioFile & ChangeAudioFileToMemory to handle
**				passing of a NULL to close the current audio patch file association
**				to the mixer
**	7/27/98		Added LockQueue & UnlockQueue
**	7/28/98		Added HAEMidiDirect::SetStereoPosition & HAEMidiDirect::GetStereoPosition
**	7/30/98		Added reverb state and fixed all the HAESound::Start/HAESoundStream::Start functions 
**				to start the verb up when the object gets started.
**	8/6/98		Changed reference to mSoundVoiceReference in the HAESound class. Fixed a few area where
**				I forgot to check for the voice being active
**				Changed mReference to mSoundStreamVoiceReference in the HAESoundStream class
**	8/10/98		Removed some sign/unsigned conflicts
**	8/11/98		Renamed pName to cName and implemented GetName in the HAEAudioNoise base class
**	8/13/98		Added voiceType to HAEAudioMixer::GetRealtimeStatus
**				Added HAEAudioMixer::GetCPULoadInMicroseconds & HAEAudioMixer::GetCPULoadInPercent
**	8/14/98		Modified HAEAudioMixer::GetCPULoadInPercent to use embedded Gen API function
**				rather than calculate it
**	9/2/98		Added HAE_TranslateAudioFileType
**	9/10/98		Fixed some problems when USE_HIGHLEVEL_FILE_API is not defined
**	9/12/98		Added HAESound::GetSamplePointerFromMixer
**	10/17/98	Fixed a problem with HAESound::Start & HAESound::StartDoubleBuffer & HAESound::StartCustomSample
**				in which two or more voices that are started will sometimes kill the previous voice allocated
**	10/26/98	Added error code to HAEAudioMixer::ServiceAudioOutputToFile
**	10/30/98	Implemented a default HAESound done callback, that marks the sample finished when playing out
**				normally. Related to the 10/17/98 bug.
**	11/19/98	Added new parameter to HAE_UseThisFile
**	11/20/98	Added support for mixer->Open passing in HAE_REVERB_NO_CHANGE
**	11/24/98	Added HAESound::SetReverbAmount & HAESound::GetReverbAmount.
**	12/3/98		Added HAESoundStream::SetReverbAmount & HAESoundStream::GetReverbAmount
**				Added HAEGroup class, and modified HAEAudioNoise for linked list issues
**	12/9/98		Added HAEMidiDirect::GetPitchBend
**	12/17/98	Added HAEAudioMixer::SetHardwareBalance & HAEAudioMixer::GetHardwareBalance
**	12/18/98	Added to HAEMidiFile::Start auto level
**	1/5/99		Changed copyright and fixed a sign conversion warning
**	1/14/99		Added HAEMidiDirect::CreateInstrumentAsData && HAEMidiDirect::LoadInstrumentFromData
**	1/29/99		Changed HAEAudioMixer::GetVersionFromAudioFile to use new XGetVersion API
**	2/12/99		Renamed USE_HAE_FOR_MPEG to USE_MPEG_DECODER
**	2/18/99		Renamed pSongVariables to m_pSongVariables, queueMidi to mQueueMidi, 
**				reference to mReference, m_performanceVariablesLength to mPerformanceVariablesLength
**				Added GetMetaCallback & GetMetaCallbackReference and support variables
**	2/24/99		Changed PV_ProcessSongEndCallbacks to use the new context from a GM_Song to deal
**				with the this pointer.
**				Changed HAEMidiFile::SetMetaEventCallback & PV_ProcessSongMetaEventCallbacks to use
**				new context of GM_Song structure rather than extra references
**	3/1/99		Fixed a bug in HAESoundStream::GetReverbAmount in which the wrong value was being
**				returned
**	3/3/99		Changed to the new way of starting samples. First a setup, then a start
**	3/5/99		Added threadContext to PV_ProcessSongTimeCallbacks & 
**				PV_ProcessSongMetaEventCallbacks & PV_ProcessSongControllerCallbacks & 
**				PV_ProcessSequencerEvents
**				Changed HAEAudioMixer::ServiceAudioOutputToFile to not call behind the wall
**				functions
**	2002-03-14	$$fb added Linux support
*/
/*****************************************************************************/


/* THINGS TO DO ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥


Fix HAERMFFile::LoadFromMemory so that duplicate object actaully does the right thing

*/


#include "HAE.h"

#include "X_API.h"
#include "GenSnd.h"
#include "GenPriv.h"
#include "X_Formats.h"
#include "HAE_API.h"
#include "HAEPrivate.h"

#define USE_LINKED_OBJECTS			0		// if 1, then all objects will be linked

#define USE_BANK_TEST				0

// variables
#if 0
#pragma mark ### variables ###
#endif

static char				audioSetup = 0;			// audio will only be setup once
static XFILE			thePatchFile = NULL;	// current audio patch library
static XShortResourceID	midiSongCount = 0;		// everytime a new song is loaded, this is increments
// this is used as an ID for song callbacks and such

// private functions

#if 0
#pragma mark ### Support functions ###
#endif

// Read a file into memory and return an allocated pointer
static XPTR PV_GetFileAsData(XFILENAME *pFile, long *pSize)
{
    XPTR	data;

    if (XGetFileAsData(pFile, &data, pSize))
	{
	    data = NULL;
	}
    return data;
}

static INLINE XFILE PV_GetIndexedFile(XFILE *fileList, unsigned long fileIndex)
{
    if (fileList)
	{
	    return fileList[fileIndex];
	}
    return NULL;
}

static INLINE void PV_SetIndexedFile(XFILE *fileList, unsigned long fileIndex, XFILE file)
{
    if (fileList)
	{
	    fileList[fileIndex] = file;
	}
}

// given an list of xfiles, a count of xfiles, and a file path; open the file, expand the list
// and store the file index. Returns NULL if file fails to open, or memory allocation failure
static XFILE * PV_OpenToFileList(XFILE * files, unsigned long fileCount, HAEPathName pAudioPathName)
{
    XFILENAME		theFile;
    XFILE			*newFileList;
    unsigned long	size;
    XFILE			file;

    newFileList = NULL;
    // everythings ok, so open the file
    XConvertNativeFileToXFILENAME(pAudioPathName, &theFile);
    file = XFileOpenResource(&theFile, TRUE);
    if (file)
	{
	    XFileUseThisResourceFile(file);
	    size = sizeof(XFILE) * (fileCount + 1);
	    newFileList = (XFILE *)XNewPtr(size);
	    if (newFileList)
		{
		    if (files)
			{
			    XBlockMove(files, newFileList, size - sizeof(XFILE));
			    XDisposePtr(files);
			}
	
		    PV_SetIndexedFile(newFileList, fileCount, file);
		}
	}
    return newFileList;
}

// given a list of xfiles, a count of xfiles, and a xfile index to delete. shrink the list, close the file and remove
// from list
static XFILE * PV_CloseFromFileList(XFILE * files, unsigned long fileCount, unsigned long thisFileIndex)
{
    unsigned long	size, count, count2;
    XFILE			*newFileList;
    XFILE			xfile, file;

    newFileList = files;	
    if (thisFileIndex < fileCount)
	{
	    xfile = PV_GetIndexedFile(files, thisFileIndex);
	    if (xfile)
		{
		    XFileClose(xfile);
		}
	    if (fileCount > 0)
		{	// something in the list
		    fileCount--;
		    size = sizeof(XFILE) * fileCount;
		    newFileList = (XFILE *)XNewPtr(size);
		    if (newFileList)
			{
				// copy all except file to remove
			    for (count = 0, count2 = 0; count < fileCount; count++)
				{
				    file = PV_GetIndexedFile(files, count);
				    if (xfile != file)
					{
					    PV_SetIndexedFile(newFileList, count2++, file);
					}
				}
			}
		}
	    else
		{	// empty, so return empty list
		    newFileList = NULL;
		}
	}
    return newFileList;
}

// close all files in file list, and delete memory
static void PV_CloseAllFromFileList(XFILE * files, unsigned long fileCount)
{
    unsigned long	count;
    XFILE			file;

    if (fileCount > 0)
	{	// something in the list
	    // copy all except file to remove
	    for (count = 0; count < fileCount; count++)
		{
		    file = PV_GetIndexedFile(files, count);
		    if (file)
			{
			    XFileClose(file);
			}
		}
	    XDisposePtr((XPTR)files);
	}
}

static const ReverbMode translateInternal[] = {
    REVERB_NO_CHANGE,
    REVERB_TYPE_1,
    REVERB_TYPE_2,
    REVERB_TYPE_3,
    REVERB_TYPE_4,
    REVERB_TYPE_5,
    REVERB_TYPE_6,
    REVERB_TYPE_7,
    REVERB_TYPE_8,
    REVERB_TYPE_9,
    REVERB_TYPE_10,
    REVERB_TYPE_11
};
static const HAEReverbMode translateExternal[] = {
    HAE_REVERB_NO_CHANGE,
    HAE_REVERB_TYPE_1,
    HAE_REVERB_TYPE_2,
    HAE_REVERB_TYPE_3,
    HAE_REVERB_TYPE_4,
    HAE_REVERB_TYPE_5,
    HAE_REVERB_TYPE_6,
    HAE_REVERB_TYPE_7,
    HAE_REVERB_TYPE_8,
    HAE_REVERB_TYPE_9,
    HAE_REVERB_TYPE_10,
    HAE_REVERB_TYPE_11
};
// translate reverb types from HAEReverbMode to ReverbMode
ReverbMode HAE_TranslateFromHAEReverb(HAEReverbMode igorVerb)
{
    ReverbMode				r;
    short int				count;

    r = REVERB_TYPE_1;
    for (count = 0; count < MAX_REVERB_TYPES; count++)
	{
	    if (igorVerb == translateExternal[count])
		{
		    r = translateInternal[count];
		    break;
		}
	}
    return r;
}

// translate reverb types to HAEReverbMode from ReverbMode
HAEReverbMode HAE_TranslateToHAEReverb(ReverbMode r)
{
    HAEReverbMode			igorVerb;
    short int				count;

    igorVerb = HAE_REVERB_TYPE_1;
    for (count = 0; count < MAX_REVERB_TYPES; count++)
	{
	    if (r == translateInternal[count])
		{
		    igorVerb = translateExternal[count];
		    break;
		}
	}
    return igorVerb;
}

static const HAEErr translateExternalError[] = {
    HAE_NO_ERROR,
    HAE_BUFFER_TO_SMALL,
    HAE_NOT_SETUP,
    HAE_PARAM_ERR,
    HAE_MEMORY_ERR,
    HAE_BAD_INSTRUMENT,
    HAE_BAD_MIDI_DATA,
    HAE_ALREADY_PAUSED,
    HAE_ALREADY_RESUMED,
    HAE_DEVICE_UNAVAILABLE,
    HAE_STILL_PLAYING,
    HAE_NO_SONG_PLAYING,
    HAE_TOO_MANY_SONGS_PLAYING,
    HAE_NO_VOLUME,
    HAE_NO_FREE_VOICES,
    HAE_STREAM_STOP_PLAY,
    HAE_BAD_FILE_TYPE,
    HAE_GENERAL_BAD,
    HAE_BAD_SAMPLE,
    HAE_BAD_FILE,
    HAE_NOT_REENTERANT,
    HAE_SAMPLE_TO_LARGE,
    HAE_UNSUPPORTED_HARDWARE
};


static const OPErr translateInternalError[] = {
    NO_ERR,
    BUFFER_TO_SMALL,
    NOT_SETUP,
    PARAM_ERR,
    MEMORY_ERR,
    BAD_INSTRUMENT,
    BAD_MIDI_DATA,
    ALREADY_PAUSED,
    ALREADY_RESUMED,
    DEVICE_UNAVAILABLE,
    STILL_PLAYING,
    NO_SONG_PLAYING,
    TOO_MANY_SONGS_PLAYING,
    NO_VOLUME,
    NO_FREE_VOICES,
    STREAM_STOP_PLAY,
    BAD_FILE_TYPE,
    GENERAL_BAD,
    BAD_SAMPLE,
    BAD_FILE,
    NOT_REENTERANT,
    SAMPLE_TO_LARGE,
    UNSUPPORTED_HARDWARE
};
										
// Translate from OPErr to HAEErr
HAEErr HAE_TranslateOPErr(OPErr theErr)
{
    HAEErr		igorErr;
    short int	count,  max;

    igorErr = HAE_GENERAL_ERR;
    max = sizeof(translateExternalError) / sizeof(HAEErr);
    for (count = 0; count < max; count++)
	{
	    if (translateInternalError[count] == theErr)
		{
		    igorErr = translateExternalError[count];
		    break;
		}
	}
    return igorErr;
}


// Translate from HAEErr to OPErr
OPErr HAE_TranslateHAErr(HAEErr theErr)
{
    OPErr		igorErr;
    short int	count,  max;

    igorErr = GENERAL_BAD;
    max = sizeof(translateExternalError) / sizeof(HAEErr);
    for (count = 0; count < max; count++)
	{
	    if (translateExternalError[count] == theErr)
		{
		    igorErr = translateInternalError[count];
		    break;
		}
	}
    return igorErr;
}




#if USE_HIGHLEVEL_FILE_API != FALSE
static AudioFileType HAE_TranslateHAEFileType(HAEFileType fileType)
{
    AudioFileType	haeFileType;

    haeFileType = FILE_INVALID_TYPE;
    switch (fileType)
	{
	case HAE_AIFF_TYPE:
	    haeFileType = FILE_AIFF_TYPE;
	    break;
	case HAE_WAVE_TYPE:
	    haeFileType = FILE_WAVE_TYPE;
	    break;
#if USE_MPEG_DECODER != FALSE
	case HAE_MPEG_TYPE:
	    haeFileType = FILE_MPEG_TYPE;
	    break;
#endif
	case HAE_AU_TYPE:
	    haeFileType = FILE_AU_TYPE;
	    break;
	}
    return haeFileType;
}
#endif

#if X_PLATFORM == X_WINDOWS
/*

For Windows, you may need to pass in the
	window name of the application window. If you do, you can pass in a partial 'C' string
	name. If you pass NULL, it will search for an open window, but Open may fail. For all
	other platforms, you should pass NULL.
	
static char *pWindowName;
static HAE_BOOL CALLBACK PV_EnumWndProc( HWND hWnd, LPARAM lParam )
{
    char szWindowName[256];

    // Get the window name and class name
    //...................................
    GetWindowText( hWnd, szWindowName, 255 );    // This should be 11
	// Scan for the Netscape Window
	if (XStrStr(szWindowName, pWindowName))
	{
		HWND	*pFoundWindow	=	(HWND *)lParam;
		
		if (pFoundWindow)
		{
			// Got it
			*pFoundWindow	=	hWnd;
		}
	}
	// Haven't found it yet.
    return( TRUE);
}
		if (pPlatformData)
		{
			pWindowName = (char *)pPlatformData;
			// Enumerate all top-level windows currently running 
			// and see if it matches our 'C' name passed in.
			EnumWindows((WNDENUMPROC)PV_EnumWndProc, (LPARAM)&w95Init.hwndOwner );
			if (w95Init.hwndOwner)
	        {
				// Make Window the Active Window
				EnableWindow(w95Init.hwndOwner, TRUE);
				SetForegroundWindow(w95Init.hwndOwner);
				SetActiveWindow(w95Init.hwndOwner);
			}
			else
			{
				pPlatformData = NULL;	// didn't find a named window, so look for current
			}
		}
		if (pPlatformData == NULL)
*/

#endif

// Class implemention for HAEAudio

#if 0
#pragma mark ### HAEAudioMixer class ###
#endif

HAEAudioMixer::HAEAudioMixer()
{
    mWritingToFile = FALSE;
    mWritingToFileReference = NULL;
    iMidiVoices = 0;
    iSoundVoices = 0;
    iMixLevel = 0;
    iQuality = HAE_22K;
    iTerpMode = HAE_LINEAR_INTERPOLATION;
    iReverbMode = HAE_REVERB_TYPE_4;
    iModifiers = HAE_USE_STEREO | HAE_USE_16;
    pTop = NULL;
    pTask = NULL;
    songNameCount = 0;
    songNameType = HAE_GROOVOID;
    sampleNameCount = 0;
    instrumentNameCount = 0;
    mOpenAudioFileCount = 0;
    mOpenAudioFiles = NULL;
    mIsAudioEngaged = FALSE;
    mCacheStatus = TRUE;
}

HAEAudioMixer::~HAEAudioMixer()
{
    Close();
}

HAE_BOOL HAEAudioMixer::IsOpen(void) const
{
    return audioSetup ? (HAE_BOOL)TRUE : (HAE_BOOL)FALSE;
}

// return number of devices available to this mixer hardware
// will return a number from 1 to max number of devices.
// ie. a value of 2 means two devices
long HAEAudioMixer::GetMaxDeviceCount(void)
{
#if USE_DEVICE_ENUM_SUPPORT == TRUE
    return GM_MaxDevices();
#else
    return 0;
#endif
}

// set current device. should be a number from 0 to HAEAudioMixer::GetDeviceCount()
void HAEAudioMixer::SetCurrentDevice(long deviceID, void *deviceParameter)
{
#if USE_DEVICE_ENUM_SUPPORT == TRUE
    if (deviceID < GetMaxDeviceCount())
	{
	    if (IsOpen())
		{
		    DisengageAudio();		// shutdown from hardware
		}
	    GM_SetDeviceID(deviceID, deviceParameter);	// change to new device
	    if (IsOpen())
		{
		    ReengageAudio();		// connect back to audio with new device
		}
	}
#else
    deviceID = deviceID;
    deviceParameter = deviceParameter;
#endif
}

// get current device.
long HAEAudioMixer::GetCurrentDevice(void *deviceParameter)
{
    return GM_GetDeviceID(deviceParameter);
}

// get device name
// NOTE:	This function needs to function before any other calls may have happened.
//			Format of string is a zero terminated comma delinated C string.
//			"platform,method,misc"
//	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
//			"WinOS,DirectSound,multi threaded"
//			"WinOS,waveOut,multi threaded"
//			"WinOS,VxD,low level hardware"
//			"WinOS,plugin,Director"
void HAEAudioMixer::GetDeviceName(long deviceID, char *cName, unsigned long cNameLength)
{
#if USE_DEVICE_ENUM_SUPPORT == TRUE
    GM_GetDeviceName(deviceID, cName, cNameLength);
#else
    deviceID = deviceID;
    cName = cName;
    cNameLength = cNameLength;
#endif
}

HAEErr HAEAudioMixer::Open(HAEPathName pAudioPathName,
			   HAEQuality q, HAETerpMode t, 
			   HAEReverbMode r, HAEAudioModifiers am,
			   short int maxSongVoices,
			   short int maxSoundVoices,
			   short int mixLevel,
			   HAE_BOOL engageAudio)
{
    OPErr			theErr;
    Quality			theQuality;
    TerpMode		theTerp;
    AudioModifiers	theMods;
    ReverbMode		theReverb;

    theErr = NO_ERR;
    // if we've never setup the audio engine, do that now
    if (audioSetup == 0)
	{		
#if (X_PLATFORM == X_MACINTOSH) && (CPU_TYPE == k68000)
	    // we're running on a MacOS 68k, so we've got to restrict the features in order to get decent playback
	    q = HAE_11K;
	    am &= ~HAE_USE_STEREO;			// mono only
	    am &= ~HAE_STEREO_FILTER;		// don't allow
	    am |= HAE_DISABLE_REVERB;		// don't allow
	    //		am &= ~HAE_USE_16;
	    switch (q)
		{
		case HAE_44K:		// no way
		case HAE_48K:
		case HAE_24K:
		case HAE_22K_TERP_44K:
		    q = HAE_22K;
		    break;
		}
	    t = HAE_DROP_SAMPLE;

	    switch (t)
		{
		case HAE_LINEAR_INTERPOLATION:
		    t = HAE_2_POINT_INTERPOLATION;
		    break;
		}
	    // reverb doesn't matter because we're only using the shallow memory version anyways
	    // and we force a disable
#endif

	    switch (q)
		{
		case HAE_8K:
		    theQuality = Q_8K;
		    break;
		case HAE_11K:
		    theQuality = Q_11K;
		    break;
		case HAE_11K_TERP_22K:
		    theQuality = Q_11K_TERP_22K;
		    break;
		case HAE_22K_TERP_44K:
		    theQuality = Q_22K_TERP_44K;
		    break;
		case HAE_22K:
		    theQuality = Q_22K;
		    break;
		case HAE_24K:
		    theQuality = Q_24K;
		    break;
		case HAE_44K:
		    theQuality = Q_44K;
		    break;
		case HAE_48K:
		    theQuality = Q_48K;
		    break;
		default:
		    theErr = PARAM_ERR;
		    break;
		}
		
	    switch (t)
		{
		case HAE_DROP_SAMPLE:
		    theTerp = E_AMP_SCALED_DROP_SAMPLE;
		    break;
		case HAE_2_POINT_INTERPOLATION:
		    theTerp = E_2_POINT_INTERPOLATION;
		    break;
		case HAE_LINEAR_INTERPOLATION:
		    theTerp = E_LINEAR_INTERPOLATION;
		    break;
		default:
		    theErr = PARAM_ERR;
		    break;
		}

	    switch (r)
		{
		case HAE_REVERB_NO_CHANGE:
		    theReverb = HAE_TranslateFromHAEReverb(GetReverbType());
		    break;
		case HAE_REVERB_TYPE_1:
		    theReverb = REVERB_TYPE_1;
		    break;
		case HAE_REVERB_TYPE_2:
		    theReverb = REVERB_TYPE_2;
		    break;
		case HAE_REVERB_TYPE_3:
		    theReverb = REVERB_TYPE_3;
		    break;
		case HAE_REVERB_TYPE_4:
		    theReverb = REVERB_TYPE_4;
		    break;
		case HAE_REVERB_TYPE_5:
		    theReverb = REVERB_TYPE_5;
		    break;
		case HAE_REVERB_TYPE_6:
		    theReverb = REVERB_TYPE_6;
		    break;
		case HAE_REVERB_TYPE_7:
		    theReverb = REVERB_TYPE_7;
		    break;
		case HAE_REVERB_TYPE_8:
		    theReverb = REVERB_TYPE_8;
		    break;
		case HAE_REVERB_TYPE_9:
		    theReverb = REVERB_TYPE_9;
		    break;
		case HAE_REVERB_TYPE_10:
		    theReverb = REVERB_TYPE_10;
		    break;
		case HAE_REVERB_TYPE_11:
		    theReverb = REVERB_TYPE_11;
		    break;
		default:
		    theErr = PARAM_ERR;
		    break;
		}

	    theMods = M_NONE;
	    if ((am & HAE_USE_16) && XIs16BitSupported())
		{
		    theMods |= M_USE_16;
		}
	    else
		{
		    am &= HAE_USE_16;			// 8 bit
		}

	    if ( (am & HAE_USE_STEREO) && XIsStereoSupported())
		{
		    theMods |= M_USE_STEREO;
		    if (am & HAE_STEREO_FILTER)
			{
			    theMods |= M_STEREO_FILTER;
			}
		}
	    else
		{
		    am &= ~HAE_USE_STEREO;			// mono
		}
	    if (am & HAE_DISABLE_REVERB)
		{
		    theMods |= M_DISABLE_REVERB;
		}

	    if (theErr == NO_ERR)
		{
		    theErr = GM_InitGeneralSound(NULL, theQuality, theTerp, theMods,
						 maxSongVoices,
						 mixLevel,
						 maxSoundVoices);
		    if (theErr == NO_ERR)
			{
			    audioSetup++;	// allocated

			    iMidiVoices = maxSongVoices;
			    iSoundVoices = maxSoundVoices;
			    iMixLevel = mixLevel;

			    iQuality = q;
			    iTerpMode = t;
			    iReverbMode = r;
			    iModifiers = am;

			    GM_SetReverbType(theReverb);

			    if (pAudioPathName)
				{
#if USE_BANK_TEST
				    void		*newFiles;

				    // everythings ok, so open the file
				    newFiles = PV_OpenToFileList((XFILE *)mOpenAudioFiles, mOpenAudioFileCount, pAudioPathName);
				    if (newFiles)
					{
					    mOpenAudioFiles = newFiles;
					    mOpenAudioFileCount++;
					}
				    else
					{
					    theErr = BAD_FILE;
					}
#else
				    XFILENAME		theFile;

				    // everythings ok, so open the file
				    XConvertNativeFileToXFILENAME(pAudioPathName, &theFile);
				    thePatchFile = XFileOpenResource(&theFile, TRUE);
				    if (thePatchFile)
					{
					    XFileUseThisResourceFile(thePatchFile);
					}
				    else
					{
					    theErr = BAD_FILE;
					}
#endif
				}
			    if (theErr == NO_ERR)
				{
				    if (engageAudio)
					{
					    theErr = GM_ResumeGeneralSound(NULL);
					    if (theErr == NO_ERR)
						{
						    audioSetup++;
						    mIsAudioEngaged = TRUE;
						}
					    else
						{
						    mIsAudioEngaged = FALSE;
						}
					}
				}
			}
		}
	}
    else
	{
	    theErr = NOT_REENTERANT;		// can't be reentrant
	}
    return HAE_TranslateOPErr(theErr);
}

void HAEAudioMixer::Close(void)
{
    if (audioSetup > 0)
	{
	    StopOutputToFile();		// just in case

	    GM_SetAudioTask(NULL);

	    if (audioSetup > 1)
		{
		    audioSetup--;
		    // Close up sound manager BEFORE releasing memory!
		    GM_StopHardwareSoundManager(NULL);
		}
	    if (audioSetup > 0)
		{
		    audioSetup--;
		    GM_FinisGeneralSound(NULL);
		}
	}

    if (thePatchFile)
	{
	    XFileClose(thePatchFile);
	    thePatchFile = NULL;
	}
#if USE_BANK_TEST
    if (mOpenAudioFiles)
	{
	    PV_CloseAllFromFileList((XFILE *)mOpenAudioFiles, mOpenAudioFileCount);
	    //		XDisposePtr(mOpenAudioFiles);
	    mOpenAudioFiles = NULL;
	    mOpenAudioFileCount = 0;
	}
#endif
}

// Get default bank URL
HAEErr HAEAudioMixer::GetURLFromAudioFile(char *pURL, unsigned long urlLength)
{
#if USE_CREATION_API == TRUE
    BankStatus	bank;
    HAEErr		theErr;

    theErr = HAE_NO_ERROR;
    if (audioSetup && pURL && urlLength)
	{
	    pURL[0] = 0;
	    XGetBankStatus(&bank);
	    if ((unsigned long)XStrLen(bank.bankURL) == 0)
		{
		    theErr = HAE_BAD_BANK;
		}
	    else
		{
		    if ((unsigned long)XStrLen(bank.bankURL) < (urlLength+1))
			{
			    XStrCpy(pURL, bank.bankURL);
			}
		}
	}
    else
	{
	    theErr = HAE_NOT_SETUP;
	}
    return theErr;
#else
    pURL = pURL;
    urlLength = urlLength;
    return HAE_NOT_SETUP;
#endif
}

// Get default bank name
HAEErr HAEAudioMixer::GetNameFromAudioFile(char *cName, unsigned long cLength)
{
#if USE_CREATION_API == TRUE
    BankStatus	bank;
    HAEErr		theErr;

    theErr = HAE_NO_ERROR;
    if (audioSetup && cName && cLength)
	{
	    cName[0] = 0;
	    XGetBankStatus(&bank);
	    if ((unsigned long)XStrLen(bank.bankName) == 0)
		{
		    theErr = HAE_BAD_BANK;
		}
	    else
		{
		    if ((unsigned long)XStrLen(bank.bankName) < (cLength+1))
			{
			    XStrCpy(cName, bank.bankName);
			}
		}
	}
    else
	{
	    theErr = HAE_NOT_SETUP;
	}
    return theErr;
#else
    cName = cName;
    cLength = cLength;
    return HAE_NOT_SETUP;
#endif
}

// Get version numbers from bank
void HAEAudioMixer::GetVersionFromAudioFile(short int *pVersionMajor, short int *pVersionMinor, short int *pVersionSubMinor)
{
    XVersion	vers;

    if (pVersionMajor && pVersionMinor && pVersionSubMinor)
	{
	    *pVersionMajor = 0;
	    *pVersionMinor = 0;
	    *pVersionSubMinor = 0;
	    if (audioSetup)
		{
		    XGetVersionNumber(&vers);
		    *pVersionMajor = vers.versionMajor;
		    *pVersionMinor = vers.versionMinor;
		    *pVersionSubMinor = vers.versionSubMinor;
		}
	}
}

// Get names of songs that are included in the audio file. Call successively until
// the name is zero
void HAEAudioMixer::GetSongNameFromAudioFile(char *cSongName, 
					     long *pID, HAEFileType *pSongType)
{
    long		size;
    XPTR		pData;
    long		id;

    if (audioSetup && cSongName)
	{
	    cSongName[0] = 0;
	    pData = NULL;
	    if (songNameType == HAE_GROOVOID)
		{
		    pData = XGetIndexedResource(ID_SONG, &id, songNameCount, cSongName, &size);
		    if (pData == NULL)
			{
			    songNameType = HAE_RMF;
			    songNameCount = 0;
			}
		}
	    if (songNameType == HAE_RMF)
		{
		    pData = XGetIndexedResource(ID_RMF, &id, songNameCount, cSongName, &size);
		    if (pData == NULL)
			{
			    songNameType = HAE_GROOVOID;
			    songNameCount = 0;
			}
		}
	    if (pData)
		{
		    XPtoCstr(cSongName);
		    XDisposePtr(pData);
		    songNameCount++;
		    if (pID)
			{
			    *pID = id;
			}
		    if (pSongType)
			{
			    *pSongType = songNameType;
			}
		}
	}
}

// Get names of samples that are included in the audio file. Call successively until
// the name is zero
void HAEAudioMixer::GetSampleNameFromAudioFile(char *cSampleName, long *pID)
{
    long		size;
    XPTR		pData;

    if (cSampleName && pID)
	{
	    cSampleName[0] = 0;
	    if (audioSetup)
		{
		    // look for compressed version first
		    pData = XGetIndexedResource(ID_CSND, pID, sampleNameCount, cSampleName, &size);
		    if (pData == NULL)
			{
				// look for standard version
			    pData = XGetIndexedResource(ID_SND, pID, sampleNameCount, cSampleName, &size);

			    if (pData == NULL)
				{
				    // look for encrypted version
				    pData = XGetIndexedResource(ID_ESND, pID, sampleNameCount, cSampleName, &size);
				}
			}
		    if (pData)
			{
			    XPtoCstr(cSampleName);
			    XDisposePtr(pData);
			    sampleNameCount++;
			}
		    else
			{
			    sampleNameCount = 0;
			    cSampleName[0] = 0;
			}
		}
	}
}

// Get names of instruments that are included in the audio file. Call successively until
// the name is zero
void HAEAudioMixer::GetInstrumentNameFromAudioFile(char *cInstrumentName, long *pID)
{
    long		size;
    XPTR		pData;

    if (cInstrumentName && pID)
	{
	    cInstrumentName[0] = 0;
	    if (audioSetup)
		{
		    pData = XGetIndexedResource(ID_INST, pID, instrumentNameCount, cInstrumentName, &size);
		    if (pData)
			{
			    XPtoCstr(cInstrumentName);
			    XDisposePtr(pData);
			    instrumentNameCount++;
			}
		    else
			{
			    instrumentNameCount = 0;
			    cInstrumentName[0] = 0;
			}
		}
	}
}

// Get names of instruments that are included in the audio file, referenced by ID only. Will return
// and error if instrument not found.
HAEErr HAEAudioMixer::GetInstrumentNameFromAudioFileFromID(char *cInstrumentName, long theID)
{
    HAEErr		theErr;

    theErr = HAE_NO_ERROR;
    if (cInstrumentName)
	{
	    cInstrumentName[0] = 0;
	    if (audioSetup)
		{
		    XGetResourceName(ID_INST, theID, cInstrumentName);
		    if (cInstrumentName[0] == 0)
			{
			    theErr = HAE_BAD_INSTRUMENT;
			}
		}
	}
    return theErr;
}

// Verify that the file passed in, is a valid audio bank file for HAE
HAEErr HAEAudioMixer::ValidateAudioFile(HAEPathName pAudioPathName)
{
    HAEErr		theErr;
    XFILENAME	theFile;
    XFILE		file;
    long		size;
    XVersion	*pData;

    theErr = HAE_BAD_BANK;
    XConvertNativeFileToXFILENAME(pAudioPathName, &theFile);
    file = XFileOpenResource(&theFile, TRUE);
    if (file)
	{
	    XFileUseThisResourceFile(file);
	    pData = (XVersion *)XGetAndDetachResource(ID_VERS, 0, &size);
	    if (pData)
		{
		    XDisposePtr(pData);
		}
	    theErr = HAE_NO_ERROR;
	    XFileClose(file);
	}
    return theErr;
}

// Flush read in or created cache for current AudioFile. TRUE allows cache, FALSE does not.
void HAEAudioMixer::SetCacheStatusForAudioFile(HAE_BOOL enableCache)
{
    mCacheStatus = enableCache;
}

HAE_BOOL HAEAudioMixer::GetCacheStatusForAudioFile(void)
{
    return mCacheStatus;
}

// Change audio file to use the passed in XFILE
HAEErr HAE_UseThisFile(XFILE audioFile, XBOOL closeOldFile)
{
    OPErr			theErr;
    XFILE			oldPatchFile;

    theErr = NO_ERR;
    oldPatchFile = thePatchFile;
    if (audioSetup)
	{
	    thePatchFile = audioFile;
	    XFileUseThisResourceFile(thePatchFile);

	    if (closeOldFile)
		{
		    // close old one, if different
		    if (oldPatchFile)
			{
			    if (oldPatchFile != audioFile)
				{
				    XFileClose(oldPatchFile);
				}
			}
		}
	}
    else
	{
	    thePatchFile = oldPatchFile;	// restore old file
	    theErr = BAD_FILE;
	}
    return HAE_TranslateOPErr(theErr);
}

// change audio file
HAEErr HAEAudioMixer::ChangeAudioFile(HAEPathName pAudioPathName)
{
    OPErr			theErr;

    theErr = NO_ERR;
    if (audioSetup)
	{
#if USE_BANK_TEST
	    void			*newFiles;

	    PV_CloseAllFromFileList((XFILE *)mOpenAudioFiles, mOpenAudioFileCount);
	    mOpenAudioFileCount = 0;
	    // everythings ok, so open the file
	    newFiles = PV_OpenToFileList((XFILE *)mOpenAudioFiles, mOpenAudioFileCount, pAudioPathName);
	    if (newFiles)
		{
		    mOpenAudioFiles = newFiles;
		    mOpenAudioFileCount++;
		}
	    else
		{
		    theErr = BAD_FILE;
		}
#else
	    XFILENAME		theFile;
	    XFILE			oldPatchFile;

	    if (pAudioPathName)
		{
		    oldPatchFile = thePatchFile;
		    XConvertNativeFileToXFILENAME(pAudioPathName, &theFile);
		    thePatchFile = XFileOpenResource(&theFile, TRUE);
		    if (thePatchFile)
			{
			    XFileUseThisResourceFile(thePatchFile);
				// close old one, if different
			    if (oldPatchFile)
				{
				    if (oldPatchFile != thePatchFile)
					{
					    XFileClose(oldPatchFile);
					}
				}
			    if (mCacheStatus == FALSE)	// don't allow cache
				{
				    XFileFreeResourceCache(thePatchFile);	// free cache
				}
			}
		    else
			{
			    thePatchFile = oldPatchFile;	// restore old file
			    theErr = BAD_FILE;
			}
		}
	    else
		{
		    if (thePatchFile)
			{
			    XFileClose(thePatchFile);
			    thePatchFile = NULL;
			}
		}
#endif
	}
    else
	{
	    theErr = MEMORY_ERR;		// can't be reentrant
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAEAudioMixer::ChangeAudioFileToMemory(void * pAudioFile, unsigned long fileSize)
{
    OPErr			theErr;
    XFILE			oldPatchFile;

    theErr = NO_ERR;
    if (audioSetup)
	{
	    if (pAudioFile)
		{
		    oldPatchFile = thePatchFile;
		    thePatchFile = XFileOpenResourceFromMemory(pAudioFile, fileSize, FALSE);
		    if (thePatchFile)
			{
			    XFileUseThisResourceFile(thePatchFile);
				// close old one, if different
			    if (oldPatchFile)
				{
				    if (oldPatchFile != thePatchFile)
					{
					    XFileClose(oldPatchFile);
					}
				}
			}
		    else
			{
			    thePatchFile = oldPatchFile;	// restore old file
			    theErr = BAD_FILE;
			}
		}
	    else
		{
		    if (thePatchFile)
			{
			    XFileClose(thePatchFile);
			    thePatchFile = NULL;
			}
		}
	}
    else
	{
	    theErr = MEMORY_ERR;		// can't be reentrant
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAEAudioMixer::ChangeAudioModes(HAEQuality q, HAETerpMode t, HAEAudioModifiers am)
{
    OPErr			theErr;
    Quality			theQuality;
    TerpMode		theTerp;
    AudioModifiers	theMods;

    theErr = NO_ERR;
    switch (q)
	{
	case HAE_8K:
	    theQuality = Q_8K;
	    break;
	case HAE_11K_TERP_22K:
	    theQuality = Q_11K_TERP_22K;
	    break;
	case HAE_22K_TERP_44K:
	    theQuality = Q_22K_TERP_44K;
	    break;
	case HAE_11K:
	    theQuality = Q_11K;
	    break;
	case HAE_22K:
	    theQuality = Q_22K;
	    break;
	case HAE_24K:
	    theQuality = Q_24K;
	    break;
	case HAE_44K:
	    theQuality = Q_44K;
	    break;
	case HAE_48K:
	    theQuality = Q_48K;
	    break;
	default:
	    theErr = PARAM_ERR;
	    break;
	}
		
    switch (t)
	{
	case HAE_DROP_SAMPLE:
	    theTerp = E_AMP_SCALED_DROP_SAMPLE;
	    break;
	case HAE_2_POINT_INTERPOLATION:
	    theTerp = E_2_POINT_INTERPOLATION;
	    break;
	case HAE_LINEAR_INTERPOLATION:
	    theTerp = E_LINEAR_INTERPOLATION;
	    break;
	default:
	    theErr = PARAM_ERR;
	    break;
	}

    theMods = M_NONE;
    if ((am & HAE_USE_16) && XIs16BitSupported())
	{
	    theMods |= M_USE_16;
	}
    else
	{
	    am &= ~HAE_USE_16;	// 8 bit
	}
    if ( (am & HAE_USE_STEREO) && XIsStereoSupported())
	{
	    theMods |= M_USE_STEREO;
	    if (am & HAE_STEREO_FILTER)
		{
		    theMods |= M_STEREO_FILTER;
		}
	}
    else
	{
	    am &= ~HAE_USE_STEREO;	// mono
	}
    if (am & HAE_DISABLE_REVERB)
	{
	    theMods |= M_DISABLE_REVERB;
	}
    if (theErr == NO_ERR)
	{
	    theErr = GM_ChangeAudioModes(NULL, theQuality, theTerp, theMods);
	    if (theErr == NO_ERR)
		{
		    iQuality = q;
		    iTerpMode = t;
		    iModifiers = am;
		}
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAEAudioMixer::ChangeSystemVoices(	short int maxSongVoices,
						short int maxSoundVoices,
						short int mixLevel)
{
    OPErr	theErr;

    theErr = GM_ChangeSystemVoices(maxSongVoices, mixLevel, maxSoundVoices);
	
    if (theErr == NO_ERR)
	{
	    iMidiVoices = maxSongVoices;
	    iSoundVoices = maxSoundVoices;
	    iMixLevel = mixLevel;
	}
    return HAE_TranslateOPErr(theErr);
}

// return time in 60ths of a second
static unsigned long PV_GetTick(void)
{
#if USE_FLOAT != FALSE
    double time;
	
    time = (double)XMicroseconds() / 16666.7;
    return (unsigned long)time;
#else
    return (XMicroseconds() * 10) / 166667;
#endif
}

// Performance testing code
unsigned long HAEAudioMixer::MeasureCPUPerformance(void)
{
    unsigned long	count, loops;
    unsigned long	postTicks;

    //	Wait to top of tick count
    postTicks = PV_GetTick();
    while (postTicks == PV_GetTick()) {};

    loops = 60L * 2;	// 2 seconds
    //	count as fast as possible for counts per ticks
    count = 0;
    postTicks = PV_GetTick() + loops;
    while (PV_GetTick() <= postTicks)
	{
	    count++;
	}
    return count / loops;
}

// reconfigure, by changing sample rates, interpolation modes, bit depth, etc based
// upon performance of host cpu
void HAEAudioMixer::PerformanceConfigure(void)
{
    unsigned long			raw_ticks, engaged_ticks, percentage;
    long					count;
    HAEQuality				q;
    HAETerpMode				t;
    HAEAudioModifiers		am;
    // under 10 is a very fast CPU
    static unsigned long	cpuGauge[] = {10, 15, 20, 25, 30, 35, 40, 45, 50, 55};

    if (audioSetup)
	{
	    DisengageAudio();		// disenguage from hardware
	    raw_ticks = MeasureCPUPerformance();
	    ReengageAudio();		// enguage from hardware
	    engaged_ticks = MeasureCPUPerformance();

	    percentage = 100L - ((engaged_ticks * 100L) / raw_ticks);

	    for (count = 0; count < 10; count++)
		{
		    if (percentage < cpuGauge[count])
			{
			    break;
			}
		}
	    switch (10-count)
		{
		default:
		    count = -1;
		    break;
		case 10:			// 10 %
		    q = HAE_22K;
		    t = HAE_LINEAR_INTERPOLATION;
		    am = HAE_USE_16 | HAE_USE_STEREO | HAE_STEREO_FILTER;
		    break;
		case 9:				// 15 %
		    q = HAE_22K;
		    t = HAE_LINEAR_INTERPOLATION;
		    am = HAE_USE_16 | HAE_USE_STEREO;
		    break;			
		case 8:				// 20 %
		    q = HAE_22K;
		    t = HAE_LINEAR_INTERPOLATION;
		    am = HAE_USE_16;
		    break;
		case 7:				// 25 %
		    q = HAE_22K;
		    t = HAE_LINEAR_INTERPOLATION;
		    am = HAE_USE_16 | HAE_DISABLE_REVERB;
		    break;
		case 6:				// 30 %
		    q = HAE_22K;
		    t = HAE_LINEAR_INTERPOLATION;
		    am = HAE_USE_16 | HAE_DISABLE_REVERB;
		    break;
		case 5:				// 35 %
		    q = HAE_22K;
		    t = HAE_2_POINT_INTERPOLATION;
		    am = HAE_USE_16 | HAE_DISABLE_REVERB;
		    break;
		case 4:				// 40 %
		    q = HAE_22K;
		    t = HAE_DROP_SAMPLE;
		    am = HAE_USE_16 | HAE_DISABLE_REVERB;
		    break;
		case 3:				// 45 %
		    q = HAE_22K;
		    t = HAE_DROP_SAMPLE;
		    am = HAE_DISABLE_REVERB;
		    break;
		case 2:				// 50 %
		    q = HAE_22K;
		    t = HAE_DROP_SAMPLE;
		    am = HAE_DISABLE_REVERB;
		    break;
		case 1:				// 55 %
		    q = HAE_11K;
		    t = HAE_DROP_SAMPLE;
		    am = HAE_DISABLE_REVERB;
		    break;
		}
	    if (count != -1)
		{
		    ChangeAudioModes(q, t, am);
		}
	}
}

HAEReverbMode HAEAudioMixer::GetReverbType(void)
{
    ReverbMode		r;
    HAEReverbMode	verb;

    r = GM_GetReverbType();
    verb = HAE_TranslateToHAEReverb(r);
    SetReverbType(verb);
    return iReverbMode;
}


void HAEAudioMixer::SetReverbType(HAEReverbMode verb)
{
    ReverbMode	r;

    r = HAE_TranslateFromHAEReverb(verb);
    iReverbMode = HAE_TranslateToHAEReverb(r);

    GM_SetReverbType(r);
}

unsigned long HAEAudioMixer::GetTick(void)
{
    return GM_GetSyncTimeStamp();
}

unsigned long HAEAudioMixer::GetAudioLatency(void)
{
    return GM_GetSyncTimeStampQuantizedAhead() - GM_GetSyncTimeStamp();
}

void HAEAudioMixer::SetMasterVolume(HAE_UNSIGNED_FIXED theVolume)
{
    GM_SetMasterVolume((INT32)(UNSIGNED_FIXED_TO_LONG_ROUNDED(theVolume * MAX_MASTER_VOLUME)));
}

HAE_UNSIGNED_FIXED HAEAudioMixer::GetMasterVolume(void) const
{
    HAE_UNSIGNED_FIXED	value;

    value = UNSIGNED_RATIO_TO_FIXED(GM_GetMasterVolume(), MAX_MASTER_VOLUME);
    return value;
}

void HAEAudioMixer::SetHardwareVolume(HAE_UNSIGNED_FIXED theVolume)
{
    XSetHardwareVolume(FIXED_TO_SHORT_ROUNDED(theVolume * X_FULL_VOLUME));
}

HAE_UNSIGNED_FIXED HAEAudioMixer::GetHardwareVolume(void) const
{
    HAE_UNSIGNED_FIXED	value;

    value = UNSIGNED_RATIO_TO_FIXED(XGetHardwareVolume(), X_FULL_VOLUME);
    return value;
}

// get and set the hardware balance. Use -1.0 for full left, and 1.0 for full
// right; use 0 for center
void HAEAudioMixer::SetHardwareBalance(HAE_FIXED balance)
{
    HAE_SetHardwareBalance(FIXED_TO_LONG(balance * X_FULL_VOLUME));
}

HAE_FIXED HAEAudioMixer::GetHardwareBalance(void) const
{
    HAE_FIXED	balance;

    balance = RATIO_TO_FIXED(HAE_GetHardwareBalance(), X_FULL_VOLUME);
    return balance;
}


void HAEAudioMixer::SetMasterSoundEffectsVolume(HAE_UNSIGNED_FIXED theVolume)
{
    GM_SetEffectsVolume(FIXED_TO_SHORT_ROUNDED(theVolume * MAX_MASTER_VOLUME));
}

HAE_UNSIGNED_FIXED HAEAudioMixer::GetMasterSoundEffectsVolume(void) const
{
    HAE_UNSIGNED_FIXED	value;

    value = UNSIGNED_RATIO_TO_FIXED(GM_GetEffectsVolume(), MAX_MASTER_VOLUME);
    return value;
}

// display feedback information
// This will return the number of samples stored into the pLeft and pRight
// arrays. Usually 1024. This returns the current data points being sent
// to the hardware.
short int HAEAudioMixer::GetAudioSampleFrame(short int *pLeft, short int *pRight) const
{
    return GM_GetAudioSampleFrame(pLeft, pRight);
}

// Get realtime information about the current synthisizer state
void HAEAudioMixer::GetRealtimeStatus(HAEAudioInfo *pStatus) const
{
    GM_AudioInfo	status;
    short int		count;
    HAEVoiceType	voiceType;

    if (pStatus)
	{
	    GM_GetRealtimeAudioInformation(&status);
	    XSetMemory(pStatus, (long)sizeof(HAEAudioInfo), 0);
	    pStatus->voicesActive = status.voicesActive;
	    for (count = 0; count < status.voicesActive; count++)
		{
		    pStatus->voice[count] = status.voice[count];

		    voiceType = HAE_UNKNOWN;
		    switch (status.voiceType[count])
			{
			case MIDI_PCM_VOICE:
			    voiceType = HAE_MIDI_PCM_VOICE;
			    break;
			case SOUND_PCM_VOICE:
			    voiceType = HAE_SOUND_PCM_VOICE;
			    break;
			}
		    pStatus->voiceType[count] = voiceType;
		    pStatus->instrument[count] = status.patch[count];
		    pStatus->scaledVolume[count] = status.scaledVolume[count];
		    pStatus->midiVolume[count] = status.volume[count];
		    pStatus->channel[count] = status.channel[count];
		    pStatus->midiNote[count] = status.midiNote[count];
		    if (status.pSong[count])
			{
			    pStatus->userReference[count] = status.pSong[count]->userReference;
			}
		}
	}
}

// is the mixer connected to the audio hardware
HAE_BOOL HAEAudioMixer::IsAudioEngaged(void)
{
    return mIsAudioEngaged;
}

// disengage from audio hardware
void HAEAudioMixer::DisengageAudio(void)
{
    if (GM_PauseGeneralSound(NULL) == NO_ERR)
	{
	    mIsAudioEngaged = FALSE;
	    if (audioSetup > 1)
		{
		    audioSetup--;
		}
	}
}

// reengage to audio hardware
void HAEAudioMixer::ReengageAudio(void)
{
    if (GM_ResumeGeneralSound(NULL) == NO_ERR)
	{
	    mIsAudioEngaged = TRUE;
	    if (audioSetup == 1)
		{
		    audioSetup++;
		}
	}
}

// Call this during idle time to service audio streams, fades, and other idle
// time processes
void HAEAudioMixer::ServiceIdle(void)
{
#if USE_LINKED_OBJECTS
    HAEAudioNoise	*pHAETop;

    // time to check fading objects
    pHAETop = (HAEAudioNoise *)pTop;
    while (pHAETop)
	{
	    // do something interesting
	    pHAETop = pHAETop->pNext;
	}
#endif
#if USE_STREAM_API
    GM_AudioStreamService(NULL);
#endif
}

// get in realtime CPU load in microseconds used to create 11 ms worth
// of sample data.
unsigned long HAEAudioMixer::GetCPULoadInMicroseconds(void)
{
    return GM_GetMixerUsedTime();
}

// get in realtime CPU load in percent used to create 11 ms worth
// of sample data.
unsigned long HAEAudioMixer::GetCPULoadInPercent(void)
{
    return GM_GetMixerUsedTimeInPercent();
}


HAEAudioModifiers HAEAudioMixer::GetModifiers(void)
{
    return iModifiers;
}

HAETerpMode HAEAudioMixer::GetTerpMode(void)
{
    return iTerpMode;
}

HAEQuality HAEAudioMixer::GetQuality(void)
{
    return iQuality;
}

short int HAEAudioMixer::GetMidiVoices(void)
{
    INT16 song, mix, sound;

    GM_GetSystemVoices(&song, &mix, &sound);
    iMidiVoices = song;
    return iMidiVoices;
}

short int HAEAudioMixer::GetSoundVoices(void)
{
    INT16 song, mix, sound;

    GM_GetSystemVoices(&song, &mix, &sound);
    iSoundVoices = sound;
    return iSoundVoices;
}

short int HAEAudioMixer::GetMixLevel(void)
{
    INT16 song, mix, sound;

    GM_GetSystemVoices(&song, &mix, &sound);
    iMixLevel = mix;
    return iMixLevel;
}



// Set a interrupt level task callback
void HAEAudioMixer::SetTaskCallback(HAETaskCallbackPtr pCallback)
{
    pTask = pCallback;
}

// start the task callback
void HAEAudioMixer::StartTaskCallback(void)
{
    GM_SetAudioTask((GM_AudioTaskCallbackPtr)pTask);
}

// Stop the task callback
void HAEAudioMixer::StopTaskCallback(void)
{
    GM_SetAudioTask(NULL);
}

// Set a output callback. This will call your output code. This is used to modify the
// sample output before its sent to the hardware. Be very careful here. Don't use to
// much time or the audio will skip
void HAEAudioMixer::SetOutputCallback(HAEOutputCallbackPtr pCallback)
{
    pOutput = pCallback;
}

// start the output callback
void HAEAudioMixer::StartOutputCallback(void)
{
    GM_SetAudioOutput((GM_AudioOutputCallbackPtr)pOutput);
}

// Stop the output callback
void HAEAudioMixer::StopOutputCallback(void)
{
    GM_SetAudioOutput(NULL);
}

// start saving audio output to a file
HAEErr HAEAudioMixer::StartOutputToFile(HAEPathName pAudioOutputFile)
{
#if USE_CREATION_API == TRUE
    OPErr			theErr;
    XFILENAME		theFile;

    theErr = NO_ERR;
    // close old one first
    if (mWritingToFile)
	{
	    StopOutputToFile();
	}
    XConvertNativeFileToXFILENAME(pAudioOutputFile, &theFile);
    mWritingToFileReference = (void *)XFileOpenForWrite(&theFile, TRUE);
    if (mWritingToFileReference)
	{
	    GM_StopHardwareSoundManager(NULL);		// disengage from hardware
	    mWritingToFile = TRUE;
	}
    else
	{
	    theErr = BAD_FILE;
	}
    return HAE_TranslateOPErr(theErr);
#else
    pAudioOutputFile = pAudioOutputFile;
    return HAE_NOT_SETUP;
#endif
}

// Stop saving audio output to a file
void HAEAudioMixer::StopOutputToFile(void)
{
    if (mWritingToFile && mWritingToFileReference)
	{
	    XFileClose((XFILE)mWritingToFileReference);
	    mWritingToFileReference = NULL;
	    GM_StartHardwareSoundManager(NULL);			// reconnect to hardware
	}
    mWritingToFile = FALSE;
}

HAEErr HAEAudioMixer::ServiceAudioOutputToFile(void)
{
    GM_AudioTaskCallbackPtr		pTaskProc;
    GM_AudioOutputCallbackPtr	pOutputProc;
    long						outputSize;
    long						sampleSize, channels;
    char						bufferData[8192];
    HAEErr						theErr;

    theErr = HAE_NO_ERROR;
    if (mWritingToFile && mWritingToFileReference)
	{
	    pTaskProc = GM_GetAudioTask();
	    pOutputProc = GM_GetAudioOutput();

	    sampleSize = (GetModifiers() & HAE_USE_16) ? 2 : 1;
	    channels = (GetModifiers() & ~HAE_USE_STEREO) ? 2 : 1;
	    outputSize = GM_GetAudioBufferOutputSize();
	    if (outputSize)
		{
		    if (outputSize < 8192)
			{
#if 1
			    HAE_BuildMixerSlice(NULL, &bufferData[0], 
						outputSize, 
						(unsigned long)(outputSize / channels / sampleSize));
#else
			    MusicGlobals->insideAudioInterrupt++;

			    MusicGlobals->syncCount += HAE_GetSliceTimeInMicroseconds();			// 11 milliseconds
			    MusicGlobals->syncBufferCount++;

				// Generate one frame audio
			    PV_ProcessSampleFrame(NULL, &bufferData[0]);		// process chunk
			    if (pTaskProc)
				{
				    (*pTaskProc)(NULL, MusicGlobals->syncCount);
				}
			    if (pOutputProc)
				{
				    (*pOutputProc)(NULL, &bufferData[0], 
						   sampleSize,
						   channels,
						   (unsigned long)(outputSize / channels / sampleSize));
										
				}
			    MusicGlobals->insideAudioInterrupt--;
#endif
			    if (XFileWrite((XFILE)mWritingToFileReference, &bufferData[0], outputSize) == -1)
				{
				    theErr = HAE_GENERAL_BAD;
				}
			}
		}
	}
    return theErr;
}


// Class implemention for HAEAudioNoise

#if 0
#pragma mark ### HAEAudioNoise class ###
#endif

HAEAudioNoise::HAEAudioNoise(HAEAudioMixer *pHAEAudioMixer, char const *cName)
{
    mAudioMixer = pHAEAudioMixer;
    mName[0] = 0;
    if (cName)
	{
	    char	tempName[2048];
		
	    XStrCpy(tempName, (char *)cName);
	    if (XStrLen(tempName) > 63)
		{
		    tempName[63] = 0;	// anything larger than 64 characters is truncated
		}
	    XStrCpy(mName, tempName);
	}

    pNext = NULL;
    pGroupNext = NULL;
#if USE_LINKED_OBJECTS
    {
	HAEAudioNoise	*pHAETop, *pHAENext;

	// add to link
	pHAETop = (HAEAudioNoise *)pHAEAudioMixer->pTop;
	if (pHAETop)
	    {
		pHAENext = NULL;
		while (pHAETop)
		    {
			pHAENext = pHAETop;
			pHAETop = (HAEAudioNoise *)pHAETop->pNext;
		    }
		if (pHAENext)
		    {
			pHAENext->pNext = this;
		    }
	    }
	else
	    {
		mAudioMixer->pTop = this;
	    }
    }
#endif
}

HAEAudioNoise::~HAEAudioNoise()
{
#if USE_LINKED_OBJECTS
    HAEAudioNoise	*pHAETop;

    // remove link
    pHAETop = (HAEAudioNoise *)mAudioMixer->pTop;
    if (pHAETop != this)
	{
	    while (pHAETop)
		{
		    if (pHAETop->pNext == this)
			{
			    pHAETop->pNext = this->pNext;
			    break;
			}

		    pHAETop = (HAEMidiDirect *)pHAETop->pNext;
		}
	}
    else
	{
	    mAudioMixer->pTop = NULL;
	}
#endif
}

HAEAudioMixer * HAEAudioNoise::GetMixer(void)
{
    return mAudioMixer;
}

	
// Class implemention for HAEMidiFile

#if 0
#pragma mark ### HAEMidiFile class ###
#endif

HAEMidiFile::HAEMidiFile(HAEAudioMixer *pHAEAudioMixer, 
			 char const* cName, 
			 void *userReference):
    HAEMidiDirect(pHAEAudioMixer, 
		  cName, userReference)
{
    m_pTimeCallbackReference = NULL;
    m_pSongTimeCallback = NULL;
    m_pControllerCallbackReference = NULL;
    m_pControllerCallbackProc = NULL;
    mSongCallbackReference = NULL;
    m_pSongCallback = NULL;
    mSongMetaReference = NULL;
    m_pSongMetaCallback = NULL;
    // most of the real variables get setup when the HAEMidiDirect
    // gets created
}

HAEMidiFile::~HAEMidiFile()
{
    Unload();
}

// Given a pointer to a file, load it into a HAEMidiFile object. 
//
// If duplicateObject is TRUE, then the pointer passed in will be duplicated. 
// You can free the memory pointer passed after success.
// If FALSE the user pointer will be used, but
// not copied. Don't delete the object until after you have deleted this object.
HAEErr HAEMidiFile::LoadFromMemory(void const* pMidiData, unsigned long midiSize, 
				   HAE_BOOL duplicateObject, HAE_BOOL ignoreBadInstruments)
{
    SongResource		*pXSong;
    OPErr				theErr;
    XShortResourceID	theID;
    GM_Song				*pSong;
    XPTR				newData;
    HAEAudioMixer		*pMixer;

    if (m_pSongVariables)
	{
	    Unload();
	}
    theErr = NO_ERR;
    if (duplicateObject)
	{
	    newData = XNewPtr(midiSize);
	    if (newData)
		{
		    XBlockMove((XPTR)pMidiData, newData, midiSize);
		    pMidiData = newData;
		}
	    else
		{
		    theErr = MEMORY_ERR;
		}
	}
    if (pMidiData && midiSize && (theErr == NO_ERR))
	{
	    theID = midiSongCount++;	// runtime midi ID
	    pMixer = HAEAudioNoise::GetMixer();
	    pXSong = XNewSongPtr(SONG_TYPE_SMS,
				 theID,
				 pMixer->GetMidiVoices(),
				 pMixer->GetMixLevel(),
				 pMixer->GetSoundVoices(),
				 HAE_TranslateFromHAEReverb(pMixer->GetReverbType()));
	
	    if (pXSong)
		{
		    m_pPerformanceVariables = (void *)pXSong;	// preserve for use later
		    mPerformanceVariablesLength = (unsigned long)XGetPtrSize((XPTR)pXSong);

		    pSong = GM_LoadSong(NULL, this, theID, (void *)pXSong,
					(void *)pMidiData,
					(long)midiSize,
					NULL,		// no callback
					TRUE,		// load instruments
					ignoreBadInstruments,
					&theErr);
		    if (pSong)
			{
				// things are cool
			    pSong->disposeSongDataWhenDone = duplicateObject;	// dispose of midi data
			    GM_SetSongLoopFlag(pSong, FALSE);					// don't loop song

			    pSong->userReference = (long)mReference;
			    m_pSongVariables = pSong;				// preserve for use later
			}
		    else
			{
			    if (duplicateObject)
				{
				    XDisposePtr((XPTR)pMidiData);
				}
			}
		}
	    else
		{
		    theErr = MEMORY_ERR;
		    if (duplicateObject)
			{
			    XDisposePtr((XPTR)pMidiData);
			}
		}
	}
    else
	{
	    theErr = BAD_FILE;
	}
    return HAE_TranslateOPErr(theErr);
}

// read a song from a file into memory
HAEErr HAEMidiFile::LoadFromFile(HAEPathName pMidiFilePath, HAE_BOOL ignoreBadInstruments)
{
    XFILENAME			name;
    XPTR				pMidiData;
    SongResource		*pXSong;
    long				midiSize;
    OPErr				theErr;
    XShortResourceID	theID;
    GM_Song				*pSong;
    HAEAudioMixer		*pMixer;

    if (m_pSongVariables)
	{
	    Unload();
	}
    theErr = NO_ERR;
    XConvertNativeFileToXFILENAME(pMidiFilePath, &name);
    pMidiData = PV_GetFileAsData(&name, &midiSize);
    if (pMidiData)
	{
	    theID = midiSongCount++;	// runtime midi ID
	    pMixer = HAEAudioNoise::GetMixer();
	    pXSong = XNewSongPtr(SONG_TYPE_SMS,
				 theID,
				 pMixer->GetMidiVoices(),
				 pMixer->GetMixLevel(),
				 pMixer->GetSoundVoices(),
				 HAE_TranslateFromHAEReverb(pMixer->GetReverbType()));
	
	    if (pXSong)
		{
		    m_pPerformanceVariables = (void *)pXSong;	// preserve for use later
		    mPerformanceVariablesLength = (unsigned long)XGetPtrSize((XPTR)pXSong);

		    pSong = GM_LoadSong(NULL, this, theID, (void *)pXSong,
					pMidiData,
					midiSize,
					NULL,		// no callback
					TRUE,		// load instruments
					ignoreBadInstruments,
					&theErr);
		    if (pSong)
			{
				// things are cool
			    pSong->disposeSongDataWhenDone = TRUE;	// dispose of midi data
			    GM_SetSongLoopFlag(pSong, FALSE);		// don't loop song

			    pSong->userReference = (long)mReference;
			    m_pSongVariables = pSong;				// preserve for use later
			}
		    else
			{
			    XDisposePtr(pMidiData);
			}
		}
	    else
		{
		    XDisposePtr(pMidiData);
		    theErr = MEMORY_ERR;
		}
	}
    else
	{
	    theErr = BAD_FILE;
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAEMidiFile::LoadFromBank(char *cName, HAE_BOOL ignoreBadInstruments)
{
    SongResource		*pXSong;
    long				size;
    OPErr				theErr;
    XShortResourceID	theID;
    GM_Song				*pSong;

    if (m_pSongVariables)
	{
	    Unload();
	}
    theErr = BAD_FILE;
#if X_PLATFORM != X_MACINTOSH
    if (thePatchFile)
#endif
	{
	    pXSong = (SongResource *)XGetNamedResource(ID_SONG, cName, &size);		// look for song
	    if (pXSong)
		{
		    m_pPerformanceVariables = (void *)pXSong;	// preserve for use later
		    mPerformanceVariablesLength = (unsigned long)size;
			
		    theID = midiSongCount++;	// runtime midi ID
		    pSong = GM_LoadSong(NULL, this, theID, (void *)pXSong,
					NULL,
					0L,
					NULL,		// no callback
					TRUE,		// load instruments
					ignoreBadInstruments,
					&theErr);
		    if (pSong)
			{
				// things are cool
			    pSong->disposeSongDataWhenDone = TRUE;	// dispose of midi data
			    GM_SetSongLoopFlag(pSong, FALSE);		// don't loop song

			    pSong->userReference = (long)mReference;
			    m_pSongVariables = pSong;				// preserve for use later
			    theErr = NO_ERR;
			}
		}
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAEMidiFile::LoadFromID(unsigned long id, HAE_BOOL ignoreBadInstruments)
{
    SongResource		*pXSong;
    unsigned long		size;
    OPErr				theErr;
    GM_Song				*pSong;

    if (m_pSongVariables)
	{
	    Unload();
	}
    theErr = BAD_FILE;
#if X_PLATFORM != X_MACINTOSH
    if (thePatchFile)
#endif
	{
	    pXSong = (SongResource *)XGetAndDetachResource(ID_SONG, (XLongResourceID)id, (long *)&size);		// look for song
	    if (pXSong)
		{
		    m_pPerformanceVariables = (void *)pXSong;	// preserve for use later
		    mPerformanceVariablesLength = size;

		    pSong = GM_LoadSong(NULL, this, (XShortResourceID)id, (void *)pXSong,
					NULL,
					0L,
					NULL,		// no callback
					TRUE,		// load instruments
					ignoreBadInstruments,
					&theErr);
		    if (pSong)
			{
				// things are cool
			    pSong->disposeSongDataWhenDone = TRUE;	// dispose of midi data
			    GM_SetSongLoopFlag(pSong, FALSE);		// don't loop song

			    pSong->userReference = (long)mReference;
			    m_pSongVariables = pSong;				// preserve for use later
			    theErr = NO_ERR;
			}
		}
	}
    return HAE_TranslateOPErr(theErr);
}

// start song. If useEmbeddedMixerSettings is TRUE then the mixer will be reconfigured
// to the embedded song settings. If false, then song will attempt to start with the
// current mixer configuration.
// if autoRemix is TRUE, then the mixer will be auto magicly reconfigured based upon how many songs
// are playing, etc
HAEErr HAEMidiFile::Start(HAE_BOOL useEmbeddedMixerSettings, HAE_BOOL autoLevel)
{
    OPErr theErr;

    theErr = BAD_FILE;
    if (m_pSongVariables)
	{
	    theErr = GM_BeginSong((GM_Song *)m_pSongVariables, NULL, 
				  (XBOOL)useEmbeddedMixerSettings, (XBOOL)autoLevel);
	    SetDoneCallback(m_pSongCallback, mSongCallbackReference);
	}
    return HAE_TranslateOPErr(theErr);
}

void HAEMidiFile::Unload(void)
{
    Stop();
    Close();
}

// end song now
void HAEMidiFile::Stop(HAE_BOOL startFade)
{
    short int	songVolume;

    if (IsPaused())
	{
	    Resume();
	}
    if (m_pSongVariables)
	{
	    if (startFade)
		{
		    songVolume = GM_GetSongVolume((GM_Song *)m_pSongVariables);
		    GM_SetSongFadeRate((GM_Song *)m_pSongVariables, FLOAT_TO_FIXED(2.2),
				       0, songVolume, TRUE);
		}
	    else
		{
		    GM_KillSongNotes((GM_Song *)m_pSongVariables);
		    GM_EndSong(NULL, (GM_Song *)m_pSongVariables);
		}
	}
}

// fade song
void HAEMidiFile::Fade(HAE_BOOL doAsync)
{
    short int	songVolume;

    if (m_pSongVariables)
	{
	    songVolume = GM_GetSongVolume((GM_Song *)m_pSongVariables);
	    if (doAsync == FALSE)
		{
		    // We're going to fade the song out and don't stop it	
		    GM_SetSongFadeRate((GM_Song *)m_pSongVariables, FLOAT_TO_FIXED(2.2),
				       0, songVolume, FALSE);
		    while (GM_GetSongVolume((GM_Song *)m_pSongVariables) && 
			   (GM_IsSongDone((GM_Song *)m_pSongVariables) == FALSE)) 
			{
			    GetMixer()->ServiceAudioOutputToFile();
			    GetMixer()->ServiceIdle();
			    XWaitMicroseocnds(1000);
			}
		}
	    else
		{
		    GM_SetSongFadeRate((GM_Song *)m_pSongVariables, FLOAT_TO_FIXED(2.2),
				       0, songVolume, FALSE);
		}
	}
}


// fade song
void HAEMidiFile::FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync)
{
    short int	songVolume, saveVolume, newSongVolume, saveNewSongVolume;
    XFIXED		delta;

    if (m_pSongVariables)
	{
	    newSongVolume = FIXED_TO_SHORT_ROUNDED(destVolume * MAX_SONG_VOLUME);
	    saveNewSongVolume = newSongVolume;
	    // We're going to fade the song out before we stop it.		
	    songVolume = GM_GetSongVolume((GM_Song *)m_pSongVariables);
	    saveVolume = songVolume;

	    if (newSongVolume < songVolume)
		{	// fade out
		    songVolume = newSongVolume;
		    newSongVolume = saveVolume;
		    delta = FLOAT_TO_UNSIGNED_FIXED(2.2);
		}
	    else
		{	// fade in
		    delta = (XFIXED)FLOAT_TO_FIXED(-2.2);
		}
	    if (doAsync == FALSE)
		{
		    GM_SetSongFadeRate((GM_Song *)m_pSongVariables, delta,
				       songVolume, newSongVolume, FALSE);
		    while (GM_GetSongVolume((GM_Song *)m_pSongVariables) != saveNewSongVolume)
			{
			    GetMixer()->ServiceAudioOutputToFile();
			    GetMixer()->ServiceIdle();
			    XWaitMicroseocnds(1000);
			}
		}
	    else
		{
		    GM_SetSongFadeRate((GM_Song *)m_pSongVariables, delta,
				       songVolume, newSongVolume, FALSE);
		}
	}
}


// pause and resume song playback
void HAEMidiFile::Pause(void)
{
    if (m_pSongVariables)
	{
	    GM_PauseSong((GM_Song *)m_pSongVariables);
	}
}

void HAEMidiFile::Resume(void)
{
    if (m_pSongVariables)
	{
	    GM_ResumeSong((GM_Song *)m_pSongVariables);
	}
}

// currently paused
HAE_BOOL HAEMidiFile::IsPaused(void)
{
    HAE_BOOL	done;

    done = TRUE;
    if (m_pSongVariables)
	{
	    done = ((GM_Song *)m_pSongVariables)->songPaused;
	}
    return done;
}



// get ticks in midi ticks of length of song
unsigned long HAEMidiFile::GetTickLength(void)
{
    OPErr			theErr;
    unsigned long	ticks;

    ticks = 0;
    if (m_pSongVariables)
	{
	    ticks = GM_GetSongTickLength((GM_Song *)m_pSongVariables, &theErr);
	}
    return ticks;
}

// set the current playback position of song in midi ticks
HAEErr HAEMidiFile::SetTickPosition(unsigned long ticks)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    theErr = GM_SetSongTickPosition((GM_Song *)m_pSongVariables, ticks);
	}
    return HAE_TranslateOPErr(theErr);
}

// get the current playback position of a song in midi ticks
unsigned long HAEMidiFile::GetTickPosition(void)
{
    if (m_pSongVariables)
	{
	    return GM_SongTicks((GM_Song *)m_pSongVariables);
	}
    return 0;
}

// get ticks in microseconds of length of song
unsigned long HAEMidiFile::GetMicrosecondLength(void)
{
    OPErr			theErr;
    unsigned long	ticks;

    ticks = 0;
    if (m_pSongVariables)
	{
	    ticks = GM_GetSongMicrosecondLength((GM_Song *)m_pSongVariables, &theErr);
	}
    return ticks;
}

// set the current playback position of song in microseconds
HAEErr HAEMidiFile::SetMicrosecondPosition(unsigned long ticks)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    theErr = GM_SetSongMicrosecondPosition((GM_Song *)m_pSongVariables, ticks);
	}
    return HAE_TranslateOPErr(theErr);
}

// get the current playback position of a song in microseconds
unsigned long HAEMidiFile::GetMicrosecondPosition(void)
{
    if (m_pSongVariables)
	{
	    return GM_SongMicroseconds((GM_Song *)m_pSongVariables);
	}
    return 0;
}

// return the patches required to play this song
HAEErr HAEMidiFile::GetInstruments(HAE_INSTRUMENT *pArray768, short int *pReturnedCount)
{
#if USE_CREATION_API == TRUE
    OPErr				theErr;
    void				*pMidiData;
    XShortResourceID	instruments[MAX_INSTRUMENTS * MAX_BANKS];
    short int			count;

    theErr = NO_ERR;
    if (pArray768 && pReturnedCount && m_pSongVariables && m_pPerformanceVariables)
	{
	    *pReturnedCount = 0;	// total number of patches loaded
	    pMidiData = ((GM_Song *)m_pSongVariables)->midiData;
	    if (pMidiData)
		{
		    *pReturnedCount = (short int)GM_GetUsedPatchlist(m_pPerformanceVariables, 
								     pMidiData, 
								     XGetPtrSize(pMidiData), 
								     instruments, 
								     &theErr);
		    for (count = 0; count < *pReturnedCount; count++)
			{
			    pArray768[count] = (HAE_INSTRUMENT)instruments[count];
			}
		}
	}
    return HAE_TranslateOPErr(theErr);
#else
    pArray768 = pArray768;
    pReturnedCount = pReturnedCount;
    return HAE_NOT_SETUP;
#endif
}

static void PV_ProcessSongEndCallbacks(GM_Song *pSong)
{
    HAEDoneCallbackPtr	pCallback;
    HAEMidiFile			*pSongObject;

    if (pSong)
	{	
	    pSongObject = (HAEMidiFile *)GM_GetSongContext(pSong);
	    if (pSongObject)
		{
		    pCallback = (HAEDoneCallbackPtr)pSongObject->GetDoneCallback();;
		    if (pCallback)
			{
			    (*pCallback)(pSongObject->GetDoneCallbackReference());
			}
		}
	}
}

// Set a call back when song is done
void HAEMidiFile::SetDoneCallback(HAEDoneCallbackPtr pSongCallback, void * pReference)
{
    // save callbacks
    mSongCallbackReference = pReference;
    m_pSongCallback = pSongCallback;

    if (m_pSongVariables)
	{
	    GM_SetSongCallback((GM_Song *)m_pSongVariables, (GM_SongCallbackProcPtr)PV_ProcessSongEndCallbacks);
	}
}

static void PV_ProcessSongTimeCallbacks(void *threadContext, GM_Song *pSong, unsigned long currentMicroseconds, unsigned long currentMidiClock)
{
    HAETimeCallbackPtr	pCallback;
    HAEMidiFile			*pSongObject;

    threadContext;
    if (pSong)
	{
	    pSongObject = (HAEMidiFile *)GM_GetSongContext(pSong);
	    if (pSongObject)
		{
		    pCallback = pSongObject->GetTimeCallback();
		    if (pCallback)
			{
			    (*pCallback)(pSongObject->GetTimeCallbackReference(), currentMicroseconds, currentMidiClock);
			}
		}
	}
}

// Set a call back during song processing
void HAEMidiFile::SetTimeCallback(HAETimeCallbackPtr pSongCallback, void *pReference)
{
    m_pTimeCallbackReference = pReference;
    m_pSongTimeCallback = pSongCallback;

    if (m_pSongVariables)
	{
	    GM_SetSongTimeCallback((GM_Song *)m_pSongVariables, 
				   (GM_SongTimeCallbackProcPtr)PV_ProcessSongTimeCallbacks,
				   0L);
	}
}

static void PV_ProcessSongMetaEventCallbacks(void *threadContext, GM_Song *pSong, 
					     char type, void *pMetaText, long metaTextLength, INT16 currentTrack)
{
    HAEMetaEventCallbackPtr	pCallback;
    HAEMidiFile				*pSongObject;

    threadContext;
    currentTrack;
    if (pSong)
	{
	    pSongObject = (HAEMidiFile *)GM_GetSongContext(pSong);
	    if (pSongObject)
		{
		    pCallback = pSongObject->GetMetaCallback();
		    if (pCallback)
			{
			    (*pCallback)(pSongObject->GetMetaCallbackReference(), 
					 (HAEMetaTypes)type, pMetaText, metaTextLength);
			}
		}
	}
}

// Set a call back during song playback for meta events. Pass NULL to clear callback.
void HAEMidiFile::SetMetaEventCallback(HAEMetaEventCallbackPtr pSongCallback, void * pReference)
{
    mSongMetaReference = pReference;
    m_pSongMetaCallback = pSongCallback;

    if (m_pSongVariables)
	{
	    GM_SetSongMetaEventCallback((GM_Song *)m_pSongVariables, 
					PV_ProcessSongMetaEventCallbacks, 
					0L);
	}
}

static void PV_ProcessSongControllerCallbacks(void *threadContext, GM_Song *pSong, void * reference, short int channel, short int track, short int controler, 
					      short int value)
{
    HAEMidiFile				*pHAEMidi;
    HAEControlerCallbackPtr	callback;
    void					*pReference;

    threadContext;
    pSong;
    pHAEMidi = (HAEMidiFile *)reference;
    if (pHAEMidi)
	{
	    callback = pHAEMidi->GetControlCallback();
	    pReference = pHAEMidi->GetControlCallbackReference();
	    if (callback)
		{
		    (*callback)(pReference, channel, track, controler, value);
		}
	}
}
												
void HAEMidiFile::SetControlCallback(short int controller, HAEControlerCallbackPtr pControllerCallback, 
				     void * pReference)
{
    m_pControllerCallbackReference = pReference;
    m_pControllerCallbackProc = pControllerCallback;

    if (m_pSongVariables)
	{
	    GM_SetControllerCallback((GM_Song *)m_pSongVariables, this, 
				     PV_ProcessSongControllerCallbacks, controller);
	}
}


// poll to see if song is done
HAE_BOOL HAEMidiFile::IsDone(void)
{
    HAE_BOOL	done;

    done = TRUE;
    if (m_pSongVariables)
	{
	    done = (HAE_BOOL)GM_IsSongDone((GM_Song *)m_pSongVariables);
	}
    return done;
}

HAE_BOOL HAEMidiFile::IsPlaying(void)
{
    if (IsDone() == FALSE)
	{
	    return TRUE;
	}
    return FALSE;
}
	

// pass TRUE to loop song, FALSE to not loop
void HAEMidiFile::SetLoopFlag(HAE_BOOL loop)
{
    if (m_pSongVariables)
	{
	    GM_SetSongLoopFlag((GM_Song *)m_pSongVariables, loop);
	}
}

HAE_BOOL HAEMidiFile::GetLoopFlag(void)
{
    if (m_pSongVariables)
	{
	    return (HAE_BOOL)GM_GetSongLoopFlag((GM_Song *)m_pSongVariables);
	}
    return FALSE;
}

void HAEMidiFile::SetLoopMax(short int maxLoopCount)
{
    if (m_pSongVariables)
	{
	    GM_SetSongLoopMax((GM_Song *)m_pSongVariables, maxLoopCount - 1);
	}
}

short int HAEMidiFile::GetLoopMax(void)
{
    short int	loop;

    loop = 0;
    if (m_pSongVariables)
	{
	    loop = GM_GetSongLoopMax((GM_Song *)m_pSongVariables);
	}
    return loop;
}

// set song master tempo
void HAEMidiFile::SetMasterTempo(HAE_UNSIGNED_FIXED newTempo)
{
    if (m_pSongVariables)
	{
	    GM_SetMasterSongTempo((GM_Song *)m_pSongVariables, newTempo);
	}
}

// get song master tempo
HAE_UNSIGNED_FIXED HAEMidiFile::GetMasterTempo(void)
{
    HAE_UNSIGNED_FIXED tempo;

    tempo = 0;
    if (m_pSongVariables)
	{
	    tempo = GM_GetMasterSongTempo((GM_Song *)m_pSongVariables);
	}
    return tempo;
}


// Sets tempo in microsecond per quarter note
void HAEMidiFile::SetTempo(unsigned long newTempo)
{
    if (m_pSongVariables)
	{
	    GM_SetSongTempo((GM_Song *)m_pSongVariables, newTempo);
	}
}

// returns tempo in microsecond per quarter note
unsigned long HAEMidiFile::GetTempo(void)
{
    unsigned long	tempo;

    tempo = 0;
    if (m_pSongVariables)
	{
	    tempo = GM_GetSongTempo((GM_Song *)m_pSongVariables);
	}
    return tempo;
}

// sets tempo in beats per minute
void HAEMidiFile::SetTempoInBeatsPerMinute(unsigned long newTempo)
{
    if (m_pSongVariables)
	{
	    GM_SetSongTempInBeatsPerMinute((GM_Song *)m_pSongVariables, newTempo);
	}
}

// returns tempo in beats per minute
unsigned long HAEMidiFile::GetTempoInBeatsPerMinute(void)
{
    unsigned long	tempo;

    tempo = 0;
    if (m_pSongVariables)
	{
	    tempo = GM_GetSongTempoInBeatsPerMinute((GM_Song *)m_pSongVariables);
	}
    return tempo;
}

// Get embedded data types
short int HAEMidiFile::GetEmbeddedMidiVoices(void)
{
    short int song;

    song = 0;
    if (m_pSongVariables)
	{
	    song = ((GM_Song *)m_pSongVariables)->maxSongVoices;
	}
    return song;
}

short int HAEMidiFile::GetEmbeddedSoundVoices(void)
{
    short int sound;

    sound = 0;
    if (m_pSongVariables)
	{
	    sound = ((GM_Song *)m_pSongVariables)->maxEffectVoices;
	}
    return sound;
}

short int HAEMidiFile::GetEmbeddedMixLevel(void)
{
    short int mix;

    mix = 0;
    if (m_pSongVariables)
	{
	    mix = ((GM_Song *)m_pSongVariables)->mixLevel;
	}
    return mix;
}

HAEReverbMode HAEMidiFile::GetEmbeddedReverbType(void)
{
    ReverbMode		r;
    HAEReverbMode	verb;

    verb = HAE_REVERB_NO_CHANGE;
    if (m_pSongVariables)
	{
	    r = ((GM_Song *)m_pSongVariables)->defaultReverbType;
	    verb = HAE_TranslateToHAEReverb(r);
	}
    return verb;
}


void HAEMidiFile::SetEmbeddedReverbType(HAEReverbMode verb)
{
    ReverbMode	r;

    if (m_pSongVariables)
	{
	    r = HAE_TranslateFromHAEReverb(verb);
	    ((GM_Song *)m_pSongVariables)->defaultReverbType = r;
	}
}

// get/set embedded volume type.
// NOTE: Does not change current settings only when Start is called
void HAEMidiFile::SetEmbeddedVolume(HAE_UNSIGNED_FIXED volume)
{
    if (m_pPerformanceVariables)
	{
	    XSetSongVolume((SongResource *)m_pPerformanceVariables,
			   FIXED_TO_SHORT_ROUNDED(volume * MAX_SONG_VOLUME));
	}
}

HAE_UNSIGNED_FIXED HAEMidiFile::GetEmbeddedVolume(void)
{
    HAE_UNSIGNED_FIXED	volume;

    volume = 0;
    if (m_pPerformanceVariables)
	{
	    volume = UNSIGNED_RATIO_TO_FIXED(XGetSongVolume((SongResource *)m_pPerformanceVariables), MAX_SONG_VOLUME);
	}
    return volume;
}


// set embedded data types
void HAEMidiFile::SetEmbeddedMidiVoices(short int midiVoices)
{
    if (m_pSongVariables)
	{
	    ((GM_Song *)m_pSongVariables)->maxSongVoices = midiVoices;
	}
}

void HAEMidiFile::SetEmbeddedSoundVoices(short int soundVoices)
{
    if (m_pSongVariables)
	{
	    ((GM_Song *)m_pSongVariables)->maxEffectVoices = soundVoices;
	}
}

void HAEMidiFile::SetEmbeddedMixLevel(short int mixLevel)
{
    if (m_pSongVariables)
	{
	    ((GM_Song *)m_pSongVariables)->mixLevel = mixLevel;
	}
}


void HAEMidiFile::MuteTrack(unsigned short int track)
{
    if (m_pSongVariables)
	{
	    GM_MuteTrack((GM_Song *)m_pSongVariables, track);
	}
}

void HAEMidiFile::UnmuteTrack(unsigned short int track)
{
    if (m_pSongVariables)
	{
	    GM_UnmuteTrack((GM_Song *)m_pSongVariables, track);
	}
}

void HAEMidiFile::GetTrackMuteStatus(HAE_BOOL *pTracks)
{
    if (m_pSongVariables)
	{
	    GM_GetTrackMuteStatus((GM_Song *)m_pSongVariables, pTracks);
	}
}

void HAEMidiFile::SoloTrack(unsigned short int track)
{
    if (m_pSongVariables)
	{
	    GM_SoloTrack((GM_Song *)m_pSongVariables, track);
	}
}

void HAEMidiFile::UnSoloTrack(unsigned short int track)
{
    if (m_pSongVariables)
	{
	    GM_UnsoloTrack((GM_Song *)m_pSongVariables, track);
	}
}

void HAEMidiFile::GetSoloTrackStatus(HAE_BOOL *pTracks)
{
    if (m_pSongVariables)
	{
	    GM_GetTrackSoloStatus((GM_Song *)m_pSongVariables, pTracks);
	}
}

static SongInfo PV_TranslateInfoTypes(HAEInfoTypes infoType)
{
    SongInfo	info;

    switch (infoType)
	{
	default:
	    info = I_INVALID;
	    break;
	case TITLE_INFO:
	    info = I_TITLE;
	    break;
	case PERFORMED_BY_INFO:
	    info = I_PERFORMED_BY;
	    break;
	case COMPOSER_INFO:
	    info = I_COMPOSER;
	    break;
	case COPYRIGHT_INFO:
	    info = I_COPYRIGHT;
	    break;
	case PUBLISHER_CONTACT_INFO:
	    info = I_PUBLISHER_CONTACT;
	    break;
	case USE_OF_LICENSE_INFO:
	    info = I_USE_OF_LICENSE;
	    break;
	case LICENSE_TERM_INFO:
	    info = I_LICENSE_TERM;
	    break;
	case LICENSED_TO_URL_INFO:
	    info = I_LICENSED_TO_URL;
	    break;
	case EXPIRATION_DATE_INFO:
	    info = I_EXPIRATION_DATE;
	    break;
	case COMPOSER_NOTES_INFO:
	    info = I_COMPOSER_NOTES;
	    break;
	case INDEX_NUMBER_INFO:
	    info = I_INDEX_NUMBER;
	    break;
	case GENRE_INFO:
	    info = I_GENRE;
	    break;
	case SUB_GENRE_INFO:
	    info = I_SUB_GENRE;
	    break;
	}
    return info;
}

// get size of info about this song file. Will an unsigned long
unsigned long HAEMidiFile::GetInfoSize(HAEInfoTypes infoType)
{
#if USE_FULL_RMF_SUPPORT == TRUE
    SongInfo		info;
    unsigned long	size;

    size = 0;
    info = PV_TranslateInfoTypes(infoType);
    if ((info != I_INVALID) && m_pSongVariables && m_pPerformanceVariables)
	{
	    size = XGetSongInformationSize((SongResource *)m_pPerformanceVariables, mPerformanceVariablesLength, 
					   info);
	}
    return size;
#else
    infoType;
    return 0;
#endif
}

// get info about this song file. Will return a 'C' string
HAEErr HAEMidiFile::GetInfo(HAEInfoTypes infoType, char *cInfo)
{
#if USE_FULL_RMF_SUPPORT == TRUE
    SongInfo	info;
    HAEErr		theErr;

    theErr = HAE_NO_ERROR;
    cInfo[0] = 0;
    info = PV_TranslateInfoTypes(infoType);

    if ((info != I_INVALID) && m_pSongVariables && mPerformanceVariablesLength)
	{
	    XGetSongInformation((SongResource *)m_pPerformanceVariables, mPerformanceVariablesLength, 
				info, cInfo);

#if ( (X_PLATFORM == X_WINDOWS) 			||	\
	  (X_PLATFORM == X_WIN_HARDWARE)	||	\
	  (X_PLATFORM == X_SOLARIS)	|| \
	  (X_PLATFORM == X_LINUX)	)
	    while (*cInfo)
		{
		    *cInfo = XTranslateMacToWin(*cInfo);
		    cInfo++;
		}
#endif
	}
    else
	{
	    theErr = HAE_PARAM_ERR;
	}
    return theErr;
#else
    infoType;
    cInfo[0] = 0;
    return HAE_NOT_SETUP;
#endif
}

// Class implemention for HAERMFFile

#if 0
#pragma mark ### HAERMFFile class ###
#endif

HAERMFFile::HAERMFFile(HAEAudioMixer *pHAEAudioMixer, 
		       char const* cName, 
		       void * userReference):
    HAEMidiFile(pHAEAudioMixer, 
		cName, userReference)
{
    // most of the real variables get setup when the HAEMidiDirect and HAEMidiFile
    // gets created
    m_pRMFDataBlock = NULL;
}

HAERMFFile::~HAERMFFile()
{
    Unload();
    XDisposePtr((XPTR)m_pRMFDataBlock);
    m_pRMFDataBlock = NULL;
}

// is this RMF file encrypted?
HAE_BOOL HAERMFFile::IsEncrypted() const
{
    HAE_BOOL	locked;

    locked = FALSE;
    if (m_pPerformanceVariables)
	{
	    locked = (HAE_BOOL)XIsSongLocked((SongResource *)m_pPerformanceVariables);
	}
    return locked;
}

// is this RMF file compressed?
HAE_BOOL HAERMFFile::IsCompressed() const
{
    HAE_BOOL	compressed;

    compressed = FALSE;
    if (m_pPerformanceVariables)
	{
	    compressed = (HAE_BOOL)XIsSongCompressed((SongResource *)m_pPerformanceVariables);
	}
    return compressed;
}

HAEErr HAERMFFile::LoadFromFile(const HAEPathName pRMFFilePath, HAE_BOOL ignoreBadInstruments)
{
    XFILE				fileRef;
    XFILENAME			name;
    SongResource		*pXSong;
    GM_Song				*pSong;
    OPErr				theErr;
    XLongResourceID		theID;
    long				size;

    theErr = BAD_FILE;

    if (m_pSongVariables)
	{
	    Unload();
	}
    XConvertNativeFileToXFILENAME(pRMFFilePath, &name);
    fileRef = XFileOpenResource(&name, TRUE);
    if (fileRef)
	{
	    // look for first song. RMF files only contain one SONG resource
	    pXSong = (SongResource *)XGetIndexedResource(ID_SONG, &theID, 0, NULL, &size);
	    if (pXSong)
		{
		    m_pPerformanceVariables = (void *)pXSong;	// preserve for use later
		    mPerformanceVariablesLength = (unsigned long)size;

		    pSong = GM_LoadSong(NULL, this, (XShortResourceID)theID, (void *)pXSong,
					NULL,
					0L,
					NULL,		// no callback
					TRUE,		// load instruments
					ignoreBadInstruments,
					&theErr);
		    if (pSong)
			{
				// things are cool
			    pSong->disposeSongDataWhenDone = TRUE;	// dispose of midi data
			    GM_SetSongLoopFlag(pSong, FALSE);		// don't loop song

			    pSong->userReference = (long)mReference;
			    m_pSongVariables = pSong;				// preserve for use later
			    theErr = NO_ERR;
			}
		}
	    XFileClose(fileRef);
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAERMFFile::LoadFromMemory(void const* pRMFData, unsigned long rmfSize, 
				  HAE_BOOL duplicateObject,
				  HAE_BOOL ignoreBadInstruments)
{
    XFILE				fileRef;
    SongResource		*pXSong;
    GM_Song				*pSong;
    OPErr				theErr;
    XLongResourceID		theID;
    long				size;
    void				*newData;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    Unload();
	}
    if (pRMFData && rmfSize)
	{
	    if (duplicateObject)
		{
		    XDisposePtr(m_pRMFDataBlock);
		    newData = XNewPtr(rmfSize);
		    if (newData)
			{
			    XBlockMove((XPTR)pRMFData, newData, rmfSize);
			    m_pRMFDataBlock = newData;
			    pRMFData = newData;
			}
		    else
			{
			    theErr = MEMORY_ERR;
			}
		}
	    if (theErr == NO_ERR)
		{
		    fileRef = XFileOpenResourceFromMemory((XPTR)pRMFData, rmfSize, 
							  (duplicateObject) ? FALSE : TRUE);
		    if (fileRef)
			{
				// look for first song. RMF files only contain one SONG resource
			    pXSong = (SongResource *)XGetIndexedResource(ID_SONG, &theID, 0, NULL, &size);
			    if (pXSong)
				{
				    m_pPerformanceVariables = (void *)pXSong;	// preserve for use later
				    mPerformanceVariablesLength = (unsigned long)size;

				    pSong = GM_LoadSong(NULL, this, (XShortResourceID)theID, (void *)pXSong,
							NULL,
							0L,
							NULL,		// no callback
							TRUE,		// load instruments
							ignoreBadInstruments,
							&theErr);
				    if (pSong)
					{
					    // things are cool
					    pSong->disposeSongDataWhenDone = TRUE;	// dispose of midi data
					    GM_SetSongLoopFlag(pSong, FALSE);		// don't loop song
	
					    pSong->userReference = (long)mReference;
					    m_pSongVariables = pSong;				// preserve for use later
					}
				}
			    XFileClose(fileRef);
			}
		    else
			{
			    theErr = BAD_FILE;
			}
		}
	    if (theErr)
		{
		    XDisposePtr(m_pRMFDataBlock);
		    m_pRMFDataBlock = NULL;
		}
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAERMFFile::LoadFromBank(char const* cName, HAE_BOOL ignoreBadInstruments)
{
    XPTR		pRMF;
    long		size;
    HAEErr		theErr;

    theErr = HAE_BAD_FILE;

    if (m_pSongVariables)
	{
	    Unload();
	}
    pRMF = XGetNamedResource(ID_RMF, (void *)cName, &size);		// look for embedded RMF song
    if (pRMF)
	{
	    theErr = LoadFromMemory(pRMF, size, TRUE, ignoreBadInstruments);
	    XDisposePtr(pRMF);
	}
    return theErr;
}

HAEErr HAERMFFile::LoadFromID(unsigned long id, HAE_BOOL ignoreBadInstruments)
{
    XPTR		pRMF;
    long		size;
    HAEErr		theErr;

    theErr = HAE_BAD_FILE;

    if (m_pSongVariables)
	{
	    Unload();
	}
    pRMF = XGetAndDetachResource(ID_RMF, (XLongResourceID)id, &size);		// look for embedded RMF song
    if (pRMF)
	{
	    theErr = LoadFromMemory(pRMF, size, TRUE, ignoreBadInstruments);
	    XDisposePtr(pRMF);
	}
    return theErr;
}

// Class implemention for HAEMidiDirect

#if 0
#pragma mark ### HAEMidiDirect class ###
#endif

HAEMidiDirect::HAEMidiDirect(HAEAudioMixer *pHAEAudioMixer, char const* cName, 
			     void* userReference) :
    HAEAudioNoise(pHAEAudioMixer, cName)
{
    m_pPerformanceVariables = NULL;
    mPerformanceVariablesLength = 0;
    mQueueMidi = TRUE;
    mReference = userReference;
    m_pSongVariables = NULL;
}

HAEMidiDirect::~HAEMidiDirect()
{
    Close();
}

HAEErr HAEMidiDirect::Open(HAE_BOOL loadInstruments)
{
    GM_Song			*pSong;
    OPErr			theErr;
    HAEAudioMixer	*pMixer;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    Close();
	}
    pSong = GM_CreateLiveSong(this, midiSongCount++);
    if (pSong)
	{
	    pMixer = GetMixer();
	    pSong->maxSongVoices = pMixer->GetMidiVoices();
	    pSong->maxEffectVoices = pMixer->GetSoundVoices();
	    pSong->mixLevel = pMixer->GetMixLevel();
	    GM_SetCacheSamples(pSong, TRUE);

	    pSong->userReference = (long)mReference;
	    theErr = GM_StartLiveSong(pSong, loadInstruments);
	    if (theErr)
		{
		    while (GM_FreeSong(NULL, pSong) == STILL_PLAYING)
			{
			    XWaitMicroseocnds(HAE_GetSliceTimeInMicroseconds());
			}
		    pSong = NULL;
		}
	}
    else
	{
	    theErr = MEMORY_ERR;
	}
    m_pSongVariables = (void *)pSong;
    return HAE_TranslateOPErr(theErr);
}

void HAEMidiDirect::Close(void)
{
    if (m_pSongVariables)
	{
	    GM_SetCacheSamples((GM_Song *)m_pSongVariables, FALSE);
	    GM_KillSongNotes((GM_Song *)m_pSongVariables);
	    while (GM_FreeSong(NULL, (GM_Song *)m_pSongVariables) == STILL_PLAYING)
		{
		    XWaitMicroseocnds(HAE_GetSliceTimeInMicroseconds());
		}
	    m_pSongVariables = NULL;
	}
    if (m_pPerformanceVariables)
	{
	    XDisposeSongPtr((SongResource *)m_pPerformanceVariables);
	    m_pPerformanceVariables = NULL;
	    mPerformanceVariablesLength = 0;
	}
}

HAE_BOOL HAEMidiDirect::IsLoaded(void)
{
    if (m_pSongVariables)
	{
	    return TRUE;
	}
    return FALSE;
}

// return controller value
char HAEMidiDirect::GetControlValue(unsigned char channel,  unsigned char controller)
{
    char	value;

    value = 0;
    if (m_pSongVariables)
	{
	    value = GM_GetControllerValue((GM_Song *)m_pSongVariables, channel, controller);
	}
    return value;
}

void HAEMidiDirect::SetCacheSample(HAE_BOOL cacheSamples)
{
    if (m_pSongVariables)
	{
	    GM_SetCacheSamples((GM_Song *)m_pSongVariables, cacheSamples);
	}
}
		
HAE_BOOL HAEMidiDirect::GetCacheSample(void)
{
    HAE_BOOL	value;

    value = 0;
    if (m_pSongVariables)
	{
	    value = GM_GetCacheSamples((GM_Song *)m_pSongVariables);
	}
    return value;
}
		
// Get the current Midi program and bank values
void HAEMidiDirect::GetProgramBank(unsigned char channel, 
				   unsigned char *pProgram,
				   unsigned char *pBank)
{
    if (m_pSongVariables && pBank && pProgram)
	{
	    *pProgram = (unsigned char)((GM_Song *)m_pSongVariables)->channelProgram[channel];
	    *pBank = ((GM_Song *)m_pSongVariables)->channelBank[channel];
	}
}

void HAEMidiDirect::MuteChannel(unsigned short int channel)
{
    if (m_pSongVariables)
	{
	    GM_MuteChannel((GM_Song *)m_pSongVariables, channel);
	}
}

void HAEMidiDirect::UnmuteChannel(unsigned short int channel)
{
    if (m_pSongVariables)
	{
	    GM_UnmuteChannel((GM_Song *)m_pSongVariables, channel);
	}
}

void HAEMidiDirect::GetChannelMuteStatus(HAE_BOOL *pChannels)
{
    if (m_pSongVariables)
	{
	    GM_GetChannelMuteStatus((GM_Song *)m_pSongVariables, pChannels);
	}
}

void HAEMidiDirect::SoloChannel(unsigned short int channel)
{
    if (m_pSongVariables)
	{
	    GM_SoloChannel((GM_Song *)m_pSongVariables, channel);
	}
}

void HAEMidiDirect::UnSoloChannel(unsigned short int channel)
{
    if (m_pSongVariables)
	{
	    GM_UnsoloChannel((GM_Song *)m_pSongVariables, channel);
	}
}

void HAEMidiDirect::GetChannelSoloStatus(HAE_BOOL *pChannels)
{
    if (m_pSongVariables)
	{
	    GM_GetChannelSoloStatus((GM_Song *)m_pSongVariables, pChannels);
	}
}


// set song volume. You can overdrive by passing values larger than 1.0
void HAEMidiDirect::SetVolume(HAE_UNSIGNED_FIXED volume)
{
    if (m_pSongVariables)
	{
	    GM_SetSongVolume((GM_Song *)m_pSongVariables,
			     FIXED_TO_SHORT_ROUNDED(volume * MAX_SONG_VOLUME));
	}
}

// get the song volume
HAE_UNSIGNED_FIXED HAEMidiDirect::GetVolume(void)
{
    HAE_UNSIGNED_FIXED	volume;

    volume = 0;
    if (m_pSongVariables)
	{
	    volume = UNSIGNED_RATIO_TO_FIXED(GM_GetSongVolume((GM_Song *)m_pSongVariables),
					     MAX_SONG_VOLUME);
	}
    return volume;
}


// Set the master stereo position of a HAEMidiDirect (-63 left to 63 right, 0 is middle)
void HAEMidiDirect::SetStereoPosition(short int stereoPosition)
{
    if (m_pSongVariables)
	{
	    GM_SetSongStereoPosition((GM_Song *)m_pSongVariables, stereoPosition);
	}
}

// Set the master stereo position of a HAEMidiDirect (-63 left to 63 right, 0 is middle)
short int HAEMidiDirect::GetStereoPosition(void)
{
    short int stereoPosition;

    stereoPosition = 0;
    if (m_pSongVariables)
	{
	    stereoPosition = GM_GetSetStereoPosition((GM_Song *)m_pSongVariables);
	}
    return stereoPosition;
}

// If allowPitch is FALSE, then "SetPitchOffset" will have no effect on passed 
// channel (0 to 15)
void HAEMidiDirect::AllowChannelPitchOffset(unsigned short int channel, HAE_BOOL allowPitch)
{
    if (m_pSongVariables)
	{
	    GM_AllowChannelPitchOffset((GM_Song *)m_pSongVariables, channel, allowPitch);
	}
}

// Return if the passed channel will allow pitch offset
HAE_BOOL HAEMidiDirect::DoesChannelAllowPitchOffset(unsigned short int channel)
{
    HAE_BOOL	flag;

    flag = FALSE;
    if (m_pSongVariables)
	{
	    flag = GM_DoesChannelAllowPitchOffset((GM_Song *)m_pSongVariables, channel);
	}
    return flag;
}

// set note offset in semi tones	(-12 is down an octave, 12 is up an octave)
void HAEMidiDirect::SetPitchOffset(long offset)
{
    if (m_pSongVariables)
	{
	    GM_SetSongPitchOffset((GM_Song *)m_pSongVariables, offset);
	}
}

// return note offset in semi tones	(-12 is down an octave, 12 is up an octave)
long HAEMidiDirect::GetPitchOffset(void)
{
    long	offset;

    offset = 0;
    if (m_pSongVariables)
	{
	    offset = GM_GetSongPitchOffset((GM_Song *)m_pSongVariables);
	}
    return offset;
}

HAE_BOOL HAEMidiDirect::IsInstrumentLoaded(HAE_INSTRUMENT instrument)
{
    return (HAE_BOOL)GM_IsSongInstrumentLoaded((GM_Song *)m_pSongVariables, (XLongResourceID)instrument);
}

HAE_INSTRUMENT HAEMidiDirect::TranslateBankProgramToInstrument(unsigned short bank, 
							       unsigned short program, unsigned short channel, unsigned short note)
{
    HAE_INSTRUMENT	instrument;

    instrument = program;
    if (channel == PERCUSSION_CHANNEL)
	{
	    bank = (bank * 2) + 1;		// odd banks are percussion
	}
    else
	{
	    bank = bank * 2 + 0;		// even banks are for instruments
	    note = 0;
	}

    if (bank < MAX_BANKS)
	{
	    instrument = (bank * 128) + program + note;
	}

    return instrument;
}


HAEErr HAEMidiDirect::LoadInstrument(HAE_INSTRUMENT instrument)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    theErr = GM_LoadSongInstrument((GM_Song *)m_pSongVariables, (XLongResourceID)instrument);
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return HAE_TranslateOPErr(theErr);
}

// load an instrument with custom patch data.
HAEErr HAEMidiDirect::LoadInstrumentFromData(HAE_INSTRUMENT instrument, void *data, unsigned long dataSize)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    theErr = GM_LoadInstrumentFromExternalData((GM_Song *)m_pSongVariables, (XLongResourceID)instrument, 
						       (InstrumentResource *)data, dataSize);
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return HAE_TranslateOPErr(theErr);
}

// create a data block that is the instrument. Data block then can be passed into LoadInstrumentFromData
HAEErr HAEMidiDirect::CreateInstrumentAsData(HAE_INSTRUMENT instrument, void **pData, unsigned long *pDataSize)
{
    OPErr				theErr;
    InstrumentResource	*pInstrument;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    pInstrument = (InstrumentResource *)XGetAndDetachResource(ID_INST, (XLongResourceID)instrument, (long *)pDataSize);		// look for an instrument
	    if (pInstrument)
		{
		    *pData = (void *)pInstrument;
		}
	    else
		{
		    theErr = BAD_INSTRUMENT;
		}
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return HAE_TranslateOPErr(theErr);
}


HAEErr HAEMidiDirect::UnloadInstrument(HAE_INSTRUMENT instrument)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (m_pSongVariables)
	{
	    theErr = GM_UnloadSongInstrument((GM_Song *)m_pSongVariables, (XLongResourceID)instrument);
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAEMidiDirect::RemapInstrument(HAE_INSTRUMENT from, HAE_INSTRUMENT to)
{
    OPErr	theErr;

    theErr = GM_RemapInstrument((GM_Song *)m_pSongVariables, (XLongResourceID)from, (XLongResourceID)to);
    return HAE_TranslateOPErr(theErr);
}


// get current midi tick in microseconds
unsigned long HAEMidiDirect::GetTick(void)
{
    return GM_GetSyncTimeStamp();
}

// set queue control of midi commmands. Use TRUE to queue commands, FALSE to
// send directly to engine. Default is TRUE
void HAEMidiDirect::SetQueue(HAE_BOOL useQueue)
{
    mQueueMidi = useQueue;
}

void HAEMidiDirect::LockQueue(void)
{
    if (mQueueMidi)
	{
	    QGM_LockExternalMidiQueue();
	}
}

void HAEMidiDirect::UnlockQueue(void)
{
    if (mQueueMidi)
	{
	    QGM_UnlockExternalMidiQueue();
	}
}

// given a midi stream, parse it out to the various midi functions
// for example:
// 0x92			0x50		0x7F		0x00
// comandByte	data1Byte	data2Byte	data3Byte
// Note 80 on with a velocity of 127 on channel 2
void HAEMidiDirect::ParseMidiData(unsigned char commandByte, unsigned char data1Byte, 
				  unsigned char data2Byte, unsigned char data3Byte,
				  unsigned long time)
{
    unsigned char	channel;

    channel = commandByte & 0x0F;
    data3Byte = data3Byte;
    switch(commandByte & 0xF0)
	{
	case 0x80:	// Note off
	    NoteOff(channel, data1Byte, data2Byte, time);
	    break;
	case 0x90:	// Note on
	    NoteOn(channel, data1Byte, data2Byte, time);
	    break;
	case 0xA0:	// key pressure (aftertouch)
	    KeyPressure(channel, data1Byte, data2Byte, time);
	    break;
	case 0xB0:	// controllers
	    ControlChange(channel, data1Byte, data2Byte, time);
	    break;
	case 0xC0:	// Program change
	    ProgramChange(channel, data1Byte, time);
	    break;
	case 0xD0:	// channel pressure (aftertouch)
	    ChannelPressure(channel, data1Byte, time);
	    break;
	case 0xE0:	// SetPitchBend
	    PitchBend(channel, data1Byte, data2Byte, time);
	    break;
	}

}

// normal note on
void HAEMidiDirect::NoteOn(unsigned char channel, 
			   unsigned char note, 
			   unsigned char velocity,
			   unsigned long time)
{
    if (time == 0)
	{
	    time = GM_GetSyncTimeStamp();
	}

    if (mQueueMidi)
	{
	    QGM_NoteOn((GM_Song *)m_pSongVariables, time, channel, note, velocity);
	}
    else
	{
	    GM_NoteOn((GM_Song *)m_pSongVariables, channel, note, velocity);
	}
}

// normal note on with load
void HAEMidiDirect::NoteOnWithLoad(unsigned char channel, 
				   unsigned char note, 
				   unsigned char velocity,
				   unsigned long time)
{
    HAE_INSTRUMENT	inst;
    unsigned char	program, bank;

    if (mQueueMidi)
	{
	    GetProgramBank(channel, &program, &bank);
	    inst = TranslateBankProgramToInstrument(bank, program, channel, note);
	    if (IsInstrumentLoaded(inst) == FALSE)
		{
		    LoadInstrument(inst);
		}
	    if (time == 0)
		{
		    time = GM_GetSyncTimeStamp();
		}

	    QGM_NoteOn((GM_Song *)m_pSongVariables, time, channel, note, velocity);
	}
}

void HAEMidiDirect::NoteOff(unsigned char channel, 
			    unsigned char note, 
			    unsigned char velocity,
			    unsigned long time)
{
    if (time == 0)
	{
	    time = GM_GetSyncTimeStamp();
	}

    if (mQueueMidi)
	{
	    QGM_NoteOff((GM_Song *)m_pSongVariables, time, channel, note, velocity);
	}
    else
	{
	    GM_NoteOff((GM_Song *)m_pSongVariables, channel, note, velocity);
	}
}


void HAEMidiDirect::ControlChange(unsigned char channel, 
				  unsigned char controlNumber,
				  unsigned char controlValue, 
				  unsigned long time)
{
    if (time == 0)
	{
	    time = GM_GetSyncTimeStamp();
	}

    if (mQueueMidi)
	{
	    QGM_Controller((GM_Song *)m_pSongVariables, time, channel, controlNumber, controlValue);
	}
    else
	{
	    GM_Controller((GM_Song *)m_pSongVariables, channel, controlNumber, controlValue);
	}
}



void HAEMidiDirect::ProgramBankChange(unsigned char channel, 
				      unsigned char programNumber,
				      unsigned char bankNumber,
				      unsigned long time)
{
    if (time == 0)
	{
	    time = GM_GetSyncTimeStamp();
	}

    if (mQueueMidi)
	{
	    QGM_Controller((GM_Song *)m_pSongVariables, time, channel, 0, bankNumber);
	    QGM_ProgramChange((GM_Song *)m_pSongVariables, time, channel, programNumber);
	}
    else
	{
	    GM_Controller((GM_Song *)m_pSongVariables, channel, 0, bankNumber);
	    GM_ProgramChange((GM_Song *)m_pSongVariables, channel, programNumber);
	}
}

void HAEMidiDirect::ProgramChange(unsigned char channel, 
				  unsigned char programNumber,
				  unsigned long time)
{
    if (time == 0)
	{
	    time = GM_GetSyncTimeStamp();
	}

    if (mQueueMidi)
	{
	    QGM_ProgramChange((GM_Song *)m_pSongVariables, time, channel, programNumber);
	}
    else
	{
	    GM_ProgramChange((GM_Song *)m_pSongVariables, channel, programNumber);
	}
}

void HAEMidiDirect::ChannelPressure(unsigned char channel, unsigned char pressure, unsigned long time)
{
    time = time;
    channel = channel;
    pressure = pressure;
}

void HAEMidiDirect::KeyPressure(unsigned char channel, unsigned char note, unsigned char pressure, unsigned long time)
{
    time = time;
    pressure = pressure;
    channel = channel;
    note = note;
}

void HAEMidiDirect::GetPitchBend(unsigned channel, unsigned char *pLSB, unsigned char *pMSB)
{
    if (m_pSongVariables && pLSB && pMSB)
	{
	    GM_GetPitchBend((GM_Song *)m_pSongVariables, channel, pLSB, pMSB);
	}
}

void HAEMidiDirect::PitchBend(unsigned char channel, 
			      unsigned char lsb, 
			      unsigned char msb,
			      unsigned long time)
{
    if (time == 0)
	{
	    time = GM_GetSyncTimeStamp();
	}

    if (mQueueMidi)
	{
	    QGM_PitchBend((GM_Song *)m_pSongVariables, time, channel, msb, lsb);
	}
    else
	{
	    GM_PitchBend((GM_Song *)m_pSongVariables, channel, msb, lsb);
	}
}


void HAEMidiDirect::AllNotesOff(unsigned long time)
{
    if (time == 0)
	{
	    time = GM_GetSyncTimeStamp();
	}

    if (mQueueMidi)
	{
	    QGM_AllNotesOff((GM_Song *)m_pSongVariables, time);
	}
    else
	{
	    GM_AllNotesOff((GM_Song *)m_pSongVariables);
	}
}


#if 0
#pragma mark ### HAESoundStream class ###
#endif

#if USE_STREAM_API

HAESoundStream::HAESoundStream(HAEAudioMixer *pHAEAudio,
			       char const *cName, void * userReference) :
    HAEAudioNoise(pHAEAudio, cName)
{
    mSoundStreamVoiceReference = 0;
    mUserReference = userReference;
    mPauseVariable = 0;
    mCallbackProc = NULL;
    mReverbState = FALSE;		// off
    mReverbAmount = 0;
    mLowPassAmount = 0;
    mResonanceAmount = 0;
    mFrequencyAmount = 0;
    mVolumeState = HAE_FIXED_1;	// 1.0
    mPanState = HAE_CENTER_PAN;	// center;
    mPrerolled = FALSE;
}


HAESoundStream::~HAESoundStream()
{
    Stop();
}

// streaming file callback. Used to decode typed files.
static OPErr PV_CustomOutputStreamCallback(void *context, GM_StreamMessage message, GM_StreamData *pAS)
{
    HAESoundStream			*pHAEStream;
    HAEStreamObjectProc		callback;
    HAEStreamData			iData;
    OPErr					theErr;
    HAEErr					igorErr;
    HAEStreamMessage		igorMessage;

    context = context;
    theErr = NO_ERR;
    igorErr = HAE_NO_ERROR;
    pHAEStream = (HAESoundStream *)pAS->userReference;
    if (pHAEStream)
	{
	    iData.userReference = (long)pHAEStream->GetReference();
	    iData.pData = pAS->pData;
	    iData.dataLength = pAS->dataLength;
	    iData.sampleRate = pAS->sampleRate;
	    iData.dataBitSize = pAS->dataBitSize;
	    iData.channelSize = pAS->channelSize;
	    iData.startSample = pAS->startSample;
	    iData.endSample = pAS->endSample;
	    callback = (HAEStreamObjectProc)pHAEStream->GetCallbackProc();
	    if (callback)
		{
		    switch (message)
			{
			default:
			    igorMessage = HAE_STREAM_NULL;
			    break;
			case STREAM_CREATE:
			    igorMessage = HAE_STREAM_CREATE;
			    break;
			case STREAM_DESTROY:
			    igorMessage = HAE_STREAM_DESTROY;
			    break;
			case STREAM_GET_DATA:
			    igorMessage = HAE_STREAM_GET_DATA;
			    break;
			case STREAM_GET_SPECIFIC_DATA:
			    igorMessage = HAE_STREAM_GET_SPECIFIC_DATA;
			    break;
			}
		    if (igorMessage != HAE_STREAM_NULL)
			{
			    igorErr = (*callback)(igorMessage, &iData);
			}
		    else
			{
			    igorErr = HAE_NOT_SETUP;
			}
		    pAS->pData = iData.pData;
		    pAS->dataLength = iData.dataLength;
		    pAS->sampleRate = iData.sampleRate;
		    pAS->dataBitSize = iData.dataBitSize;
		    pAS->channelSize = iData.channelSize;
		    pAS->startSample = iData.startSample;
		    pAS->endSample = iData.endSample;
		}
	}
    switch (igorErr)
	{
	default:
	    theErr = GENERAL_BAD;
	    break;
	case HAE_NO_ERROR:
	    theErr = NO_ERR;
	    break;
	case HAE_BUFFER_TO_SMALL:
	    theErr = BUFFER_TO_SMALL;
	    break;
	case HAE_NOT_SETUP:
	    theErr = NOT_SETUP;
	    break;
	case HAE_PARAM_ERR:
	    theErr = PARAM_ERR;
	    break;
	case HAE_MEMORY_ERR:
	    theErr = MEMORY_ERR;
	    break;
	case HAE_STREAM_STOP_PLAY:
	    theErr = STREAM_STOP_PLAY;
	    break;
	}
    return theErr;
}

// start streaming a file
HAEErr HAESoundStream::SetupFileStream(HAEPathName pWaveFilePath, 
				       HAEFileType fileType,
				       unsigned long bufferSize,			// temp buffer to read file
				       HAE_BOOL loopFile)		// TRUE will loop file
{
#if USE_HIGHLEVEL_FILE_API
    XFILENAME		theFile;
    GM_Waveform		fileInfo;
    AudioFileType	type;
    HAEErr			theErr;

    theErr = HAE_NO_ERROR;
    XConvertNativeFileToXFILENAME(pWaveFilePath, &theFile);

    type = HAE_TranslateHAEFileType(fileType);
    if (type != FILE_INVALID_TYPE)
	{
	    if (bufferSize >= HAE_MIN_STREAM_BUFFER_SIZE)
		{
		    mSoundStreamVoiceReference = GM_AudioStreamFileSetup(NULL, &theFile, type, bufferSize, &fileInfo, loopFile);
		    mStreamSampleInfo.bitSize = fileInfo.bitSize;
		    mStreamSampleInfo.channels = fileInfo.channels;
		    mStreamSampleInfo.sampledRate = fileInfo.sampledRate;
		    mStreamSampleInfo.baseMidiPitch = fileInfo.baseMidiPitch;
		    mStreamSampleInfo.waveSize = fileInfo.waveSize;
		    mStreamSampleInfo.waveFrames = fileInfo.waveFrames;
		    mStreamSampleInfo.startLoop = 0;
		    mStreamSampleInfo.endLoop = 0;
		    theErr = HAE_TranslateOPErr(GM_AudioStreamError(mSoundStreamVoiceReference));
		}
	    else
		{
		    theErr = HAE_BUFFER_TO_SMALL;
		}
	}
    else
	{
	    theErr = HAE_BAD_FILE_TYPE;
	}
    return theErr;
#else
    fileType;
    pWaveFilePath;
    bufferSize;
    loopFile;
    return HAE_NOT_SETUP;
#endif
}

HAEErr HAESoundStream::SetupCustomStream(HAEStreamObjectProc pProc, 	// control callback
					 unsigned long bufferSize, 			// buffer size 
					 HAE_UNSIGNED_FIXED sampleRate,				// Fixed 16.16
					 char dataBitSize,					// 8 or 16 bit data
					 char channelSize)					// 1 or 2 channels of date
{
    mCallbackProc = pProc;
    if (bufferSize >= HAE_MIN_STREAM_BUFFER_SIZE)
	{
	    mSoundStreamVoiceReference = GM_AudioStreamSetup(NULL, (long)this, PV_CustomOutputStreamCallback, bufferSize,
							     sampleRate, dataBitSize, channelSize);					

	    mStreamSampleInfo.bitSize = dataBitSize;
	    mStreamSampleInfo.channels = channelSize;
	    mStreamSampleInfo.baseMidiPitch = 60;
	    mStreamSampleInfo.waveSize = 0;
	    mStreamSampleInfo.waveFrames = 0;
	    mStreamSampleInfo.startLoop = 0;
	    mStreamSampleInfo.endLoop = 0;
	    mStreamSampleInfo.sampledRate = sampleRate;
	    return HAE_TranslateOPErr(GM_AudioStreamError(mSoundStreamVoiceReference));
	}
    return HAE_BUFFER_TO_SMALL;
}

// Get the position of a audio stream in samples
unsigned long HAESoundStream::GetPlaybackPosition(void)
{
    return GM_AudioStreamGetFileSamplePosition(mSoundStreamVoiceReference);
}

HAEErr HAESoundStream::GetInfo(HAESampleInfo *pInfo)
{
    if (pInfo)
	{
	    *pInfo = mStreamSampleInfo;
	}
    return HAE_NO_ERROR;
}

// This will return the last AudioStream Error
HAEErr HAESoundStream::LastError(void)
{
    return HAE_TranslateOPErr(GM_AudioStreamError(mSoundStreamVoiceReference));
}

HAEErr HAESoundStream::Preroll(void)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (mPrerolled == FALSE)
	{
	    if (mSoundStreamVoiceReference)
		{
		    GM_AudioStreamSetVolume(mSoundStreamVoiceReference,
					    FIXED_TO_SHORT_ROUNDED(mVolumeState * MAX_NOTE_VOLUME), TRUE);
		    GM_AudioStreamSetStereoPosition(mSoundStreamVoiceReference, mPanState);
		    theErr = GM_AudioStreamPreroll(mSoundStreamVoiceReference);
		    if (theErr == NO_ERR)
			{
			    mPrerolled = TRUE;
			}
		}
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return HAE_TranslateOPErr(theErr);
}

// This will start a stream once data has been loaded
HAEErr HAESoundStream::Start(void)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (mSoundStreamVoiceReference)
	{
	    Preroll();
	    theErr = GM_AudioStreamStart(mSoundStreamVoiceReference);
	    SetReverb(mReverbState);	// set current reverb state
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return HAE_TranslateOPErr(theErr);
}

// This will stop a streaming audio object and free any memory.
void HAESoundStream::Stop(HAE_BOOL startFade)
{
    short int	streamVolume;

    mPrerolled = FALSE;
    if (mSoundStreamVoiceReference)
	{
	    mPrerolled = FALSE;
	    if (IsPaused())
		{
		    Resume();
		}
	    if (startFade)
		{
		    streamVolume = GM_AudioStreamGetVolume(mSoundStreamVoiceReference);
		    GM_SetAudioStreamFadeRate(mSoundStreamVoiceReference, FLOAT_TO_FIXED(2.2), 0, streamVolume, TRUE);
		}
	    else
		{
		    GM_AudioStreamStop(NULL, mSoundStreamVoiceReference);
		    //			GM_AudioStreamDrain(NULL, mSoundStreamVoiceReference);	// wait for it to be finished
		}
	    mSoundStreamVoiceReference = DEAD_STREAM;
	}
}

// This will stop and flush the current stream and force a read of data. This
// will cause gaps in the audio.
void HAESoundStream::Flush(void)
{
    if (IsPaused())
	{
	    Resume();
	}
    GM_AudioStreamFlush(mSoundStreamVoiceReference);
}

// Returns TRUE or FALSE if a given AudioStream is still active
HAE_BOOL HAESoundStream::IsPlaying(void)
{
    if (mSoundStreamVoiceReference)
	{
	    return (HAE_BOOL)GM_IsAudioStreamPlaying(mSoundStreamVoiceReference);
	}
    return FALSE;
}

HAE_BOOL HAESoundStream::IsDone(void)
{
    if (IsPlaying() == FALSE)
	{
	    return TRUE;
	}
    return FALSE;
}

// Returns TRUE if a given AudioStream is valid
HAE_BOOL HAESoundStream::IsValid(void)
{
    return (HAE_BOOL)GM_IsAudioStreamValid(mSoundStreamVoiceReference);
}

// Set the volume level of a audio stream
void HAESoundStream::SetVolume(HAE_UNSIGNED_FIXED newVolume)
{
    mVolumeState = newVolume;
    GM_AudioStreamSetVolume(mSoundStreamVoiceReference,
			    FIXED_TO_SHORT_ROUNDED(newVolume * MAX_NOTE_VOLUME), FALSE);
}

// Get the volume level of a audio stream
HAE_UNSIGNED_FIXED HAESoundStream::GetVolume(void)
{
    if (mSoundStreamVoiceReference)
	{
	    return UNSIGNED_RATIO_TO_FIXED(GM_AudioStreamGetVolume(mSoundStreamVoiceReference), MAX_NOTE_VOLUME);
	}
    return mVolumeState;
}

void HAESoundStream::Fade(HAE_BOOL doAsync)
{
    short int	sampleVolume;

    sampleVolume = GM_AudioStreamGetVolume(mSoundStreamVoiceReference);
    if (doAsync == FALSE)
	{
	    // We're going to fade the stream, but don't stop

	    GM_SetAudioStreamFadeRate(mSoundStreamVoiceReference, FLOAT_TO_FIXED(2.2),
				      0, sampleVolume, FALSE);
	    while (GM_AudioStreamGetVolume(mSoundStreamVoiceReference) && GM_IsAudioStreamPlaying(mSoundStreamVoiceReference)) 
		{
		    GetMixer()->ServiceAudioOutputToFile();
		    GetMixer()->ServiceIdle();
		    XWaitMicroseocnds(1000);
		}
	}
    else
	{
	    GM_SetAudioStreamFadeRate(mSoundStreamVoiceReference, FLOAT_TO_FIXED(2.2), 0, sampleVolume, FALSE);
	}
}


void HAESoundStream::FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync)
{
    short int	songVolume, saveVolume, newSongVolume, saveNewSongVolume;
    XFIXED		delta;

    if (mSoundStreamVoiceReference)
	{
	    newSongVolume = FIXED_TO_SHORT_ROUNDED(destVolume * MAX_SONG_VOLUME);
	    saveNewSongVolume = newSongVolume;
	    // We're going to fade the song out before we stop it.		
	    songVolume = GM_AudioStreamGetVolume(mSoundStreamVoiceReference);
	    saveVolume = songVolume;

	    if (newSongVolume < songVolume)
		{	// fade out
		    songVolume = newSongVolume;
		    newSongVolume = saveVolume;
		    delta = FLOAT_TO_UNSIGNED_FIXED(2.2);
		}
	    else
		{	// fade in
		    delta = (XFIXED)FLOAT_TO_FIXED(-2.2);
		}
	    if (doAsync == FALSE)
		{
		    GM_SetAudioStreamFadeRate(mSoundStreamVoiceReference, delta, songVolume, newSongVolume, FALSE);
		    while (GM_AudioStreamGetVolume(mSoundStreamVoiceReference) != saveNewSongVolume)
			{
			    GetMixer()->ServiceAudioOutputToFile();
			    GetMixer()->ServiceIdle();
			    XWaitMicroseocnds(1000);
			}
		}
	    else
		{
		    GM_SetAudioStreamFadeRate(mSoundStreamVoiceReference, delta, songVolume, newSongVolume, FALSE);
		}
	}
}

// Set the sample rate of a audio stream
void HAESoundStream::SetRate(HAE_UNSIGNED_FIXED newRate)
{
    GM_AudioStreamSetRate(mSoundStreamVoiceReference, newRate);
}

// Get the sample rate of a audio stream
HAE_UNSIGNED_FIXED HAESoundStream::GetRate(void)
{
    return GM_AudioStreamGetRate(mSoundStreamVoiceReference);
}

// Set the stereo position of a audio stream
void HAESoundStream::SetStereoPosition(short int stereoPosition)
{
    mPanState = stereoPosition;
    GM_AudioStreamSetStereoPosition(mSoundStreamVoiceReference, stereoPosition);
}

// Get the stereo position of a audio stream
short int HAESoundStream::GetStereoPosition(void)
{
    if (mSoundStreamVoiceReference)
	{
	    mPanState = GM_AudioStreamGetStereoPosition(mSoundStreamVoiceReference);
	}
    return mPanState;
}

// Enable/Disable reverb on this particular audio stream
void HAESoundStream::SetReverb(HAE_BOOL useReverb)
{
    mReverbState = useReverb;
    if (mSoundStreamVoiceReference)
	{
	    GM_AudioStreamReverb(mSoundStreamVoiceReference, useReverb);
	}
}

HAE_BOOL HAESoundStream::GetReverb(void)
{
    if (mSoundStreamVoiceReference)
	{
	    mReverbState = GM_AudioStreamGetReverb(mSoundStreamVoiceReference);
	}
    return mReverbState;
}

void HAESoundStream::SetReverbAmount(short int reverbAmount)
{
    mReverbAmount = reverbAmount;
    SetReverb((reverbAmount) ? TRUE : FALSE);
    if (mSoundStreamVoiceReference)
	{
	    GM_SetStreamReverbAmount(mSoundStreamVoiceReference, reverbAmount);
	}
}

short int HAESoundStream::GetReverbAmount(void)
{
    if (mSoundStreamVoiceReference)
	{
	    mReverbAmount = GM_GetStreamReverbAmount(mSoundStreamVoiceReference);
	}
    return mReverbAmount;
}

void HAESoundStream::SetLowPassAmountFilter(short int lowpassamount)
{
    mLowPassAmount = lowpassamount;
    if (mSoundStreamVoiceReference)
	{
	    GM_AudioStreamSetLowPassAmountFilter(mSoundStreamVoiceReference, lowpassamount);
	}
}

short int HAESoundStream::GetLowPassAmountFilter(void)
{
    if (mSoundStreamVoiceReference)
	{
	    mLowPassAmount = GM_AudioStreamGetLowPassAmountFilter(mSoundStreamVoiceReference);
	}
    return mLowPassAmount;
}

void HAESoundStream::SetResonanceAmountFilter(short int resonance)
{
    mResonanceAmount = resonance;
    if (mSoundStreamVoiceReference)
	{
	    GM_AudioStreamSetResonanceFilter(mSoundStreamVoiceReference, resonance);
	}
}

short int HAESoundStream::GetResonanceAmountFilter(void)
{
    if (mSoundStreamVoiceReference)
	{
	    mResonanceAmount = GM_AudioStreamGetResonanceFilter(mSoundStreamVoiceReference);
	}
    return mResonanceAmount;
}

void HAESoundStream::SetFrequencyAmountFilter(short int frequency)
{
    mFrequencyAmount = frequency;
    if (mSoundStreamVoiceReference)
	{
	    GM_AudioStreamSetFrequencyFilter(mSoundStreamVoiceReference, frequency);
	}
}

short int HAESoundStream::GetFrequencyAmountFilter(void)
{
    if (mSoundStreamVoiceReference)
	{
	    mFrequencyAmount = GM_AudioStreamGetFrequencyFilter(mSoundStreamVoiceReference);
	}
    return mFrequencyAmount;
}

// currently paused
HAE_BOOL HAESoundStream::IsPaused(void)
{
    return (mPauseVariable) ? (HAE_BOOL)TRUE : (HAE_BOOL)FALSE;
}

// Pause the stream
void HAESoundStream::Pause(void)
{
    if (mPauseVariable == 0)
	{
	    mPauseVariable = (unsigned long)GetRate();
	    SetRate(0L);	// pause samples in their tracks
	}
}

// Resume the stream
void HAESoundStream::Resume(void)
{
    if (mPauseVariable)
	{
	    SetRate((HAE_UNSIGNED_FIXED)mPauseVariable);
	    mPauseVariable = 0;
	}
}
#endif	// USE_STREAM_API

#if 0
#pragma mark ### HAESound class ###
#endif

HAESound::HAESound(HAEAudioMixer *pHAEAudio, 
		   char const *cName, void * userReference) :
    HAEAudioNoise(pHAEAudio, cName)
{
    userReference = userReference;
    mSoundVoiceReference = DEAD_VOICE;
    pauseVariable = 0;
    pFileVariables = NULL;
    pSoundVariables = NULL;
    pSampleFrameVariable = NULL;
    mReverbState = FALSE;	// off
    mReverbAmount = 0;
    mLowPassAmount = 0;
    mResonanceAmount = 0;
    mFrequencyAmount = 0;
    cName = cName;
    mDoneCallback = NULL;
    mLoopDoneCallback = NULL;
    mCallbackReference = NULL;
    mSoundVolume = 0;
    mStereoPosition = HAE_CENTER_PAN;
}


HAESound::~HAESound()
{
    GM_SampleCallbackEntry	*pNext, *pLast;

    Stop();

    // remove callback links
    pNext = (GM_SampleCallbackEntry *)pSampleFrameVariable;
    while (pNext)
	{
	    pLast = pNext;
	    pNext = pNext->pNext;
	    XDisposePtr((XPTR)pLast);
	}

    if (pFileVariables)
	{
	    GM_FreeWaveform((GM_Waveform *)pFileVariables);
	    pFileVariables = NULL;
	}
    if (pSoundVariables)
	{
	    XDisposePtr(pSoundVariables);
	    pSoundVariables = NULL;
	}
}

// currently paused
HAE_BOOL HAESound::IsPaused(void)
{
    return (pauseVariable) ? (HAE_BOOL)TRUE : (HAE_BOOL)FALSE;
}

void HAESound::Resume(void)
{
    if (pauseVariable)
	{
	    SetRate((HAE_UNSIGNED_FIXED)pauseVariable);
	    pauseVariable = 0;
	}
}

void HAESound::Pause(void)
{
    if (pauseVariable == 0)
	{
	    pauseVariable = (unsigned long)GetRate();
	    SetRate(0L);	// pause samples in their tracks
	}
}

// load a sample playing from a formatted block of memory. The memory will be deallocated 
// when this object is destroyed. Call start once loaded to start the playback.
HAEErr HAESound::LoadMemorySample(void *pMemoryFile, unsigned long memoryFileSize, HAEFileType fileType)
{
#if USE_HIGHLEVEL_FILE_API
    OPErr			theErr;
    AudioFileType	type;

    theErr = NO_ERR;
    pFileVariables = NULL;
    type = HAE_TranslateHAEFileType(fileType);
    if (type != FILE_INVALID_TYPE)
	{
	    pFileVariables = GM_ReadFileIntoMemoryFromMemory(pMemoryFile, memoryFileSize, type, &theErr);
	    if ( (pFileVariables == NULL) && (theErr == NO_ERR) )
		{
		    theErr = BAD_FILE;
		}
	}
    else
	{
	    theErr = BAD_FILE_TYPE;
	}
    return HAE_TranslateOPErr(theErr);
#else
    pMemoryFile = pMemoryFile;
    memoryFileSize = memoryFileSize;
    fileType = fileType;
    return HAE_NOT_SETUP;
#endif
}

// Load file into sound object. This will copy the file directly into memory. It
// will get disposed once you destroy this object.
HAEErr HAESound::LoadFileSample(HAEPathName pWaveFilePath, HAEFileType fileType)
{
#if USE_HIGHLEVEL_FILE_API
    XFILENAME		theFile;
    OPErr			theErr;
    AudioFileType	type;

    theErr = NO_ERR;
    XConvertNativeFileToXFILENAME(pWaveFilePath, &theFile);
    pFileVariables = NULL;
    type = HAE_TranslateHAEFileType(fileType);
    if (type != FILE_INVALID_TYPE)
	{
	    pFileVariables = GM_ReadFileIntoMemory(&theFile, type, &theErr);
	}
    else
	{
	    theErr = BAD_FILE_TYPE;
	}
    if ((pFileVariables == NULL) && (theErr == NO_ERR))
	{
	    theErr = BAD_FILE;
	}
    return HAE_TranslateOPErr(theErr);
#else
    fileType;
    pWaveFilePath;
    return HAE_NOT_SETUP;
#endif
}


HAEErr HAESound::AddSampleFrameCallback(unsigned long frame, HAESampleFrameCallbackPtr pCallback, void * pReference)
{
    GM_SampleCallbackEntry	*pEntry;
    GM_SampleCallbackEntry	*pNext;
    OPErr					theErr;

    theErr = MEMORY_ERR;
    pEntry = (GM_SampleCallbackEntry *)XNewPtr((long)sizeof(GM_SampleCallbackEntry));
    if (pEntry)
	{
	    pEntry->frameOffset = frame;
	    pEntry->pCallback = (GM_SampleFrameCallbackPtr)pCallback;
	    pEntry->reference = (long)pReference;

	    // add to linked list
	    pNext = (GM_SampleCallbackEntry *)pSampleFrameVariable;
	    while (pNext)
		{
		    if (pNext->pNext == NULL)
			{
			    break;
			}
		    else
			{
			    pNext = pNext->pNext;
			}
		}
	    if (pNext == NULL)
		{
		    pSampleFrameVariable = (void *)pEntry;
		}
	    else
		{
		    pNext->pNext = pEntry;
		}
	    theErr = NO_ERR;
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAESound::RemoveSampleFrameCallback(unsigned long frame, HAESampleFrameCallbackPtr pCallback)
{
    GM_SampleCallbackEntry	*pEntry;
    GM_SampleCallbackEntry	*pNext, *pLast;
    OPErr					theErr;

    // find link
    pNext = (GM_SampleCallbackEntry *)pSampleFrameVariable;
    pEntry = NULL;
    while (pNext)
	{
	    if (pNext->frameOffset == frame)
		{
		    if (pNext->pCallback == (GM_SampleFrameCallbackPtr)pCallback)
			{
			    pEntry = pNext;	// found
			    break;
			}
		}
	    pNext = pNext->pNext;
	}
    theErr = PARAM_ERR;
    if (pEntry)
	{
	    // remove from linked list
	    pLast = pNext = (GM_SampleCallbackEntry *)pSampleFrameVariable;
	    while (pNext)
		{
		    if (pNext == pEntry)								// found object in list?
			{
			    if (pNext == (GM_SampleCallbackEntry *)pSampleFrameVariable)			// is object the top object
				{
				    pSampleFrameVariable = (void *)pNext->pNext;		// yes, change to next object
				}
			    else
				{
				    if (pLast)									// no, change last to point beyond next
					{
					    pLast->pNext = pNext->pNext;
					}
				}
			    break;
			}
		    pLast = pNext;
		    pNext = pNext->pNext;
		}
	    theErr = NO_ERR;
	}
    return HAE_TranslateOPErr(theErr);
}



// load a custom sample. This will copy sample data into memory.
HAEErr HAESound::LoadCustomSample(void * sampleData,
				  unsigned long frames,
				  unsigned short int bitSize,
				  unsigned short int channels,
				  HAE_UNSIGNED_FIXED rate,
				  unsigned long loopStart,
				  unsigned long loopEnd)
{
    GM_Waveform 	*pWave;
    OPErr			theErr;
    long			size;

    theErr = NO_ERR;
    size = frames * channels * (bitSize / 8);
    pSoundVariables = XNewPtr(size);
    if (pSoundVariables)
	{
	    XBlockMove(sampleData, pSoundVariables, size);
	    pWave = (GM_Waveform *)XNewPtr((long)sizeof(GM_Waveform));
	    if (pWave)
		{
		    pWave->waveSize = size;
		    pWave->waveFrames = frames;
		    pWave->startLoop = loopStart;
		    pWave->endLoop = loopEnd;
		    pWave->baseMidiPitch = 60;
		    pWave->bitSize = (unsigned char)bitSize;
		    pWave->channels = (unsigned char)channels;
		    pWave->sampledRate = rate;
	
		    pWave->theWaveform = (SBYTE *)pSoundVariables;
		}
	    else
		{
		    XDisposePtr(pSoundVariables);
		    pSoundVariables = NULL;
		}
	    pFileVariables = pWave;
	}
    else
	{
	    theErr = MEMORY_ERR;
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAESound::LoadResourceSample(void *pResource, unsigned long resourceSize)
{
    SampleDataInfo	newSoundInfo;
    GM_Waveform 	*pWave;
    XPTR			theData;
    OPErr			theErr;

    theErr = NO_ERR;
    if (pResource)
	{
	    theData = XNewPtr(resourceSize);
	    if (theData)
		{
		    XBlockMove(pResource, theData, resourceSize);
		    pResource = theData;
		    theData = XGetSamplePtrFromSnd(pResource, &newSoundInfo);
		    if (theData)
			{
			    pSoundVariables = pResource;
			    pWave = (GM_Waveform *)XNewPtr((long)sizeof(GM_Waveform));
			    if (pWave)
				{
				    pWave->waveSize = newSoundInfo.size;
				    pWave->waveFrames = newSoundInfo.frames;
				    pWave->startLoop = newSoundInfo.loopStart;
				    pWave->endLoop = newSoundInfo.loopEnd;
				    pWave->baseMidiPitch = (unsigned char)newSoundInfo.baseKey;
				    pWave->bitSize = (unsigned char)newSoundInfo.bitSize;
				    pWave->channels = (unsigned char)newSoundInfo.channels;
				    pWave->sampledRate = newSoundInfo.rate;
				    pWave->theWaveform = (SBYTE *)theData;

				    pFileVariables = pWave;
				}
			}
		    else
			{
			    XDisposePtr(pResource);
			    theErr = MEMORY_ERR;
			}
		}
	    else
		{
		    theErr = MEMORY_ERR;
		}
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAESound::LoadBankSample(char *cName)
{
    OPErr			theErr;
    XPTR			thePreSound, theData;
    GM_Waveform 	*pWave;
    SampleDataInfo	newSoundInfo;
    long			size;

    theErr = BAD_SAMPLE;

#if X_PLATFORM != X_MACINTOSH
    if (thePatchFile)
#endif
	{
	    theData = XGetSoundResourceByName(cName, &size);
	    pSoundVariables = theData;
	    if (theData)
		{
		    // convert snd resource into a simple pointer of data with information

		    thePreSound = XGetSamplePtrFromSnd(theData, &newSoundInfo);

		    if (newSoundInfo.pMasterPtr != theData)
			{	// this means that XGetSamplePtrFromSnd created a new sample
			    XDisposePtr(theData);
			}
		    if (thePreSound)
			{
			    pSoundVariables = newSoundInfo.pMasterPtr;
			    pWave = (GM_Waveform *)XNewPtr((long)sizeof(GM_Waveform));
			    if (pWave)
				{
				    pWave->waveSize = newSoundInfo.size;
				    pWave->waveFrames = newSoundInfo.frames;
				    pWave->startLoop = newSoundInfo.loopStart;
				    pWave->endLoop = newSoundInfo.loopEnd;
				    pWave->baseMidiPitch = (unsigned char)newSoundInfo.baseKey;
				    pWave->bitSize = (unsigned char)newSoundInfo.bitSize;
				    pWave->channels = (unsigned char)newSoundInfo.channels;
				    pWave->sampledRate = newSoundInfo.rate;
				    pWave->theWaveform = (SBYTE *)thePreSound;

				    pFileVariables = pWave;
				    theErr = NO_ERR;
				}
			}
		    else
			{
			    theErr = MEMORY_ERR;
			}
		}
	    else
		{
		    theErr = BAD_SAMPLE;
		}
	}
    return HAE_TranslateOPErr(theErr);
}

// set the loop points in sample frames
HAEErr HAESound::SetSampleLoopPoints(unsigned long start, unsigned long end)
{
    GM_Waveform 	*pWave;
    HAEErr			theErr;

    theErr = HAE_NO_ERROR;
    if (pFileVariables)
	{
	    if (start > end)
		{
		    theErr = HAE_PARAM_ERR;
		}
	    else
		{
		    if ( (end - start) < MIN_LOOP_SIZE)
			{
			    theErr = HAE_BUFFER_TO_SMALL;
			}
		}
	    if (theErr == HAE_NO_ERROR)
		{
		    pWave = (GM_Waveform *)pFileVariables;
		    pWave->startLoop = start;
		    pWave->endLoop = end;
		    if (mSoundVoiceReference != DEAD_VOICE)
			{
			    GM_SetSampleLoopPoints(mSoundVoiceReference, start, end);
			}
		}
	}
    else
	{
	    theErr = HAE_NOT_SETUP;
	}
    return theErr;
}

// Get the current loop points in sample frames
HAEErr HAESound::GetSampleLoopPoints(unsigned long *pStart, unsigned long *pEnd)
{
    GM_Waveform 	*pWave;
    HAEErr			theErr;

    theErr = HAE_NO_ERROR;
    if (pFileVariables && pStart && pEnd)
	{
	    pWave = (GM_Waveform *)pFileVariables;
	    *pStart = pWave->startLoop;
	    *pEnd = pWave->endLoop;
	}
    else
	{
	    theErr = HAE_NOT_SETUP;
	}
    return theErr;
}

// Get the sample point offset by a sample frame count
void * HAESound::GetSamplePointer(unsigned long sampleFrame)
{
    char			*pSample;
    GM_Waveform 	*pWave;

    pSample = NULL;
    if (pFileVariables)
	{
	    pWave = (GM_Waveform *)pFileVariables;
	    pSample = (char *)pWave->theWaveform;
	    if (pSample)
		{
		    pSample += sampleFrame * (pWave->bitSize / 8) * pWave->channels;
		}
	}
    return (void *)pSample;
}

// Get the mixer pointer for this sample as its being mixed. This pointer is 
// the actual pointer used by the mixer. This will change if this object is
// an double buffer playback.
void * HAESound::GetSamplePointerFromMixer(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    return GM_GetSamplePlaybackPointer(mSoundVoiceReference);
	}
    return NULL;
}

// Get the position of a audio playback in samples
unsigned long HAESound::GetPlaybackPosition(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    return GM_GetSamplePlaybackPosition(mSoundVoiceReference);
	}
    return 0;
}

HAEErr HAESound::GetInfo(HAESampleInfo *pInfo)
{
    GM_Waveform 	*pWave;
    HAEErr			theErr;

    theErr = HAE_NO_ERROR;
    if (pFileVariables)
	{
	    pWave = (GM_Waveform *)pFileVariables;
	    pInfo->waveSize = pWave->waveSize;
	    pInfo->waveFrames = pWave->waveFrames;
	    pInfo->startLoop = pWave->startLoop;
	    pInfo->endLoop = pWave->endLoop;
	    pInfo->baseMidiPitch = pWave->baseMidiPitch;
	    pInfo->bitSize = pWave->bitSize;
	    pInfo->channels = pWave->channels;
	    pInfo->sampledRate = pWave->sampledRate;
	}
    else
	{
	    theErr = HAE_NOT_SETUP;
	}
    return theErr;
}

// This is the default loop callback, which tells the mixer to always loop samples
static XBOOL PV_DefaultSampleLoopCallback(void *context)
{
    HAESound				*pSound;
    GM_LoopDoneCallbackPtr	userCallback;

    pSound = (HAESound *)context;
    if (pSound)
	{
	    userCallback = (GM_LoopDoneCallbackPtr)pSound->GetLoopDoneCallback();
	    if (userCallback)
		{
		    return (*userCallback)(pSound->GetDoneCallbackReference());
		}
	}
    return TRUE;	// always loop
}

void HAESound::DefaultSampleDoneCallback(void)
{
    HAEDoneCallbackPtr	userCallback;

    userCallback = GetDoneCallback();
    if (userCallback)
	{
	    (*userCallback)(GetDoneCallbackReference());
	}
    mSoundVoiceReference = DEAD_VOICE;	// voice dead
}

// Glue "C" code between Gen API and the HAE C++ API
static void PV_DefaultSampleDoneCallback(void *context)
{
    HAESound				*pSound;

    pSound = (HAESound *)context;
    if (pSound)
	{
	    pSound->DefaultSampleDoneCallback();
	}
}

// private function that sets everything up prior to starting a sample
HAEErr HAESound::PreStart(HAE_UNSIGNED_FIXED sampleVolume, 			// sample volume
			  short int stereoPosition,					// stereo placement
			  void * refData, 							// callback reference
			  HAELoopDoneCallbackPtr pLoopContinueProc,
			  HAEDoneCallbackPtr pDoneProc,
			  HAE_BOOL stopIfPlaying)
{
    OPErr	theErr;
    long	volume;

    theErr = NO_ERR;
    mDoneCallback = pDoneProc;
    mLoopDoneCallback = pLoopContinueProc;
    mCallbackReference = refData;
    mSoundVolume = sampleVolume;
    mStereoPosition = stereoPosition;
    if (pFileVariables)
	{
	    if (mSoundVoiceReference != DEAD_VOICE)
		{
		    if (IsPlaying())		// sample playing?
			{
			    if (stopIfPlaying)	// can we stop it
				{
				    Stop();
				}
			    else
				{
				    theErr = STILL_PLAYING;
				}
			}
		}
	    if (theErr == NO_ERR)
		{
		    mSoundVoiceReference = DEAD_VOICE;
		    volume = UNSIGNED_FIXED_TO_LONG_ROUNDED(sampleVolume * MAX_NOTE_VOLUME);
		    if (volume)
			{
			    if (((GM_Waveform *)pFileVariables)->waveFrames > MAX_SAMPLE_FRAMES)
				{
				    theErr = SAMPLE_TO_LARGE;
				}
			}
		    else
			{
			    theErr = NO_VOLUME;
			}
		}
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAESound::Start(HAE_UNSIGNED_FIXED sampleVolume, 			// sample volume
		       short int stereoPosition,					// stereo placement
		       void * refData, 							// callback reference
		       HAELoopDoneCallbackPtr pLoopContinueProc,
		       HAEDoneCallbackPtr pDoneProc,
		       unsigned long startOffsetFrame,
		       HAE_BOOL stopIfPlaying)
{
    OPErr	theErr;
    long	volume;

    theErr = HAE_TranslateHAErr(
				PreStart(sampleVolume, stereoPosition, refData, 
					 pLoopContinueProc, pDoneProc, stopIfPlaying)
				);
    if (theErr == NO_ERR)
	{
	    mSoundVoiceReference = DEAD_VOICE;
	    volume = UNSIGNED_FIXED_TO_LONG_ROUNDED(sampleVolume * MAX_NOTE_VOLUME);
	    if (volume)
		{
		    mSoundVoiceReference = GM_SetupSampleFromInfo((GM_Waveform *)pFileVariables, (void *)this, 
								  volume,
								  mStereoPosition,
								  (GM_LoopDoneCallbackPtr)PV_DefaultSampleLoopCallback, 
								  PV_DefaultSampleDoneCallback,
								  startOffsetFrame);
		    if (mSoundVoiceReference == DEAD_VOICE)
			{
			    theErr = NO_FREE_VOICES;
			}
		    else
			{
			    GM_StartSample(mSoundVoiceReference);
			    GM_SetSampleOffsetCallbackLinks(mSoundVoiceReference, 
							    (GM_SampleCallbackEntry *)pSampleFrameVariable);
			    SetReverb(mReverbState);	// set current reverb state
			}
		}
	}
    return HAE_TranslateOPErr(theErr);
}

// This will setup a HAESound once data has been loaded. Call __Start to start playing
HAEErr HAESound::__Setup(HAE_UNSIGNED_FIXED sampleVolume, 		// sample volume	(1.0)
			 short int stereoPosition,			// stereo placement -63 to 63
			 void * refData, 								// callback reference
			 HAELoopDoneCallbackPtr pLoopContinueProc,
			 HAEDoneCallbackPtr pDoneProc,
			 unsigned long startOffsetFrame,				// starting offset in frames
			 HAE_BOOL stopIfPlaying)						// TRUE will restart sound otherwise return and error
{

    OPErr	theErr;
    long	volume;

    theErr = HAE_TranslateHAErr(
				PreStart(sampleVolume, stereoPosition, refData, 
					 pLoopContinueProc, pDoneProc, stopIfPlaying)
				);
    if (theErr == NO_ERR)
	{
	    mSoundVoiceReference = DEAD_VOICE;
	    volume = UNSIGNED_FIXED_TO_LONG_ROUNDED(sampleVolume * MAX_NOTE_VOLUME);
	    if (volume)
		{
		    mSoundVoiceReference = GM_SetupSampleFromInfo((GM_Waveform *)pFileVariables, (void *)this, 
								  volume,
								  mStereoPosition,
								  (GM_LoopDoneCallbackPtr)PV_DefaultSampleLoopCallback, 
								  PV_DefaultSampleDoneCallback,
								  startOffsetFrame);
		    if (mSoundVoiceReference == DEAD_VOICE)
			{
			    theErr = NO_FREE_VOICES;
			}
		}
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAESound::__Start(void)
{
    HAEErr	err;

    err = HAE_NOT_SETUP;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_StartSample(mSoundVoiceReference);
	    GM_SetSampleOffsetCallbackLinks(mSoundVoiceReference, 
					    (GM_SampleCallbackEntry *)pSampleFrameVariable);
	    SetReverb(mReverbState);	// set current reverb state
	    err = HAE_NO_ERROR;
	}
    return err;
}

void HAESound::Stop(HAE_BOOL startFade)
{
    short int	sampleVolume;

    if (IsPaused())
	{
	    Resume();
	}
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    if (startFade)
		{
		    sampleVolume = GM_GetSampleVolume(mSoundVoiceReference);
		    GM_SetSampleFadeRate(mSoundVoiceReference, FLOAT_TO_FIXED(2.2),
					 0, sampleVolume, TRUE);
		}
	    else
		{
		    //$$fb 2002-04-20: use thread safe version of GM_EndSample
		    GM_ReleaseSample(mSoundVoiceReference);
		    GM_SetSampleOffsetCallbackLinks(mSoundVoiceReference, NULL);
		}
	}
    mSoundVoiceReference = DEAD_VOICE;	// done
}

// This will, given all the information about a sample, will play sample memory without
// copying the data. Be carefull and do not dispose of the memory associated with this sample
// while its playing. Call __Start to start sound
HAEErr HAESound::__SetupCustom(void * sampleData,					// pointer to audio data
			       unsigned long frames, 				// number of frames of audio
			       unsigned short int bitSize, 		// bits per sample 8 or 16
			       unsigned short int channels, 		// mono or stereo 1 or 2
			       HAE_UNSIGNED_FIXED rate, 			// 16.16 fixed sample rate
			       unsigned long loopStart, 			// loop start in frames
			       unsigned long loopEnd,				// loop end in frames
			       HAE_UNSIGNED_FIXED sampleVolume, 	// sample volume	(1.0)
			       short int stereoPosition,			// stereo placement
			       void *refData, 						// callback reference
			       HAELoopDoneCallbackPtr pLoopContinueProc,
			       HAEDoneCallbackPtr pDoneProc,
			       HAE_BOOL stopIfPlaying)
{
    OPErr	theErr;
    long	volume;

    theErr = HAE_TranslateHAErr(
				PreStart(sampleVolume, stereoPosition, refData, 
					 pLoopContinueProc, pDoneProc, stopIfPlaying)
				);
    if (theErr == NO_ERR)
	{
	    mSoundVoiceReference = DEAD_VOICE;
	    volume = UNSIGNED_FIXED_TO_LONG_ROUNDED(sampleVolume * MAX_NOTE_VOLUME);
	    if (volume)
		{
		    mSoundVoiceReference = GM_SetupSample((XPTR)sampleData, frames, rate, 
							  loopStart, loopEnd, 0, 
							  volume, 
							  stereoPosition,
							  (void *)this, 
							  bitSize, channels, 
							  (GM_LoopDoneCallbackPtr)PV_DefaultSampleLoopCallback, 
							  PV_DefaultSampleDoneCallback);
		    if (mSoundVoiceReference == DEAD_VOICE)
			{
			    theErr = NO_FREE_VOICES;
			}
		}
	}
    return HAE_TranslateOPErr(theErr);
}

HAEErr HAESound::StartCustomSample(void * sampleData,					// pointer to audio data
				   unsigned long frames, 				// number of frames of audio
				   unsigned short int bitSize, 		// bits per sample 8 or 16
				   unsigned short int channels, 		// mono or stereo 1 or 2
				   HAE_UNSIGNED_FIXED rate, 			// 16.16 fixed sample rate
				   unsigned long loopStart, 			// loop start in frames
				   unsigned long loopEnd,				// loop end in frames
				   HAE_UNSIGNED_FIXED sampleVolume, 	// sample volume	(1.0)
				   short int stereoPosition,			// stereo placement
				   void *refData, 						// callback reference
				   HAELoopDoneCallbackPtr pLoopContinueProc,
				   HAEDoneCallbackPtr pDoneProc,
				   HAE_BOOL stopIfPlaying)
{
    OPErr	theErr;
    long	volume;

    theErr = NO_ERR;
    mDoneCallback = pDoneProc;
    mLoopDoneCallback = pLoopContinueProc;
    mCallbackReference = refData;
    mSoundVolume = sampleVolume;
    mStereoPosition = stereoPosition;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    if (IsPlaying())		// sample playing?
		{
		    if (stopIfPlaying)	// can we stop it
			{
			    Stop();
			}
		    else
			{
			    theErr = STILL_PLAYING;
			}
		}
	}
    if (theErr == NO_ERR)
	{
	    mSoundVoiceReference = DEAD_VOICE;
	    volume = UNSIGNED_FIXED_TO_LONG_ROUNDED(sampleVolume * MAX_NOTE_VOLUME);
	    if (volume)
		{
		    if (frames < MAX_SAMPLE_FRAMES)
			{
			    mSoundVoiceReference = GM_SetupSample((XPTR)sampleData, frames, rate, 
								  loopStart, loopEnd, 0, 
								  volume, stereoPosition,
								  (void *)this, 
								  bitSize, channels, 
								  (GM_LoopDoneCallbackPtr)PV_DefaultSampleLoopCallback, 
								  PV_DefaultSampleDoneCallback);
			    if (mSoundVoiceReference == DEAD_VOICE)
				{
				    theErr = NO_FREE_VOICES;
				}
			    else
				{
				    GM_StartSample(mSoundVoiceReference);
				    GM_SetSampleOffsetCallbackLinks(mSoundVoiceReference, 
								    (GM_SampleCallbackEntry *)pSampleFrameVariable);
				    SetReverb(mReverbState);	// set current reverb state
				}
			}
		    else
			{
			    theErr = SAMPLE_TO_LARGE;
			}
		}
	    else
		{
		    theErr = NO_ERR;
		}
	}
    else
	{
	    theErr = NO_VOLUME;
	}
    return HAE_TranslateOPErr(theErr);
}


HAEErr HAESound::StartDoubleBuffer(	void *buffer1,						// pointer to audio data 1 & 2
					void *buffer2,
					unsigned long frames, 				// number of frames of audio
					unsigned short int bitSize, 		// bits per sample 8 or 16
					unsigned short int channels, 		// mono or stereo 1 or 2
					HAE_UNSIGNED_FIXED rate, 			// 16.16 fixed sample rate
					HAE_UNSIGNED_FIXED sampleVolume, 	// sample volume	(1.0)
					short int stereoPosition,			// stereo placement
					void *refData, 						// callback reference
					HAEDoubleBufferCallbackPtr pDoubleBufferCallback,
					HAE_BOOL stopIfPlaying)
{
    OPErr	theErr;
    long	volume;

    mDoneCallback = NULL;
    mLoopDoneCallback = NULL;
    mCallbackReference = refData;
    mSoundVolume = sampleVolume;
    mStereoPosition = stereoPosition;
    theErr = NO_ERR;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    if (IsPlaying())		// sample playing?
		{
		    if (stopIfPlaying)	// can we stop it
			{
			    Stop();
			}
		    else
			{
			    theErr = STILL_PLAYING;
			}
		}
	}
    if (theErr == NO_ERR)
	{
	    mSoundVoiceReference = DEAD_VOICE;
	    volume = UNSIGNED_FIXED_TO_LONG_ROUNDED(sampleVolume * MAX_NOTE_VOLUME);
	    if (volume)
		{
		    mSoundVoiceReference = GM_SetupSampleDoubleBuffer((XPTR)buffer1, (XPTR)buffer2, frames, 
								      rate, bitSize, channels,
								      volume, stereoPosition,
								      (void *)this, 
								      (GM_DoubleBufferCallbackPtr)pDoubleBufferCallback,
								      PV_DefaultSampleDoneCallback);
		    if (mSoundVoiceReference == DEAD_VOICE)
			{
			    theErr = NO_FREE_VOICES;
			}
		    else
			{
				// ok
			    GM_StartSample(mSoundVoiceReference);
			    SetReverb(mReverbState);	// set current reverb state
			}
		}
	    else
		{
		    theErr = NO_ERR;
		}
	}
    else
	{
	    theErr = NO_VOLUME;
	}
    return HAE_TranslateOPErr(theErr);
}

// Set a call back when song is done
void HAESound::SetDoneCallback(HAEDoneCallbackPtr pDoneProc, void * pReference)
{
    mDoneCallback = pDoneProc;
    mCallbackReference = pReference;
}


HAE_BOOL HAESound::IsPlaying(void)
{
    if (IsDone() == FALSE)
	{
	    return TRUE;
	}
    return FALSE;
}

HAE_BOOL HAESound::IsDone(void)
{
    HAE_BOOL	done;

    done = TRUE;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    done = (HAE_BOOL)GM_IsSoundDone(mSoundVoiceReference);
	    if (done)
		{
		    mSoundVoiceReference = DEAD_VOICE;
		}
	}
    return done;
}

void HAESound::SetRate(HAE_UNSIGNED_FIXED newRate)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_ChangeSamplePitch(mSoundVoiceReference, newRate);
	}
}

HAE_UNSIGNED_FIXED HAESound::GetRate(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    return GM_GetSamplePitch(mSoundVoiceReference);
	}
    return 0L;
}

void HAESound::SetVolume(HAE_UNSIGNED_FIXED volume)
{
    mSoundVolume = volume;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_ChangeSampleVolume(mSoundVoiceReference, FIXED_TO_SHORT_ROUNDED(volume * MAX_NOTE_VOLUME));
	}
}

HAE_UNSIGNED_FIXED HAESound::GetVolume(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    mSoundVolume = UNSIGNED_RATIO_TO_FIXED(GM_GetSampleVolume(mSoundVoiceReference), MAX_NOTE_VOLUME);
	}
    return mSoundVolume;
}

void HAESound::Fade(HAE_BOOL doAsync)
{
    short int	sampleVolume;

    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    sampleVolume = GM_GetSampleVolume(mSoundVoiceReference);
	    if (doAsync == FALSE)
		{
		    // We're going to fade the sample out and don't stop it		

		    GM_SetSampleFadeRate(mSoundVoiceReference, FLOAT_TO_FIXED(2.2),
					 0, sampleVolume, FALSE);
		    while (GM_GetSampleVolume(mSoundVoiceReference) && (GM_IsSoundDone(mSoundVoiceReference) == FALSE)) 
			{
			    GetMixer()->ServiceAudioOutputToFile();
			    GetMixer()->ServiceIdle();
			    XWaitMicroseocnds(1000);
			}
		}
	    else
		{
		    GM_SetSampleFadeRate(mSoundVoiceReference, FLOAT_TO_FIXED(2.2),
					 0, sampleVolume, FALSE);
		}
	}
}


void HAESound::FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync)
{
    short int	soundVolume, saveVolume, newSoundVolume, saveNewSoundVolume;
    XFIXED		delta;

    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    newSoundVolume = FIXED_TO_SHORT_ROUNDED(destVolume * MAX_NOTE_VOLUME);
	    saveNewSoundVolume = newSoundVolume;
	    // We're going to fade the Sound out and don't stop it		
	    soundVolume = GM_GetSampleVolume(mSoundVoiceReference);
	    saveVolume = soundVolume;

	    if (newSoundVolume < soundVolume)
		{	// fade out
		    soundVolume = newSoundVolume;
		    newSoundVolume = saveVolume;
		    delta = FLOAT_TO_UNSIGNED_FIXED(2.2);
		}
	    else
		{	// fade in
		    delta = (XFIXED)FLOAT_TO_FIXED(-2.2);
		}
	    if (doAsync == FALSE)
		{
		    GM_SetSampleFadeRate(mSoundVoiceReference, delta, soundVolume, newSoundVolume, FALSE);
		    while (GM_GetSampleVolume(mSoundVoiceReference) != saveNewSoundVolume)
			{
			    GetMixer()->ServiceAudioOutputToFile();
			    GetMixer()->ServiceIdle();
			    XWaitMicroseocnds(1000);
			}
		}
	    else
		{
		    GM_SetSampleFadeRate(mSoundVoiceReference, delta, soundVolume, newSoundVolume, FALSE);
		}
	}
}

void HAESound::SetStereoPosition(short int stereoPosition)
{
    mStereoPosition = stereoPosition;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_ChangeSampleStereoPosition(mSoundVoiceReference, stereoPosition);
	}
}

short int HAESound::GetStereoPosition(void)
{
    mStereoPosition;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    mStereoPosition = GM_GetSampleStereoPosition(mSoundVoiceReference);
	}
    return mStereoPosition;
}

void HAESound::SetReverb(HAE_BOOL useReverb)
{
    mReverbState = useReverb;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_ChangeSampleReverb(mSoundVoiceReference, useReverb);
	}
}

HAE_BOOL HAESound::GetReverb(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    mReverbState = GM_GetSampleReverb(mSoundVoiceReference);
	}
    return mReverbState;
}

void HAESound::SetReverbAmount(short int reverbAmount)
{
    mReverbAmount = reverbAmount;
    SetReverb((reverbAmount) ? TRUE : FALSE);
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_SetSampleReverbAmount(mSoundVoiceReference, reverbAmount);
	}
}

short int HAESound::GetReverbAmount(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    mReverbAmount = GM_GetSampleReverbAmount(mSoundVoiceReference);
	}
    return mReverbAmount;
}

void HAESound::SetLowPassAmountFilter(short int lowpassamount)
{
    mLowPassAmount = lowpassamount;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_SetSampleLowPassAmountFilter(mSoundVoiceReference, lowpassamount);
	}
}

short int HAESound::GetLowPassAmountFilter(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    mLowPassAmount = GM_GetSampleLowPassAmountFilter(mSoundVoiceReference);
	}
    return mLowPassAmount;
}

void HAESound::SetResonanceAmountFilter(short int resonance)
{
    mResonanceAmount = resonance;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_SetSampleResonanceFilter(mSoundVoiceReference, resonance);
	}
}

short int HAESound::GetResonanceAmountFilter(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    mResonanceAmount = GM_GetSampleResonanceFilter(mSoundVoiceReference);
	}
    return mResonanceAmount;
}

void HAESound::SetFrequencyAmountFilter(short int frequency)
{
    mFrequencyAmount = frequency;
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    GM_SetSampleFrequencyFilter(mSoundVoiceReference, frequency);
	}
}

short int HAESound::GetFrequencyAmountFilter(void)
{
    if (mSoundVoiceReference != DEAD_VOICE)
	{
	    mFrequencyAmount = GM_GetSampleFrequencyFilter(mSoundVoiceReference);
	}
    return mFrequencyAmount;
}

#if 0
#pragma mark ### HAEMod class ###
#endif
#if USE_MOD_API
HAEMod::HAEMod(HAEAudioMixer *pHAEAudio, 
	       char const *cName, void * userReference) :
    HAEAudioNoise(pHAEAudio, cName)
{
    userReference = userReference;
    pauseVariable = 0;
    pSoundVariables = NULL;
}

HAEMod::~HAEMod()
{
    Stop();

    if (pSoundVariables)
	{
	    GM_FreeModFile((GM_ModData *)pSoundVariables);
	    pSoundVariables = NULL;
	}
}

// currently paused
HAE_BOOL HAEMod::IsPaused(void)
{
    return (pauseVariable) ? (HAE_BOOL)TRUE : (HAE_BOOL)FALSE;
}

void HAEMod::Pause(void)
{
    if (pauseVariable == 0)
	{
	    pauseVariable = 1;
	    GM_PauseMod((GM_ModData *)pSoundVariables);
	}
}

void HAEMod::Resume(void)
{
    if (pauseVariable)
	{
	    pauseVariable = 0;
	    GM_ResumeMod((GM_ModData *)pSoundVariables);
	}
}

// Load file into sound object. This will copy the file directly into memory. It
// will get disposed once you destroy this object.
HAEErr HAEMod::LoadFromFile(HAEPathName pModFilePath)
{
    XFILENAME		theFile;
    OPErr			theErr;
    long			fileSize;
    XPTR			pFileVariables;

    theErr = NO_ERR;
    XConvertNativeFileToXFILENAME(pModFilePath, &theFile);
    pFileVariables = PV_GetFileAsData(&theFile, &fileSize);
    if (pFileVariables)
	{
	    pSoundVariables = GM_LoadModFile(pFileVariables, fileSize);
	    if (pSoundVariables == NULL)
		{
		    theErr = BAD_FILE;
		}
	    XDisposePtr(pFileVariables);	// throw away file, we've parsed the mod file
	}
    else
	{
	    theErr = MEMORY_ERR;
	}
    return HAE_TranslateOPErr(theErr);
}

// Load memory mapped MOD pointer into HAEMod object. This will parse the MOD file and get
// it ready for playing. You can dispose of the data passed once this method returns
HAEErr HAEMod::LoadFromMemory(void const* pModData, unsigned long modSize)
{
    OPErr			theErr;

    theErr = PARAM_ERR;
    if (pModData && modSize)
	{
	    pSoundVariables = GM_LoadModFile((void *)pModData, modSize);
	    if (pSoundVariables == NULL)
		{
		    theErr = BAD_FILE;
		}
	    else
		{
		    theErr = NO_ERR;
		}
	}
    return HAE_TranslateOPErr(theErr);
}

static void PV_ModDoneCallback(GM_ModData *pMod)
{
    HAEDoneCallbackPtr pDoneProc;

    pDoneProc = (HAEDoneCallbackPtr)pMod->reference2;
    if (pDoneProc)
	{
	    (*pDoneProc)((void *)pMod->reference);
	}		
}

HAEErr HAEMod::Start(HAEDoneCallbackPtr pDoneProc)
{
    OPErr	theErr;

    theErr = BAD_FILE;
    if (pSoundVariables)
	{
	    if (IsPlaying())
		{
		    Stop();
		}
	    ((GM_ModData *)pSoundVariables)->reference2 = (long)pDoneProc;
	    GM_BeginModFile((GM_ModData *)pSoundVariables, 
			    (GM_ModDoneCallbackPtr)PV_ModDoneCallback, (long)GetReference());
	    theErr = NO_ERR;
	}
    return HAE_TranslateOPErr(theErr);
}

void HAEMod::Stop(HAE_BOOL startFade)
{
    short int	songVolume;

    if (IsPaused())
	{
	    Resume();
	}
    if (pSoundVariables)
	{
	    if (startFade)
		{
		    songVolume = GM_GetModVolume((GM_ModData *)pSoundVariables);
		    GM_SetModFadeRate((GM_ModData *)pSoundVariables, FLOAT_TO_FIXED(2.2),
				      0, songVolume, TRUE);
		}
	    else
		{
		    GM_StopModFile((GM_ModData *)pSoundVariables);
		}
	}
}


void HAEMod::Fade(HAE_BOOL doAsync)
{
    short int	songVolume;

    if (pSoundVariables)
	{
	    songVolume = GM_GetModVolume((GM_ModData *)pSoundVariables);
	    if (doAsync == FALSE)
		{
		    // We're going to fade the song out and don't stop it
		    GM_SetModFadeRate((GM_ModData *)pSoundVariables, FLOAT_TO_FIXED(2.2),
				      0, songVolume, FALSE);
		    while (	GM_GetModVolume((GM_ModData *)pSoundVariables) && 
				GM_IsModPlaying((GM_ModData *)pSoundVariables)) 
			{
			    GetMixer()->ServiceAudioOutputToFile();
			    GetMixer()->ServiceIdle();
			    XWaitMicroseocnds(1000);
			}
		}
	    else
		{
		    GM_SetModFadeRate((GM_ModData *)pSoundVariables, FLOAT_TO_FIXED(2.2),
				      0, songVolume, FALSE);
		}
	}
}

// FadeTo a volume level
void HAEMod::FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync)
{
    short int	songVolume, saveVolume, newSongVolume, saveNewSongVolume;
    XFIXED		delta;

    if (pSoundVariables)
	{
	    newSongVolume = FIXED_TO_SHORT_ROUNDED(destVolume * MAX_SONG_VOLUME);
	    saveNewSongVolume = newSongVolume;
	    // We're going to fade the song out before we stop it.		
	    songVolume = GM_GetModVolume((GM_ModData *)pSoundVariables);
	    saveVolume = songVolume;

	    if (newSongVolume < songVolume)
		{	// fade out
		    songVolume = newSongVolume;
		    newSongVolume = saveVolume;
		    delta = FLOAT_TO_UNSIGNED_FIXED(2.2);
		}
	    else
		{	// fade in
		    delta = (XFIXED)FLOAT_TO_FIXED(-2.2);
		}
	    if (doAsync == FALSE)
		{
		    GM_SetModFadeRate((GM_ModData *)pSoundVariables, delta,
				      songVolume, newSongVolume, FALSE);
		    while (GM_GetModVolume((GM_ModData *)pSoundVariables) != saveNewSongVolume)
			{
			    GetMixer()->ServiceAudioOutputToFile();
			    GetMixer()->ServiceIdle();
			    XWaitMicroseocnds(1000);
			}
		}
	    else
		{
		    GM_SetModFadeRate((GM_ModData *)pSoundVariables, delta,
				      songVolume, newSongVolume, FALSE);
		}
	}
}

// Set a call back when song is done
void HAEMod::SetDoneCallback(HAEDoneCallbackPtr pDoneProc, void * pReference)
{
    if (pSoundVariables)
	{
	    ((GM_ModData *)pSoundVariables)->reference = (long)pReference;
	    ((GM_ModData *)pSoundVariables)->reference2 = (long)pDoneProc;
	}
}


// set song master tempo. (1.0 uses songs encoded tempo, 2.0 will play
// song twice as fast, and 0.5 will play song half as fast
void HAEMod::SetMasterTempo(HAE_UNSIGNED_FIXED tempoFactor)
{
    if (pSoundVariables)
	{
	    GM_SetModTempoFactor((GM_ModData *)pSoundVariables, tempoFactor);
	}
}


HAE_BOOL HAEMod::IsPlaying(void)
{
    HAE_BOOL	play;

    play = FALSE;
    if (pSoundVariables)
	{
	    play = GM_IsModPlaying((GM_ModData *)pSoundVariables);
	}
    return play;
}

HAE_BOOL HAEMod::IsDone(void)
{
    if (IsPlaying() == FALSE)
	{
	    return TRUE;
	}
    return FALSE;
}

void HAEMod::SetVolume(HAE_UNSIGNED_FIXED volume)
{
    if (pSoundVariables)
	{
	    GM_SetModVolume((GM_ModData *)pSoundVariables,
			    (short int)(UNSIGNED_FIXED_TO_LONG_ROUNDED(volume * MAX_SONG_VOLUME)));
	}
}

HAE_UNSIGNED_FIXED HAEMod::GetVolume(void)
{
    HAE_UNSIGNED_FIXED	value;

    value = 0;
    if (pSoundVariables)
	{
	    value = UNSIGNED_RATIO_TO_FIXED(GM_GetModVolume((GM_ModData *)pSoundVariables),
					    MAX_SONG_VOLUME);
	}
    return value;
}

// sets tempo in beats per minute
void HAEMod::SetTempoInBeatsPerMinute(unsigned long newTempoBPM)
{
    if (pSoundVariables)
	{
	    GM_SetModTempoBPM((GM_ModData *)pSoundVariables, newTempoBPM);
	}
}

// returns tempo in beats per minute
unsigned long HAEMod::GetTempoInBeatsPerMinute(void)
{
    unsigned long tempo;
	
    tempo = 0;
    if (pSoundVariables)
	{
	    tempo = (unsigned long)GM_GetModTempoBPM((GM_ModData *)pSoundVariables);
	}
    return tempo;
}

// pass TRUE to entire loop song, FALSE to not loop
void HAEMod::SetLoopFlag(HAE_BOOL loop)
{
    if (pSoundVariables)
	{
	    GM_SetModLoop((GM_ModData *)pSoundVariables, loop);
	}
}

HAE_BOOL HAEMod::GetLoopFlag(void)
{
    if (pSoundVariables)
	{
	    return (HAE_BOOL)GM_GetModLoop((GM_ModData *)pSoundVariables);
	}
    return FALSE;
}

unsigned long HAEMod::GetInfoSize(HAEInfoTypes infoType)
{
    unsigned long	size;

    size = 0;

    if (pSoundVariables)
	{
	    switch (infoType)
		{
		case TITLE_INFO:
		    size = GM_GetModSongNameLength((GM_ModData *)pSoundVariables);
		    break;
		case COMPOSER_NOTES_INFO:
		    size = GM_GetModSongCommentsLength((GM_ModData *)pSoundVariables);
		    break;
		}
	}
    return size;
}

HAEErr HAEMod::GetInfo(HAEInfoTypes infoType, char *cInfo)
{
    if (pSoundVariables && cInfo)
	{
	    cInfo[0] = 0;
	    switch (infoType)
		{
		case TITLE_INFO:
		    GM_GetModSongName((GM_ModData *)pSoundVariables, cInfo);
		    break;
		case COMPOSER_NOTES_INFO:
		    GM_GetModSongComments((GM_ModData *)pSoundVariables, cInfo);
		    break;
		}
	}
    return HAE_NO_ERROR;
}

#endif	// USE_MOD_API


#if 0
#pragma mark ### HAEGroup class ###
#endif

HAEGroup::HAEGroup(HAEAudioMixer *pHAEAudio, 
		   char const *cName, void * userReference) :
    HAEAudioNoise(pHAEAudio, cName)
{
    userReference = userReference;
    m_topStream = NULL;
    m_topSound = NULL;
    linkedPlaybackReference = DEAD_LINKED_VOICE;
}


HAEGroup::~HAEGroup()
{
    HAEAudioNoise	*pTop, *pNext;
    HAESound		*pSound;
    HAESoundStream	*pStream;

    Stop();		// stop this group

    // clear links for HAESound objects
    pTop = (HAEAudioNoise *)m_topSound;
    m_topSound = NULL;
    while (pTop)
	{
	    pSound = (HAESound *)pTop;
	    delete pSound;
	    pNext = pTop->pGroupNext;
	    pTop->pGroupNext = NULL;
	    pTop = pNext;
	}

    // clear links for HAESoundStream objects
    pTop = (HAEAudioNoise *)m_topStream;
    m_topStream = NULL;
    while (pTop)
	{
	    pStream = (HAESoundStream *)pTop;
	    delete pStream;
	    pNext = pTop->pGroupNext;
	    pTop->pGroupNext = NULL;
	    pTop = pNext;
	}
}

// Associate an HAE object to this group
HAEErr HAEGroup::AddSound(HAESound *pSound)
{
    HAESound	*pTop, *pNext;

    // add to link
    pTop = (HAESound *)m_topSound;
    if (pTop)
	{
	    pNext = NULL;
	    while (pTop)
		{
		    pNext = pTop;
		    pTop = (HAESound *)pTop->pGroupNext;
		}
	    if (pNext)
		{
		    pNext->pGroupNext = pSound;
		}
	}
    else
	{
	    m_topSound = pSound;
	}
    return HAE_NO_ERROR;
}

HAEErr HAEGroup::AddStream(HAESoundStream *pStream)
{
    HAESoundStream			*pTop, *pNext;
    HAEErr					err;

    err = HAE_NO_ERROR;
    // add to link
    pTop = (HAESoundStream *)m_topStream;
    if (pTop)
	{
	    if (pStream->mSoundStreamVoiceReference)
		{
		    // add link into our local list
		    pNext = NULL;
		    while (pTop)
			{
			    pNext = pTop;
			    pTop = (HAESoundStream *)pTop->pGroupNext;
			}
		    if (pNext)
			{
			    pNext->pGroupNext = pStream;
			}
		}
	    else
		{
		    err = HAE_NOT_SETUP;
		}
	}
    else
	{
	    m_topStream = pStream;
	}

    return err;
}

// Disassociate an HAE object from this group
HAEErr HAEGroup::RemoveSound(HAESound *pSound)
{
    HAESound	*pTop;

    // remove link
    pTop = (HAESound *)m_topSound;
    if (pTop != pSound)
	{
	    while (pTop)
		{
		    if (pTop->pGroupNext == pSound)
			{
			    pTop->pGroupNext = pSound->pGroupNext;
			    break;
			}

		    pTop = (HAESound *)pTop->pGroupNext;
		}
	}
    else
	{
	    m_topSound = NULL;
	}
    return HAE_NO_ERROR;
}

HAEErr HAEGroup::RemoveStream(HAESoundStream *pStream)
{
    HAESoundStream	*pTop;

    // remove link
    pTop = (HAESoundStream *)m_topStream;
    if (pTop != pStream)
	{
	    while (pTop)
		{
		    if (pTop->pGroupNext == pStream)
			{
			    pTop->pGroupNext = pStream->pGroupNext;
			    break;
			}

		    pTop = (HAESoundStream *)pTop->pGroupNext;
		}
	}
    else
	{
	    m_topStream = NULL;
	}
    return HAE_NO_ERROR;
}

HAEErr HAEGroup::Start(void)
{
    HAEAudioNoise			*pNext;
    HAESound				*pSound;
#if USE_STREAM_API == TRUE
    HAESoundStream			*pStream;
    LINKED_STREAM_REFERENCE	link, top;
#endif

    // process sounds
    pNext = (HAEAudioNoise *)m_topSound;
    while (pNext)
	{
	    pSound = (HAESound *)pNext;
	    pSound->__Start();
	    pNext = pNext->pGroupNext;
	}

#if USE_STREAM_API == TRUE
    // process streams
    pNext = (HAEAudioNoise *)m_topStream;
    top = NULL;
    while (pNext)
	{
	    pStream = (HAESoundStream *)pNext;
	    pStream->Preroll();

	    link = GM_NewLinkedStreamList((STREAM_REFERENCE)pStream->mSoundStreamVoiceReference, NULL);
	    top = GM_AddLinkedStream(top, link);
	    pNext = pNext->pGroupNext;
	}
    if (top)
	{
	    GM_StartLinkedStreams(top);
	    GM_FreeLinkedStreamList(top);
	}
#endif

    return HAE_NO_ERROR;
}

void HAEGroup::Stop(HAE_BOOL startFade)
{
    HAEAudioNoise	*pNext;
    HAESound		*pSound;
#if USE_STREAM_API == TRUE
    HAESoundStream	*pStream;
#endif
    // process sounds
    pNext = (HAEAudioNoise *)m_topSound;
    while (pNext)
	{
	    pSound = (HAESound *)pNext;
	    pSound->Stop(startFade);
	    pNext = pNext->pGroupNext;
	}

#if USE_STREAM_API == TRUE
    // process streams
    pNext = (HAEAudioNoise *)m_topStream;
    while (pNext)
	{
	    pStream = (HAESoundStream *)pNext;
	    pStream->Stop(startFade);
	    pNext = pNext->pGroupNext;
	}
#endif
}

// Set the stereo position an entire group (-63 left to 63 right, 0 is middle)
void HAEGroup::SetStereoPosition(short int stereoPosition)
{
    HAEAudioNoise	*pNext;
#if USE_STREAM_API == TRUE
    HAESoundStream	*pStream;
#endif
    HAESound		*pSound;

    // process sounds
    pNext = (HAEAudioNoise *)m_topSound;
    while (pNext)
	{
	    pSound = (HAESound *)pNext;
	    pSound->SetStereoPosition(stereoPosition);
	    pNext = pNext->pGroupNext;
	}

#if USE_STREAM_API == TRUE
    // process streams
    pNext = (HAEAudioNoise *)m_topStream;
    while (pNext)
	{
	    pStream = (HAESoundStream *)pNext;
	    pStream->SetStereoPosition(stereoPosition);
	    pNext = pNext->pGroupNext;
	}
#endif
}

// Set the volume level of an entire group
void HAEGroup::SetVolume(HAE_UNSIGNED_FIXED newVolume)
{
    HAEAudioNoise	*pNext;
#if USE_STREAM_API == TRUE
    HAESoundStream	*pStream;
#endif
    HAESound		*pSound;

    // process sounds
    pNext = (HAEAudioNoise *)m_topSound;
    while (pNext)
	{
	    pSound = (HAESound *)pNext;
	    pSound->SetVolume(newVolume);
	    pNext = pNext->pGroupNext;
	}

#if USE_STREAM_API == TRUE
    // process streams
    pNext = (HAEAudioNoise *)m_topStream;
    while (pNext)
	{
	    pStream = (HAESoundStream *)pNext;
	    pStream->SetVolume(newVolume);
	    pNext = pNext->pGroupNext;
	}
#endif
}

// Enable/Disable reverb of an entire group
void HAEGroup::SetReverb(HAE_BOOL useReverb)
{
    HAEAudioNoise	*pNext;
#if USE_STREAM_API == TRUE
    HAESoundStream	*pStream;
#endif
    HAESound		*pSound;

    // process sounds
    pNext = (HAEAudioNoise *)m_topSound;
    while (pNext)
	{
	    pSound = (HAESound *)pNext;
	    pSound->SetReverb(useReverb);
	    pNext = pNext->pGroupNext;
	}

#if USE_STREAM_API == TRUE
    // process streams
    pNext = (HAEAudioNoise *)m_topStream;
    while (pNext)
	{
	    pStream = (HAESoundStream *)pNext;
	    pStream->SetReverb(useReverb);
	    pNext = pNext->pGroupNext;
	}
#endif
}

// set reverb mix amount of an entire group
void HAEGroup::SetReverbAmount(short int reverbAmount)
{
    HAEAudioNoise	*pNext;
#if USE_STREAM_API == TRUE
    HAESoundStream	*pStream;
#endif
    HAESound		*pSound;

    // process sounds
    pNext = (HAEAudioNoise *)m_topSound;
    while (pNext)
	{
	    pSound = (HAESound *)pNext;
	    pSound->SetReverbAmount(reverbAmount);
	    pNext = pNext->pGroupNext;
	}

#if USE_STREAM_API == TRUE
    // process streams
    pNext = (HAEAudioNoise *)m_topStream;
    while (pNext)
	{
	    pStream = (HAESoundStream *)pNext;
	    pStream->SetReverbAmount(reverbAmount);
	    pNext = pNext->pGroupNext;
	}
#endif
}

// EOF of HAE.cpp



