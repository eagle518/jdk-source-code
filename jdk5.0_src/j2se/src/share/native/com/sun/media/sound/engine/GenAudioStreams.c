/*
 * @(#)GenAudioStreams.c	1.75 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/***************************************************************************/
/*
**	"GenAudioStreams.c"
**
**	This implements multi source audio streaming code.
**
**	History	-
**	10/5/96		Created
**	11/8/96		Added GM_AudioStreamGetStereoPosition
**	12/30/96	Changed copyright
**	1/24/97		Added fade API & GM_SetAudioStreamFadeRate
**				Changed GM_AudioStreamSetVolume to return 0
**				when stream is dead
**	3/25/97		Fixed a bug when servicing streams and it gets released
**				Removed hard coded constants for stream mode and replaced
**				with way spiffy defines
**	4/24/97		Fixed bug with STREAM_DESTROY that caused recursive callbacks
**	5/3/97		Fixed a few potential problems that the MOT compiler found
**	5/15/97		Reworked file streaming to support compressed streams
**	5/20/97		Fixed a memory leak with GM_AudioStreamFileStart
**	5/21/97		Renamed GM_AudioStreamStart to GM_AudioStreamSetup
**				Seperated load and starting of a stream into two seperate functions:
**				now you call GM_AudioStreamFileSetup OR GM_AudioStreamSetup then
**				call GM_AudioStreamStart
**	6/3/97		Added GM_AudioStreamGetReference
**				Modified GM_AudioStreamStop to not STREAM_DESTROY imeadiately. Instead
**				it calls it at the next GM_AudioServiceStream. This makes sure that
**				the right thread context is setup for the deallocation of memory
**	6/4/97		Changed GM_AudioStreamFileSetup & GM_AudioStreamSetup & 
**				GM_AudioStreamGetData & GM_AudioStreamService to pass in a thread
**				context
**	6/16/97		Added GM_AudioStreamSetVolumeAll
**	6/25/97		Removed extra commented out code in GM_AudioStreamSetup
**				Redid setup and handling of small buffer sizes when getting the first and
**				second buffer. Fixed a bug in which the buffer would stop because the first
**				buffer get was smaller than the actual buffer allocated.
**	6/26/97		Modified PV_StartThisBufferPlaying to not shutdown the buffer if the oppisite
**				buffer length equals zero.
**				Fixed a bug with PV_FileStreamCallback that included extra data after
**				the sample end of file, so it caused playback noise.
**	7/16/97		Incorporated changes from David
**	7/22/97		Changed GM_AudioStreamSetVolumeAll to accecpt -1 as a volume change
**				to reset all volumes and recalculate all volumes
**	8/18/97		Added GM_AudioStreamFlush & GM_AudioStreamGetSampleOffset
**				Eliminated the 10k limit in GM_AudioStreamSetup to new streams buffer sizes.
**				This will have the effect of skipping if the buffer is too small or there is
**				not enough CPU bandwidth to process samples.
**	9/15/97		Added GM_AudioStreamGetReverb
**	9/25/97		Added some underflow condition code to stream engine
**	9/30/97		Eliminated lastAudioStreamErr, and changed GM_AudioStreamError to take
**				a stream reference to get the last error
**	10/13/97	Added new code to handle the underflow condition for the first two buffers
**				of GM_AudioStreamSetup. In that case its handle inside of GM_AudioStreamService.
**	10/16/97	Changed GM_AudioStreamError to return NO_ERR instead of NOT_SETUP if the stream
**				is invalid.
**				Cleaned out old commented out code from PV_AudioBufferCallback
**	11/10/97	Changed some preprocessor tests and flags to explicity test for flags rather
**				than assume
**	11/17/97	Added GM_AudioStreamGetSamplesPlayed & GM_AudioStreamUpdateSamplesPlayed from Kara
**	11/18/97	Fixed more underflow problems
**				Modified GM_AudioStreamPause & GM_AudioStreamResume to work with a particular
**				stream
**				Added GM_AudioStreamResumeAll & GM_AudioStreamPauseAll
**	11/19/97	Fixed a bug in GM_AudioStreamService::STREAM_MODE_DEAD that forgot to set the dataLength
**				Fixed bug with GM_AudioStreamSetup in which the dataLength (in frames) was setup as bytes
**				and fixed a potential bug in which bufferSize was less than round value and it rounded
**				into a negitive (large) number
**	11/24/97	Added GM_AudioStreamFlush
**	12/4/97		Modifed PV_StartThisBufferPlaying to set a flush flag to FALSE. Modified GM_AudioStream to
**				included a flush flag when GM_AudioStreamFlush is called. Modified 
**				GM_AudioStreamService::STREAM_MODE_STOP_STREAM to detect the flush flag and free the stream
**				if the stream is already flushed.
**	12/8/97		Modified GM_AudioStreamStop to pass in a context
**				Changed GM_IsAudioStreamPlaying to check for the stream playing.
**	12/16/97	Moe: removed compiler warnings
**	1/29/98		Added defer parameter to GM_AudioStreamSetVolume
**	2/11/98		Modified for support of Q_48K & Q_24K & Q_8K
**	3/11/98		Added in PV_FreeStream to clear out streamID
**	3/12/98		Modified PV_StartThisBufferPlaying and various API's to implement a callback
**				when the sample is stopped to clear our playbackReference for a particular
**				voice
**	5/7/98		Changed GM_AudioStreamFileSetup to handle error codes
**	7/6/98		Changed usage of streamPlaybackOffset to use constant STREAM_OFFSET_UNSET
**
**	JAVASOFT
**	02.13.98	$$kk: set playbackReference = -1 whenever we call GM_EndSample to avoid jmf stream crossing
**	05.20.98	$$kk: changed GM_AudioStreamStop to handle the shutdown case for a paused stream.
**				otherwise, if the stream is paused, it'll never get shut down properly in
**				GM_AudioStreamService.  need to set up the state so that it can shut down.
**	???			$$kk: added GM_AudioStreamDrain
**	???			$$kk: changed GM_AudioStreamService to allow streams to exist in the engine until all samples played
**	???			$$kk: fixed bugs in GM_AudioStreamUpdateSamplesPlayed
**	9/9/98		Modified various routines that use the GM_Waveform structure to use the new elements
**				for file position
**	09.10.98	$$kk: added	STREAM_MODE_INACTIVE mode
**	09.23.98	$$kk: added GM_EventStatus struct and code to generate callbacks whenever
**				playback of a stream starts or stops in response to start(), stop(), or EOM.
**	9/10/98		Fixed a problem when USE_HIGHLEVEL_FILE_API is not defined
**	9/26/98		Changed GM_AudioStreamFileSetup & PV_FileStreamCallback to handle a change in a 
**				Gen API call for block allocations of custom file decoders
**				Fixed a bug with GM_AudioStreamGetFileSamplePosition that returned bytes rather
**				than samples
**	12/3/98		Added GM_GetStreamReverbAmount & GM_SetStreamReverbAmount
**	2/12/99		Renamed USE_HAE_FOR_MPEG to USE_MPEG_DECODER
**	2/25/99		Changed PV_AudioBufferCallback to use XPTR instead of G_PTR
**				Changed playbackReference to use VOICE_REFERENCE
**				Changed callbacks to use a context
**				Removed extra code in GM_AudioStreamStart
**	3/2/99		Changed all stream references to STREAM_REFERENCE rather than
**				the blank 'long'
**				Renamed pStream->reference to pStream->userReference
**	3/3/99		Changed to the new way of starting samples. First a setup, then a start.
**				Changed PV_StartThisBufferPlaying to set streamPlaybackOffset only when actaully
**				starting the voice used by the stream.
**				Added GM_AudioStreamGetFrequencyFilter & GM_AudioStreamSetFrequencyFilter 
**						GM_AudioStreamGetResonanceFilter & GM_AudioStreamSetResonanceFilter 
**						GM_AudioStreamGetLowPassAmountFilter & GM_AudioStreamSetLowPassAmountFilter
**	3/4/99		Broke PV_StartThisBufferPlaying into two functions: PV_PrepareThisBufferForPlaying & 
**				PV_StartStreamBuffers. One for setting up everything and one for actaully starting the
**				playback at the closest point inside of HAE. Required for Group streaming
**	3/5/99		Changed context to threadContext to reflect what is really is used
**				in the system.
**	2002-03-14	$$fb cleaned up debug code
**	2002-03-17      $$fb added resample code
*/
/***************************************************************************/


//#define USE_DEBUG			2
//#define TEST_UNDERFLOW_CODE		1

#include "X_API.h"
#include "X_Formats.h"
#include "GenSnd.h"
#include "GenPriv.h"
#include "HAE_API.h"

/* put these in comments to enable debugging of this file */
#undef DEBUG_STR
#define DEBUG_STR(x)

/* THINGS TO DO ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥

*/

/*
	Description of use:

	filter controls		GM_AudioStreamGetFrequencyFilter GM_AudioStreamSetFrequencyFilter 
						GM_AudioStreamGetResonanceFilter GM_AudioStreamSetResonanceFilter 
						GM_AudioStreamGetLowPassAmountFilter GM_AudioStreamSetLowPassAmountFilter


	SYNC START
	To start streams at the same time, call one of the GM_AudioStreamSetup or GM_AudioStreamFileSetup functions 
	then call GM_AudioStreamPreroll to get all data ready. Then call GM_SetSyncAudioStreamReference
	with a unique reference. The reference can be a pointer to a local structure. Its not used as anything other
	that common reference for all streams that you need to start at the moment. After they are started it is ignored.
	Then GM_SyncAudioStreamStart to actaully activate the streams. They will start at the next 11 ms slice. Be careful
	using these functions directly because they don't wait for the mixer slice to be ready, so you might actaully
	start streams between 11ms slices. The best way to insure it is to use the linked streams below. Those function
	use the ones described.

	LINKED STREAMS
	Call one of the GM_AudioStreamSetup or GM_AudioStreamFileSetup functions in the various standard ways, to get an 
	allocate stream then call GM_AudioStreamPreroll then call GM_NewLinkedStreamList. Then add it to your maintained 
	top list of linked voices with by calling GM_AddLinkedStream. Use GM_FreeLinkedStreamList to delete an entire list, 
	or GM_RemoveLinkedStream to just one link.

	Then you can call GM_StartLinkedStreams to start them all at the same time, or GM_EndLinkedStreams
	to end them all, or GM_SetLinkedStreamVolume, GM_SetLinkedStreamRate, and GM_SetLinkedStreamPosition
	set parameters about them all.

	management			GM_NewLinkedStreamList GM_FreeLinkedStreamList GM_AddLinkedStream GM_RemoveLinkedStream

	info				GM_GetLinkedStreamPlaybackReference

	control				GM_StartLinkedStreams GM_EndLinkedStreams

	sync control		GM_SetLinkedStreamVolume GM_SetLinkedStreamRate GM_SetLinkedStreamPosition


*/






#if USE_STREAM_API == TRUE

#define STREAM_ID						FOUR_CHAR('L','I','V','E')	//	'LIVE' id for verification of valid stream

#define MAX_SAMPLE_OVERSAMPLE			4		// number of samples extra per buffer

// Stream modes
#define STREAM_MODE_INTERRUPT_ACTIVE	0x80	// this value is or'd onto the overall value
#define STREAM_MODE_DEAD				0x00
#define STREAM_MODE_START_BUFFER_1		0x01	// has to be 1
#define STREAM_MODE_START_BUFFER_2		0x02	// has to be 2
#define STREAM_MODE_STOP_STREAM			0x03
#define STREAM_MODE_FREE_STREAM			0x04

// $$kk: 09.10.98: added this mode.
// we want to be able to PV_AddStream even if we underflowed in  GM_AudioStreamSetup;
// otherwise GM_AudioStreamStart fails with PARAM_ERR because  PV_AudioStreamGetFromReference
// fails.  but we need a mode which allows us to go through the service loop (so we can free
// the stream) without trying to play sound.
#define STREAM_MODE_INACTIVE			0x05


/* Used for file playback */
struct GM_AudioStreamFileInfo
{
    XFILENAME		playbackFile;
    XFILE			fileOpenRef;
    SAMPLE_COUNT	fileStartPosition;			// in bytes
    SAMPLE_COUNT	filePlaybackPosition;		// in bytes
    SAMPLE_COUNT	fileEndPosition;			// in bytes
    XBOOL			loopFile;

#if USE_HIGHLEVEL_FILE_API != FALSE
    AudioFileType	fileType;
#endif
    INT32			formatType;		// typed file compression mode

    XPTR			pBlockBuffer;	// used for decompression
    UINT32	blockSize;		// used for decompression
};
typedef struct GM_AudioStreamFileInfo GM_AudioStreamFileInfo;

// $$kk: 09.23.98: added this block ->

/* Status of playback events.  Playback events occur when rendering
 * at the _device_ starts or stops.  
 * When an action for which a playback event will be generate happens
 * (start, stop, pause, resume), the event is put in the EVENT_PENDING
 * state.  When it is detected as having occurred in GM_AudioStreamUpdateSamplesPlayed,
 * it is marked EVENT_DETECTED.  When the event representing it is 
 * dispatched in GM_AudioStreamService, it is marked EVENT_RESOLVED.
 */
/*
typedef enum
{
	EVENT_RESOLVED				= 0,
	EVENT_PENDING,
	EVENT_DETECTED,
	BUFFER_STATUS_CHANGE_DETECTED,
	BUFFER_STATUS_CHANGE_RESOLVED
} GM_EventStatus;
*/
// $$kk: 11.10.99: changed this enum
typedef enum
{
    START_OF_MEDIA				= 0,
    RESUME,
    ACTIVE,
    END_OF_MEDIA,
    PAUSE,
    INACTIVE,
    RESOLVED					// RESOLVED is for marking that we're done forever after sending the EOM message
} GM_EventStatus;



/*
 * Describes a playback event: its status and the stream playback frame position 
 * when it occurred.
 */
struct GM_PlaybackEvent
{
    GM_EventStatus status;
    XBOOL detected;						// TRUE if an event was detected and is pending (set to FALSE after event is sent)
    SAMPLE_COUNT framePosition;	
};
typedef struct GM_PlaybackEvent GM_PlaybackEvent;
// $$kk: 09.23.98: end changes <-

#define STREAM_OFFSET_UNSET		(((SAMPLE_COUNT)1) << 60)

// this structure, once allocated, becomes a STREAM_REFERENCE
struct GM_AudioStream
{
    void *					userReference;
    INT32					streamID;
    VOICE_REFERENCE			playbackReference;	// voice reference to live mixer voice. It
    // will be DEAD_VOICE if not active

    OPErr					startupStatus;		// error return before startup
    short int				startupBufferFullCount;

    GM_StreamObjectProc		streamCallback;
    GM_StreamData			streamData;
    void					*pStreamBuffer;
    UINT32			streamBufferLength;

    UINT32			streamOrgLength1;
    UINT32			streamOrgLength2;

    void					*pStreamData1;
    void					*pStreamData2;
    UINT32			streamLength1;
    UINT32			streamLength2;
    XBYTE					streamMode;					// Stream modes
    XBYTE					lastStreamBufferPlayed;

    SAMPLE_COUNT			streamPlaybackPosition;		// in samples; samples in this stream processed by engine
    SAMPLE_COUNT			streamPlaybackOffset;		// in samples; total samples processed by engine when this stream starts
    SAMPLE_COUNT           samplesWritten;             // update in GM_AudioStreamService. total number of samples
    SAMPLE_COUNT           samplesPlayed;              // update in GM_AudioStreamUpdateSamplesPlayed. total number of samples played

    // $$kk: 08.12.98 merge: added this field   
    SAMPLE_COUNT           residualSamples;			// if we're underflowing, then get more data and reset streamPlaybackOffset
    // before playing all samples, need to record that we can still play these
    // samples before reaching the new streamPlaybackOffset.

    // $$kk: 11.10.99: added this variable
    XBOOL					active;						// TRUE if the stream is currently active as determined in GM_AudioStreamUpdateSamplesPlayed

    // $$kk: 09.23.98: added these two variables ->
    GM_PlaybackEvent		startEvent;					
    GM_PlaybackEvent		stopEvent;					
    // $$kk: 09.23.98: end changes <-
	
    XBOOL					streamPrerolled;				// will be true, if PV_PrepareThisBufferForPlaying has been called
    XBOOL					streamActive;
    XBOOL					streamShuttingDown;
    XBOOL					streamPaused;
    XBOOL					streamFirstTime;			// first time active
    XBOOL					streamUnderflow;
    XBOOL					streamFlushed;				// only set to TRUE when flush is called. Reset to FALSE at start

    XFIXED					streamFadeRate;				// when non-zero fading is enabled
    XFIXED					streamFixedVolume;			// inital volume level that will be changed by streamFadeRate
    short int				streamFadeMaxVolume;		// max volume
    short int				streamFadeMinVolume;		// min volume
    XBOOL					streamEndAtFade;

    // state
    XBOOL					streamUseReverb;
    short int				streamReverbAmount;
    short int				streamVolume;
    short int				streamStereoPosition;
    short int				streamFrequencyFilter;
    short int				streamLowPassAmountFilter;
    short int				streamResonanceFilter;

    OPErr					streamErr;
	
    /* $$fb 2002-03-17 added resample algorithms */
    SR_ResampleParams*			resampleParams;
    VOICE_REFERENCE                         resampleVoice;  // the voice the resampleParams are set to

    GM_AudioStreamFileInfo	*pFileStream;				// if not NULL, then streaming file
    struct GM_AudioStream	*pNext;
};
typedef struct GM_AudioStream GM_AudioStream;

// linked list of all active streams. Required for servicing. Call GM_AudioStreamService() to process
// all the streams, for fades, callbacks, reads, etc
static GM_AudioStream	*theStreams = NULL;


// change activation of resampler. Force on, or off
void GM_SetStreamResample(GM_AudioStream* pStream, XBOOL enable) {
    SR_ResampleParams	*params;

    if (pStream) {
	if (enable) {
	    /* resample on */
	    if (pStream->resampleParams == NULL) {
#ifdef USE_DEBUG
		printf("GenAudioStreams:SetStreamResample Creating resampleParams for voice %d\n", 
		       (int) pStream->playbackReference); fflush(stdout);
#endif
		params = (SR_ResampleParams*) XNewPtr((INT32)sizeof(SR_ResampleParams));
		if (SR_init(params, 
			    pStream->streamData.sampleRate,
			    GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality),		// output_samplerate
			    MusicGlobals->generateStereoOutput?2:1,	// (output) channels
			    MusicGlobals->generate16output?16:8	// (output) sample_size_in_bits
			    )) {
		    pStream->resampleParams = params;
		} 
		else if (params) {
		    XDisposePtr((XPTR)params);
		}
	    }
	    if (pStream->playbackReference != DEAD_VOICE) {
		GM_SetSampleResampleExtern(pStream->playbackReference, pStream->resampleParams);
		pStream->resampleVoice = pStream->playbackReference;
#ifdef USE_DEBUG
		printf("GenAudioStreams:SetStreamResample. Setting pStream=%p voice to %d \n", 
		       pStream, (int) pStream->playbackReference); fflush(stdout);
#endif
	    }
	} else {
	    /* resample off */
	    params = pStream->resampleParams;
	    if (params != NULL) {
		pStream->resampleParams = NULL;
		if (pStream->resampleVoice != DEAD_VOICE) {
#ifdef USE_DEBUG
		    printf("GenAudioStreams:SetStreamResample: setting external to NULL on voice %d\n",(int) pStream->resampleVoice); fflush(stdout);
#endif
		    GM_RemoveSampleResampleExtern(pStream->resampleVoice, params);
		    pStream->resampleVoice = DEAD_VOICE;
		}
		SR_exit(params);
#ifdef USE_DEBUG
		printf("GenAudioStreams:SetStreamResample: Disposing resampleParams\n"); fflush(stdout);
#endif
		XDisposePtr((XPTR)params);
	    }
			
	}
    }
}

// verify reference is a valid audio stream structure
static GM_AudioStream * PV_AudioStreamGetFromReference(STREAM_REFERENCE reference)
{
    GM_AudioStream *pStream;
    GM_AudioStream *next;
	
    pStream = (GM_AudioStream *)reference;
    next = theStreams;
    while ( next != NULL )
	{
	    if (next == pStream)
		{
		    if (pStream->streamID == STREAM_ID)
			{
			    return pStream;
			}
		}
	    next = next->pNext;
	}
    return NULL;
}

// add a valid stream to the global stream list
static void PV_AddStream(GM_AudioStream *next)
{
    GM_AudioStream *last;

    if (next)
	{
	    next->streamID = STREAM_ID;
	    if (theStreams == NULL)
		{
		    theStreams = next;
		}
	    else
		{
		    last = theStreams;
		    while (last->pNext)
			{
			    last = last->pNext;
			}
		    last->pNext = next;
		}
	    next->pNext = NULL;
	}
}

// remove a stream from the global stream list
static void PV_FreeStream(GM_AudioStream *found)
{
    GM_AudioStream *next, *last;

    found = PV_AudioStreamGetFromReference((STREAM_REFERENCE)found);		// verify as valid
    if (found)
	{
	    if (found->streamID == STREAM_ID)
		{
		    last = next = theStreams;
		    while (next)
			{
			    if (next == found)						// found object in list?
				{
				    if (next == theStreams)				// is object the top object
					{
					    theStreams = next->pNext;		// yes, change to next object
					}
				    else
					{
					    if (last)						// no, change last to point beyond next
						{
						    last->pNext = next->pNext;
						}
					}
				    if (next->pFileStream)
					{
					    XDisposePtr(next->pFileStream->pBlockBuffer);
					    XDisposePtr(next->pFileStream);
					}
				    /* $$fb 2002-03-17: added resample code */
				    GM_SetStreamResample(next, FALSE);
				    next->streamID = 0;
				    XDisposePtr(next);					// clean up
				    break;
				}
			    last = next;
			    next = next->pNext;
			}
		}
	}
}

// Get an empty GM_AudioStream. Will return DEAD_STREAM if can't allocate stream
static STREAM_REFERENCE PV_GetEmptyAudioStream(void)
{
    GM_AudioStream	*pStream;
    STREAM_REFERENCE	ref;

    ref = DEAD_STREAM;
    pStream = (GM_AudioStream *)XNewPtr((INT32)sizeof(GM_AudioStream));
    if (pStream)
	{
	    pStream->userReference = 0;
	    pStream->playbackReference = DEAD_VOICE;
	    pStream->resampleParams = NULL;
	    pStream->resampleVoice = DEAD_VOICE;
	    ref = (STREAM_REFERENCE)pStream;
	}
    return ref;
}


static UINT32 PV_GetSampleSizeInBytes(GM_StreamData * pAS)
{
    return pAS->channelSize * (pAS->dataBitSize / 8);
}


static void PV_FillBufferEndWithSilence(char *pDest, GM_StreamData * pAS)
{
    UINT32	bufferSize, blockSize;
    UINT32	count;
    short int		*pWData;

    if (pDest)
	{
	    blockSize = MAX_SAMPLE_OVERSAMPLE * PV_GetSampleSizeInBytes(pAS);
	    bufferSize = (pAS->dataLength * PV_GetSampleSizeInBytes(pAS));

	    pDest += bufferSize;
	    if (pAS->dataBitSize == 8)
		{
		    for (count = 0; count < blockSize; count++)
			{
				// $$fb 2002-02-01: wrong type cast [was: (char)0x80]
			    *pDest++ = (char)-128;
			}
		}
	    else
		{
		    pWData = (short int *)pDest;
		    blockSize /= 2;
		    for (count = 0; count < blockSize; count++)
			{
			    *pWData++ = 0;
			}
		}
	}
}

static void PV_CopyLastSamplesToFirst(char *pSource, char *pDest, GM_StreamData * pAS)
{
    UINT32	bufferSize, blockSize;

    if (pAS->dataLength && pSource && pDest)
	{
	    blockSize = MAX_SAMPLE_OVERSAMPLE * PV_GetSampleSizeInBytes(pAS);
	    bufferSize = (pAS->dataLength * PV_GetSampleSizeInBytes(pAS));
	    if (bufferSize)
		{
		    XBlockMove(pSource + bufferSize, pDest, blockSize);
		}
	}
}

// this callback will only be called in extreme cases in which the voice is starved to death
// and when the stream is forced to stop

/* $$fb 2003-03-14: 
 * verify the sender's voice: if it is not "our" voice, then
 * do NOT kill this stream. This happens when e.g. a series of
 * start/stop calls cause 2 different voices to be used for this
 * stream and the DoneCallback of an old voice comes in after a
 * new voice was established.
 * This is the  fix for 4828556 
 */
static void PV_AudioBufferFinished(VOICE_REFERENCE sender, void *context, void *threadContext)
{
    GM_AudioStream	*pStream;

    pStream = (GM_AudioStream *)context;

    //printf("PV_AudioBufferFinished with sender=%d, pStream=%x\n", pStream); fflush(stdout);

    // This is the same thing as the above line, just faster.
    //	pStream = PV_AudioStreamGetFromReference(context);

    if (pStream) {
	if (pStream->playbackReference == sender) {
	    //printf("--sender is our voice. kill it!\n"); fflush(stdout);
	    pStream->playbackReference = DEAD_VOICE;	// kill our reference to this voice
	} else {
	    //printf("--sender is different than my voice! do not kill it.\n"); fflush(stdout);
	}
    }
}

static void PV_AudioBufferCallback(void *context, XPTR pWhichBufferFinished, INT32 *pBufferSize_IN_OUT)
{
    GM_AudioStream	*pStream;

    //#pragma unused (pWhichBufferFinished)
    pWhichBufferFinished = pWhichBufferFinished;

    pStream = (GM_AudioStream *)context;

    // This is the same thing as the above line, just faster.
    //	pStream = PV_AudioStreamGetFromReference(context);

    if (pStream)
	{
#ifdef USE_DEBUG
	    printf("Playback Pos %ld Length %ld\r", 
		   pStream->streamPlaybackPosition, 
		   *pBufferSize_IN_OUT);
	    printf("buffer done %lx size %ld", 
		   pWhichBufferFinished, 
		   *pBufferSize_IN_OUT);
#endif
	    pStream->streamPlaybackPosition += *pBufferSize_IN_OUT;
	    switch (pStream->streamMode & (~STREAM_MODE_INTERRUPT_ACTIVE))
		{
		default:
		    DEBUG_STR("Bad case in PV_AudioBufferCallback");
		    break;
		case STREAM_MODE_STOP_STREAM:
		    DEBUG_STR("PV_AudioBufferCallback::STREAM_MODE_STOP_STREAM");
		    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_FREE_STREAM;		// end
		    pStream->streamShuttingDown = TRUE;
		    *pBufferSize_IN_OUT = 0;
		    break;
		    // buffer 1 ends playback
		case STREAM_MODE_START_BUFFER_1:	// start buffer 2 playing
		    DEBUG_STR("PV_AudioBufferCallback::STREAM_MODE_START_BUFFER_1");
		    if (pStream->streamFirstTime)
			{
				// copy end of buffer 2 into the start of buffer 1
				// This only needs to happen once at the start because the buffers are different in the begining
			    PV_CopyLastSamplesToFirst((char *)pStream->pStreamData2, (char *)pStream->pStreamData1, &pStream->streamData);
			    pStream->streamFirstTime = FALSE;
			}

		    *pBufferSize_IN_OUT = pStream->streamLength2;
		    if (pStream->streamShuttingDown)
			{
			    pStream->streamShuttingDown = TRUE;
			    DEBUG_STR("    End of BUFFER_1");
			    if (pStream->streamLength2)
				{
				    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_STOP_STREAM;		// end
				}
			    else
				{
				    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_FREE_STREAM;		// end
				}
			}
		    else
			{
			    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_START_BUFFER_2;		// buffer1 read and playing2
			}
		    break;
		    // buffer 2 ends playback
		case STREAM_MODE_START_BUFFER_2:
		    DEBUG_STR("PV_AudioBufferCallback::STREAM_MODE_START_BUFFER_2");
		    *pBufferSize_IN_OUT = pStream->streamLength1;
		    if (pStream->streamShuttingDown)
			{
			    pStream->streamShuttingDown = TRUE;
			    DEBUG_STR("    End of BUFFER_2");
			    if (pStream->streamLength1)
				{
				    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_STOP_STREAM;		// end
				}
			    else
				{
				    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_FREE_STREAM;		// end
				}
			}
		    else
			{
			    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_START_BUFFER_1;		// buffer2 read and playing1
			}
		    break;
		}
	}
}

// Call this after calling PV_PrepareThisBufferForPlaying. All this function does is start
// the voice playing and record its start time in the stream. Its already allocated and ready.
static void PV_StartStreamBuffers(GM_AudioStream * pStream)
{
    if (pStream)
	{
	    if (pStream->streamPrerolled)
		{
		    // if it is currently unset (STREAM_OFFSET_UNSET value), set the streamPlaybackOffset 
		    // to the current position of the engine in samples
		    if (pStream->streamPlaybackOffset == STREAM_OFFSET_UNSET)
			{
			    pStream->streamPlaybackOffset = MusicGlobals->samplesWritten;
			}
		    GM_StartSample(pStream->playbackReference);
		}
	}
}

// setup and allocate an HAE voice for a playing buffer. Account for MAX_SAMPLE_OVERSAMPLE in length
static XBOOL PV_PrepareThisBufferForPlaying(GM_AudioStream * pStream, XBYTE bufferNumber)
{
    XBYTE	mode;

    pStream->streamPrerolled = FALSE;
    // $$kk: 08.12.98 merge: added this block 
    // $$kk: we use (pStream->streamPlaybackOffset == STREAM_OFFSET_UNSET) to indicate that the
    // value needs to be set.  this is the case 1) when the stream is just
    // starting up and 2) when the stream has shut down (probably due to
    // underflow).  this is now the centralized location for setting
    // pStream->streamPlaybackOffset to STREAM_OFFSET_UNSET during play.  (before, there were
    // potential timing problems, and we didn't necessarily count all the
    // committed samples....

    if (GM_IsSoundDone(pStream->playbackReference))
	{
	    // if we were set to a valid offset before (i.e. we're not just starting)
	    // record any residual samples that we should count.
	    if (pStream->streamPlaybackOffset != STREAM_OFFSET_UNSET)
		{
		    pStream->residualSamples = pStream->samplesWritten - pStream->samplesPlayed;
		}
	    pStream->streamPlaybackOffset = STREAM_OFFSET_UNSET;
	}
    // $$kk: 08.12.98 merge: end new block  

    if (pStream->streamShuttingDown == FALSE)
	{
	    switch (bufferNumber)
		{
		case 0:	// don't know which buffer to play, figure it out, eh?
		    DEBUG_STR("PV_PrepareThisBufferForPlaying:0");
		    if (pStream->lastStreamBufferPlayed)
			{
			    mode = pStream->streamMode &= (~STREAM_MODE_INTERRUPT_ACTIVE);
			    if (mode != pStream->lastStreamBufferPlayed)
				{
				    DEBUG_STR("DIFFERENT!!");
				    mode = pStream->lastStreamBufferPlayed;
				}
			    pStream->streamPrerolled = PV_PrepareThisBufferForPlaying(pStream, mode);	// play last successfull buffer played
			}
		    break;

		case STREAM_MODE_START_BUFFER_1:
		    DEBUG_STR("PV_PrepareThisBufferForPlaying:STREAM_MODE_START_BUFFER_1");
		    if (pStream->streamLength1)
			{
			    pStream->playbackReference =
				GM_SetupSampleDoubleBuffer(	(XPTR)pStream->pStreamData1,
								(XPTR)pStream->pStreamData2,
								pStream->streamLength1,
								pStream->streamData.sampleRate,
								pStream->streamData.dataBitSize, pStream->streamData.channelSize,
								pStream->streamVolume,
								pStream->streamStereoPosition,
								(void *)pStream,
								PV_AudioBufferCallback,
								PV_AudioBufferFinished);
			    if (pStream->playbackReference != DEAD_VOICE)	// successfull?
				{
				    pStream->lastStreamBufferPlayed = STREAM_MODE_START_BUFFER_1;
				    pStream->streamPrerolled = TRUE;
				    /*
				      // if it is currently unset (STREAM_OFFSET_UNSET value), set the streamPlaybackOffset 
				      // to the current position of the engine in samples
				      if (pStream->streamPlaybackOffset == STREAM_OFFSET_UNSET)
				      {
				      pStream->streamPlaybackOffset = MusicGlobals->samplesWritten;
				      }
				      GM_StartSample(pStream->playbackReference);
				    */
				    /* $$fb 2002-03-14: enable high quality sample rate conversion */
#ifdef USE_DEBUG
				    printf("GenAudioStreams:PV_PrepareThisBufferForPlaying: "
					   "call to GM_SetSampleResampleExtern. "
					   "Setting pStream=%p voice to %d \n", 
					   pStream, (int) pStream->playbackReference); 
				    fflush(stdout);
#endif
				    GM_SetSampleResampleExtern(pStream->playbackReference, pStream->resampleParams);
				    pStream->resampleVoice = pStream->playbackReference;
				}
#ifdef USE_DEBUG
			    else { 
				printf("GenAudioStreams:PV_PrepareThisBufferForPlaying: "
				       "call to GM_SetupSampleDoubleBuffer failed. "
				       "playbackReference is NULL\n"); 
				fflush(stdout); 
			    }
#endif
			    pStream->streamFlushed = FALSE;
			}
		    break;
		case STREAM_MODE_START_BUFFER_2:
		    DEBUG_STR("PV_PrepareThisBufferForPlaying:STREAM_MODE_START_BUFFER_2");
		    if (pStream->streamLength2)
			{
			    pStream->playbackReference =
				GM_SetupSampleDoubleBuffer(	(XPTR)pStream->pStreamData2,
								(XPTR)pStream->pStreamData1,
								pStream->streamLength2,
								pStream->streamData.sampleRate,
								pStream->streamData.dataBitSize, pStream->streamData.channelSize,
								pStream->streamVolume,
								pStream->streamStereoPosition,
								(void *)pStream,
								PV_AudioBufferCallback,
								PV_AudioBufferFinished);
			    if (pStream->playbackReference != DEAD_VOICE)	// successfull?
				{
				    pStream->lastStreamBufferPlayed = STREAM_MODE_START_BUFFER_2;
				    pStream->streamPrerolled = TRUE;
				    /*
		                // if it is currently unset (STREAM_OFFSET_UNSET value), set the streamPlaybackOffset 
		                // to the current position of the engine in samples
		                if (pStream->streamPlaybackOffset == STREAM_OFFSET_UNSET)
		                {
				pStream->streamPlaybackOffset = MusicGlobals->samplesWritten;
				}
				GM_StartSample(pStream->playbackReference);
				    */
				    /* $$fb 2002-03-14: enable high quality interpolation */
#ifdef USE_DEBUG
				    printf("GenAudioStreams:PV_PrepareThisBufferForPlaying: "
					   "call to  GM_SetSampleResampleExtern. "
					   "Setting pStream=%p voice to %d \n", 
					   pStream, (int) pStream->playbackReference); 
				    fflush(stdout);
#endif
				    GM_SetSampleResampleExtern(pStream->playbackReference, pStream->resampleParams);
				    pStream->resampleVoice = pStream->playbackReference;
				}
#ifdef USE_DEBUG
			    else { 
				printf("GenAudioStreams:PV_PrepareThisBufferForPlaying: "
				       "call to GM_SetupSampleDoubleBuffer failed. "
				       "playbackReference is NULL\n"); 
				fflush(stdout); 
			    }
#endif
			    pStream->streamFlushed = FALSE;
			}
		    break;
		}
	}
    return pStream->streamPrerolled;
}

//$$kk: 10.22.97
// i am taking out these pause/resume methods and replacing them with the
// last version; these for some reasons cause synch to be off ON WIN32 ONLY
// after pause/resume

// Pause this particular audio stream
// $$kk: 08.12.98 merge: changed this function to avoid stream crossing
void GM_AudioStreamPause(STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (pStream->streamActive && (!pStream->streamPaused))
		{
		    //  $$kk: 02.13.98: set playbackReference = DEAD_VOICE whenever we call GM_EndSample to avoid jmf stream crossing
		    VOICE_REFERENCE playbackReference = pStream->playbackReference;

		    // $$kk: 09.23.98: added this ->
			
		    // $$kk: 11.10.99
		    //pStream->stopEvent.status = EVENT_PENDING;
		    pStream->stopEvent.status = PAUSE;

		    // $$kk: 09.23.98: end changes <-

		    pStream->streamPaused = TRUE;

		    pStream->playbackReference = DEAD_VOICE;
			
		    //$$fb 2002-04-20: use thread safe version of GM_EndSample
		    GM_ReleaseSample(playbackReference, NULL);
		}
	}
}

// Resume this particular audio stream
// $$kk: 08.12.98 merge: changed this function to avoid stutter
void GM_AudioStreamResume(STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (pStream->streamActive && pStream->streamPaused)
		{
		    // $$kk: 04.30.99: added this here; otherwise it only gets set
		    // in GM_AudioStreamPreroll, and that doesn't get called here!
		    // $$kk: 09.23.98: added this ->
		    //pStream->startEvent.status = EVENT_PENDING;

		    // $$kk: 11.10.99
		    if (pStream->startEvent.status != START_OF_MEDIA)
			{
			    pStream->startEvent.status = RESUME;
			}

		    // $$kk: 09.23.98: end changes <-

		    pStream->streamPaused = FALSE;

		    // $$kk: 02.20.98
		    // if the stream is flushed and STREAM_MODE_DEAD, should not start playing here!
		    // this change should fix the JMF stutter problem, bug #4098200
		    if (!(pStream->streamFlushed))
			{
			    if (PV_PrepareThisBufferForPlaying(pStream, (XBYTE)(pStream->streamMode & (~STREAM_MODE_INTERRUPT_ACTIVE))))
				{
				    PV_StartStreamBuffers(pStream);
				}
			}
		}
	}
}



// Pause all audio streams
// $$kk: 08.12.98 merge: changed this function to avoid stream crossing
void GM_AudioStreamPauseAll(void)
{
    GM_AudioStream	*pStream;

    pStream = theStreams;
    while (pStream)
	{
	    if (pStream->streamActive && (!pStream->streamPaused))
		{
		    //  $$kk: 02.13.98: set playbackReference = DEAD_VOICE whenever we call GM_EndSample to avoid jmf stream crossing
		    VOICE_REFERENCE playbackReference = pStream->playbackReference;

		    pStream->streamPaused = TRUE;

		    pStream->playbackReference = DEAD_VOICE;
			
		    //$$fb 2002-04-20: use thread safe version of GM_EndSample
		    GM_ReleaseSample(playbackReference, NULL);
		}
	    pStream = pStream->pNext;
	}
}

// Resume all audio streams
void GM_AudioStreamResumeAll(void)
{
    GM_AudioStream	*pStream;

    pStream = theStreams;
    while (pStream)
	{
	    if (pStream->streamActive && pStream->streamPaused)
		{
		    pStream->streamPaused = FALSE;
		    if (PV_PrepareThisBufferForPlaying(pStream, (XBYTE)(pStream->streamMode & (~STREAM_MODE_INTERRUPT_ACTIVE))))
			{
			    PV_StartStreamBuffers(pStream);
			}
		}
	    pStream = pStream->pNext;
	}
}

OPErr GM_AudioStreamError(STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;
    OPErr				theErr;

    theErr = NO_ERR;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    theErr = pStream->streamErr;
	}
    return theErr;
}

#if USE_HIGHLEVEL_FILE_API
// streaming file callback. Used for GM_AudioStreamFileStart to decode typed files.
static OPErr PV_FileStreamCallback(void *context, GM_StreamMessage message, GM_StreamData *pAS)
{
    OPErr					error;
    UINT32			bufferSize, fileSize, outputBufferSize;
    GM_AudioStreamFileInfo	*pASInfo;
    GM_AudioStream			*pStream;

#if TEST_UNDERFLOW_CODE
    static int				pv_count = 0;
    static int				pv_count2 = 0;
#endif

    context = context;
    error = NO_ERR;
    switch (message)
	{
	case STREAM_CREATE:
	    DEBUG_STR("PV_FileStreamCallback::STREAM_CREATE\r");
	    pStream = (GM_AudioStream *)pAS->streamReference;
	    pASInfo = (GM_AudioStreamFileInfo *)pAS->userReference;
	    pStream->pFileStream = pASInfo;

	    switch (pASInfo->fileType)
		{
#if USE_MPEG_DECODER != 0
		case FILE_MPEG_TYPE:
		    if (pASInfo->pBlockBuffer)
			{
			    XMPEGDecodedData	*pMPG;
					
			    pMPG = (XMPEGDecodedData *)pASInfo->pBlockBuffer;
			    // remember to take in account that the data length passed is always in audio frames not bytes
			    bufferSize = pAS->dataLength * pAS->channelSize * (pAS->dataBitSize / 8);
			    pAS->pData = XNewPtr(bufferSize);
			    if (pAS->pData)
				{
				    error = NO_ERR;
				}
			    else
				{
				    error = MEMORY_ERR;
				}
			}
		    else
			{
			    error = GENERAL_BAD;
			}
		    break;
#endif
		default:
		    pASInfo->fileOpenRef = XFileOpenForRead(&pASInfo->playbackFile);
		    if (pASInfo->fileOpenRef)
			{
			    // remember to take in account that the data length passed is always in audio frames not bytes
			    bufferSize = pAS->dataLength * pAS->channelSize * (pAS->dataBitSize / 8);

			    pAS->pData = XNewPtr(bufferSize);
			    if (pAS->pData)
				{
				    error = NO_ERR;
				    XFileSetPosition(pASInfo->fileOpenRef, pASInfo->filePlaybackPosition);
				}
			    else
				{
				    error = MEMORY_ERR;
				}
			}
		    else
			{
			    error = GENERAL_BAD;
			}
		}
	    break;
	case STREAM_DESTROY:
	    DEBUG_STR("PV_FileStreamCallback::STREAM_DESTROY\r");
	    pASInfo = (GM_AudioStreamFileInfo *)pAS->userReference;

	    switch (pASInfo->fileType)
		{
#if USE_MPEG_DECODER != 0
		case FILE_MPEG_TYPE:
		    if (pASInfo->pBlockBuffer)
			{
			    XMPEGDecodedData	*pMPG;
					
			    pMPG = (XMPEGDecodedData *)pASInfo->pBlockBuffer;
			    XCloseMPEGStream(pMPG);					
			}
		    break;
#endif
		default:
		    if (pASInfo->fileOpenRef)
			{
			    XFileClose(pASInfo->fileOpenRef);
			}
		    break;
		}
	    XDisposePtr((XPTR)pAS->pData);
	    pAS->pData = NULL;
	    break;
	case STREAM_GET_SPECIFIC_DATA:
	    error = NOT_SETUP;
	    break;
	case STREAM_GET_DATA:
#if TEST_UNDERFLOW_CODE
	    pv_count++;
	    if (pv_count == 0)
		{
		    pAS->dataLength = 0;
		    DEBUG_STR("Fake underflow!");
		    return NO_ERR;
		}
#endif
	    pASInfo = (GM_AudioStreamFileInfo *)pAS->userReference;
	    if (pAS->pData)
		{
				// get the desired length, and account for stereo and bit size
		    error  = GM_ReadAndDecodeFileStream(	pASInfo->fileOpenRef,
								(AudioFileType)pASInfo->fileType,
								pASInfo->formatType,
								pASInfo->pBlockBuffer,
								pASInfo->blockSize,
								(XPTR)pAS->pData,
								pAS->dataLength,
								pAS->channelSize,
								pAS->dataBitSize,
								&outputBufferSize,
								&fileSize);

#ifdef USE_DEBUG
		    printf("STREAM_GET_DATA::frames %ld outputBufferSize %ld fileSize %ld", 
			   pAS->dataLength, outputBufferSize, fileSize);
#endif
				// NOTE: we are incrementing the file position by the buffer size rather
				// than the file size because our units are in audio bytes.
		    pASInfo->filePlaybackPosition += outputBufferSize;

				// return actual length, if this is not done, then it will
				// continue to play the last buffer size
				// NOTE: we're returned the number of audio frames, not the actual byte size
		    pAS->dataLength = outputBufferSize / pAS->channelSize / (pAS->dataBitSize / 8);

		    switch (pASInfo->fileType)
			{
#if USE_MPEG_DECODER != 0
			case FILE_MPEG_TYPE:
			    break;
#endif
			default:
			    if (pASInfo->filePlaybackPosition >= pASInfo->fileEndPosition)
				{
				    // how much over are we? Our fileEndPosition is at the exact end of the file,
				    // so if we're over then there's extra data after the sample data. We don't want to
				    // include that in the playback because its noise.
				    fileSize = pASInfo->filePlaybackPosition - pASInfo->fileEndPosition;
				    pASInfo->filePlaybackPosition -= fileSize;
				    pAS->dataLength -= (fileSize / pAS->channelSize / (pAS->dataBitSize / 8));

				    error = NO_ERR;
				    if (pASInfo->loopFile)
					{
					    pASInfo->filePlaybackPosition = pASInfo->fileStartPosition;
					    XFileSetPosition(pASInfo->fileOpenRef, pASInfo->filePlaybackPosition);
					    // we've hit the end of the file, so reset to begining and continue
					    // clear IMA transient data
					    XSetMemory(pASInfo->pBlockBuffer, pASInfo->blockSize, 0);
					}
				    else
					{
					    error = STREAM_STOP_PLAY;	// we've hit the end of the file, so stop
					}
				}
			    else
				{
				    error = NO_ERR;
				}
			    break;
			}
		}
	    else
		{
		    error = GENERAL_BAD;
		}
	    break;
	}
    return error;
}
#endif	// USE_HIGHLEVEL_FILE_API

#if USE_HIGHLEVEL_FILE_API != FALSE
// setup streaming a file and place it into pause mode. Don't start
STREAM_REFERENCE GM_AudioStreamFileSetup(void *threadContext,
					 XFILENAME *file, AudioFileType fileType,
					 UINT32 bufferSize, GM_Waveform *pFileInfo,
					 XBOOL loopFile)
{
    STREAM_REFERENCE		reference;
    GM_Waveform				*pWaveform;
    GM_AudioStreamFileInfo	*pStream;
    INT32					format;
    UINT32			blockSize;
    OPErr					err;
    void					*blockPtr;

    reference = DEAD_STREAM;
    blockPtr = NULL;
    pWaveform = GM_ReadFileInformation(file, fileType, &format, &blockPtr, &blockSize, &err);
    if (pWaveform && (err == NO_ERR))
	{
	    pStream = (GM_AudioStreamFileInfo *)XNewPtr((INT32)sizeof(GM_AudioStreamFileInfo));
	    if (pStream)
		{
		    pStream->playbackFile = *file;
		    pStream->loopFile = loopFile;
		    pStream->formatType = format;
		    pStream->fileType = fileType;
		    pStream->blockSize = blockSize;
		    if (blockPtr)
			{
			    pStream->pBlockBuffer = blockPtr;
			}
		    else if (blockSize)
			{
			    pStream->pBlockBuffer = XNewPtr(blockSize);
			}
		    // now the file is positioned right at the data block
		    pStream->filePlaybackPosition = pWaveform->currentFilePosition;
		    pStream->fileStartPosition = pWaveform->currentFilePosition;
		    // we want the byte size
		    pStream->fileEndPosition = pWaveform->waveSize + pStream->fileStartPosition;

		    if (pFileInfo)
			{
			    *pFileInfo = *pWaveform;
			}
		    if (blockSize)
			{
			    bufferSize = (bufferSize * blockSize) / blockSize;	// round down to blockSize
			}
		    reference = GM_AudioStreamSetup(threadContext, (void *)pStream, PV_FileStreamCallback,
						    bufferSize,
						    pWaveform->sampledRate,
						    pWaveform->bitSize,
						    pWaveform->channels);
		}
	    XDisposePtr(pWaveform);
	}
    return reference;
}
#endif // USE_HIGHLEVEL_FILE_API

// This will start a streaming audio object.
//
// INPUT:
//	userReference	This is a reference value that will be returned and should be passed along to all GM_AudioStream
//					functions.
//
//	pProc		is a GM_StreamObjectProc proc pointer. At startup of the streaming the proc will be called
//				with STREAM_CREATE, then followed by two STREAM_GET_DATA calls to get two buffers of data,
//				and finally STREAM_DESTROY when finished.
//
// OUTPUT:
//	long			This is an audio reference number. Will be non-zero for valid stream

STREAM_REFERENCE GM_AudioStreamSetup(void *threadContext, void *userReference, GM_StreamObjectProc pProc, 						
				     UINT32 bufferSize, 
				     XFIXED sampleRate,	// Fixed 16.16 sample rate
				     char dataBitSize,		// 8 or 16 bit data
				     char channelSize)		// 1 or 2 channels of date
{
    STREAM_REFERENCE	reference;
    GM_AudioStream		*pStream;
    GM_StreamData		ssData;
    OPErr				theErr;
    UINT32		byteLength;

    reference = DEAD_STREAM;
    theErr = NO_ERR;
    DEBUG_STR("> GM_AudioStreamSetup: Starting.\n");

    if (MusicGlobals->MaxEffects)
	{
	    //		if (bufferSize >= 10000L)
	    {
		if ( (pProc) && ( (channelSize >= 1) || (channelSize <= 2) ) && ( (dataBitSize == 8) || (dataBitSize == 16) ) )
		    {
			reference = PV_GetEmptyAudioStream();
			if (reference)
			    {
				pStream = (GM_AudioStream *)reference;
				pStream->streamCallback = pProc;
				pStream->streamUnderflow = FALSE;
				pStream->userReference = userReference;
				pStream->streamPlaybackPosition = 0;
				pStream->streamPlaybackOffset = STREAM_OFFSET_UNSET;     // not known
				pStream->samplesWritten = 0;
				pStream->samplesPlayed = 0;

				// $$kk: 08.12.98 merge 
				// $$kk: added residualSamples
				pStream->residualSamples = 0; 


				// $$kk: 11.10.99: added / edited this block

				pStream->active = FALSE;

				pStream->startEvent.status = START_OF_MEDIA;
				pStream->startEvent.detected = FALSE;
				pStream->startEvent.framePosition = 0;

				pStream->stopEvent.status = INACTIVE;
				pStream->stopEvent.detected = FALSE;
				pStream->stopEvent.framePosition = 0;

				/*
				  // $$kk: 09.23.98: added this block ->
				  //pStream->startEvent.status = EVENT_RESOLVED;
				  pStream->startEvent.status = BUFFER_STATUS_CHANGE_RESOLVED;
				  pStream->startEvent.framePosition = 0;
				  //pStream->stopEvent.status = EVENT_RESOLVED;
				  pStream->stopEvent.status = BUFFER_STATUS_CHANGE_RESOLVED;
				  pStream->stopEvent.framePosition = 0;
				  // $$kk: 09.23.98: end changes <-
				*/
				pStream->streamShuttingDown = FALSE;
				pStream->streamVolume = MAX_NOTE_VOLUME;
				pStream->streamStereoPosition = 0;
				ssData.pData = NULL;
				ssData.userReference = pStream->userReference;
				ssData.streamReference = (STREAM_REFERENCE)pStream;
				ssData.sampleRate = sampleRate;
				ssData.dataBitSize = dataBitSize;
				ssData.channelSize = channelSize;
				if (bufferSize > 1000)
				    {
					bufferSize -= (bufferSize % 1000L);		// round down to the nearest 100
				    }
				// add over sample extra
				bufferSize += PV_GetSampleSizeInBytes(&ssData) * MAX_SAMPLE_OVERSAMPLE;
				pStream->streamBufferLength = bufferSize;

				// convert from bytes to frames
				ssData.dataLength = bufferSize / PV_GetSampleSizeInBytes(&ssData);

				theErr = (*pProc)(threadContext, STREAM_CREATE, &ssData);

				if (theErr == NO_ERR)
				    {
					pStream->streamData = ssData;
					pStream->pStreamBuffer = ssData.pData;
					pStream->pStreamData1 = pStream->streamData.pData;
						
					/* $$fb 2002-03-17: add resampling for streams */
					GM_SetStreamResample(pStream, TRUE);

					byteLength = ssData.dataLength * PV_GetSampleSizeInBytes(&ssData);
					pStream->pStreamData2 = (char *)pStream->streamData.pData + (byteLength / 2);
					pStream->streamLength1 = ssData.dataLength / 2;
					pStream->streamLength2 = ssData.dataLength / 2;

					pStream->streamOrgLength1 = pStream->streamLength1;
					pStream->streamOrgLength2 = pStream->streamLength2;
					pStream->streamMode = STREAM_MODE_DEAD;
					pStream->streamUnderflow = FALSE;
					pStream->startupBufferFullCount = 0;

					// ok, fill first buffer
					ssData.userReference = pStream->userReference;
					ssData.streamReference = (STREAM_REFERENCE)pStream;
					ssData.pData = pStream->pStreamData1;
					// get the full amount this buffer only
					ssData.dataLength = pStream->streamLength1;
					ssData.dataBitSize = dataBitSize;
					ssData.channelSize = channelSize;
					ssData.sampleRate = sampleRate;
					DEBUG_STR("StreamSetup: Call callback with GET_DATA\n");

					theErr = (*pProc)(threadContext, STREAM_GET_DATA, &ssData);

#ifdef USE_DEBUG
					if( theErr) {
					    DEBUG_STR("StreamSetup: Return from 1st callback with an error\n");
					} else {
					    DEBUG_STR("StreamSetup: Return 1st from callback o.k.\n");
					}
#endif
					pStream->streamLength1 = ssData.dataLength;			// just in case it changes
					if (pStream->streamLength1 == 0)
					    {	// underflow, get this buffer again
						pStream->streamUnderflow = TRUE;

						// $$kk: 09.10.98: added these two lines.  need to PV_AddStream; otherwise AudioStreamStart 
						// will fail (cannot get stream from reference: PARAM_ERR).  we use STREAM_MODE_INACTIVE
						// to avoid trying to play sound in GM_AudioStreamService.  GM_AudioStreamStart will set
						// STREAM_MODE_DEAD; then we'll start trying to read data.
						pStream->streamMode = STREAM_MODE_INACTIVE;
						PV_AddStream(pStream);		// ok add stream
						
					    }
					else
					    {
						pStream->startupBufferFullCount++;	// first buffer full
						if ( (theErr == NO_ERR) || (theErr == STREAM_STOP_PLAY) )
						    {
							if (theErr == NO_ERR)
							    {
								// update count of samples written
								pStream->samplesWritten += pStream->streamLength1;

								if (ssData.dataLength < MAX_SAMPLE_OVERSAMPLE)
								    {
									ssData.dataLength += MAX_SAMPLE_OVERSAMPLE;		// going to click for sure
								    }
								else
								    {
									ssData.dataLength -= MAX_SAMPLE_OVERSAMPLE;
								    }
								// copy end of buffer 1 into the start of buffer 2
								PV_CopyLastSamplesToFirst((char *)pStream->pStreamData1, (char *)pStream->pStreamData2, &ssData);

								// ok, now fill second buffer
								ssData.userReference = pStream->userReference;
								ssData.streamReference = (STREAM_REFERENCE)pStream;

								// now, push second pointer out for oversampling, and get fewer bytes for this buffer
								ssData.pData = (char *)pStream->pStreamData2 + (PV_GetSampleSizeInBytes(&ssData) * MAX_SAMPLE_OVERSAMPLE);
								ssData.dataLength = pStream->streamLength2 - MAX_SAMPLE_OVERSAMPLE;

								theErr = (*pProc)(threadContext, STREAM_GET_DATA, &ssData);

								pStream->streamLength2 = ssData.dataLength;			// just in case it changes

								// update count of samples written
								pStream->samplesWritten += pStream->streamLength2;

								if (pStream->streamLength2 == 0)
								    {	// underflow, get this buffer again
									pStream->streamUnderflow = TRUE;
								    }
								else
								    {
									pStream->startupBufferFullCount++;	// second buffer full
								    }
							    }
							else
							    {
								pStream->streamLength2 = 0;
							    }

							pStream->startupStatus = theErr;
							theErr = NO_ERR;	// we don't want to return a stop fail here

							// Ok, start the sample playback
							pStream->streamData = ssData;
							PV_AddStream(pStream);		// ok add stream
						    }
					    }
					if ( (theErr != NO_ERR) && (theErr != STREAM_STOP_PLAY) )
					    {	// we've got to dispose of the data now
						ssData.userReference = pStream->userReference;
						ssData.streamReference = (STREAM_REFERENCE)pStream;
						ssData.pData = pStream->pStreamBuffer;
						ssData.dataLength = pStream->streamBufferLength;
						ssData.sampleRate = sampleRate;
						ssData.dataBitSize = dataBitSize;
						ssData.channelSize = channelSize;
						pStream->streamCallback = NULL;
						/* $$fb 2002-03-17: add resampling for streams */
						GM_SetStreamResample(pStream, FALSE);
						(*pProc)(threadContext, STREAM_DESTROY, &ssData);
					    }
				    }
			    }
		    }
		else
		    {
			theErr = PARAM_ERR;
		    }
	    }
	}
    else
	{
	    theErr = NO_FREE_VOICES;
	}

#ifdef USE_DEBUG
    if (theErr) {
	DEBUG_STR("Exiting stream create with an error.\n");
    } else {
	DEBUG_STR("Exiting stream create NO error.\n");
    }
#endif
    if (pStream)
	{
	    pStream->streamErr = theErr;
	}

    DEBUG_STR("< GM_AudioStreamSetup\n");
    return reference;
}


// $$kk: 07.01.99: added this method.
// this method tries to pre-fill the stream's buffers
// if they're empty.  my goal here is to allow us to
// pre-prepare buffers for a stream that didn't get data
// during the first two GET_DATA calls in GM_AudioStreamSetup,
// but does need to pre-load before starting playback.
// this also specifically should fix the problem i see
// where GM_StartLinkedStreams fails for me because my
// pStream->playbackReferences are DEAD_STREAM or whatever....
OPErr GM_AudioStreamPrebuffer(STREAM_REFERENCE reference, void *threadContext)
{
    GM_AudioStream		*pStream;
    GM_StreamData		ssData;
    GM_StreamObjectProc	theProc;

    XBOOL				done;
    OPErr				theErr = NO_ERR;



    pStream = PV_AudioStreamGetFromReference(reference);
    theProc = pStream->streamCallback;

    if ( (pStream) && (theProc) )
	{
	    done = GM_IsSoundDone(pStream->playbackReference);

	    if (!done)
		{
		    // this should only be applied to a dead voice.
		    theErr = STILL_PLAYING;
		}
	    else
		{
		    pStream->streamMode = STREAM_MODE_DEAD;
		    pStream->streamUnderflow = FALSE;

		    ssData.dataLength = pStream->streamOrgLength1;
		    ssData.pData = pStream->pStreamData1;
		    ssData.userReference = pStream->userReference;
		    ssData.streamReference = (STREAM_REFERENCE)pStream;

		    theErr = (*theProc)(threadContext, STREAM_GET_DATA, &ssData);

		    pStream->streamLength1 = ssData.dataLength;			// just in case it changes

		    if (pStream->streamLength1 == 0)
			{	// underflow, get this buffer again
			    pStream->streamUnderflow = TRUE;
			    pStream->streamMode = STREAM_MODE_INACTIVE;
			    theErr = NOT_READY;
			}
		    else
			{
			    pStream->startupBufferFullCount++;	// first buffer full
			    if ( (theErr == NO_ERR) || (theErr == STREAM_STOP_PLAY) )
				{
				    if (theErr == NO_ERR)
					{
					    // update count of samples written
					    pStream->samplesWritten += pStream->streamLength1;

					    if (ssData.dataLength < MAX_SAMPLE_OVERSAMPLE)
						{
						    ssData.dataLength += MAX_SAMPLE_OVERSAMPLE;		// going to click for sure
						}
					    else
						{
						    ssData.dataLength -= MAX_SAMPLE_OVERSAMPLE;
						}
					    // copy end of buffer 1 into the start of buffer 2
					    PV_CopyLastSamplesToFirst((char *)pStream->pStreamData1, (char *)pStream->pStreamData2, &ssData);

					    // ok, now fill second buffer
					    ssData.userReference = pStream->userReference;
					    ssData.streamReference = (STREAM_REFERENCE)pStream;

					    // now, push second pointer out for oversampling, and get fewer bytes for this buffer
					    ssData.pData = (char *)pStream->pStreamData2 + (PV_GetSampleSizeInBytes(&ssData) * MAX_SAMPLE_OVERSAMPLE);
					    ssData.dataLength = pStream->streamLength2 - MAX_SAMPLE_OVERSAMPLE;

					    theErr = (*theProc)(threadContext, STREAM_GET_DATA, &ssData);

					    pStream->streamLength2 = ssData.dataLength;			// just in case it changes

					    // update count of samples written
					    pStream->samplesWritten += pStream->streamLength2;

					    if (pStream->streamLength2 == 0)
						{	// underflow, get this buffer again
						    pStream->streamUnderflow = TRUE;
						}
					    else
						{
						    pStream->startupBufferFullCount++;	// second buffer full
						}
					}
				    else
					{
					    pStream->streamLength2 = 0;
					}

				    pStream->startupStatus = theErr;
				    theErr = NO_ERR;	// we don't want to return a stop fail here

				    // Ok, start the sample playback
				    pStream->streamData = ssData;
				}
			}
		}
	}
    else
	{
	    theErr = NOT_SETUP;
	}

    return theErr;
}


OPErr GM_AudioStreamPreroll(STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;
    OPErr				theErr;

    theErr = NO_ERR;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    // Ok, start the sample playback
	    pStream->streamActive = TRUE;
	    pStream->streamPaused = FALSE;
	    pStream->streamFirstTime = TRUE;

	    // $$kk: 09.23.98: added this ->

	    // $$kk: 11.10.99: removed this line
	    //pStream->startEvent.status = EVENT_PENDING;
	    if (pStream->startEvent.status != START_OF_MEDIA)
		{
		    pStream->startEvent.status = RESUME;
		}


	    // $$kk: 09.23.98: end changes <-


	    if (pStream->startupBufferFullCount == 0)
		{	// underflow on first buffer
		    pStream->streamMode = STREAM_MODE_DEAD;
		    //theErr = NOT_READY;
		}
	    else
		{	// everythings ok, or at least the first buffer has data
		    pStream->streamMode = STREAM_MODE_START_BUFFER_1;
		    if (PV_PrepareThisBufferForPlaying(pStream, STREAM_MODE_START_BUFFER_1) == FALSE)
			{
			    theErr = NOT_READY;
			}

		    //{
		    //	PV_StartStreamBuffers(pStream);
		    //}
		}
	    if (pStream->startupStatus == STREAM_STOP_PLAY)
		{
		    pStream->streamShuttingDown = TRUE;
		    if (pStream->startupBufferFullCount == 1)
			{
			    pStream->streamLength2 = 0;
			}
		}
	}
    else
	{
	    theErr = PARAM_ERR;
	}
    return theErr;
}

// set all the streams you want to start at the same time the same syncReference. Then call GM_SyncAudioStreamStart
// to start the sync start. Will return an error (not NO_ERR) if its an invalid reference, or syncReference is NULL.
OPErr GM_SetSyncAudioStreamReference(STREAM_REFERENCE reference, void *syncReference)
{
    GM_AudioStream		*pStream;
    OPErr				err;

    err = NO_ERR;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    err = GM_SetSyncSampleStartReference(pStream->playbackReference, syncReference);
	}
    return err;
}

// Once you have called GM_SetSyncAudioStreamReference on all the streams, this will set them to start at the next
// mixer slice. Will return an error (not NO_ERR) if its an invalid reference, or syncReference is NULL.
OPErr GM_SyncAudioStreamStart(STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;
    OPErr				err;

    err = NO_ERR;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    err = GM_SyncStartSample(pStream->playbackReference);
	}
    else
	{
	    err = NOT_SETUP;
	}
    return err;
}


OPErr GM_AudioStreamStart(STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;
    OPErr				theErr;

    theErr = NO_ERR;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (pStream->streamPrerolled == FALSE)
		{
		    theErr = GM_AudioStreamPreroll(reference);
		}
	    if (theErr == NO_ERR)
		{
		    PV_StartStreamBuffers(pStream);
		}
	}
    else
	{
	    theErr = PARAM_ERR;
	}
    return theErr;
}

void *GM_AudioStreamGetReference(STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;
    void *				userReference;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    userReference = pStream->userReference;
	}
    return userReference;
}

void GM_AudioStreamStopAll(void *threadContext)
{
    GM_AudioStream	*pStream;

    pStream = theStreams;
    while (pStream)
	{
	    if (pStream->streamActive)
		{
		    GM_AudioStreamStop(threadContext, (STREAM_REFERENCE)pStream);
		}
	    pStream = pStream->pNext;
	}
}

// This will stop a streaming audio object.
//
// INPUT:
//	This is the reference number returned from AudioStreamStart.
//
static OPErr PV_AudioStreamStopAndFreeNow(void *threadContext, STREAM_REFERENCE reference)
{
    GM_AudioStream		*pStream;
    GM_StreamData		ssData;
    short int			theErr;
    GM_StreamObjectProc	pProc;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (pStream->streamActive)
		{
		    // $$kk: 08.12.98 merge 
		    //  $$kk: 02.13.98: set playbackReference = DEAD_VOICE whenever we call GM_EndSample to avoid jmf stream crossing
		    VOICE_REFERENCE playbackReference = pStream->playbackReference;

		    pStream->streamLength1 = 0;
		    pStream->streamLength2 = 0;		// don't play next buffer.

		    pStream->playbackReference = DEAD_VOICE;
			
		    //$$fb 2002-04-20: apparently, this is only called in idle time of the main audio thread
		    // -> thread safe
		    GM_EndSample(playbackReference, threadContext);

		    pStream->streamActive = FALSE;
		    pStream->streamShuttingDown = FALSE;
		}
	    if (pStream->streamCallback)
		{
		    ssData = pStream->streamData;

		    ssData.userReference = pStream->userReference;
		    ssData.streamReference = (STREAM_REFERENCE)pStream;
		    ssData.pData = pStream->pStreamBuffer;
		    ssData.dataLength = pStream->streamBufferLength;
		    pProc = pStream->streamCallback;
		    pStream->streamCallback = NULL;		// prevent recursive callbacks
		    theErr = (*pProc)(threadContext, STREAM_DESTROY, &ssData);
		    pStream->userReference = 0;
		}
	    DEBUG_STR("Freeing Stream...\n");
	    PV_FreeStream(pStream);
	}
    DEBUG_STR("End of PV_AudioStreamStopAndFreeNow...\n");
    return NO_ERR;
}

// This will stop a streaming audio object, during the next GM_AudioServiceStream
//
// INPUT:
//	This is the reference number returned from AudioStreamStart.
//
// $$kk: 08.12.98 merge: changed this function 
OPErr GM_AudioStreamStop(void *threadContext, STREAM_REFERENCE reference)
{
    /*
      PV_AudioStreamStopAndFreeNow(threadContext, reference);
*/
/*
	GM_AudioStream		*pStream;

	pStream = PV_AudioStreamGetFromReference(reference);
	if (pStream)
	{
		// Set various flags to iniate a shutdown of the stream from inside GM_AudioStreamService
		pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_STOP_STREAM;		// end

		// if audio data is already dead, then
		if (GM_IsSoundDone(pStream->playbackReference))
		{	// we've already flushed the audio data, so just free it now
			PV_AudioStreamStopAndFreeNow(threadContext, (long)pStream);
			pStream->streamMode = STREAM_MODE_DEAD;
		}
	}
*/

// $$kk: 01.06.98
// want stream to stay in engine until all samples played.
// we make it stop playing and mark it to be freed; it should not actually
// get freed until all samples have been played.
//	PV_AudioStreamStopAndFreeNow(threadContext, reference);

    GM_AudioStream		*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);

    if (pStream)
	{

	    // $$kk: 05.20.98: if the stream is paused, it'll never get shut down properly in
	    // GM_AudioStreamService.  need to set up the state so that it can shut down.
	    if (pStream->streamPaused)
		{

		    // $$kk: 11.10.99: avoid sending any sort of start event after the 
		    // GM_AudioStreamResume call below
		    pStream->startEvent.status = RESOLVED;

		    //PV_AudioStreamStopAndFreeNow(threadContext, reference);
		    //return NO_ERR;
		    GM_AudioStreamFlush(reference);
		    GM_AudioStreamResume(reference);
		}
		
	    if (pStream->streamActive)
		{
		    //  $$kk: 02.13.98: set playbackReference = DEAD_VOICE whenever we call GM_EndSample to avoid jmf stream crossing
		    VOICE_REFERENCE playbackReference = pStream->playbackReference;

#ifdef USE_DEBUG
		    printf("GenAudioStreams:GM_AudioStreamStop: "
			   "playbackReference set to NULL\n"); 
		    fflush(stdout); 
#endif
		    // stop current voice
		    pStream->playbackReference = DEAD_VOICE;

		    pStream->streamLength1 = 0;
		    pStream->streamLength2 = 0;		// don't play next buffer.

		    //$$fb 2002-04-20: use thread safe version of GM_EndSample
		    GM_ReleaseSample(playbackReference, threadContext);
		}
		
	    if (pStream->active) 
		{
		    pStream->stopEvent.status = PAUSE;		
		}
	    else
		{
		    pStream->stopEvent.status = RESOLVED;
		}

	    pStream->streamShuttingDown = TRUE;
	    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_FREE_STREAM;		// end

	    // $$kk: 11.03.98: added this  ->
	    if (pStream->samplesWritten > 0)
		{

		    // $$kk: 09.23.98: added this  ->

		    // $$kk: 11.10.99
		    //pStream->stopEvent.status = EVENT_PENDING;
		    //pStream->stopEvent.status = END_OF_MEDIA;

		    // $$kk: 09.23.98: end changes <-
		} else 
		    {
			// $$kk: 11.03.98: need to handle the case where a stream was started,
			// but no data was ever delivered to it.  in this case, we must mark
			// the start and stop events, which will never be generated, resolved.
			
			// $$kk: 11.10.99
			//pStream->startEvent.status = EVENT_RESOLVED;
			//pStream->stopEvent.status = EVENT_RESOLVED;
			pStream->startEvent.status = RESOLVED;
			pStream->stopEvent.status = RESOLVED;
		    }
	    // $$kk: 11.03.98: end changes <-

	}
    return NO_ERR;
}

// Get the file position of a audio stream, in samples
SAMPLE_COUNT GM_AudioStreamGetFileSamplePosition(STREAM_REFERENCE reference)
{
    GM_AudioStreamFileInfo	*pInfo;
    GM_AudioStream			*pStream;
    SAMPLE_COUNT			samplePosition;
    short int				blockAlign;

    samplePosition = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (pStream->streamActive)
		{
		    blockAlign = PV_GetSampleSizeInBytes(&pStream->streamData);
		    pInfo = pStream->pFileStream;
		    if (pInfo)
			{
			    samplePosition = pInfo->filePlaybackPosition / blockAlign;	// convert from bytes to samples
			}
		    else
			{
			    samplePosition = pStream->streamPlaybackPosition;	// no conversion because its in samples
			}
		    // now that we have the stream position, add in the playback of the current chunk of stream
		    // for an actual position
		    samplePosition += GM_GetSamplePlaybackPosition(pStream->playbackReference);
		}
	}
    return samplePosition;
}


OPErr GM_AudioStreamGetData(void *threadContext, STREAM_REFERENCE reference, UINT32 startFrame, UINT32 stopFrame,
			    XPTR pBuffer, UINT32 bufferLength)
{
    GM_AudioStream	*pStream;
    OPErr			theErr;
    GM_StreamData	ssData;

    theErr = NOT_SETUP;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream && pBuffer)
	{
	    if (pStream->streamCallback)
		{
		    ssData = pStream->streamData;
		    ssData.dataLength = bufferLength / ssData.channelSize / (ssData.dataBitSize / 8);
		    ssData.pData = (char *)pBuffer;
		    ssData.userReference = pStream->userReference;
		    ssData.streamReference = (STREAM_REFERENCE)pStream;
		    ssData.startSample = startFrame;
		    ssData.endSample = stopFrame;
		    theErr = (*pStream->streamCallback)(threadContext, STREAM_GET_SPECIFIC_DATA, &ssData);
		}
	}
    return theErr;
}


// Set the stereo position of a audio stream
void GM_AudioStreamSetStereoPosition(STREAM_REFERENCE reference, short int stereoPosition)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamStereoPosition = stereoPosition;
	    GM_ChangeSampleStereoPosition(pStream->playbackReference, stereoPosition);
	}
}

// Get the stereo position of a audio stream
short int GM_AudioStreamGetStereoPosition(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    short int		stereoPosition;

    stereoPosition = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    stereoPosition = pStream->streamStereoPosition;
	}
    return stereoPosition;
}

// Get the playback offset in samples for the stream.
// This is the offset between the number of samples processed
// by the mixer and the number of samples processed from this stream.
SAMPLE_COUNT GM_AudioStreamGetSampleOffset(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    SAMPLE_COUNT offset = 0;

	
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    offset = pStream->streamPlaybackOffset;
	}
    return offset;
}

// Get the engine's count of samples from this stream actually
// played through the device.
SAMPLE_COUNT GM_AudioStreamGetSamplesPlayed(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    SAMPLE_COUNT samplesPlayed = 0;


    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    samplesPlayed = pStream->samplesPlayed;
	}

    return samplesPlayed;
}

// $$kk: 08.12.98 merge: added this method
// Drain this stream
void GM_AudioStreamDrain(void *threadContext, STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    SAMPLE_COUNT	samplesWritten;

    // get the samples written.
    // we have to drain until samples played reaches this value.
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (pStream->streamActive)
		{
		    samplesWritten = pStream->samplesWritten;
		}
	    else
		{
		    return;
		}
	}
    else
	{
	    return;
	}

    //printf("samplesWritten = %d\n", samplesWritten);

    while(1)
	{
	    pStream = PV_AudioStreamGetFromReference(reference);
	    if (pStream)
		{
		    if (pStream->streamActive)
			{
			    SAMPLE_COUNT newSamplesWritten;

			    newSamplesWritten = pStream->samplesWritten;
			    if (newSamplesWritten < samplesWritten)
				{
				    samplesWritten =  newSamplesWritten;
				}

			    if (pStream->samplesPlayed >= samplesWritten)
				{
				    // done!
				    //printf("GM_AudioStreamDrain: played all samples\n");
				    return;
				}
			}
		    else
			{
				// stream no longer active
				//printf("GM_AudioStreamDrain: stream no longer active\n");
			    return;
			}
		}
	    else
		{
		    // stream no longer valid
		    //printf("GM_AudioStreamDrain: stream no longer valid\n");
		    return;
		}

	    // if we get here, the stream is still valid and active and has samples still to play.
	    // wait 10 milliseconds
	    GM_AudioStreamService(threadContext);
	    XWaitMicroseocnds(10000);
	}
}



// Flush this stream
// $$kk: 08.12.98 merge: changed to avoid stream crossing
void GM_AudioStreamFlush(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (pStream->streamActive)
		{
		    //  $$kk: 02.13.98: set playbackReference = DEAD_VOICE whenever we call GM_EndSample to avoid jmf stream crossing
		    VOICE_REFERENCE playbackReference = pStream->playbackReference;

#ifdef USE_DEBUG
		    printf("GenAudioStreams:GM_AudioStreamFlush. "
			   "playbackReference set to NULL\n"); 
		    fflush(stdout); 
#endif
		    // stop current voice
		    pStream->playbackReference = DEAD_VOICE;

		    //$$fb 2002-04-20: use thread safe version of GM_EndSample
		    GM_ReleaseSample(playbackReference, NULL);

		    // set to a dead voice, which will force a new complete buffer read before starting next buffer

		    if (pStream->streamShuttingDown == TRUE)
			{
			    pStream->streamMode = STREAM_MODE_STOP_STREAM;
			}

		    else
			{
			    pStream->streamMode = STREAM_MODE_DEAD;
			}

		    pStream->streamUnderflow = TRUE;	// induce underflow
		    pStream->streamFlushed = TRUE;

		    // here we reduce the count of samples written to the number which we will
		    // actually end up playing.  (the difference between the old and new values
		    // is the amount of data flushed!)
		    pStream->samplesWritten = GM_AudioStreamGetFileSamplePosition(reference);
		}
	}
}


// Set the volume level of a audio stream
void GM_AudioStreamSetVolume(STREAM_REFERENCE reference, short int newVolume, XBOOL defer)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamVolume = newVolume;
	    if (defer == FALSE)
		{
		    GM_ChangeSampleVolume(pStream->playbackReference, newVolume);
		}
	}
}

// Get the volume level of a audio stream
short int GM_AudioStreamGetVolume(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    short int		volume;

    volume = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    volume = pStream->streamVolume;
	}
    return volume;
}

// set the volume level of all open streams. Scale is 0 to MAX_NOTE_VOLUME. If you pass
// -1, then it will reset all volumes. This is used to grab the master volume changes.
void GM_AudioStreamSetVolumeAll(short int newVolume)
{
    GM_AudioStream	*pStream;
    short int		thisVolume;

    pStream = theStreams;
    while (pStream)
	{
	    if (newVolume == -1)
		{
		    thisVolume = GM_GetSampleVolumeUnscaled(pStream->playbackReference);
		}
	    else
		{
		    thisVolume = newVolume;
		}
	    pStream->streamVolume = thisVolume;
	    GM_ChangeSampleVolume(pStream->playbackReference, thisVolume);
	    pStream = pStream->pNext;
	}
}


// Set stream fade rate. Its a 16.16 fixed value
// Input:	reference	stream to affect
//			fadeRate	amount to change every 11 ms
//						example:	FLOAT_TO_XFIXED(2.2) will decrease volume
//									FLOAT_TO_XFIXED(2.2) * DEAD_VOICE will increase volume
//			minVolume	lowest volume level fade will go
//			maxVolume	highest volume level fade will go
void GM_SetAudioStreamFadeRate(STREAM_REFERENCE reference, XFIXED fadeRate, 
			       INT16 minVolume, INT16 maxVolume, XBOOL endStream)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamFixedVolume = LONG_TO_XFIXED(pStream->streamVolume);
	    pStream->streamFadeMaxVolume = maxVolume;
	    pStream->streamFadeMinVolume = minVolume;
	    pStream->streamEndAtFade = endStream;
	    pStream->streamFadeRate = fadeRate;

	}
}

// Enable/Disable reverb on this particular audio stream
void GM_AudioStreamReverb(STREAM_REFERENCE reference, XBOOL useReverb)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamUseReverb = useReverb;
	    GM_ChangeSampleReverb(pStream->playbackReference, useReverb);
	}
}

XBOOL GM_AudioStreamGetReverb(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    XBOOL			verb;

    verb = FALSE;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    verb = pStream->streamUseReverb;
	}
    return verb;
}

void GM_SetStreamReverbAmount(STREAM_REFERENCE reference, short int reverbAmount)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamReverbAmount = reverbAmount;
	    GM_SetSampleReverbAmount(pStream->playbackReference, reverbAmount);
	}
}

short int GM_GetStreamReverbAmount(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    short int		verbAmount;

    verbAmount = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    verbAmount = pStream->streamReverbAmount;
	}
    return verbAmount;
}

// Set the filter frequency of a audio stream
// Range is 512 to 32512
void GM_AudioStreamSetFrequencyFilter(STREAM_REFERENCE reference, short int frequency)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamFrequencyFilter = frequency;
	    GM_SetSampleFrequencyFilter(pStream->playbackReference, frequency);
	}
}

// Get the filter frequency of a audio stream
// Range is 512 to 32512
short int GM_AudioStreamGetFrequencyFilter(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    short int		frequency;

    frequency = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    frequency = pStream->streamFrequencyFilter;
	}
    return frequency;
}

// Set the filter resonance of a audio stream
// Range is 0 to 256
void GM_AudioStreamSetResonanceFilter(STREAM_REFERENCE reference, short int resonance)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamResonanceFilter = resonance;
	    GM_SetSampleResonanceFilter(pStream->playbackReference, resonance);
	}
}

// Get the filter resonance of a audio stream
// Range is 0 to 256
short int GM_AudioStreamGetResonanceFilter(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    short int		resonance;

    resonance = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    resonance = pStream->streamResonanceFilter;
	}
    return resonance;
}

// get/set filter low pass amount of a audio stream
// lowPassAmount range is -255 to 255
short int GM_AudioStreamGetLowPassAmountFilter(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    short int		lowpass;

    lowpass = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    lowpass = pStream->streamLowPassAmountFilter;
	}
    return lowpass;
}

// lowPassAmount range is -255 to 255
void GM_AudioStreamSetLowPassAmountFilter(STREAM_REFERENCE reference, short int lowPassAmount)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamLowPassAmountFilter = lowPassAmount;
	    GM_SetSampleLowPassAmountFilter(pStream->playbackReference, lowPassAmount);
	}
}

// Set the sample rate of a audio stream
void GM_AudioStreamSetRate(STREAM_REFERENCE reference, XFIXED newRate)
{
    GM_AudioStream	*pStream;

    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    pStream->streamData.sampleRate = newRate;
	    GM_ChangeSamplePitch(pStream->playbackReference, newRate);
	}
}


// Get the sample rate of a audio stream
XFIXED GM_AudioStreamGetRate(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    XFIXED			rate;

    rate = 0;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    rate = pStream->streamData.sampleRate;
	}
    return rate;
}

// Returns TRUE or FALSE if a given GM_AudioStream is still active
XBOOL GM_IsAudioStreamPlaying(STREAM_REFERENCE reference)
{
    GM_AudioStream	*pStream;
    XBOOL			active;


    active = FALSE;
    pStream = PV_AudioStreamGetFromReference(reference);
    if (pStream)
	{
	    if (GM_IsSoundDone(pStream->playbackReference) == FALSE)
		{
		    if (pStream->streamActive)
			{
			    active = TRUE;
			}
		}
	}
    return active;
}

// Returns TRUE or FALSE if a given reference is still valid    
// $$kk: 08.12.98 merge: changed to avoid stream crossing
XBOOL GM_IsAudioStreamValid(STREAM_REFERENCE reference)
{
    return (PV_AudioStreamGetFromReference(reference)) ? TRUE : FALSE;
}

// Process any fading streams
void PV_ServeStreamFades(void)
{
    GM_AudioStream	*pStream;
    INT32			value;

    pStream = theStreams;
    while (pStream)
	{
	    if ((pStream->streamActive) && (pStream->streamPaused == FALSE))
		{
		    if (pStream->streamFadeRate)
			{
			    pStream->streamFixedVolume -= pStream->streamFadeRate;
			    value = XFIXED_TO_LONG(pStream->streamFixedVolume);
			    if (value > pStream->streamFadeMaxVolume)
				{
				    value = pStream->streamFadeMaxVolume;
				    pStream->streamFadeRate = 0;
				}
			    if (value < pStream->streamFadeMinVolume)
				{
				    value = pStream->streamFadeMinVolume;
				    pStream->streamFadeRate = 0;
				}
			    pStream->streamVolume = (short)value;

			    GM_ChangeSampleVolume(pStream->playbackReference, (INT16)value);

			    if ((pStream->streamFadeRate == 0) && pStream->streamEndAtFade)
				{
				    //  $$kk: 02.13.98: set playbackReference = DEAD_VOICE whenever we call GM_EndSample to avoid jmf stream crossing
				    VOICE_REFERENCE playbackReference = pStream->playbackReference;

				    pStream->playbackReference = DEAD_VOICE;
					
				    //$$fb 2002-04-20: here it is always in main audio thread
				    // -> safe to use GM_EndSample
				    GM_EndSample(playbackReference, NULL);

				    pStream->streamMode = STREAM_MODE_INTERRUPT_ACTIVE | STREAM_MODE_FREE_STREAM;		// end
				}
			}
		}
	    pStream = pStream->pNext;
	}
}


// This is the streaming audio service routine. Call this as much as possible, but not during an
// interrupt. This is a very quick routine. A good place to call this is in your main event loop.
// $$kk: 08.12.98 merge: changed this method to allow streams to exist in the engine until all samples played
void GM_AudioStreamService(void *threadContext) {
    GM_AudioStream		*pStream, *pNext;
    GM_StreamData		ssData;
    GM_StreamObjectProc	theProc;
    XBOOL				done;
    OPErr				theErr;

    pStream = theStreams;
    while (pStream) {

	// $$kk: 09.23.98: added this block ->

	//if (pStream->startEvent.status == EVENT_DETECTED)
	if ( (pStream->startEvent.status != RESOLVED) && (pStream->startEvent.detected == TRUE) ) {			
	    pStream->startEvent.detected = FALSE;
	    theProc = pStream->streamCallback;

	    if (theProc) {
		ssData = pStream->streamData;
		// do i need the rest of this ssData setup??
		ssData.userReference = pStream->userReference;
		ssData.startSample = pStream->startEvent.framePosition;
				
		if ((pStream->startEvent.status == START_OF_MEDIA) || (pStream->startEvent.status == RESUME)) {
		    theErr = (*theProc)(threadContext, STREAM_START, &ssData);
		} else {
		    theErr = (*theProc)(threadContext, STREAM_ACTIVE, &ssData);
		}
	    }

	    pStream->startEvent.status = ACTIVE;
	}

	//if (pStream->stopEvent.status == EVENT_DETECTED)
	if ( (pStream->stopEvent.status != RESOLVED) && (pStream->stopEvent.detected == TRUE) ) {
	    pStream->stopEvent.detected = FALSE;
	    //pStream->stopEvent.status = EVENT_RESOLVED;
	    theProc = pStream->streamCallback;

	    if (theProc) {
		ssData = pStream->streamData;
				// do i need the rest of this ssData setup??
		ssData.userReference = pStream->userReference;
		ssData.startSample = pStream->stopEvent.framePosition;
		if (pStream->startupStatus == STREAM_STOP_PLAY) {
		    theErr = (*theProc)(threadContext, STREAM_EOM, &ssData);
		    pStream->startEvent.status = RESOLVED;
		    pStream->stopEvent.status = RESOLVED;
		} else {
		    if (pStream->stopEvent.status == PAUSE) {
			theErr = (*theProc)(threadContext, STREAM_STOP, &ssData);
		    } else {
			theErr = (*theProc)(threadContext, STREAM_INACTIVE, &ssData);
		    }
		}
	    }

	    if (pStream->streamShuttingDown == TRUE) {
		pStream->stopEvent.status = RESOLVED;
	    } else {
		pStream->stopEvent.status = INACTIVE;
	    }
	}
	// $$kk: 09.23.98: end changes <-


	pNext = pStream->pNext;
	if ((pStream->streamActive) && (pStream->streamPaused == FALSE)) {
	    done = GM_IsSoundDone(pStream->playbackReference);
	    // voice has shutdown, either from a voice setup change or some other problem, restart
	    // the stream from the last place that we're aware of.
	    if ( (done) && (pStream->streamShuttingDown == FALSE) ) {
		pStream->streamMode = STREAM_MODE_DEAD;
	    }
	}

	if ( ((pStream->streamActive) &&
	      (pStream->streamMode & STREAM_MODE_INTERRUPT_ACTIVE) &&
	      (pStream->streamPaused == FALSE)) ||
	     // $$kk: added check for pStream->streamPaused == FALSE
	     (pStream->streamUnderflow && (pStream->streamPaused == FALSE))) {
	    theProc = pStream->streamCallback;
			
	    if (theProc) {
		pStream->streamMode &= (~STREAM_MODE_INTERRUPT_ACTIVE);
		ssData = pStream->streamData;
		switch (pStream->streamMode) {
		default:
		    DEBUG_STR("Bad case in GM_AudioStreamService");
		    break;
		case STREAM_MODE_FREE_STREAM:
		    DEBUG_STR("GM_AudioStreamService::STREAM_MODE_FREE_STREAM");

		    // $$kk: 02.06.98
		    // stream should stay in engine until all samples played.

		    //printf("GM_AudioStreamService::STREAM_MODE_FREE_STREAM %d\n", pStream);
		    //printf("pStream->samplesPlayed: %d, sample position: %d \n", pStream->samplesPlayed, GM_AudioStreamGetFileSamplePosition((long)pStream));

		    // figure out whether all the samples have been played out through the device
		    // before freeing the stream

		    // $$kk: 09.23.98: changed this ->
		    // not sure if this is ok!!
		    //						if (pStream->samplesPlayed >= GM_AudioStreamGetFileSamplePosition((long)pStream))
		    //if (pStream->stopEvent.status == EVENT_RESOLVED)

		    // $$kk: 11.10.99
		    if (pStream->stopEvent.status == RESOLVED) {

			// $$kk: 09.23.98: end changes <-
			// This will free the stream
			//printf("freeing stream %d\n", pStream);
			PV_AudioStreamStopAndFreeNow(threadContext, (STREAM_REFERENCE)pStream);
		    } else {
			// keep coming back through this loop to check again
			DEBUG_STR("setting stream mode to INTERRUPT_ACTIVE\n");
			pStream->streamMode |= (STREAM_MODE_INTERRUPT_ACTIVE);
		    }

		    //						// This will free the stream
		    //						PV_AudioStreamStopAndFreeNow(threadContext, (long)pStream);
		    break;
		case STREAM_MODE_STOP_STREAM:

		    // $$kk: 02.06.98
		    // stream should stay in engine until all samples played.

		    DEBUG_STR("GM_AudioStreamService::STREAM_MODE_STOP_STREAM\n");
		    GM_AudioStreamStop(threadContext, (STREAM_REFERENCE)pStream);
		    break;
		case STREAM_MODE_DEAD:	// first buffer failed to fill
		    DEBUG_STR("GM_AudioStreamService::STREAM_MODE_DEAD");
		    ssData.dataLength = pStream->streamOrgLength1;
		    ssData.pData = pStream->pStreamData1;
		    ssData.userReference = pStream->userReference;
		    ssData.streamReference = (STREAM_REFERENCE)pStream;

		    // $$kk: 09.23.98: changed this ->
		    //						if ((*theProc)(threadContext, STREAM_GET_DATA, &ssData) != NO_ERR)
		    pStream->startupStatus = (*theProc)(threadContext, STREAM_GET_DATA, &ssData);
		    // $$kk: 09.23.98: end changes <-
		    if (pStream->startupStatus != NO_ERR) {
			DEBUG_STR("    STOP!");
			pStream->streamShuttingDown = TRUE;
			pStream->streamLength2 = 0;
			PV_FillBufferEndWithSilence((char *)ssData.pData, &ssData);
		    }
		    pStream->streamLength1 = ssData.dataLength;			// just in case it changes
		    if (ssData.dataLength < MAX_SAMPLE_OVERSAMPLE) {
			ssData.dataLength += MAX_SAMPLE_OVERSAMPLE;		// going to click for sure
		    } else {
			ssData.dataLength -= MAX_SAMPLE_OVERSAMPLE;
		    }
		    // copy end of buffer 1 into the start of buffer 2
		    PV_CopyLastSamplesToFirst((char *)pStream->pStreamData1, (char *)pStream->pStreamData2, &ssData);

		    if (pStream->streamLength1 == 0) {
			// underflow, get this buffer again
			pStream->streamUnderflow = TRUE;
		    } else {

			// $$kk: added this
			// update count of samples written
			pStream->samplesWritten += pStream->streamLength1;

			// ok, now fill second buffer
			ssData.userReference = pStream->userReference;
			ssData.streamReference = (STREAM_REFERENCE)pStream;

			// now, push second pointer out for oversampling, and get fewer bytes for this buffer
			ssData.pData = (char *)pStream->pStreamData2 + (PV_GetSampleSizeInBytes(&ssData) * MAX_SAMPLE_OVERSAMPLE);
			ssData.dataLength = pStream->streamOrgLength2 - MAX_SAMPLE_OVERSAMPLE;

			theErr = (*theProc)(threadContext, STREAM_GET_DATA, &ssData);

			// $$kk: 09.23.98: added this ->
			pStream->startupStatus = theErr;
			// $$kk: 09.23.98: end changes <-

			// $$kk: 11.03.98: note that if we return STREAM_STOP_PLAY here and never again, the stream does *not* shut down!!
			// this means that short files with only one buffer worth of data are hosed here.

			pStream->streamLength2 = ssData.dataLength;			// just in case it changes
			if (pStream->streamLength2 == 0) {
			    // underflow, get this buffer again
			    pStream->streamUnderflow = TRUE;
			} else {
			    // $$kk: added this
			    // update count of samples written
			    pStream->samplesWritten += pStream->streamLength2;

			    pStream->streamUnderflow = FALSE;
			}

			pStream->streamMode = STREAM_MODE_START_BUFFER_1;
			done = GM_IsSoundDone(pStream->playbackReference);
			if (done) {
			    if (PV_PrepareThisBufferForPlaying(pStream, STREAM_MODE_START_BUFFER_1)) {
				PV_StartStreamBuffers(pStream);
			    }
			}
			pStream->streamData = ssData;
		    }
		    break;
		case STREAM_MODE_START_BUFFER_2:		// read buffer 1 into memory
		    DEBUG_STR("GM_AudioStreamService::STREAM_MODE_START_BUFFER_2");

		    // ivg: there's a race condition when GM_AudioStreamFlush is being called.
		    // There's no real way to avoid other than to do the proper thread synchronization.
		    // I'm putting a extra check here for streamFlushed.  This made the race condition
		    // more difficult to reproduce.  But it's still there.

		    if (pStream->streamShuttingDown == FALSE && pStream->streamFlushed == FALSE) {
			ssData.dataLength = pStream->streamOrgLength1 - MAX_SAMPLE_OVERSAMPLE;
			ssData.pData = (char *)pStream->pStreamData1 + (PV_GetSampleSizeInBytes(&ssData) * MAX_SAMPLE_OVERSAMPLE);
			ssData.userReference = pStream->userReference;
			ssData.streamReference = (STREAM_REFERENCE)pStream;

			// $$kk: 09.23.98: changed this ->
			//							if ((*theProc)(threadContext, STREAM_GET_DATA, &ssData) != NO_ERR)
							
			pStream->startupStatus = (*theProc)(threadContext, STREAM_GET_DATA, &ssData);														
			// $$kk: 09.23.98: end changes <-
			if (pStream->startupStatus != NO_ERR) {
			    DEBUG_STR("    STOP!");
			    pStream->streamShuttingDown = TRUE;
			    pStream->streamLength2 = 0;
			    PV_FillBufferEndWithSilence((char *)ssData.pData, &ssData);
			}
			pStream->streamLength1 = ssData.dataLength;			// just in case it changes
			// copy end of buffer 1 into the start of buffer 2
			PV_CopyLastSamplesToFirst((char *)pStream->pStreamData1, (char *)pStream->pStreamData2, &ssData);

			if (pStream->streamLength1 == 0) {
			    // underflow, get this buffer again
			    pStream->streamUnderflow = TRUE;
			} else {
			    // update count of samples written
			    pStream->samplesWritten += pStream->streamLength1;

			    // did our last buffer underflow?
			    if (pStream->streamUnderflow) {
				done = GM_IsSoundDone(pStream->playbackReference);
				// voice has shutdown from running out of data, restart previous buffer.
				if (done && pStream->streamFlushed == FALSE) {
				    if (PV_PrepareThisBufferForPlaying(pStream, STREAM_MODE_START_BUFFER_1)) {
					PV_StartStreamBuffers(pStream);
				    }
				}
			    }
			    if (pStream->streamFlushed == FALSE)
				pStream->streamUnderflow = FALSE;
			}
		    }
#ifdef USE_DEBUG
		    printf("B1-> %ld len %ld", 
			   pStream->streamPlaybackPosition, 
			   pStream->streamLength1);
		    fflush(stdout);
#endif
		    break;
		case STREAM_MODE_START_BUFFER_1:		// read buffer 2 into memory
		    DEBUG_STR("GM_AudioStreamService::STREAM_MODE_START_BUFFER_1");

		    // ivg: there's a race condition when GM_AudioStreamFlush is being called.
		    // There's no real way to avoid other than to do the proper thread synchronization.
		    // I'm putting a extra check here for streamFlushed.  This made the race condition
		    // more difficult to reproduce.  But it's still there.

		    if (pStream->streamShuttingDown == FALSE && pStream->streamFlushed == FALSE) {
			ssData.dataLength = pStream->streamOrgLength2 - MAX_SAMPLE_OVERSAMPLE;
			ssData.pData = (char *)pStream->pStreamData2 + (PV_GetSampleSizeInBytes(&ssData) * MAX_SAMPLE_OVERSAMPLE);
			ssData.userReference = pStream->userReference;
			ssData.streamReference = (STREAM_REFERENCE)pStream;
			// $$kk: 09.23.98: changed this ->
			//							if ((*theProc)(threadContext, STREAM_GET_DATA, &ssData) != NO_ERR)
			pStream->startupStatus = (*theProc)(threadContext, STREAM_GET_DATA, &ssData);
							
			// $$kk: 09.23.98: end changes <-
			if (pStream->startupStatus != NO_ERR) {
			    DEBUG_STR("    STOP!");
			    pStream->streamShuttingDown = TRUE;
			    pStream->streamLength1 = 0;
			    PV_FillBufferEndWithSilence((char *)ssData.pData, &ssData);
			}
			pStream->streamLength2 = ssData.dataLength;
			// copy end of buffer 2 into the start of buffer 1
			PV_CopyLastSamplesToFirst((char *)pStream->pStreamData2, (char *)pStream->pStreamData1, &ssData);

			if (pStream->streamLength2 == 0) {
			    // underflow, get this buffer again
			    pStream->streamUnderflow = TRUE;
			} else {
			    // update count of samples written
			    pStream->samplesWritten += pStream->streamLength2;

			    // did our last buffer underflow?
			    if (pStream->streamUnderflow) {
				done = GM_IsSoundDone(pStream->playbackReference);
				// voice has shutdown from running out of data, restart previous buffer.
				if (done && pStream->streamFlushed == FALSE) {
				    if (PV_PrepareThisBufferForPlaying(pStream, STREAM_MODE_START_BUFFER_2)) {
					PV_StartStreamBuffers(pStream);
				    }
				}
			    }
			    if (pStream->streamFlushed == FALSE)
				pStream->streamUnderflow = FALSE;
			}
		    }
#ifdef USE_DEBUG
		    printf("B2-> %ld len %ld", 
			   pStream->streamPlaybackPosition, 
			   pStream->streamLength2);
		    fflush(stdout);
#endif
		    break;
		}
	    }
	}
	pStream = pNext;
    }
}


// update number of samples played for each stream
// delta is number of samples engine advanced, in its format
// $$kk: 08.12.98 merge: changed this method to allow streams to exist in the engine until all samples played
void GM_AudioStreamUpdateSamplesPlayed(UINT32 delta) 
{
    GM_AudioStream		*pStream, *pNext;

    UINT32		outputSampleRate;		// mixer sample rate
    UINT32		streamSampleRate;		// stream sample rate

    UINT32		streamDelta;			// samples played delta in stream-format samples
    SAMPLE_COUNT		samplesCommitted = 0;	// number of samples from the stream processed by the mixer

    INT32 mixerLatency = MusicGlobals->samplesWritten - MusicGlobals->samplesPlayed;
    INT32 streamLatency;
    SAMPLE_COUNT_SIGNED minStreamSamplesPlayed;

    // $$ay: Safeguard when MusicGlobals->samplesWritten wraps around to 0
    if (mixerLatency < 0)
	mixerLatency = 0;

    pStream = theStreams;

    while (pStream)
	{

	    pNext = pStream->pNext;

	    // convert from engine samples to stream samples
	    outputSampleRate = GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality);
	    streamSampleRate = XFIXED_TO_LONG_ROUNDED(pStream->streamData.sampleRate);
	    streamDelta = delta * streamSampleRate / outputSampleRate;
	    streamLatency = mixerLatency * streamSampleRate / outputSampleRate;

	    // $$kk: we can't count higher than the number of samples processed by the engine for this stream
	    samplesCommitted = GM_AudioStreamGetFileSamplePosition((STREAM_REFERENCE)pStream);


	    // $$kk: 03.05.98: at the end of the stream, GM_GetSamplePlaybackPosition remains at its last (highest) value even though
	    // pStream->streamPlaybackPosition already equals samplesWritten, so GM_GetSamplePlaybackPosition returns an impossibly high
	    // value.  make sure we don't use it....
	    if (samplesCommitted > pStream->samplesWritten)
		{
		    samplesCommitted = pStream->samplesWritten;
		}


	    // this is the minimum number of samples played from the stream.
	    // if the stream is finising up or underflowing, the count may be higher.
	    minStreamSamplesPlayed = samplesCommitted - streamLatency;

	    // normal case: stream is playing
	    if ((minStreamSamplesPlayed > (SAMPLE_COUNT_SIGNED) pStream->samplesPlayed) && (minStreamSamplesPlayed > 0))
		{			
		    if ( ! (pStream->active))
			{				
			    pStream->active = TRUE;
			    pStream->startEvent.detected = TRUE;
			    pStream->startEvent.framePosition = pStream->samplesPlayed;
			}
		    pStream->samplesPlayed = minStreamSamplesPlayed;
		}
		
	    // underflow case
	    else
		{
		    // if we are already as high as we can go, just check for 
		    // sending a stop event
		    if (pStream->samplesPlayed >= samplesCommitted)
			{		
			    if (pStream->active)
				{					
				    pStream->active = FALSE;
				    pStream->stopEvent.detected = TRUE;
				    pStream->stopEvent.framePosition = pStream->samplesPlayed;
				}
			}



		    // otherwise, increment and check for a start event
		    else
			{			
			    minStreamSamplesPlayed = pStream->samplesPlayed + streamDelta;
			    if (minStreamSamplesPlayed > 0)
				{				
				    if ( ! (pStream->active))
					{					
					    pStream->active = TRUE;
					    pStream->startEvent.detected = TRUE;
					    pStream->startEvent.framePosition = pStream->samplesPlayed;
					}


				    if (minStreamSamplesPlayed > (SAMPLE_COUNT_SIGNED) samplesCommitted)
					{
					    minStreamSamplesPlayed = samplesCommitted;
					}
				    pStream->samplesPlayed = minStreamSamplesPlayed;
				}
			}
		}

	    pStream = pNext; 
	} // while
}



// $$kk: 04.29.99: i changed GM_AudioStreamUpdateSamplesPlayed so radically that i just commented
// out the old version and wrote the new one above.

/*
  // update number of samples played for each stream
  // delta is number of samples engine advanced, in its format
  // $$kk: 08.12.98 merge: changed this method to allow streams to exist in the engine until all samples played
  void GM_AudioStreamUpdateSamplesPlayed(unsigned long delta) 
  {
	GM_AudioStream		*pStream, *pNext;
    unsigned long		outputSampleRate;

	//$$kk: 05.08.98: adding this variable and removing the next one
	unsigned long		streamSampleRate;
    //XFIXED				scaling;

    unsigned long		streamDelta;    // delta in stream-format samples
	long				samplesCommitted = 0;

	pStream = theStreams;

	while (pStream)
	{

		pNext = pStream->pNext;

        // convert from engine samples to stream samples
		// $$kk: 05.08.98: the outputSampleRate we get here is a regular sample rate, not a fixed value.
		// i am changing this to account for this.
		// scaling = delta * pStream->streamData.sampleRate / outputSampleRate;
		// streamDelta = XFIXED_TO_LONG_ROUNDED(scaling);
		outputSampleRate = GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality);
		streamSampleRate = XFIXED_TO_LONG_ROUNDED(pStream->streamData.sampleRate);
		streamDelta = delta * streamSampleRate / outputSampleRate;

		// $$kk: we can't count higher than the number of samples processed by the engine for this stream
		samplesCommitted = GM_AudioStreamGetFileSamplePosition((STREAM_REFERENCE)pStream);


		// $$kk: 03.05.98: at the end of the stream, GM_GetSamplePlaybackPosition remains at its last (highest) value even though
		// pStream->streamPlaybackPosition already equals samplesWritten, so GM_GetSamplePlaybackPosition returns an impossibly high
		// value.  make sure we don't use it....
		if (samplesCommitted > pStream->samplesWritten)
		{
				samplesCommitted = pStream->samplesWritten;
		}

		// do we know our offset, and are we in play range?
        if ((pStream->streamPlaybackOffset != STREAM_OFFSET_UNSET) &&
        	(MusicGlobals->samplesPlayed > pStream->streamPlaybackOffset)) 
        {
            // we need to handle the case where the stream may just be starting up again
            if ((unsigned long)(MusicGlobals->samplesPlayed -
            					pStream->streamPlaybackOffset) < streamDelta) 
            {
                streamDelta = MusicGlobals->samplesPlayed - pStream->streamPlaybackOffset;
            }

            // are we incrementing normally?
            if ((pStream->samplesPlayed + streamDelta) < samplesCommitted)
            {
// $$kk: 09.23.98: changed this block ->
				if (pStream->startEvent.status == EVENT_PENDING)
				{
					pStream->startEvent.status = EVENT_DETECTED;
					pStream->startEvent.framePosition = pStream->samplesPlayed;
				}

                pStream->samplesPlayed += streamDelta;
            }
            // no, actual underflow is occurring
            else
            {
                
				if (samplesCommitted > pStream->samplesPlayed)
				{
					if (pStream->startEvent.status == EVENT_PENDING)
					{
						pStream->startEvent.status = EVENT_DETECTED;
						pStream->startEvent.framePosition = pStream->samplesPlayed;
					}
	
					pStream->samplesPlayed = samplesCommitted;
				}
				else 
				{
					if (pStream->stopEvent.status == EVENT_PENDING)
					{
						pStream->stopEvent.status = EVENT_DETECTED;
						pStream->stopEvent.framePosition = pStream->samplesPlayed;
					}

				}
// $$kk: 09.23.98: end changes <-
			}
		}

		pStream = pNext; 
	} // while
	}
*/

// LINKED STREAMS

// group functions
// new
// free
// add GM_Waveform
// add double buffer
// remove
// start
// stop
// setvolume

// Group samples
//
// USE:
//
// linked streams
// Call GM_AudioStreamSetup, to get an allocate stream then call GM_NewLinkedStreamList. Then add it to your 
// maintained top list of linked streams by calling GM_AddLinkedStream. Use GM_FreeLinkedStreamList to delete an entire list, 
// or GM_RemoveLinkedStream to just one link.
//
// Then you can call GM_StartLinkedStreams to start them all at the same time, or GM_EndLinkedStreams
// to end them all, or GM_SetLinkedStreamVolume, GM_SetLinkedStreamRate, and GM_SetLinkedStreamPosition
// set parameters about them all.

// private structure of linked streams
struct GM_LinkedStream
{
    STREAM_REFERENCE			playbackReference;
    void						*threadContext;
    struct GM_LinkedStream		*pNext;
};
typedef struct GM_LinkedStream GM_LinkedStream;


LINKED_STREAM_REFERENCE GM_NewLinkedStreamList(STREAM_REFERENCE reference, void *threadContext)
{
    GM_LinkedStream	*pNew;

    pNew = NULL;
    if (GM_IsAudioStreamValid(reference))
	{
	    pNew = (GM_LinkedStream *)XNewPtr((INT32)sizeof(GM_LinkedStream));
	    if (pNew)
		{
		    pNew->playbackReference	= reference;
		    pNew->pNext = NULL;
		    pNew->threadContext = threadContext;
		}
	}
    return (LINKED_STREAM_REFERENCE)pNew;
}

// Given a top link, deallocates the linked list. DOES NOT deallocate the streams.
void GM_FreeLinkedStreamList(LINKED_STREAM_REFERENCE pTop)
{
    GM_LinkedStream	*pNext, *pLast;

    pNext = (GM_LinkedStream *)pTop;
    while (pNext)
	{
	    pLast = pNext;
	    pNext = pNext->pNext;
	    XDisposePtr((XPTR)pLast);
	}
}

// Given a top link, and a new link this will add to a linked list, and return a new top
// if required.
LINKED_STREAM_REFERENCE GM_AddLinkedStream(LINKED_STREAM_REFERENCE pTop, LINKED_STREAM_REFERENCE pEntry)
{
    GM_LinkedStream	*pNext;

    if (pEntry)
	{
	    pNext = (GM_LinkedStream *)pTop;
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
		    pTop = pEntry;
		}
	    else
		{
		    pNext->pNext = (GM_LinkedStream *)pEntry;
		}
	}
    return pTop;
}

// Given a top link and an link to remove this will disconnect the link from the list and
// return a new top if required.
LINKED_STREAM_REFERENCE GM_RemoveLinkedStream(LINKED_STREAM_REFERENCE pTop, LINKED_STREAM_REFERENCE pEntry)
{
    GM_LinkedStream	*pNext, *pLast;

    if (pEntry)
	{
	    pLast = pNext = (GM_LinkedStream *)pTop;
	    while (pNext)
		{
		    if (pNext == (GM_LinkedStream *)pEntry)								// found object in list?
			{
			    if (pNext == (GM_LinkedStream *)pTop)								// is object the top object
				{
				    pTop = (LINKED_STREAM_REFERENCE)pNext->pNext;						// yes, change to next object
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
	}
    return pTop;
}

STREAM_REFERENCE GM_GetLinkedStreamPlaybackReference(LINKED_STREAM_REFERENCE pLink)
{
    STREAM_REFERENCE			reference;

    reference = DEAD_STREAM;
    if (pLink)
	{
	    reference = ((GM_LinkedStream *)pLink)->playbackReference;
	    if (GM_IsAudioStreamValid(reference) == FALSE)
		{
		    reference = DEAD_STREAM;
		}
	}
    return reference;
}

void * GM_GetLinkedStreamThreadContext(LINKED_STREAM_REFERENCE pLink)
{
    void	*threadContext;

    threadContext = NULL;
    if (pLink)
	{
	    threadContext = ((GM_LinkedStream *)pLink)->threadContext;
	}
    return threadContext;
}


OPErr GM_StartLinkedStreams(LINKED_STREAM_REFERENCE pTop)
{
    GM_LinkedStream		*pNext;
    OPErr				err;

    //printf("\n\nGM_StartLinkedStreams, MusicGlobals = %d, pTop = %d\n", MusicGlobals, pTop);

    err = NO_ERR;	// ok, until proved otherwise
    if (MusicGlobals)
	{
	    // set sync reference. Use our group because its easy and will be unique
	    pNext = (GM_LinkedStream *)pTop;
	    while (pNext)
		{
		    err = GM_SetSyncAudioStreamReference(((GM_LinkedStream *)pNext)->playbackReference, (void *)pTop);
		
		    //printf("GM_StartLinkedStreams, GM_SetSyncAudioStreamReference returned %d\n", err);

		    pNext = pNext->pNext;
		}
	    if (err == NO_ERR)
		{
		    // ok, now wait for mixer to be free
		    while (MusicGlobals->insideAudioInterrupt)
			{
				//printf("GM_StartLinkedStreams, XWaitMicroseocnds\n");
			    XWaitMicroseocnds(HAE_GetSliceTimeInMicroseconds());
			}
		    pNext = (GM_LinkedStream *)pTop;
		    while (pNext)
			{
				//printf("GM_StartLinkedStreams, calling GM_SyncAudioStreamStart with %d\n", ((GM_LinkedStream *)pNext->playbackReference));
			    err = GM_SyncAudioStreamStart(((GM_LinkedStream *)pNext)->playbackReference);
				//printf("GM_StartLinkedStreams, GM_SyncAudioStreamStart returned %d\n", err);
			    pNext = pNext->pNext;
			}
		}
	}
    else
	{
	    err = NOT_SETUP;
	    //printf("GM_StartLinkedStreams, NOT_SETUP!! err = %d\n", err);
	}
    //printf("GM_StartLinkedStreams, returning %d\n\n", err);
    return err;
}

// end in unison the samples for all the linked streams
void GM_EndLinkedStreams(LINKED_STREAM_REFERENCE pTop)
{
    GM_LinkedStream	*pNext;

    pNext = (GM_LinkedStream *)pTop;
    while (pNext)
	{
	    GM_AudioStreamStop(pNext->threadContext, pNext->playbackReference);
	    pNext = pNext->pNext;
	}
}

// Volume range is from 0 to MAX_NOTE_VOLUME
// set in unison the sample volume for all the linked streams
void GM_SetLinkedStreamVolume(LINKED_STREAM_REFERENCE pTop, INT16 sampleVolume, XBOOL defer)
{
    GM_LinkedStream	*pNext;

    pNext = (GM_LinkedStream *)pTop;
    while (pNext)
	{
	    GM_AudioStreamSetVolume(pNext->playbackReference, sampleVolume, defer);
	    pNext = pNext->pNext;
	}
}

// set in unison the sample rate for all the linked streams
void GM_SetLinkedStreamRate(LINKED_STREAM_REFERENCE pTop, XFIXED theNewRate)
{
    GM_LinkedStream	*pNext;

    pNext = (GM_LinkedStream *)pTop;
    while (pNext)
	{
	    GM_AudioStreamSetRate(pNext->playbackReference, theNewRate);
	    pNext = pNext->pNext;
	}
}


// set in unison the sample position for all the linked streams
// range from -63 to 63
void GM_SetLinkedStreamPosition(LINKED_STREAM_REFERENCE pTop, INT16 newStereoPosition)
{
    GM_LinkedStream	*pNext;

    pNext = (GM_LinkedStream *)pTop;
    while (pNext)
	{
	    GM_AudioStreamSetStereoPosition(pNext->playbackReference, newStereoPosition);
	    pNext = pNext->pNext;
	}
}

#endif	// USE_STREAM_API

// EOF of GenAudioStreams.c

