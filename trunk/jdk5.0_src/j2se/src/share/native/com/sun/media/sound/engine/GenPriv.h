/*
 * @(#)GenPriv.h	1.38 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
** "GenPriv.h"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Overview
**	Private structures
**
** Modification History
**
**	11/7/95		Major changes, revised just about everything.
**	11/11/95	Added microSyncCount for live link
**	11/16/95	Removed microSyncCount
**				Moved static variables into MusicVar structure
**				Created an external function 'PV_GetExternalTimeSync()' for the external midi source
**   12/95 		upgraded mixing bus to 32 bit; improved scaleback resolution; added reverb unit; first pass at volume ADSR
**	12/6/95 	removed reference to USE_AMP_LOOKUP
**				moved REVERB_TYPE to GENSND.H
**	12/7/95		Added channelReverb to GM_Mixer structure
**				Added REVERB_CONTROLER_THRESHOLD
**	1/18/96		Spruced up for C++ extra error checking
**				Changed InstUsedList to pUsedList and allocate it when needed
**	2/5/96		Removed unused variables. Working towards multiple songs
**				Moved lots of variables from the GM_Mixer structure into
**				Moved the MAX_TRACKS define to GenSnd.h
**	2/12/96		Added PV_CleanExternalQueue
**				Moved SongMicroseconds to GenSnd.h
**	2/13/96		Added multi song support
**	3/5/96		Eliminated the global songVolume
**	3/28/96		Added PV_SetSampleIntoCache & PV_GetInstrument
**	4/10/96		Reworked the sample cache system to not clone the sample data
**	5/2/96		Changed int to BOOL_FLAG
**	5/18/96		Added error condition to PV_MusicIRQ
**	6/30/96		Changed font and re tabbed
**	7/3/96		Added packing pragmas
**				Removed usage of Machine.h. Now merged into X_API.h
**	7/14/96		Fixed structure alignment issue for PowerPC
**	7/23/96		Changed PV_GetExternalTimeSync to unsigned long
**	7/24/96		Changed Midi Queue system to use a head/tail
**	7/25/96		Moved Mac audio variables to GenMacTools.c
**				Changed PV_GetExternalTimeSync to GM_GetSyncTimeStampQuantizedAhead
**	8/12/96		Changed PV_ResetControlers to support semi-complete reset
**	9/25/96		Added GM_Song pointer in NoteRecord structure
**	9/27/96		Added more parameters to ServeMIDINote & StopMIDINote
**	10/18/96	Made CacheSampleInfo smaller
**	10/23/96	Removed reference to BYTE and changed them all to UBYTE or SBYTE
**	12/19/96	Added Sparc pragmas
**	12/30/96	Changed copyrights
**	1/23/97		Added support for stereoFilter
**				In NoteRecord changed PitchBend to NotePitchBend
**				Added in NoteRecord NoteFadeRate
**	1/30/97		Changed SYMPHONY_SIZE to MAX_VOICES
**	3/17/97		Changed the API to PV_GetInstrument. Enlarged CacheSampleInfo member
**				theID to a long
**	4/9/97		Added sampleExpansion factor
**	4/20/97		Changed PV_MusicIRQ to PV_ProcessMidiSequencerSlice
**	6/4/97		Renamed InitSoundManager to GM_StartHardwareSoundManager, and
**				renamed FinsSoundManager to GM_StopHardwareSoundManager, and
**				now pass in a thread context
**	712/97		Added a drift fixer flag to GM_Mixer that tries
**				to compensate for real time midi time stamping
**	7/16/97		Moved GM_Mixer *MusicGlobals to be protected againsts C++
**				name mangling
**	7/17/97		Aligned GM_Mixer structure to 8 bytes
**	7/18/97		Moved GM_AudioTaskCallbackPtr pTaskProc &
**				GM_AudioOutputCallbackPtr pOutputProc into here from GenXXXTools.c
**	7/22/97		Changed SYNC_BUFFER_TIME to BUFFER_SLICE_TIME
**	7/28/97		Changed pack structure alignment for SPARC from 8 to 4. Compiler bug.
**	8/8/97		Added PV_FreePgmEntries
**	8/27/97		Moved GM_StartHardwareSoundManager & GM_StopHardwareSoundManager to
**				GenSnd.h
**	9/2/97		Fixed bug with THE_CHECK that forgot to look at zero length buffers
**	9/19/97		Changed name of PV_FreePatchInfo.
**				Added PV_InsertBankSelect
**	10/15/97	Added processingSlice to NoteRecord to handle threading issues
**	10/27/97	Removed reference to MusicGlobals->theSongPlaying
**	10/28/97	Eliminated reference to FAR
**	10/29/97	Promoted PV_AnyStereoInstrumentsLoaded to GM_AnyStereoInstrumentsLoaded and
**				moved it to GenSnd.h
**	12/4/97		Renamed GM_Mixer to GM_Mixer. Renamed NoteRecord to GM_Voice
**	1/14/98		kk: added NoteLoopTarget to GM_Voice (number of loops between loop points desired
**				before sample continues to end.
**				changed NoteLoopCount from UBYTE to UINT32 because we are actually counting loops now and  
**				may want quite a few.
**	1/27/98		Renamed MACINTOSH to H_MACINTOSH
**	2/3/98		Renamed songBufferLeftMono to songBufferDry
**	2/5/98		Added a GM_Song pointer to PV_SetSampleIntoCache
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/10/98		added a bunch of structures for storing new effect parameters
**	2/20/98		kcr	converted floating-point to fixed for new effects -- added chorus buffer
**	2/23/98		Removed last of old variable reverb code
**	2/24/98		kcr	deal with sample-rate changes for chorus and reverb...
**	3/16/98		Removed PV_ProcessReverbMono & PV_ProcessReverbStereo & PV_PostFilterStereo
**				from public view
**				Changed InitNewReverb to return a XBOOL for success or failure
**	4/1/98		MOE: took out references to FilterEnvelope{} so that all compiles
**	4/14/98		Added some comments and removed extra structures that are not being used
**	7/1/98		Changed various API to use the new XResourceType and XLongResourceID
**	7/7/98		Removed reverbIsVariable from GM_Mixer structure. Using function 
**				GM_IsReverbFixed instead.
**	7/28/98		Renamed inst_struct to pInstrument
**				Changed meaning of processExternalMidiQueue in GM_Song. Now its a counter
**				instead of just a boolean.
**	7/30/98		Added constant value to MAX_CHUNK_SIZE for 48k output in GM_Mixer structure
**	8/12/98		Added PV_ModifyVelocityFromCurve
**	10/27/98	Moved MIN_LOOP_SIZE to GenSnd.h
**	11/9/98		Renamed NoteDur to voiceMode
**	12/22/98	Removed old USE_SEQUENCER flag
**	1/12/99		Added a useKatmaiCPU flag that is dynamic if the USE_KAT flag
**				is set to build.
**	3/1/99		Changed NoteRefNum to NoteContext
**	3/3/99		Added PV_GetVoiceFromSoundReference
**				Added voiceStartTimeStamp to GM_Voice
**				Removed USE_DIRECT_MIXDOWN
**	3/5/99		Added VOICE_ALLOCATED_READY_TO_SYNC_START
**				Added threadContext to PV_ServeEffectCallbacks & PV_ProcessSampleFrame &
**				PV_ProcessSequencerEvents & PV_ProcessMidiSequencerSlice
**      2002-01-06      $$fb added resample algorithms
**	2002-03-14	$$fb cleaned up pragma code, included Itanium and sparcv9 architecture support
**	2003-08-21	$$fb for amd64, cleaner pragma code
*/
/*****************************************************************************/

#ifndef G_PRIVATE
#define G_PRIVATE

#ifndef __X_API__
#include "X_API.h"
#endif

#ifndef G_SOUND
#include "GenSnd.h"
#endif

/* $$fb 2002-01-06 added resample algorithms */
#include "SincResample.h"


#define VOLUME_PRECISION_SCALAR		6L		// used to be 8, so we must scale down output by 2
#define OUTPUT_SCALAR				9L		// 9 for volume minus 4 for increased volume_range resolution, plus 2 for increased volume precision scalar
#define VOLUME_RANGE				4096	// original range was 256, therefore:
#define UPSCALAR					16L		// multiplier (NOT a shift count!) for increasing amplitude resolution
#define MAXRESONANCE				127		// mask and buffer size for resonant filter.  Higher means wider frequency range.


#define MAX_CHUNK_SIZE				512		// max samples to build per slice at 44k
#define SOUND_EFFECT_CHANNEL		16		// channel used for sound effects. One beyond the normal

// BUFFER_SLICE_TIME is calculated by the formula:
//
// 1 second / sample rate * samples
// 1 000 000 / 22050 * 256
//
#define BUFFER_SLICE_TIME			11610	// the amount of time in microseconds that
// passes when calling ProcessSampleFrame

#define REVERB_CONTROLER_THRESHOLD	13	// past this value via controlers, reverb is enabled. only for fixed reverb
#define DEFAULT_REVERB_LEVEL		40	// value used for reverb-enabled instruments
#define DEFAULT_CHORUS_LEVEL		0	// value used for chorus-enabled instruments

// 20.12 (whole.fractional)
#define STEP_BIT_RANGE				12L
#define STEP_OVERFLOW_FLAG			(1<<(STEP_BIT_RANGE-1))		
#define STEP_FULL_RANGE				((1<<STEP_BIT_RANGE)-1)

#if (MAX_CHUNK_SIZE%4) != 0
#error "Bad Chunk Size, Divisible by 4 only!" 
#endif

// a macro to handle broken loops and partial buffers in the inner loop code
#define THE_CHECK(TYPE) \
	if (cur_wave >= end_wave)\
	{\
		if (looping)\
		{\
			cur_wave -= wave_adjust;	/* back off pointer for previous sample*/ \
			if (this_voice->doubleBufferProc)\
			{\
				/* we hit the end of the loop call double buffer to notify swap*/ \
				if (PV_DoubleBufferCallbackAndSwap(this_voice->doubleBufferProc, this_voice)) \
				{\
					/* recalculate our internal pointers */\
					end_wave = (XFIXED)(this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;\
					wave_adjust =  (XFIXED)(this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;\
					source = (TYPE) this_voice->NotePtr;\
				}\
				else\
				{\
					goto FINISH;\
				}\
			}\
		}\
		else\
		{\
			this_voice->voiceMode = VOICE_UNUSED;\
			/* PV_DoCallBack(this_voice);*/\
			PV_DoCallBack(this_voice, threadContext);\
			goto FINISH;\
		}\
	}

#define ALLOW_16_BIT			1		// 1 - allow 16 bit if available, 0 - force 8 bit
#define ALLOW_STEREO			1		// 1 - allow stereo if available, 0 - force mono
#define ALLOW_DEBUG_STEREO		0		// 1 - allow keyboard debugging of stereo code
#define USE_DLS					0		// 1 - allow DLS changes, 0 - IGOR

typedef unsigned char			OUTSAMPLE8;
typedef short int		        OUTSAMPLE16;		// 16 bit output sample

enum
{
    SUS_NORMAL			=	0,		// normal release at note off
    SUS_ON_NOTE_ON		=	1,		// note on, with pedal
    SUS_ON_NOTE_OFF		=	2		// note off, with pedal
};

#if CPU_TYPE == kRISC
#pragma options align=power
#elif defined(X_REQUIRE_64BIT_ALIGNMENT)
#pragma pack (8)
#elif defined(X_REQUIRE_32BIT_ALIGNMENT)
#pragma pack (4)
#endif

// Mode in which a GM_Voice is currently being used
typedef enum 
{
    // These are left as reference. They refer to the old code base of what the numbers
    // ment.
    //	VOICE_UNUSED		=	-1,				// voice is free
    //	VOICE_RELEASING		=	0,				// voice is releasing
    //	VOICE_SUSTAINING	=	32767,			// voice is sustaining
    //	VOICE_ALLOCATED		=	1				// voice is allocated, but not active

    VOICE_UNUSED		=	0,				// voice is free
    VOICE_ALLOCATED,						// voice is allocated, but not active
    VOICE_ALLOCATED_READY_TO_SYNC_START,	// voice is allocated, ready to start and on the next slice it will
    // set them to VOICE_SUSTAINING. This will look at the syncVoiceReference
    // variable and start all voices with the same reference
    VOICE_RELEASING,						// voice is releasing
    VOICE_SUSTAINING						// voice is sustaining
} VoiceMode;


// This structure is created and maintained for each sample that is to mixed into the final output
struct GM_Voice
{
    VoiceMode                   voiceMode;          // duration of note to play. -1 is dead
    // This field must be first!
    void*                       syncVoiceReference; // this field is used when voiceMode has been set to VOICE_ALLOCATED_READY_TO_SYNC_START
    // A single pass search will happen and it will look for matching syncVoiceReference
    // values. Once the voice is started it will be set to NULL.
    INT16                       NoteDecay;          // after voiceMode == VOICE_RELEASING then this is ticks of decay
    INT32                       voiceStartTimeStamp;// this is a time stamp of when this voice is started, used to
    // track unique voices
    GM_Instrument               *pInstrument;       // read-only copy of instrument information
    GM_Song					*pSong;					// read-only copy of song information
    // used to backtrace where note came from
    UBYTE 					*NotePtr;		// pointer to start of sample
    UBYTE 					*NotePtrEnd;		// pointer to end of sample: one after last byte ! (=NotePtr+size_in_bytes)
    XFIXED					NoteWave;		// current fractional position within sample (NotePtr:NotePtrEnd)
    XFIXED					NotePitch;		// playback pitch in 16.16 fixed. 1.0 will play recorded speed
    XFIXED					noteSamplePitchAdjust;	// adjustment to pitch based on difference from 22KHz in recorded rate
    UBYTE 					*NoteLoopPtr;		// pointer to start of loop point within NotePtr & NotePtrEnd
    UBYTE 					*NoteLoopEnd;		// pointer to end of loop point within NotePtr & NotePtrEnd
	
    // $$kk: 01.14.98: added NoteLoopTarget
    UINT32					NoteLoopTarget;			// target number of loops before continuing to end of sample

    void					*NoteContext;			// user context for callbacks

    // Double buffer variables. If using double buffering, then doubleBufferPtr1 will be non-zero. These variables
    // will be swapped with NotePtr, NotePtrEnd, NoteLoopPtr, NoteLoopPtrEnd
    UBYTE						*doubleBufferPtr1;
    UBYTE						*doubleBufferPtr2;
    GM_DoubleBufferCallbackPtr	doubleBufferProc;

    // Call back procs
    GM_LoopDoneCallbackPtr		NoteLoopProc;		// normal loop continue proc
    GM_SoundDoneCallbackPtr		NoteEndCallback;	// sample done callback proc

    INT16					NoteNextSize;			// number of samples per slice. Use 0 to recalculate
    SBYTE					NoteMIDIPitch;			// midi note pitch to start note
    SBYTE					noteOffsetStart;		// at the start of the midi note, what was the offset
    INT16					ProcessedPitch;			// actual pitch to play (proccessed)
    INT16					NoteProgram;			// midi program number
    SBYTE					NoteChannel;			// channel note is playing on
    SBYTE					NoteTrack;			// track note is playing on
    INT32					NoteVolume;			// note volume (scaled)
    INT16					NoteVolumeEnvelope;		// scalar from volume ADSR and LFO's.  0 min, VOLUME_RANGE max.
    INT16					NoteVolumeEnvelopeBeforeLFO;	// as described.
    INT16					NoteMIDIVolume;			// note volume (unscaled)
    INT16					NotePitchBend;			// 8.8 Fixed amount of bend
    INT16					ModWheelValue;			// 0-127
    INT16					LastModWheelValue;		// has it changed?  This is how we know.
    INT16					LastPitchBend;			// last bend
    INT16					stereoPosition;			// -63 (left) 0 (Middle) 63 (Right)

    // $$kk: 01.14.98: changed NoteLoopCount from UBYTE to INT32 because we are actually counting loops now and may want quite a few
    UINT32					NoteLoopCount;

    UBYTE					bitSize;			// 8 or 16 bit data
    UBYTE					channels;			// mono or stereo data
    UBYTE					sustainMode;			// sustain mode, for pedal controls
    UBYTE					sampleAndHold;			// flag whether to sample & hold, or sample & release
    UBYTE					avoidReverb;			// don't mix into reverb unit
    UBYTE					reverbLevel;			// 0-127 when reverb is enabled

    UBYTE					processingSlice;		// if TRUE, then thread is processing slice of this instrument

    // sound effects variables. Not used for normal envelope or instruments
    UBYTE					soundEndAtFade;
    XFIXED					soundFadeRate;			// when non-zero fading is enabled
    XFIXED					soundFixedVolume;		// inital volume level that will be changed by soundFadeRate
    INT16					soundFadeMaxVolume;		// max volume
    INT16					soundFadeMinVolume;		// min volume
    GM_SampleCallbackEntry	*pSampleMarkList;		// linked list of callbacks on a per sample frame basis

    INT32					stereoPanBend;

    ADSRRecord				volumeADSRRecord;
    INT32					volumeLFOValue;
    INT16					LFORecordCount;
    LFORecord				LFORecords[MAX_LFOS+1];	// allocate for maximum allowed
    INT32					lastAmplitudeL;
    INT32					lastAmplitudeR;			// used to interpolate between points in volume ADSR
    INT16					chorusLevel;			// 0-127 when chorus is enabled
    INT16					z[MAXRESONANCE+1];
    INT32					zIndex, Z1value, previous_zFrequency;
    INT32					LPF_lowpassAmount, LPF_frequency, LPF_resonance;
    INT32					LPF_base_lowpassAmount, LPF_base_frequency, LPF_base_resonance;
    /* $$fb 2002-01-06 added resample algorithms */
    SR_ResampleParams*			resampleParams;
    XBOOL				disposeResampleParams;
    //	INT32					s1Left, s2Left, s3Left, s4Left, s5Left, s6Left; // for INTERP3 mode only
};
typedef struct GM_Voice GM_Voice;

// support for historical reasons
#define NoteRecord	GM_Voice

// Structure used for caching samples for instruments
struct CacheSampleInfo
{
    UINT32	cacheBlockID;	// block ID. for debugging
    UINT32	rate;			// sample rate
    UINT32	waveSize;		// size in bytes
    UINT32	waveFrames;		// number of frames
    UINT32	loopStart;		// loop start frame
    UINT32	loopEnd;		// loop end frame
    char			bitSize;		// sample bit size; 8 or 16
    char			channels;		// mono or stereo; 1 or 2
    short int		baseKey;		// base sample key
    XLongResourceID	theID;			// sample ID
    INT32			referenceCount;	// how many references to this sample block
    void			*pSampleData;	// pointer to sample data. This may be an offset into the pMasterPtr
    void			*pMasterPtr;	// master pointer that contains the snd format information
};
typedef struct CacheSampleInfo CacheSampleInfo;

struct InstrumentRemap
{
    INT16	from;
    INT16	to;
};
typedef struct InstrumentRemap InstrumentRemap;

#if X_PLATFORM == X_WEBTV
#define MAX_QUEUE_EVENTS				32
#else
#define MAX_QUEUE_EVENTS				256
#endif

#define REVERB_BUFFER_SIZE_SMALL		4096		// * sizeof(long)
#define REVERB_BUFFER_MASK_SMALL		4095
#if USE_SMALL_MEMORY_REVERB
#define REVERB_BUFFER_SIZE			REVERB_BUFFER_SIZE_SMALL
#define REVERB_BUFFER_MASK 			REVERB_BUFFER_MASK_SMALL
#else
#define REVERB_BUFFER_SIZE			16384
#define REVERB_BUFFER_MASK_SHORT 	16383
#define REVERB_BUFFER_MASK 			32767
#endif

// This structure is to allow for queuing midi events into the playback other than those that are
// pulled from the midi file stream
struct Q_MIDIEvent
{
    GM_Song			*pSong;			// pSong the event was placed from
    UINT32			timeStamp;		// timestamp of event
    UBYTE			midiChannel;	// which channel
    UBYTE			command;		// which command
    UBYTE			byte1;			// note, controller
    UBYTE			byte2;			// velocity, lsb/msb
};
typedef struct Q_MIDIEvent Q_MIDIEvent;

// $$kk: 04.19.99
// typedef void			(*InnerLoop)(GM_Voice *r);
// typedef void			(*InnerLoop2)(GM_Voice *r, XBOOL looping);
typedef void			(*InnerLoop)(GM_Voice *r, void *threadContext);
typedef void			(*InnerLoop2)(GM_Voice *r, XBOOL looping, void *threadContext);


// tried to 8 byte align structure (7/17/97)
struct GM_Mixer
{

    CacheSampleInfo 	*sampleCaches[MAX_SAMPLES];		// cache of samples loaded
    GM_Voice			NoteEntry[MAX_VOICES];

#if USE_MOD_API
    // MOD interpreter file playing. NULL is no file
    GM_ModData			*pModPlaying;
#endif

    // MIDI Interpreter variables
    GM_Song				*pSongsToPlay[MAX_SONGS];		// number of songs to play at once

    // normal inner loop procs
    InnerLoop2			partialBufferProc;
    InnerLoop			fullBufferProc;
    InnerLoop2			partialBufferProc16;
    InnerLoop			fullBufferProc16;

    // procs for resonant low-pass filtering
    InnerLoop2			filterPartialBufferProc;
    InnerLoop			filterFullBufferProc;
    InnerLoop2			filterPartialBufferProc16;
    InnerLoop			filterFullBufferProc16;

    // procs for high quality low-pass filtered inner loop
    /* $$fb 2002-01-06 added resample algorithms */
    InnerLoop2			resamplePartialBufferProc;
    InnerLoop			resampleFullBufferProc;
    InnerLoop2			resamplePartialBufferProc16;
    InnerLoop			resampleFullBufferProc16;

    // external midi control variables
    Q_MIDIEvent			theExternalMidiQueue[MAX_QUEUE_EVENTS];
    // pointers for circular event buffer
    Q_MIDIEvent			*pHead;							// pointer to events to read from queue
    Q_MIDIEvent			*pTail;							// pointer to events to write to queue
    // always points to the next one to use
    GM_AudioTaskCallbackPtr		pTaskProc;				// callback for audio tasks
    GM_AudioOutputCallbackPtr	pOutputProc;			// callback for audio output

    // variables used for "classic" fixed verb
    INT32				*reverbBuffer;			// this is the master pointer used
    // for verb. It is shared between
    // different types of verbs, although
    // the data maybe different

    // voice allocation, and dry and wet mix buffers
    INT32				songBufferDry[(MAX_CHUNK_SIZE+64)*2];	// interleaved samples: left-right
    INT32				songBufferReverb[MAX_CHUNK_SIZE+64];	// the +64 is for 48k output
    INT32				songBufferChorus[MAX_CHUNK_SIZE+64];

    TerpMode			interpolationMode;				// output interpolation mode
    Quality				outputQuality;					// output sample rate

    ReverbMode			reverbUnitType;					// verb mode
    ReverbMode			reverbTypeAllocated;			// verb mode allocated

    XBYTE				sampleFrameSize;				// size in bytes of each sample frame
    XBYTE				sampleExpansion;				// output expansion factor 1, 2, or 4
    INT16 				MasterVolume;

    INT16				effectsVolume;					// volume multiplier of all effects
    INT32				scaleBackAmount;

    INT16				MaxNotes;
    INT16				mixLevel;
    INT16				MaxEffects;
    INT16				maxChunkSize;

    LOOPCOUNT			One_Slice, One_Loop, Two_Loop, Four_Loop;
    LOOPCOUNT			Sixteen_Loop;

    XBOOL		/*0*/	generate16output;				// if TRUE, then build 16 bit output
    XBOOL		/*1*/	generateStereoOutput;			// if TRUE, then output stereo data
    XBOOL		/*2*/	insideAudioInterrupt;
    XBOOL		/*3*/	systemPaused;					// all sound paused and disengaged from hardware

    XBOOL 		/*4*/	enableDriftFixer;				// if enabled, this will fix the drift of real time with our synth time.
    XBOOL		/*5*/	sequencerPaused;				// MIDI sequencer paused
    XBOOL		/*6*/	cacheSamples;					// if TRUE, then samples will be cached
    XBOOL		/*7*/	cacheInstruments;				// current not used

    XBOOL		/*0*/	stereoFilter;					// if TRUE, then filter stereo output
    XBYTE		/*1*/	processExternalMidiQueue;		// counter flag to lock processing of queue. 0 means process
#if USE_KAT
    XBOOL		/*2*/	useKatmaiCPU;
#endif

    UINT32				syncCount;						// in microseconds. Current tick of audio output
    INT32				syncBufferCount;

    UINT32				samplesPlayed;					// number of samples played by device
    UINT32				samplesWritten;					// number of samples written to device
    UINT32				lastSamplePosition;				// last time GM_UpdateSamplesPlayed was called

    UINT32				timeSliceDifference;			// value in microseconds between calls to
    // HAE_BuildMixerSlice
	
    UINT32				reverbBufferSize;		// Set the size of memory allocated here.
    // Make sure you set this because it is
    // compared and tested against
    INT32				reverbPtr;				// delay line index into verb buffer
    INT32				LPfilterL, LPfilterR;	// used for fixed verb
    INT32				LPfilterLz, LPfilterRz;
};
typedef struct GM_Mixer GM_Mixer;

// support for historical reasons
#define MusicVars	GM_Mixer

#if CPU_TYPE == kRISC
#pragma options align=reset
#elif defined(X_REQUIRE_64BIT_ALIGNMENT) || defined(X_REQUIRE_32BIT_ALIGNMENT)
#pragma pack()
#endif

#ifdef __cplusplus
extern "C" {
#endif

    extern GM_Mixer *MusicGlobals;

#if USE_NEW_EFFECTS
    /******************************* new reverb stuff *****************************/

#define kCombBufferFrameSize			4096	/* 5000 */
#define kDiffusionBufferFrameSize		4096	/* 4410 */
#define kStereoizerBufferFrameSize		1024	/* 1000 */
#define kEarlyReflectionBufferFrameSize	0x2000	/* 0x1500 */

#define kCombBufferMask					(kCombBufferFrameSize - 1)
#define kDiffusionBufferMask			(kDiffusionBufferFrameSize - 1)
#define kStereoizerBufferMask			(kStereoizerBufferFrameSize - 1)
#define kEarlyReflectionBufferMask		(kEarlyReflectionBufferFrameSize - 1)


#define kNumberOfCombFilters		6
#define kNumberOfEarlyReflections	7


#define kNumberOfDiffusionStages	3


    struct NewReverbParams
    {
	XBOOL				mIsInitialized;
	Quality				mSampleRate;
	INT32				mReverbType;	
	
	/* early reflection params */
	INT32				*mEarlyReflectionBuffer;
	INT32				mEarlyReflectionGain[kNumberOfEarlyReflections];
	int					mReflectionWriteIndex;
	int					mReflectionReadIndex[kNumberOfEarlyReflections];
	
	
	/* comb filter params */	
	INT32				*mReverbBuffer[kNumberOfCombFilters];
	
	int					mReadIndex[kNumberOfCombFilters];
	int					mWriteIndex[kNumberOfCombFilters];
	
	INT32				mUnscaledDelayFrames[kNumberOfCombFilters];
	INT32				mDelayFrames[kNumberOfCombFilters];
	
	INT32 				mFeedbackList[kNumberOfCombFilters];
	
	INT32				mRoomSize;
	INT32				mRoomChoice;
	INT32				mMaxRegen;		// 0-127
	INT32				mDiffusedBalance;
	
	/* diffusion params */
	INT32				*mDiffusionBuffer[kNumberOfDiffusionStages];
	int					mDiffReadIndex[kNumberOfDiffusionStages];
	int					mDiffWriteIndex[kNumberOfDiffusionStages];
	
	/* output filter */
	INT32				mLopassK;
	INT32				mFilterMemory;
	
	/* stereoizer params */
	INT32				*mStereoizerBufferL;
	INT32				*mStereoizerBufferR;
	int					mStereoReadIndex;
	int					mStereoWriteIndex;
    };

    typedef struct NewReverbParams NewReverbParams;

    extern NewReverbParams		gNewReverbParams;

    /* prototypes */
    NewReverbParams*	GetNewReverbParams();
    XBOOL InitNewReverb();	// returns TRUE if success
    void ShutdownNewReverb();
    XBOOL CheckReverbType();
    void ScaleDelayTimes();
    void GenerateDelayTimes();
    void GenerateFeedbackValues();
    void SetupDiffusion();
    void SetupStereoizer();
    void SetupEarlyReflections();
    void RunNewReverb(INT32 *sourceP, INT32 *destP, int nSampleFrames);
    UINT32 GetSamplingRate();
    UINT32 GetSR_44100Ratio();
    UINT32 Get44100_SRRatio();

    /******************************* new chorus stuff *****************************/
#define kChorusBufferFrameSize		4410L

    struct ChorusParams
    {
	XBOOL				mIsInitialized;
	Quality				mSampleRate;
	
	INT32*				mChorusBufferL;
	INT32*				mChorusBufferR;

	int					mWriteIndex;
	INT32				mReadIndexL;
	INT32				mReadIndexR;
	
	int					mSampleFramesDelay;

	INT32				mRate;
	//float				mDepth;
	INT32				mPhi;
	
	INT32				mFeedbackGain;	// between 0-127
    };

    typedef struct ChorusParams ChorusParams;

    /* prototypes */
    ChorusParams* GetChorusParams();
    void InitChorus();
    void ShutdownChorus();
    INT32 GetChorusReadIncrement(INT32 readIndex, INT32 writeIndex, INT32 nSampleFrames, INT32 phase);
    void SetupChorusDelay();
    void RunChorus(INT32 *sourceP, INT32 *destP, int nSampleFrames);


#if 0	// only reverb and chorus are currently activated...

    /******************************* delay stuff *****************************/
#define kDelayBufferFrameSize		44100

    struct DelayEffect
    {
	INT32*				mDelayBuffer;

	int					mWriteIndex;
	int					mReadIndex;
	
	float				mSecondsDelay;
	
	float				mFeedbackValue;
	float				mFeedbackGain;
	
	INT32				mFilterMemoryL;
	INT32				mFilterMemoryR;
	INT32				mLopassK;
    };

    typedef struct DelayEffect DelayEffect;

    /* prototypes */
    void Delay_Initialize(DelayEffect *This);
    void Delay_Shutdown(DelayEffect *This);
    void Delay_Run(DelayEffect *This, INT32 *sourceP);

    extern DelayEffect		gDelay;


    /******************************* graphic eq stuff *****************************/
#define kNumberOfBands		7

    struct GraphicEqParams
    {
	/* right and left filter memory */
	INT32		mHistory1L[kNumberOfBands];
	INT32		mHistory2L[kNumberOfBands];
	INT32		mHistory1R[kNumberOfBands];
	INT32		mHistory2R[kNumberOfBands];
	
	float		mControlList[kNumberOfBands];		/* values between 0.0 and 1.0 */
	float		mGain[kNumberOfBands];
    };

    typedef struct GraphicEqParams GraphicEqParams;

    /* prototypes */
    GraphicEqParams* GetGraphicEqParams();
    void InitGraphicEq();
    void CalculateGraphicEqGains();
    void RunGraphicEq(INT32 *sourceP, int nSampleFrames);


    /******************************* parametric eq stuff *****************************/

    struct ParametricEq
    {
	float	mFreqValue;
	float	mQValue;
	float	mGainValue;

	float	mControlList[3];
	
	double	pi;

	float	sweep;
	
	/* filter memory */
	INT32	x1;
	INT32	x2;
	INT32	y1;
	INT32	y2;
	
	/* filter coefficients */
	float	b0;
	float	b1;
	float	b2;
	float	a1;
	float	a2;
    };

    typedef struct ParametricEq ParametricEq;

    /* prototypes */
    void 	ParametricEq_Initialize(ParametricEq *This);
    void 	ParametricEq_CalculateParams(ParametricEq *This);
    void 	ParametricEq_Run(ParametricEq *This, INT32 *buffer);

    extern ParametricEq		gParametricEq;

    /******************************* resonant filter stuff *****************************/

    struct ResonantFilterParams
    {
	float	mFrequency;
	float	mResonance;

	float	mControlList[2];
	
	double	pi;

	float	sweep;
	
	/* filter memory */
	INT32	y1;
	INT32	y2;
	
	/* filter coefficients */
	float	c0;
	float	c1;
	float	c2;
    };

    typedef struct ResonantFilterParams ResonantFilterParams;

    /* prototypes */
    ResonantFilterParams* GetResonantFilterParams();
    void InitResonantFilter();
    void CalculateResonantParams(float inFrequency, float inResonance);
    void RunResonantFilter(INT32 *buffer, int nSampleFrames);

#endif // 0

#endif // USE_NEW_EFFECTS
    /******************************************************************************/





    /* Interal function declarations
     */
    INT32		XGetMouse(void);
    XBOOL		XIsOptionOn(void);
    XBOOL		XIsShiftOn(void);


    void PV_Generate8outputStereo(OUTSAMPLE8 * dest8);
    void PV_Generate8outputMono(OUTSAMPLE8 * dest8);
    void PV_Generate16outputStereo(OUTSAMPLE16 * dest16);
    void PV_Generate16outputMono(OUTSAMPLE16 * dest16);

    INT32 PV_DoubleBufferCallbackAndSwap(GM_DoubleBufferCallbackPtr doubleBufferCallback, 
					 GM_Voice *this_voice);
    void PV_CalculateStereoVolume(GM_Voice *this_voice, INT32 *pLeft, INT32 *pRight);

    // $$kk: 04.19.99
    //void PV_ServeEffectsFades(void);
    void PV_ServeEffectsFades(void *threadContext);

    void PV_ServeEffectCallbacks(void *threadContext);

    // $$kk: 04.19.99
    /*
      void PV_ServeStereoInterp3FullBuffer (GM_Voice *this_voice);
      void PV_ServeStereoInterp3PartialBuffer (GM_Voice *this_voice, XBOOL looping);

      void PV_ServeInterp2FilterFullBufferNewReverb (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FilterFullBufferNewReverb (GM_Voice *this_voice);
      void PV_ServeInterp2FilterFullBufferNewReverb16 (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FilterFullBufferNewReverb16 (GM_Voice *this_voice);

      void PV_ServeInterp2FilterFullBuffer (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FilterFullBuffer (GM_Voice *this_voice);
      void PV_ServeInterp2FilterFullBuffer16 (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FilterFullBuffer16 (GM_Voice *this_voice);

      void PV_ServeInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping);

      void PV_ServeInterp2FilterPartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeInterp2FilterPartialBufferNewReverb16 (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2FilterPartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2FilterPartialBufferNewReverb16 (GM_Voice *this_voice, XBOOL looping);

      void PV_ServeInterp2FullBuffer (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FullBuffer (GM_Voice *this_voice);
      void PV_ServeInterp2FullBuffer16 (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FullBuffer16 (GM_Voice *this_voice);

      void PV_ServeInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping);

      void PV_ServeInterp2FullBufferNewReverb (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FullBufferNewReverb (GM_Voice *this_voice);
      void PV_ServeInterp2FullBuffer16NewReverb (GM_Voice *this_voice);
      void PV_ServeStereoInterp2FullBuffer16NewReverb (GM_Voice *this_voice);

      void PV_ServeInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping);

      void PV_ServeInterp1FullBuffer (GM_Voice *this_voice);
      void PV_ServeInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoInterp1FullBuffer (GM_Voice *this_voice);
      void PV_ServeStereoInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping);

      void PV_ServeDropSampleFullBuffer (GM_Voice *this_voice);
      void PV_ServeDropSamplePartialBuffer (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeDropSampleFullBuffer16 (GM_Voice *this_voice);
      void PV_ServeDropSamplePartialBuffer16 (GM_Voice *this_voice, XBOOL looping);
      void PV_ServeStereoAmpFullBuffer (GM_Voice *this_voice);
      void PV_ServeStereoAmpPartialBuffer (GM_Voice *this_voice, XBOOL looping);
    */


    void PV_ServeStereoInterp3FullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp3PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);

    void PV_ServeInterp2FilterFullBufferNewReverb (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FilterFullBufferNewReverb (GM_Voice *this_voice, void *threadContext);
    void PV_ServeInterp2FilterFullBufferNewReverb16 (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FilterFullBufferNewReverb16 (GM_Voice *this_voice, void *threadContext);

    void PV_ServeInterp2FilterFullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FilterFullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeInterp2FilterFullBuffer16 (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FilterFullBuffer16 (GM_Voice *this_voice, void *threadContext);

    void PV_ServeInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);

    void PV_ServeInterp2FilterPartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeInterp2FilterPartialBufferNewReverb16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2FilterPartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2FilterPartialBufferNewReverb16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);

    void PV_ServeInterp2FullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeInterp2FullBuffer16 (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FullBuffer16 (GM_Voice *this_voice, void *threadContext);

    void PV_ServeInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);

    void PV_ServeInterp2FullBufferNewReverb (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FullBufferNewReverb (GM_Voice *this_voice, void *threadContext);
    void PV_ServeInterp2FullBuffer16NewReverb (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp2FullBuffer16NewReverb (GM_Voice *this_voice, void *threadContext);

    void PV_ServeInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext);

    void PV_ServeInterp1FullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoInterp1FullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);

    void PV_ServeDropSampleFullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeDropSamplePartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeDropSampleFullBuffer16 (GM_Voice *this_voice, void *threadContext);
    void PV_ServeDropSamplePartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoAmpFullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoAmpPartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);

    /* $$fb 2002-01-06 added resample algorithms */
    void PV_ServeResampleFullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoResampleFullBuffer (GM_Voice *this_voice, void *threadContext);
    void PV_ServeResampleFullBuffer16 (GM_Voice *this_voice, void *threadContext);
    void PV_ServeStereoResampleFullBuffer16 (GM_Voice *this_voice, void *threadContext);

    void PV_ServeResamplePartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoResamplePartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeResamplePartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);
    void PV_ServeStereoResamplePartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext);



    void ServeMIDINote(GM_Song *pSong, INT16 the_instrument, 
		       INT16 the_channel, INT16 the_track, INT16 notePitch, INT32 Volume);
    void StopMIDINote(GM_Song *pSong, INT16 the_instrument, 
		      INT16 the_channel, INT16 the_track, INT16 notePitch);

    // voices modifiers
    INT16 SetChannelPitchBend(GM_Song *pSong, INT16 the_channel, UBYTE bendRange, UBYTE bendMSB, UBYTE bendLSB);
    void SetChannelVolume(GM_Song *pSong, INT16 the_channel, INT16 newVolume);
    INT16 SetChannelStereoPosition(GM_Song *pSong, INT16 the_channel, UINT16 newPosition);
    void SetChannelModWheel(GM_Song *pSong, INT16 the_channel, UINT16 value);
    void PV_ChangeSustainedNotes(GM_Song *pSong, INT16 the_channel, INT16 data);

    void PV_CleanExternalQueue(void);

    // process 11 ms worth of sample data
    void PV_ProcessSampleFrame(void *threadContext, void *destSampleData);
    void PV_ProcessSequencerEvents(void *threadContext);

    OPErr PV_ProcessMidiSequencerSlice(void *threadContext, GM_Song *pSong);

    void PV_ConfigureInstruments(GM_Song *theSong);
    OPErr PV_ConfigureMusic(GM_Song *theSong);
    void PV_ResetControlers(GM_Song *pSong, INT16 channel2Reset, XBOOL completeReset);

    // GenPatch.c
    GM_Instrument * PV_GetInstrument(XLongResourceID theID, void *theExternalX, INT32 patchSize);
    void PV_SetSampleIntoCache(GM_Song *pSong, XLongResourceID theID, XPTR pSndFormatData);

    UINT32 PV_ScaleVolumeFromChannelAndSong(GM_Song *pSong, INT16 channel, UINT32 volume);

    // $$kk: 04.19.99
    //void PV_DoCallBack(GM_Voice *this_one);
    void PV_DoCallBack(GM_Voice *this_one, void *threadContext);
									  
    void PV_CleanNoteEntry(GM_Voice * the_entry);
    void PV_CalcScaleBack(void);
    XFIXED PV_GetWavePitch(XFIXED notePitch);

    // GenModFiles.c
    void PV_WriteModOutput(Quality q, XBOOL stereo);

    // GenAudioStreams.c
    void PV_ServeStreamFades(void);

    // GenSeq.c
    void PV_FreePatchInfo(GM_Song *pSong);
    void PV_InsertBankSelect(GM_Song *pSong, short channel, short currentTrack);

    // GenSynth.c
    INT32 PV_ModifyVelocityFromCurve(GM_Song *pSong, INT32 volume);

    // GenSample.c
    GM_Voice * PV_GetVoiceFromSoundReference(VOICE_REFERENCE reference);
    /* $$fb 2003-03-14: add this function. Related to fix for 4828556 */
    VOICE_REFERENCE PV_GetSoundReferenceFromVoice(GM_Voice* pVoice);

#ifdef __cplusplus
}
#endif

#endif 	/* G_PRIVATE	*/ 
