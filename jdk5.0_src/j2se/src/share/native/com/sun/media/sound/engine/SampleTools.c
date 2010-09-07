/*
 * @(#)SampleTools.c	1.24 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	SampleTools.c
**
**	Tools for manipulating 'snd' resources.
**
**	Confidential-- Internal use only
**
**	History	-
**	1/31/93		Created
**	6/23/95		Integrated into SoundMusicSys Library
**	11/22/95	Added MACE support for CreateSampleFromSnd
**	12/15/95	Changed memory allocation to X_API
**				Pulled out most Mac specific code
**	1/18/96		Spruced up for C++ extra error checking
**	2/3/96		Removed extra includes
**	2/11/96		No longer relies on MacOS Sound.h
**	2/12/96		Changed Boolean to SMSBoolean
**	2/17/96		Added more platform compile arounds
**	3/28/96		Added XPutShort & XPutLong
**	3/29/96		Tried to figure out ima 4 to 1 and failed
**	4/2/96		Added more snd tools to set loop points, rates, and basekey
**	4/9/96		Changed AIFFSampleRate from extended80 to char [10]
**	4/21/96		Removed CPU edian issues by use XGetShort & XGetLong
**				Removed register usage in parameters
**	4/25/96		Added PV_Swap16BitSamples to deal with 16 bit sample on non 68k ordered platforms
**	5/9/96		Fixed byte swap problem
**	6/30/96		Changed font and re tabbed
**	7/6/96		Fixed possible problem with XGetSamplePtrFromSnd by no zeroing pSampleData
**	7/14/96		Fixed structure alignment problem for Pentium compilers
**	9/22/96		Added the ability to select an IMA 16 bit decompression, or an 8 bit version
**	11/14/96	Removed dependancy on MacSpecificSMS.h
**	12/15/96	Removed last remants of MacOS code and moved it all in MacSpecificSMS.c
**				Moved all snd resource types and structs into X_Formats.h
**	12/30/96	Changed copyright
**	1/30/97		Changed all '' constants to defines
**	5/3/97		Fixed a few potential problems that the MOT compiler found
**	6/20/97		Added XGetSampleNameFromID
**	7/17/97		Wrapped X_WORD_ORDER macros around PV_Swap16BitSamples. Saved code size,
**				and fixed potential bug because its word order based, not platform based
**	7/24/97		Added XCreateSoundObjectFromData & XGetSoundEmbeddedStatus & 
**				XSetSoundEmbeddedStatus
**	8/6/97		Changed XGetSampleNameFromID to be smarter about how many types
**				to search through.
**				Added XCreateAudioObjectFromData
**	8/7/97		Added XTranslateFromWaveformToSampleData
**	9/3/97		Wrapped a few more functions around USE_CREATION_API == TRUE
**	12/16/97	Moe: removed compiler warnings
**	1/31/98		Modified XGetSamplePtrFromSnd to handle the case of samples that
**				are ordered a motorola way on an intel platform. Modify them in place
**				and set a flag that allows for the next reference to be correct
**	2/11/98		Modified XGetSamplePtrFromSnd to elimate code to support MacOS MACE compression
**				Put code wrappers around XPhase8BitWaveform to eliminate it from the compile
**				if its not being used
**				Reduced code size of XGetSamplePtrFromSnd when decoding IMA samples, and fixed
**				a memory bug in which too much data was being allocated.
**	3/5/98		Fixed XCreateSoundObjectFromData to make sure the data created was motorola ordered
**	3/23/98		MOE: Changed name of call to XExpandAiffIma(),
**					also added block size parameter
**				Changed XGetSamplePtrFromSnd to use IMA constant. Changed XCreateSoundObjectFromData
**				to support compression types
**	3/23/98		MOE: fixed XCreateSoundObjectFromData() and consolidated its code a bit
**	3/24/98		Fixed some warnings with XCreateSoundObjectFromData
**	5/12/98		MOE: Changed XCreateSoundObjectFromData() to encode IMA4 with proper "sampleSize" and
**				"forceSample8bit" values  (Since Beatnik ignores the former and MacOS ignores the latter,
**				this wasn't causing an audible bug.)
**	7/6/98		Fixed a compiler warning with XCreateSoundObjectFromData
**	7/7/98		Modified XGetSampleNameFromID to use new XLongResourceID type
**	9/29/98		Renamed firstSoundFormat & secondSoundFormat to XFirstSoundFormat & XSecondSoundFormat
**				Added support for new 'snd ' XThirdSoundFormat
**				Renamed stdSH, extSH, cmpSH	to XStandardHeader, XExtendedHeader, XCompressedHeader
**				Changed parameter type of XSetSoundSampleRate
**	9/30/98		Added XTranslateFromSampleDataToWaveform
**	10/2/98		Added support in XCreateSoundObjectFromData to create ALAW/ULAW/ and MPEG compressed
**				Snd resources
**	10/9/98		Changed XCreateSoundObjectFromData to convert 8 bit to 16 bit data before compressing
**				in ulaw/alaw/mpeg formats
**	10/15/98	Added PV_GetClosestMPEGSampleRate which looks for the closest MPEG rate to use for encoding
**	2/12/99		Renamed USE_HAE_FOR_MPEG to USE_MPEG_DECODER
**	2/23/99		Fixed a bug in XGetSamplePtrFromSnd in which a bad decode type would return an invalid sample
**				block
**
**	JAVASOFT
**	04.02.99	$$kk: 04.02.99: fix for P2 bug #4225976, RMF and MIDI crash w/ jdk1.2.2-M.
**				change from steve hales.
*/
/*****************************************************************************/
#include "X_API.h"
#include "X_Formats.h"

#if X_WORD_ORDER != FALSE
// This is used to swap samples that are ordered in a 68k way, to the native platform
static void PV_Swap16BitSamples(short int *pSamples, INT32 length, short channels)
{
    length *= channels;		// walk through channels also
    while (length--)
	{
	    *pSamples = XGetShort(pSamples);
	    pSamples++;
	}
}
#endif

#if USE_HIGHLEVEL_FILE_API == TRUE
// This is used on 8 bit samples that are sign encoded. We want 0 to 255 not -128 to 127
void XPhase8BitWaveform(unsigned char * pByte, INT32 size)
{
    while (size--)
	{
	    *pByte++ -= 128;
	}
}
#endif

// Given a Mac snd pointer, this will return the encoding type, and a pointer to a SoundHeader structure
static void * PV_GetSoundHeaderPtr(XPTR pRes, short int *pEncode)
{
    XSoundHeader	*pSndBuffer;
    short int		soundFormat;
    short int		numSynths, numCmds;
    INT32			offset;
    char			*pSndFormat;

    numSynths = 0;
    numCmds = 0;
    pSndBuffer = NULL;
    if (pRes && pEncode)
	{
	    *pEncode = -1;

	    pSndFormat = (char *)pRes;
	    soundFormat = XGetShort(pSndFormat);
	    switch (soundFormat)
		{
		case XFirstSoundFormat:
				// look inside the format 1 resource and decode offsets
		    numSynths = XGetShort(pSndFormat + 2);
		    numCmds = XGetShort(pSndFormat + 4 + numSynths * 6);		// get number of commands

				// compute address of sound header. 
		    offset = 6 + 6 * numSynths + 8 * numCmds;
		    pSndBuffer = (XSoundHeader *) ((char *)pRes + offset);
		    *pEncode = pSndBuffer->encode & 0xFF;
		    break;
		case XSecondSoundFormat:
		    numSynths = 0;		// format 2 has none
		    numCmds = XGetShort(pSndFormat + 4);

				// compute address of sound header. 
		    offset = 6 + 6 * numSynths + 8 * numCmds;
		    pSndBuffer = (XSoundHeader *) ((char *)pRes + offset);
		    *pEncode = pSndBuffer->encode & 0xFF;
		    break;
		case XThirdSoundFormat:
		    numSynths = 0;		// format 3 has none
		    numCmds = 0;
		    pSndBuffer = (XSoundHeader *) ((char *)pRes + sizeof(short));
		    *pEncode = XType3Header;
		    break;
		default:
		    soundFormat = -1;
		    break;
		}
	}
    return pSndBuffer;
}


// Given a Mac sample, and loop points, this will change the data associated with it
#if USE_CREATION_API == TRUE
void XSetSoundLoopPoints(XPTR pRes, INT32 loopStart, INT32 loopEnd)
{
    register XSoundHeader		*pSndBuffer;
    register XCmpSoundHeader 	*pCmpBuffer;
    register XExtSoundHeader	*pExtBuffer;
    register XSoundHeader3		*pType3Buffer;
    short int					encode, count;

    pSndBuffer = (XSoundHeader *)PV_GetSoundHeaderPtr(pRes, &encode);
    if (pSndBuffer)	/* did we get the right format? */
	{
	    switch (encode)
		{
		case XStandardHeader:	// standard header
		    XPutLong(&pSndBuffer->loopStart, loopStart);
		    XPutLong(&pSndBuffer->loopEnd, loopEnd);
		    break;

		case XExtendedHeader:	// extended header
		    pExtBuffer = (XExtSoundHeader *)pSndBuffer;
		    XPutLong(&pExtBuffer->loopStart, loopStart);
		    XPutLong(&pExtBuffer->loopEnd, loopEnd);
		    break;
				
		case XCompressedHeader:	// compressed header
		    pCmpBuffer = (XCmpSoundHeader *)pSndBuffer;
		    XPutLong(&pCmpBuffer->loopStart, loopStart);
		    XPutLong(&pCmpBuffer->loopEnd, loopEnd);
		    break;
				
		case XType3Header:		// type 3 header
		    pType3Buffer = (XSoundHeader3 *)pSndBuffer;
		    for (count = 0; count < pType3Buffer->channels; count++)
			{
			    XPutLong(&pType3Buffer->loopStart[count], loopStart);
			    XPutLong(&pType3Buffer->loopEnd[count], loopEnd);
			}
		    break;
		}
	}
}

void XSetSoundEmbeddedStatus(XPTR pRes, XBOOL soundEmbedded)
{
    register XSoundHeader		*pSndBuffer;
    register XCmpSoundHeader 	*pCmpBuffer;
    register XExtSoundHeader	*pExtBuffer;
    register XSoundHeader3		*pType3Buffer;
    short int					encode;

    pSndBuffer = (XSoundHeader *)PV_GetSoundHeaderPtr(pRes, &encode);
    if (pSndBuffer)	/* did we get the right format? */
	{
	    switch (encode)
		{
		    //			case XStandardHeader:	// standard header
		    //				break;

		case XExtendedHeader:	// extended header
		    pExtBuffer = (XExtSoundHeader *)pSndBuffer;
		    pExtBuffer->soundIsEmbedded = soundEmbedded;
		    break;
				
		case XCompressedHeader:	// compressed header
		    pCmpBuffer = (XCmpSoundHeader *)pSndBuffer;
		    pCmpBuffer->soundIsEmbedded = soundEmbedded;
		    break;

		case XType3Header:		// type 3 header
		    pType3Buffer = (XSoundHeader3 *)pSndBuffer;
		    pType3Buffer->isEmbedded = soundEmbedded;
		    break;
		}
	}
}

XBOOL XGetSoundEmbeddedStatus(XPTR pRes)
{
    register XSoundHeader		*pSndBuffer;
    register XCmpSoundHeader 	*pCmpBuffer;
    register XExtSoundHeader	*pExtBuffer;
    register XSoundHeader3		*pType3Buffer;
    short int					encode;
    XBOOL						soundEmbedded;
	
    soundEmbedded = FALSE;
    pSndBuffer = (XSoundHeader *)PV_GetSoundHeaderPtr(pRes, &encode);
    if (pSndBuffer)	/* did we get the right format? */
	{
	    switch (encode)
		{
		    //			case XStandardHeader:	// standard header
		    //				break;

		case XExtendedHeader:	// extended header
		    pExtBuffer = (XExtSoundHeader *)pSndBuffer;
		    soundEmbedded = pExtBuffer->soundIsEmbedded;
		    break;
				
		case XCompressedHeader:	// compressed header
		    pCmpBuffer = (XCmpSoundHeader *)pSndBuffer;
		    soundEmbedded = pCmpBuffer->soundIsEmbedded;
		    break;

		case XType3Header:		// type 3 header
		    pType3Buffer = (XSoundHeader3 *)pSndBuffer;
		    soundEmbedded = pType3Buffer->isEmbedded;
		    break;
		}
	}
    return soundEmbedded;
}

// Given a Mac sample, and a sample rate, this will change the data associated with it
void XSetSoundSampleRate(XPTR pRes, XFIXED sampleRate)
{
    register XSoundHeader		*pSndBuffer;
    register XCmpSoundHeader 	*pCmpBuffer;
    register XExtSoundHeader	*pExtBuffer;
    register XSoundHeader3		*pType3Buffer;
    short int					encode;

    pSndBuffer = (XSoundHeader *)PV_GetSoundHeaderPtr(pRes, &encode);
    if (pSndBuffer)	/* did we get the right format? */
	{
	    switch (encode)
		{
		case XStandardHeader:	// standard header
		    XPutLong(&pSndBuffer->sampleRate, sampleRate);
		    break;

		case XExtendedHeader:	// extended header
		    pExtBuffer = (XExtSoundHeader *)pSndBuffer;
		    XPutLong(&pExtBuffer->sampleRate, sampleRate);
		    break;
				
		case XCompressedHeader:	// compressed header
		    pCmpBuffer = (XCmpSoundHeader *)pSndBuffer;
		    XPutLong(&pCmpBuffer->sampleRate, sampleRate);
		    break;

		case XType3Header:		// type 3 header
		    pType3Buffer = (XSoundHeader3 *)pSndBuffer;
		    XPutLong(&pType3Buffer->sampleRate, sampleRate);
		    break;
		}
	}
}

// Given a Mac sample, and a sample rate, this will change the data associated with it
void XSetSoundBaseKey(XPTR pRes, short int baseKey)
{
    register XSoundHeader		*pSndBuffer;
    register XCmpSoundHeader 	*pCmpBuffer;
    register XExtSoundHeader	*pExtBuffer;
    register XSoundHeader3		*pType3Buffer;
    short int					encode;

    pSndBuffer = (XSoundHeader *)PV_GetSoundHeaderPtr(pRes, &encode);
    if (pSndBuffer)	/* did we get the right format? */
	{
	    switch (encode)
		{
		case XStandardHeader:	// standard header
		    pSndBuffer->baseFrequency = (unsigned char)baseKey;
		    break;

		case XExtendedHeader:	// extended header
		    pExtBuffer = (XExtSoundHeader *)pSndBuffer;
		    pExtBuffer->baseFrequency = (unsigned char)baseKey;
		    break;
				
		case XCompressedHeader:	// compressed header
		    pCmpBuffer = (XCmpSoundHeader *)pSndBuffer;
		    pCmpBuffer->baseFrequency = (unsigned char)baseKey;
		    break;

		case XType3Header:		// type 3 header
		    pType3Buffer = (XSoundHeader3 *)pSndBuffer;
		    pType3Buffer->baseKey = (unsigned char)baseKey;
		    break;
		}
	}
}

short int XGetSoundBaseKey(XPTR pRes)
{
    register XSoundHeader		*pSndBuffer;
    register XCmpSoundHeader 	*pCmpBuffer;
    register XExtSoundHeader	*pExtBuffer;
    register XSoundHeader3		*pType3Buffer;
    short int					encode;
    short int					baseKey;

    baseKey = kMiddleC;
    pSndBuffer = (XSoundHeader *)PV_GetSoundHeaderPtr(pRes, &encode);
    if (pSndBuffer)	/* did we get the right format? */
	{
	    switch (encode)
		{
		case XStandardHeader:	// standard header
		    baseKey = pSndBuffer->baseFrequency;
		    break;

		case XExtendedHeader:	// extended header
		    pExtBuffer = (XExtSoundHeader *)pSndBuffer;
		    baseKey = pExtBuffer->baseFrequency;
		    break;
				
		case XCompressedHeader:	// compressed header
		    pCmpBuffer = (XCmpSoundHeader *)pSndBuffer;
		    baseKey = pCmpBuffer->baseFrequency;
		    break;

		case XType3Header:		// type 3 header
		    pType3Buffer = (XSoundHeader3 *)pSndBuffer;
		    baseKey = pType3Buffer->baseKey;
		    break;
		}
	}
    return baseKey;
}
#endif	// USE_CREATION_API == TRUE


// This will return a pointer into a snd2 pointer block or create a new block if compressed
XPTR XGetSamplePtrFromSnd(XPTR pRes, SampleDataInfo *pInfo)
{
    register XPTR				hSnd, newSound;
    register XSoundHeader		*pSndBuffer;
    register XCmpSoundHeader 	*pCmpBuffer;
    register XExtSoundHeader	*pExtBuffer;
    register XSoundHeader3		*pType3Buffer;
    INT32						offset;
    char						*pSampleData;
    short int					encode;
#if X_PLATFORM == X_MACINTOSH
    XPTR						monoSound;
    INT32						count;
    char						*pLeft, *pRight;
#endif

    pSampleData = NULL;
    hSnd = NULL;
    pInfo->size = 0;		// if left alone, then wrong type of resource
    pInfo->frames = 0;
    pInfo->rate = rate22khz;
    pInfo->loopStart = 0;
    pInfo->loopEnd = 0;
    pInfo->baseKey = kMiddleC;
    pInfo->bitSize = 8;
    pInfo->channels = 1;
    pInfo->compressionType = C_NONE;

    pSndBuffer = (XSoundHeader *)PV_GetSoundHeaderPtr(pRes, &encode);
    if (pSndBuffer)	/* did we get the right format? */
	{
	    /* compute address of sound header. 
	     */
	    switch (encode)
		{
		case XStandardHeader:	// standard header
		    pSampleData = (char *)&pSndBuffer->sampleArea[0];
		    pInfo->size = XGetLong(&pSndBuffer->length);
		    pInfo->frames = pInfo->size;
		    pInfo->loopStart = XGetLong(&pSndBuffer->loopStart);
		    pInfo->loopEnd = XGetLong(&pSndBuffer->loopEnd);
		    pInfo->baseKey = pSndBuffer->baseFrequency;
		    pInfo->rate = XGetLong(&pSndBuffer->sampleRate);
		    pInfo->channels = 1;
		    pInfo->bitSize = 8;			// defaults for standard headers
		    pInfo->pMasterPtr = (void *)pRes;
		    break;

		case XType3Header:
		    pType3Buffer = (XSoundHeader3 *)pSndBuffer;
		    pSampleData = (char *)&pType3Buffer->sampleArea[0];

		    pInfo->rate = XGetLong(&pType3Buffer->sampleRate);
		    pInfo->channels = pType3Buffer->channels;
		    pInfo->bitSize = pType3Buffer->bitSize;
		    pInfo->frames = XGetLong(&pType3Buffer->lengthInFrames);
		    pInfo->size = XGetLong(&pType3Buffer->lengthInBytes);
		    pInfo->loopStart = XGetLong(&pType3Buffer->loopStart[0]);
		    pInfo->loopEnd = XGetLong(&pType3Buffer->loopEnd[0]);
		    pInfo->baseKey = pType3Buffer->baseKey;

		    switch (XGetLong(&pType3Buffer->subType))
			{
			default:
			    pSampleData = NULL;	// bad type
			    break;
			case C_NONE:	// raw pcm audio
#if X_WORD_ORDER != FALSE
			    // if we're on a non motorola platform, swap sample data for 16 bit samples because its stored in a motorola way
			    if (pInfo->bitSize == 16)
				{
				    // if we're reading from a memory file, check to see if the samples are intel ordered
				    // if not, then swap them and set this flag. This will fail if we're working from
				    // ROM. If its ROM then we need to copy the data.
				    if (pType3Buffer->isSampleIntelOrder == FALSE)
					{
					    pType3Buffer->isSampleIntelOrder = TRUE;	// try and modify copy
					    // next time sample is read it will be right
					    if (pType3Buffer->isSampleIntelOrder)
						{	// Not ROM, must be a memory file, or a local copy
						    PV_Swap16BitSamples((short *)pSampleData, pInfo->frames, pInfo->channels);
						}
					    else
						{
						    // since we might be reading from ROM, we need to copy the data
						    // before we can swap it.
						    newSound = XNewPtr(pInfo->size);
						    pInfo->pMasterPtr = newSound;
						    if (newSound)
							{
							    XBlockMove(pSampleData, newSound, pInfo->size);
							    PV_Swap16BitSamples((short *)newSound, pInfo->frames, pInfo->channels);
							}
						    pSampleData = (char *)newSound;
						}
					}
				}
#endif
			    break;
			case C_IMA4:
			case C_ULAW:
			case C_ALAW:
			    break;
#if USE_MPEG_DECODER != 0
			    // Headspace MPEG
			case C_MPEG_32:
			case C_MPEG_40:
			case C_MPEG_48:
			case C_MPEG_56:
			case C_MPEG_64:
			case C_MPEG_80:
			case C_MPEG_96:
			case C_MPEG_112:
			case C_MPEG_128:
			case C_MPEG_160:
			case C_MPEG_192:
			case C_MPEG_224:
			case C_MPEG_256:
			case C_MPEG_320:
			    // XSoundHeader3 info is stored:
			    // lengthInBytes is the size of the compressed stream in bytes
			    // frameLengthInBytes is the size in bytes of a compressed frame
			    // lengthInFrames is the number of compressed frames
			    // so... lengthInFrames * frameLengthInBytes gets you the size in bytes of the uncompressed stream
			    {
				XDWORD	frames;					// number of compressed frames
				XDWORD	frameSize;				// size of compressed frame in bytes;
				XDWORD	lengthInBytes;			// size of uncompressed stream in bytes
				XDWORD	streamLengthInBytes;	// size of compressed stream in bytes
				XDWORD	pcmDataOffset;			// offset of compressed PCM audio data

				streamLengthInBytes = pInfo->size;
				frames = pInfo->frames;		// get compressed frames
				frameSize = XGetLong(&pType3Buffer->frameLengthInBytes);	// frameLengthInBytes is size of frame
				pcmDataOffset = XGetLong(&pType3Buffer->pcmDataOffset);

				lengthInBytes = (frames + 1) * frameSize;

				// get real playback size, ignoring extra decode size
				pInfo->bitSize = 16;
				pInfo->size = XGetLong(&pType3Buffer->lengthInBytesUncompressed);
				pInfo->frames = pInfo->size / (pInfo->channels) / (pInfo->bitSize / 8);

				newSound = XNewPtr(lengthInBytes);
				if (newSound)
				    {
					if (XExpandMPEG((XPTR)pSampleData, streamLengthInBytes, 
							newSound, lengthInBytes, pcmDataOffset) != NO_ERR)
					    {
						XDisposePtr(newSound);
						newSound = NULL;
					    }
				    }
				pInfo->pMasterPtr = newSound;
				pInfo->compressionType = XGetLong(&pType3Buffer->subType);
				pSampleData = (char *)newSound;
			    }
			    break;
#endif
			}
		    break;

		case XExtendedHeader:	// extended header
		    pExtBuffer = (XExtSoundHeader *)pSndBuffer;
		    pSampleData = (char *)&pExtBuffer->sampleArea[0];
		    pInfo->channels = (short)XGetLong(&pExtBuffer->numChannels);
		    pInfo->bitSize = XGetShort(&pExtBuffer->sampleSize);
		    pInfo->frames = XGetLong(&pExtBuffer->numFrames);
		    pInfo->size = pInfo->frames * (pInfo->channels) * (pInfo->bitSize / 8);
		    pInfo->loopStart = XGetLong(&pExtBuffer->loopStart);
		    pInfo->loopEnd = XGetLong(&pExtBuffer->loopEnd);
		    pInfo->baseKey = pExtBuffer->baseFrequency;
		    pInfo->rate = XGetLong(&pExtBuffer->sampleRate);

#if X_WORD_ORDER != FALSE
				// if we're on a non motorola platform, swap sample data for 16 bit samples because its stored in a motorola way
		    if (pInfo->bitSize == 16)
			{
			    // if we're reading from a memory file, check to see if the samples are intel ordered
			    // if not, then swap them and set this flag. This will fail if we're working from
			    // ROM. If its ROM then we need to copy the data.
			    if (pExtBuffer->sampleIsIntelOrder == FALSE)
				{
				    pExtBuffer->sampleIsIntelOrder = TRUE;	// try and modify copy
				    // next time sample is read it will be right
				    if (pExtBuffer->sampleIsIntelOrder)
					{	// Not ROM, must be a memory file, or a local copy
					    PV_Swap16BitSamples((short *)pSampleData, pInfo->frames, pInfo->channels);
					}
				    else
					{
					    // since we might be reading from ROM, we need to copy the data
					    // before we can swap it.
					    newSound = XNewPtr(pInfo->size);
					    pInfo->pMasterPtr = newSound;
					    if (newSound)
						{
						    XBlockMove(pSampleData, newSound, pInfo->size);
						    PV_Swap16BitSamples((short *)newSound, pInfo->frames, pInfo->channels);
						}
					    pSampleData = (char *)newSound;
					}
				}
			}
#endif
		    pInfo->pMasterPtr = (void *)pRes;
		    break;
				
		case XCompressedHeader:	// compressed header
		    pCmpBuffer = (XCmpSoundHeader *)pSndBuffer;
				
				// $$kk: 04.02.99: fix for P2 bug #4225976, RMF and MIDI crash w/ jdk1.2.2-M  
				// change from steve hales
				
				// $$fb: 2002-02-01: this is not 64-bit safe. Needs more investigation
				// as for now, issue a warning to stdout.
#ifdef _WIN64
		    if (XGetLong(&pCmpBuffer->samplePtr) != 0) {
			DEBUG_STR("ITANIUM problem ! Please investigate in SampleTools.c:621 .\n");
			pSampleData = NULL;
			break;
		    }
#else
#ifdef _LP64
		    if (XGetLong(&pCmpBuffer->samplePtr) != 0) {
			DEBUG_STR("ITANIUM problem ! Please investigate in SampleTools.c:621 .\n");
			pSampleData = NULL;
			break;
		    }
#else
				/* NOT a 64-bit architecture */
		    pSampleData = (void *)XGetLong(&pCmpBuffer->samplePtr);
#ifdef USE_DEBUG
		    if (pSampleData != NULL) {
			fprintf(stderr, "ITANIUM investigation in SampleTools.c:621: pSampleData = %p \n", pSampleData);
		    }
#endif
#endif
#endif

		    if (pSampleData == NULL)	/* get ptr to sample data */
			{
			    pSampleData = (char *) pCmpBuffer->sampleArea;
			}
		    pInfo->channels = (short)XGetLong(&pCmpBuffer->numChannels);
		    pInfo->bitSize = XGetShort(&pCmpBuffer->sampleSize);
		    pInfo->frames = XGetLong(&pCmpBuffer->numFrames);
		    pInfo->loopStart = XGetLong(&pCmpBuffer->loopStart);
		    pInfo->loopEnd = XGetLong(&pCmpBuffer->loopEnd);
		    pInfo->baseKey = pCmpBuffer->baseFrequency;
		    pInfo->rate = XGetLong(&pCmpBuffer->sampleRate);

		    encode = XGetShort(&pCmpBuffer->compressionID);
		    switch(encode)
			{
			default:
			    DEBUG_STR("\nInvalid compression ID");
			    break;
	
			case fixedCompression:
			    offset = XGetLong(&pCmpBuffer->format);
			    switch (offset)
				{
				case C_IMA4:	// IMA 4 : 1
				    offset = pInfo->frames * AIFF_IMA_BLOCK_FRAMES;	// Apple Sound Manager defines 64 samples per packet
				    pInfo->frames = offset;							// number of sample frames
				    pInfo->size = offset * pInfo->channels * 2;		// assume 16 bit
				    if ((pCmpBuffer->forceSample8bit & 0x80) == 0)
					{
					    // do a 16 bit decompression. As we decompress the IMA we build a 16 bit sample
					    pInfo->bitSize = 16;			// must change to final output size
					}
				    else
					{
					    // do a 8 bit decompression. As we decompress the IMA we build a 8 bit sample
					    pInfo->size /= 2;
					    pInfo->bitSize = 8;			// must change to final output size
					}
				    newSound = XNewPtr(pInfo->size);
				    pInfo->pMasterPtr = newSound;
				    if (newSound)
					{
					    XExpandAiffIma((XBYTE const*)pSampleData, AIFF_IMA_BLOCK_BYTES,
							   newSound, pInfo->bitSize,
							   pInfo->frames,pInfo->channels);
					}
				    pInfo->compressionType = C_IMA4;
				    pSampleData = (char *)newSound;
				    break;
				case C_ALAW:	// alaw 2 : 1
				    pInfo->bitSize = 16;								// must change, its stored as 8 bit
				    pInfo->size = pInfo->frames * pInfo->channels * 2;	// always 16 bit
				    newSound = XNewPtr(pInfo->size);
				    pInfo->pMasterPtr = newSound;
				    if (newSound)
					{
					    XExpandALawto16BitLinear(	(unsigned char *)pSampleData, 
									(short int *)newSound, 
									pInfo->frames, pInfo->channels);
					}
				    pInfo->compressionType = C_ALAW;
				    pSampleData = (char *)newSound;
				    break;
				case C_ULAW:	// ulaw 2 : 1
				    pInfo->bitSize = 16;								// must change, its stored as 8 bit
				    pInfo->size = pInfo->frames * pInfo->channels * 2;	// always 16 bit
				    newSound = XNewPtr(pInfo->size);
				    pInfo->pMasterPtr = newSound;
				    if (newSound)
					{
					    XExpandULawto16BitLinear(	(unsigned char *)pSampleData, 
									(short int *)newSound, 
									pInfo->frames, pInfo->channels);
					}
				    pInfo->compressionType = C_ULAW;
				    pSampleData = (char *)newSound;
				    break;
				}
			    break;
			case threeToOne:
#if X_PLATFORM != X_MACINTOSH
			    newSound = NULL;	// don't support this format
#else
			    pInfo->size = pInfo->frames * (pInfo->channels) * (pInfo->bitSize / 8);
			    pInfo->size *= 6;	// 2 bytes at 3:1 is 6 bytes for a packet, 1 byte at 6:1 is 6 bytes too
			    newSound = XNewPtr(pInfo->size);
			    pInfo->pMasterPtr = newSound;
			    if (newSound)
				{
				    if (pInfo->channels == 1)
					{
					    XExpandMace1to3(pSampleData, newSound, pInfo->frames, NULL, NULL, 1, 1);
					    pInfo->frames *= 6;			// adjust the frame count to equal the real frames
					}
				    else
					{
					    monoSound = XNewPtr(pInfo->size / 2);
					    if (monoSound)
						{
						    XExpandMace1to3(pSampleData, newSound, pInfo->frames, NULL, NULL, pInfo->channels, 1);
						    XExpandMace1to3(pSampleData, monoSound, pInfo->frames, NULL, NULL, pInfo->channels, 2);
						    pLeft = (char *)newSound;
						    pRight = (char *)monoSound;
						    pInfo->frames *= 6;			// adjust the frame count to equal the real frames
						    offset = pInfo->frames - 1;
						    // copy the data into a stereo sample block, copy backwards so that we don't have to create
						    // two blocks of data
						    for (count = offset; count >= 0; count--)
							{
							    pLeft[count*2+0] = pLeft[count];
							    pLeft[count*2+1] = pRight[count];
							}
						    XDisposePtr(monoSound);
						}
					}
				}
#endif
			    pInfo->compressionType = C_MACE3;
			    pSampleData = (char *)newSound;
			    break;
			case sixToOne:
#if X_PLATFORM != X_MACINTOSH
			    newSound = NULL;	// don't support this format
#else
			    pInfo->size = pInfo->frames * (pInfo->channels) * (pInfo->bitSize / 8);
			    pInfo->size *= 6;	// 2 bytes at 3:1 is 6 bytes for a packet, 1 byte at 6:1 is 6 bytes too
			    newSound = XNewPtr(pInfo->size);
			    pInfo->pMasterPtr = newSound;
			    if (newSound)
				{
				    if (pInfo->channels == 1)
					{
					    XExpandMace1to6(pSampleData, newSound, pInfo->frames, NULL, NULL, 1, 1);
					    pInfo->frames *= 6;			// adjust the frame count to equal the real frames
					}
				    else
					{
					    monoSound = XNewPtr(pInfo->size / 2);
					    if (monoSound)
						{
						    XExpandMace1to6(pSampleData, newSound, pInfo->frames, NULL, NULL, pInfo->channels, 1);
						    XExpandMace1to6(pSampleData, monoSound, pInfo->frames, NULL, NULL, pInfo->channels, 2);
						    pLeft = (char *)newSound;
						    pRight = (char *)monoSound;
						    pInfo->frames *= 6;			// adjust the frame count to equal the real frames
						    offset = pInfo->frames - 1;
						    // copy the data into a stereo sample block, copy backwards so that we don't have to create
						    // two blocks of data
						    for (count = offset; count >= 0; count--)
							{
							    pLeft[count*2+0] = pLeft[count];
							    pLeft[count*2+1] = pRight[count];
							}
						    XDisposePtr(monoSound);
						}
					}
				}
#endif
			    pInfo->compressionType = C_MACE6;
			    pSampleData = (char *)newSound;
			    break;
			}
		    break;
		}
	}
    // verify loops
    {
	XBOOL	pass;
	
	pass = TRUE;
	if ((INT32)pInfo->loopStart < 0)
	    {
		pass = FALSE;
	    }
	if (pInfo->loopStart > pInfo->loopEnd)
	    {
		pass = FALSE;
	    }
	if (pInfo->loopEnd > pInfo->frames)
	    {
		pass = FALSE;
	    }
	if (pass == FALSE)
	    {
		pInfo->loopStart = 0;
		pInfo->loopEnd = 0;
	    }
    }
    return pSampleData;
}

// Given a sample ID, this will search through sample types and return a 'C' string
// of the resource name of the currently open resource files
#if USE_CREATION_API == TRUE
XBOOL XGetSampleNameFromID(XLongResourceID sampleSoundID, char *cName)
{
    static XResourceType	sampleType[] = {ID_CSND, ID_ESND, ID_SND};
    short int				count;
    XBOOL					bad;

    bad = FALSE;
    if (cName)
	{
	    for (count = 0; count < (short int)(sizeof(sampleType) / sizeof(INT32)); count++)
		{
		    cName[0] = 0;
		    XGetResourceName(sampleType[count], sampleSoundID, cName);
		    if (cName[0])
			{
			    break;
			}
		}
	    if (cName[0] == 0)
		{
		    bad = TRUE;
		}
	}
    return bad;
}
#endif

#if USE_CREATION_API == TRUE
// given 8 bit data, convert this to 16 bit data
static XWORD * XConvert8BitTo16Bit(XBYTE * p8BitPCMData, UINT32 frames, UINT32 channels)
{
    XWORD			*newData;
    UINT32	count, ccount;
    short int		sample;

    newData = (XWORD *)XNewPtr(frames * channels * (UINT32)sizeof(short));
    if (newData)
	{
	    for (count = 0; count < frames; count++)
		{
		    for (ccount = 0; ccount < channels; ccount++)
			{
			    sample = (p8BitPCMData[count + ccount] - 128) << 8;
			    sample += XRandomRange(3);		// add a little noise. This is a trick of the ear. This doesn't make
			    // sense, but the brain filters the noise and the 8 bit conversion sounds
			    // less fuzzy.
			    newData[count + ccount] = sample;
			}
		}
							
	}
    return newData;
}
#endif

#if USE_CREATION_API == TRUE
#if USE_MPEG_DECODER != 0
static XFIXED PV_GetClosestMPEGSampleRate(XFIXED sourceRate)
{
    static XFIXED	mpeg1Rates[] = {UNSIGNED_LONG_TO_XFIXED(32000), UNSIGNED_LONG_TO_XFIXED(44100), UNSIGNED_LONG_TO_XFIXED(48000)};
    XFIXED			targetRate;
    int				count;

    targetRate = UNSIGNED_LONG_TO_XFIXED(32000);
    for (count = 0; count < 3; count++)
	{
	    if (sourceRate >= mpeg1Rates[count])
		{
		    targetRate = mpeg1Rates[count];
		}
	}
    return targetRate;
}
#endif
#endif

// Given a structure filled with no compression, create a type 1 sound object.
// Will return NULL if out of memory, failed to get the right info from the
// SampleDataInfo structure, and if pPCMData is NULL.
// pSampleInfo->pMasterPtr is ignored, pPCMData points to the PCM data formated
// the way pSampleInfo describes
#if USE_CREATION_API == TRUE
XPTR XCreateSoundObjectFromData(XPTR pPCMData, SampleDataInfo *pSampleInfo, SndCompressionType type, XCompressStatusProc pProc)
{
    XPTR			pNewSampleObject;
    XSoundFormat1*	snd;

#if USE_MPEG_DECODER == 0
    pProc;
#endif

    pNewSampleObject = NULL;
    if (!pPCMData ||
	!pSampleInfo ||
	(pSampleInfo->size == 0)) return NULL;

    switch (type)
	{
	default:
	case C_MACE3:
	case C_MACE6:
	    return NULL;
	case C_NONE:
	    // raw PCM audio data
	    {
		XExtSndHeader1*	pSnd;

		pNewSampleObject = XNewPtr((INT32)sizeof(XExtSndHeader1) + pSampleInfo->size);
		if (!pNewSampleObject) return NULL;

		pSnd = (XExtSndHeader1*)pNewSampleObject;
		snd = &pSnd->sndHeader;
				
		XPutLong(&pSnd->sndBuffer.samplePtr, 0L);
		XPutLong(&pSnd->sndBuffer.numChannels, pSampleInfo->channels);
		XPutShort(&pSnd->sndBuffer.sampleSize, pSampleInfo->bitSize);
		XPutLong(&pSnd->sndBuffer.sampleRate, pSampleInfo->rate);
		XConvertToIeeeExtended(pSampleInfo->rate, (unsigned char *)pSnd->sndBuffer.AIFFSampleRate);
		XPutLong(&pSnd->sndBuffer.loopStart, pSampleInfo->loopStart);
		XPutLong(&pSnd->sndBuffer.loopEnd, pSampleInfo->loopEnd);
		pSnd->sndBuffer.encode = XExtendedHeader;		// extended header
		pSnd->sndBuffer.baseFrequency = (unsigned char)pSampleInfo->baseKey;
		XPutLong(&pSnd->sndBuffer.numFrames, pSampleInfo->frames);
		XBlockMove(pPCMData, &pSnd->sndBuffer.sampleArea, pSampleInfo->size);
#if X_WORD_ORDER != FALSE
		if (pSampleInfo->bitSize == 16)
		    {
			PV_Swap16BitSamples((short *)&pSnd->sndBuffer.sampleArea, pSampleInfo->frames, pSampleInfo->channels);
		    }
#endif
	    }
	    break;

#if USE_MPEG_DECODER != 0
	    // Headspace MPEG
	case C_MPEG_32:
	case C_MPEG_40:
	case C_MPEG_48:
	case C_MPEG_56:
	case C_MPEG_64:
	case C_MPEG_80:
	case C_MPEG_96:
	case C_MPEG_112:
	case C_MPEG_128:
	case C_MPEG_160:
	case C_MPEG_192:
	case C_MPEG_224:
	case C_MPEG_256:
	case C_MPEG_320:
	    {
		GM_Waveform		wave;
		XPTR			mpegStream;
		UINT32	streamSize, frameSizeInBytes, maxFrames;
		XSndHeader3		*mpg_snd;
		XPTR			savePtr;
		XFIXED			saveSampleRate;
		short int		saveBitSize;
		UINT32	saveSize;
		XDWORD			pcmDataOffset;
		XWORD			*pcm16Bit;
		XMPEGEncodeRate	encodeRate;

		pcm16Bit = NULL;
		savePtr = pSampleInfo->pMasterPtr;
		saveBitSize = pSampleInfo->bitSize;
		saveSize = pSampleInfo->size;
		pSampleInfo->pMasterPtr = pPCMData;

		if (pSampleInfo->bitSize != 16)
		    {
			// convert to 16 bit data first, then compress
			pcm16Bit = (XWORD *)XConvert8BitTo16Bit((XBYTE *)pPCMData, 
								(UINT32)pSampleInfo->frames, 
								(UINT32)pSampleInfo->channels);
			if (pcm16Bit == NULL)
			    {
				return NULL;
			    }
			pSampleInfo->pMasterPtr = pcm16Bit;
			pSampleInfo->bitSize = 16;
			pSampleInfo->size *= 2;
		    }

		XTranslateFromSampleDataToWaveform(pSampleInfo, &wave);
		pSampleInfo->bitSize = saveBitSize;
		pSampleInfo->pMasterPtr = savePtr;
		pSampleInfo->size = saveSize;
		saveSampleRate = wave.sampledRate;

				// now find the closest rate to the mpeg source sample rates				
		wave.sampledRate = PV_GetClosestMPEGSampleRate(saveSampleRate);

		switch (type)	// Headspace MPEG
		    {
		    case C_MPEG_32:
			encodeRate = MPG_32;
			break;
		    case C_MPEG_40:
			encodeRate = MPG_40;
			break;
		    case C_MPEG_48:
			encodeRate = MPG_48;
			break;
		    case C_MPEG_56:
			encodeRate = MPG_56;
			break;
		    case C_MPEG_64:
			encodeRate = MPG_64;
			break;
		    case C_MPEG_80:
			encodeRate = MPG_80;
			break;
		    case C_MPEG_96:
			encodeRate = MPG_96;
			break;
		    case C_MPEG_112:
			encodeRate = MPG_112;
			break;
		    case C_MPEG_128:
			encodeRate = MPG_128;
			break;
		    case C_MPEG_160:
			encodeRate = MPG_160;
			break;
		    case C_MPEG_192:
			encodeRate = MPG_192;
			break;
		    case C_MPEG_224:
			encodeRate = MPG_224;
			break;
		    case C_MPEG_256:
			encodeRate = MPG_256;
			break;
		    case C_MPEG_320:
			encodeRate = MPG_320;
			break;
		    }
		mpegStream = XCompressMPEG(&wave, encodeRate, pProc, &streamSize, &maxFrames, &frameSizeInBytes, &pcmDataOffset);
		XDisposePtr((XPTR)pcm16Bit);
		if (mpegStream)
		    {
			wave.sampledRate = saveSampleRate;
			mpg_snd = (XSndHeader3 *)XNewPtr((INT32)sizeof(XSndHeader3) + streamSize);
			if (mpg_snd)
			    {
				XPutShort(&mpg_snd->type, XThirdSoundFormat);
				XPutLong(&mpg_snd->sndBuffer.subType, type);
				XPutLong(&mpg_snd->sndBuffer.sampleRate, wave.sampledRate);
				XPutLong(&mpg_snd->sndBuffer.frameLengthInBytes, frameSizeInBytes);
				XPutLong(&mpg_snd->sndBuffer.lengthInFrames, maxFrames);
				XPutLong(&mpg_snd->sndBuffer.lengthInBytes, streamSize);
				XPutLong(&mpg_snd->sndBuffer.pcmDataOffset, pcmDataOffset);
				XPutLong(&mpg_snd->sndBuffer.lengthInBytesUncompressed, wave.waveSize);
				XPutLong(&mpg_snd->sndBuffer.loopStart[0], wave.startLoop);
				XPutLong(&mpg_snd->sndBuffer.loopEnd[0], wave.endLoop);
				mpg_snd->sndBuffer.baseKey = (XBYTE)wave.baseMidiPitch;
				mpg_snd->sndBuffer.channels = (XBYTE)wave.channels;
				mpg_snd->sndBuffer.bitSize = (XBYTE)wave.bitSize;
				XBlockMove(mpegStream, mpg_snd->sndBuffer.sampleArea, streamSize);
				XDisposePtr(mpegStream);
				return (XPTR)mpg_snd;
			    }
			else
			    {
				XDisposePtr(mpegStream);
				return NULL;
			    }
		    }
		else
		    {
			return NULL;
		    }
	    }
	    break;
#endif
	case C_IMA4:
	    // IMA compressed audio data
	    {
		XDWORD			imaBlocks;
		XCmpSndHeader1*	pCSnd;
				
		imaBlocks = (pSampleInfo->frames + AIFF_IMA_BLOCK_FRAMES - 1) /
		    AIFF_IMA_BLOCK_FRAMES;
		pNewSampleObject = XNewPtr((INT32)sizeof(XCmpSndHeader1) +
					   imaBlocks * pSampleInfo->channels * AIFF_IMA_BLOCK_BYTES);
		if (!pNewSampleObject) return NULL;

		pCSnd = (XCmpSndHeader1*)pNewSampleObject;
		snd = &pCSnd->sndHeader;

		XPutLong(&pCSnd->sndBuffer.samplePtr, 0L);
		XPutLong(&pCSnd->sndBuffer.numChannels, pSampleInfo->channels);
		XPutShort(&pCSnd->sndBuffer.sampleSize, 2);
		if (pSampleInfo->bitSize == 8)
		    {
			pCSnd->sndBuffer.forceSample8bit |= 0x80;
		    }

		XPutLong(&pCSnd->sndBuffer.sampleRate, pSampleInfo->rate);
		XConvertToIeeeExtended(pSampleInfo->rate, (unsigned char *)pCSnd->sndBuffer.AIFFSampleRate);

		XPutLong(&pCSnd->sndBuffer.loopStart, pSampleInfo->loopStart);
		XPutLong(&pCSnd->sndBuffer.loopEnd, pSampleInfo->loopEnd);

		pCSnd->sndBuffer.encode = XCompressedHeader;		// compressed header
		XPutShort(&pCSnd->sndBuffer.compressionID, (unsigned short)fixedCompression);
		XPutLong(&pCSnd->sndBuffer.format, C_IMA4);
		pCSnd->sndBuffer.baseFrequency = (unsigned char)pSampleInfo->baseKey;

		XPutLong(&pCSnd->sndBuffer.numFrames, imaBlocks);
		XPutShort(&pCSnd->sndBuffer.packetSize, 0);
		XPutShort(&pCSnd->sndBuffer.snthID, 0);

				// compress in place
		XCompressAiffIma(pPCMData, pSampleInfo->bitSize, pCSnd->sndBuffer.sampleArea,
				 pSampleInfo->frames, pSampleInfo->channels);
	    }
	    break;
	case C_ULAW:
	case C_ALAW:
	    // ALAW & ULAW compressed audio data
	    {
		XDWORD			lawBlocks;
		XCmpSndHeader1	*pCSnd;
		XWORD			*pcm16Bit;
				
		pcm16Bit = NULL;
		if (pSampleInfo->bitSize != 16)
		    {
			// convert to 16 bit data first, then compress
			pcm16Bit = (XWORD *)XConvert8BitTo16Bit((XBYTE *)pPCMData, 
								(UINT32)pSampleInfo->frames, 
								(UINT32)pSampleInfo->channels);
			pPCMData = pcm16Bit;
		    }
		lawBlocks = pSampleInfo->frames / 2;

		pNewSampleObject = XNewPtr((INT32)sizeof(XCmpSndHeader1) +
					   lawBlocks * pSampleInfo->channels * (INT32)sizeof(short));
		if (pNewSampleObject == NULL)
		    {
			XDisposePtr((XPTR)pcm16Bit);
			return NULL;
		    }

		pCSnd = (XCmpSndHeader1*)pNewSampleObject;
		snd = &pCSnd->sndHeader;

		XPutLong(&pCSnd->sndBuffer.samplePtr, 0L);
		XPutLong(&pCSnd->sndBuffer.numChannels, pSampleInfo->channels);
		XPutShort(&pCSnd->sndBuffer.sampleSize, 2);

		XPutLong(&pCSnd->sndBuffer.sampleRate, pSampleInfo->rate);
		XConvertToIeeeExtended(pSampleInfo->rate, (unsigned char *)pCSnd->sndBuffer.AIFFSampleRate);

		XPutLong(&pCSnd->sndBuffer.loopStart, pSampleInfo->loopStart);
		XPutLong(&pCSnd->sndBuffer.loopEnd, pSampleInfo->loopEnd);

		pCSnd->sndBuffer.encode = XCompressedHeader;		// compressed header
		XPutShort(&pCSnd->sndBuffer.compressionID, (unsigned short)fixedCompression);
		XPutLong(&pCSnd->sndBuffer.format, type);
		pCSnd->sndBuffer.baseFrequency = (unsigned char)pSampleInfo->baseKey;

		XPutLong(&pCSnd->sndBuffer.numFrames, pSampleInfo->frames);
		XPutShort(&pCSnd->sndBuffer.packetSize, 0);
		XPutShort(&pCSnd->sndBuffer.snthID, 0);

				// compress in place
		XCompressLaw((SndCompressionType)type, (short int *)pPCMData, (char *)pCSnd->sndBuffer.sampleArea,
			     pSampleInfo->frames, pSampleInfo->channels);
		XDisposePtr((XPTR)pcm16Bit);
	    }
	    break;
	}	// switch

    XPutShort(&snd->type, XFirstSoundFormat);
    XPutShort(&snd->numModifiers, 1);
    XPutShort(&snd->modNumber, 5);
    XPutLong(&snd->modInit, 224);
    XPutShort(&snd->numCommands, 1);
    XPutShort(&snd->cmd, 0x8000 + bufferCmd);
    XPutShort(&snd->param1, 0);
    XPutLong(&snd->param2, (INT32)sizeof(XSoundFormat1));

    return pNewSampleObject;
}
#endif	// USE_CREATION_API == TRUE


// Given a structure filled with no compression, create a AudioResource object.
// Will return NULL if out of memory, failed to get the right info from the
// SampleDataInfo structure, and if pPCMData is NULL.
// pSampleInfo->pMasterPtr is ignored, pPCMData points to the PCM data formated
// the way pSampleInfo describes
#if USE_CREATION_API == TRUE
static AudioResource *XCreateAudioObjectFromData(XPTR pPCMData, SampleDataInfo *pSampleInfo)
{

    AudioResource	*pNewSampleObject;

    pNewSampleObject = NULL;
    if (pSampleInfo)
	{
	    if (pPCMData && pSampleInfo->size)
		{
		    pNewSampleObject = (AudioResource *)XNewPtr((INT32)sizeof(AudioResource) + pSampleInfo->size);
		    if (pNewSampleObject)
			{
			    XPutLong(&pNewSampleObject->version, AUDIO_OBJECT_VERSION);
			    XPutLong(&pNewSampleObject->dataLength, pSampleInfo->size);
			    XPutLong(&pNewSampleObject->sampleFrames, pSampleInfo->frames);
			    XPutLong(&pNewSampleObject->dataOffset, (INT32)sizeof(AudioResource));
			    XPutLong(&pNewSampleObject->audioType, AUDIO_SND);
			    XPutLong(&pNewSampleObject->nameResourceType, ID_NULL);
			    XPutLong(&pNewSampleObject->sampleRate, pSampleInfo->rate);
			    XPutLong(&pNewSampleObject->loopStart, pSampleInfo->loopStart);
			    XPutLong(&pNewSampleObject->loopEnd, pSampleInfo->loopEnd);
			    XPutShort(&pNewSampleObject->baseMidiKey, pSampleInfo->baseKey);
			    XPutShort(&pNewSampleObject->bitSize, pSampleInfo->channels);
			    XPutShort(&pNewSampleObject->channels, pSampleInfo->bitSize);
			    XBlockMove(pPCMData, (char *)(&pNewSampleObject->firstSampleFiller) + sizeof(INT32),
				       pSampleInfo->size);
			}
		}
	}
    return pNewSampleObject;
}
#endif	// USE_CREATION_API == TRUE


#if USE_CREATION_API == TRUE
static XPTR XGetAudioObjectFromMemory(AudioResource *pAudioObject, SampleDataInfo *pInfo)
{
    XPTR				pSampleData;
    UINT32		size;

    pSampleData = NULL;
    pInfo->size = 0;		// if left alone, then wrong type of resource
    pInfo->frames = 0;
    pInfo->rate = rate22khz;
    pInfo->loopStart = 0;
    pInfo->loopEnd = 0;
    pInfo->baseKey = kMiddleC;
    pInfo->bitSize = 8;
    pInfo->channels = 1;
    pInfo->compressionType = C_NONE;
    if (pAudioObject)
	{
	    if (XGetLong(&pAudioObject->version) == AUDIO_OBJECT_VERSION)
		{
		    switch (XGetLong(&pAudioObject->audioType))
			{
			case AUDIO_SND:
			    size = XGetLong(&pAudioObject->dataLength);
			    if (size)
				{
				    pInfo->size = size;
				    pInfo->frames = XGetLong(&pAudioObject->sampleFrames);
				    pInfo->rate = XGetLong(&pAudioObject->sampleRate);
				    pInfo->loopStart = XGetLong(&pAudioObject->loopStart);
				    pInfo->loopEnd = XGetLong(&pAudioObject->loopEnd);
				    pInfo->baseKey = XGetShort(&pAudioObject->baseMidiKey);
				    pInfo->bitSize = XGetShort(&pAudioObject->bitSize);
				    pInfo->channels = XGetShort(&pAudioObject->channels);
				    pSampleData = XGetSamplePtrFromSnd((XPTR)((char *)(&pAudioObject->firstSampleFiller) + sizeof(INT32)), pInfo);
				}
			    break;
			}
		}
	}
    return (XPTR)pSampleData;
}
#endif	// USE_CREATION_API == TRUE

// Translate a GM_Waveform structure into a SampleDataInfo structure
#if USE_CREATION_API == TRUE
void XTranslateFromWaveformToSampleData(GM_Waveform *pSource, SampleDataInfo *pDest)
{
    if (pSource && pDest)
	{
	    pDest->rate = pSource->sampledRate;
	    pDest->frames = pSource->waveFrames;
	    pDest->size = pSource->waveSize;
	    pDest->loopStart = pSource->startLoop;
	    pDest->loopEnd = pSource->endLoop;
	    pDest->bitSize = pSource->bitSize;
	    pDest->channels = pSource->channels;
	    pDest->baseKey = pSource->baseMidiPitch;
	    pDest->theID = (short)pSource->waveformID;
	    pDest->compressionType = ID_NULL;
	    pDest->pMasterPtr = pSource->theWaveform;
	}
}
#endif

// Translate a SampleDataInfo structure into a GM_Waveform structure
#if USE_CREATION_API == TRUE
void XTranslateFromSampleDataToWaveform(SampleDataInfo *pSource, GM_Waveform *pDest)
{
    if (pSource && pDest)
	{
	    pDest->sampledRate = pSource->rate;
	    pDest->waveFrames = pSource->frames;
	    pDest->waveSize = pSource->size;
	    pDest->startLoop = pSource->loopStart;
	    pDest->endLoop = pSource->loopEnd;
	    pDest->bitSize = (XBYTE)pSource->bitSize;
	    pDest->channels = (XBYTE)pSource->channels;
	    pDest->baseMidiPitch = (XBYTE)pSource->baseKey;
	    pDest->waveformID = pSource->theID;
	    pDest->theWaveform = (SBYTE *)pSource->pMasterPtr;
	}
}
#endif

/* EOF of SampleTools.c
 */
