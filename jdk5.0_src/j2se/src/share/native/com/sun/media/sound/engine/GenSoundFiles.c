/*
 * @(#)GenSoundFiles.c	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	GenSoundFiles.c
**
**	Reads AIFF, WAVE, Sun AU sound files
**
**	Confidential-- Internal use only
**
**	History	-
**	10/1/96		Created
**	10/13/96	Added GM_ReadIntoMemoryWaveFile & GM_ReadIntoMemoryAIFFFile
**	11/5/96		Changed WaveformInfo to GM_Waveform
**				Added more error checking
**				Added GM_FreeWaveform
**	12/30/96	Changed copyright
**	1/20/97		Added support for Sun AU files
**	1/23/97		More memory fail checking code
**	1/29/97		Changed PV_ConvertFromIeeeExtended to return a XFIXED
**				instead of a double
**	2/4/97		Fixed problem with PV_ReadSunAUFile. Didn't define the procptr
**	2/5/97		Added GM_ReadFileIntoMemoryFromMemory with the ability to parse a
**				memory mapped file as a fileType
**	3/19/97		Fixed a positioning and platform word swap bug 
**				with PV_ReadIntoMemorySunAUFile
**	3/29/97		Removed 4 character constants and changed to use macro FOUR_CHAR
**	5/3/97		Fixed a few potential problems that the MOT compiler found
**	5/15/97		Put in code for ADPCM decoding for WAVE files & AIFF files
**	6/2/97		Finished up GM_ReadAndDecodeFileStream to support AU streaming files
**	6/9/97		Added support for X_WAVE_FORMAT_ALAW & X_WAVE_FORMAT_MULAW for wave files
**	7/8/97		Fixed divide by zero when reading WAVE files.
**	7/25/97		Added PV_ConvertToIeeeExtended
**	11/10/97	Changed some preprocessor tests and flags to explicity test for flags rather
**				than assume
**	12/18/97	Cleaned up some warnings
**	3/20/98	MOE: Changed code that deals with XExpandAiffImaStream(),
**					which now accepts a short[2] parameter
**					PV_ReadAIFFAndDecompressIMA() and
**					PV_ReadIntoMemoryAIFFFile() were altered
**	3/20/98	MOE: Change name of call to XExpandWavIma()
**	3/23/98		Fixed warnings in PV_ReadAIFFAndDecompressIMA
**				Renamed PV_ConvertToIeeeExtended to XConvertToIeeeExtended and moved into
**				X_Formats.h. Renamed PV_ConvertFromIeeeExtended to XConvertFromIeeeExtended and
**				moved into X_Formats.h
**	5/5/98		Changed PV_ReadSunAUFile & PV_ReadAIFFAndDecompressIMA to return an OPErr.
**				Added OPErr parameter to PV_ReadIntoMemorySunAUFile & PV_ReadIntoMemoryAIFFFile &
**				PV_ReadAIFFAndDecompressIMA
**				Modified all the IFF_ functions to return OPErr's correctly
**	5/7/98		Changed GM_ReadFileInformation & GM_ReadFileIntoMemory & GM_ReadFileIntoMemoryFromMemory
**				to set an error code if failure
**	5/12/98		MOE: Changed PV_ReadAIFFAndDecompressIMA() to use short[] predictor
**				array rather than int[] index array.
**	5/14/98		Added support for loop points in WAV files. See IFF_GetWAVLoopPoints
**	5/21/98		Changed IFF_GetAIFFSampleSize to use AIFF_IMA_BLOCK_FRAMES constant instead of 64.
**				Changed all structures to be typedef structures
**	7/6/98		Fixed a compiler warning in PV_ReadWAVEAndDecompressIMA about testing for a negitive number
**				with an unsigned value (destLength < 0)
**	9/9/98		Added support for MPEG stream decode
**	9/10/98		Changed various routines that passed the file position in the GM_Waveform structure
**				in the waveformID element to now use the currentFilePosition element
**	9/14/98		Added PV_ReadIntoMemoryMPEGFile
**	2/12/99		Renamed USE_HAE_FOR_MPEG to USE_MPEG_DECODER
**	2002-03-14	$$fb cleaned up pragma code, included Itanium and sparcv9 architecture support
**	2003-08-21	$$fb support for amd64, cleaner pragma code
*/
/*****************************************************************************/

//#define USE_DEBUG	2

#include "X_API.h"
#include "X_Formats.h"
#include "GenSnd.h"
#include <math.h>

#if USE_HIGHLEVEL_FILE_API == TRUE

#define ODD(x)			((INT32)(x) & 1L)

#if CPU_TYPE == kRISC
#pragma options align=mac68k
#elif defined(X_ALLOW_PRAGMA)
#pragma pack (1)
#endif

/**********************- WAVe Defines -**************************/
//
//  extended waveform format structure used for all non-PCM formats. this
//  structure is common to all non-PCM formats.
//
typedef struct
{
    XWORD		wFormatTag;			/* format type */
    XWORD		nChannels;			/* number of channels (i.e. mono, stereo...) */
    XDWORD		nSamplesPerSec;		/* sample rate */
    XDWORD		nAvgBytesPerSec;	/* for buffer estimation */
    XWORD		nBlockAlign;		/* block size of data */
    XWORD		wBitsPerSample;		/* number of bits per sample of mono data */
    XWORD		cbSize;				/* the count in bytes of the size of */
    /* extra information (after cbSize) */
} XWaveHeader;


//
//  IMA endorsed ADPCM structure definitions--note that this is exactly
//  the same format as Intel's DVI ADPCM.
//
//      for WAVE_FORMAT_IMA_ADPCM   (0x0011)
//
//
typedef struct 
{
    XWaveHeader		wfx;
    XWORD			wSamplesPerBlock;
} XWaveHeaderIMA;




typedef struct 
{
    XDWORD			dwIdentifier;		// a unique number (ie, different than the ID number of any other SampleLoop structure). This field may
    // correspond with the dwIdentifier field of some CuePoint stored in the Cue chunk. In other words, the CuePoint structure which has the
    // same ID number would be considered to be describing the same loop as this SampleLoop structure. Furthermore, this field corresponds to
    // the dwIndentifier field of any label stored in the Associated Data List. In other words, the text string (within some chunk in the Associated
    // Data List) which has the same ID number would be considered to be this loop's "name" or "label".

    XDWORD			dwType;				// the loop type (ie, how the loop plays back) as so:
    // 0 - Loop forward (normal)
    // 1 - Alternating loop (forward/backward)
    // 2 - Loop backward
    // 3-31 - reserved for future standard types
    // 32-? - sampler specific types (manufacturer defined)   

    XDWORD			dwStart;			// the startpoint of the loop. In sample frames
    XDWORD 			dwEnd;				// the endpoint of the loop. In sample frames
    XDWORD 			dwFraction;			// allows fine-tuning for loop fractional areas between samples. Values range from 0x00000000 to 0xFFFFFFFF. A
    // value of 0x80000000 represents 1/2 of a sample length.
    XDWORD			dwPlayCount;		// number of times to play the loop. A value of 0 specifies an infinite sustain loop (ie, the wave keeps looping
    // until some external force interrupts playback, such as the musician releasing the key that triggered that wave's playback).
} XSampleLoop;

typedef struct 
{
    XDWORD			dwManufacturer;		// the MMA Manufacturer code for the intended sampler
    XDWORD			dwProduct;			// Product code (ie, model ID) of the intended sampler for the dwManufacturer.

    XDWORD			dwSamplePeriod;		// specifies the period of one sample in nanoseconds (normally 1/nSamplesPerSec from the Format chunk. But
    // note that this field allows finer tuning than nSamplesPerSec). For example, 44.1 KHz would be specified as 22675 (0x00005893).

    XDWORD			dwMIDIUnityNote;	// MIDI note number at which the instrument plays back the waveform data without pitch modification
    // (ie, at the same sample rate that was used when the waveform was created). This value ranges 0 through 127, inclusive. Middle C is 60

    XDWORD			dwMIDIPitchFraction;// specifies the fraction of a semitone up from the specified dwMIDIUnityNote. A value of 0x80000000 is
    // 1/2 semitone (50 cents); a value of 0x00000000 represents no fine tuning between semitones.

    XDWORD			dwSMPTEFormat;		// specifies the SMPTE time format used in the dwSMPTEOffset field. Possible values are:
    //	0  = no SMPTE offset (dwSMPTEOffset should also be 0)
    //	24 = 24 frames per second
    //	25 = 25 frames per second
    //	29 = 30 frames per second with frame dropping ('30 drop')
    //	30 = 30 frames per second       

    XDWORD			dwSMPTEOffset;		// specifies a time offset for the sample if it is to be syncronized or calibrated according to a start time other than
    // 0. The format of this value is 0xhhmmssff. hh is a signed Hours value [-23..23]. mm is an unsigned Minutes value [0..59]. ss is
    // unsigned Seconds value [0..59]. ff is an unsigned value [0..( - 1)]. 

    XDWORD			cSampleLoops;		// number (count) of SampleLoop structures that are appended to this chunk. These structures immediately
    // follow the cbSamplerData field. This field will be 0 if there are no SampleLoop structures.

    XDWORD			cbSamplerData;		// The cbSamplerData field specifies the size (in bytes) of any optional fields that an application wishes to append to this chunk.

    XSampleLoop		loops[];
} XSamplerChunk;

// WAVE form wFormatTag IDs

// supported 
enum 
{
    X_WAVE_FORMAT_PCM					=	0x0001,
    X_WAVE_FORMAT_ALAW					=	0x0006,	/*  Microsoft Corporation  */
    X_WAVE_FORMAT_MULAW					=	0x0007,	/*  Microsoft Corporation  */
    X_WAVE_FORMAT_DVI_ADPCM				=	0x0011,	/*  Intel Corporation  */
    X_WAVE_FORMAT_IMA_ADPCM				=	0x0011	/*  Intel Corporation  */
};

// not supported yet.
enum 
{
    X_WAVE_FORMAT_UNKNOWN				=	0x0000,	/*  Microsoft Corporation  */
    X_WAVE_FORMAT_ADPCM					=	0x0002,	/*  Microsoft Corporation  */
    X_WAVE_FORMAT_IBM_CVSD				=	0x0005,	/*  IBM Corporation  */
    X_WAVE_FORMAT_OKI_ADPCM				=	0x0010,	/*  OKI  */
    X_WAVE_FORMAT_MEDIASPACE_ADPCM		=	0x0012,	/*  Videologic  */
    X_WAVE_FORMAT_SIERRA_ADPCM			=	0x0013,	/*  Sierra Semiconductor Corp  */
    X_WAVE_FORMAT_G723_ADPCM			=	0x0014,	/*  Antex Electronics Corporation  */
    X_WAVE_FORMAT_DIGISTD				=	0x0015,	/*  DSP Solutions, Inc.  */
    X_WAVE_FORMAT_DIGIFIX				=	0x0016,	/*  DSP Solutions, Inc.  */
    X_WAVE_FORMAT_DIALOGIC_OKI_ADPCM	=	0x0017,	/*  Dialogic Corporation  */
    X_WAVE_FORMAT_YAMAHA_ADPCM			=	0x0020,	/*  Yamaha Corporation of America  */
    X_WAVE_FORMAT_SONARC				=	0x0021,	/*  Speech Compression  */
    X_WAVE_FORMAT_DSPGROUP_TRUESPEECH	=	0x0022,	/*  DSP Group, Inc  */
    X_WAVE_FORMAT_ECHOSC1				=	0x0023,	/*  Echo Speech Corporation  */
    X_WAVE_FORMAT_AUDIOFILE_AF36		=	0x0024,	/*    */
    X_WAVE_FORMAT_APTX					=	0x0025,	/*  Audio Processing Technology  */
    X_WAVE_FORMAT_AUDIOFILE_AF10		=	0x0026,	/*    */
    X_WAVE_FORMAT_DOLBY_AC2				=	0x0030,	/*  Dolby Laboratories  */
    X_WAVE_FORMAT_GSM610				=	0x0031,	/*  Microsoft Corporation  */
    X_WAVE_FORMAT_ANTEX_ADPCME			=	0x0033,	/*  Antex Electronics Corporation  */
    X_WAVE_FORMAT_CONTROL_RES_VQLPC		=	0x0034,	/*  Control Resources Limited  */
    X_WAVE_FORMAT_DIGIREAL				=	0x0035,	/*  DSP Solutions, Inc.  */
    X_WAVE_FORMAT_DIGIADPCM				=	0x0036,	/*  DSP Solutions, Inc.  */
    X_WAVE_FORMAT_CONTROL_RES_CR10		=	0x0037,	/*  Control Resources Limited  */
    X_WAVE_FORMAT_NMS_VBXADPCM			=	0x0038,	/*  Natural MicroSystems  */
    X_WAVE_FORMAT_CS_IMAADPCM			=	0x0039,	/* Crystal Semiconductor IMA ADPCM */
    X_WAVE_FORMAT_G721_ADPCM			=	0x0040,	/*  Antex Electronics Corporation  */
    X_WAVE_FORMAT_MPEG					=	0x0050,	/*  Microsoft Corporation  */
    X_WAVE_FORMAT_CREATIVE_ADPCM		=	0x0200,	/*  Creative Labs, Inc  */
    X_WAVE_FORMAT_CREATIVE_FASTSPEECH8	=	0x0202,	/*  Creative Labs, Inc  */
    X_WAVE_FORMAT_CREATIVE_FASTSPEECH10	=	0x0203,	/*  Creative Labs, Inc  */
    X_WAVE_FORMAT_FM_TOWNS_SND			=	0x0300,	/*  Fujitsu Corp.  */
    X_WAVE_FORMAT_OLIGSM				=	0x1000,	/*  Ing C. Olivetti & C., S.p.A.  */
    X_WAVE_FORMAT_OLIADPCM				=	0x1001,	/*  Ing C. Olivetti & C., S.p.A.  */
    X_WAVE_FORMAT_OLICELP				=	0x1002,	/*  Ing C. Olivetti & C., S.p.A.  */
    X_WAVE_FORMAT_OLISBC				=	0x1003,	/*  Ing C. Olivetti & C., S.p.A.  */
    X_WAVE_FORMAT_OLIOPR				=	0x1004	/*  Ing C. Olivetti & C., S.p.A.  */
};

enum 
{
    X_WAVE			= FOUR_CHAR('W','A','V','E'),		//		'WAVE'
    X_RIFF			= FOUR_CHAR('R','I','F','F'),		//		'RIFF'
    X_DATA			= FOUR_CHAR('d','a','t','a'),		//		'data'
    X_FMT			= FOUR_CHAR('f','m','t',' '),		//		'fmt '
    X_SMPL			= FOUR_CHAR('s','m','p','l')		//		'smpl'
};

/**********************- AIFF Defines -**************************/

enum 
{
    X_AIFF			= FOUR_CHAR('A','I','F','F'),		//		'AIFF'
    X_AIFC			= FOUR_CHAR('A','I','F','C'),		//		'AIFC'
    X_Common		= FOUR_CHAR('C','O','M','M'),		//		'COMM'
    X_SoundData		= FOUR_CHAR('S','S','N','D'),		//		'SSND'
    X_Marker		= FOUR_CHAR('M','A','R','K'),		//		'MARK'
    X_Instrument	= FOUR_CHAR('I','N','S','T'),		//		'INST'
    X_FORM			= FOUR_CHAR('F','O','R','M'),		//		'FORM'
    X_CAT			= FOUR_CHAR('C','A','T',' '),		//		'CAT '
    X_LIST			= FOUR_CHAR('L','I','S','T'),		//		'LIST'
    X_BODY			= FOUR_CHAR('B','O','D','Y')		//		'BODY'
};

typedef struct
{
    INT32	ckID;	   /* ID */
    INT32	ckSize;    /* size */
    INT32	ckData;
} XIFFChunk;

typedef struct
{
    short int		numChannels;
    UINT32	numSampleFrames;
    short int		sampleSize;
    unsigned char	sampleRate[10];
} XAIFFHeader;

typedef struct
{
    short int		numChannels;
    UINT32	numSampleFrames;
    short int		sampleSize;
    unsigned char	sampleRate[10];
    UINT32	compressionType;
    char			compressionName[256];			/* variable length array, Pascal string */
} XAIFFExtenedHeader;

typedef struct
{
    unsigned char	baseFrequency;
    unsigned char	detune;
    unsigned char	lowFrequency;
    unsigned char	highFrequency;
    unsigned char	lowVelocity;
    unsigned char	highVelocity;
    short int		gain;

    short int		sustainLoop_playMode;
    short int		sustainLoop_beginLoop;
    short int		sustainLoop_endLoop;
    short int		releaseLoop_beginLoop;
    short int		releaseLoop_endLoop;
    short int		extra;
} XInstrumentHeader;

typedef struct
{
    UINT32	offset;
    UINT32	blockSize;
} XSoundData;

#if CPU_TYPE == kRISC
#pragma options align=reset
#elif defined(X_ALLOW_PRAGMA)
#pragma pack ()
#endif

/**********************- AU Defines -**************************/

#if CPU_TYPE == kRISC
#pragma options align=mac68k
#elif defined(X_ALLOW_PRAGMA)
#pragma pack (1)
#endif
	
#include "g72x.h"

typedef struct 
{
    UINT32 magic;          // magic number 
    UINT32 hdr_size;       // size of the whole header, including optional comment.
    UINT32 data_size;      // optional data size - usually unusalble. 
    UINT32 encoding;       // format of data contained in this file 
    UINT32 sample_rate;    // sample rate of data in this file 
    UINT32 channels;       // numbder of interleaved channels (usually 1 or 2) 
} SunAudioFileHeader;


// Note these are defined for big-endian architectures 
#define SUN_AUDIO_FILE_MAGIC_NUMBER (0x2E736E64L)

// Define the encoding fields 
#define	SUN_AUDIO_FILE_ENCODING_MULAW_8			(1)		// 8-bit ISDN u-law 
#define	SUN_AUDIO_FILE_ENCODING_LINEAR_8		(2)		// 8-bit linear PCM 
#define	SUN_AUDIO_FILE_ENCODING_LINEAR_16		(3)		// 16-bit linear PCM 
#define	SUN_AUDIO_FILE_ENCODING_LINEAR_24		(4)		// 24-bit linear PCM 
#define	SUN_AUDIO_FILE_ENCODING_LINEAR_32		(5)		// 32-bit linear PCM 
#define	SUN_AUDIO_FILE_ENCODING_FLOAT			(6)		// 32-bit IEEE floating point 
#define	SUN_AUDIO_FILE_ENCODING_DOUBLE			(7)		// 64-bit IEEE floating point 
#define	SUN_AUDIO_FILE_ENCODING_ADPCM_G721		(23)	// 4-bit CCITT g.721 ADPCM 
#define	SUN_AUDIO_FILE_ENCODING_ADPCM_G722		(24)	// CCITT g.722 ADPCM 
#define	SUN_AUDIO_FILE_ENCODING_ADPCM_G723_3	(25)	// CCITT g.723 3-bit ADPCM 
#define	SUN_AUDIO_FILE_ENCODING_ADPCM_G723_5	(26)	// CCITT g.723 5-bit ADPCM 
#define SUN_AUDIO_FILE_ENCODING_ALAW_8			(27) 	// 8-bit ISDN A-law 


#if CPU_TYPE == kRISC
#pragma options align=reset
#elif defined(X_ALLOW_PRAGMA)
#pragma pack ()
#endif

// internal structures
typedef struct
{
    INT32			formType;				//	either X_RIFF, or X_FORM;
    INT32			headerType;
    INT32			formPosition;			//	position in file of current FORM
    INT32			formLength;				//	length of FORM
    OPErr			lastError;
    XFILE			fileReference;
} X_IFF;

// IFF functions

static OPErr IFF_Error(X_IFF *pIFF)
{
    if (pIFF)
	{
	    return pIFF->lastError;
	}
    return NO_ERR;
}

static void IFF_SetFormType(X_IFF *pIFF, INT32 formType)
{
    if (pIFF)
	{
	    pIFF->formType = formType;
	}
}


/* -1 is error */
static INT32 IFF_GetNextGroup(X_IFF *pIFF, XIFFChunk *pChunk)
{
    INT32	err = 0;

    if (XFileRead(pIFF->fileReference, pChunk, (INT32)sizeof(XIFFChunk) - (INT32)sizeof(INT32)) != -1)	// get chunk ID
	{
	    if (pIFF->formType == X_RIFF)
		{
#if X_WORD_ORDER == FALSE	// motorola?
		    pChunk->ckSize = XSwapLong(pChunk->ckSize);
#endif
		}
	    else
		{
		    pChunk->ckSize = XGetLong(&pChunk->ckSize);
		}
	    pChunk->ckData = 0L;
	    pChunk->ckID = XGetLong(&pChunk->ckID);		// swap if not motorola
	    switch(pChunk->ckID)
		{
		case X_FORM:
		case X_RIFF:
		case X_LIST:
		case X_CAT:
		    pIFF->formPosition = XFileGetPosition(pIFF->fileReference);	 /* get current pos */
		    pIFF->formLength = pChunk->ckSize;

		    if (XFileRead(pIFF->fileReference, &pChunk->ckData, (INT32)sizeof(INT32)) == -1)
			{
			    pIFF->lastError = BAD_FILE;
			}
		    pChunk->ckData = XGetLong(&pChunk->ckData);
		    break;
		}
	}
    else
	{
	    pIFF->lastError = BAD_FILE;
	    err = -1;
	}
    return err;
}


/******- Determine if a file is a IFF type -******************************/

static INT32 IFF_FileType(X_IFF *pIFF)
{
    XIFFChunk type;

    if (pIFF)
	{
	    XFileSetPosition(pIFF->fileReference, 0L);	/* set to begining of file */

	    pIFF->formPosition = 0;
	    pIFF->formLength = 0;
	    XSetMemory(&type, (INT32)sizeof(XIFFChunk), 0);
	    IFF_GetNextGroup(pIFF, &type);
	    return( (type.ckID == pIFF->formType) ? type.ckData : -1L);
	}
    return -1L;
}

/*- scan past nested FORM's -*/
static INT32 IFF_NextBlock(X_IFF *pIFF, INT32 blockID)
{
    INT32 saveFORM, saveFLEN;
    XIFFChunk type;
    INT32 flag;

    flag = -1;
    while (XFileGetPosition(pIFF->fileReference) < (pIFF->formPosition + pIFF->formLength))
	{
	    saveFORM = pIFF->formPosition;
	    saveFLEN = pIFF->formLength;
	    if (IFF_GetNextGroup(pIFF, &type) == -1)	/* error? */
		{
		    break;
		}
	    if (type.ckID == pIFF->formType)
		{
		    type.ckSize -= 4L;
		}
	    pIFF->formPosition = saveFORM;
	    pIFF->formLength = saveFLEN;
	    if (type.ckID != blockID)
		{
		    if (XFileSetPositionRelative(pIFF->fileReference, type.ckSize + (type.ckSize&1)))
	 		{
			    flag = -1;  /* error */
			    break;
			}
		}
	    else
		{
		    flag = 0;	/* ok, found block ID */
		    break;
		}
	}
    if (flag == -1)
	{
	    pIFF->lastError = BAD_FILE;
	}
    return flag;
}


/******- Scan a FORM for a 4 letter block -*******************************/
static INT32 IFF_ScanToBlock(X_IFF *pIFF, INT32 block)
{
    if (XFileSetPosition(pIFF->fileReference, pIFF->formPosition + 4) == 0)		// set to inside of FORM
 	{
	    return IFF_NextBlock(pIFF, block);
	}
    else
	{
	    pIFF->lastError = BAD_FILE;
	    return -1;
	}
}

/******************- Return Chunk size -**********************************/
static INT32 IFF_ChunkSize(X_IFF *pIFF, INT32 block)
{
    INT32 size;

    if (IFF_ScanToBlock(pIFF, block) == -1L)
	{
	    return -1;	// bad
	}
    XFileSetPositionRelative(pIFF->fileReference, -4L);		// back-up and get size
    if (XFileRead(pIFF->fileReference, &size, 4L) == -1)
	{
	    pIFF->lastError = BAD_FILE;
	    return -1;
	}
    if (pIFF->formType == X_RIFF)
	{
#if X_WORD_ORDER == FALSE	// motorola?
	    size = XSwapLong(size);
#endif
	}
    return size;
}


/*- go inside a FORM that has been found -*/
static INT32 IFF_NextForm(X_IFF *pIFF)
{
    XIFFChunk type;

    XFileSetPosition(pIFF->fileReference, pIFF->formPosition);	// set to inside of FORM
	
    if (IFF_GetNextGroup(pIFF, &type) != -1)
	{
	    if (type.ckID == pIFF->formType)
		{
		    return type.ckData;
		}
	}
    pIFF->lastError =  BAD_FILE;
    return -1;
}

static INT32 IFF_CurrentForm(X_IFF *pIFF)
{
    pIFF->formPosition = XFileGetPosition(pIFF->fileReference);
    return IFF_NextForm(pIFF);
}

static INT32 IFF_ReadBlock(X_IFF *pIFF, XPTR pData, INT32 Length)
{
    if (XFileRead(pIFF->fileReference, pData, Length) == -1)
	{
	    pIFF->lastError = BAD_FILE;
	}
    return Length;
}


static INT32 IFF_GetChunk(X_IFF *pIFF, INT32 block, INT32 size, XPTR p)
{
    if (IFF_ScanToBlock(pIFF, block) == -1L)
	{
	    return(-1); /* bad */
	}
    if (size == -1L)	/* size not known? */
	{
	    XFileSetPositionRelative(pIFF->fileReference, -4L);		// back-up and get size
	    if (XFileRead(pIFF->fileReference, &size, (INT32)sizeof(INT32)) == -1)
		{
		    pIFF->lastError = BAD_FILE;
		}
	    if (pIFF->formType == X_RIFF)
		{
#if X_WORD_ORDER == FALSE	// motorola?
		    size = XSwapLong(size);
#endif
		}
	    else
		{
		    size = XGetLong(&size);
		}
	}

    IFF_ReadBlock(pIFF, p, size);	/* read block */
    pIFF->lastError = NO_ERR;
    if (size&1) /* odd? */
	{
	    XFileSetPositionRelative(pIFF->fileReference, 1L);		// skip one byte
	}
    return pIFF->lastError;
}


static INT32 IFF_NextChunk(X_IFF *pIFF, INT32 block, INT32 size, XPTR p)
{
    if (IFF_NextBlock(pIFF, block) == -1)
	{
	    return(-1); /* bad */
	}
    if (size == -1L)	/* size not known? */
	{
	    XFileSetPositionRelative(pIFF->fileReference, -4L);		// back-up and get size
	    if (XFileRead(pIFF->fileReference, &size, (INT32)sizeof(INT32)) == -1)
		{
		    pIFF->lastError = BAD_FILE;
		}
	    if (pIFF->formType == X_RIFF)
		{
#if X_WORD_ORDER == FALSE	// motorola?
		    size = XSwapLong(size);
#endif
		}
	    else
		{
		    size = XGetLong(&size);
		}
	}
    IFF_ReadBlock(pIFF, p, size);	/* read block */
    if (size&1) /* odd? */
	{
	    XFileSetPositionRelative(pIFF->fileReference, 1L);		// skip one byte
	}
    return pIFF->lastError;
}

/*
 * Unpack input codes and pass them back as bytes.
 * Returns 1 if there is residual input, returns -1 if eof, else returns 0.
 */
static XFILE		sunFileReference;
static unsigned int	in_buffer = 0;
static int			in_bits = 0;
static INLINE int PV_UnpackInput( unsigned char	*code, int bits)
{
    unsigned char		in_byte;

    if (in_bits < bits) 
	{
	    if (XFileRead(sunFileReference, &in_byte, 1L) == -1)
		{
		    *code = 0;
		    return -1;
		}
	    in_buffer |= (in_byte << in_bits);
	    in_bits += 8;
	}
    *code = in_buffer & ((1 << bits) - 1);
    in_buffer >>= bits;
    in_bits -= bits;
    return (in_bits > 0);
}

// Sun AU file code

static OPErr PV_ReadSunAUFile(	INT32 encoding,
				XFILE fileReference, 
				void *pSample,
				INT32 sampleByteLength,
				UINT32 *pBufferLength
				)
{
    OPErr				err;
    short				sample;
    unsigned char		code;
    struct g72x_state	state;
    int					out_size;
    int					(*dec_routine)(int i, int out_coding, struct g72x_state *state_ptr);
    int					dec_bits;
    short int			*pSample16;
    unsigned char		*pSample8;
    unsigned char		codeBlock[1026];
    UINT32		writeLength;

    writeLength = 0;
    err = NO_ERR;
    dec_bits = 0;
    pSample8 = (unsigned char *)pSample;
    pSample16 = (short int *)pSample;
    switch (encoding)
	{
	case SUN_AUDIO_FILE_ENCODING_LINEAR_16:
	    writeLength = sampleByteLength;
	    XFileRead(fileReference, pSample, writeLength);
	    break;
	case SUN_AUDIO_FILE_ENCODING_LINEAR_8:
	    writeLength = sampleByteLength;
	    XFileRead(fileReference, pSample, writeLength);
	    break;
	case SUN_AUDIO_FILE_ENCODING_MULAW_8:
	    while (sampleByteLength > 0)
		{
		    XFileRead(fileReference, codeBlock, 1024L);
		    for (out_size = 0; (out_size < 1024) && sampleByteLength; out_size++)
			{
			    *pSample16++ = ulaw2linear(codeBlock[out_size]);
			    sampleByteLength -= 2;
			    writeLength += 2;
			}
		}
	    break;
	case SUN_AUDIO_FILE_ENCODING_ALAW_8:
	    while (sampleByteLength > 0)
		{
		    XFileRead(fileReference, codeBlock, 1024L);
		    for (out_size = 0; (out_size < 1024) && sampleByteLength; out_size++)
			{
			    *pSample16++ = alaw2linear(codeBlock[out_size]);
			    sampleByteLength -= 2;
			    writeLength += 2;
			}
		}
	    break;

	case SUN_AUDIO_FILE_ENCODING_ADPCM_G721:
	    out_size = (int)sizeof (short);
	    dec_routine = g721_decoder;
	    dec_bits = 4;
	    goto decode_adpcm;
	case SUN_AUDIO_FILE_ENCODING_ADPCM_G723_3:
	    out_size = (int)sizeof (short);
	    dec_routine = g723_24_decoder;
	    dec_bits = 3;
	    goto decode_adpcm;
	case SUN_AUDIO_FILE_ENCODING_ADPCM_G723_5:
	    out_size = (int)sizeof (short);
	    dec_routine = g723_40_decoder;
	    dec_bits = 5;

	decode_adpcm:
	    g72x_init_state(&state);
	    sunFileReference = fileReference;
	    in_bits = 0;
	    in_buffer = 0;
	    /* Read and unpack input codes and process them */
	    while (PV_UnpackInput(&code, dec_bits) >= 0) 
		{
		    sample = (*dec_routine)(code, AUDIO_ENCODING_LINEAR, &state);
		    if (out_size == 2) 
			{
			    *pSample16++ = sample;
			    writeLength += 2;
			}
		    else 
			{
			    code = (unsigned char)sample;
			    *pSample8++ = code;
			    writeLength++;
			}
		    sampleByteLength--;
		    if (sampleByteLength <= 0)
			{
			    break;
			}
		}
	    break;
	}
    if (pBufferLength)
	{
	    *pBufferLength = writeLength;
	}
    return err;
}






// WAVE functions

static INT32 IFF_GetWAVFormatTag(X_IFF *pIFF)
{
    INT32		theErr;
    XWaveHeader	header;

    theErr = IFF_GetChunk(pIFF, X_FMT, (INT32)sizeof(XWaveHeader), (void *)&header);

#if X_WORD_ORDER == FALSE	// motorola?
    header.wFormatTag = XSwapShort(header.wFormatTag);
#endif
    return (INT32)header.wFormatTag;
}


static INT32 IFF_GetWAVHeader(X_IFF *pIFF, XWaveHeader * pHeaderInfo)
{
    INT32	theErr;

    theErr = IFF_GetChunk(pIFF, X_FMT, (INT32)sizeof(XWaveHeader), (void *)pHeaderInfo);

#if X_WORD_ORDER == FALSE	// motorola?
    pHeaderInfo->nSamplesPerSec = XSwapLong(pHeaderInfo->nSamplesPerSec);
    pHeaderInfo->nAvgBytesPerSec = XSwapLong(pHeaderInfo->nAvgBytesPerSec);
    pHeaderInfo->wFormatTag = XSwapShort(pHeaderInfo->wFormatTag);
    pHeaderInfo->nChannels = XSwapShort(pHeaderInfo->nChannels);
    pHeaderInfo->nBlockAlign = XSwapShort(pHeaderInfo->nBlockAlign);
    pHeaderInfo->wBitsPerSample = XSwapShort(pHeaderInfo->wBitsPerSample);
    pHeaderInfo->cbSize = XSwapShort(pHeaderInfo->cbSize);
#endif
    return theErr;
}

static INT32 IFF_GetWAVIMAHeader(X_IFF *pIFF, XWaveHeaderIMA * pHeaderInfo)
{
    INT32	theErr;

    theErr = IFF_GetChunk(pIFF, X_FMT, (INT32)sizeof(XWaveHeaderIMA), (void *)pHeaderInfo);

#if X_WORD_ORDER == FALSE	// motorola?
    pHeaderInfo->wfx.nSamplesPerSec = XSwapLong(pHeaderInfo->wfx.nSamplesPerSec);
    pHeaderInfo->wfx.nAvgBytesPerSec = XSwapLong(pHeaderInfo->wfx.nAvgBytesPerSec);
    pHeaderInfo->wfx.wFormatTag = XSwapShort(pHeaderInfo->wfx.wFormatTag);
    pHeaderInfo->wfx.nChannels = XSwapShort(pHeaderInfo->wfx.nChannels);
    pHeaderInfo->wfx.nBlockAlign = XSwapShort(pHeaderInfo->wfx.nBlockAlign);
    pHeaderInfo->wfx.wBitsPerSample = XSwapShort(pHeaderInfo->wfx.wBitsPerSample);
    pHeaderInfo->wfx.cbSize = XSwapShort(pHeaderInfo->wfx.cbSize);
    pHeaderInfo->wSamplesPerBlock = XSwapShort(pHeaderInfo->wSamplesPerBlock);
#endif
    return theErr;
}

// Get compressed and uncompressed size. Return 0 if successful, 1 if failure
static INT32 IFF_GetWAVSampleSize(X_IFF *pIFF, UINT32 *pUncompressedSize, UINT32 *pCompressedSize)
{
    INT32			size, error;
    XWaveHeaderIMA	header;

    error = 0;
    size = 0;
    if ( (IFF_GetWAVIMAHeader(pIFF, &header) == 0) && pUncompressedSize && pCompressedSize)
	{
	    if (IFF_ScanToBlock(pIFF, X_DATA) == 0)	/* skip to body */
		{
		    XFileSetPositionRelative(pIFF->fileReference, -4L);		// back-up and get size
		    if (XFileRead(pIFF->fileReference, &size, (INT32)sizeof(INT32)) == -1)
			{
			    pIFF->lastError = BAD_FILE;
			}
		    if (pIFF->formType == X_RIFF)
			{
#if X_WORD_ORDER == FALSE	// motorola?
			    size = XSwapLong(size);
#endif
			}
		    *pCompressedSize = size;
		    switch(header.wfx.wFormatTag)
			{
			default:
			    size = 0;
			    error = 1;
			    break;
			case X_WAVE_FORMAT_ALAW:
			case X_WAVE_FORMAT_MULAW:
			    size *= 2;	// double size
			    break;
			case X_WAVE_FORMAT_PCM:
			    // size will be valid
			    break;
				// calculate the uncompressed byte size
			case X_WAVE_FORMAT_IMA_ADPCM:
			    size *= 4;
			    /*
			      {
			      unsigned long	cBlocks;
			      unsigned long	cbBytesPerBlock;

		                //
		                //  how many destination PCM bytes are needed to hold
		                //  the decoded ADPCM data
		                //
		                //  always round UP
		                //
		                cBlocks = size / header.wfx.nBlockAlign;
		                if (cBlocks)
		                {
				cbBytesPerBlock = header.wSamplesPerBlock * 2; // dest block aline

				if ( ! ((0xFFFFFFFFL / cbBytesPerBlock) < cBlocks) )
				{
				if ((size % header.wfx.nBlockAlign) == 0)
				{
				size = cBlocks * cbBytesPerBlock;
				}
				else
				{
				size = (cBlocks + 1) * cbBytesPerBlock;
				}
				}
				else
				{
				size = 0;	// out of range
				error = 1;
				}
				}
				else
				{
				size = 0;	// out of range
				error = 1;
				}
				}
			    */
			    break;
			}
		    *pUncompressedSize = size;
		}
	}
    else
	{
	    error = 1;
	}
    return error;
}

// Returns WAV loop points, if there. Return 0 if successful, -1 if failure
static INT32 IFF_GetWAVLoopPoints(X_IFF *pIFF, UINT32 *pLoopStart, UINT32 *pLoopEnd, UINT32 *pLoopCount)
{
    XSamplerChunk	*pSampler;
    INT32			theErr;
    UINT32	size;

    *pLoopStart = 0;
    *pLoopEnd = 0;
    theErr = 0;
    if (IFF_ScanToBlock(pIFF, X_SMPL) == 0)	/* skip to body */
	{
	    XFileSetPositionRelative(pIFF->fileReference, -4L);		// back-up and get size
	    if (XFileRead(pIFF->fileReference, &size, (INT32)sizeof(INT32)) == -1)
		{
		    pIFF->lastError = BAD_FILE;
		    theErr = -1;
		}
	    if (pIFF->formType == X_RIFF)
		{
#if X_WORD_ORDER == FALSE	// motorola?
		    size = XSwapLong(size);
#endif
		}
	    if (theErr == 0)
		{
		    pSampler = (XSamplerChunk *)XNewPtr(size);
		    if (pSampler)
			{
			    if (XFileRead(pIFF->fileReference, pSampler, size) == -1)
				{
				    pIFF->lastError = BAD_FILE;
				    theErr = -1;
				}
			    else
				{	// ok
#if X_WORD_ORDER == FALSE	// motorola?
				    if (XSwapLong(pSampler->cSampleLoops))
#else
					if (pSampler->cSampleLoops)
#endif
					    {
#if X_WORD_ORDER == FALSE	// motorola?
						if (XSwapLong(pSampler->loops[0].dwType) == 0)
#else
						    if (pSampler->loops[0].dwType == 0)
#endif
							{
							    // WAV loop points are in bytes not in frames
#if X_WORD_ORDER == FALSE	// motorola?
							    *pLoopStart = XSwapLong(pSampler->loops[0].dwStart);
							    *pLoopEnd = XSwapLong(pSampler->loops[0].dwEnd);
#else
							    *pLoopStart = pSampler->loops[0].dwStart;
							    *pLoopEnd = pSampler->loops[0].dwEnd;
#endif
#if X_WORD_ORDER == FALSE	// motorola?
							    *pLoopCount = XSwapLong(pSampler->loops[0].dwPlayCount);
#else
							    *pLoopCount = pSampler->loops[0].dwPlayCount;
#endif
							}
					    }
				}
			}
		    XDisposePtr(pSampler);
		}
		
	}
    return theErr;
}


// return NO_ERR if successfull
static OPErr PV_ReadWAVEAndDecompressIMA(XFILE fileReference, UINT32 sourceLength,
					 char *pDestSample, UINT32 destLength,
					 char outputBitSize, char channels,
					 XPTR pBlockBuffer, UINT32 blockSize,
					 UINT32 *pBufferLength)
{
    UINT32			writeBufferLength, size, offset;
    OPErr					error;
    XBOOL					customBlockBuffer;
	
    error = MEMORY_ERR;
    writeBufferLength = 0;
    customBlockBuffer = FALSE;
    if (pBlockBuffer == NULL)
	{
	    pBlockBuffer = XNewPtr(blockSize);
	    customBlockBuffer = TRUE;
	}
    if (pBlockBuffer)
	{
	    error = NO_ERR;
	    while (sourceLength > 0)
		{
		    sourceLength -= blockSize;
		    if (sourceLength > blockSize)
			{
			    size = XFileGetPosition(fileReference);	 /* get current pos */
			    if (XFileRead(fileReference, pBlockBuffer, blockSize) == -1)
				{
				    error = BAD_FILE;
				}
			    size = XFileGetPosition(fileReference) - size;
			}
		    else
			{
				// last block, just stop
			    size = 0;
			}
		    offset = 0;
		    if (size)
			{
			    offset = XExpandWavIma((XBYTE const*)pBlockBuffer, blockSize,
						   pDestSample, outputBitSize,
						   size, channels);
			}
		    if (offset == 0)
			{
				// we're done
			    break;
			}
		    if (destLength < offset)	// done filling?
			{
			    break;
			}
		    destLength -= offset;
		    pDestSample += offset;
		    writeBufferLength += offset;
		}
	}
    if (customBlockBuffer)
	{
	    XDisposePtr(pBlockBuffer);
	}
    if (pBufferLength)
	{
	    *pBufferLength = writeBufferLength;
	}
    return error;
}


// This will read into memory the entire wave file and return a GM_Waveform structure.
// When disposing make sure and dispose of both the GM_Waveform structure and the
// theWaveform inside of that structure with XDisposePtr
static GM_Waveform * PV_ReadIntoMemoryWaveFile(XFILENAME *file, XBOOL loadData, 
					       XPTR pMemoryFile, UINT32 memoryFileSize,
					       INT32 *pFormat, UINT32 *pBlockSize, OPErr *pError)
{
    GM_Waveform		*pSample;
    X_IFF			*pIFF;
    XWaveHeaderIMA	waveHeader;
    UINT32	size, sourceLength;

    if (pError)
	{
	    *pError = NO_ERR;
	}
    pSample = NULL;
    if (pFormat)
	{
	    *pFormat = 0;
	}
    pIFF = (X_IFF *)XNewPtr((INT32)sizeof(X_IFF));
    if (pIFF)
	{
	    IFF_SetFormType(pIFF, X_RIFF);
	    if (file)
		{
		    pIFF->fileReference = XFileOpenForRead(file);
		}
	    else
		{
		    if (pMemoryFile)
			{
			    pIFF->fileReference = XFileOpenForReadFromMemory(pMemoryFile, memoryFileSize);
			}
		}
	    if (pIFF->fileReference)
		{
		    pSample = (GM_Waveform *)XNewPtr((INT32)sizeof(GM_Waveform));
		    if (pSample)
			{
			    if (IFF_FileType(pIFF) == X_WAVE)
				{
				    if (IFF_GetWAVIMAHeader(pIFF, &waveHeader) == 0)
					{
					    pSample->channels = (UBYTE)waveHeader.wfx.nChannels;
					    pSample->bitSize = (UBYTE)waveHeader.wfx.wBitsPerSample;
					    pSample->sampledRate = waveHeader.wfx.nSamplesPerSec << 16L;
					    pSample->baseMidiPitch = 60;

					    IFF_GetWAVLoopPoints(pIFF, &pSample->startLoop, &pSample->endLoop, &pSample->numLoops);

					    // we want the byte size
					    size = 0;
					    if (IFF_GetWAVSampleSize(pIFF, &size, &sourceLength) == 0)
						{
						    switch (waveHeader.wfx.wFormatTag)
							{
							case X_WAVE_FORMAT_IMA_ADPCM:
							    pSample->bitSize = 16;
							    break;
							case X_WAVE_FORMAT_ALAW:
							case X_WAVE_FORMAT_MULAW:
							    pSample->bitSize = 16;
							    break;
							}
						}
					    else
						{
						    pIFF->lastError = BAD_FILE_TYPE;
						}
					    if (pFormat)
						{
						    *pFormat = waveHeader.wfx.wFormatTag;
						}
					    if (pBlockSize)
						{
						    *pBlockSize = waveHeader.wfx.nBlockAlign;
						}
					    // if no error, file will be positioned at the sample data
					    pSample->waveSize = size;
					    if (size)
						{
						    pSample->waveFrames = (size / pSample->channels) / (pSample->bitSize / 8);
						}
					    // now the file is positioned right at the data block. So let's allocate it and read
					    // it into memory
					    if (loadData && pSample->waveSize)
						{
						    pSample->theWaveform = (SBYTE *)XNewPtr(pSample->waveSize);
						    if (pSample->theWaveform)
							{
							    switch(waveHeader.wfx.wFormatTag)
								{
								default:
								    pIFF->lastError = BAD_FILE_TYPE;
								    break;
								case X_WAVE_FORMAT_ALAW:
								    pIFF->lastError = PV_ReadSunAUFile(SUN_AUDIO_FILE_ENCODING_ALAW_8,
												       pIFF->fileReference, 
												       pSample->theWaveform,
												       pSample->waveSize,
												       &size);
								    break;
								case X_WAVE_FORMAT_MULAW:
								    pIFF->lastError = PV_ReadSunAUFile(SUN_AUDIO_FILE_ENCODING_MULAW_8,
												       pIFF->fileReference, 
												       pSample->theWaveform,
												       pSample->waveSize,
												       &size);
								    break;
								case X_WAVE_FORMAT_PCM:
								    pIFF->lastError = NO_ERR;
								    if (XFileRead(pIFF->fileReference, 
										  pSample->theWaveform,
										  pSample->waveSize) != -1)
									{
									    // now, if the file is 16 bit sample on a motorola ordered system, swap the bytes
#if X_WORD_ORDER == FALSE	// motorola?
									    if ((pIFF->lastError == 0) && (pSample->bitSize == 16) )
										{
										    INT32			count, frames;
										    short int		*pAudio;

										    pAudio = (short int *)pSample->theWaveform;
										    frames = pSample->waveFrames * pSample->channels;
										    for (count = 0; count < frames; count++)
											{
											    *pAudio = XSwapShort(*pAudio);
											    pAudio++;
											}
										}
#endif
									}
								    else
									{
									    pIFF->lastError = BAD_FILE;
									}
								    break;
								case X_WAVE_FORMAT_IMA_ADPCM:
								    pIFF->lastError = PV_ReadWAVEAndDecompressIMA(pIFF->fileReference, 
														  sourceLength,
														  (char *)pSample->theWaveform,
														  pSample->waveSize,
														  pSample->bitSize,
														  pSample->channels,
														  NULL,	// allocate block buffer
														  waveHeader.wfx.nBlockAlign,
														  NULL);
								    break;
								}
							}
						    else
							{
							    pIFF->lastError = MEMORY_ERR;
							}
						}
					    else
						{
						    // now the file is positioned right at the data block
						    pSample->currentFilePosition = XFileGetPosition(pIFF->fileReference);
						}
					}
				    else
					{
					    pIFF->lastError = BAD_FILE_TYPE;
					}
				}
			    else
				{
				    pIFF->lastError = BAD_FILE_TYPE;
				}
			    if (pIFF->lastError)
				{
				    if (pSample)
					{
					    XDisposePtr((XPTR)pSample->theWaveform);
					}
				    XDisposePtr((XPTR)pSample);
				    pSample = NULL;
				}
			}
		    else
			{
			    pIFF->lastError = MEMORY_ERR;
			}
		    XFileClose(pIFF->fileReference);
		}
	    if (pError)
		{
		    *pError = pIFF->lastError;
		}
	    XDisposePtr((XPTR)pIFF);
	}
    return pSample;
}


/*
 * C O N V E R T   F R O M   I E E E   E X T E N D E D  
 */

/* 
 * Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */

#define UNSIGNED_TO_FLOAT(u)         (((double)((INT32)(u - 2147483647L - 1))) + 2147483648.0)

/****************************************************************
 * Extended precision IEEE floating-point conversion routine.
 ****************************************************************/

XFIXED XConvertFromIeeeExtended(unsigned char *bytes)
{
    double			f;
    int				expon;
    UINT32	hiMant, loMant;
    XFIXED			ieeeRate;

    expon = ((bytes[0] & 0x7F) << 8L) | (bytes[1] & 0xFF);
    hiMant	=    ((UINT32)(bytes[2] & 0xFF) << 24L)
	|    ((UINT32)(bytes[3] & 0xFF) << 16L)
	|    ((UINT32)(bytes[4] & 0xFF) << 8L)
	|    ((UINT32)(bytes[5] & 0xFF));
    loMant	=    ((UINT32)(bytes[6] & 0xFF) << 24L)
	|    ((UINT32)(bytes[7] & 0xFF) << 16L)
	|    ((UINT32)(bytes[8] & 0xFF) << 8L)
	|    ((UINT32)(bytes[9] & 0xFF));

    if (expon == 0 && hiMant == 0 && loMant == 0) 
	{
	    f = 0;
	}
    else 
	{
	    if (expon == 0x7FFF) 
		{    /* Infinity or NaN */
		    //#ifndef HUGE_VAL
		    //	#define HUGE_VAL HUGE
		    //#endif
		    //			f = HUGE_VAL;
		    f = 0.0;
		}
	    else 
		{
		    expon -= 16383;
		    expon -= 31;
		    f  = ldexp(UNSIGNED_TO_FLOAT(hiMant), expon);
		    expon -= 32;
		    f += ldexp(UNSIGNED_TO_FLOAT(loMant), expon);
		}
	}

    if (bytes[0] & 0x80)
	{
	    f = -f;
	}
    ieeeRate = FLOAT_TO_XFIXED(f);
    return ieeeRate;
}

/*
 * C O N V E R T   T O   I E E E   E X T E N D E D
 */

/* Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */

#define FLOAT_TO_UNSIGNED(f)      ((UINT32)(((INT32)(f - 2147483648.0)) + 2147483647L + 1))

void XConvertToIeeeExtended(XFIXED ieeeFixedRate, unsigned char *bytes)
{
    int				sign;
    int				expon;
    double			fMant, fsMant;
    UINT32	hiMant, loMant;
    double			num;

    num = XFIXED_TO_FLOAT(ieeeFixedRate);
    if (num < 0) 
	{
	    sign = 0x8000;
	    num *= -1;
	} 
    else 
	{
	    sign = 0;
	}

    if (num == 0) 
	{
	    expon = 0; hiMant = 0; loMant = 0;
	}
    else 
	{
	    fMant = frexp(num, &expon);
	    if ((expon > 16384) || !(fMant < 1)) 
		{    /* Infinity or NaN */
		    expon = sign|0x7FFF; 
		    hiMant = 0; 
		    loMant = 0; /* infinity */
		}
	    else 
		{    /* Finite */
		    expon += 16382;
		    if (expon < 0) 
			{    /* denormalized */
			    fMant = ldexp(fMant, expon);
			    expon = 0;
			}
		    expon |= sign;
		    fMant = ldexp(fMant, 32);          
		    fsMant = floor(fMant); 
		    hiMant = FLOAT_TO_UNSIGNED(fsMant);
		    fMant = ldexp(fMant - fsMant, 32); 
		    fsMant = floor(fMant); 
		    loMant = FLOAT_TO_UNSIGNED(fsMant);
		}
	}
    
    bytes[0] = expon >> 8;
    bytes[1] = expon;
    bytes[2] = (char)(hiMant >> 24);
    bytes[3] = (char)(hiMant >> 16);
    bytes[4] = (char)(hiMant >> 8);
    bytes[5] = (char)hiMant;
    bytes[6] = (char)(loMant >> 24);
    bytes[7] = (char)(loMant >> 16);
    bytes[8] = (char)(loMant >> 8);
    bytes[9] = (char)loMant;
}

// get AIFF header. Returns 0 if ok, -1 if failed
static INT32 IFF_GetAIFFHeader(X_IFF *pIFF, XAIFFHeader * pHeaderInfo)
{
    INT32	theErr;

    theErr = IFF_GetChunk(pIFF, X_Common, (INT32)sizeof(XAIFFHeader), (void *)pHeaderInfo);

#if X_WORD_ORDER != FALSE	// intel?
    pHeaderInfo->numChannels = XSwapShort(pHeaderInfo->numChannels);
    pHeaderInfo->numSampleFrames = XSwapLong(pHeaderInfo->numSampleFrames);
    pHeaderInfo->sampleSize = XSwapShort(pHeaderInfo->sampleSize);
#endif
    return theErr;
}

// get AIFF extended header. Returns 0 if ok, -1 if failed
static INT32 IFF_GetAIFFExtenedHeader(X_IFF *pIFF, XAIFFExtenedHeader * pHeaderInfo)
{
    INT32	theErr;
    char	size;

    theErr = IFF_GetChunk(pIFF, X_Common, (INT32)sizeof(XAIFFHeader) + (INT32)sizeof(INT32), (void *)pHeaderInfo);

#if X_WORD_ORDER != FALSE	// intel?
    pHeaderInfo->numChannels = XSwapShort(pHeaderInfo->numChannels);
    pHeaderInfo->numSampleFrames = XSwapLong(pHeaderInfo->numSampleFrames);
    pHeaderInfo->sampleSize = XSwapShort(pHeaderInfo->sampleSize);
    pHeaderInfo->compressionType = XSwapLong(pHeaderInfo->compressionType);
#endif
	
    theErr = XFileRead(pIFF->fileReference, &size, 1L);
    pHeaderInfo->compressionName[0] = size;
    theErr = XFileRead(pIFF->fileReference, &pHeaderInfo->compressionName[1], size);
    return theErr;
}

static INT32 IFF_GetAIFFInstrument(X_IFF *pIFF, XInstrumentHeader * pInstrumentInfo)
{
    INT32	theErr;

    theErr = IFF_GetChunk(pIFF, X_Instrument, (INT32)sizeof(XInstrumentHeader), (void *)pInstrumentInfo);

#if X_WORD_ORDER != FALSE	// intel?
    pInstrumentInfo->gain = XSwapShort(pInstrumentInfo->gain);
    pInstrumentInfo->sustainLoop_playMode = XSwapShort(pInstrumentInfo->sustainLoop_playMode);
    pInstrumentInfo->sustainLoop_beginLoop = XSwapShort(pInstrumentInfo->sustainLoop_beginLoop);
    pInstrumentInfo->sustainLoop_endLoop = XSwapShort(pInstrumentInfo->sustainLoop_endLoop);
    pInstrumentInfo->releaseLoop_beginLoop = XSwapShort(pInstrumentInfo->releaseLoop_beginLoop);
    pInstrumentInfo->releaseLoop_endLoop = XSwapShort(pInstrumentInfo->releaseLoop_endLoop);
    pInstrumentInfo->extra = XSwapShort(pInstrumentInfo->extra);
#endif
    return theErr;
}


/*
	unsigned short					numMarkers;			// 2 markers
	short							id
	unsigned long					position;			// 1
	pstring							name;				// begloop
	short							id
	unsigned long					position;			// 2
	pstring							name;				// endloop
*/
// Returns AIFF loop points, if there. Return 0 if successful, -1 if failure
static INT32 IFF_GetAIFFLoopPoints(X_IFF *pIFF, UINT32 *pLoopStart, UINT32 *pLoopEnd)
{
    char	loopMark[1024];
    INT32	theErr;
    char	*pData;
    short	len;

    *pLoopStart = 0;
    *pLoopEnd = 0;
    theErr = IFF_GetChunk(pIFF, X_Marker, 1023L, loopMark);
    if (theErr == 0)
	{
	    pData = loopMark;
	    len = XGetShort(pData);
	    if (len >= 2)
		{
		    pData += 2;				// skip marker count
		    pData += 2;				// skip marker id
		    *pLoopStart = XGetLong(pData);
		    pData += 4;				// skip past start

		    len = *pData;
		    pData += len + 1;		// walk past first string
		    pData += ODD(pData);

		    pData += 2;				// skip marker id
		    *pLoopEnd = XGetLong(pData);
		}
	}
    return theErr;
}

// Get compressed and uncompressed size. Return 0 if successful, -1 if failure
static INT32 IFF_GetAIFFSampleSize(X_IFF *pIFF, INT32 *pUncompressedSize, INT32 *pCompressedSize)
{
    INT32				size, error;
    XAIFFExtenedHeader	header;

    size = 0L;
    error = 0;
    if (pUncompressedSize && pCompressedSize)
	{
	    switch (pIFF->headerType)
		{
		case X_AIFF:
		    error = IFF_GetAIFFHeader(pIFF, (XAIFFHeader *)&header);
		    size = header.numSampleFrames * header.numChannels * (header.sampleSize / 8);
		    break;
		case X_AIFC:
		    error = IFF_GetAIFFExtenedHeader(pIFF, &header);
		    switch (header.compressionType)
			{
			default:
			    pIFF->lastError = BAD_FILE_TYPE;
			    // fail because we don't know how to decompress
			    break;
			case X_NONE:
			    size = header.numSampleFrames * header.numChannels * (header.sampleSize / 8);
			    break;
			case X_IMA4:
			    // Sound Manager defines 64 samples per packet number of sample frames
			    size = header.numSampleFrames * AIFF_IMA_BLOCK_FRAMES * header.numChannels * (header.sampleSize / 8);
			    break;
			}
		    break;
		}
	}

    if (size)
	{
	    // now position right to data block
	    if (IFF_ScanToBlock(pIFF, X_SoundData))	// skip to body
		{
		    error = -1;	// failed
		}
	    else
		{
		    XFileSetPositionRelative(pIFF->fileReference, -4L);		// back-up and get size
		    if (XFileRead(pIFF->fileReference, pCompressedSize, (INT32)sizeof(INT32)) == -1)
			{
			    pIFF->lastError = BAD_FILE;
			    error = -1;
			}
		    *pCompressedSize = XGetLong(pCompressedSize);
		    //			XFileSetPositionRelative(pIFF->fileReference, sizeof(long) * 2L);
		}
	}
    *pUncompressedSize = size;
    return error;
}

#define AIFF_IMA_BUFFER_SIZE	AIFF_IMA_BLOCK_BYTES * 40

static OPErr PV_ReadAIFFAndDecompressIMA(XFILE fileReference, INT32 sourceLength,
					 unsigned char *pDestSample, INT32 destLength,
					 char outputBitSize, char channels,
					 UINT32 *pBufferLength,
					 short predictorCache[2])
{
    XBYTE		codeBlock[AIFF_IMA_BUFFER_SIZE];
    INT32		writeBufferLength, size, offset;
    OPErr		err;

    err = NO_ERR;
    writeBufferLength = 0;
    sourceLength -= sourceLength % AIFF_IMA_BUFFER_SIZE;	// round to block size

#if USE_DEBUG && 0
    {
	char text[256];
		
	sprintf(text, "sourceLength %ld, AIFF_IMA_BUFFER_SIZE %ld", sourceLength, AIFF_IMA_BUFFER_SIZE);
	DEBUG_STR(XCtoPstr(text));
    }
#endif

    while (sourceLength > 0)
	{
	    if (sourceLength > AIFF_IMA_BUFFER_SIZE)
		{
		    size = XFileGetPosition(fileReference);	 /* get current pos */
		    XFileRead(fileReference, codeBlock, AIFF_IMA_BUFFER_SIZE);
		    size = XFileGetPosition(fileReference) - size;
		}
	    else
		{
		    // last block so just stop
		    size = 0;
		}
	    sourceLength -= AIFF_IMA_BUFFER_SIZE;
	    offset = 0;
	    if (size)
		{
		    offset = XExpandAiffImaStream(codeBlock, AIFF_IMA_BLOCK_BYTES,
						  pDestSample, outputBitSize,
						  size, channels, predictorCache);
		    if (offset == 0)
			{
				// we're done
			    break;
			}
		    destLength -= offset;
		    if (destLength < 0)
			{
				// time to quit, we've hit the end
			    break;
			}
		    else
			{
			    pDestSample += offset;
			    writeBufferLength += offset;
			}
		}
	}
    if (pBufferLength)
	{
	    *pBufferLength = writeBufferLength;
	}
    return err;
}

// This will read into memory the entire AIFF file and return a GM_Waveform structure.
// When disposing make sure and dispose of both the GM_Waveform structure and the
// theWaveform inside of that structure with XDisposePtr
static GM_Waveform * PV_ReadIntoMemoryAIFFFile(XFILENAME *file, XBOOL loadData, 
					       XPTR pMemoryFile, UINT32 memoryFileSize,
					       INT32 *pFormat, UINT32 *pBlockSize, OPErr *pError)
{
    GM_Waveform			*pSample;
    X_IFF				*pIFF;
    XAIFFExtenedHeader	aiffHeader;
    INT32				type, size, sourceLength;

    if (pError)
	{
	    *pError = NO_ERR;
	}
    XSetMemory(&aiffHeader, (INT32)sizeof(XAIFFExtenedHeader), 0);
    pSample = NULL;
    if (pFormat)
	{
	    *pFormat = 0;
	}
    if (pBlockSize)
	{
	    *pBlockSize = 0;
	}
    pIFF = (X_IFF *)XNewPtr((INT32)sizeof(X_IFF));
    if (pIFF)
	{
	    IFF_SetFormType(pIFF, X_FORM);
	    if (file)
		{
		    pIFF->fileReference = XFileOpenForRead(file);
		}
	    else
		{
		    if (pMemoryFile)
			{
			    pIFF->fileReference = XFileOpenForReadFromMemory(pMemoryFile, memoryFileSize);
			}
		}
	    if (pIFF->fileReference)
		{
		    pSample = (GM_Waveform *)XNewPtr((INT32)sizeof(GM_Waveform));
		    if (pSample)
			{
			    type = IFF_FileType(pIFF);
			    pIFF->headerType = type;
			    if ( (type == X_AIFF) || (type == X_AIFC) )
				{
				    if (type == X_AIFF)
					{
					    IFF_GetAIFFHeader(pIFF, (XAIFFHeader *)&aiffHeader);
					    aiffHeader.compressionType = X_NONE;
					}
				    else
					{
					    IFF_GetAIFFExtenedHeader(pIFF, &aiffHeader);
					    if (pBlockSize)
						{
						    *pBlockSize = (UINT32)sizeof(INT32) * 2;
						}
					}
				    if (pFormat)
					{
					    *pFormat = aiffHeader.compressionType;
					}
				    pSample->channels = (UBYTE)aiffHeader.numChannels;
				    pSample->bitSize = (UBYTE)aiffHeader.sampleSize;
				    pSample->baseMidiPitch = 60;

				    // get loop points, if there
				    IFF_GetAIFFLoopPoints(pIFF, &pSample->startLoop, &pSample->endLoop);
					
				    // Convert the ieee number into a 16.16 fixed value
				    size = 0;
				    pSample->sampledRate = XConvertFromIeeeExtended(aiffHeader.sampleRate);
				    if (pSample->sampledRate == 0)
					{
					    pIFF->lastError = BAD_SAMPLE;
					}
				    else
					{
					    // we want the file byte size, uncompressed
					    if (IFF_GetAIFFSampleSize(pIFF, &size, &sourceLength) == 0)
						{
						    pSample->waveSize = size;
						    pSample->waveFrames = pSample->waveSize / pSample->channels;
						    if (pSample->bitSize > 8)
							{
							    pSample->waveFrames /= 2;
							}
						}
					}
				    if (size == 0)
					{
					    pIFF->lastError = BAD_SAMPLE;
					}
				    // now the file is positioned right at the data block. So let's allocate it and read
				    // it into memory
				    XFileSetPositionRelative(pIFF->fileReference, (INT32)sizeof(INT32) * 2L);
				    if (loadData && size)
					{
					    pSample->theWaveform = (SBYTE *)XNewPtr(pSample->waveSize);
					    if (pSample->theWaveform)
						{
						    switch (aiffHeader.compressionType)
							{
							default:
							    pIFF->lastError = BAD_FILE_TYPE;
							    break;
							case X_NONE:
							    if (XFileRead(pIFF->fileReference, 
									  pSample->theWaveform,
									  pSample->waveSize) != -1)
								{
#if X_WORD_ORDER != FALSE	// intel?
								    short int		*pAudio;
								    INT32			count, frames;

								    // now, if the file is 16 bit sample on a intel ordered system, swap the bytes
								    if (pSample->bitSize == 16)
									{
									    pAudio = (short int *)pSample->theWaveform;
									    frames = pSample->waveFrames * pSample->channels;
									    for (count = 0; count < frames; count++)
										{
										    *pAudio = XSwapShort(*pAudio);
										    pAudio++;
										}
									}
#endif
								}
							    else
								{
								    pIFF->lastError = BAD_FILE;
								}
							    break;

							case X_IMA4:
							    {
								short		predictorCache[2];
									
								predictorCache[0] = 0;
								predictorCache[1] = 0;
								pIFF->lastError = PV_ReadAIFFAndDecompressIMA(pIFF->fileReference,
													      sourceLength,
													      (unsigned char *)pSample->theWaveform,
													      pSample->waveSize,
													      pSample->bitSize,
													      pSample->channels,
													      NULL,
													      predictorCache);
							    }
							    break;
							}

						    if (pIFF->lastError == 0)
							{
							    // now, if the file is 8 bit sample, change the sample phase
							    if (pSample->bitSize == 8)
								{
								    XPhase8BitWaveform((unsigned char *)pSample->theWaveform, pSample->waveSize);
								}
							}
						}
					    else
						{
						    pIFF->lastError = MEMORY_ERR;
						}
					}
				    else
					{
					    // now the file is positioned right at the data block
					    pSample->currentFilePosition = XFileGetPosition(pIFF->fileReference);
					}
				}
			    else
				{
				    pIFF->lastError = BAD_FILE_TYPE;
				}
			    if (pError)
				{
				    *pError = (OPErr)pIFF->lastError;
				}
			    if (pIFF->lastError)
				{
				    if (pSample)
					{
					    XDisposePtr((XPTR)pSample->theWaveform);
					}
				    XDisposePtr((XPTR)pSample);
				    pSample = NULL;
				}
			}
		    else
			{
			    pIFF->lastError = MEMORY_ERR;
			}
		    XFileClose(pIFF->fileReference);
		}
	    XDisposePtr((XPTR)pIFF);
	}
    return pSample;
}

// This will read into memory the entire Sun AU file and return a GM_Waveform structure.
// When disposing make sure and dispose of both the GM_Waveform structure and the
// theWaveform inside of that structure with XDisposePtr
static GM_Waveform * PV_ReadIntoMemorySunAUFile(XFILENAME *file, XBOOL loadData, 
						XPTR pMemoryFile, UINT32 memoryFileSize,
						INT32 *pFormat, UINT32 *pBlockSize, OPErr *pError)
{
    GM_Waveform			*pSample;
    SunAudioFileHeader	sunHeader;
    XFILE				fileReference;
    OPErr				err;
    INT32				encoding, size;
    short int			bits;
    INT32				filePos, originalLength, waveLength;

    pSample = NULL;
    fileReference = 0;
    waveLength = 0;
    if (pBlockSize)
	{
	    *pBlockSize = 0;
	}
    if (file)
	{
	    fileReference = XFileOpenForRead(file);
	}
    else
	{
	    if (pMemoryFile)
		{
		    fileReference = XFileOpenForReadFromMemory(pMemoryFile, memoryFileSize);
		}
	}
    err = NO_ERR;
    if (fileReference)
	{
	    if (XFileRead(fileReference, &sunHeader, (INT32)sizeof(SunAudioFileHeader)) == 0)
		{
		    // now skip past any info string
		    size = XGetLong(&sunHeader.hdr_size) - (INT32)sizeof(SunAudioFileHeader);
		    filePos = XFileGetPosition(fileReference) + size;
		    originalLength = XFileGetLength(fileReference) - size + (INT32)sizeof(SunAudioFileHeader);

		    XFileSetPosition(fileReference, filePos);	// go back to ending
		    // Make sure we've got a legitimate audio file
		    encoding = XGetLong(&sunHeader.encoding);
		    if(XGetLong(&sunHeader.magic) == SUN_AUDIO_FILE_MAGIC_NUMBER)
			{
				// determine what file types we support
			    bits = -1;
			    switch (encoding)
				{
				case SUN_AUDIO_FILE_ENCODING_LINEAR_8:
				    waveLength = originalLength;
				    bits = 8;
				    break;
				case SUN_AUDIO_FILE_ENCODING_MULAW_8:
				case SUN_AUDIO_FILE_ENCODING_ALAW_8:
				    waveLength = originalLength * 2;
				    bits = 16;
				    break;
				case SUN_AUDIO_FILE_ENCODING_LINEAR_16:
				    waveLength = originalLength;
				    bits = 16;
				    break;
				case SUN_AUDIO_FILE_ENCODING_ADPCM_G721:
				    waveLength = originalLength * 4;
				    bits = 16;
				    break;
				case SUN_AUDIO_FILE_ENCODING_ADPCM_G723_3:
				case SUN_AUDIO_FILE_ENCODING_ADPCM_G723_5:
				    waveLength = originalLength * 4;
				    bits = 16;
				    break;
				}
			    if (pFormat)
				{
				    *pFormat = encoding;
				}
			    if (bits != -1)
				{
				    pSample = (GM_Waveform *)XNewPtr((INT32)sizeof(GM_Waveform));
				    if (pSample)
					{
					    pSample->channels = (UBYTE)XGetLong(&sunHeader.channels);
					    pSample->baseMidiPitch = 60;
					    pSample->bitSize = (UBYTE)bits;
					    pSample->sampledRate = XGetLong(&sunHeader.sample_rate) << 16L;
					    // we want the byte size
					    pSample->waveSize = waveLength;
					    // now the file is positioned right at the data block. So let's allocate it and read
					    // it into memory
					    if (loadData)
						{
						    pSample->theWaveform = (SBYTE *)XNewPtr(pSample->waveSize);
						    if (pSample->theWaveform)
							{
							    pSample->waveFrames = pSample->waveSize / pSample->channels;
							    if (pSample->bitSize > 8)
								{
								    pSample->waveFrames /= 2;
								}

							    err = PV_ReadSunAUFile(encoding, fileReference,
										   pSample->theWaveform, pSample->waveSize, NULL);
							    if (err == 0)
								{
								    // we don't need to byte swap these based upon platform because
								    // we generate samples from runtime code, so the results are in
								    // native format already

								    // now, if the file is 8 bit sample, change the sample phase
								    if (pSample->bitSize == 8)
									{
									    XPhase8BitWaveform((unsigned char *)pSample->theWaveform, pSample->waveSize);
									}
								}
							}
						    else
							{
							    err = MEMORY_ERR;
							}
						}
					    else
						{
						    // now the file is positioned right at the data block
						    pSample->currentFilePosition = XFileGetPosition(fileReference);
						}
					}
				    else
					{
					    err = BAD_FILE_TYPE;
					}
				    if (err)
					{
					    if (pSample)
						{
						    XDisposePtr((XPTR)pSample->theWaveform);
						}
					    XDisposePtr((XPTR)pSample);
					    pSample = NULL;
					}
				}
			    else
				{
				    err = MEMORY_ERR;
				}
			}
		    else
			{
			    err = BAD_FILE_TYPE;
			}
		}
	    else
		{
		    err = BAD_FILE;
		}
	    XFileClose(fileReference);
	}
    if (pError)
	{
	    *pError = err;
	}
    return pSample;
}


#if USE_MPEG_DECODER != 0
// Functions specific to HAE
static GM_Waveform * PV_ReadIntoMemoryMPEGFile(XFILENAME *file, XBOOL loadData, 
					       XPTR pMemoryFile, UINT32 memoryFileSize,
					       INT32 *pFormat, void **ppBlockPtr, UINT32 *pBlockSize, OPErr *pError)
{

    GM_Waveform			*pSample;
    XFILE				fileReference;
    OPErr				err;
    XMPEGDecodedData	*pMPEGStream;
    UINT32		bufferSize;

    pSample = NULL;
    fileReference = 0;
    pFormat;
    if (pBlockSize)
	{
	    *pBlockSize = 0;
	}
    if (file)
	{
	    fileReference = XFileOpenForRead(file);
	}
    else
	{
	    if (pMemoryFile)
		{
		    fileReference = XFileOpenForReadFromMemory(pMemoryFile, memoryFileSize);
		}
	}
    err = NO_ERR;
    pMPEGStream = NULL;
    if (fileReference)
	{
	    // file is already open, create and validate an MPEG stream
	    pMPEGStream = XOpenMPEGStreamFromXFILE(fileReference, &err);

	    if (pMPEGStream && (err == NO_ERR))
		{
		    pSample = (GM_Waveform *)XNewPtr((INT32)sizeof(GM_Waveform));
		    if (pSample)
			{
			    pSample->channels = (UBYTE)pMPEGStream->channels;
			    pSample->bitSize = (UBYTE)pMPEGStream->bitSize;
			    pSample->baseMidiPitch = 60;
			    pSample->sampledRate = pMPEGStream->sampleRate;
				// we want the byte size
			    pSample->waveSize = pMPEGStream->lengthInBytes;
			    pSample->waveFrames = pMPEGStream->lengthInSamples;
			    bufferSize = pMPEGStream->frameBufferSize;

			    if (pBlockSize)
				{
				    *pBlockSize = bufferSize;
				}
				// now the file is positioned right at the data block. So let's allocate it and read
				// it into memory
			    if (loadData)
				{
				    //					if (pSample->waveFrames < MAX_SAMPLE_FRAMES)
				    {
					pSample->theWaveform = (SBYTE *)XNewPtr(pSample->waveSize);
					if (pSample->theWaveform)
					    {
						// now decode the mpeg sample and store into the resulting buffer
						{
						    char			*pBuffer;
						    UINT32	realSize;
						    XBOOL			done;
							
						    pBuffer = pSample->theWaveform;
						    realSize = 0;
						    while (realSize <= pSample->waveSize)
							{
							    done = FALSE;
							    if (XFillMPEGStreamBuffer(pMPEGStream, pBuffer, &done) == NO_ERR)
								{
								    if (done == FALSE)
									{
									    pBuffer += bufferSize;
									    realSize += bufferSize;
									}
								    else
									{
									    break;
									}
								}
							    else
								{
								    break;
								}
							}
						}
					    }
					else
					    {
						err = MEMORY_ERR;
					    }
				    }
				    //					else
				    //					{
				    //						err = SAMPLE_TO_LARGE;
				    //					}
				    XCloseMPEGStream(pMPEGStream);
				    pMPEGStream = NULL;
				}
			    else
				{
				    // now if a ppBlockPtr was passed in, then we want to keep the mpeg stream open
				    // because we're going to continue to decode from it, otherwise we just close it.
				    if (ppBlockPtr)
					{
					    *ppBlockPtr = (void *)pMPEGStream;
					}
				    else
					{
					    XCloseMPEGStream(pMPEGStream);
					    pMPEGStream = NULL;
					}
				}
			}
		    else
			{
			    err = MEMORY_ERR;
			}
		}
	    else
		{
		    err = BAD_FILE_TYPE;
		}
	    if (err)
		{
		    if (pMPEGStream)
			{
			    XCloseMPEGStream(pMPEGStream);
			    pMPEGStream = NULL;
			}
		    if (pSample)
			{
			    XDisposePtr((XPTR)pSample->theWaveform);
			}
		    XDisposePtr((XPTR)pSample);
		    pSample = NULL;
		}
	}
    else
	{
	    err = BAD_FILE;
	}
    if (pError)
	{
	    *pError = err;
	}
    return pSample;
}
#endif	// USE_MPEG_DECODER != FALSE

// Read into memory a file
GM_Waveform * GM_ReadFileIntoMemory(XFILENAME *file, AudioFileType fileType, OPErr *pErr)
{
    GM_Waveform *pWave;
    OPErr		err;

    pWave = NULL;
    err = NO_ERR;
    switch (fileType)
	{
	case FILE_AIFF_TYPE:
	    pWave = PV_ReadIntoMemoryAIFFFile(file, TRUE, NULL, 0L, NULL, NULL, &err);		// load samples
	    break;
	case FILE_WAVE_TYPE:
	    pWave = PV_ReadIntoMemoryWaveFile(file, TRUE, NULL, 0L, NULL, NULL, &err);		// load samples
	    break;
	case FILE_AU_TYPE:
	    pWave = PV_ReadIntoMemorySunAUFile(file, TRUE, NULL, 0L, NULL, NULL, &err);		// load samples
	    break;
#if USE_MPEG_DECODER != FALSE
	case FILE_MPEG_TYPE:
	    pWave = PV_ReadIntoMemoryMPEGFile(file, TRUE, NULL, 0L, NULL, NULL, NULL, &err);		// load samples
	    break;
#endif
	}
    if (pErr)
	{
	    *pErr = err;
	}
    return pWave;
}

// Read into memory a file
GM_Waveform * GM_ReadFileIntoMemoryFromMemory(void *pFileBlock, UINT32 fileBlockSize, AudioFileType fileType, OPErr *pErr)
{
    GM_Waveform *pWave;
    OPErr		err;

    pWave = NULL;
    err = NO_ERR;
    switch (fileType)
	{
	case FILE_AIFF_TYPE:
	    pWave = PV_ReadIntoMemoryAIFFFile(NULL, TRUE, pFileBlock, fileBlockSize, NULL, NULL, &err);		// load samples
	    break;
	case FILE_WAVE_TYPE:
	    pWave = PV_ReadIntoMemoryWaveFile(NULL, TRUE, pFileBlock, fileBlockSize, NULL, NULL, &err);		// load samples
	    break;
	case FILE_AU_TYPE:
	    pWave = PV_ReadIntoMemorySunAUFile(NULL, TRUE, pFileBlock, fileBlockSize, NULL, NULL, &err);	// load samples
	    break;
#if USE_MPEG_DECODER != FALSE
	case FILE_MPEG_TYPE:
	    pWave = PV_ReadIntoMemoryMPEGFile(NULL, TRUE, pFileBlock, fileBlockSize, NULL, NULL, NULL, &err);		// load samples
	    break;
#endif
	}
    if (pErr)
	{
	    *pErr = err;
	}
    return pWave;
}

// Read file information from file, which is a fileType file. If pFormat is not NULL, then
// store format specific format type
GM_Waveform * GM_ReadFileInformation(XFILENAME *file, AudioFileType fileType, 
				     INT32 *pFormat, 
				     void **ppBlockPtr, UINT32 *pBlockSize, 
				     OPErr *pErr)
{
    GM_Waveform *pWave;
    OPErr		err;

#if USE_MPEG_DECODER == 0
    ppBlockPtr;
#endif
    pWave = NULL;
    err = NO_ERR;
    if (pBlockSize)
	{
	    *pBlockSize = 0;
	}
    switch (fileType)
	{
	case FILE_AIFF_TYPE:
	    pWave = PV_ReadIntoMemoryAIFFFile(file, FALSE, NULL, 0L, 
					      pFormat, pBlockSize, &err);		// don't load samples
	    if (pBlockSize)
		{
		    *pBlockSize = (UINT32)sizeof(INT32) * 2;
		}
	    break;
	case FILE_WAVE_TYPE:
	    pWave = PV_ReadIntoMemoryWaveFile(file, FALSE, NULL, 0L, 
					      pFormat, pBlockSize, &err);		// don't load samples
	    break;
	case FILE_AU_TYPE:
	    pWave = PV_ReadIntoMemorySunAUFile(file, FALSE, NULL, 0L,
					       pFormat, pBlockSize, &err);		// don't load samples
	    break;
#if USE_MPEG_DECODER != FALSE
	case FILE_MPEG_TYPE:
	    pWave = PV_ReadIntoMemoryMPEGFile(file, FALSE, NULL, 0L,
					      pFormat, ppBlockPtr, pBlockSize, &err);		// don't load samples
	    break;
#endif
	}
    if (pErr)
	{
	    *pErr = err;
	}
    return pWave;
}

// Read a block of data, based apon file type and format, decode and store into a buffer.
// Return length of buffer stored or 0 if error.
OPErr GM_ReadAndDecodeFileStream(XFILE fileReference, 
				 AudioFileType fileType, INT32 format, 
				 XPTR pBlockBuffer, UINT32 blockSize,
				 XPTR pBuffer, UINT32 bufferFrames,
				 short int channels, short int bitSize,
				 UINT32 *pStoredBufferLength,
				 UINT32 *pReadBufferLength)
{
    UINT32			returnedLength, writeLength, bufferSize;
    OPErr					fileError;
    unsigned short int		*pAudio;
    INT32					count, frames, filePosition;
    XBOOL					calculateFileSize;

    returnedLength = 0;
    writeLength = 0;
    calculateFileSize = TRUE;
    filePosition = 0;
    fileError = NO_ERR;
    if (pBuffer)
	{
	    if (fileReference)
		{
		    filePosition = XFileGetPosition(fileReference);
		}
	    bufferSize = bufferFrames * channels * (bitSize / 8);
	    switch (fileType)
		{
		case FILE_AU_TYPE:
		    fileError = PV_ReadSunAUFile(format, fileReference,
						 pBuffer, bufferSize, &writeLength);
				
		    calculateFileSize = FALSE;
		    returnedLength = writeLength;
				// adjust for size actually read into the buffer
		    //				returnedLength = XFileGetPosition(fileReference) - filePosition;	// length of data read from the file
		    break;
#if USE_MPEG_DECODER != 0
		case FILE_MPEG_TYPE:
		    if (pBlockBuffer)
			{
			    XMPEGDecodedData	*pMPG;
			    XBOOL				mpegDone;
			    char				*pcmAudio;					

			    pMPG = (XMPEGDecodedData *)pBlockBuffer;
			    frames = bufferSize / pMPG->frameBufferSize;
			    pcmAudio = (char *)pBuffer;
			    for (count = 0; count < frames; count++)
				{
				    fileError = XFillMPEGStreamBuffer(pMPG, pcmAudio, &mpegDone);
				    if (mpegDone == FALSE)
					{
					    pcmAudio += pMPG->frameBufferSize;
					}
				    else
					{	// done
					    fileError = BAD_FILE;
					    break;
					}
				}
			    calculateFileSize = FALSE;
			    writeLength = pMPG->frameBufferSize * frames;
			    returnedLength = writeLength;
			}
		    break;
#endif
		case FILE_AIFF_TYPE:
		    switch(format)
			{
			default:
			    calculateFileSize = FALSE;
			    break;
			case X_NONE:
			    if (XFileRead(fileReference, 
					  pBuffer,
					  bufferSize) == -1)
				{
				    fileError = BAD_FILE;
				}
			    else
				{
#if X_WORD_ORDER != FALSE	// intel?
				    // now, if the file is 16 bit sample on a intel ordered system, swap the bytes
				    if (bitSize == 16)
					{
					    pAudio = (unsigned short int *)pBuffer;
					    frames = bufferFrames * channels;	// convert to sample frames
					    for (count = 0; count < frames; count++)
						{
						    *pAudio = XSwapShort(*pAudio);
						    pAudio++;
						}
					}
#endif
				    // now, if the file is 8 bit sample, change the sample phase
				    if (bitSize == 8)
					{
					    XPhase8BitWaveform((unsigned char *)pBuffer, bufferSize);
					}
				}
			    break;
			case X_IMA4:
			    if (pBlockBuffer)
				{
				    fileError = PV_ReadAIFFAndDecompressIMA(fileReference,
									    bufferSize / 4,
									    (unsigned char *)pBuffer,
									    bufferSize,
									    (char)bitSize,
									    (char)channels,
									    (UINT32 *)&writeLength,
									    (short*)pBlockBuffer);

				    returnedLength = bufferSize / 4;
				}
			    else
				{
				    writeLength = 0;
				    returnedLength = 0;
				}
			    calculateFileSize = FALSE;
			    break;
			}
		    break;
		case FILE_WAVE_TYPE:				
		    switch(format)
			{
			default:
			    calculateFileSize = FALSE;
			    fileError = BAD_FILE_TYPE;
			    break;
			case X_WAVE_FORMAT_PCM:			// normal PCM data
			    if (XFileRead(fileReference, 
					  pBuffer,
					  bufferSize) != -1)
				{
#if X_WORD_ORDER == FALSE	// motorola?
				    // now, if the file is 16 bit sample on a intel ordered system, swap the bytes
				    if (bitSize == 16)
					{
					    pAudio = (unsigned short int *)pBuffer;
					    frames = bufferFrames * channels;	// convert to sample frames
					    for (count = 0; count < frames; count++)
						{
						    *pAudio = XSwapShort(*pAudio);
						    pAudio++;
						}
					}
#endif
				}
			    else
				{
				    fileError = BAD_FILE;
				}
			    break;

			case X_WAVE_FORMAT_ALAW:
			    fileError = PV_ReadSunAUFile(SUN_AUDIO_FILE_ENCODING_ALAW_8,
							 fileReference, 
							 (char *)pBuffer,
							 bufferSize,
							 &writeLength);
			    break;
			case X_WAVE_FORMAT_MULAW:
			    fileError = PV_ReadSunAUFile(SUN_AUDIO_FILE_ENCODING_MULAW_8,
							 fileReference, 
							 (char *)pBuffer,
							 bufferSize,
							 &writeLength);
			    break;
			case X_WAVE_FORMAT_IMA_ADPCM:	// IMA 4 to 1 compressed data
			    if (pBlockBuffer)
				{
				    fileError = PV_ReadWAVEAndDecompressIMA(fileReference, 
									    bufferSize / 4,
									    (char *)pBuffer,
									    bufferSize,
									    (char)bitSize,
									    (char)channels,
									    pBlockBuffer,
									    blockSize,
									    (UINT32 *)&writeLength);
				    returnedLength = bufferSize / 4;
				    // since we decode the samples at runtime, we don't have
				    // to byte swap the words.
				}
			    else
				{
				    writeLength = 0;
				    returnedLength = 0;
				}
			    calculateFileSize = FALSE;
			    break;
			}
		    break;
		}
	    if (calculateFileSize && fileReference)
		{
		    // adjust for size actually read into the buffer
		    returnedLength = XFileGetPosition(fileReference) - filePosition;	// length of data read from the file
		    writeLength = returnedLength;										// length of data created for audio buffer

#if USE_DEBUG && 0
		    {
			char text[256];
				
			sprintf(text, "%ld %ld", returnedLength, bufferLength);
			DEBUG_STR(XCtoPstr(text));
		    }
#endif
		}
	    *pReadBufferLength = returnedLength;
	    *pStoredBufferLength = writeLength;
	}
    else
	{
	    fileError = PARAM_ERR;
	}
    return fileError;
}
#endif	// USE_HIGHLEVEL_FILE_API

// EOF of GenSoundFiles.c
