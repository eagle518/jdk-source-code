/*
 * @(#)X_Formats.h	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	X_Formats.h
**
**		This is platform independent file and data formats for SoundMusicSys
**
**	History	-
**	6/30/96		Created
**	7/3/96		Added packing pragmas
**	7/14/96		Removed PRAGMA_ALIGN_SUPPORTED
**	8/19/96		Added compressionType to SampleDataInfo
**	10/23/96	Changed GetKeySplitFromPtr to XGetKeySplitFromPtr
**	12/5/96		Added locked flags for songs and instruments
**	12/10/96	Added ID_RMF type
**	12/19/96	Added Sparc pragmas
**	12/30/96	Changed copyrights
**	1/2/97		Added ID_MOD type
**	1/3/97		Added XGetSongInformation
**	1/6/97		Added songType to XNewSongPtr
**	1/7/97		Changed structures typedef forms
**	1/12/97		Broke SongResource into two types: SongResource_SMS and SongResource_RMF
**	1/13/97		Added XGetSongResourceInfo & XDisposeSongResourceInfo & 
**				XGetSongResourceObjectID & XGetSongPerformanceSettings &
**				XGetSongResourceObjectType
**	1/18/97		Added XCheckAllInstruments & XCheckValidInstrument
**	1/24/97		Added SongResource_MOD
**	1/29/97		Added XGetSongInstrumentList
**				Added XGetMidiData
**				Added ID_ESND and ID_EMID types
**	1/30/97		Added XGetSoundResourceByName & XGetSoundResourceByID
**				Added XGetSongVoices & XSetSongVoices
**	2/14/97		Added volumeGain in SongResource_RMF
**				Added XSetSongVolumeGain & XGetSongVolumeGain
**	2/15/97		Changed volumeGain to songVolume, changed XSetSongVolumeGain & 
**				XGetSongVolumeGain to XSetSongVolume & XGetSongVolume
**	3/13/97		Added embeddedSong to SongResource_RMF structure
**				Added XGetSongEmbeddedStatus & XSetSongEmbeddedStatus
**	3/14/97		Added XIsSoundUsedInInstrument & XRenumberSampleInsideInstrument
**	3/16/97		Changed KeySplit to support a replacement for sample root key
**				on a per split basis
**	3/27/97		Changed all 4 character constants to use the FOUR_CHAR macro
**	5/13/97		Added ID_VERS & XVersion
**	6/20/97		Added XGetSampleNameFromID
**	7/24/97		Added the structure XExtSndHeader2
**				Added XCreateSoundObjectFromData & XGetSoundEmbeddedStatus & 
**				XSetSoundEmbeddedStatus
**	7/28/97		Changed a pragma for Sparc compilers (jdr)
**	8/7/97		Added XTranslateFromWaveformToSampleData
**	8/25/97		Fixed some Sun compiler warnings
**	9/30/97		Added BankStatus structure, added XCreateBankStatus & XGetBankStatus
**	10/9/97		Added ID_TEXT
**	1/22/98		Modifed XGetMidiData to return type of compression, if any
**				Added XIsSongCompressed
**	3/23/98		MOE: Added definitions of:
**					AIFF_IMA_HEADER_BYTES
**					AIFF_IMA_BLOCK_FRAMES
**					AIFF_IMA_BLOCK_BYTES
**					WAV_IMA_HEADER_BYTES
**				Added XConvertFromIeeeExtended & XConvertToIeeeExtended. Came
**				from GenSoundFiles.c. Added SndCompressionType
**				Added compression parameter to XCreateSoundObjectFromData
**	4/27/98		Changed XCompressAndEncrypt parameters to be UINT32
**	4/30/98		Added I_GENRE & I_SUB_GENRE & R_GENRE & R_SUB_GENRE
**				Modified the structure SongResource_Info to handle the new genre and sub_genre fields
**	5/4/98		Removed ZBF_neverInterpolate to ZBF_reserved_0. Moved XBF_disableClickRemoval to reserved
**	5/11/98		Added XGetAllSoundID
**	5/21/98		Added X_IMA3
**	7/1/98		Changed various API to use the new XResourceType and XLongResourceID and XShortResourceID
**	7/6/98		Fixed type problems with XGetMidiData
**	7/7/98		Modified XGetSampleNameFromID to use new XLongResourceID type
**	7/15/98		Added XGetMusicObjectFromSong
**
**	JAVASOFT
**	??			$$kk: added different pragma packs for intel hardware depending on os (solaris or win32)
**	9/21/98		Added MPEG decoder/encoder API's
**	9/26/98		Added C_MPEG enum type SndCompressionType
**	9/29/98		Renamed firstSoundFormat & secondSoundFormat to XFirstSoundFormat & XSecondSoundFormat
**				Added support for new 'snd ' XThirdSoundFormat. Added structure XSoundHeader3 & XSndHeader3
**				Renamed stdSH, extSH, cmpSH	to XStandardHeader, XExtendedHeader, XCompressedHeader
**				Changed parameter type of XSetSoundSampleRate
**	9/30/98		Added XTranslateFromSampleDataToWaveform
**	10/2/98		Added XCompressLaw
**	10/10/98	Added to XCreateSoundObjectFromData the ability to call a function during compression
**	10/20/98	Removed SongResource_MOD structure and support. Added SongResource_RMF_Linear.
**				Removed SongType SONG_TYPE_MOD, and added SONG_TYPE_RMF_LINEAR
**	12/17/98	Moved some editor related functions into X_EditorTools.h
**	1/27/99		Added PasswordAccess structure & ID_PASSWORD
**	1/29/99		Added XGetVersionNumber & XCreateVersion
**	2/12/99		Renamed USE_HAE_FOR_MPEG to USE_MPEG_DECODER
**	2002-03-14	$$fb added support for IA64, sparcv9 architectures
**	2003-08-21	$$fb support for amd64, cleaner pragma code
*/
/*****************************************************************************/

#ifndef X_FORMATS
#define X_FORMATS

#ifndef __X_API__
#include "X_API.h"
#endif

#ifndef G_SOUND
#include "GenSnd.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if CPU_TYPE == kRISC
#pragma options align=mac68k
#elif defined(X_ALLOW_PRAGMA)
#pragma pack (1)
#endif

    /* Instrument and Song structures
     */
    typedef struct 
    {
	short int		instrumentNumber;
	short int		ResourceINSTID;
    } Remap;
	
    enum
    {
	ID_NULL		=	0L,
	ID_SONG		=	FOUR_CHAR('S','O','N','G'),	//	'SONG'		// song
	ID_INST		=	FOUR_CHAR('I','N','S','T'),	//	'INST'		// instrument format
	ID_MIDI		=	FOUR_CHAR('M','i','d','i'),	//	'Midi'		// standard midi file
	ID_MIDI_OLD	=	FOUR_CHAR('M','I','D','I'),	//	'MIDI'		// standard midi file
	ID_CMID		=	FOUR_CHAR('c','m','i','d'),	//	'cmid'		// compressed midi file
	ID_CMIDI	=	ID_CMID,
	ID_EMID		=	FOUR_CHAR('e','m','i','d'),	//	'emid'		// encrypted midi file
	ID_ECMI		=	FOUR_CHAR('e','c','m','i'),	//	'ecmi'		// encrypted and compressed midi file
	ID_SND		=	FOUR_CHAR('s','n','d',' '),	//	'snd '		// sample
	ID_CSND		=	FOUR_CHAR('c','s','n','d'),	//	'csnd'		// compressed sample
	ID_ESND		=	FOUR_CHAR('e','s','n','d'),	//	'esnd'		// encrypted sample
	ID_RMF		=	FOUR_CHAR('R','M','F','!'),	//	'RMF!'		// rmf object

	ID_BANK		=	FOUR_CHAR('B','A','N','K'),	//	'BANK'		// bank ID
 
 	ID_PASSWORD	=	FOUR_CHAR('E','A','C','S'),	//	'EACS'		// encryption access control string (password)
 
	ID_VERS		=	FOUR_CHAR('V','E','R','S'),	//	'VERS'		// version ID
	ID_TEXT		=	FOUR_CHAR('T','E','X','T'),	//	'TEXT'		// text
 
	ID_MTHD		=	FOUR_CHAR('M','T','h','d'),	//	'MThd'		// midi header ID
	ID_MTRK		=	FOUR_CHAR('M','T','r','k')	//	'MTrk'		// midi track ID
    };

    // Macro's used to set/clear various flags
#define SET_FLAG_VALUE(oldflag, newflag, value)		(value) ? ((oldflag) | (newflag)) : ((oldflag) & ~(newflag))
#define TEST_FLAG_VALUE(flags, flagbit)				((flags) & (flagbit)) ? TRUE : FALSE

    typedef enum
    {
	SONG_TYPE_BAD = -1,
	SONG_TYPE_SMS = 0,
	SONG_TYPE_RMF = 1,			// structured
	SONG_TYPE_RMF_LINEAR = 2	// linear
    } SongType;


    // bits for SongResource_SMS flags1
#define XBF_locked						0x80
#define XBF_terminateDecay				0x40
#define XBF_reserved11					0x20	// not used. use to be XBF_interpolateSong
#define XBF_reserved12					0x10	// not used. use to be XBF_interpolateLead
#define XBF_fileTrackFlag				0x08
#define XBF_enableMIDIProgram			0x04
#define XBF_reserved13					0x02	// not used. use to be XBF_disableClickRemoval
#define XBF_useLeadForAllVoices			0x01

    // bits for SongResource_SMS flags2
#define XBF_ignoreBadPatches			0x80
#define XBF_reserved4					0x40
#define XBF_reserved5					0x20
#define XBF_reserved7					0x10	// not used. use to be XBF_masterEnablePitchRandomness
#define XBF_reserved8					0x08	// not used. use to be XBF_ampScaleLead
#define XBF_reserved9					0x04	// not used. use to be XBF_forceAmpScale
#define XBF_reserved10					0x02	// not used. use to be XBF_masterEnableAmpScale
#define XBF_reserved6					0x01

    // Song resource (SMS type)
    typedef struct
    {
	XShortResourceID	midiResourceID;
	char				reserved_0;
	char				reverbType;
	unsigned short		songTempo;
	char				songType;						//	0 - SMS, 1 - RMF, 2 - MOD

	char				songPitchShift;
	char				maxEffects;
	char				maxNotes;
	short int			mixLevel;
	unsigned char		flags1;							// see XBF for flags1
	char				noteDecay;
	char				defaultPercusionProgram;		// yes, I wanted signed!
	unsigned char		flags2;							// see XBF for flags2
	short int			remapCount;
	char 				remaps;							// Remap variable
	//	unsigned char		copyright;						// variable pascal string
	unsigned char		author;							// variable pascal string
	unsigned char		title;							// variable pascal string
	unsigned char		licensor_contact;
    } SongResource_SMS;


    typedef enum
    {
	R_LAST_RESOURCE		= 0,							// empty. Not used
	R_TITLE				= FOUR_CHAR('T','I','T','L'),	// TITL Byte zero terminated string
	R_PERFORMED_BY		= FOUR_CHAR('P','E','R','F'),	// PERF Byte zero terminated string
	R_COMPOSER			= FOUR_CHAR('C','O','M','P'),	// COMP Byte zero terminated string
	R_COPYRIGHT_DATE	= FOUR_CHAR('C','O','P','D'),	// COPD Byte zero terminated string
	R_COPYRIGHT_LINE	= FOUR_CHAR('C','O','P','L'),	// COPL Byte zero terminated string
	R_PUBLISHER_CONTACT	= FOUR_CHAR('L','I','C','C'),	// LICC Byte zero terminated string
	R_USE_OF_LICENSE	= FOUR_CHAR('L','U','S','E'),	// LUSE Byte zero terminated string
	R_LICENSED_TO_URL	= FOUR_CHAR('L','D','O','M'),	// LDOM Byte zero terminated string
	R_LICENSE_TERM		= FOUR_CHAR('L','T','R','M'),	// LTRM Byte zero terminated string
	R_EXPIRATION_DATE	= FOUR_CHAR('E','X','P','D'),	// EXPD Byte zero terminated string
	R_COMPOSER_NOTES	= FOUR_CHAR('N','O','T','E'),	// NOTE Byte zero terminated string
	R_INDEX_NUMBER		= FOUR_CHAR('I','N','D','X'),	// INDX Byte zero terminated string
	R_GENRE				= FOUR_CHAR('G','E','N','R'),	// GENR Byte zero terminated string
	R_SUB_GENRE			= FOUR_CHAR('S','U','B','G'),	// SUBG Byte zero terminated string

	R_INSTRUMENT_REMAP	= FOUR_CHAR('R','M','A','P'),	// RMAP variable amount
	R_VELOCITY_CURVE	= FOUR_CHAR('V','E','L','C')	// VELC 128 words
    } SongResourceType;


    // Song resource (RMF type - structured)
    typedef struct
    {
	XShortResourceID	rmfResourceID;
	char				reserved_0;
	char				reverbType;
	unsigned short		songTempo;
	char				songType;						//	0 - SMS, 1 - RMF, 2 - MOD

	char				locked;							// are resource encrypted
	short int			songPitchShift;
	short int			maxEffects;
	short int			maxNotes;
	short int			mixLevel;
	short int			songVolume;						// 127 is 100%, 256 is 200% etc.
	char				embeddedSong;					// TRUE if embedded in a bank
	char				reserved_1;
	INT32				unused[7];
	
	short int			resourceCount;
	short int			resourceData;					// subtract this when calculating empty structure
	//
	// from this point on, the data is based upon types and data blocks
	//	char				title[1];						// variable C string
	//	char				composer[1];					// variable C string
	//	char				copyright_date[1];				// variable C string
	//	char				copyright_line[1];				// variable C string
	//	char				contact_info[1];				// variable C string
	//	char				use_license[1];					// variable C string
	//	char				license_term[1];				// variable C string
	//	char				territory[1];					// variable C string
	//	char				expire_date[1];					// variable C string
	//	char				foreign_rights[1];				// variable C string
	//	char				compser_notes[1];				// variable C string
	//	char				index_number[1];				// variable C string
	//	short int			velocityCurve[128];
    } SongResource_RMF;

    // bits for SongResource_RMF_Linear flags
#define XBFL_disableLoops				0x80
#define XBFL_embedded					0x40
#define XBFL_reserved5					0x20
#define XBFL_reserved4					0x10
#define XBFL_reserved3					0x08
#define XBFL_reserved2					0x04
#define XBFL_reserved1					0x02
#define XBFL_reserved0					0x01

    // Song resource (RMF type - linear)
    typedef struct
    {
	XShortResourceID	audioResourceID;
	char				reserved_0;
	char				reverbType;
	unsigned short		songTempo;
	char				songType;						//	0 - SMS, 1 - RMF, 2 - RMF LINEAR

	char				locked;							// are resource encrypted
	short int			maxEffects;
	short int			maxNotes;
	short int			mixLevel;
	short int			songVolume;						// 100 is 100%, 200 is 200% etc.
	XResourceType		audioFormatType;
	XFIXED				sampleRate;
	UINT32		lengthInBytes;					// length in bytes uncompressed
	UINT32		lengthInFrames;					// length in frames uncompressed
	char				channels;
	char				bitSize;
	char				flags;							// see SongResource_RMF_Linear flags
	
	char				unused1;
	INT32				unused2[3];
	
	short int			resourceCount;
	short int			resourceData;					// subtract this when calculating empty structure
	//
	// from this point on, the data is based upon types and data blocks
	//	char				title[1];						// variable C string
	//	char				composer[1];					// variable C string
	//	char				copyright_date[1];				// variable C string
	//	char				copyright_line[1];				// variable C string
	//	char				contact_info[1];				// variable C string
	//	char				use_license[1];					// variable C string
	//	char				license_term[1];				// variable C string
	//	char				territory[1];					// variable C string
	//	char				expire_date[1];					// variable C string
	//	char				foreign_rights[1];				// variable C string
	//	char				compser_notes[1];				// variable C string
	//	char				index_number[1];				// variable C string
    } SongResource_RMF_Linear;

#define DEFAULT_RESOURCE_VERS_ID	0		// ID used inside of RMF file, or a bank
    // version structure
    typedef struct
    {
	short int	versionMajor;
	short int	versionMinor;
	short int	versionSubMinor;
    } XVersion;

    typedef void SongResource;

    // SongResource structure expanded. These values are always in native word order
    // Use XGetSongResourceInfo, and XDisposeSongResourceInfo
    typedef struct
    {
	short int			maxMidiNotes;
	short int			maxEffects;
	short int			mixLevel;
	short int			reverbType;
	XShortResourceID	objectResourceID;
	short int			songVolume;
	SongType			songType;
	INT32				songTempo;
	short int			songPitchShift;
	XBOOL				songLocked;
	XBOOL				songEmbedded;

	char				*title;							// 0
	char				*performed;						// 1
	char				*composer;						// 2
	char				*copyright;						// 3
	char				*publisher_contact_info;		// 4
	char				*use_license;					// 5
	char				*licensed_to_URL;				// 6
	char				*license_term;					// 7
	char				*expire_date;					// 8
	char				*compser_notes;					// 9
	char				*index_number;					// 10
	char				*genre;							// 11
	char				*sub_genre;						// 12

	short int			*remaps;

	short int			*velocityCurve;
	short int			remapCount;
    } SongResource_Info;


    typedef struct
    {
	char				lowMidi;
	char				highMidi;
	XShortResourceID	sndResourceID;
	short int			miscParameter1;		// can be smodParmeter1 if ZBF_enableSoundModifier
	// enabled, otherwise its a replacement
	// rootKey for sample
	short int			miscParameter2;		// if ZBF_enableSoundModifier is enabled then its
	// used as smodParmeter2, otherwise its a volume level
    } KeySplit;

    // bits for Instrument flags1
#define ZBF_enableInterpolate			0x80	// not used
#define ZBF_enableAmpScale				0x40	// not used
#define ZBF_disableSndLooping			0x20
#define ZBF_reserved_1					0x10
#define ZBF_useSampleRate				0x08
#define ZBF_sampleAndHold				0x04
#define ZBF_extendedFormat				0x02
#define ZBF_avoidReverb					0x01	// this is a default enable switch to send to the mix buss. TRUE is off.
    // bits for Instrument flags2
#define ZBF_reserved_0					0x80
#define ZBF_playAtSampledFreq			0x40
#define ZBF_fitKeySplits				0x20	// not used
#define ZBF_enableSoundModifier			0x10
#define ZBF_useSoundModifierAsRootKey	0x08
#define ZBF_notPolyphonic				0x04	// not used
#define ZBF_enablePitchRandomness		0x02	// not used
#define ZBF_playFromSplit				0x01	// not used

    // Special Instrument resource. This can only be used when there is no tremolo data, or key splits
    typedef struct
    {
	XShortResourceID	sndResourceID;
	short int			midiRootKey;
	char				panPlacement;
	unsigned char		flags1;				// see ZBF bits for values
	unsigned char		flags2;				// see ZBF bits for values
	char				smodResourceID;		// Really a smaller version of XShortResourceID
	short int			miscParameter1;		// can be smodParmeter1 if ZBF_enableSoundModifier
	// enabled, otherwise its a replacement
	// rootKey for sample
	short int			miscParameter2;		// if ZBF_enableSoundModifier is enabled then its
	// used as smodParmeter2, otherwise its a volume level
	short int			keySplitCount;		// if this is non-zero, then KeySplit structure is inserted
	// to go beyond this point, if keySplitCount is non-zero, you must use function calls.
	short int			tremoloCount;		// if this is non-zero, then a Word is inserted.
	short int			tremoloEnd;			// Always 0x8000
	short int			reserved_3;
	short int			descriptorName;		// Always 0
	short int			descriptorFlags;	// Always 0
    } InstrumentResource;

#define DEFAULT_RESOURCE_BANK_ID	0		// ID used inside of RMF file
#define BANK_NAME_MAX_SIZE			4096
    // This is a ID_BANK resource
    // FIX THIS: This is a possible problem in LP64, this structure is not
    // aligned properly
    typedef struct
    {
	UINT32	version;
	char			bankURL[BANK_NAME_MAX_SIZE];
	char			bankName[BANK_NAME_MAX_SIZE];
    } BankStatus;

#define DEFAULT_RESOURCE_PASSWORD_ID 0		// ID used inside of RMF file
    // This is a ID_PASSWORD resource
    typedef struct
    {
	UINT32	version;
	//	char			eacs[];		// variable length password accessed with XDecryptAndDuplicateStr
    } PasswordAccess;

    // audioType for the AudioResource structure
    enum
    {
	AUDIO_NAME_TYPE	=	FOUR_CHAR('A','T','X','T'),	//	'ATXT'		// non-terminated string. Length of string is
	//				// length of resource
	AUDIO_SND		=	FOUR_CHAR('s','n','d',' '),	//	'snd '		// MacOS 'snd' format
	AUDIO_CSND		=	FOUR_CHAR('c','s','n','d'),	//	'csnd'		// MacOS 'snd' format compressed
	AUDIO_ESND		=	FOUR_CHAR('e','s','n','d'),	//	'esnd'		// MacOS 'snd' format encrypted
	AUDIO_MPEG		=	FOUR_CHAR('M','P','E','G'),	//	'MPEG'		// MPEG
	AUDIO_IMA2_PCM	=	FOUR_CHAR('I','M','A','2'),	//	'IMA2'		// IMA PCM 2 to 1
	AUDIO_IMA4_PCM	=	FOUR_CHAR('I','M','A','4'),	//	'IMA4'		// IMA PCM 4 to 1
	AUDIO_RAW_PCM	=	FOUR_CHAR('R','P','C','M')	//	'RPCM'		// raw pcm data
    };

#define AUDIO_OBJECT_VERSION			0x0001
    typedef struct
    {
	UINT32	version;			// structure version 1
	UINT32	dataLength;			// length of sample data in bytes
	UINT32	dataOffset;			// offset from begining of structure to data
	UINT32	audioType;			// audio type of audioType
	UINT32	usageType;
	UINT32	sampleRate;			// sample rate in 16.16 fixed point
	UINT32	sampleFrames;		// number of sample frames
	UINT32	loopStart;			// first loop start
	UINT32	loopEnd;			// first loop end
	short int		baseMidiKey;		// base root midi key
	short int		bitSize;			// 8 or 16 bits per sample
        short int               pack1;
	INT32                   pack2;
	short int 		channels;			// 1 or 2 channels
	XResourceType	nameResourceType;	// Resource name type. ie (AUDIO_NAME_TYPE)
	// if ID_NULL, then no name
	XLongResourceID	nameResourceID;		// Resource name id. ie AUDIO_NAME_TYPE ID 2000
	char			usedInBank;			// if true, then sample is embedded in a bank
	char			unusedFlag2;
	char			unusedFlag3;
	char			unusedFlag4;
        UINT32                  pack3;
	UINT32	filler[16];
	UINT32	firstSampleFiller;
	//	data
    } AudioResource;

    // Sun compiler warns about this as an enum
#define rate48khz			0xBB800000L	// 48000.00000 in fixed-point

    // These are included here, because we want to be independent of MacOS, but use this standard format
#ifndef __SOUND__

    // Sun compiler warns about this as an enum
#define rate44khz			0xAC440000L	// 44100.00000 in fixed-point

    enum 
    {
	notCompressed			= 0,			/*compression ID's*/
	fixedCompression		= -1,			/*compression ID for fixed-sized compression*/
	variableCompression		= -2,			/*compression ID for variable-sized compression*/
	twoToOne				= 1,
	eightToThree			= 2,
	threeToOne				= 3,
	sixToOne				= 4,

	rate22050hz				= 0x56220000L,	/*22050.00000 in fixed-point*/
	rate22khz				= 0x56EE8BA3L,	/*22254.54545 in fixed-point*/
	rate11khz				= 0x2B7745D1L,	/*11127.27273 in fixed-point*/
	rate11025hz				= 0x2B110000,	/*11025.00000 in fixed-point*/

	kMiddleC				= 60,			/*MIDI note value for middle C*/

	soundCmd				= 80,
	bufferCmd				= 81
    };
#endif

    enum 
    {
	// Sound format types for 'snd ' resources
	XFirstSoundFormat		= 0x0001,		// general sound format
	XSecondSoundFormat		= 0x0002,		// special sampled sound format (HyperCard)
	XThirdSoundFormat		= 0x0003,		// new Headspace 'snd ' type

	// Sound type 1 & 2 encode sub types, and the last one for type 3
	XStandardHeader			= 0x00,			// Standard sound header encode value
	XExtendedHeader			= 0xFF,			// Extended sound header encode value
	XCompressedHeader		= 0xFE,			// Compressed sound header encode value
	XType3Header			= 0x80			// New standard type 3 snd resource
    };

    // This is the third sample format support by Headspace.
    //
    typedef struct
    {
	XResourceType		subType;			// sub type: C_NONE, C_IMA4, C_ULAW, C_MPEG
	XFIXED				sampleRate;			// sample rate
	XDWORD				lengthInBytesUncompressed;
	XDWORD				lengthInFrames;		// number of audio frames
	XDWORD				lengthInBytes;		// size in bytes
	XDWORD				frameLengthInBytes;	// if a compressed sub type, size of each frame in bytes
	XDWORD				pcmDataOffset;		// if a compressed sub type, this is the number of samples to skip
	// after uncompressing
	XDWORD				loopStart[6];		// loop start frame for each channel. max 6 channels
	XDWORD				loopEnd[6];			// loop end frame
	XResourceType		nameResourceType;	// Resource name type. ie (AUDIO_NAME_TYPE)
	// if ID_NULL, then no name
	XLongResourceID		nameResourceID;		// Resource name id. ie AUDIO_NAME_TYPE ID 2000

	XBYTE				baseKey;			// base sample key
	XBYTE				channels;			// mono or stereo; 1 or 2
	XBYTE				bitSize;			// sample bit size; 8 or 16
	XBYTE				isEmbedded;			// is sample embedded
	XBYTE				isEncrypted;		// is sample encrypted
	XBYTE				isSampleIntelOrder;	// if true, then sampleArea data is intel ordered
	XBYTE				reserved2[2];		// alignment to 8 bytes
	XDWORD				reserved3[8];		// extra

	XBYTE				sampleArea[1];		// space for when samples follow directly
    } XSoundHeader3;


    typedef struct
    {
	UINT32			samplePtr;		/*if NIL then samples are in sampleArea*/
	UINT32			length;			/*length of sound in bytes*/
	UINT32			sampleRate;		/*sample rate for this sound*/
	UINT32			loopStart;		/*start of looping portion*/
	UINT32			loopEnd;		/*end of looping portion*/
	unsigned char			encode;			/*header encoding*/
	unsigned char			baseFrequency;	/*baseFrequency value*/
	unsigned char			sampleArea[1];	/*space for when samples follow directly*/
    } XSoundHeader;
    typedef XSoundHeader *XSoundHeaderPtr;

    typedef struct
    {
	UINT32			samplePtr;			/*if nil then samples are in sample area*/
	UINT32			numChannels;		/*number of channels i.e. mono = 1*/
	UINT32			sampleRate;			/*sample rate in Apples Fixed point representation*/
	UINT32			loopStart;			/*loopStart of sound before compression*/
	UINT32			loopEnd;			/*loopEnd of sound before compression*/
	unsigned char			encode;				/*data structure used , stdSH, extSH, or cmpSH*/
	unsigned char			baseFrequency;		/*same meaning as regular SoundHeader*/
	UINT32			numFrames;			/*length in frames ( packetFrames or sampleFrames )*/
	char					AIFFSampleRate[10];	/*IEEE sample rate*/
	UINT32					markerChunk;		/*sync track*/
	INT32					format;				/*data format type, was futureUse1*/
	char					forceSample8bit;	/*reserved by Apple, Igor will use as IMA encoder to 8 or 16 bit output. Set to 0x80 */
	// to encode as 8 bit output
	char					soundIsEmbedded;	/*reserved by Apple. Igor uses it as a flag */
	char					futureUse2_2;		/*reserved by Apple*/
	char					futureUse2_3;		/*reserved by Apple*/
	UINT32					stateVars;			/*pointer to State Block*/
	UINT32					leftOverSamples;	/*used to save truncated samples between compression calls*/
	short					compressionID;		/*0 means no compression, non zero means compressionID*/
	unsigned short			packetSize;			/*number of bits in compressed sample packet*/
	unsigned short			snthID;				/*resource ID of Sound Manager snth that contains NRT C/E*/
	unsigned short			sampleSize;			/*number of bits in non-compressed sample*/
	unsigned char			sampleArea[1];		/*space for when samples follow directly*/
    } XCmpSoundHeader;
    typedef XCmpSoundHeader * XCmpSoundHeaderPtr;

    typedef struct
    {
	UINT32			samplePtr;			/*if nil then samples are in sample area*/
	UINT32			numChannels;		/*number of channels,  ie mono = 1*/
	UINT32			sampleRate;			/*sample rate in Apples Fixed point representation*/
	UINT32			loopStart;			/*same meaning as regular SoundHeader*/
	UINT32			loopEnd;			/*same meaning as regular SoundHeader*/
	unsigned char			encode;				/*data structure used , stdSH, extSH, or cmpSH*/
	unsigned char			baseFrequency;		/*same meaning as regular SoundHeader*/
	UINT32			numFrames;			/*length in total number of frames*/
	char					AIFFSampleRate[10];	/*IEEE sample rate*/
	UINT32				        markerChunk;		/*sync track*/
	UINT32					instrumentChunks;	/*AIFF instrument chunks*/
	UINT32					AESRecording;
	unsigned short			sampleSize;			/*number of bits in sample*/
	char					soundIsEmbedded;	// reserved by Apple. Igor uses it as a flag
	char					sampleIsIntelOrder;	// reserved by Apple. Igor uses it to determine if samples are Intel ordered
	UINT32			futureUse2;			/*reserved by Apple*/
	UINT32			futureUse3;			/*reserved by Apple*/
	UINT32			futureUse4;			/*reserved by Apple*/
	unsigned char			sampleArea[1];		/*space for when samples follow directly*/
    } XExtSoundHeader;
    typedef XExtSoundHeader *XExtSoundHeaderPtr;

    typedef struct
    {
	short int		type;
	short int		numModifiers;
	unsigned short	modNumber;
	INT32			modInit;
	short int		numCommands;
	// first command
	unsigned short	cmd;
	short int		param1;
	INT32			param2;
    } XSoundFormat1;

    typedef struct
    {
	XSoundFormat1	sndHeader;
	XSoundHeader	sndBuffer;
    } XSndHeader1;

    typedef struct
    {
	XSoundFormat1	sndHeader;
	XExtSoundHeader	sndBuffer;
    } XExtSndHeader1;

    typedef struct
    {
	XSoundFormat1	sndHeader;
	XCmpSoundHeader	sndBuffer;
    } XCmpSndHeader1;

    typedef struct
    {
	short int		type;
	short int		refCount;
	short int		numCmds;
	// first command
	unsigned short	cmd;
	short int		param1;
	INT32			param2;
    } XSoundFormat2;

    typedef struct
    {
	XSoundFormat2	sndHeader;
	XSoundHeader	sndBuffer;
    } XSndHeader2;

    typedef struct
    {
	XSoundFormat2	sndHeader;
	XExtSoundHeader	sndBuffer;
    } XExtSndHeader2;

    typedef struct
    {
	short int		type;
	XSoundHeader3	sndBuffer;
    } XSndHeader3;

#if CPU_TYPE == kRISC
#pragma options align=reset
#elif defined(X_ALLOW_PRAGMA)
#pragma pack ()
#endif


    typedef enum 
    {
	// Compression Types
	// these are used in the snd formatted resource
	C_NONE				= FOUR_CHAR('n','o','n','e'),	// 'none'
	C_IMA4				= FOUR_CHAR('i','m','a','4'),	// 'ima4'	CCITT G.721 ADPCM compression (IMA 4 to 1)
	C_MACE3				= FOUR_CHAR('m','a','c','3'),	// 'mac3'	Apple MACE type 3 to 1
	C_MACE6				= FOUR_CHAR('m','a','c','6'),	// 'mac6'	Apple MACE type 6 to 1
	C_ULAW				= FOUR_CHAR('u','l','a','w'),	// 'ulaw'	µLaw; 2 to 1
	C_ALAW				= FOUR_CHAR('a','l','a','w'),	// 'ulaw'	aLaw; 2 to 1

	C_MPEG_32			= FOUR_CHAR('m','p','g','n'),	// 'mpgn'	Headspace mpeg implementation 32k bits
	C_MPEG_40			= FOUR_CHAR('m','p','g','a'),	// 'mpga'	Headspace mpeg implementation 40k bits
	C_MPEG_48			= FOUR_CHAR('m','p','g','b'),	// 'mpgb'	Headspace mpeg implementation 48k bits
	C_MPEG_56			= FOUR_CHAR('m','p','g','c'),	// 'mpgc'	Headspace mpeg implementation 56k bits
	C_MPEG_64			= FOUR_CHAR('m','p','g','d'),	// 'mpgd'	Headspace mpeg implementation 64k bits
	C_MPEG_80			= FOUR_CHAR('m','p','g','e'),	// 'mpge'	Headspace mpeg implementation 80k bits
	C_MPEG_96			= FOUR_CHAR('m','p','g','f'),	// 'mpgf'	Headspace mpeg implementation 96k bits
	C_MPEG_112			= FOUR_CHAR('m','p','g','g'),	// 'mpgg'	Headspace mpeg implementation 112k bits
	C_MPEG_128			= FOUR_CHAR('m','p','g','h'),	// 'mpgh'	Headspace mpeg implementation 128k bits
	C_MPEG_160			= FOUR_CHAR('m','p','g','i'),	// 'mpgh'	Headspace mpeg implementation 160k bits
	C_MPEG_192			= FOUR_CHAR('m','p','g','j'),	// 'mpgj'	Headspace mpeg implementation 192k bits
	C_MPEG_224			= FOUR_CHAR('m','p','g','k'),	// 'mpgk'	Headspace mpeg implementation 224k bits
	C_MPEG_256			= FOUR_CHAR('m','p','g','l'),	// 'mpgl'	Headspace mpeg implementation 256k bits
	C_MPEG_320			= FOUR_CHAR('m','p','g','m')	// 'mpgm'	Headspace mpeg implementation 320k bits

    } SndCompressionType;

    enum
    {
	// these are used in AIFF/AIFC formatted files
	X_NONE				= FOUR_CHAR('N','O','N','E'),	//	'NONE'
	X_ACE2				= FOUR_CHAR('A','C','E','2'),	//	'ACE2'
	X_ACE8				= FOUR_CHAR('A','C','E','8'),	//	'ACE8'
	X_MACE3				= FOUR_CHAR('M','A','C','3'),	//	'MAC3'
	X_MACE6				= FOUR_CHAR('M','A','C','6'),	//	'MAC6'
	X_IMA3				= FOUR_CHAR('i','m','a','3'),	//	'ima3'
	X_IMA4				= FOUR_CHAR('i','m','a','4')	//	'ima4'
    };

    // Apple AIFF SND-recource header length
#define	AIFF_IMA_HEADER_BYTES	2
    // Apple AIFF SND-recource standard frames-per-block
#define	AIFF_IMA_BLOCK_FRAMES	64
    // Apple AIFF SND-recource block length
#define	AIFF_IMA_BLOCK_BYTES	(AIFF_IMA_BLOCK_FRAMES / 2 + AIFF_IMA_HEADER_BYTES)

    // Microsoft WAV file header length
#define WAV_IMA_HEADER_BYTES	4	// bytes, one for each channel


    typedef struct
    {
	XFIXED				rate;				// sample rate
	UINT32		frames;				// number of audio frames
	UINT32		size;				// size in bytes
	UINT32		loopStart;			// loop start frame
	UINT32		loopEnd;			// loop end frame
	short int			bitSize;			// sample bit size; 8 or 16
	short int			channels;			// mono or stereo; 1 or 2
	short int			baseKey;			// base sample key
	XShortResourceID	theID;				// sample ID if required
	XResourceType		compressionType;	// compression type
	void				*pMasterPtr;		// master pointer if required
    } SampleDataInfo;


    typedef enum
    {
	I_INVALID = 0,				// invalid type
	I_TITLE,					// Title
	I_PERFORMED_BY,				// Performed by
	I_COMPOSER,					// Composer(s)
	I_COPYRIGHT,				// Copyright Date
	I_PUBLISHER_CONTACT,		// Publisher Contact Info
	I_USE_OF_LICENSE,			// Use of License
	I_LICENSED_TO_URL,			// License to URL
	I_LICENSE_TERM,				// License term
	I_EXPIRATION_DATE,			// Expiration Date
	I_COMPOSER_NOTES,			// Composer Notes
	I_INDEX_NUMBER,				// Index Number
	I_GENRE,					// Genre
	I_SUB_GENRE					// Sub-genre
    } SongInfo;

    void XGetSongInformation(SongResource *theSong, INT32 songSize, SongInfo type, char *cName);

    UINT32 XGetSongInformationSize(SongResource *theSong, INT32 songSize, SongInfo type);


    // Create a new song resource.
    SongResource * XNewSongPtr(	SongType songType, 
				XShortResourceID midiID,
				short int maxSongVoices, 
				short int mixLevel, 
				short int maxEffectVoices,
				ReverbMode reverbType);

    void XDisposeSongPtr(SongResource *theSong);

    void XGetKeySplitFromPtr(InstrumentResource *theX, short int entry, KeySplit *keysplit);

    XPTR XGetSoundResourceByID(XLongResourceID theID, INT32 *pReturnedSize);
    XPTR XGetSoundResourceByName(void *cName, INT32 *pReturnedSize);

    XPTR XGetMidiData(XLongResourceID theID, INT32 *pReturnedSize, XResourceType *pType);

    XPTR XGetSamplePtrFromSnd(XPTR pRes, SampleDataInfo *pInfo);

    void XSetSoundLoopPoints(XPTR pRes, INT32 loopStart, INT32 loopEnd);
    void XSetSoundSampleRate(XPTR pRes, XFIXED sampleRate);
    void XSetSoundBaseKey(XPTR pRes, short int baseKey);
    short int XGetSoundBaseKey(XPTR pRes);

    XBOOL XGetSoundEmbeddedStatus(XPTR pRes);
    void XSetSoundEmbeddedStatus(XPTR pRes, XBOOL soundEmbedded);

    // return TRUE to stop
    typedef XBOOL		(*XCompressStatusProc)(UINT32 currentBuffer, UINT32 maxBuffer);

    // Given a structure filled with no compression, create a type 1 sound object.
    // Will return NULL if out of memory, failed to get the right info from the
    // SampleDataInfo structure, and if pSampleInfo->pMasterPtr is NULL.
    // Pass in type a compression type use C_NONE for no compression
    // Pass in a function that will be called pProc during compression only
    XPTR XCreateSoundObjectFromData(XPTR pPCMData, SampleDataInfo *pSampleInfo, SndCompressionType type, XCompressStatusProc pProc);

    XBOOL XGetSampleNameFromID(XLongResourceID sampleSoundID, char *cName);

    SongResource_Info * XGetSongResourceInfo(SongResource *pSong, INT32 songSize);
    void XDisposeSongResourceInfo(SongResource_Info *pSongInfo);

    SongResource * XNewSongFromSongResourceInfo(SongResource_Info *pSongInfo);

    XShortResourceID XGetSongResourceObjectID(SongResource *pSong);
    void XSetSongResourceObjectID(SongResource *pSong, XShortResourceID id);
    XBOOL XIsSongLocked(SongResource *pSong);
    void XSetSongLocked(SongResource *pSong, XBOOL locked);

    // will determine if song is using compression. Requires active resource file
    XBOOL XIsSongCompressed(SongResource *pSong);

    void XGetSongPerformanceSettings(SongResource * theSong, short int *maxMidiVoices, 
				     short int *maxEffectsVoices, short int *mixLevel);
    void XSetSongPerformanceSettings(SongResource *pSong, short int maxMidiVoices, short int maxEffectsVoices,
				     short int mixLevel);

    short int XGetSongReverbType(SongResource *pSong);
    void XSetSongReverbType(SongResource *pSong, short int reverbType);

    short int XGetSongVolume(SongResource *pSong);
    void XSetSongVolume(SongResource *pSong, short int volume);


    SongType XGetSongResourceObjectType(SongResource *pSong);

    SongResource * XChangeSongResource(SongResource *theSong, INT32 songSize, 
				       SongResourceType resourceType, void *pResource, INT32 resourceLength);

    XBOOL XGetSongEmbeddedStatus(SongResource *pSong);
    void XSetSongEmbeddedStatus(SongResource *pSong, XBOOL embedded);

    // Translate a GM_Waveform structure into a SampleDataInfo structure
    void XTranslateFromWaveformToSampleData(GM_Waveform *pSource, SampleDataInfo *pDest);
    // Translate a SampleDataInfo structure into a GM_Waveform structure
    void XTranslateFromSampleDataToWaveform(SampleDataInfo *pSource, GM_Waveform *pDest);

    // Create a bank resource from a BankStatus structure. This data can be written out
    // as a BANK_ID resource
    XPTR XCreateBankStatus(BankStatus *pStatus);
    // Get bank resource from currently open resource file
    void XGetBankStatus(BankStatus *pStatus);
    // Create version resource that is ready to be stored
    XPTR XCreateVersion(short int major, short int minor, short int subMinor);
    // Will return a XVersion number in platform order from the currently open resource file
    void XGetVersionNumber(XVersion *pVersionNumber);

    // convert to and from ieee to XFIXED. Used for AIF files, and SND resources
    XFIXED XConvertFromIeeeExtended(unsigned char *bytes);
    void XConvertToIeeeExtended(XFIXED ieeeFixedRate, unsigned char *bytes);

    // a law / u law compression
    void XCompressLaw(SndCompressionType compressionType, short int *pSource, char *pDest, 
		      UINT32 frames, UINT32 channels);


    // MPEG decoder
#if USE_MPEG_DECODER != 0
    struct XMPEGDecodedData
    {
	void			*stream;
	XFIXED			sampleRate;
	XBYTE			bitSize;
	XBYTE			channels;
	UINT32	lengthInBytes;
	UINT32	lengthInSamples;
	UINT32	frameBufferSize;
	UINT32	maxFrameBuffers;
    };
    typedef struct XMPEGDecodedData XMPEGDecodedData;

    XMPEGDecodedData * XOpenMPEGStreamFromXFILENAME(XFILENAME *file, OPErr *pErr);
    XMPEGDecodedData * XOpenMPEGStreamFromXFILE(XFILE file, OPErr *pErr);
    XMPEGDecodedData * XOpenMPEGStreamFromMemory(XPTR pBlock, UINT32 blockSize, OPErr *pErr);
    OPErr XCloseMPEGStream(XMPEGDecodedData *stream);
    OPErr XFillMPEGStreamBuffer(XMPEGDecodedData *stream, void *pcmAudioBuffer, XBOOL *pDone);

#endif	// USE_MPEG_DECODER

    // MPEG encoder
#if USE_MPEG_DECODER != 0
    // Encoder rates
    typedef enum
    {
	MPG_32 = 32,		// n
	MPG_40 = 40,		// a
	MPG_48 = 48,		// b
	MPG_56 = 56,		// c
	MPG_64 = 64,		// d
	MPG_80 = 80,		// e
	MPG_96 = 96,		// f
	MPG_112 = 112,		// g
	MPG_128 = 128,		// h
	MPG_160 = 160,		// i
	MPG_192 = 192,		// j
	MPG_224 = 224,		// k
	MPG_256 = 256,		// l
	MPG_320 = 320		// m
    } XMPEGEncodeRate;


    struct XMPEGEncodeData
    {
	UINT32		currentFrameBuffer;		// OUT	current frame buffer processing
	UINT32		maxFrameBuffers;		// OUT	max number of MPEG frames
	UINT32		frameBufferSizeInBytes;	// OUT	byte size of each frame buffer
	char				*pFrameBuffer;			// OUT	bytes of completed buffer
	UINT32		frameBufferSize;		// OUT	size in bytes of completed MPEG buffer

	//private:
	void				*pPrivateData;
	XMPEGEncodeRate		encodeRate;
	GM_Waveform			*pAudio;			// NON-ZERO if from memory
	XFILE				file;				// NON-ZERO if from file
    };
    typedef struct XMPEGEncodeData XMPEGEncodeData;

    // This MPEG library only encode MPEG I, layer 3.

    // Open a encode stream; either from a file, or from a memory sample. This function will return an interm structure
    // that contains the total number of MPEG buffers. If pFrameBuffer is non-zero then write out or move from memory
    // the buffer. Then call XProcessMPEGEncoder for XMPEGEncodeData->maxFrameBuffers and write out every pFrameBuffer.
    // When done, then call XCloseMPEGEncodeStream to cleanup and free memory.
    XMPEGEncodeData *XOpenMPEGEncodeStreamFromMemory(GM_Waveform *pAudio, XMPEGEncodeRate encodeRate, OPErr *pErr);
    OPErr	XProcessMPEGEncoder(XMPEGEncodeData *stream);	// call this XMPEGEncodeData->maxFrameBuffers times
    OPErr	XCloseMPEGEncodeStream(XMPEGEncodeData *stream, XPTR *pReturnedBuffer, UINT32 *pReturnedSize);


    // This will encode an MPEG stream from a formatted GM_Waveform
    XPTR XCompressMPEG(GM_Waveform *pWave, XMPEGEncodeRate encodeRate, XCompressStatusProc pProc, 
		       UINT32 *pStreamReturnedSize, UINT32 *pMaxFrames, 
		       UINT32 *pFrameLengthInBytesReturned, UINT32 *pFirstBufferOffsetReturned);

    OPErr XExpandMPEG(XPTR pMPEGStream, UINT32 mpegStreamSize, 
		      XPTR pAudioBuffer, UINT32 pcmAudioBufferLength,
		      UINT32 firstBufferOffset);

#endif	// USE_MPEG_DECODER

#ifdef __cplusplus
}
#endif

#endif	// X_FORMATS


