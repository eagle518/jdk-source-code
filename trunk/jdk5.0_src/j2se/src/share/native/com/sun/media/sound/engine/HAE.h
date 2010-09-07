/*
 * @(#)HAE.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "HAE.h"
**
**	Generalized Audio Synthesis package presented in an oop fashion
**
** Modification History:
**
**	7/8/96		Created
**	11/18/96	Added LoadResourceSample
**	11/19/96	Changed char to HAE_BOOL in cases of boolean use
**	11/21/96	Changed LoadResourceSample to pass in size of data
**				Seperated HAEMidi into two object types. HAEMidiFile and
**				HAEMidiDirect, to make it clear as to what you are controlling
**	11/25/96	Modifed class to sub class the HAEMidiDirect class.
**	11/26/96	Added Fade
**	12/1/96		Changed GetCurrentProgramBank to GetProgramBank
**				Changed GetCurrentControlValue to GetControlValue
**				Added TRUE/FALSE
**	12/2/96		Added Mod code
**	12/3/96		Fixed bug with HAESound::Start
**	12/10/96	Added HAEAudioMixer::ChangeAudioFileToMemory
**	12/30/96	Changed copyright
**	1/2/97		Added GetSongNameFromAudioFile & GetSampleNameFromAudioFile &
**				GetInstrumentNameFromAudioFile
**	1/7/97		Added HAEMidiFile::LoadFromMemory
**	1/10/97		Added some const
**	1/16/97		Added GetInfoSize
**	1/22/97		Added HAEAudioMixer::GetTick
**				Fixed callbacks and added SetDoneCallback in HAESound and HAEMod
**	1/24/97		Fixed a bug with HAEAudioMixer::Close that could make my counters
**				get out of sync
**				Added HAEMod::SetLoopFlag & HAEMod::SetTempoInBeatsPerMinute &
**				HAEMod::GetTempoInBeatsPerMinute
**				Added LONG_TO_FIXED & FIXED_TO_LONG
**				Rebuilt fade system. Completely intergrated with low level mixer
**	1/28/97		Added more comments on how to use the ChangeAudioFileToMemory method
**				Added a HAERMFFile class. Its pretty much the same as a HAEMidiFile
**				object, except it knows how to read an RMF file
**	2/1/97		Added HAEMidiDirect::AllowChannelPitchOffset & 
**				HAEMidiDirect::DoesChannelAllowPitchOffset
**	2/5/97		Added HAESound::LoadMemorySample
**				Changed MOD handling not to keep MOD loaded into memory once parsed
**				Added HAEMod::LoadFromMemory
**	2/19/97		Added support for platform specific data when creating an HAEAudioMixer
**	2/26/97		Removed extra copyright date field
**	2/28/97		Added NoteOnWithLoad & IsInstrumentLoaded & TranslateBankProgramToInstrument & 
**				TranslateCurrentProgramToInstrument
**	3/3/97		Removed NoteOnWithLoad & TranslateCurrentProgramToInstrument
**				Added the typedef for special instrument
**	3/10/97		Removed extra reverbmode get, and typed the SetReverb correctly
**	3/12/97		Changed the default for MixLevel to 8 in the HAEMixer Open method
**				Added a HAE_REVERB_NO_CHANGE reverb mode. This means don't change the
**				current mixer state
**	3/18/97		Added GetLoopFlag in HAEMidiFile & HAEMod
**	3/20/97		Added LoadFromID in the HAEMidiFile method
**				Added GetMicrosecondLength & SetMicrosecondPosition & GetMicrosecondPosition
**				in the HAEMidiFile class
**	4/18/97		Removed extra linkage and unused variables
**	4/20/97		Added HAEMidiFile::SetMetaEventCallback
**	5/1/97		Changed HAESound::Start to accecpt an frame offset when starting the sample
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
**	6/25/97		Changed an unsigned to an unsigned long in HAESound::SetSampleLoopPoints
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
**	8/15/97		Moved the method HAEAudioMixer::SetControlCallback to HAEMidiFile::SetControlCallback
**	8/20/97		Changed HAESoundStream::SetupCustomStream & HAESoundStream::SetupFileStream
**				to use HAE_MIN_STREAM_BUFFER_SIZE to control minimumn size of buffer
**	9/11/97		Added HAEMidiFile::IsPaused && HAEMod::IsDone
**	9/18/97		Respelled GetAudioLatancy to GetAudioLatency
**	9/30/97		Added HAEMidiFile::GetEmbeddedMidiVoices & HAEMidiFile::GetEmbeddedMixLevel & 
**				HAEMidiFile::GetEmbeddedSoundVoices
**	10/2/97		Added HAE_MAX_SONGS
**	10/3/97		Added HAEMidiFile::SetEmbeddedMidiVoices & HAEMidiFile::SetEmbeddedMixLevel & 
**				HAEMidiFile::SetEmbeddedSoundVoices
**				Added HAEMidiFile::GetEmbeddedReverbType & HAEMidiFile::SetEmbeddedReverbType
**				Added HAEAudioMixer::GetQuality & HAEAudioMixer::GetTerpMode
**	10/12/97	Added HAERMFFile::LoadFromBank
**	10/16/97	Changed HAEMidiFile::Start to allow for an optional reconfigure of
**				the mixer when starting a song
**				Modified HAEMidiFile::LoadFromID & HAEMidiFile::LoadFromBank & HAEMidiFile::LoadFromFile &
**				HAEMidiFile::LoadFromMemory & HAERMFFile::LoadFromFile & HAERMFFile::LoadFromMemory &
**				HAERMFFile::LoadFromBank to allow for optional reporting of failure to load instruments
**				Removed HAEMidiDirect::FlushInstrumentCache. Not required or used.
**				Renamed GetPatches to GetInstruments and changed the array passed to
**				be of type HAE_INSTRUMENT
**	11/6/97		Added HAERMFFile::LoadFromID
**	11/11/97	Added GetMaxDeviceCount & SetCurrentDevice & GetCurrentDevice & GetDeviceName
**	11/24/97	Added HAESoundStream::Flush
**	12/18/97	Cleaned up some warnings and added some rounding devices
**	1/22/98		Added HAERMFFile::IsCompressed HAERMFFile::IsEncrypted
**	1/27/98		Added a parameter to HAEMidiFile::SetTimeCallback and changed the way the callbacks
**				are handled
**	2/6/98		Added virtuals to all sub classed destructor methods
**	2/11/98		Changed HAESound::SetSampleLoopPoints to accept 0 as a valid start
**				loop point
**				Added HAE_8K, HAE_48K, HAE_11K_TERP_22K, HAE_22K_TERP_44K, HAE_24K
**	2/18/98		Added HAESound::StartDoubleBuffer
**	2/19/98		Added HAEAudioMixer::GetURLFromAudioFile & HAEAudioMixer::GetNameFromAudioFile
**	2/24/98		Fixed a problem with the way the SONG resource and memory based files handle
**				retriving the size of memory blocks inside of a memory file
**	3/5/98		Changed TranslateBankProgramToInstrument to a static object
**	3/9/98		Modified open to allow an optional not connect to audio hardware. If you call
**				Open without connecting to hardware, you'll need to call HAEAudioMixer::ReengageAudio
**				Added new method HAEAudioMixer::IsAudioEngaged
**	3/12/98		Added HAEMidiFile::SetEmbeddedVolume & HAEMidiFile::GetEmbeddedVolume
**	3/16/98		Added new verb types
**	3/23/98		MOE: Added error codes: HAE_BAD_SAMPLE_RATE, HAE_TOO_MANY_SAMPLES,
**				HAE_UNSUPPORTED_FORMAT, HAE_FILE_IO_ERROR
**	3/23/98		MOE: Added enums: HAEEncryptionType{}, HAECompressionType{}
**	4/30/98		Added SUB_GENRE_INFO & GENRE_INFO
**	5/7/98		Added HAE_REVERB_TYPE_COUNT
**	6/18/98		Added SetCacheStatusForAudioFile & GetCacheStatusForAudioFile
**	7/27/98		Added LockQueue & UnlockQueue
**	7/28/98		Added HAEMidiDirect::SetStereoPosition & HAEMidiDirect::GetStereoPosition
**	7/30/98		Added reverb state and fixed all the HAESound::Start/HAESoundStream::Start functions 
**				to start the verb up when the object gets started.
**	8/6/98		Changed reference to mSoundVoiceReference in the HAESound class.
**				Changed mReference to mSoundStreamVoiceReference in the HAESoundStream class
**	8/10/98		Added macros FLOAT_TO_UNSIGNED_FIXED & UNSIGNED_FIXED_TO_FLOAT & LONG_TO_UNSIGNED_FIXED &
**				UNSIGNED_FIXED_TO_LONG & UNSIGNED_FIXED_TO_SHORT
**	8/11/98		Renamed pName to cName and implemented GetName in the HAEAudioNoise base class
**				Changed default behavior of HAEMidiFile::LoadFromFile & HAEMidiFile::LoadFromBank & 
**				HAEMidiFile::LoadFromID & HAEMidiFile::LoadFromMemory to ignore bad instruments
**	8/13/98		Added HAEVoiceType to HAEAudioInfo structure
**				Added HAEAudioMixer::GetCPULoadInMicroseconds & HAEAudioMixer::GetCPULoadInPercent
**	9/2/98		Added HAE_MPEG_TYPE to HAEFileType
**	9/8/98		Added HAE_SAMPLE_TO_LARGE
**	9/12/98		Added HAESound::GetSamplePointerFromMixer
**	10/26/98	Added error code to HAEAudioMixer::ServiceAudioOutputToFile
**	10/30/98	Implemented a default HAESound done callback, that marks the sample finished when playing out
**				normally. Related to the 10/17/98 bug.
**	11/24/98	Added HAESound::SetReverbAmount & HAESound::GetReverbAmount.
**	12/3/98		Added HAESoundStream::SetReverbAmount & HAESoundStream::GetReverbAmount
**				Added HAEGroup class, and modified HAEAudioNoise for linked list issues
**	12/9/98		Added HAEMidiDirect::GetPitchBend
**	12/17/98	Added HAEAudioMixer::SetHardwareBalance & HAEAudioMixer::GetHardwareBalance
**	12/18/98	Added to HAEMidiFile::Start auto level
**	1/14/99		Added HAEMidiDirect::CreateInstrumentAsData && HAEMidiDirect::LoadInstrumentFromData
**	2/18/99		Renamed pSongVariables to m_pSongVariables, queueMidi to m_queueMidi, 
**				reference to m_reference
**				Added GetMetaCallback & GetMetaCallbackReference and support variables
**	3/1/99		Added new types to HAECompressionType
*/
/*****************************************************************************/

/*
	Description of use:
	
	Create one HAEAudioMixer and only one. Call open to setup the mixing engine. When
	done call close or the destructor method. When you call Open, make sure you allocate
	voices for Midi and Sound effects. If you allocate 0 sound effects no effects will 
	play, and the same for midi. Pass in the path of the sound bank you want to work as 
	your default. The HAEPathName path name is a FSSpec for Macintosh, and everyone 
	else is a 'C' string with a complete path name. If you want to change instrument
	libraries, call ChangeAudioFile. Once a song is loaded, the file is no longer
	needed, and you can changed them at will.
	
	If you have your sample library, in ROM for example, and you want to set up the engine
	to access that data but not duplicate it, then call the Open method from the HAEAudioMixer
	with NULL for the sound bank. Then call the method ChangeAudioFileToMemory and point
	it to the block of memory that represents the file. The data will not be copied
	unless its compressed. Do not release the memory block until you are finished.

	To play a midi file create a HAEMidiFile and call LoadFromFile or LoadFromBank
	then call Start.

	You can have as many of HAEMidiFile objects as memory will hold, but you can only
	call Start on 4 of them. Start will return an error if you are trying to play to
	many. Once the midi file is playing you can call any method from the HAEMidiDirect
	class except for Open. This allows you to modify the song in realtime. If you call open
	then the HAEMidiFile object will basicly become a HAEMidiDirect object.
	
	To drive the synthesizer directly. ie NoteOn, NoteOff, etc; create a HAEMidiDirect
	and call Open. Then LoadInstrument for each instrument you want, unless you passed 
	TRUE to Open. You can then call NoteOn, NoteOff, etc. To post events in the future, 
	call GetTick then add the amount of time you want to post. Remember all times are 
	in microseconds.
	
	To play sound effects create a HAESound. You can have as many of these as you like.
	If you dispose of the HAESound before the sound is done playing, the sound will be
	stopped. First you create an HAESound for every sound object you want to play, then
	you load the object with the type of sample data required, ie. from a resource; from
	a file, from a memory pointer, then you can play it. It will always copy the data
	from the source data. When you destroy the object the sample data will go as well.
	
	To stream sound, create a HAESoundStream. You can have up the total number of sound
	effects you allocated when you created the HAEAudioMixer. Call the SetupFileStream method
	with your choice of file types, file, and buffer size, then call the Start method to start
	the stream. You must call the  HAEAudioMixer::ServiceStreams method as much as possible. 
	If you don't you'll get hicups in the sound. Once the stream is running, you can control 
	various features from the HAESoundStream class. If you want to stream custom data you'll 
	use the SetupCustomStream method with a callback that prompts you when to allocate, 
	destroy memory and fill memory.

	Remember to dispose of all of your HAESound, HAESoundStream, HAEMidiDirect and 
	HAEMidiFile objects before disposing of the HAEAudioMixer.

*/

#ifndef HAE_AUDIO
#define HAE_AUDIO

// types
enum HAETerpMode
{
    HAE_DROP_SAMPLE = 0,
    HAE_2_POINT_INTERPOLATION,
    HAE_LINEAR_INTERPOLATION
};

// Quality types
enum HAEQuality
{
    HAE_8K = 0,								// output at 8 kHz
    HAE_11K,								// 11 kHz
    HAE_11K_TERP_22K,						// 11 kHz interpolated to 22 kHz
    HAE_22K,								// 22 kHz
    HAE_22K_TERP_44K,						// 22 kHz interpolated to 44 kHz
    HAE_24K,								// 24 kHz
    HAE_44K,								// 44 kHz
    HAE_48K									// 48 kHz
};

// Modifier types
#define HAE_NONE				0L
#define HAE_USE_16				(1<<0L)		// use 16 bit output
#define HAE_USE_STEREO			(1<<1L)		// use stereo output
#define HAE_DISABLE_REVERB		(1<<2L)		// disable reverb
#define HAE_STEREO_FILTER		(1<<3L)		// if stereo is enabled, use a stereo filter
typedef long HAEAudioModifiers;

enum HAEReverbMode 
{
    HAE_REVERB_NO_CHANGE = 0,				// don't change the mixer settings
    HAE_REVERB_NONE = 1,
    HAE_REVERB_TYPE_1 = 1,					// None
    HAE_REVERB_TYPE_2,						// Igor's Closet
    HAE_REVERB_TYPE_3,						// Igor's Garage
    HAE_REVERB_TYPE_4,						// IgorÕs Acoustic Lab
    HAE_REVERB_TYPE_5,						// Igor's Cavern
    HAE_REVERB_TYPE_6,						// Igor's Dungeon
    HAE_REVERB_TYPE_7,						// Small reflections Reverb used for WebTV
    HAE_REVERB_TYPE_8,						// Early reflections (variable verb)
    HAE_REVERB_TYPE_9,						// Basement (variable verb)
    HAE_REVERB_TYPE_10,						// Banquet hall (variable verb)
    HAE_REVERB_TYPE_11,						// Catacombs (variable verb)
    HAE_REVERB_TYPE_COUNT
};

// used by the HAEExporter code
enum HAEEncryptionType
{
    HAE_ENCRYPTION_NONE,
    HAE_ENCRYPTION_NORMAL,
    HAE_ENCRYPTION_TYPE_COUNT
};
enum HAECompressionType
{
    HAE_COMPRESSION_NONE,
    HAE_COMPRESSION_NORMAL,
    HAE_COMPRESSION_IMA,
    HAE_COMPRESSION_MPEG_32,
    HAE_COMPRESSION_MPEG_40,
    HAE_COMPRESSION_MPEG_48,
    HAE_COMPRESSION_MPEG_56,
    HAE_COMPRESSION_MPEG_64,
    HAE_COMPRESSION_MPEG_80,
    HAE_COMPRESSION_MPEG_96,
    HAE_COMPRESSION_MPEG_112,
    HAE_COMPRESSION_MPEG_128,
    HAE_COMPRESSION_MPEG_160,
    HAE_COMPRESSION_MPEG_192,
    HAE_COMPRESSION_MPEG_224,
    HAE_COMPRESSION_MPEG_256,
    HAE_COMPRESSION_MPEG_320,
    HAE_COMPRESSION_TYPE_COUNT
};

typedef void *		HAEPathName;			// this a pointer to 
// a 'C' string
// ie. "C:\FOLDER\FILE" for WinOS
// and is a FSSpec for MacOS

/* Common errors returned from the system */
enum HAEErr
{
    HAE_NO_ERROR = 0,
    HAE_PARAM_ERR = 10000,
    HAE_MEMORY_ERR,
    HAE_BAD_INSTRUMENT,
    HAE_BAD_MIDI_DATA,
    HAE_ALREADY_PAUSED,
    HAE_ALREADY_RESUMED,
    HAE_DEVICE_UNAVAILABLE,
    HAE_NO_SONG_PLAYING,
    HAE_STILL_PLAYING,
    HAE_TOO_MANY_SONGS_PLAYING,
    HAE_NO_VOLUME,
    HAE_GENERAL_ERR,
    HAE_NOT_SETUP,
    HAE_NO_FREE_VOICES,
    HAE_STREAM_STOP_PLAY,
    HAE_BAD_FILE_TYPE,
    HAE_GENERAL_BAD,
    HAE_BAD_FILE,
    HAE_NOT_REENTERANT,
    HAE_BAD_SAMPLE,
    HAE_BUFFER_TO_SMALL,
    HAE_BAD_BANK,
    HAE_BAD_SAMPLE_RATE,
    HAE_TOO_MANY_SAMPLES,
    HAE_UNSUPPORTED_FORMAT,
    HAE_FILE_IO_ERROR,
    HAE_SAMPLE_TO_LARGE,
    HAE_UNSUPPORTED_HARDWARE,
	
    HAE_ERROR_COUNT = 31
};

enum HAEInfoTypes
{
    TITLE_INFO = 0,				// Title
    PERFORMED_BY_INFO,			// Performed by
    COMPOSER_INFO,				// Composer(s)
    COPYRIGHT_INFO,				// Copyright Date
    PUBLISHER_CONTACT_INFO,		// Publisher Contact Info
    USE_OF_LICENSE_INFO,		// Use of License
    LICENSED_TO_URL_INFO,		// Licensed to what URL
    LICENSE_TERM_INFO,			// License term
    EXPIRATION_DATE_INFO,		// Expiration Date
    COMPOSER_NOTES_INFO,		// Composer Notes
    INDEX_NUMBER_INFO,			// Index Number
    GENRE_INFO,					// Genre
    SUB_GENRE_INFO,				// Sub-genre
    INFO_TYPE_COUNT				// always count of type InfoTypes
};
#define InfoTypes	HAEInfoTypes

// These are embedded text events inside of midi files
enum HAEMetaTypes
{
    GENERIC_TEXT_TYPE	=	1,	// generic text
    COPYRIGHT_TYPE		=	2,	// copyright text
    TRACK_NAME_TYPE		=	3,	// track name of sequence text
    LYRIC_TYPE			=	5,	// lyric text
    MARKER_TYPE	 		=	6,	// marker text (HAE supports LOOPSTART, LOOPEND, LOOPSTART= commands)
    CUE_POINT_TYPE		=	7	// cue point text
};

enum HAEFileType
{
    HAE_INVALID_TYPE = 0,
    HAE_AIFF_TYPE = 1,
    HAE_WAVE_TYPE,
    HAE_MPEG_TYPE,
    HAE_AU_TYPE,
    HAE_GROOVOID,
    HAE_RMF
};


// All volume levels are a 16.16 fixed value. 1.0 is 0x10000. Use can use this macro
// to convert a floating point number to a fixed value, and visa versa

typedef long								HAE_FIXED;				// fixed point value can be signed
typedef unsigned long						HAE_UNSIGNED_FIXED;		// fixed point value unsigned 

#define	HAE_FIXED_1							0x10000L
#define FLOAT_TO_FIXED(x)					((HAE_FIXED)((double)(x) * 65536.0))	// the extra long is for signed values
#define FIXED_TO_FLOAT(x)					((double)(x) / 65536.0)
#define LONG_TO_FIXED(x)					((HAE_FIXED)(x) * HAE_FIXED_1)
#define FIXED_TO_LONG(x)					((x) / HAE_FIXED_1)
#define	FIXED_TO_SHORT(x)					((short)((x) / HAE_FIXED_1))

#define FLOAT_TO_UNSIGNED_FIXED(x)			((HAE_UNSIGNED_FIXED)((double)(x) * 65536.0))	// the extra long is for signed values
#define UNSIGNED_FIXED_TO_FLOAT(x)			((double)(x) / 65536.0)
#define LONG_TO_UNSIGNED_FIXED(x)			((HAE_UNSIGNED_FIXED)(x) * HAE_FIXED_1)
#define UNSIGNED_FIXED_TO_LONG(x)			((x) / HAE_FIXED_1)
#define	UNSIGNED_FIXED_TO_SHORT(x)			((unsigned short)((x) / HAE_FIXED_1))

#define RATIO_TO_FIXED(a,b)					(LONG_TO_FIXED(a) / (b))
#define FIXED_TO_LONG_ROUNDED(x)			FIXED_TO_LONG((x) + HAE_FIXED_1 / 2)
#define FIXED_TO_SHORT_ROUNDED(x)			FIXED_TO_SHORT((x) + HAE_FIXED_1 / 2)

#define UNSIGNED_RATIO_TO_FIXED(a,b)		(LONG_TO_UNSIGNED_FIXED(a) / (b))
#define UNSIGNED_FIXED_TO_LONG_ROUNDED(x)	UNSIGNED_FIXED_TO_LONG((x) + HAE_FIXED_1 / 2)
#define UNSIGNED_FIXED_TO_SHORT_ROUNDED(x)	UNSIGNED_FIXED_TO_SHORT((x) + HAE_FIXED_1 / 2)


typedef char			HAE_BOOL;
typedef unsigned long	HAE_INSTRUMENT;

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

#undef NULL
#define	NULL	0L

// Callback for midi controllers
typedef void		(*HAEControlerCallbackPtr)(void * pReference, short int channel, short int track, short int controler, 
						   short int value);
// Callback for task called at 11 ms intervals
typedef void		(*HAETaskCallbackPtr)(long ticks);

// Callback from midi sequencer
typedef void		(*HAETimeCallbackPtr)(void *pReference, unsigned long currentMicroseconds, unsigned long currentMidiClock);

// Callback from midi sequencer when meta events are encountered
typedef void		(*HAEMetaEventCallbackPtr)(void * pReference, HAEMetaTypes type, void *pMetaText, long metaTextLength);

// Callback when object is finished
typedef void		(*HAEDoneCallbackPtr)(void * pReference);

// Callback for HAESound double buffer playback
typedef void		(*HAEDoubleBufferCallbackPtr)(void * pReference, void *pWhichBufferFinished, unsigned long *pNewBufferSize);


// Callback when object needs to continue a loop
typedef HAE_BOOL	(*HAELoopDoneCallbackPtr)(void * pReference);

// The void will become either a short int, or a unsigned char depending upon how the mixer is
// setup
typedef void		(*HAEOutputCallbackPtr)(void *samples, long sampleSize, long channels, unsigned long lengthInFrames);

// Callback for sample frame calls
typedef void		(*HAESampleFrameCallbackPtr)(void * pReference, unsigned long sampleFrame);

#define HAE_MAX_VOICES				64		// total number of voices. This is shared amongst
// all HAESound's, HAESoundStream's, and HAEMidiFile's
#define HAE_MAX_MIDI_VOLUME			127
#define HAE_MAX_MIDI_TRACKS			65		// 64 midi tracks, plus 1 tempo track
#define HAE_MAX_SONGS				8
#define HAE_MIN_STREAM_BUFFER_SIZE	30000

#define HAE_FULL_LEFT_PAN			(-63)
#define HAE_CENTER_PAN				0
#define HAE_FULL_RIGHT_PAN			(63)

enum HAEVoiceType
{
    HAE_UNKNOWN = 0,				// Voice is an undefined type
    HAE_MIDI_PCM_VOICE,			// Voice is a PCM voice used by MIDI
    HAE_SOUND_PCM_VOICE			// Voice is a PCM sound effect used by HAESound/HAESoundStream
};


struct HAEAudioInfo
{
    short int		voicesActive;						// number of voices active
    short int		voice[HAE_MAX_VOICES];				// voice index
    HAEVoiceType	voiceType[HAE_MAX_VOICES];			// voice type
    short int		instrument[HAE_MAX_VOICES];			// current instruments
    short int		midiVolume[HAE_MAX_VOICES];			// current volumes
    short int		scaledVolume[HAE_MAX_VOICES];		// current scaled volumes
    short int		channel[HAE_MAX_VOICES];			// current channel
    short int		midiNote[HAE_MAX_VOICES];			// current midi note
    long			userReference[HAE_MAX_VOICES];		// userReference associated with voice
};
typedef struct HAEAudioInfo HAEAudioInfo;



struct HAESampleInfo
{
    unsigned short		bitSize;			// number of bits per sample
    unsigned short		channels;			// number of channels (1 or 2)
    unsigned short		baseMidiPitch;		// base Midi pitch of recorded sample ie. 60 is middle 'C'
    unsigned long		waveSize;			// total waveform size in bytes
    unsigned long		waveFrames;			// number of frames
    unsigned long		startLoop;			// start loop point offset
    unsigned long		endLoop;			// end loop point offset
    HAE_UNSIGNED_FIXED	sampledRate;		// fixed 16.16 value for recording
};
typedef struct HAESampleInfo HAESampleInfo;


// HAE mixer class. Can only be one mixer object.
class HAEAudioMixer 
{
    public:
    friend class HAERMFFile;
    friend class HAEMidiFile;
    friend class HAEMidiDirect;
    friend class HAESound;
    friend class HAESoundStream;
    friend class HAEAudioNoise;

    HAEAudioMixer();
    virtual	~HAEAudioMixer();

    // number of devices. ie different versions of the HAE connection. DirectSound and waveOut
    // return number of devices. ie 1 is one device, 2 is two devices.
    // NOTE: This function can be called before Open is called
    long 			GetMaxDeviceCount(void);

    // set current device. should be a number from 0 to HAEAudioMixer::GetMaxDeviceCount()
    // deviceParameter is a pointer to device specific info. It will
    // be NULL if there's nothing interesting. For Windows Device 1 (DirectSound) it
    // is the window handle that DirectSound will attach to. Pass NULL, if you don't know
    // what is correct.
    void 			SetCurrentDevice(long deviceID, void *deviceParameter = NULL);

    // get current device. deviceParameter is a pointer to device specific info. It will
    // be NULL if there's nothing interesting. For Windows Device 1 (DirectSound) it
    // is the window handle that DirectSound will attach to.
    long 			GetCurrentDevice(void *deviceParameter = NULL);

    // get device name
    // NOTE:	This function can be called before Open()
    //			Format of string is a zero terminated comma delinated C string.
    //			"platform,method,misc"
    //	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
    //			"WinOS,DirectSound,multi threaded"
    //			"WinOS,waveOut,multi threaded"
    //			"WinOS,VxD,low level hardware"
    //			"WinOS,plugin,Director"
    void			GetDeviceName(long deviceID, char *cName, unsigned long cNameLength);
		
    // Will return TRUE if the HAEMixer is already open
    HAE_BOOL			IsOpen(void) const;

    // Verify that the file passed in, is a valid audio bank file for HAE
    HAEErr				ValidateAudioFile(HAEPathName pAudioPathName);

    // open the mixer. Only one of these.
    // Note: it is valid to pass NULL to pAudioPathName. This means open the mixer
    // for sound effects and other types of mixing, but Midi and built in songs will
    // not work. If you want to associate a patch file to an open mixer, call 
    // ChangeAudioFile or ChangeAudioFileToMemory.
    HAEErr				Open(HAEPathName pAudioPathName = NULL,
					     HAEQuality q = HAE_22K,
					     HAETerpMode t = HAE_LINEAR_INTERPOLATION,
					     HAEReverbMode r = HAE_REVERB_TYPE_4,
					     HAEAudioModifiers am = (HAE_USE_16 | HAE_USE_STEREO),
					     short int maxMidiVoices = 28,
					     short int maxSoundVoices = 4,
					     short int mixLevel = 8,
					     HAE_BOOL engageAudio = TRUE
					     );
    // close the mixer
    void				Close(void);

    // reconfigure, by changing sample rates, interpolation modes, bit depth, etc based
    // upon performance of host computer
    void				PerformanceConfigure(void);

    // return a value that measures CPU performance
    unsigned long		MeasureCPUPerformance(void);

    // Flush read in or created cache for current AudioFile. TRUE allows cache, FALSE does not.
    void				SetCacheStatusForAudioFile(HAE_BOOL enableCache);
    HAE_BOOL			GetCacheStatusForAudioFile(void);

    // Flush read in or created cache for current AudioFile. TRUE allows cache, FALSE does not.
    void				SetCacheStatusAudioFileCache(HAE_BOOL enableCache);

    // change audio file
    HAEErr				ChangeAudioFile(HAEPathName pAudioPathName);

    // change audio file to work from memory. Assumes file was loaded into memory
    HAEErr				ChangeAudioFileToMemory(void * pAudioFile, unsigned long fileSize);

    // Get default bank URL
    HAEErr				GetURLFromAudioFile(char *pURL, unsigned long urlLength);
    // Get default bank name
    HAEErr 				GetNameFromAudioFile(char *cName, unsigned long cLength);

    // get audio file version numbers
    void				GetVersionFromAudioFile(short int *pVersionMajor, short int *pVersionMinor, short int *pVersionSubMinor);

    // Get names of songs that are included in the audio file. Call successively until
    // the name is zero
    void				GetSongNameFromAudioFile(char *cSongName, long *pID = NULL, HAEFileType *pSongType = NULL);

    // Get names of samples that are included in the audio file. Call successively until
    // the name is zero
    void				GetSampleNameFromAudioFile(char *cSampleName, long *pID);

    // Get names of instruments that are included in the audio file. Call successively until
    // the name is zero
    void				GetInstrumentNameFromAudioFile(char *cInstrumentName, long *pID);

    // Get names of instruments that are included in the audio file, referenced by ID only. Will return
    // and error if instrument not found.
    HAEErr				GetInstrumentNameFromAudioFileFromID(char *cInstrumentName, long theID);

    // change audio modes
    HAEErr				ChangeAudioModes(HAEQuality q, HAETerpMode t, HAEAudioModifiers am);

    // change voice allocation
    HAEErr				ChangeSystemVoices(	short int maxMidiVoices,
								short int maxSoundVoices,
								short int mixLevel);
    // get current system tick in microseconds
    unsigned long		GetTick(void);

    // get current system audio lantency. This is platform specific. Result is in
    // microseconds
    unsigned long		GetAudioLatency(void);

    // change reverb mode
    void				SetReverbType(HAEReverbMode r);
    HAEReverbMode		GetReverbType(void);

    // get and set the master mix volume. A volume level of 1.0
    // is normal, and volume level of 4.0 will overdrive 4 times
    void				SetMasterVolume(HAE_UNSIGNED_FIXED theVolume);
    HAE_UNSIGNED_FIXED	GetMasterVolume(void) const;

    // get and set the hardware volume. A volume level of 1.0 is
    // considered hardware full volume. Overdriving is up to the particular
    // hardware platform
    void				SetHardwareVolume(HAE_UNSIGNED_FIXED theVolume);
    HAE_UNSIGNED_FIXED	GetHardwareVolume(void) const;

    // get and set the hardware balance. Use -1.0 for full left, and 1.0 for full
    // right; use 0 for center
    void				SetHardwareBalance(HAE_FIXED theVolume);
    HAE_FIXED			GetHardwareBalance(void) const;

    // get and set the master sound effects volume. A volume level of 1.0
    // is normal, and volume level of 4.0 will overdrive 4 times
    void				SetMasterSoundEffectsVolume(HAE_UNSIGNED_FIXED theVolume);
    HAE_UNSIGNED_FIXED	GetMasterSoundEffectsVolume(void) const;

    // display feedback information
    // This will return the number of samples stored into the pLeft and pRight
    // arrays. Usually 1024. This returns the current data points being sent
    // to the hardware.
    short int			GetAudioSampleFrame(short int *pLeft, short int *pRight) const;

    // Get realtime information about the current synthisizer state
    void				GetRealtimeStatus(HAEAudioInfo *pStatus) const;

    // is the mixer connected to the audio hardware
    HAE_BOOL			IsAudioEngaged(void);
    // disengage from audio hardware
    void				DisengageAudio(void);
    // reengage to audio hardware
    void				ReengageAudio(void);

    // Set a interrupt level task callback
    void				SetTaskCallback(HAETaskCallbackPtr pCallback);
    // start the task callback
    void				StartTaskCallback(void);
    // Stop the task callback
    void				StopTaskCallback(void);

    // Set a output callback. This will call your output code. This is used to modify the
    // sample output before its sent to the hardware. Be very careful here. Don't use to
    // much time or the audio will skip
    void				SetOutputCallback(HAEOutputCallbackPtr pCallback);
    // start the output callback
    void				StartOutputCallback(void);
    // Stop the output callback
    void				StopOutputCallback(void);

    // start saving audio output to a file
    HAEErr 				StartOutputToFile(HAEPathName pAudioOutputFile);

    // Stop saving audio output to a file
    void				StopOutputToFile(void);

    // once started saving to a file, call this to continue saving to file
    HAEErr				ServiceAudioOutputToFile(void);

    // Call this during idle time to service audio streams, and other
    // idle time processes
    void				ServiceIdle(void);

    // get in realtime CPU load in microseconds used to create 11 ms worth
    // of sample data.
    unsigned long		GetCPULoadInMicroseconds(void);
    unsigned long		GetCPULoadInPercent(void);

    HAEAudioModifiers	GetModifiers(void);
    HAETerpMode			GetTerpMode(void);
    HAEQuality			GetQuality(void);

    short int			GetMidiVoices(void);
    short int			GetSoundVoices(void);
    short int			GetMixLevel(void);

    private:
	HAEQuality			iQuality;
    HAETerpMode			iTerpMode;

    HAEReverbMode		iReverbMode;
    HAEAudioModifiers	iModifiers;

    short int			iMidiVoices;
    short int			iSoundVoices;
    short int			iMixLevel;

    short int			songNameCount;
    HAEFileType			songNameType;

    short int			sampleNameCount;
    short int			instrumentNameCount;

    // hooks
    HAETaskCallbackPtr		pTask;
    HAEOutputCallbackPtr	pOutput;

    // banks
    void					*mOpenAudioFiles;
    unsigned short int		mOpenAudioFileCount;
    HAE_BOOL				mCacheStatus;

    // link to all objects
    void				*pTop;

    HAE_BOOL			mIsAudioEngaged;

    HAE_BOOL			mWritingToFile;
    void				*mWritingToFileReference;
};

// base class
class HAEAudioNoise
{
    friend class HAERMFFile;
    friend class HAEMidiFile;
    friend class HAEMidiDirect;
    friend class HAESound;
    friend class HAESoundStream;
    friend class HAEAudioMixer;
    friend class HAEGroup;
    public:
	HAEAudioNoise(HAEAudioMixer *pHAEAudioMixer, char const *cName);
    virtual	~HAEAudioNoise();
    HAEAudioMixer	*GetMixer(void);
    const char		*GetName(void)	const	{return mName;}
    private:
	char			mName[64];				// name of object
    HAEAudioMixer	*mAudioMixer;
    HAEAudioNoise	*pNext;					// link of objects starting from a mixer
    HAEAudioNoise	*pGroupNext;			// link of objects starting from a group
};

// Sound effects
class HAESound : public HAEAudioNoise
{
    friend class HAEGroup;
    public:
	HAESound(HAEAudioMixer *pHAEAudioMixer, 
		 char const *cName = 0L, void * userReference = 0);
    virtual			~HAESound();

    void					*GetReference(void)				const	{return userReference;}
    HAEDoneCallbackPtr		GetDoneCallback(void)			const	{return mDoneCallback;}
    HAELoopDoneCallbackPtr	GetLoopDoneCallback(void)			const	{return mLoopDoneCallback;}
    void					*GetDoneCallbackReference(void)	const	{return mCallbackReference;}
	
    HAEErr			AddSampleFrameCallback(unsigned long frame, HAESampleFrameCallbackPtr pCallback, void * pReference);
    HAEErr			RemoveSampleFrameCallback(unsigned long frame, HAESampleFrameCallbackPtr pCallback);
	
    // load a sample from a raw pointer. This will make a copy of the passed data,
    // so you can dispose of your origial pointer once this has been called.
    HAEErr			LoadCustomSample(void * sampleData,				// pointer to audio data
						 unsigned long frames, 			// number of frames of audio
						 unsigned short int bitSize, 	// bits per sample 8 or 16
						 unsigned short int channels, 	// mono or stereo 1 or 2
						 HAE_UNSIGNED_FIXED rate, 		// 16.16 fixed sample rate
						 unsigned long loopStart, 		// loop start in frames
						 unsigned long loopEnd);			// loop end in frames

    // load a sample from a resource 'snd' memory block.The memory will be deallocated when this
    // object is destroyed. Call start once loaded to start the playback.
    HAEErr			LoadResourceSample(void *pResource, unsigned long resourceSize);

    // load a sample playing from a formatted file. The memory will be deallocated when this
    // object is destroyed. Call start once loaded to start the playback.
    HAEErr			LoadFileSample(HAEPathName pWaveFilePath, HAEFileType fileType);

    // load a sample playing from a formatted block of memory. The memory will be deallocated 
    // when this object is destroyed. Call start once loaded to start the playback.
    HAEErr			LoadMemorySample(void *pMemoryFile, unsigned long memoryFileSize, HAEFileType fileType);

    // load a sample playing from the current audio file. The memory will be deallocated when this
    // object is destroyed.. Call start once loaded to start the playback.
    HAEErr			LoadBankSample(char *cName);

    // currently paused
    HAE_BOOL		IsPaused(void);

    // pause HAESound from playback
    void			Pause(void);

    // resume HAESound where paused
    void			Resume(void);

    // fade sample from current volume to silence
    void			Fade(HAE_BOOL doAsync = FALSE);

    // fade from current volume to specified volume
    void			FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync = FALSE);

    // This will setup a HAESound once data has been loaded with one of the Load... calls. Call __Start to start playing
    HAEErr			__Setup(HAE_UNSIGNED_FIXED sampleVolume = HAE_FIXED_1, 		// sample volume	(1.0)
					short int stereoPosition = HAE_CENTER_PAN,			// stereo placement -63 to 63
					void * refData = NULL, 								// callback reference
					HAELoopDoneCallbackPtr pLoopContinueProc = NULL,
					HAEDoneCallbackPtr pDoneProc = NULL,
					unsigned long startOffsetFrame = 0L,				// starting offset in frames
					HAE_BOOL stopIfPlaying = TRUE);						// TRUE will restart sound otherwise return and error

    // This will, given all the information about a sample, will play sample memory without
    // copying the data. Be carefull and do not dispose of the memory associated with this sample
    // while its playing. Call __Start to start sound
    HAEErr			__SetupCustom(void * sampleData,						// pointer to audio data
					      unsigned long frames, 						// number of frames of audio
					      unsigned short int bitSize, 				// bits per sample 8 or 16
					      unsigned short int channels, 				// mono or stereo 1 or 2
					      HAE_UNSIGNED_FIXED rate, 					// 16.16 fixed sample rate
					      unsigned long loopStart = 0L, 				// loop start in frames
					      unsigned long loopEnd = 0L,					// loop end in frames
					      HAE_UNSIGNED_FIXED sampleVolume = HAE_FIXED_1, 		// sample volume	(1.0)
					      short int stereoPosition = HAE_CENTER_PAN,	// stereo placement -63 to 63
					      void *refData = NULL, 						// callback reference
					      HAELoopDoneCallbackPtr pLoopContinueProc = NULL,
					      HAEDoneCallbackPtr pDoneProc = NULL,
					      HAE_BOOL stopIfPlaying = TRUE);

    HAEErr			__Start(void);

    // This will, given all the information about a sample, will play sample memory without
    // copying the data. Be carefull and do not dispose of the memory associated with this sample
    // while its playing.
    HAEErr			StartCustomSample(void * sampleData,						// pointer to audio data
						  unsigned long frames, 						// number of frames of audio
						  unsigned short int bitSize, 				// bits per sample 8 or 16
						  unsigned short int channels, 				// mono or stereo 1 or 2
						  HAE_UNSIGNED_FIXED rate, 					// 16.16 fixed sample rate
						  unsigned long loopStart = 0L, 				// loop start in frames
						  unsigned long loopEnd = 0L,					// loop end in frames
						  HAE_UNSIGNED_FIXED sampleVolume = HAE_FIXED_1, 		// sample volume	(1.0)
						  short int stereoPosition = HAE_CENTER_PAN,	// stereo placement -63 to 63
						  void *refData = NULL, 						// callback reference
						  HAELoopDoneCallbackPtr pLoopContinueProc = NULL,
						  HAEDoneCallbackPtr pDoneProc = NULL,
						  HAE_BOOL stopIfPlaying = TRUE);

    // This will start the first buffer of audio playing, and when its finished, it will call your callback
    // and start the second buffer of audio playing, when the second buffer is finished, it will call your
    // callback again, and start the first buffer of audio playing. If you return 0 in the callback's
    // pNewBufferSize the sample will then stop.
    //
    // This is a very low level call. Since the HAEAudioMixer looks 4 samples ahead for terping, make sure that
    // your buffers have copied an extra 4 samples of the previous data, otherwise you'll get clicks. Since
    // this is being called during the mixing stage, any delays will cause the mixer to skip or even shutdown.
    // Be very CPU sensitive here.
    //
    // The best way to use this function is to block move data into the opposite pointer that is playing.
    // .ie: fill buffer1 with something, start playing, fill buffer2 with something, accecpt callback, 
    // fill buffer1 and so on. Making sure that when you fill the buffers that you copy the previous 4 samples
    // into the next buffer before your new data.
    HAEErr			StartDoubleBuffer(void *buffer1,								// pointer to first buffer
						  void *buffer2,									// pointer to second buffer
						  unsigned long frames, 							// number of frames of audio
						  unsigned short int bitSize, 					// bits per sample 8 or 16
						  unsigned short int channels, 					// mono or stereo 1 or 2
						  HAE_UNSIGNED_FIXED rate, 						// 16.16 fixed sample rate
						  HAE_UNSIGNED_FIXED sampleVolume = HAE_FIXED_1, 	// sample volume	(1.0)
						  short int stereoPosition = HAE_CENTER_PAN,		// stereo placement -63 to 63
						  void * refData = NULL, 							// callback reference
						  HAEDoubleBufferCallbackPtr pDoubleBufferCallback = NULL,
						  HAE_BOOL stopIfPlaying = TRUE);

    // This will start a HAESound once data has been loaded
    HAEErr			Start(HAE_UNSIGNED_FIXED sampleVolume = HAE_FIXED_1, 		// sample volume	(1.0)
				      short int stereoPosition = HAE_CENTER_PAN,			// stereo placement -63 to 63
				      void * refData = NULL, 								// callback reference
				      HAELoopDoneCallbackPtr pLoopContinueProc = NULL,
				      HAEDoneCallbackPtr pDoneProc = NULL,
				      unsigned long startOffsetFrame = 0L,				// starting offset in frames
				      HAE_BOOL stopIfPlaying = TRUE);						// TRUE will restart sound otherwise return and error
			
    // This will stop a HAESound
    void			Stop(HAE_BOOL startFade = FALSE);

    // get information about the sample
    HAEErr			GetInfo(HAESampleInfo *pInfo);

    // Set a call back when song is done
    void			SetDoneCallback(HAEDoneCallbackPtr pDoneProc, void * pReference);

    // Get the position of a audio stream in samples
    unsigned long	GetPlaybackPosition(void);

    // Get the mixer pointer for this sample as its being mixed. This pointer is 
    // the actual pointer used by the mixer. This will change if this object is
    // an double buffer playback.
    void *			GetSamplePointerFromMixer(void);

    // Returns TRUE or FALSE if a given HAESound is still active
    HAE_BOOL		IsPlaying(void);

    // Returns the opposite of IsPlaying
    HAE_BOOL		IsDone(void);

    // Set the volume level of a HAESound
    void			SetVolume(HAE_UNSIGNED_FIXED newVolume);

    // Get the volume level of a HAESound
    HAE_UNSIGNED_FIXED	GetVolume(void);

    // Set the sample rate of a HAESound
    void			SetRate(HAE_UNSIGNED_FIXED newRate);

    // Get the sample rate of a HAESound
    HAE_UNSIGNED_FIXED	GetRate(void);

    // Set the stereo position of a HAESound (-63 left to 63 right, 0 is middle)
    void			SetStereoPosition(short int stereoPosition);

    // Set the stereo position of a HAESound (-63 left to 63 right, 0 is middle)
    short int		GetStereoPosition(void);

    // Enable/Disable reverb on this particular HAESound
    void			SetReverb(HAE_BOOL useReverb);
    HAE_BOOL 		GetReverb(void);
    // get/set reverb mix amount of this particular HAESound
    short int		GetReverbAmount(void);
    void 			SetReverbAmount(short int reverbAmount);

    // get/set low pass filter amount of this particular HAESound
    // amount range is -255 to 255
    short int		GetLowPassAmountFilter(void);
    void 			SetLowPassAmountFilter(short int lowpassamount);
    // get/set resonance filter amount of this particular HAESound
    // resonance range is 0 to 256
    short int		GetResonanceAmountFilter(void);
    void 			SetResonanceAmountFilter(short int resonanceAmount);
    // get/set frequency filter amount of this particular HAESound
    // Range is 512 to 32512
    short int		GetFrequencyAmountFilter(void);
    void 			SetFrequencyAmountFilter(short int frequencyAmount);

    // Get the sample point offset by a sample frame count. This is a pointer to
    // the actual PCM audio data stored. This is used by the mixer when playing back.
    void *			GetSamplePointer(unsigned long sampleFrame = 0L);

    // set the loop points in sample frames. Don't set a loop point starting with zero
    HAEErr			SetSampleLoopPoints(unsigned long start = 0L, unsigned long end = 0L);

    // Get the current loop points in sample frames
    HAEErr			GetSampleLoopPoints(unsigned long *pStart, unsigned long *pEnd);


    //private:
    // This function is called when a sample finishes naturally or is stopped. Note: If you
    // override this function and don't call the parent, auto voice allocation will fail.
    void					DefaultSampleDoneCallback(void);

    private:
	HAEErr			PreStart(HAE_UNSIGNED_FIXED sampleVolume,
					 short int stereoPosition,
					 void * refData,
					 HAELoopDoneCallbackPtr pLoopContinueProc,
					 HAEDoneCallbackPtr pDoneProc,
					 HAE_BOOL stopIfPlaying);

    HAE_BOOL				mReverbState;
    short int				mReverbAmount;
    short int				mLowPassAmount;
    short int				mResonanceAmount;
    short int				mFrequencyAmount;
    void					*userReference;
    unsigned long			pauseVariable;
    void					*pFileVariables;
    void					*pSoundVariables;
    void					*pSampleFrameVariable;
    HAEDoneCallbackPtr		mDoneCallback;
    HAELoopDoneCallbackPtr	mLoopDoneCallback;
    void					*mCallbackReference;
    //$$fb itanium port: mSoundVoiceReference should match VOICE_REFERENCE
    UINT32					mSoundVoiceReference;
    //void*					mSoundVoiceReference;
    HAE_UNSIGNED_FIXED		mSoundVolume;
    short int				mStereoPosition;

};

// Mod file playback. CAN ONLY PLAY ONE MOD FILE AT A TIME
// Can play MOD file types: MOD, STM, S3M, ULT
class HAEMod : public HAEAudioNoise
{
    public:
    HAEMod(HAEAudioMixer *pHAEAudioMixer, 
	   char const *cName = 0L, void * userReference = 0);
    virtual			~HAEMod();

    void			*GetReference(void)	const	{return userReference;}

    // load a sample playing from a file. The memory will be deallocated when this
    // object is destroyed. Call start once loaded to start the playback.
    HAEErr			LoadFromFile(HAEPathName pModFilePath);

    // Load memory mapped MOD pointer into HAEMod object. This will parse the MOD file and get
    // it ready for playing. You can dispose of the data passed once this method returns
    HAEErr			LoadFromMemory(void const* pModData, unsigned long modSize);

    // get info about this song file. Will return a 'C' string.
    // MOD only supports TITLE_INFO and COMPOSER_NOTES_INFO
    HAEErr			GetInfo(HAEInfoTypes infoType, char *cInfo);

    // get size of info about this song file. Will an unsigned long
    unsigned long	GetInfoSize(HAEInfoTypes infoType);

    // currently paused
    HAE_BOOL		IsPaused(void);

    // pause HAEMod from playback
    void			Pause(void);

    // resume HAEMod where paused
    void			Resume(void);

    // fade sample from current volume to silence
    void			Fade(HAE_BOOL doAsync = FALSE);

    // fade from current volume to specified volume
    void			FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync = FALSE);

    // This will start a HAEMod once data has been loaded
    HAEErr			Start(HAEDoneCallbackPtr pDoneProc = NULL);

    // Set a call back when song is done
    void			SetDoneCallback(HAEDoneCallbackPtr pDoneProc, void * pReference);

    // This will stop a HAEMod
    void			Stop(HAE_BOOL startFade = FALSE);

    // Returns TRUE or FALSE if a given HAEMod is still active
    HAE_BOOL		IsPlaying(void);
    // returns the opposite of IsPlaying
    HAE_BOOL		IsDone(void);
    // set song master tempo. (1.0 uses songs encoded tempo, 2.0 will play
    // song twice as fast, and 0.5 will play song half as fast
    void			SetMasterTempo(HAE_UNSIGNED_FIXED tempoFactor);

    // sets tempo in beats per minute
    void			SetTempoInBeatsPerMinute(unsigned long newTempoBPM);
    // returns tempo in beats per minute
    unsigned long	GetTempoInBeatsPerMinute(void);

    // pass TRUE to entire loop song, FALSE to not loop
    void			SetLoopFlag(HAE_BOOL loop);

    HAE_BOOL		GetLoopFlag(void);

    // Set the volume level of a HAEMod
    void			SetVolume(HAE_UNSIGNED_FIXED newVolume);

    // Get the volume level of a HAEMod
    HAE_UNSIGNED_FIXED		GetVolume(void);

    private:
	void			*userReference;
    void			*pSoundVariables;
    long			pauseVariable;
};



// Audio Sample Data Format. (ASDF)
// Support for 8, 16 bit data, mono and stereo. Can be extended for multi channel beyond 2 channels, but
// not required at the moment.
//
//	DATA BLOCK
//		8 bit mono data
//			ZZZZZZZÉ
//				Where Z is signed 8 bit data
//
//		16 bit mono data
//			WWWWWÉ
//				Where W is signed 16 bit data
//
//		8 bit stereo data
//			ZXZXZXZXÉ
//				Where Z is signed 8 bit data for left channel, and X is signed 8 bit data for right channel.
//
//		16 bit stereo data
//			WQWQWQÉ
//				Where W is signed 16 bit data for left channel, and Q is signed 16 bit data for right channel.
//



typedef enum
{
    HAE_STREAM_NULL				=	0,
    HAE_STREAM_CREATE,
    HAE_STREAM_DESTROY,
    HAE_STREAM_GET_DATA,
    HAE_STREAM_GET_SPECIFIC_DATA,
    HAE_STREAM_HAVE_DATA				//	Used for HAECaptureStream
} HAEStreamMessage;

// The HAEStreamObjectProc callback is called to allocate buffer memory, get the next block of data to stream and
// mix into the final audio output, and finally dispose of the memory block. All messages will happen at 
// non-interrupt time. All messages will be called with the structure HAEStreamData.
//
// INPUT:
// Message
//	HAE_STREAM_CREATE
//		Use this message to create a block a data with a length of dataLength. Keep in mind that dataLength
//		is always total number of samples,  not bytes allocated. Allocate the block of data into the Audio Sample 
//		Data Format based upon dataBitSize and channelSize. Store the pointer into pData.
//
//	HAE_STREAM_DESTROY
//		Use this message to dispose of the memory allocated. pData will contain the pointer allocated.
//		dataLength will be the sample size not the buffer size. ie. for 8 bit data use dataLength, 
//		for 16 bit mono data double dataLength.
//
//	HAE_STREAM_GET_DATA
//		This message is called whenever the streaming object needs a new block of data. Right after HAE_STREAM_CREATE
//		is called, HAE_STREAM_GET_DATA will be called twice. Fill pData with the new data to be streamed.
//		Set dataLength to the amount of data put into pData.
//
//	HAE_STREAM_GET_SPECIFIC_DATA
//		This message is optional. It will be called when a specific sample frame and length needs to be captured.
//		The method GetSampleData will call this message. If you do not want to implement this message
//		return an error of HAE_NOT_SETUP. Fill (pData) with the new sample data betweem sample frames (startSample)
//		and (endSample). Set (dataLength) to the amount of data put into (pData).
//		Note: this message will should not be called to retrive sample data for streaming. Its only used to capture
//		a range of data inside of a stream for display or other processes.
//
//	HAE_STREAM_HAVE_DATA
//		This message is used with an HAECapture object when audio is being captured into the buffer and the
//		buffer is full.
//
// OUTPUT:
// returns
//	HAE_NO_ERR
//		Everythings ok
//
//	HAE_STREAM_STOP_PLAY
//		Everything is fine, but stop playing stream
//
//	HAE_MEMORY_ERR
//		Couldn't allocate memory for buffers.
//
//	HAE_PARAM_ERR
//		General purpose error. Something wrong with parameters passed.
//
//	HAE_NOT_SETUP
//		If message HAE_STREAM_GET_SPECIFIC_DATA is called and it is not implemented you should return this error.
//

struct HAEStreamData
{
    long				userReference;		// IN for all messages. userReference is passed in at AudioStreamStart
    void				*pData;				// OUT for HAE_STREAM_CREATE, IN for HAE_STREAM_DESTROY and HAE_STREAM_GET_DATA
    unsigned long		dataLength;			// OUT for HAE_STREAM_CREATE, IN for HAE_STREAM_DESTROY. IN and OUT for HAE_STREAM_GET_DATA
    HAE_UNSIGNED_FIXED	sampleRate;			// IN for all messages. Fixed 16.16 value
    char				dataBitSize;		// IN for HAE_STREAM_CREATE. Not used elsewhere
    char				channelSize;		// IN for HAE_STREAM_CREATE. Not used elsewhere
    unsigned long		startSample;		// IN for HAE_STREAM_GET_SPECIFIC_DATA only.
    unsigned long		endSample;			// IN for HAE_STREAM_GET_SPECIFIC_DATA only
};
typedef struct HAEStreamData	HAEStreamData;

typedef HAEErr (*HAEStreamObjectProc)(HAEStreamMessage message, HAEStreamData *pAS);

// Audio Stream
class HAESoundStream : public HAEAudioNoise
{
    friend class HAEGroup;
    public:
	HAESoundStream(HAEAudioMixer *pHAEAudioMixer,
		       char const *cName = 0L, void * userReference = 0);
    virtual			~HAESoundStream();

    void			*GetReference(void)	const	{return mUserReference;}
    HAEStreamObjectProc GetCallbackProc(void) const {return mCallbackProc;}

    // setup a streaming file. Does not require a callback control
    HAEErr			SetupFileStream(	HAEPathName pWaveFilePath, 
							HAEFileType fileType,
							unsigned long bufferSize,			// temp buffer to read file
							HAE_BOOL loopFile);				// TRUE will loop file

    // Multi source user config based streaming
    // This will start a streaming audio object.
    // INPUT:
    //	pProc			is a HAEStreamObjectProc proc pointer. At startup of the streaming the proc will be called
    //					with HAE_STREAM_CREATE, then followed by two HAE_STREAM_GET_DATA calls to get two buffers 
    //					of data and finally HAE_STREAM_DESTROY when finished.
    //	bufferSize		total size of buffer to work with. This will not allocate memory, instead it will call
    //					your control callback with a HAE_STREAM_CREATE with a size
    HAEErr			SetupCustomStream(	HAEStreamObjectProc pProc, 	// control callback
							unsigned long bufferSize, 		// buffer size 
							HAE_UNSIGNED_FIXED sampleRate,	// Fixed 16.16
							char dataBitSize,				// 8 or 16 bit data
							char channelSize);				// 1 or 2 channels of date

    // This will return the last HAESoundStream error
    HAEErr			LastError(void);

    // This will preroll a stream (get everything ready for syncronized start)
    HAEErr 			Preroll(void);

    // This will start a stream once data has been loaded
    HAEErr			Start(void);

    // get information about the current stream
    HAEErr			GetInfo(HAESampleInfo *pInfo);

    // This will stop a streaming audio object and free any memory.
    void			Stop(HAE_BOOL startFade = FALSE);

    // This will stop and flush the current stream and force a read of data. This
    // will cause gaps in the audio.
    void			Flush(void);

    // Get the position of a audio stream in samples
    unsigned long	GetPlaybackPosition(void);

    // currently paused
    HAE_BOOL		IsPaused(void);

    // Pause HAESoundStream from playback
    void			Pause(void);

    // Resume HAESoundStream from where paused.
    void			Resume(void);

    // Returns TRUE or FALSE if a given HAESoundStream is still active
    HAE_BOOL		IsPlaying(void);

    // Returns the opposite of IsPlaying
    HAE_BOOL		IsDone(void);

    // Returns TRUE if a given HAESoundStream is valid
    HAE_BOOL		IsValid(void);

    // Set the volume level of a audio stream
    void			SetVolume(HAE_UNSIGNED_FIXED newVolume);

    // Get the volume level of a audio stream
    HAE_UNSIGNED_FIXED		GetVolume(void);

    // fade stream from current volume to silence
    void			Fade(HAE_BOOL doAsync = FALSE);

    // fade stream from current volume to specified volume
    void			FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync = FALSE);

    // Set the sample rate of a audio stream
    void			SetRate(HAE_UNSIGNED_FIXED newRate);

    // Get the sample rate of a audio stream
    HAE_UNSIGNED_FIXED		GetRate(void);

    // Set the stereo position of a audio stream (-63 left to 63 right, 0 is middle)
    void			SetStereoPosition(short int stereoPosition);

    // Get the stereo position of a audio stream (-63 left to 63 right, 0 is middle)
    short int		GetStereoPosition(void);

    // Enable/Disable reverb on this particular audio stream
    void			SetReverb(HAE_BOOL useReverb);
    HAE_BOOL 		GetReverb(void);
    // get reverb mix amount of this particular HAESoundStream
    short int		GetReverbAmount(void);
    void 			SetReverbAmount(short int reverbAmount);

    // get/set low pass filter amount of this particular HAESoundStream
    // amount range is -255 to 255
    short int		GetLowPassAmountFilter(void);
    void 			SetLowPassAmountFilter(short int lowpassamount);
    // get/set resonance filter amount of this particular HAESoundStream
    // resonance range is 0 to 256
    short int		GetResonanceAmountFilter(void);
    void 			SetResonanceAmountFilter(short int resonanceAmount);
    // get/set frequency filter amount of this particular HAESoundStream
    // Range is 512 to 32512
    short int		GetFrequencyAmountFilter(void);
    void 			SetFrequencyAmountFilter(short int frequencyAmount);

    private:
	HAE_BOOL				mPrerolled;
    HAE_BOOL				mReverbState;
    short int				mReverbAmount;
    short int				mLowPassAmount;
    short int				mResonanceAmount;
    short int				mFrequencyAmount;
    HAE_UNSIGNED_FIXED		mVolumeState;
    short int				mPanState;
    HAESampleInfo			mStreamSampleInfo;
    unsigned long			mPauseVariable;
    unsigned long			mSoundStreamVoiceReference;
    void					*mUserReference;
    HAEStreamObjectProc		mCallbackProc;
};



// Midi Device class for direct control
class HAEMidiDirect : public HAEAudioNoise
{
    friend class HAEMidiFile;
    friend class HAERMFFile;
    friend class HAEAudioMixer;
    public:
	HAEMidiDirect(HAEAudioMixer *pHAEAudioMixer, 
		      char const* cName = 0L, void *userReference = 0);
    virtual		~HAEMidiDirect();

    HAEErr			Open(HAE_BOOL loadInstruments);
    void			Close(void);

    HAE_BOOL		IsLoaded(void);

    void			*GetReference(void)	const	{return mReference;}

    void			*GetSongVariables(void) const	{return m_pSongVariables;}

    // set song volume. You can overdrive by passing values larger than 1.0
    void			SetVolume(HAE_UNSIGNED_FIXED volume);
    // get the song volume
    HAE_UNSIGNED_FIXED		GetVolume(void);

    // Set the master stereo position of a HAEMidiDirect (-63 left to 63 right, 0 is middle)
    void			SetStereoPosition(short int stereoPosition);

    // Set the master stereo position of a HAEMidiDirect (-63 left to 63 right, 0 is middle)
    short int		GetStereoPosition(void);

    // return note offset in semi tones	(-12 is down an octave, 12 is up an octave)
    long			GetPitchOffset(void);
    // set note offset in semi tones	(-12 is down an octave, 12 is up an octave)
    void			SetPitchOffset(long offset);

    // If allowPitch is FALSE, then "SetPitchOffset" will have no effect on passed 
    // channel (0 to 15)
    void			AllowChannelPitchOffset(unsigned short int channel, HAE_BOOL allowPitch);
    // Return if the passed channel will allow pitch offset
    HAE_BOOL		DoesChannelAllowPitchOffset(unsigned short int channel);

    // Mute and unmute channels (0 to 15)
    void			MuteChannel(unsigned short int channel);
    void			UnmuteChannel(unsigned short int channel);
    void			GetChannelMuteStatus(HAE_BOOL *pChannels);

    void			SoloChannel(unsigned short int channel);
    void			UnSoloChannel(unsigned short int channel);
    void			GetChannelSoloStatus(HAE_BOOL *pChannels);


    // Use these functions to drive the synth engine directly
    HAEErr			LoadInstrument(HAE_INSTRUMENT instrument);
    HAEErr			UnloadInstrument(HAE_INSTRUMENT instrument);
    HAE_BOOL		IsInstrumentLoaded(HAE_INSTRUMENT instrument);

    // load an instrument with custom patch data.
    HAEErr			LoadInstrumentFromData(HAE_INSTRUMENT instrument, void *data, unsigned long dataSize);
    // create a data block that is the instrument. Data block then can be passed into LoadInstrumentFromData
    HAEErr			CreateInstrumentAsData(HAE_INSTRUMENT instrument, void **pData, unsigned long *pDataSize);

    HAE_BOOL		GetCacheSample(void);
    void			SetCacheSample(HAE_BOOL cacheSamples);

    static HAE_INSTRUMENT	TranslateBankProgramToInstrument(unsigned short bank, 
								 unsigned short program, 
								 unsigned short channel,
								 unsigned short note = 0);
    HAEErr			RemapInstrument(HAE_INSTRUMENT from, HAE_INSTRUMENT to);

    // get current midi tick in microseconds
    unsigned long	GetTick(void);

    // set queue control of midi commmands. Use TRUE to queue commands, FALSE to
    // send directly to engine. Default is TRUE
    void			SetQueue(HAE_BOOL useQueue);

    // Get current queue control status. TRUE is queuing enabled, FALSE is no queue
    HAE_BOOL		GetQueue(void)	const	{return mQueueMidi;}

    // Lock/Unlock queue processing. Storing of queue'd events still happens.
    // you need to match Locks with unlocks. You can nest and they are accumulitive
    void			LockQueue(void);
    void			UnlockQueue(void);

    // Get the current Midi controler value. Note this is not time based
    char			GetControlValue(unsigned char channel, 
						unsigned char controller);

    // Get the current Midi program and bank values
    void			GetProgramBank(unsigned char channel,
					       unsigned char *pProgram,
					       unsigned char *pBank);
	
    void			GetPitchBend(unsigned channel, unsigned char *pLSB, unsigned char *pMSB);

    // given a midi stream, parse it out to the various midi functions
    // for example:
    // 0x92			0x50		0x7F		0x00
    // comandByte	data1Byte	data2Byte	data3Byte
    // Note 80 on with a velocity of 127 on channel 2
    void			ParseMidiData(unsigned char commandByte, unsigned char data1Byte, 
					      unsigned char data2Byte, unsigned char data3Byte,
					      unsigned long time = 0);

    // if you pass 0 for time the current time will be passed
    // The channel variable is 0 to 15. Channel 9 is percussion for example.
    // The programNumber variable is a number from 0-127
    // If queuing is disabled, by calling SetQueue(FALSE), time has no meaning.
    void			NoteOff(unsigned char channel, 
					unsigned char note, 
					unsigned char velocity,
					unsigned long time = 0);

    // note on that checks to see if an instrument needs to be loaded. DO NOT call this
    // during an interrupt, as it might load memory. This only works when queuing is enabled
    void 			NoteOnWithLoad(unsigned char channel, 
					       unsigned char note, 
					       unsigned char velocity,
					       unsigned long time = 0);

    void			NoteOn(unsigned char channel, 
				       unsigned char note, 
				       unsigned char velocity,
				       unsigned long time = 0);

    void			KeyPressure(unsigned char channel, 
					    unsigned char note, 
					    unsigned char pressure,
					    unsigned long time = 0);

    void			ControlChange(unsigned char channel, 
					      unsigned char controlNumber,
					      unsigned char controlValue, 
					      unsigned long time = 0);

    void			ProgramBankChange(unsigned char channel,
						  unsigned char programNumber,
						  unsigned char bankNumber,
						  unsigned long time = 0);

    void			ProgramChange(unsigned char channel, 
					      unsigned char programNumber,
					      unsigned long time = 0);

    void			ChannelPressure(unsigned char channel, 
						unsigned char pressure, 
						unsigned long time = 0);

    void			PitchBend(unsigned char channel, 
					  unsigned char lsb, 
					  unsigned char msb,
					  unsigned long time = 0);

    void			AllNotesOff(unsigned long time = 0);


    private:
	void			*m_pSongVariables;
    void			*m_pPerformanceVariables;
    unsigned long	mPerformanceVariablesLength;

    void			*mReference;
    HAE_BOOL		mQueueMidi;
};

// Midi Device class for Standard Midi files
class HAEMidiFile : public HAEMidiDirect
{
    public:
    HAEMidiFile(HAEAudioMixer *pHAEAudioMixer, 
		char const* cName = 0L, void *userReference = 0);
    virtual	~HAEMidiFile();

    // get embedded voice configuration from file
    short int		GetEmbeddedMidiVoices(void);
    short int		GetEmbeddedMixLevel(void);
    short int		GetEmbeddedSoundVoices(void);
    // set embedded voice configuration that is in file
    // NOTE: Does not change current mixer settings only when Start is called
    void			SetEmbeddedMidiVoices(short int midiVoices);
    void			SetEmbeddedMixLevel(short int mixLevel);
    void			SetEmbeddedSoundVoices(short int soundVoices);

    // get/set embedded reverb type.
    // NOTE: Does not change current mixer settings only when Start is called
    HAEReverbMode	GetEmbeddedReverbType(void);
    void			SetEmbeddedReverbType(HAEReverbMode verb);

    // get/set embedded volume type.
    // NOTE: Does not change current settings only when Start is called
    void			SetEmbeddedVolume(HAE_UNSIGNED_FIXED volume);
    HAE_UNSIGNED_FIXED		GetEmbeddedVolume(void);

    // get info about this song file. Will return a 'C' string
    HAEErr			GetInfo(HAEInfoTypes infoType, char *cInfo);

    // get size of info about this song file. Will an unsigned long
    unsigned long	GetInfoSize(HAEInfoTypes infoType);

    // load a midi file into memory from a file
    HAEErr			LoadFromFile(HAEPathName pMidiFilePath, HAE_BOOL ignoreBadInstruments = TRUE);

    // load a midi file into memory from the current audio file	by name
    HAEErr			LoadFromBank(char *cName, HAE_BOOL ignoreBadInstruments = TRUE);

    // load a midi file into memory from the current audio file	by ID
    HAEErr 			LoadFromID(unsigned long id, HAE_BOOL ignoreBadInstruments = TRUE);

    // Given a pointer to a file, load it into a HAEMidiFile object. 
    //
    // If duplicateObject is TRUE, then the pointer passed in will be duplicated. 
    // You can free the memory pointer passed after success.
    // If FALSE the user pointer will be used, but
    // not copied. Don't delete the object until after you have deleted this object.
    HAEErr			LoadFromMemory(void const* pMidiData, unsigned long midiSize, 
					       HAE_BOOL duplicateObject = TRUE, HAE_BOOL ignoreBadInstruments = TRUE);

    // free song from memory
    void			Unload(void);
    // start song. If useEmbeddedMixerSettings is TRUE then the mixer will be reconfigured
    // to the embedded song settings. If false, then song will attempt to start with the
    // current mixer configuration.
    HAEErr			Start(HAE_BOOL useEmbeddedMixerSettings = TRUE, HAE_BOOL autoLevel = FALSE);
    // end song. If startFade is TRUE, song will fade out and then stop. This is asyncronous
    void			Stop(HAE_BOOL startFade = FALSE);

    // fade song from current volume to silence. If doAsync is FALSE this function will
    // return only when the fade is finished. This will not stop the song.
    void			Fade(HAE_BOOL doAsync = FALSE);
    // fade song from current volume to specified volume
    void			FadeTo(HAE_FIXED destVolume, HAE_BOOL doAsync = FALSE);

    // pause and resume song playback
    void			Pause(void);
    void			Resume(void);
    HAE_BOOL		IsPaused(void);

    // Set song loop playback control
    void			SetLoopMax(short int maxLoops);
    short int 		GetLoopMax(void);

    // pass TRUE to entire loop song, FALSE to not loop
    void			SetLoopFlag(HAE_BOOL loop);

    HAE_BOOL		GetLoopFlag(void);

    // get ticks in midi ticks of length of song
    unsigned long	GetTickLength(void);
    // set the current playback position of song in midi ticks
    HAEErr			SetTickPosition(unsigned long ticks);
    // get the current playback position of a song in midi ticks
    unsigned long	GetTickPosition();

    // get ticks in microseconds of length of song
    unsigned long	GetMicrosecondLength(void);
    // set the current playback position of song in microseconds
    HAEErr			SetMicrosecondPosition(unsigned long ticks);
    // get the current playback position of a song in microseconds
    unsigned long	GetMicrosecondPosition();

    // return the instruments required to play this song. Make sure pArray768 is
    // an array of HAE_INSTRUMENT with 768 elements.
    HAEErr			GetInstruments(HAE_INSTRUMENT *pArray768, short int *pReturnedCount);

    // Set a call back during song playback. Pass NULL to clear callback
    void					SetTimeCallback(HAETimeCallbackPtr pSongCallback, void *pReference);

    HAETimeCallbackPtr		GetTimeCallback(void)	{return m_pSongTimeCallback;}
    void					*GetTimeCallbackReference(void)	{return m_pTimeCallbackReference;}

    // Set a call back during song playback for meta events. Pass NULL to clear callback.
    void					SetMetaEventCallback(HAEMetaEventCallbackPtr pSongCallback, void * pReference);
    HAEMetaEventCallbackPtr	GetMetaCallback(void)			{return m_pSongMetaCallback;}
    void					*GetMetaCallbackReference(void)	{return mSongMetaReference;}

    // Set a call back on midi controler events
    void					SetControlCallback(short int controller, 		// controller to connect
								   HAEControlerCallbackPtr pControllerCallback,
								   void * pReference);
    HAEControlerCallbackPtr	GetControlCallback(void)			{return m_pControllerCallbackProc;}
    void					*GetControlCallbackReference(void)	{return m_pControllerCallbackReference;}

    // Set a call back when song is done. Pass NULL to clear event
    void					SetDoneCallback(HAEDoneCallbackPtr pSongCallback, void * pReference);
    HAEDoneCallbackPtr		GetDoneCallback(void)			{return m_pSongCallback;}
    void					*GetDoneCallbackReference(void)	{return mSongCallbackReference;}

    // poll to see if song is done
    HAE_BOOL		IsDone(void);

    // poll to see if a song is playing
    HAE_BOOL		IsPlaying(void);

    // set song master tempo. (1.0 uses songs encoded tempo, 2.0 will play
    // song twice as fast, and 0.5 will play song half as fast
    void			SetMasterTempo(HAE_UNSIGNED_FIXED tempoFactor);
    // get song master tempo
    HAE_UNSIGNED_FIXED		GetMasterTempo(void);

    // Sets tempo in microsecond per quarter note
    void			SetTempo(unsigned long newTempo);
    // returns tempo in microsecond per quarter note
    unsigned long	GetTempo(void);

    // sets tempo in beats per minute
    void			SetTempoInBeatsPerMinute(unsigned long newTempoBPM);
    // returns tempo in beats per minute
    unsigned long	GetTempoInBeatsPerMinute(void);

    // Mute and unmute tracks (0 to 64)
    void			MuteTrack(unsigned short int track);
    void			UnmuteTrack(unsigned short int track);
    void			GetTrackMuteStatus(HAE_BOOL *pTracks);

    void			SoloTrack(unsigned short int track);
    void			UnSoloTrack(unsigned short int track);
    void			GetSoloTrackStatus(HAE_BOOL *pTracks);

    private:
	void					*m_pControllerCallbackReference;
    HAEControlerCallbackPtr	m_pControllerCallbackProc;

    void					*m_pTimeCallbackReference;
    HAETimeCallbackPtr		m_pSongTimeCallback;

    void					*mSongCallbackReference;
    HAEDoneCallbackPtr		m_pSongCallback;

    void					*mSongMetaReference;
    HAEMetaEventCallbackPtr	m_pSongMetaCallback;
};


// Midi Device class for RMF files
class HAERMFFile : public HAEMidiFile
{
    public:
    HAERMFFile(HAEAudioMixer *pHAEAudioMixer, 
	       char const* cName = 0L, void *userReference = 0);
    virtual	~HAERMFFile();

    // is this RMF file encrypted?
    HAE_BOOL		IsEncrypted() const;
    // is this RMF file compressed?
    HAE_BOOL		IsCompressed() const;
	
    // load a RMF into memory from the current audio file by name. This assumes
    // that you have imported the RMF file into the bank
    HAEErr			LoadFromBank(char const* cName, HAE_BOOL ignoreBadInstruments = TRUE);

    // load a RMF file into memory from the current audio file	by ID
    HAEErr			LoadFromID(unsigned long id, HAE_BOOL ignoreBadInstruments = TRUE);

    // load a RMF file into memory from a file.
    HAEErr			LoadFromFile(const HAEPathName pRMFFilePath, HAE_BOOL ignoreBadInstruments = TRUE);

    // Given a pointer to a file, load it into a HAERMFFile object. 
    //
    // If duplicateObject is TRUE, then the pointer passed in will be duplicated. 
    // You can free the memory pointer passed after success.
    // If FALSE the user pointer will be used, but
    // not copied. Don't delete the object until after you have deleted this object.
    HAEErr			LoadFromMemory(void const* pRMFData, unsigned long rmfSize, 
					       HAE_BOOL duplicateObject = TRUE,
					       HAE_BOOL ignoreBadInstruments = TRUE);

    private:
	void	*m_pRMFDataBlock;
};

// Class designed to associate HAE objects together and start/stop/change
// them in sync. Create HAESound/HAESoundStream objects, associate them
// then perform functions. When deleting the HAEGroup make sure you delete
// the objects you've added. Deleting an HAEGroup will not delete the associated
// object
class HAEGroup : public HAEAudioNoise
{
    public:
    HAEGroup(HAEAudioMixer *pHAEAudioMixer, 
	     char const* cName = 0L, void *userReference = 0);
    ~HAEGroup();

    void			*GetReference(void)				const	{return userReference;}

    // Associate an HAE object to this group
    HAEErr			AddSound(HAESound *pSound);
    HAEErr			AddStream(HAESoundStream *pStream);

    // Disassociate an HAE object from this group
    HAEErr			RemoveSound(HAESound *pSound);
    HAEErr			RemoveStream(HAESoundStream *pStream);

    HAEErr			Start(void);
    void			Stop(HAE_BOOL startFade = FALSE);

    // Set the stereo position an entire group (-63 left to 63 right, 0 is middle)
    void			SetStereoPosition(short int stereoPosition);

    // Set the volume level of an entire group
    void			SetVolume(HAE_UNSIGNED_FIXED newVolume);

    // Enable/Disable reverb of an entire group
    void			SetReverb(HAE_BOOL useReverb);
    // set reverb mix amount of an entire group
    void 			SetReverbAmount(short int reverbAmount);

    private:
	void			*userReference;

    void			*linkedPlaybackReference;

    HAESound		*m_topSound;
    HAESoundStream	*m_topStream;
};
#endif	// HAE_AUDIO
