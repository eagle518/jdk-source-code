/*
 * @(#)GenSnd.h	1.60 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenSnd.h"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Overview
**	The purpose of this layer of code is to remove Macintosh Specific code.
**	No file, or memory access. All functions are passed pointers to data
**	that needs to be passed into the mixer, and MIDI sequencer
**
** Modification History
**
**	4/6/93		Created
**	4/12/93		First draft ready
**	4/14/93		Added Waveform structure
**	7/7/95		Added Instrument API
**	11/7/95		Major changes, revised just about everything.
**	11/11/95	Added external queued midi links
**	11/20/95	Removed the BF_ flags, now you must walk through the union structure
**				Remove bit fields. BIT FIELDS DON'T WORK WITH MPW!!!!
**	12/5/95		Added avoidReverb into instrument field; removed drop sample case
**	12/6/95		Move REVERB_TYPE from GENPRIV.H
**				Added GM_SetReverbType; removed extern references
**				Added ReverbType to GM_Song structure
**				Removed defaultPlaybackRate & defaultInterpolationMode from the GM_Song
**				structure
**	12/7/95		Moved DEFAULT_REVERB_TYPE from GenSnd.c
**				Added GM_GetReverbType
**	1/4/96		Added GM_ChangeSampleReverb for sound effects
**	1/7/96		Changed GM_BeginDoubleBuffer to use a 32 bit value for volume
**				Added GM_SetEffectsVolume & GM_GetEffectsVolume
**	1/13/96		Added extendedFormat bit to internal instrument format
**	1/18/96		Spruced up for C++ extra error checking
**	1/19/96		Changed GM_BeginSample to support bitsize and channels
**	1/29/96		Changed WaveformInfo to support XFIXED for sample rate
**				Added useSampleRate factor for playback of instruments & sampleAndHold bits
**	2/4/96		Added songMidiTickLength to GM_Song
**	2/5/96		Moved lots of variables from the GM_Mixer structure into
**				the GM_Song structure.
**				Changed GM_EndSong & GM_SongTicks & GM_IsSongDone &
**				GM_SetMasterSongTempo to pass in a GM_Song pointer
**				Added GM_GetSongTickLength
**	2/12/96		Added GM_SetSongTickPosition
**				Added songMicrosecondLength to GM_Song structure
**	2/14/96		Added GM_StartLiveSong
**	2/18/96		Added panPlacement to the GM_Instrument structure
**	2/29/96		Added trackMuted to GM_Song structure
**	3/5/96		Added MAX_SONG_VOLUME
**	4/15/96		Added support to interpret SONG resource via GM_MergeExternalSong
**	4/20/96		Added defines for max lfos, and max curves, MAX_LFOS & MAX_CURVES
**	4/21/96		Added GM_GetRealtimeAudioInformation
**	6/7/96		Increased MAX_TRACKS to 65 to include tempo track and 64 tracks
**	6/9/96		Added GM_GetUserReference & GM_SetUserReference
**	6/30/96		Changed font and retabbed
**	7/2/96		Added packing pragmas
**				Removed usage of Machine.h. Now merged into X_API.h
**	7/3/96		Added support for one level of loop save and playback
**				Required for Headspace support
**	7/5/96		Added GM_KillAllNotes
**	7/6/96		Added GM_GetSyncTimeStamp
**	7/14/96		Fixed structure alignment issue for PowerPC
**	7/23/96		Changed QGM_ functions to use an unsigned long for time stamp
**	7/25/96		Added GM_GetSyncTimeStampQuantizedAhead
**	8/15/96		Added constant for LOW_PASS_AMOUNT & LPF_DEPTH
**	8/19/96		Added GM_SetAudioTask
**	9/10/96		Added GM_NoteOn & GM_NoteOff & GM_ProgramChange & GM_PitchBend &
**				GM_Controller & GM_AllNotesOff for direct control to bypass the queue
**	9/17/96		Added GM_LoadSongInstrument & GM_UnloadSongInstrument
**				Added Q_GET_TICK
**	9/18/96		Changed GM_SongCallbackProcPtr to pass a GM_Song structure
**				rather than an ID
**	9/19/96		Added GM_GetSongTempo
**	9/20/96		Added GM_SetSongTempo & GM_ResumeSong & GM_PauseSong
**	9/23/96		Added GM_MuteTrack & GM_UnmuteTrack & GM_MuteChannel & GM_UnmuteChannel
**	9/24/96		Added GM_SetSongTempInBeatsPerMinute & GM_GetSongTempoInBeatsPerMinute
**				Added GM_SoloTrack & GM_SoloChannel
**				Added GM_GetSongPitchOffset & GM_SetSongPitchOffset
**	9/25/96		Added GM_GetChannelMuteStatus & GM_GetTrackMuteStatus
**				Added controller 90 to change the global reverb type
**				Added GM_EndSongNotes
**	10/8/96		Removed pascal from GM_SongCallbackProcPtr
**	10/11/96	Added GM_BeginSampleFromInfo
**	10/13/96	Changed QGM_AllNotesOff to work with the queue and post an event
**				Added GM_ReadIntoMemoryWaveFile & GM_ReadIntoMemoryAIFFFile
**	10/18/96	Made WaveformInfo smaller
**	10/23/96	Removed reference to BYTE and changed them all to UBYTE or SBYTE
**				Added defines for MOD_WHEEL_CONTROL = 'MODW & SAMPLE_NUMBER = 'SAMP'
**				Added more defines for instrument types
**	10/28/96	Modified QGM_NoteOn & QGM_NoteOff & QGM_ProgramChange &
**				QGM_PitchBend & QGM_Controller & QGM_AllNotesOff to accept a GM_Song *
**	10/31/96	Changed trackMuted and channelMuted to be bit flag based
**				Added soloMuted bit array for solo control
**	10/31/96	Added GM_IsInstrumentLoaded
**	11/3/96		Added midiNote to GM_AudioInfo structure
**	11/5/96		Changed WaveformInfo to GM_Waveform
**				Added GM_ReadFileInformation and GM_FreeWaveform
**	11/6/96		Added GM_UnsoloTrack, GM_GetTrackSoloStatus
**	11/7/96		Added GM_SongTimeCallbackProcPtr & GM_SetSongTimeCallback
**	11/8/96		Added GM_GetSampleVolume & GM_GetSamplePitch & GM_GetSampleStereoPosition
**	11/9/96		Added GM_KillSongNotes
**	11/11/96	Added more error codes and removed extra ones
**	11/14/96	Added pSong reference in GM_GetRealtimeAudioInformation
**	11/19/96	Changed MAX_CHANNELS to 17, 16 for Midi, 1 for sfx
**				Added a GM_Song structure to GM_ConvertPatchBank and removed bank
**	11/21/96	Removed GM_ConvertPatchBank
**	11/26/96	Changed MAX_BANKS to 6
**				Added GM_GetControllerValue
**	12/2/96		Added MOD file code API
**	12/9/96		Added GM_LoadInstrumentFromExternal
**	12/15/96	Added controls for DEFAULT_VELOCITY_CURVE
**	12/19/96	Added Sparc pragmas
**	1/2/97		Moved USE_MOD_API and USE_STREAM_API into X_API.h
**	1/12/97		Changed maxNormalizedVoices to mixLevel
**	1/16/97		Changed LFORecord to LFORecords
**	1/22/97		Added GM_SetSampleDoneCallback
**	1/23/97		Added M_STEREO_FILTER
**				Added GM_SetAudioOutput
**	1/24/97		Added GM_SetSongFadeRate
**				Added GM_SetModLoop & GM_GetModTempoBPM & GM_SetModTempoBPM
**				Changed disposeMidiDataWhenDone to disposeSongDataWhenDone
**				Added GM_SetAudioStreamFadeRate
**	1/28/97		Added more parmeters to GM_SetSongFadeRate to support async
**				ending of song
**	1/28/97		Eliminated terminateDecay flag. Not used anymore
**	1/30/97		Changed SYMPHONY_SIZE to MAX_VOICES
**	2/1/97		Added support for pitch offset control on a per channel basis
**				Added GM_DoesChannelAllowPitchOffset & GM_AllowChannelPitchOffset
**	2/2/97		Tightened up GM_Song and GM_Instrument a bit
**	2/13/97		Added GM_GetSystemVoices
**	2/20/97		Added support for TYPE 7 and 8 reverb in GM_SetReverbType
**	3/12/97		Added a REVERB_NO_CHANGE reverb mode. This means don't change the
**				current mixer state
**	3/20/97		Added GM_SetSongMicrosecondPosition & GM_GetSongMicrosecondLength
**	3/27/97		Added channelChorus
**	3/27/97		Changed all 4 character constants to use the FOUR_CHAR macro
**	4/14/97		Changed KeymapSplit to GM_KeymapSplit
**	4/20/97		Added GM_SetSongMetaEventCallback
**	4/21/97		Removed enableInterpolate from GM_Instrument structure
**	5/1/97		Added startOffsetFrame to GM_BeginSampleFromInfo
**	5/19/97		Added GM_ReadAndDecodeFileStream
**	6/10/97		Added GM_SetModReverbStatus to allow for verb enabled Mod files
**	6/16/97		Added GM_AudioStreamSetVolumeAll
**	6/25/97		Added GM_SetSampleLoopPoints
**	7/8/97		Added or changed GM_GetSongLoopFlag & GM_SetSongLoopMax &
**				GM_SetSongLoopFlag & GM_GetSongLoopMax
**	7/15/97		Added GM_GetAudioOutput & GM_GetAudioTask & GM_GetAudioBufferOutputSize
**	7/17/97		Aligned GM_Song and GM_Instrument structures to 8 bytes
**	7/21/97		Put mins and maxs on GM_SetSongTempInBeatsPerMinute. 0 and 500
**				Changed GM_SetSongTempInBeatsPerMinute & GM_GetSongTempoInBeatsPerMinute
**				GM_SetSongTempo & GM_GetSongTempo & GM_SetMasterSongTempo to all be unsigned longs
**				Changed key calculations to floating point to test for persicion loss. Use
**				USE_FLOAT in GenSnd.h to control this change.
**	7/22/97		Removed MicroJif from the GM_Mixer structure. Now using constant BUFFER_SLICE_TIME
**				Added GM_GetSampleVolumeUnscaled
**	7/28/97		Moved GM_FreeWaveform so it is compiled in all platform cases.
**				Removed define wrapper from errors codes in defining OPErr.
**				Fixed a praga pack problem for Sun
**	8/6/97		Moved USE_FLOAT to X_API.h
**  8/06/97		(ddz) Added PgmEntry structure def. Added midiSize, TSNumerator, TSDenominator,
**				pgmChangeInfo, and trackcumuticks[] fields to GM_Song structure for
**				implementing the Song Program List capability
**				Changed GM_SongMetaCallbackProcPtr and added currenTrack
**	8/13/97		Renamed GM_GetSongProgramChanges to GM_GetSongInstrumentChanges and changed
**				Byte reference to XBYTE
**	8/27/97		Moved GM_StartHardwareSoundManager & GM_StopHardwareSoundManager from
**				GenPriv.h
**	9/15/97		Added GM_GetSampleReverb && GM_AudioStreamGetReverb
**  		    Added PatchInfo structure to combine some of instrument scan variables into one ptr
**              Changed PgmEntry to InstrumentEntry, added bank select fields
**	9/25/97		Added TrackStatus to represent track status rather than 'R' and 'F'. Symbolic use.
**	10/15/97	Added processingSlice to GM_Song and GM_Instrument to handle threading release issues.
**	10/16/97	Changed GM_LoadSong parmeters to include an option to ignore bad instruments
**				when loading.
**				Changed GM_LoadSongInstruments to use a XBOOL for a flag rather than an int
**				Added GM_AnyStereoInstrumentsLoaded
**				Added GM_CacheSamples
**	11/12/97	Added GM_MaxDevices & GM_SetDeviceID & GM_GetDeviceID & GM_GetDeviceName
**	11/17/97	Added GM_AudioStreamGetSamplesPlayed & GM_AudioStreamUpdateSamplesPlayed from Kara
**	11/18/97	Added GM_AudioStreamResumeAll & GM_AudioStreamPauseAll
**	12/16/97	Modified GM_GetDeviceID and GM_SetDeviceID to pass a device parameter pointer
**				that is specific for that device.
**	12/16/97	Moe: removed compiler warnings
**	1/14/98		kk: added numLoops field to GM_Waveform struct.
**				added theLoopTarget to GM_BeginSample
**	1/21/98		Added GM_SetChannelVolume & GM_GetChannelVolume
**	1/29/98		Added new defer parameter to GM_AudioStreamSetVolume
**	2/2/98		Added GM_SetVelocityCurveType
**	2/3/98		Added GM_SetupReverb & GM_CleanupReverb
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/11/98		Added Q_48K, Q_24K, Q_8K and GM_ConvertFromOutputQualityToRate
**	2/15/98		Removed songMicrosecondIncrement from the GM_Song structure. Not used
**	2/23/98		Removed GM_InitReverbTaps & GM_GetReverbTaps & GM_SetReverbTaps
**	3/4/98		Added constant MAX_SAMPLE_FRAMES that represents the mixer limit
**	3/12/98		Modified GM_BeginDoubleBuffer to include a sample done callback
**	3/16/98		Added GM_ProcessReverb && GM_GetReverbEnableThreshold
**	3/23/98		Added MAX_SAMPLE_RATE and moved some fields around in GM_Instrument
**				for alignment
**				Added MIN_SAMPLE_RATE
**	3/23/98		MOE: Changed MAX_SAMPLE_RATE and MIN_SAMPLE_RATE to be unsigned
**	3/24/98		Changed a code wrapper USE_STREAM_API to USE_HIGHLEVEL_FILE_API
**	5/4/98		Eliminated neverInterpolate & enablePitchRandomness from the
**				GM_Instrument structure. Its not used. Also got rid of
**				enablePitchRandomness & disableClickRemoval in the GM_Song structure.
**	5/5/98		Changed GM_ReadAndDecodeFileStream to return an OPErr
**	5/7/98		Changed GM_ReadFileInformation & GM_ReadFileIntoMemory & GM_ReadFileIntoMemoryFromMemory
**				to set an error code if failure
**				Added GM_SetChannelReverb & GM_GetChannelReverb
**	5/15/98		Added trackStatusSave to the GM_Song structure will helps when doing loopstart/loopend
**	6/19/98		Added message STREAM_HAVE_DATA to GM_StreamMessage
**	6/26/98		Added GM_IsReverbFixed
**	6/30/98		Changed songID from INT16 to INT32 in GM_Song structure
**	7/1/98		Changed various API to use the new XResourceType and XLongResourceID or XShortResourceID
**	7/6/98		Added GM_IsSongInstrumentLoaded
**				Fixed type problems with GM_LoadSong
**	7/28/98		Added songMasterStereoPlacement to GM_Song
**				Added GM_SetSongStereoPosition & GM_GetSetStereoPosition
**
**  JAVASOFT
**	03.17.98:	kk: added UNSUPPORTED_HARDWARE to OPErr
**	??			kk: added GM_AudioStreamDrain()
**	08.17.98:	kk: put USE_STREAM_API back rather than USE_HIGHLEVEL_FILE_API;
**				otherwise cannot compile.
**	8/13/98		Added GM_VoiceType to GM_AudioInfo structure
**				Added GM_GetMixerUsedTime
**	8/14/98		Added GM_GetMixerUsedTimeInPercent
**	9/8/98		Added SAMPLE_TO_LARGE to error codes
**	9/10/98		Added two new fields to GM_Waveform to handle streams and custom
**				object pointers.
**	9/12/98		Added GM_GetSamplePlaybackPointer
**	9/22/98		Added BAD_SAMPLE_RATE
**	9/25/98		Added new parameter to GM_ReadFileInformation to handle block
**				allocation by file types
**	10/27/98	Moved MIN_LOOP_SIZE to GenPriv.h
**	11/6/98		Removed noteDecayPref from the GM_Waveform structure.
**	11/24/98	Added GM_GetSampleReverbAmount & GM_SetSampleReverbAmount
**				Added NOT_READY as a new OPErr
**	11/30/98	Added support for omni mode in GM_Song
**				Added GM_EndSongChannelNotes to support omni mode
**	12/3/98		Added GM_GetStreamReverbAmount & GM_SetStreamReverbAmount
**	12/9/98		Added GM_GetPitchBend
**	12/18/98	Changed GM_BeginSong to include a new parameter for autoLevel
**	1/13/99		Added a dynamic Katmai flag and some voice calculation flags
**	1/14/99		Added GM_LoadInstrumentFromExternalData
**	2/12/99		Renamed USE_HAE_FOR_MPEG to USE_MPEG_DECODER
**	2/18/99		Changed GM_LoadSong & GM_CreateLiveSong to pass in a context
**				Added GM_GetSongContext & GM_SetSongContext
**				Removed GM_GetUserReference & GM_SetUserReference
**	2/24/99		Removed songEndCallbackReference1 & songEndCallbackReference2 from
**				GM_Song structure
**				Removed the extra reference in GM_SetSongMetaEventCallback and the
**				GM_Song structure
**	2/25/99		Increased MAX_SONGS to 16 from 8.
**	3/2/99		Changed all stream references to STREAM_REFERENCE rather than
**				the blank 'long'
**	3/3/99		Added GM_GetSampleStartTimeStamp
**				Renamed GM_BeginSample to GM_SetupSample, GM_BeginDoubleBuffer to GM_SetupSampleDoubleBuffer,
**				GM_BeginSampleFromInfo to GM_SetupSampleFromInfo, and added GM_StartSample
**				Added GM_GetSampleFrequencyFilter GM_SetSampleFrequencyFilter GM_GetSampleResonanceFilter
**				GM_SetSampleResonanceFilter GM_GetSampleLowPassAmountFilter GM_SetSampleLowPassAmount
**	3/5/99		Added GM_SetSyncSampleStartReference & GM_SyncStartSample
**	3/8/99		Renamed GM_EndSoundEffects to GM_EndAllSamples
**
**	JAVASOFT
**	05.04.99	Changed calls to synth to work through a pSynth pointer so that we can direct the
**	06.21.99	calls to an external synth or the Beatnik synth.
**	2002-01-07	$$fb Added GM_ChangeSampleResample and GM_GetSampleResample
**	2002-03-14	$$fb cleaned up pragma code, included Itanium and sparcv9 architecture support
**	2002-04-20	$$fb Added GM_ReleaseSample and GM_ReleaseAllSamples
**	2003-08-21	$$fb support for amd64, cleaner pragma code
*/
/*****************************************************************************/

#ifndef G_SOUND
#define G_SOUND

#ifndef __X_API__
#include "X_API.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

    // $$kk: START CHANGES
    // $$kk: 05.04.99

    // Need a forward reference to the GM_Song struct to keep
    // our compiler from complaining.
    struct GM_Song;

    // $$kk: 07.10.99: need one for GM_Synth and OPErr now too!
    // (bad form!  should clean this up later....)
    struct GM_Synth;

    /* Midi Synth defines */
    /*
      enum
      {
      HAE_SYNTH = 0,			// Headspace software synthesizer
      HARDWARE_SYNTH,			// hardware synth
      };
      typedef long SynthMode
    */



    /* function pointers for synth methods */
    typedef void (*Synth_ProgramChangeProcPtr)(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 program);
    typedef void (*Synth_NoteOffProcPtr)(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume);
    typedef void (*Synth_NoteOnProcPtr)(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume);
    typedef void (*Synth_PitchBendProcPtr)(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, UBYTE valueMSB, UBYTE valueLSB);
    typedef void (*Synth_ProcessControllerProcPtr)(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 controler, UINT16 value);
    typedef void (*Synth_ProcessSongSoundOffProcPtr)(struct GM_Song *pSong);


    /* GM_Synth struct */
    struct GM_Synth
    {
	void *								deviceHandle;
	Synth_ProgramChangeProcPtr			pProgramChangeProcPtr;
	Synth_NoteOffProcPtr				pNoteOffProcPtr;
	Synth_NoteOnProcPtr					pNoteOnProcPtr;
	Synth_PitchBendProcPtr				pPitchBendProcPtr;
	Synth_ProcessControllerProcPtr		pProcessControllerProcPtr;
	Synth_ProcessSongSoundOffProcPtr	pProcessSongSoundOffProcPtr;
	struct GM_Synth						*pNext;
    };
    typedef struct GM_Synth GM_Synth;

    /* this was static in GenSeq.c, but i'm using it for my external synth stuff */
    // test to see if a channel or track is muted
    XBOOL PV_IsMuted(struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack);


    /* these are the methods we need function pointers for */


    // Methods Representing MIDI Messages

    // Process midi program change
    /*static*/ void PV_ProcessProgramChange(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 program);

    // Process note off
    /*static*/ void PV_ProcessNoteOff(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume);

    // Process note on
    /*static*/ void PV_ProcessNoteOn(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume);

    // Process pitch bend
    /*static*/ void PV_ProcessPitchBend(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, UBYTE valueMSB, UBYTE valueLSB);

    // Process midi controlers
    /*static*/ void PV_ProcessController(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 controler, UINT16 value);


    // Methods Representing Synthesizer Functionality

    // End sound for the song.  This is really synthesizer functionality, not a MIDI message.
    //PV_SongSoundOff(struct GM_Song *pSong);

    // Sequencer creation: must init synth
    // GM_Song * GM_CreateLiveSong(void *context, XShortResourceID songID)
    // GM_LoadSong

    // Close synthesizer
    //OPErr GM_FreeSong(void *threadContext, GM_Song *pSong)


    // Get the synthesizer.
    // GM_Synth *PV_GetSongSynth(GM_Song *pSong);

    // Set the synthesizer.
    // void PV_SetSynth(struct GM_Song *pSong, struct GM_Synth *pSynth);


    // $$kk: END CHANGES

    // $$kk: 07.10.99: more changes!

    // add a synth to respond to this song's events
    void GM_AddSongSynth(struct GM_Song *pSong, struct GM_Synth *pSynth);

    // remove a synth from responding to this song's events
    void GM_RemoveSongSynth(struct GM_Song *pSong, struct GM_Synth *pSynth);

    // given a synth, return the next one for the song.
    // i.e. pass NULL to get the first one (if any), and
    // pass in each one returned to get the next.  when you
    // pass in the last one, NULL is returned.
    GM_Synth * GM_GetSongSynth(struct GM_Song *pSong, struct GM_Synth *pSynth);

    // $$kk: 06.21.99: BEGIN CHANGES
    //#define USE_EXTERNAL_SYNTH TRUE

#if USE_EXTERNAL_SYNTH == TRUE

    // Open an external synth for use with this song.
    // Returns the synth if successful, otherwise NULL.
    //GM_Synth * PV_OpenExternalSynth(struct GM_Song *pSong);
    //GM_Synth * PV_OpenExternalSynth(struct GM_Song *pSong, long deviceID);

    // Close an external synth for this song.
    //void PV_CloseExternalSynth(struct GM_Song *pSong, long deviceHandle);

    // Creates a GM_Synth structure based around the specified deviceHandle.
    // The deviceHandle is the handle to an open native device (MIDI OUT / Synthesizer).
    // Returns null for failure.
    GM_Synth * PV_CreateExternalSynthForDevice(struct GM_Song *pSong, void *deviceHandle);


    // Process midi program change
    void PV_ProcessExternalSynthProgramChange(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 program);

    // Process note off
    void PV_ProcessExternalSynthNoteOff(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume);

    // Process note on
    void PV_ProcessExternalSynthNoteOn(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume);

    // Process pitch bend
    void PV_ProcessExternalSynthPitchBend(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, UBYTE valueMSB, UBYTE valueLSB);

    // Process midi controlers
    void PV_ProcessExternalSynthController(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 controler, UINT16 value);

#endif // USE_EXTERNAL_SYNTH

    // $$kk: 06.21.99: END CHANGES


    /* System defines */

    // This is an active voice reference that represents a valid/active voice.
    // Used in various functions that need to return and reference a voice.
    //$$fb itanium port: voice reference should be an integer
    typedef UINT32				VOICE_REFERENCE;	// reference returned that is a allocated active voice
#define DEAD_VOICE			(VOICE_REFERENCE)-1L	// this represents a dead or invalid voice
    typedef void *				LINKED_VOICE_REFERENCE;	// this represents a series of linked VOICE_REFERENCE's
#define DEAD_LINKED_VOICE	(LINKED_VOICE_REFERENCE)0L	// this represents a dead or invalid voice

    /* Used in InitGeneralSound */

    // Quality types
    enum
    {
	Q_8K = 0,			// 8 kHz
	Q_11K_TERP_22K,		// 11 kHz terped to 22 kHz
	Q_11K,				// 11 kHz
	Q_22K,				// 22 kHz
	Q_22K_TERP_44K,		// 22 kHz terped to 44 kHz
	Q_24K,				// 24 kHz
	Q_44K,				// 44 kHz
	Q_48K				// 48 kHz
    };
    typedef INT32 Quality;

    // Modifier types
#define M_NONE				0L
#define M_USE_16			(1<<0L)
#define M_USE_STEREO		(1<<1L)
#define M_DISABLE_REVERB	(1<<2L)
#define M_STEREO_FILTER		(1<<3L)
    typedef INT32 AudioModifiers;

    // Interpolation types
    enum
    {
	E_AMP_SCALED_DROP_SAMPLE = 0,
	E_2_POINT_INTERPOLATION,
	E_LINEAR_INTERPOLATION
    };
    typedef INT32 TerpMode;

    // verb types
    enum
    {
	REVERB_NO_CHANGE = 0,	// Don't change current mixer settings
	REVERB_TYPE_1 = 1,		// None
	REVERB_TYPE_2,			// Igor's Closet
	REVERB_TYPE_3,			// Igor's Garage
	REVERB_TYPE_4,			// Igor's Acoustic Lab
	REVERB_TYPE_5,			// Igor's Dungeon
	REVERB_TYPE_6,			// Igor's Cavern
	REVERB_TYPE_7,			// Small reflections Reverb used for WebTV
	REVERB_TYPE_8,			// Early reflections (variable verb)
	REVERB_TYPE_9,			// Basement (variable verb)
	REVERB_TYPE_10,			// Banquet hall (variable verb)
	REVERB_TYPE_11			// Catacombs (variable verb)
    };
    typedef char ReverbMode;
#define DEFAULT_REVERB_TYPE	REVERB_TYPE_4
#define MAX_REVERB_TYPES	12

    typedef void (*GM_ReverbProc)(ReverbMode which);

    typedef struct
    {
	ReverbMode		type;
	UBYTE			thresholdEnableValue;	// 0 for variable, value to enable fixed
	XBOOL			isFixed;
	UINT32  	globalReverbUsageSize;	// GM_Mixer->reverbBuffer size
	GM_ReverbProc	pMonoRuntimeProc;
	GM_ReverbProc	pStereoRuntimeProc;
    } GM_ReverbConfigure;


    typedef enum
    {
	SCAN_NORMAL = 0,		// normal Midi scan
	SCAN_SAVE_PATCHES,		// save patches during Midi scan. Fast
	SCAN_DETERMINE_LENGTH,	// calculate tick length. Slow
	SCAN_FIND_PATCHES,		// mode for scanning to find the program changes
	SCAN_COUNT_PATCHES      // mode for counting the number of program changes
    } ScanMode;

    typedef enum
    {
	SAFE_TO_ACCESS = 0,		// memory structure is safe to access
	ASK_FOR_ACCESS,			// ask for access
	ACKNOWLEGE_ACCESS		// acknowlege access
    } AccessSemaphore;

    // used in GM_Song->trackon to track progress of a midi file
    enum
    {
	TRACK_OFF = 0,
	TRACK_FREE,
	TRACK_RUNNING
    };
    typedef unsigned char TrackStatus;

    enum
    {
	VELOCITY_CURVE_1 = 0,	// default S curve
	VELOCITY_CURVE_2,		// more peaky S curve
	VELOCITY_CURVE_3,		// inward curve centered around 95 (used for WebTV)
	VELOCITY_CURVE_4,		// two time exponential
	VELOCITY_CURVE_5		// two times linear
    };
    typedef unsigned char VelocityCurveType;
#define DEFAULT_VELOCITY_CURVE	VELOCITY_CURVE_1

#if X_PLATFORM == X_WEBTV
#define MAX_VOICES			36		// max voices at once. 28 for midi, 4 for audio
#else
#define MAX_VOICES			64		// max voices at once
#endif
#define MAX_INSTRUMENTS			128		// MIDI number of programs per patch bank
#define MAX_BANKS				6		// three GM banks; three user banks
#define MAX_TRACKS				65		// max MIDI file tracks to process (64 + tempo track)
#define MAX_CHANNELS			17		// max MIDI channels + one extra for sound effects
#define MAX_CONTROLLERS			128		// max MIDI controllers
#define MAX_SONG_VOLUME			127
#define MAX_NOTE_VOLUME			127		// max note volume
#define MAX_CURVES				4		// max curve entries in instruments
#define MAX_LFOS				6		// max LFO's, make sure to add one extra for MOD wheel support
#define MAX_MASTER_VOLUME		256		// max volume level for master volume level
#define MAX_SAMPLES				768		// max number of samples that can be loaded
#define MAX_SONGS				16		// max number of songs that can play at one time
#define DEFAULT_PITCH_RANGE		2		// number of semi-tones that change with pitch bend wheel
#define PERCUSSION_CHANNEL		9		// which channel (zero based) is the default percussion channel
#define MAX_SAMPLE_FRAMES		1048576	// max number of sample frames that we can play in one voice
    // 1024 * 1024 = 1MB
#define MIN_LOOP_SIZE			20		// min number of loop samples that can be processed

#define MIN_SAMPLE_RATE			((UINT32)1L)		// min sample rate. 1.5258789E-5 kHz
#define MAX_SAMPLE_RATE			rate48khz	// max sample rate	48 kHz
#define MAX_PAN_LEFT			(-63)	// max midi pan to the left
#define MAX_PAN_RIGHT			63		// max midi pan to the right

    /* Common errors returned from the system */
    typedef enum
    {
	NO_ERR = 0,
	PARAM_ERR,
	MEMORY_ERR,
	BAD_SAMPLE,
	BAD_INSTRUMENT,
	BAD_MIDI_DATA,
	ALREADY_PAUSED,
	ALREADY_RESUMED,
	DEVICE_UNAVAILABLE,
	NO_SONG_PLAYING,
	STILL_PLAYING,
	NO_VOLUME,
	TOO_MANY_SONGS_PLAYING,
	BAD_FILE,
	NOT_REENTERANT,
	NOT_SETUP,
	BUFFER_TO_SMALL,
	NO_FREE_VOICES,
	OUT_OF_RANGE,
	INVALID_REFERENCE,
	STREAM_STOP_PLAY,
	BAD_FILE_TYPE,
	GENERAL_BAD,
	SAMPLE_TO_LARGE,
	BAD_SAMPLE_RATE,
	NOT_READY,
	UNSUPPORTED_HARDWARE
    } OPErr;

    // Need a forward reference to the GM_Song struct to keep
    // our compiler from complaining.
    // $$kk: 06.21.99: moving this way up top
    // struct GM_Song;

    // sample and instrument callbacks

    // $$kk: 04.19.99
    //typedef XBOOL		(*GM_LoopDoneCallbackPtr)(void *context);
    typedef XBOOL		(*GM_LoopDoneCallbackPtr)(void *context, void *threadContext);

    typedef void		(*GM_DoubleBufferCallbackPtr)(void *context, XPTR pWhichBufferFinished, INT32 *pBufferSize);

    /* $$fb 2003-03-14: add VOICE_REFERENCE to callback. Fix for 4828556 */
    //typedef void		(*GM_SoundDoneCallbackPtr)(void *context);
    typedef void		(*GM_SoundDoneCallbackPtr)(VOICE_REFERENCE sender, void *context, void *threadContext);

    typedef void		(*GM_SampleFrameCallbackPtr)(void *threadContext, INT32 reference, INT32 sampleFrame);

    // sequencer callbacks
    typedef void		(*GM_ControlerCallbackPtr)(void *threadContext, struct GM_Song *pSong, void * reference, short int channel, short int track, short int controler, short int value);
    typedef	void		(*GM_SongCallbackProcPtr)(void *threadContext, struct GM_Song *pSong);
    typedef	void		(*GM_SongTimeCallbackProcPtr)(void *threadContext, struct GM_Song *pSong, UINT32 currentMicroseconds, UINT32 currentMidiClock);
    typedef	void		(*GM_SongMetaCallbackProcPtr)(void *threadContext, struct GM_Song *pSong, char markerType, void *pMetaText, INT32 metaTextLength, INT16 currentTrack);

    // mixer callbacks
    typedef void		(*GM_AudioTaskCallbackPtr)(void *threadContext, UINT32 ticks);
    typedef void		(*GM_AudioOutputCallbackPtr)(void *threadContext, void *samples, INT32 sampleSize, INT32 channels, UINT32 lengthInFrames);



#if CPU_TYPE == kRISC
#pragma options align=power
#elif defined(X_REQUIRE_64BIT_ALIGNMENT)
#pragma pack (8)
#elif defined(X_REQUIRE_32BIT_ALIGNMENT)
#pragma pack (4)
#endif


    struct GM_SampleCallbackEntry
    {
	UINT32					frameOffset;
	GM_SampleFrameCallbackPtr		pCallback;
	INT32							reference;
	struct GM_SampleCallbackEntry	*pNext;
    };
    typedef struct GM_SampleCallbackEntry GM_SampleCallbackEntry;

    // Flags for embedded midi objects. (eMidi support)
    enum
    {
	EM_FLUSH_ID					=	FOUR_CHAR('F','L','U','S'),	//	'FLUS'		// immediate command
	EM_CACHE_ID					=	FOUR_CHAR('C','A','C','H'),	//	'CACH'		// immediate command
	EM_DATA_ID					=	FOUR_CHAR('D','A','T','A')	//	'DATA'		// block resources
    };


    // Flags for ADSR module. ADSRRecord.ADSRFlags
    enum
    {
	ADSR_OFF_LONG				=	0,
	ADSR_LINEAR_RAMP_LONG 		=	FOUR_CHAR('L','I','N','E'),	//	'LINE'
	ADSR_SUSTAIN_LONG 			=	FOUR_CHAR('S','U','S','T'),	//	'SUST'
	ADSR_TERMINATE_LONG			=	FOUR_CHAR('L','A','S','T'),	//	'LAST'
	ADSR_GOTO_LONG				=	FOUR_CHAR('G','O','T','O'),	//	'GOTO'
	ADSR_GOTO_CONDITIONAL_LONG	=	FOUR_CHAR('G','O','S','T'),	//	'GOST'
	ADSR_RELEASE_LONG			=	FOUR_CHAR('R','E','L','S'),	//	'RELS'

	ADSR_STAGES					=	8,		// max number of ADSR stages

	ADSR_OFF 					=	ADSR_OFF_LONG,
	ADSR_LINEAR_RAMP 			=	ADSR_LINEAR_RAMP_LONG,
	ADSR_SUSTAIN 				=	ADSR_SUSTAIN_LONG,
	ADSR_TERMINATE				=	ADSR_TERMINATE_LONG,
	ADSR_GOTO					=	ADSR_GOTO_LONG,
	ADSR_GOTO_CONDITIONAL		=	ADSR_GOTO_CONDITIONAL_LONG,
	ADSR_RELEASE				=	ADSR_RELEASE_LONG
    };


    struct ADSRRecord
    {
	INT32			currentTime;
	INT32			currentLevel;
	INT32			previousTarget;
	INT32			sustainingDecayLevel;
	INT32			ADSRLevel[ADSR_STAGES];
	INT32			ADSRTime[ADSR_STAGES];
	INT32			ADSRFlags[ADSR_STAGES];
	INT32			mode;
	UBYTE			currentPosition;	//	ranges from 0 to ADSR_STAGES
    };
    typedef struct ADSRRecord ADSRRecord;

    // kinds of LFO modules. LFORecord.where_to_feed & CurveRecord.tieTo
    enum
    {
	VOLUME_LFO_LONG			=	FOUR_CHAR('V','O','L','U'),	// 'VOLU'
	PITCH_LFO_LONG			=	FOUR_CHAR('P','I','T','C'),	// 'PITC'
	STEREO_PAN_LFO_LONG		=	FOUR_CHAR('S','P','A','N'),	// 'SPAN'
	STEREO_PAN_NAME2_LONG	=	FOUR_CHAR('P','A','N',' '),	// 'PAN '
	LPF_FREQUENCY_LONG		=	FOUR_CHAR('L','P','F','R'),	// 'LPFR'
	LPF_DEPTH_LONG			= 	FOUR_CHAR('L','P','R','E'),	// 'LPRE'
	LOW_PASS_AMOUNT_LONG	=	FOUR_CHAR('L','P','A','M'),	// 'LPAM'

	LOW_PASS_AMOUNT			=	LOW_PASS_AMOUNT_LONG,
	VOLUME_LFO				=	VOLUME_LFO_LONG,
	PITCH_LFO				=	PITCH_LFO_LONG,
	STEREO_PAN_LFO			=	STEREO_PAN_LFO_LONG,
	STEREO_PAN_NAME2		=	STEREO_PAN_NAME2_LONG,
	LPF_FREQUENCY			=	LPF_FREQUENCY_LONG,
	LPF_DEPTH				= 	LPF_DEPTH_LONG
    };

    // kinds of LFO wave shapes. LFORecord.waveShape parameter
    enum
    {
	// 4 byte file codes
	SINE_WAVE_LONG 			=		FOUR_CHAR('S','I','N','E'),	// 'SINE'
	TRIANGLE_WAVE_LONG 		=		FOUR_CHAR('T','R','I','A'),	// 'TRIA'
	SQUARE_WAVE_LONG 		=		FOUR_CHAR('S','Q','U','A'),	// 'SQUA'
	SQUARE_WAVE2_LONG 		=		FOUR_CHAR('S','Q','U','2'),	// 'SQU2'
	SAWTOOTH_WAVE_LONG 		=		FOUR_CHAR('S','A','W','T'),	// 'SAWT'
	SAWTOOTH_WAVE2_LONG		=		FOUR_CHAR('S','A','W','2'),	// 'SAW2'

	// small footprint translations
	SINE_WAVE 				=		SINE_WAVE_LONG,
	TRIANGLE_WAVE 			=		TRIANGLE_WAVE_LONG,
	SQUARE_WAVE 			=		SQUARE_WAVE_LONG,
	SQUARE_WAVE2 			=		SQUARE_WAVE2_LONG,
	SAWTOOTH_WAVE 			=		SAWTOOTH_WAVE_LONG,
	SAWTOOTH_WAVE2			=		SAWTOOTH_WAVE2_LONG
    };

    // Additional elements used by Curve functions. CurveRecord.tieTo parameter
    enum
    {
	VOLUME_LFO_FREQUENCY_LONG	=	FOUR_CHAR('V','O','L','F'),	// 'VOLF'
	PITCH_LFO_FREQUENCY_LONG 	=	FOUR_CHAR('P','I','T','F'),	// 'PITF'
	NOTE_VOLUME_LONG			=	FOUR_CHAR('N','V','O','L'),	// 'NVOL'
	VOLUME_ATTACK_TIME_LONG		=	FOUR_CHAR('A','T','I','M'),	// 'ATIM'
	VOLUME_ATTACK_LEVEL_LONG 	=	FOUR_CHAR('A','L','E','V'),	// 'ALEV'
	SUSTAIN_RELEASE_TIME_LONG 	=	FOUR_CHAR('S','U','S','T'),	// 'SUST'
	SUSTAIN_LEVEL_LONG			=	FOUR_CHAR('S','L','E','V'),	// 'SLEV'
	RELEASE_TIME_LONG			=	FOUR_CHAR('R','E','L','S'),	// 'RELS'
	WAVEFORM_OFFSET_LONG		=	FOUR_CHAR('W','A','V','E'),	// 'WAVE'
	SAMPLE_NUMBER_LONG			=	FOUR_CHAR('S','A','M','P'),	// 'SAMP'
	MOD_WHEEL_CONTROL_LONG		=	FOUR_CHAR('M','O','D','W'),	// 'MODW'

	VOLUME_LFO_FREQUENCY	=	VOLUME_LFO_FREQUENCY_LONG,
	PITCH_LFO_FREQUENCY 	=	PITCH_LFO_FREQUENCY_LONG,
	NOTE_VOLUME				=	NOTE_VOLUME_LONG,
	VOLUME_ATTACK_TIME		=	VOLUME_ATTACK_TIME_LONG,
	VOLUME_ATTACK_LEVEL 	=	VOLUME_ATTACK_LEVEL_LONG,
	SUSTAIN_RELEASE_TIME 	=	SUSTAIN_RELEASE_TIME_LONG,
	SUSTAIN_LEVEL			=	SUSTAIN_LEVEL_LONG,
	RELEASE_TIME			=	RELEASE_TIME_LONG,
	WAVEFORM_OFFSET			=	WAVEFORM_OFFSET_LONG,
	SAMPLE_NUMBER			=	SAMPLE_NUMBER_LONG,
	MOD_WHEEL_CONTROL		=	MOD_WHEEL_CONTROL_LONG
    };

    // these are used in the raw instrument format to determine a top level command and
    // structure of the following date
    enum
    {
	DEFAULT_MOD_LONG		=	FOUR_CHAR('D','M','O','D'),		//	'DMOD'
	ADSR_ENVELOPE_LONG		=	FOUR_CHAR('A','D','S','R'),		//	'ADSR'
	EXPONENTIAL_CURVE_LONG	=	FOUR_CHAR('C','U','R','V'),		//	'CURV'
	LOW_PASS_FILTER_LONG	=	FOUR_CHAR('L','P','G','F'),		//	'LPGF'

	INST_ADSR_ENVELOPE		=	ADSR_ENVELOPE_LONG,
	INST_EXPONENTIAL_CURVE	=	EXPONENTIAL_CURVE_LONG,
	INST_LOW_PASS_FILTER	=	LOW_PASS_FILTER_LONG,
	INST_DEFAULT_MOD		=	DEFAULT_MOD_LONG,
	INST_PITCH_LFO			=	PITCH_LFO_LONG,
	INST_VOLUME_LFO			=	VOLUME_LFO_LONG,
	INST_STEREO_PAN_LFO		=	STEREO_PAN_LFO_LONG,
	INST_STEREO_PAN_NAME2	=	STEREO_PAN_NAME2_LONG,
	INST_LOW_PASS_AMOUNT	=	LOW_PASS_AMOUNT_LONG,
	INST_LPF_DEPTH			=	LPF_DEPTH_LONG,
	INST_LPF_FREQUENCY		=	LPF_FREQUENCY_LONG
    };

    // Controls for registered parameter status
    enum
    {
	USE_NO_RP = 0,		// no registered parameters selected
	USE_NRPN,			// use non registered parameters
	USE_RPN				// use registered parameters
    };

    // Controls for channelBankMode
    enum
    {
	USE_GM_DEFAULT = 0,			// this is default behavior
	// normal bank for channels 1-9 and 11-16
	// percussion for channel 10
	USE_NON_GM_PERC_BANK,		// will force the use of the percussion
	// bank reguardless of channel and allow program
	// changes to reflect the percussion instrments and
	// allow you to change them with normal notes
	USE_GM_PERC_BANK,			// will force the use of the percussion
	// bank reguardless of channel and act just like a GM
	// percussion channel. ie. midi note represents the instrument
	// to play
	USE_NORM_BANK				// will force the use of the normal
	// bank reguardless of channel

    };

    struct LFORecord
    {
	INT32		DC_feed;		// DC feed amount
	INT32		level;
	INT32		period;
	INT32		currentTime;
	INT32		LFOcurrentTime;
	INT32		currentWaveValue;
	INT32		where_to_feed;
	INT32		waveShape;
	INT32		mode;
	ADSRRecord	a;
    };
    typedef struct LFORecord LFORecord;

    // possible tieFrom values		to_Scalar range		from_Value range
    //	PITCH_LFO					|
    //	VOLUME_LFO					|
    //	MOD_WHEEL_CONTROL			|
    //	SAMPLE_NUMBER				|

    // possible tieTo values		to_Scalar range		from_Value range
    //	LPF_FREQUENCY				|
    //	NOTE_VOLUME					|
    //	SUSTAIN_RELEASE_TIME		|
    //	SUSTAIN_LEVEL				|
    //	RELEASE_TIME				|
    //	VOLUME_ATTACK_TIME			|
    //	VOLUME_ATTACK_LEVEL			|
    //	WAVEFORM_OFFSET				|
    //	LOW_PASS_AMOUNT				|
    //	PITCH_LFO					|
    //	VOLUME_LFO					|
    //	PITCH_LFO_FREQUENCY			|
    //	VOLUME_LFO_FREQUENCY		|

    struct CurveRecord
    {
	INT16		to_Scalar[MAX_CURVES];
	UBYTE		from_Value[MAX_CURVES];		// midi range 0 to 127
	INT32		tieFrom;
	INT32		tieTo;
	INT16		curveCount;
    };
    typedef struct CurveRecord CurveRecord;

    struct InstrumentEntry
    {
	struct InstrumentEntry *next; // link
	UINT32		ticks;
	UBYTE		*ptr;	// location in MIDI data
	UBYTE		*bsPtr; // location of bank select pointer
	INT16		track;
	UBYTE		channel;
	UBYTE		program;
	UBYTE		bank;
	UBYTE		dirty;
	UBYTE		bankMode; // bank mode at time of pgm change
    };
    typedef struct InstrumentEntry InstrumentEntry;

    // structure for maintaining info about patch changes in a GM_Song

    struct PatchInfo
    {
	InstrumentEntry		*instrChangeInfo;
	UBYTE				*instrPtrLoc; // used to communicate location of a program change (move dec. elsewhere?)
	UBYTE				*bankPtrLoc; // location of a bank change msg
	INT32				instrCount; // count of program change messages found
	INT32				rsCount;   // count of PCs with running status (unused right now)
	INT32				streamIncrement; // how much to add to midi_stream ptr after
	// PV_ProcessProgramChange has modified the data
    };
    typedef struct PatchInfo PatchInfo;

    struct GM_KeymapSplit
    {
	UBYTE					lowMidi;
	UBYTE					highMidi;
	INT16					miscParameter1;		// can be smodParmeter1 if enableSoundModifier
	// enabled, otherwise its a replacement
	// rootKey for sample
	INT16					miscParameter2;
	struct GM_Instrument	*pSplitInstrument;
    };
    typedef struct GM_KeymapSplit GM_KeymapSplit;

    struct GM_KeymapSplitInfo
    {
	XShortResourceID	defaultInstrumentID;
	UINT16				KeymapSplitCount;
	GM_KeymapSplit		keySplits[1];
    };
    typedef struct GM_KeymapSplitInfo GM_KeymapSplitInfo;

    // main structure for all waveform objects
    struct GM_Waveform
    {
	XLongResourceID		waveformID;			// extra specific data
	UINT32				currentFilePosition;// if used will be an file byte position
	UINT16				baseMidiPitch;		// base Midi pitch of recorded sample ie. 60 is middle 'C'
	UBYTE				bitSize;			// number of bits per sample
	UBYTE				channels;			// number of channels
	UINT32				waveSize;			// total waveform size in bytes
	UINT32				waveFrames;			// number of frames
	UINT32				startLoop;			// start loop point offset
	UINT32				endLoop;			// end loop point offset
	UINT32				numLoops;			// number of loops between loop points before continuing to end of sample
	XFIXED				sampledRate;		// FIXED_VALUE 16.16 value for recording
	SBYTE 				*theWaveform;		// array data that morphs into what ever you need
    };
    typedef struct GM_Waveform GM_Waveform;

    // Internal Instrument structure
    // aligned structure to 8 bytes
    struct GM_Instrument
    {
	AccessSemaphore		accessStatus;
	INT16				masterRootKey;
	XShortResourceID	smodResourceID;
	INT16				miscParameter1;
	INT16				miscParameter2;

	XBOOL		/*0*/	disableSndLooping;		// Disable waveform looping
	XBOOL		/*1*/	playAtSampledFreq;		// Play instrument at sampledRate only
	XBOOL		/*2*/	doKeymapSplit;			// If TRUE, then this instrument is a keysplit defination
	XBOOL		/*3*/	notPolyphonic;			// if FALSE, then instrument is a mono instrument
	XBOOL		/*4*/	enableSoundModifier;
	XBOOL		/*5*/	extendedFormat;			// extended format instrument
	XBOOL		/*6*/	sampleAndHold;
	XBOOL		/*7*/	useSampleRate;			// factor in sample rate into pitch calculation

	XBOOL		/*0*/	processingSlice;
	XBOOL		/*1*/	useSoundModifierAsRootKey;
	XBOOL		/*2*/	avoidReverb;			// if TRUE, this instrument is not mixed into reverb unit
	UBYTE		/*3*/	usageReferenceCount;	// number of references this instrument is associated to

	UBYTE		/*4*/	LFORecordCount;
	UBYTE		/*5*/	curveRecordCount;

	INT16				panPlacement;			// inital stereo pan placement of this instrument

	INT32				LPF_frequency;
	INT32				LPF_resonance;
	INT32				LPF_lowpassAmount;

	LFORecord			LFORecords[MAX_LFOS];
	ADSRRecord			volumeADSRRecord;
	CurveRecord			curve[MAX_CURVES];
	union
	    {
		GM_KeymapSplitInfo	k;
		GM_Waveform			w;
	    } u;
    };
    typedef struct GM_Instrument GM_Instrument;

    struct GM_ControlCallback
    {
	// these pointers are NULL until used, then they are allocated
	GM_ControlerCallbackPtr	callbackProc[MAX_CONTROLLERS];		// current controller callback functions
	void					*callbackReference[MAX_CONTROLLERS];
    };
    typedef struct GM_ControlCallback GM_ControlCallback;
    typedef struct GM_ControlCallback * GM_ControlCallbackPtr;


    // Internal Song structure
    // aligned structure to 8 bytes
    struct GM_Song
    {
	// $$kk: 05.04.99
	// synthesizer this song should use
	//GM_Synth			*pSynth;
	// $$kk: 07.10.99: let's allow more than one!!
	GM_Synth			*pSynths;


	XShortResourceID	songID;
	INT16				maxSongVoices;
	INT16				mixLevel;
	INT16				maxEffectVoices;

	// various values calculated during first scan of midi file. The one's marked
	// with a '*' are actaully usefull.
	INT16				averageTotalVoices;
	INT16				averageActiveVoices;
	INT16				voiceCount, voiceSustain;
	INT16				averageVoiceUsage;		// * average voice usage over time
	INT16				maxVoiceUsage;			// * max number of voices used

	XFIXED				MasterTempo;			// master midi tempo (fixed point)
	UINT16				songTempo;				// tempo (16667 = 1.0)
	INT16				songPitchShift;			// master pitch shift

	UINT16				allowPitchShift[(MAX_CHANNELS / 16) + 1];		// allow pitch shift

	void				*context;			// context of song creation. C++ 'this' pointer, thread, etc
	void *				userReference;			// user reference. Can be anything

	GM_SongCallbackProcPtr		songEndCallbackPtr;		// called when song ends/stops/free'd up

	GM_SongTimeCallbackProcPtr	songTimeCallbackPtr;	// called every slice to pass the time
	INT32				songTimeCallbackReference;

	GM_SongMetaCallbackProcPtr	metaEventCallbackPtr;	// called during playback with current meta events
	INT32				metaEventCallbackReference;

	// these pointers are NULL until used, then they are allocated
	GM_ControlCallbackPtr		controllerCallback;		// called during playback with controller info

	ReverbMode	/* 0 */	defaultReverbType;

	VelocityCurveType	velocityCurveType;			// which curve to use. (Range is 0 to 2)

	ScanMode	/* 2 */	AnalyzeMode;				// analyze mode (Byte)
	XBOOL		/* 3 */	ignoreBadInstruments;		// allow bad patches. Don't fail because it can't load

	XBOOL		/* 0 */	allowProgramChanges;
	XBOOL		/* 1 */	loopSong;					// loop song when done
	XBOOL		/* 2 */	disposeSongDataWhenDone;		// if TRUE, then free midi data
	XBOOL		/* 3 */	SomeTrackIsAlive;			// song still alive
	XBOOL		/* 4 */	songFinished;				// TRUE at start of song, FALSE and end
	XBOOL		/* 5 */	processingSlice;			// TRUE if processing slice of this song
	XBOOL		/* 6 */	cacheSamples;				// if TRUE, then samples will be cached
	XBOOL				omniModeOn;					// if TRUE, then omni mode is on

	XFIXED				songFadeRate;				// when non-zero fading is enabled
	XFIXED				songFixedVolume;			// inital volume level that will be changed by songFadeRate
	INT16				songFadeMaxVolume;			// max volume
	INT16				songFadeMinVolume;			// min volume
	XBOOL				songEndAtFade;				// when true, stop song at end of fade

	INT16				songVolume;
	INT16				songMasterStereoPlacement;	// master stereo placement (-8192 to 8192)

	INT16				defaultPercusionProgram;	// default percussion program for percussion channel.
	// -1 means GM style bank select, -2 means allow program changes on percussion

	INT16				songLoopCount;				// current loop counter. Starts at 0
	INT16				songMaxLoopCount;			// when songLoopCount reaches songMaxLoopCount it will be set to 0

	UINT32				songMidiTickLength;			// song midi tick length. -1 not calculated yet.
	UINT32				songMicrosecondLength;		// song microsecond length. -1 not calculated yet.

	void				*midiData;					// pointer to midi data for this song
	UINT32				midiSize;					// size of midi data

	//	instrument array. These are the instruments that are used by just this song
	GM_Instrument		*instrumentData[MAX_INSTRUMENTS*MAX_BANKS];

	XLongResourceID		instrumentRemap[MAX_INSTRUMENTS*MAX_BANKS];
	XLongResourceID		remapArray[MAX_INSTRUMENTS*MAX_BANKS];

	void				*pUsedPatchList;				// This is NULL most of the time, only
	// GM_LoadSongInstruments sets it
	// instruments by notes
	// This should be about 4096 bytes
	// during preprocess of the midi file, this will be
	// the instruments that need to be loaded
	// total divided by 8 bits. each bit represents an instrument

	SBYTE				firstChannelBank[MAX_CHANNELS];		// set during preprocess. this is the program
	INT16				firstChannelProgram[MAX_CHANNELS];	// to be set at the start of a song

	// channel based controler values
	SBYTE				channelWhichParameter[MAX_CHANNELS];			// 0 for none, 1 for RPN, 2 for NRPN
	SBYTE				channelRegisteredParameterLSB[MAX_CHANNELS];	// Registered Parameter least signifcant byte
	SBYTE				channelRegisteredParameterMSB[MAX_CHANNELS];	// Registered Parameter most signifcant byte
	SBYTE				channelNonRegisteredParameterLSB[MAX_CHANNELS];	// Non-Registered Parameter least signifcant byte
	SBYTE				channelNonRegisteredParameterMSB[MAX_CHANNELS];	// Non-Registered Parameter most signifcant byte
	UBYTE				channelBankMode[MAX_CHANNELS];					// channel bank mode
	UBYTE				channelSustain[MAX_CHANNELS];					// sustain pedal on/off
	UBYTE				channelVolume[MAX_CHANNELS];					// current channel volume
	UBYTE				channelChorus[MAX_CHANNELS];					// current channel chorus
	UBYTE				channelExpression[MAX_CHANNELS];				// current channel expression
	UBYTE				channelPitchBendRange[MAX_CHANNELS];			// current bend range in half steps
	UBYTE				channelReverb[MAX_CHANNELS];					// current channel reverb
	UBYTE				channelModWheel[MAX_CHANNELS];					// Mod wheel (primarily affects pitch bend)
	UBYTE				channelLowPassAmount[MAX_CHANNELS];				// low pass amount controller (NOT CONNECTED as of 3.8.99)
	UBYTE				channelResonanceFilterAmount[MAX_CHANNELS];		// Resonance amount controller (NOT CONNECTED as of 3.8.99)
	UBYTE				channelFrequencyFilterAmount[MAX_CHANNELS];		// Frequency amount controller (NOT CONNECTED as of 3.8.99)
	INT16				channelBend[MAX_CHANNELS];						// MUST BE AN INT16!! current amount to bend new notes
	INT16				channelProgram[MAX_CHANNELS];					// current channel program
	SBYTE				channelBank[MAX_CHANNELS];						// current bank
	INT16				channelStereoPosition[MAX_CHANNELS];			// current channel stereo position

	// mute controls for tracks, channels, and solos
	// NOTE: Do not access these directly. Use XSetBit & XClearBit & XTestBit
	UINT32				trackMuted[(MAX_TRACKS / 32) + 1];			// track mute control bits
	UINT32				soloTrackMuted[(MAX_TRACKS / 32) + 1];		// solo track mute control bits
	UINT16				channelMuted[(MAX_CHANNELS / 16) + 1];		// current channel muted status
	UINT16				soloChannelMuted[(MAX_CHANNELS / 16) + 1];	// current channel muted status

	// internal timing variables for sequencer
	UFLOAT				UnscaledMIDITempo;
	UFLOAT				MIDITempo;
	UFLOAT				MIDIDivision;
	UFLOAT				UnscaledMIDIDivision;
	UFLOAT				CurrentMidiClock;
	UFLOAT				songMicroseconds;

	XBOOL				songPaused;

	// storage for loop playback
	XBOOL				loopbackSaved;
	UBYTE				*pTrackPositionSave[MAX_TRACKS];
	IFLOAT				trackTicksSave[MAX_TRACKS];		// must be signed
	TrackStatus			trackStatusSave[MAX_TRACKS];
	UFLOAT				currentMidiClockSave;
	UFLOAT				songMicrosecondsSave;
	SBYTE				loopbackCount;

	// internal position variables for sequencer. Set after inital preprocess
	TrackStatus			trackon[MAX_TRACKS];			// track playing? TRACK_FREE is free, TRACK_RUNNING is playing
	UINT32				tracklen[MAX_TRACKS];			// length of track in bytes
	UBYTE				*ptrack[MAX_TRACKS];			// current position in track
	UBYTE				*trackstart[MAX_TRACKS];		// start of track
	UBYTE				runningStatus[MAX_TRACKS];		// midi running status
	IFLOAT				trackticks[MAX_TRACKS];			// current position of track in ticks. must be signed

	INT32				trackcumuticks[MAX_TRACKS];		// current number of beat ticks into track
	PatchInfo			*pPatchInfo;
	// these things are currently only used by the PgmChange search but are generally useful
	// time signature info
	UBYTE				TSNumerator;
	UBYTE				TSDenominator;
    };
    typedef struct GM_Song GM_Song;

#if CPU_TYPE == kRISC
#pragma options align=reset
#elif defined(X_REQUIRE_64BIT_ALIGNMENT) || defined(X_REQUIRE_32BIT_ALIGNMENT)
#pragma pack()
#endif


    // Functions


    /**************************************************/
    /*
    ** FUNCTION InitGeneralSound(Quality theQuality,
    **								TerpMode theTerp, INT16 maxVoices,
    **								INT16 normVoices, INT16 maxEffects,
    **								INT16 maxChunkSize)
    **
    ** Overvue --
    **	This will setup the sound system, allocate memory, and such,
    **  so that any calls to play effects or songs can happen right
    **  away.
    **
    ** Private notes:
    **  This code will preinitialize the MIDI sequencer, allocate
    **  amplitude scaling buffer & init it, and init the
    **  General Magic sound system.
    **
    **	INPUT	--	theQuality
    **					Q_11K	Sound ouput is XFIXED at 11127
    **					Q_22K	Sound output is XFIXED at 22254
    **			--	theTerp
    **					Interpolation type
    **			--	use16bit
    **					If true, then hardware will be setup for 16 bit output
    **			--	maxVoices
    **					maximum voices
    **			--	normVoices
    **					number of voices normally. ie a gain
    **			--	maxEffects
    **					number of voices to be used as effects
    **
    **	OUTPUT	--
    **
    ** NOTE:
    **	Only call this once.
    */
    /**************************************************/
    OPErr GM_InitGeneralSound(void *threadContext, Quality theQuality, TerpMode theTerp, AudioModifiers theMods,
			      INT16 maxVoices, INT16 normVoices, INT16 maxEffects);

    // convert GenSynth Quality to actual sample rate used
    UINT32 GM_ConvertFromOutputQualityToRate(Quality quality);

    /**************************************************/
    /*
    ** FUNCTION FinisGeneralSound;
    **
    ** Overvue --
    **	This will release any memory allocated by InitGeneralSound, clean up.
    **
    **	INPUT	--
    **	OUTPUT	--
    **
    ** NOTE:
    **	Only call this once.
    */
    /**************************************************/
    void GM_FinisGeneralSound(void *threadContext);

    // get calculated microsecond time different between mixer slices.
    UINT32 GM_GetMixerUsedTime(void);

    // Get CPU load in percent. This function is realtime and assumes the mixer has been allocated
    UINT32 GM_GetMixerUsedTimeInPercent(void);

    // allocate the reverb buffers and enable them.
    void GM_SetupReverb(void);
    // deallocate the reverb buffers
    void GM_CleanupReverb(void);
    // Is current reverb fixed (old style)?
    XBOOL GM_IsReverbFixed(void);
    // get highest MIDI verb amount required to activate verb
    UBYTE GM_GetReverbEnableThreshold(void);
    // Set the global reverb type
    void GM_SetReverbType(ReverbMode theReverbMode);
    // Get the global reverb type
    ReverbMode GM_GetReverbType(void);

    // process the verb. Only call on data currently in the mix bus
    void GM_ProcessReverb(void);

    /* Sound hardware specific
     */
    XBOOL		GM_StartHardwareSoundManager(void *threadContext);
    void		GM_StopHardwareSoundManager(void *threadContext);

    // number of devices. ie different versions of the HAE connection. DirectSound and waveOut
    // return number of devices. ie 1 is one device, 2 is two devices.
    // NOTE: This function needs to function before any other calls may have happened.
    INT32 GM_MaxDevices(void);

    // set the current device. device is from 0 to HAE_MaxDevices()
    // NOTE:	This function needs to function before any other calls may have happened.
    //			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
    //			in order for the change to take place. deviceParameter is a device specific
    //			pointer. Pass NULL if you don't know what to use.
    void GM_SetDeviceID(INT32 deviceID, void *deviceParameter);

    // return current device ID, and fills in the deviceParameter with a device specific
    // pointer. It will pass NULL if there is nothing to use.
    // NOTE: This function needs to function before any other calls may have happened.
    INT32 GM_GetDeviceID(void *deviceParameter);

    // get deviceID name
    // NOTE:	This function needs to function before any other calls may have happened.
    //			Format of string is a zero terminated comma delinated C string.
    //			"platform,method,misc"
    //	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
    //			"WinOS,DirectSound,multi threaded"
    //			"WinOS,waveOut,multi threaded"
    //			"WinOS,VxD,low level hardware"
    //			"WinOS,plugin,Director"
    void GM_GetDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength);

    void GM_GetSystemVoices(INT16 *pMaxSongVoices, INT16 *pMixLevel, INT16 *pMaxEffectVoices);

    OPErr GM_ChangeSystemVoices(INT16 maxVoices, INT16 mixLevel, INT16 maxEffects);

    OPErr GM_ChangeAudioModes(void *threadContext, Quality theQuality, TerpMode theTerp, AudioModifiers theMods);

    /**************************************************/
    /*
    ** FUNCTION PauseGeneralSound;
    **
    ** Overvue --
    **	This is used to pause the system, and release hardware for other tasks to
    **	play around. Call ResumeGeneralSound when ready to resume working with
    **	the system.
    **
    **	INPUT	--
    **	OUTPUT	--	OPErr	Errors in pausing system.
    **
    ** NOTE:
    */
    /**************************************************/
    OPErr GM_PauseGeneralSound(void *threadContext);

    // Pause all songs
    void GM_PauseSequencer(void);
    // resume all songs
    void GM_ResumeSequencer(void);

    // Pause just this song
    void GM_PauseSong(GM_Song *pSong);
    // Resume just this song
    void GM_ResumeSong(GM_Song *pSong);

    char GM_GetControllerValue(GM_Song *pSong, INT16 channel, INT16 controller);
    // return pitch bend for channel
    void GM_GetPitchBend(GM_Song *pSong, INT16 channel, unsigned char *pLSB, unsigned char *pMSB);

    /**************************************************/
    /*
    ** FUNCTION ResumeGeneralSound;
    **
    ** Overvue --
    **	This is used to resume the system, and take over the sound hardware. Call this
    **	after calling PauseGeneralSound.
    **
    **	INPUT	--
    **	OUTPUT	--	OPErr	Errors in resuming. Continue to call until errors have
    **						stops, or alert user that something is still holding
    **						the sound hardware.
    **
    ** NOTE:
    */
    /**************************************************/
    OPErr GM_ResumeGeneralSound(void *threadContext);





    /**************************************************/
    /*
    ** FUNCTION BeginSong(GM_Song *theSong, GM_SongCallbackProcPtr theCallbackProc);
    **
    ** Overvue --
    **	This will start a song, given the data structure that contains the midi data,
    **	instrument data, and specifics about playback of this song.
    **
    **	INPUT	--	theSong,			contains pointers to the song data, midi data,
    **									all instruments used in this song.
    **				theCallbackProc,	when the song is done, even in the looped
    **									case, this procedure will be called.
    **				useEmbeddedMixerSettings
    **									when TRUE the mixer will be reconfigured
    **									to use the embedded song settings. If FALSE
    **									then the song will use the current settings
    **	OUTPUT	--
    **
    ** NOTE:
    */
    /**************************************************/
    OPErr GM_BeginSong(GM_Song *theSong, GM_SongCallbackProcPtr theCallbackProc, XBOOL useEmbeddedMixerSettings, XBOOL autoLevel);

    // return valid context for song that was pass in when calling GM_LoadSong or GM_CreateLiveSong
    void * GM_GetSongContext(GM_Song *pSong);
    // set valid context for song
    void GM_SetSongContext(GM_Song *pSong, void *context);

    // Load the SongID from an external SONG resource and or a extneral midi resource.
    //
    //	threadContext		context of thread. passed all the way down to callbacks
    //	context				context of song creation. C++ 'this' pointer, etc. this is a user variable
    //						Its just stored in the GM_Song->context variable
    //	songID				will be the ID used during playback
    //	theExternalSong		standard SONG resource structure

    //	theExternalMidiData	if not NULL, then will use this midi data rather than what is found in external SONG resource
    //	midiSize			size of midi data if theExternalMidiData is not NULL
    //						theExternalMidiData and midiSize is not used if the songType is RMF

    //	pInstrumentArray	array, if not NULL will be filled with the instruments that need to be loaded.
    //	loadInstruments		if not zero, then instruments and samples will be loaded
    //	pErr				pointer to an OPErr
    GM_Song * GM_LoadSong(void *threadContext, void *context, XShortResourceID songID, void *theExternalSong,
			  void *theExternalMidiData, INT32 midiSize,
			  XShortResourceID *pInstrumentArray,
			  XBOOL loadInstruments, XBOOL ignoreBadInstruments,
			  OPErr *pErr);
    // Create Song with no midi data associated. Used for direct control of a synth object
    GM_Song * GM_CreateLiveSong(void *context, XShortResourceID songID);
    OPErr GM_StartLiveSong(GM_Song *pSong, XBOOL loadPatches);

    // return note offset in semi tones	(12 is down an octave, -12 is up an octave)
    INT32 GM_GetSongPitchOffset(GM_Song *pSong);
    // set note offset in semi tones	(12 is down an octave, -12 is up an octave)
    void GM_SetSongPitchOffset(GM_Song *pSong, INT32 offset);
    // If allowPitch is FALSE, then "GM_SetSongPitchOffset" will have no effect on passed
    // channel (0 to 15)
    void GM_AllowChannelPitchOffset(GM_Song *pSong, unsigned short int channel, XBOOL allowPitch);
    // Return if the passed channel will allow pitch offset
    XBOOL GM_DoesChannelAllowPitchOffset(GM_Song *pSong, unsigned short int channel);

    /* removes song from list of active songs, no callbacks, doesn't free song */
    void GM_RemoveFromSongsToPlay(GM_Song *pSong);

    /**************************************************/
    /*
    ** FUNCTION EndSong;
    **
    ** Overvue --
    **	This will end the current song playing.
    **  All song resources not be will be disposed or released.
    **
    **	INPUT	--
    **	OUTPUT	--
    **
    ** NOTE:
    */
    /*************************************************/
    void GM_EndSong(void *threadContext, GM_Song *pSong);

    OPErr GM_FreeSong(void *threadContext, GM_Song *theSong);

    void GM_MergeExternalSong(void *theExternalSong, XShortResourceID theSongID, GM_Song *theSong);

    /**************************************************/
    /*
    ** FUNCTION SongTicks;
    **
    ** Overvue --
    **	This will return in 1/60th of a second, the count since the start of the song
    **	currently playing.
    **
    **	INPUT	--
    **	OUTPUT	--	INT32,		returns ticks since BeginSong.
    **						returns 0 if no song is playing.
    **
    ** NOTE:
    */
    /**************************************************/
    UINT32 GM_SongTicks(GM_Song *pSong);

    // Return the length in MIDI ticks of the song passed
    //	pSong	GM_Song structure. Data will be cloned for this function.
    //	pErr		OPErr error type
    UINT32 GM_GetSongTickLength(GM_Song *pSong, OPErr *pErr);
    OPErr GM_SetSongTickPosition(GM_Song *pSong, UINT32 songTickPosition);

    // scan through song and build data structure with all program changes
    OPErr GM_GetSongInstrumentChanges(void *theSongResource, GM_Song **outSong, XBYTE **outTrackNames);

    UINT32 GM_SongMicroseconds(GM_Song *pSong);
    UINT32 GM_GetSongMicrosecondLength(GM_Song *pSong, OPErr *pErr);
    // Set the song position in microseconds
    OPErr GM_SetSongMicrosecondPosition(GM_Song *pSong, UINT32 songMicrosecondPosition);


    // Get current audio time stamp based upon the audio built interrupt
    UINT32 GM_GetSyncTimeStamp(void);

    // Get current audio time stamp in microseconds; this is the
    // microseconds' worth of samples that have passed through the
    // audio device.  it never decreases.
    UINT32 GM_GetDeviceTimeStamp(void);

    // Update count of samples played.  This function caluculates from number of bytes,
    // given the sample frame size from the mixer variables, and the bytes of data written
    void GM_UpdateSamplesPlayed(UINT32 currentPos);

    // Get current audio time stamp based upon the audio built interrupt, but ahead in time
    // and quantized for the particular OS
    UINT32 GM_GetSyncTimeStampQuantizedAhead(void);

    // Get current number of samples played; this is the
    // number of samples that have passed through the
    // audio device.  it never decreases.
    UINT32 GM_GetSamplesPlayed(void);

    // Return the used patch array of instruments used in the song passed.
    //	theExternalSong	standard SONG resource structure
    //	theExternalMidiData	if not NULL, then will use this midi data rather than what is found in external SONG resource
    //	midiSize			size of midi data if theExternalMidiData is not NULL
    //	pInstrumentArray	array, if not NULL will be filled with the instruments that need to be loaded.
    //	pErr				pointer to an OPErr
    INT32 GM_GetUsedPatchlist(void *theExternalSong, void *theExternalMidiData, INT32 midiSize,
			      XShortResourceID *pInstrumentArray, OPErr *pErr);

    // set key velocity curve type
    void GM_SetVelocityCurveType(GM_Song *pSong, VelocityCurveType velocityCurveType);

    /**************************************************/
    /*
    ** FUNCTION IsSongDone;
    **
    ** Overvue --
    **	This will return a XBOOL if a song is done playing or not.
    **
    **	INPUT	--
    **	OUTPUT	--	XBOOL,	returns TRUE if song is done playing,
    **							or FALSE if not playing.
    **
    ** NOTE:
    */
    /**************************************************/
    XBOOL GM_IsSongDone(GM_Song *pSong);

    // Mute and unmute tracks (0 to 64)
    void GM_MuteTrack(GM_Song *pSong, short int track);
    void GM_UnmuteTrack(GM_Song *pSong, short int track);
    // Get mute status of all tracks. pStatus should be an array of 65 bytes
    void GM_GetTrackMuteStatus(GM_Song *pSong, char *pStatus);

    void GM_SoloTrack(GM_Song *pSong, short int track);
    void GM_UnsoloTrack(GM_Song *pSong, short int track);
    // will write only MAX_TRACKS bytes for MAX_TRACKS Midi tracks
    void GM_GetTrackSoloStatus(GM_Song *pSong, char *pStatus);

    // Mute and unmute channels (0 to 15)
    void GM_MuteChannel(GM_Song *pSong, short int channel);
    void GM_UnmuteChannel(GM_Song *pSong, short int channel);
    // Get mute status of all channels. pStatus should be an array of 16 bytes
    void GM_GetChannelMuteStatus(GM_Song *pSong, char *pStatus);

    void GM_SoloChannel(GM_Song *pSong, short int channel);
    void GM_UnsoloChannel(GM_Song *pSong, short int channel);
    void GM_GetChannelSoloStatus(GM_Song *pSong, char *pStatus);

    /*************************************************/
    /*
    ** FUNCTION SetMasterVolume;
    **
    ** Overvue --
    **	This will set the master output volume of the mixer.
    **
    **	INPUT	--	theVolume,	0-256 which is the master volume.
    **	OUTPUT	--
    **
    ** NOTE:
    **	This is different that the hardware volume. This will scale the output by
    **	theVolume factor.
    **	There is somewhat of a CPU hit, while calulating the new scale buffer.
    */
    /*************************************************/
    void GM_SetMasterVolume(INT32 theVolume);
    INT32 GM_GetMasterVolume(void);



    /**************************************************/
    /*
    **	The functions:
    **			GM_SetupSample, GM_SetupSampleDoubleBuffer, GM_SetupSampleFromInfo
    **
    **			Overall these functions are the same. They take various parmeters
    **			to allocate a voice. The VOICE_REFERENCE then can be used to start
    **			the voice playing and to modify various realtime parmeters.
    **
    **			Keep in mind that the VOICE_REFERENCE is an active voice in the mixer.
    **			You can run out of active voices. The total number is allocate when you
    **			call GM_InitGeneralSound.
    **
    **			If the GM_Setup... functions to return DEAD_VOICE this means that
    **			the total number of voices available from the mixer have all been allocated.
    **			The functions do NOT steal voices, and do NOT allocate from MIDI voices.
    */
    /**************************************************/
    VOICE_REFERENCE GM_SetupSample(XPTR theData, UINT32 frames, XFIXED theRate,
				   UINT32 theStartLoop, UINT32 theEndLoop, UINT32 theLoopTarget,
				   INT32 sampleVolume, INT32 stereoPosition,
				   void *context, INT16 bitSize, INT16 channels,
				   GM_LoopDoneCallbackPtr theLoopContinueProc,
				   GM_SoundDoneCallbackPtr theCallbackProc);

    // given a VOICE_REFERENCE returned from GM_Begin... this will tell return TRUE, if voice is
    // valid
    XBOOL GM_IsSoundReferenceValid(VOICE_REFERENCE reference);

    VOICE_REFERENCE GM_SetupSampleDoubleBuffer(XPTR pBuffer1, XPTR pBuffer2, UINT32 theSize, XFIXED theRate,
					       INT16 bitSize, INT16 channels,
					       INT32 sampleVolume, INT16 stereoPosition,
					       void *context,
					       GM_DoubleBufferCallbackPtr bufferCallback,
					       GM_SoundDoneCallbackPtr doneCallbackProc);

    VOICE_REFERENCE GM_SetupSampleFromInfo(GM_Waveform *pSample, void *context,
					   INT32 sampleVolume, INT32 stereoPosition,
					   GM_LoopDoneCallbackPtr theLoopContinueProc,
					   GM_SoundDoneCallbackPtr theCallbackProc,
					   UINT32 startOffsetFrame);

    // set all the voices you want to start at the same time the same syncReference. Then call GM_SyncStartSample
    // to start the sync start. Will return an error if its an invalid reference, or syncReference is NULL.
    OPErr GM_SetSyncSampleStartReference(VOICE_REFERENCE reference, void *syncReference);
    // Once you have called GM_SetSyncSampleStartReference on all the voices, this will set them to start at the next
    // mixer slice. Will return an error if its an invalid reference, or syncReference is NULL.
    OPErr GM_SyncStartSample(VOICE_REFERENCE reference);
    // after voice is setup, call this to start it playing now. returns 0 if started
    OPErr GM_StartSample(VOICE_REFERENCE reference);

    void GM_FreeWaveform(GM_Waveform *pWaveform);

    void GM_SetSampleDoneCallback(VOICE_REFERENCE reference, GM_SoundDoneCallbackPtr theCallbackProc, void *context);

    // This will end the sample by the reference number that is passed.
    void GM_EndSample(VOICE_REFERENCE reference, void *threadContext); // stop in current thread

    // This will end the sample by the reference number that is passed, actual ending will occur in main audio thread.
    void GM_ReleaseSample(VOICE_REFERENCE reference, void *threadContext); // stop in audio thread

    // This will return status of a sound that is being played.
    XBOOL GM_IsSoundDone(VOICE_REFERENCE reference);

    void GM_ChangeSamplePitch(VOICE_REFERENCE reference, XFIXED theNewRate);
    XFIXED GM_GetSamplePitch(VOICE_REFERENCE reference);

    void GM_ChangeSampleVolume(VOICE_REFERENCE reference, INT16 newVolume);
    INT16 GM_GetSampleVolumeUnscaled(VOICE_REFERENCE reference);
    INT16 GM_GetSampleVolume(VOICE_REFERENCE reference);
    void GM_SetSampleFadeRate(VOICE_REFERENCE reference, XFIXED fadeRate,
			      INT16 minVolume, INT16 maxVolume, XBOOL endSample);

    // get/set frequency filter amount. Range is 512 to 32512
    short int GM_GetSampleFrequencyFilter(VOICE_REFERENCE reference);
    void GM_SetSampleFrequencyFilter(VOICE_REFERENCE reference, short int frequency);

    // get/set resonance filter amount. Range is 0 to 256
    short int GM_GetSampleResonanceFilter(VOICE_REFERENCE reference);
    void GM_SetSampleResonanceFilter(VOICE_REFERENCE reference, short int resonance);

    // get/set low pass filter amount. Range is -255 to 255
    short int GM_GetSampleLowPassAmountFilter(VOICE_REFERENCE reference);
    void GM_SetSampleLowPassAmountFilter(VOICE_REFERENCE reference, short int amount);

    void GM_SetSampleLoopPoints(VOICE_REFERENCE reference, UINT32 start, UINT32 end);

    UINT32 GM_GetSampleStartTimeStamp(VOICE_REFERENCE reference);

    UINT32 GM_GetSamplePlaybackPosition(VOICE_REFERENCE reference);
    void * GM_GetSamplePlaybackPointer(VOICE_REFERENCE reference);

    void GM_ChangeSampleStereoPosition(VOICE_REFERENCE reference, INT16 newStereoPosition);
    INT16 GM_GetSampleStereoPosition(VOICE_REFERENCE reference);

    // return the current amount of reverb mix. 0-127 is the range.
    short int GM_GetSampleReverbAmount(VOICE_REFERENCE reference);
    // set amount of reverb to mix. 0-127 is the range.
    void GM_SetSampleReverbAmount(VOICE_REFERENCE reference, short int amount);
    // change status of reverb. Force on, or off
    void GM_ChangeSampleReverb(VOICE_REFERENCE reference, XBOOL enable);
    // Get current status of reverb. On or off
    XBOOL GM_GetSampleReverb(VOICE_REFERENCE reference);

    void GM_SetSampleOffsetCallbackLinks(VOICE_REFERENCE reference, GM_SampleCallbackEntry *pTopEntry);
    void GM_AddSampleOffsetCallback(VOICE_REFERENCE reference, GM_SampleCallbackEntry *pEntry);
    void GM_RemoveSampleOffsetCallback(VOICE_REFERENCE reference, GM_SampleCallbackEntry *pEntry);

    // This will end all sound effects from the system.  It does not shut down sound hardware
    // or deallocate memory used by the music system.

    void GM_EndAllSamples(void *threadContext);     // stop samples from current thread
    void GM_ReleaseAllSamples(void *threadContext); // stop samples from main audio thread

    void GM_EndAllNotes(void);
    void GM_KillAllNotes(void);

    void GM_EndSongNotes(GM_Song *pSong);
    void GM_KillSongNotes(GM_Song *pSong);
    // stop notes for a song and channel passed. This will put the note into release mode.
    void GM_EndSongChannelNotes(GM_Song *pSong, short int channel);

    /**************************************************/
    /*
    ** FUNCTION SetMasterSongTempo(INT32 newTempo);
    **
    ** Overvue --
    **	This will set the master tempo for the currently playing song.
    **
    **	INPUT	--	newTempo is in microseconds per MIDI quater-note.
    **				Another way of looking at it is, 24ths of a microsecond per
    **				MIDI clock.
    **	OUTPUT	--	NO_SONG_PLAYING is returned if there is no song playing
    **
    ** NOTE:
    */
    /**************************************************/
    OPErr GM_SetMasterSongTempo(GM_Song *pSong, XFIXED newTempo);
    XFIXED GM_GetMasterSongTempo(GM_Song *pSong);

    // Sets tempo in microsecond per quarter note
    void GM_SetSongTempo(GM_Song *pSong, UINT32 newTempo);
    // returns tempo in microsecond per quarter note
    UINT32 GM_GetSongTempo(GM_Song *pSong);

    // sets tempo in beats per minute
    void GM_SetSongTempInBeatsPerMinute(GM_Song *pSong, UINT32 newTempoBPM);
    // returns tempo in beats per minute
    UINT32 GM_GetSongTempoInBeatsPerMinute(GM_Song *pSong);

    // Instrument API
    OPErr	GM_LoadInstrument(GM_Song *pSong, XLongResourceID instrument);
    OPErr	GM_LoadInstrumentFromExternalData(GM_Song *pSong, XLongResourceID instrument,
						  void *theX, UINT32 theXPatchSize);
    XBOOL	GM_AnyStereoInstrumentsLoaded(GM_Song *pSong);


    //	Can return STILL_PLAYING if instrument fails to unload
    OPErr	GM_UnloadInstrument(GM_Song *pSong, XLongResourceID instrument);
    OPErr	GM_RemapInstrument(GM_Song *pSong, XLongResourceID from, XLongResourceID to);

    // Pass TRUE to cache samples, and share them. FALSE to create new copy for each sample
    void GM_SetCacheSamples(GM_Song *pSong, XBOOL cacheSamples);
    XBOOL GM_GetCacheSamples(GM_Song *pSong);

    // returns TRUE if instrument is loaded, FALSE if otherwise
    XBOOL	GM_IsInstrumentLoaded(GM_Song *pSong, XLongResourceID instrument);

    /**************************************************/
    /*
    ** FUNCTION GM_LoadSongInstruments(GM_Song *theSong)
    **
    ** Overvue --
    **	This will load the instruments required for this song to play
    **
    **	INPUT	--	theSong,	a pointer to the GM_Song data containing the MIDI Data
    **			--	pArray,	an array that will be filled with the instruments that are
    **						loaded, if not NULL.
    **			--	loadInstruments, if TRUE will load instruments and samples
    **
    **	OUTPUT	--	OPErr,		an error will be returned if the song
    **							cannot be loaded
    **
    ** NOTE:
    */
    /**************************************************/
    OPErr	GM_LoadSongInstruments(GM_Song *theSong, XShortResourceID *pArray, XBOOL loadInstruments);

    // can return STILL_PLAYING if instruments are still in process. Call again to clear
    OPErr	GM_UnloadSongInstruments(GM_Song *theSong);

    OPErr	GM_LoadSongInstrument(GM_Song *pSong, XLongResourceID instrument);
    // can return STILL_PLAYING if instruments are still in process. Call again to clear
    OPErr	GM_UnloadSongInstrument(GM_Song *pSong, XLongResourceID instrument);
    // Returns TRUE if instrument is loaded, otherwise FALSE
    XBOOL GM_IsSongInstrumentLoaded(GM_Song *pSong, XLongResourceID instrument);

    void	GM_SetSongLoopFlag(GM_Song *theSong, XBOOL loopSong);
    XBOOL GM_GetSongLoopFlag(GM_Song *theSong);

    short int GM_GetSongLoopMax(GM_Song *theSong);
    void GM_SetSongLoopMax(GM_Song *theSong, short int maxLoopCount);

    short int GM_GetChannelVolume(GM_Song *theSong, short int channel);
    void GM_SetChannelVolume(GM_Song *theSong, short int channel, short int volume, XBOOL updateNow);

    // set reverb of a channel of a current song. If updateNow is active and the song is playing
    // the voice will up updated
    void	GM_SetChannelReverb(GM_Song *theSong, short int channel, UBYTE reverbAmount, XBOOL updateNow);

    // Given a song and a channel, this will return the current reverb level
    short int GM_GetChannelReverb(GM_Song *theSong, short int channel);

    // Given a song and a new volume set/return the master volume of the song
    // Range is 0 to 127. You can overdrive
    void	GM_SetSongVolume(GM_Song *theSong, INT16 newVolume);
    INT16	GM_GetSongVolume(GM_Song *theSong);

    // set/get song position. Range is MAX_PAN_LEFT to MAX_PAN_RIGHT
    void GM_SetSongStereoPosition(GM_Song *theSong, INT16 newStereoPosition);
    INT16 GM_GetSetStereoPosition(GM_Song *theSong);

    void	GM_SetSongFadeRate(GM_Song *pSong, XFIXED fadeRate,
				   INT16 minVolume, INT16 maxVolume, XBOOL endSong);


    // range is 0 to MAX_MASTER_VOLUME (256)
    void	GM_SetEffectsVolume(INT16 newVolume);
    INT16	GM_GetEffectsVolume(void);

    XBOOL GM_IsInstrumentRangeUsed(GM_Song *pSong, XLongResourceID thePatch, INT16 theLowKey, INT16 theHighKey);
    XBOOL GM_IsInstrumentUsed(GM_Song *pSong, XLongResourceID thePatch, INT16 theKey);
    void GM_SetUsedInstrument(GM_Song *pSong, XLongResourceID thePatch, INT16 theKey, XBOOL used);
    void GM_SetInstrumentUsedRange(GM_Song *pSong, XLongResourceID thePatch, SBYTE *pUsedArray);
    void GM_GetInstrumentUsedRange(GM_Song *pSong, XLongResourceID thePatch, SBYTE *pUsedArray);

    void GM_SetSongCallback(GM_Song *theSong, GM_SongCallbackProcPtr songEndCallbackPtr);
    void GM_SetSongTimeCallback(GM_Song *theSong, GM_SongTimeCallbackProcPtr songTimeCallbackPtr, INT32 reference);
    void GM_SetSongMetaEventCallback(GM_Song *theSong, GM_SongMetaCallbackProcPtr theCallback, INT32 reference);

    void GM_SetControllerCallback(GM_Song *theSong, void * reference, GM_ControlerCallbackPtr controllerCallback, short int controller);

    // Display
    INT16 GM_GetAudioSampleFrame(INT16 *pLeft, INT16 *pRight);

    typedef enum
    {
	MIDI_PCM_VOICE = 0,		// Voice is a PCM voice used by MIDI
	SOUND_PCM_VOICE			// Voice is a PCM sound effect used by HAESound/HAESoundStream
    } GM_VoiceType;


    struct GM_AudioInfo
    {
	INT16			maxNotesAllocated;
	INT16			maxEffectsAllocated;
	INT16			mixLevelAllocated;
	INT16			voicesActive;				// number of voices active
	INT16			patch[MAX_VOICES];			// current patches
	INT16			volume[MAX_VOICES];			// current volumes
	INT16			scaledVolume[MAX_VOICES];	// current scaled volumes
	INT16			channel[MAX_VOICES];		// current channel
	INT16			midiNote[MAX_VOICES];		// current midi note
	INT16			voice[MAX_VOICES];			// voice index
	GM_VoiceType	voiceType[MAX_VOICES];		// voice type
	GM_Song			*pSong[MAX_VOICES];			// song associated with voice
    };
    typedef struct GM_AudioInfo GM_AudioInfo;

    void GM_GetRealtimeAudioInformation(GM_AudioInfo *pInfo);

    // External MIDI links
#define Q_GET_TICK	0L		// if you pass this constant for timeStamp it will get the current
    // tick
    void QGM_NoteOn(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 note, INT16 velocity);
    void QGM_NoteOff(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 note, INT16 velocity);
    void QGM_ProgramChange(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 program);
    void QGM_PitchBend(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, UBYTE valueMSB, UBYTE valueLSB);
    void QGM_Controller(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 controller, INT16 value);
    void QGM_AllNotesOff(void *threadContext, GM_Song *pSong, UINT32 timeStamp);
    void QGM_LockExternalMidiQueue(void);
    void QGM_UnlockExternalMidiQueue(void);
    void QGM_ClearSongFromQueue(GM_Song* pSong);

    // External MIDI Links using a GM_Song. Will be unstable unless called via GM_AudioTaskCallbackPtr
    void GM_NoteOn(void *threadContext, GM_Song *pSong, INT16 channel, INT16 note, INT16 velocity);
    void GM_NoteOff(void *threadContext, GM_Song *pSong, INT16 channel, INT16 note, INT16 velocity);
    void GM_ProgramChange(void *threadContext, GM_Song *pSong, INT16 channel, INT16 program);
    void GM_PitchBend(void *threadContext, GM_Song *pSong, INT16 channel, UBYTE valueMSB, UBYTE valueLSB);
    void GM_Controller(void *threadContext, GM_Song *pSong, INT16 channel, INT16 controller, INT16 value);
    void GM_AllNotesOff(void *threadContext, GM_Song *pSong);

    // mixer callbacks and tasks
    void GM_SetAudioTask(GM_AudioTaskCallbackPtr pTaskProc);
    void GM_SetAudioOutput(GM_AudioOutputCallbackPtr pOutputProc);
    GM_AudioOutputCallbackPtr GM_GetAudioOutput(void);
    GM_AudioTaskCallbackPtr GM_GetAudioTask(void);

    INT32 GM_GetAudioBufferOutputSize(void);

#if USE_HIGHLEVEL_FILE_API == TRUE
    typedef enum
    {
	FILE_INVALID_TYPE = 0,
	FILE_AIFF_TYPE = 1,
	FILE_WAVE_TYPE,
	FILE_AU_TYPE
#if USE_MPEG_DECODER != FALSE
	,
	FILE_MPEG_TYPE
#endif
    } AudioFileType;

    // This will read into memory the entire file and return a GM_Waveform structure.
    // To dispose of a GM_Waveform structure, call GM_FreeWaveform
    GM_Waveform * GM_ReadFileIntoMemory(XFILENAME *file, AudioFileType fileType, OPErr *pErr);

    // This will read from a block of memory an entire file and return a GM_Waveform structure.
    // Assumes that the block of memory is formatted as a fileType
    // To dispose of a GM_Waveform structure, call GM_FreeWaveform
    GM_Waveform * GM_ReadFileIntoMemoryFromMemory(void *pFileBlock, UINT32 fileBlockSize,
						  AudioFileType fileType, OPErr *pErr);

    // This will read into memory just the information about the file and return a
    // GM_Waveform structure.
    // Read file information from file, which is a fileType file. If pFormat is not NULL, then
    // store format specific format type
    // To dispose of a GM_Waveform structure, call GM_FreeWaveform
    // If pBlockPtr is a void *, then allocate a *pBlockSize buffer, otherwise do nothing
    GM_Waveform * GM_ReadFileInformation(XFILENAME *file, AudioFileType fileType, INT32 *pFormat,
					 void **pBlockPtr, UINT32 *pBlockSize, OPErr *pErr);

    // Read a block of data, based apon file type and format, decode and store into a buffer.
    // Return length of buffer stored and return an OPErr. NO_ERR if successfull.
    OPErr GM_ReadAndDecodeFileStream(XFILE fileReference,
				     AudioFileType fileType, INT32 format,
				     XPTR pBlockBuffer, UINT32 blockSize,
				     XPTR pBuffer, UINT32 bufferLength,
				     short int channels, short int bitSize,
				     UINT32 *pStoredBufferLength,
				     UINT32 *pReadBufferLength);

#endif	// USE_HIGHLEVEL_FILE_API

#if USE_STREAM_API == TRUE
    // Audio Sample Data Format. (ASDF)
    // Support for 8, 16 bit data, mono and stereo. Can be extended for multi channel beyond 2 channels, but
    // not required at the moment.
    //
    //	DATA BLOCK
    //		8 bit mono data
    //			ZZZZZZZ...
    //				Where Z is signed 8 bit data
    //
    //		16 bit mono data
    //			WWWWW...
    //				Where W is signed 16 bit data
    //
    //		8 bit stereo data
    //			ZXZXZXZX...
    //				Where Z is signed 8 bit data for left channel, and X is signed 8 bit data for right channel.
    //
    //		16 bit stereo data
    //			WQWQWQ...
    //				Where W is signed 16 bit data for left channel, and Q is signed 16 bit data for right channel.
    //



    typedef enum
    {
	STREAM_CREATE				=	1,
	STREAM_DESTROY,
	STREAM_GET_DATA,
	STREAM_GET_SPECIFIC_DATA,
	STREAM_HAVE_DATA,

	// $$kk: 09.21.98: i am adding these
	STREAM_START,		// stream started in response to start or resume
	STREAM_STOP,		// stream stopped in response to pause
	STREAM_EOM,			// stream stopped / end of stream
	STREAM_ACTIVE,		// stream became active after underflow
	STREAM_INACTIVE		// stream became inactive due to underflow
    } GM_StreamMessage;

    // The GM_StreamObjectProc callback is called to allocate buffer memory, get the next block of data to stream and
    // mix into the final audio output, and finally dispose of the memory block. All messages will happen at
    // non-interrupt time. The structure GM_StreamData will be passed into all messages.
    //
    // INPUT:
    // Message
    //	STREAM_CREATE
    //		Use this message to create a block a data with a length of (dataLength). Keep in mind that dataLength
    //		is always total number of samples,  not bytes allocated. Allocate the block of data into the Audio Sample
    //		Data Format based upon (dataBitSize) and (channelSize). Store the pointer into (pData).
    //
    //	STREAM_DESTROY
    //		Use this message to dispose of the memory allocated. (pData) will contain the pointer allocated.
    //		(dataLength) will be the sample size not the buffer size. ie. for 8 bit data use (dataLength),
    //		for 16 bit mono data double (dataLength).
    //
    //	STREAM_GET_DATA
    //		This message is called whenever the streaming object needs a new block of data. Right after STREAM_CREATE
    //		is called, STREAM_GET_DATA will be called twice. Fill (pData) with the new data to be streamed.
    //		Set (dataLength) to the amount of data put into (pData). This message is called in a linear fashion.
    //
    //	STREAM_GET_SPECIFIC_DATA
    //		This message is optional. It will be called when a specific sample frame and length needs to be captured.
    //		The function GM_AudioStreamGetData will call this message. If you do not want to implement this message
    //		return an error of NOT_SETUP. Fill (pData) with the new sample data betweem sample frames (startSample)
    //		and (endSample). Set (dataLength) to the amount of data put into (pData).
    //		Note: this message will should not be called to retrive sample data for streaming. Its only used to capture
    //		a range of data inside of a stream for display or other processes.
    //
    //
    // OUTPUT:
    // returns
    //	NO_ERR
    //		Everythings ok
    //
    //	STREAM_STOP_PLAY
    //		Everything is fine, but stop playing stream
    //
    //	MEMORY_ERR
    //		Couldn't allocate memory for buffers.
    //
    //	PARAM_ERR
    //		General purpose error. Something wrong with parameters passed.
    //
    //	NOT_SETUP
    //		If message STREAM_GET_SPECIFIC_DATA is called and it is not implemented you should return this error.
    //

#define DEAD_STREAM	0L				// this represents a dead or invalid stream
    typedef void *		STREAM_REFERENCE;
    typedef void *		LINKED_STREAM_REFERENCE;

    struct GM_StreamData
    {
	STREAM_REFERENCE	streamReference;	// IN for all messages
	void				*userReference;		// IN for all messages. userReference is passed in at AudioStreamStart
	void				*pData;				// OUT for STREAM_CREATE, IN for STREAM_DESTROY and STREAM_GET_DATA and STREAM_GET_SPECIFIC_DATA
	UINT32		dataLength;			// OUT for STREAM_CREATE, IN for STREAM_DESTROY. IN and OUT for STREAM_GET_DATA and STREAM_GET_SPECIFIC_DATA
	XFIXED				sampleRate;			// IN for all messages. Fixed 16.16 value
	char				dataBitSize;		// IN for STREAM_CREATE only.
	char				channelSize;		// IN for STREAM_CREATE only.
	/* $$fb 2002-02-07: itanium port */
	SAMPLE_COUNT		startSample;		// IN for STREAM_GET_SPECIFIC_DATA only.
	SAMPLE_COUNT		endSample;			// IN for STREAM_GET_SPECIFIC_DATA only
    };
    typedef struct GM_StreamData	GM_StreamData;


    // CALLBACK FUNCTIONS TYPES

    // AudioStream object callback
    typedef OPErr (*GM_StreamObjectProc)(void *threadContext, GM_StreamMessage message, GM_StreamData *pAS);



    // Multi source user config based streaming
    // This will setup a streaming audio object.
    //
    // INPUT:
    //	userReference	This is a reference value that will be returned and should be passed along to all AudioStream
    //					functions.
    //
    //	pProc			is a GM_StreamObjectProc proc pointer. At startup of the streaming the proc will be called
    //					with STREAM_CREATE, then followed by two STREAM_GET_DATA calls to get two buffers of data,
    //					and finally STREAM_DESTROY when finished.
    //
    // OUTPUT:
    //	long			This is an audio stream reference number. Will be 0 if error
    STREAM_REFERENCE	GM_AudioStreamSetup(	void *threadContext,					// platform threadContext
						void *userReference, 			// user reference
						GM_StreamObjectProc pProc, 		// control callback
						UINT32 bufferSize, 		// buffer size
						XFIXED sampleRate,			// Fixed 16.16
						char dataBitSize,				// 8 or 16 bit data
						char channelSize);				// 1 or 2 channels of date

#if USE_HIGHLEVEL_FILE_API
    // setup a streaming file
    // OUTPUT:
    //	long			This is an audio stream reference number. Will be 0 if error
    STREAM_REFERENCE	GM_AudioStreamFileSetup(	void *threadContext,				// platform threadContext
							XFILENAME *file,			// file name
							AudioFileType fileType, 	// type of file
							UINT32 bufferSize,	// temp buffer to read file
							GM_Waveform *pFileInfo,
							XBOOL loopFile);			// TRUE will loop file
#endif	// USE_HIGHLEVEL_FILE_API

    // return position of playback in samples. Only works for file based streaming
    SAMPLE_COUNT GM_AudioStreamGetFileSamplePosition(STREAM_REFERENCE reference);



    // $$kk: 07.01.99: added this method.
    // this method tries to pre-fill the stream's buffers
    // if they're empty.  my goal here is to allow us to
    // pre-prepare buffers for a stream that didn't get data
    // during the first two GET_DATA calls in GM_AudioStreamSetup,
    // but does need to pre-load before starting playback.
    // this also specifically should fix the problem i see
    // where GM_StartLinkedStreams fails for me because my
    // pStream->playbackReferences are DEAD_STREAM or whatever....
    OPErr		GM_AudioStreamPrebuffer(STREAM_REFERENCE reference, void *threadContext);



    // call this to preroll everything and allocate a voice in the mixer, then call
    // GM_AudioStreamStart to start it playing
    OPErr		GM_AudioStreamPreroll(STREAM_REFERENCE reference);
    // once a stream has been setup, call this function
    OPErr		GM_AudioStreamStart(STREAM_REFERENCE reference);

    // set all the streams you want to start at the same time the same syncReference. Then call GM_SyncAudioStreamStart
    // to start the sync start. Will return an error (not NO_ERR) if its an invalid reference, or syncReference is NULL.
    OPErr		GM_SetSyncAudioStreamReference(STREAM_REFERENCE reference, void *syncReference);

    // Once you have called GM_SetSyncAudioStreamReference on all the streams, this will set them to start at the next
    // mixer slice. Will return an error (not NO_ERR) if its an invalid reference, or syncReference is NULL.
    OPErr		GM_SyncAudioStreamStart(STREAM_REFERENCE reference);

    void *		GM_AudioStreamGetReference(STREAM_REFERENCE reference);

    OPErr		GM_AudioStreamGetData(	void *threadContext,					// platform threadContext
						STREAM_REFERENCE reference,
						UINT32 startFrame,
						UINT32 stopFrame,
						XPTR pBuffer, UINT32 bufferLength);

    // This will stop a streaming audio object and free any memory.
    //
    // INPUT:
    //	reference	This is the reference number returned from AudioStreamStart.
    //
    OPErr		GM_AudioStreamStop(void *threadContext, STREAM_REFERENCE reference);

    // This will return the last error of this stream
    OPErr		GM_AudioStreamError(STREAM_REFERENCE reference);

    // This will stop all streams that are current playing and free any memory.
    void		GM_AudioStreamStopAll(void *threadContext);

    // This is the streaming audio service routine. Call this as much as possible, but not during an
    // interrupt. This is a very quick routine. A good place to call this is in your main event loop.
    void		GM_AudioStreamService(void *threadContext);

    // Returns TRUE or FALSE if a given AudioStream is still active
    XBOOL	GM_IsAudioStreamPlaying(STREAM_REFERENCE reference);

    // Returns TRUE if a given AudioStream is valid
    XBOOL	GM_IsAudioStreamValid(STREAM_REFERENCE reference);

    // Set the volume level of a audio stream
    void		GM_AudioStreamSetVolume(STREAM_REFERENCE reference, short int newVolume, XBOOL defer);

    // set the volume level of all open streams
    void		GM_AudioStreamSetVolumeAll(short int newVolume);

    // Get the volume level of a audio stream
    short int	GM_AudioStreamGetVolume(STREAM_REFERENCE reference);

    // start a stream fading
    void GM_SetAudioStreamFadeRate(STREAM_REFERENCE reference, XFIXED fadeRate,
				   INT16 minVolume, INT16 maxVolume, XBOOL endStream);

    // Set the sample rate of a audio stream
    void		GM_AudioStreamSetRate(STREAM_REFERENCE reference, XFIXED newRate);

    // Get the sample rate of a audio stream
    XFIXED		GM_AudioStreamGetRate(STREAM_REFERENCE reference);

    // Set the stereo position of a audio stream
    void		GM_AudioStreamSetStereoPosition(STREAM_REFERENCE reference, short int stereoPosition);

    // Get the stereo position of a audio stream
    short int	GM_AudioStreamGetStereoPosition(STREAM_REFERENCE reference);

    // Get the offset, in mixer-format samples, between the mixer play
    // position and the stream play position.
    SAMPLE_COUNT GM_AudioStreamGetSampleOffset(STREAM_REFERENCE reference);

    // Update the number of samples played from each stream, in the
    // format of the stream.  delta is given in engine-format samples.
    void        GM_AudioStreamUpdateSamplesPlayed(UINT32 delta);

    // get number of samples played for this stream
    SAMPLE_COUNT GM_AudioStreamGetSamplesPlayed(STREAM_REFERENCE reference);

    // $$kk: 08.12.98 merge: added this
    // Drain this stream
    void GM_AudioStreamDrain(void *threadContext, STREAM_REFERENCE reference);

    // Get the number of samples actually played through the device
    // from this stream, in stream-format samples.
    // Flush this stream
    void		GM_AudioStreamFlush(STREAM_REFERENCE reference);

    // Enable/Disable reverb on this particular audio stream
    void		GM_AudioStreamReverb(STREAM_REFERENCE reference, XBOOL useReverb);
    XBOOL		GM_AudioStreamGetReverb(STREAM_REFERENCE reference);
    // get/set reverb mix level
    void		GM_SetStreamReverbAmount(STREAM_REFERENCE reference, short int reverbAmount);
    short int	GM_GetStreamReverbAmount(STREAM_REFERENCE reference);

    // get/set filter frequency of a audio stream
    // Range is 512 to 32512
    void		GM_AudioStreamSetFrequencyFilter(STREAM_REFERENCE reference, short int frequency);
    short int	GM_AudioStreamGetFrequencyFilter(STREAM_REFERENCE reference);
    // get/set filter resonance of a audio stream
    // Range is 0 to 256
    short int	GM_AudioStreamGetResonanceFilter(STREAM_REFERENCE reference);
    void		GM_AudioStreamSetResonanceFilter(STREAM_REFERENCE reference, short int resonance);
    // get/set filter low pass amount of a audio stream
    // lowPassAmount range is -255 to 255
    short int	GM_AudioStreamGetLowPassAmountFilter(STREAM_REFERENCE reference);
    void		GM_AudioStreamSetLowPassAmountFilter(STREAM_REFERENCE reference, short int lowPassAmount);


    // Pause or resume this particular audio stream
    void		GM_AudioStreamResume(STREAM_REFERENCE reference);
    void		GM_AudioStreamPause(STREAM_REFERENCE reference);

    // Pause or resume all audio streams
    void		GM_AudioStreamResumeAll(void);
    void		GM_AudioStreamPauseAll(void);

    LINKED_STREAM_REFERENCE GM_NewLinkedStreamList(STREAM_REFERENCE reference, void *threadContext);

    // Given a top link, deallocates the linked list. DOES NOT deallocate the streams.
    void GM_FreeLinkedStreamList(LINKED_STREAM_REFERENCE pTop);

    // Given a top link, and a new link this will add to a linked list, and return a new top
    // if required.
    LINKED_STREAM_REFERENCE GM_AddLinkedStream(LINKED_STREAM_REFERENCE pTop, LINKED_STREAM_REFERENCE pEntry);

    // Given a top link and an link to remove this will disconnect the link from the list and
    // return a new top if required.
    LINKED_STREAM_REFERENCE GM_RemoveLinkedStream(LINKED_STREAM_REFERENCE pTop, LINKED_STREAM_REFERENCE pEntry);

    STREAM_REFERENCE GM_GetLinkedStreamPlaybackReference(LINKED_STREAM_REFERENCE pLink);
    void * GM_GetLinkedStreamThreadContext(LINKED_STREAM_REFERENCE pLink);

    OPErr GM_StartLinkedStreams(LINKED_STREAM_REFERENCE pTop);

    // end in unison the samples for all the linked streams
    void GM_EndLinkedStreams(LINKED_STREAM_REFERENCE pTop);

    // Volume range is from 0 to MAX_NOTE_VOLUME
    // set in unison the sample volume for all the linked streams
    void GM_SetLinkedStreamVolume(LINKED_STREAM_REFERENCE pTop, INT16 sampleVolume, XBOOL defer);

    // set in unison the sample rate for all the linked streams
    void GM_SetLinkedStreamRate(LINKED_STREAM_REFERENCE pTop, XFIXED theNewRate);

    // set in unison the sample position for all the linked streams
    // range from -63 to 63
    void GM_SetLinkedStreamPosition(LINKED_STREAM_REFERENCE pTop, INT16 newStereoPosition);

#endif	// USE_STREAM_API

#if USE_CAPTURE_API == TRUE
    // Multi source user config based streaming
    // This will setup a streaming audio object.
    //
    // INPUT:
    //	userReference	This is a reference value that will be returned and should be passed along to all AudioStream
    //					functions.
    //
    //	pProc			is a GM_StreamObjectProc proc pointer. At startup of the streaming the proc will be called
    //					with STREAM_CREATE, then followed by two STREAM_GET_DATA calls to get two buffers of data,
    //					and finally STREAM_DESTROY when finished.
    //
    // OUTPUT:
    //	long			This is an audio stream reference number. Will be 0 if error
    void *		GM_AudioCaptureStreamSetup(	void *threadContext,				// platform threadContext
							void *userReference, 			// user reference
							GM_StreamObjectProc pProc, 		// control callback
							UINT32 bufferSize, 		// buffer size
							XFIXED sampleRate,				// Fixed 16.16
							char dataBitSize,				// 8 or 16 bit data
							char channelSize,				// 1 or 2 channels of date
							OPErr *pErr);

    OPErr		GM_AudioCaptureStreamCleanup(void *threadContext, void *reference);

    // once a stream has been setup, call this function
    OPErr		GM_AudioCaptureStreamStart(void *threadContext, void *reference);

    OPErr		GM_AudioCaptureStreamStop(void* threadContext, void *reference);

    void *		GM_AudioCaptureStreamGetReference(void *reference);

    /*
      // Pause or resume this particular audio stream
      OPErr		GM_AudioCaptureStreamResume(long reference);
      OPErr		GM_AudioCaptureStreamPause(long reference);
    */

    // Returns TRUE if a given AudioStream is valid
    XBOOL		GM_IsAudioCaptureStreamValid(void *reference);

    // Get the count of samples captured to this stream.
    UINT32 GM_AudioCaptureStreamGetSamplesCaptured(void *reference);

#endif	// USE_CAPTURE_API

#if USE_MOD_API

    typedef void		(*GM_ModDoneCallbackPtr)(struct GM_ModData *pMod);

    struct GM_ModData
    {
	void					*modControl;
	INT16					modVolume;

	XFIXED					modFadeRate;		// when non-zero fading is enabled
	XFIXED					modFixedVolume;		// inital volume level that will be changed by modFadeRate
	INT16					modFadeMaxVolume;	// max volume
	INT16					modFadeMinVolume;	// min volume
	XBOOL					modEndAtFade;
	XBOOL					enableReverb;

	void					*reference;
	void					*reference2;
	GM_ModDoneCallbackPtr	callback;
    };
    typedef struct GM_ModData GM_ModData;


    GM_ModData		*GM_LoadModFile(void *pModFile, INT32 fileSize);
    void			GM_FreeModFile(GM_ModData *modReference);
    void			GM_BeginModFile(GM_ModData *modReference, GM_ModDoneCallbackPtr callback, void *reference);
    void			GM_StopModFile(GM_ModData *modReference);

    // volue max is MAX_SONG_VOLUME. You can overdrive
    void			GM_SetModVolume(GM_ModData *modReference, INT16 volume);
    short int		GM_GetModVolume(GM_ModData *modReference);
    // start volume fade. For every 1.0 of fadeRate, volume will change by 11 ms
    void			GM_SetModFadeRate(GM_ModData *modReference, XFIXED fadeRate,
						  INT16 minVolume, INT16 maxVolume, XBOOL endSong);

    void			GM_SetModReverbStatus(GM_ModData *mod, XBOOL enableReverb);

    XBOOL			GM_IsModPlaying(GM_ModData *modReference);
    void			GM_ResumeMod(GM_ModData *modReference);
    void			GM_PauseMod(GM_ModData *modReference);
    void			GM_SetModTempoFactor(GM_ModData *modReference, UINT32 fixedFactor);
    void			GM_SetModLoop(GM_ModData *modReference, XBOOL loop);
    XBOOL			GM_GetModLoop(GM_ModData *mod);
    void			GM_SetModTempoBPM(GM_ModData *modReference, UINT32 newTempoBPM);
    UINT32	GM_GetModTempoBPM(GM_ModData *modReference);
    void			GM_GetModSongName(GM_ModData *mod, char *cName);
    UINT32	GM_GetModSongNameLength(GM_ModData *mod);
    void			GM_GetModSongComments(GM_ModData *mod, char *cName);
    UINT32	GM_GetModSongCommentsLength(GM_ModData *mod);
#endif	// USE_MOD_API

    // linked samples
    // Call one of the GM_SetupSample... functions in the various standard ways, to get an allocate voice
    // then call GM_NewLinkedSampleList. Then add it to your maintained top list of linked voices with
    // by calling GM_AddLinkedSample. Use GM_FreeLinkedSampleList to delete an entire list,
    // or GM_RemoveLinkedSample to just one link.
    //
    // Then you can call GM_StartLinkedSamples to start them all at the same time, or GM_EndLinkedSamples
    // to end them all, or GM_SetLinkedSampleVolume, GM_SetLinkedSampleRate, and GM_SetLinkedSamplePosition
    // set parameters about them all.

    LINKED_VOICE_REFERENCE GM_NewLinkedSampleList(VOICE_REFERENCE reference);

    void GM_FreeLinkedSampleList(LINKED_VOICE_REFERENCE reference);

    // Given a top link, and a new link this will add to a linked list, and return a new top
    // if required.
    LINKED_VOICE_REFERENCE GM_AddLinkedSample(LINKED_VOICE_REFERENCE pTop, LINKED_VOICE_REFERENCE pEntry);

    // Given a top link and an link to remove this will disconnect the link from the list and
    // return a new top if required.
    LINKED_VOICE_REFERENCE GM_RemoveLinkedSample(LINKED_VOICE_REFERENCE pTop, LINKED_VOICE_REFERENCE pEntry);

    VOICE_REFERENCE GM_GetLinkedSamplePlaybackReference(LINKED_VOICE_REFERENCE pLink);

    OPErr GM_StartLinkedSamples(LINKED_VOICE_REFERENCE reference);

    // end in unison the samples for all the linked samples
    // $$kk: 04.19.99
    //void GM_EndLinkedSamples(LINKED_VOICE_REFERENCE reference);
    void GM_EndLinkedSamples(LINKED_VOICE_REFERENCE reference, void *threadContext);

    // Volume range is from 0 to MAX_NOTE_VOLUME
    // set in unison the sample volume for all the linked samples
    void GM_SetLinkedSampleVolume(LINKED_VOICE_REFERENCE reference, INT16 sampleVolume);

    // set in unison the sample rate for all the linked samples
    void GM_SetLinkedSampleRate(LINKED_VOICE_REFERENCE reference, XFIXED theNewRate);

    // set in unison the sample position for all the linked samples
    // range from -63 to 63
    void GM_SetLinkedSamplePosition(LINKED_VOICE_REFERENCE reference, INT16 newStereoPosition);

    // Get current status of resampled interpolation. On or off
    XBOOL GM_GetSampleResample(VOICE_REFERENCE reference);

    // change activation of resampler. Force on, or off
    void GM_SetSampleResample(VOICE_REFERENCE reference, XBOOL enable);
    void GM_SetSampleResampleFromVoice(void* voice, XBOOL enable);

    // change to resampler that is initialized externally. 
    // This voice will not dispose resampleParams
    void GM_SetSampleResampleExtern(VOICE_REFERENCE reference, void* resampleParams);
    // remove a reference to an externally allocated resampler
    // This will even work on UNUSED samples.
    // if oldResampleParams is != NULL, then the reference is only removed if
    // the reference is equal to oldResampleParams. This prevents accidental overwriting
    // of a voice that has been already re-initialized.
    void GM_RemoveSampleResampleExtern(VOICE_REFERENCE reference, void* oldResampleParams);


#ifdef __cplusplus
}
#endif

#endif /* GenSnd.h */



