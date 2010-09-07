/*
 * @(#)GenAudioCaptureStreams.c	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/***************************************************************************/
/*
**	"GenAudioCaptureStreams.c"
**
**	This implements audio capture
**
**	History	-
**	6/5/98		Created
**	7/23/98		Continued development and general flushing out of everything
**	7/31/98		Fixed a memory leak in GM_AudioCaptureStreamCleanup
**	8/10/98		Modified GM_AudioCaptureStreamPause & GM_AudioCaptureStreamResume to
**				disconnect and reconnect to the capture hardware and update various
**				settings
**	8/13/98		Changed the way the callbacks are handle. Now dealing with
**				parameters rather than direct values. See PV_AudioCaptureCallback
** JAVASOFT
** 10.06.98		$$kk: PV_AudioCaptureCallback gets bufferSize in bytes, but 
**				streamData.dataLength should be in frames.  i made the conversion.
** 10.14.98:	$$kk: added GM_AudioCaptureStreamGetSamplesCaptured
** 2002-03-14	$$fb cleaned up debug code
*/
/***************************************************************************/

#include "X_API.h"
#include "GenSnd.h"
#include "GenPriv.h"
#include "HAE_API.h"

#if ((USE_CAPTURE_API == TRUE) && (USE_STREAM_API == TRUE))

#define CAPTURE_STREAM_ID				FOUR_CHAR('E','A','R','S')	//	'EARS' id for verification of valid stream

// Internal structures
struct GM_CaptureAudioStream
{
    void *							reference;
    INT32							streamID;

    GM_StreamObjectProc				streamCallback;
    GM_StreamData					streamData;

    // $$kk: 10.09.98: changed these
    // $$kk: 10.14.98: not using this double buffer mechanism anymore....
    void							*pStreamBuffer1;
    void							*pStreamBuffer2;
    UINT32					streamBufferLength;
    // $$kk: 10.09.98: end changes

    UINT32					samplesCaptured;

    XBOOL							streamActive:1;
    XBOOL							streamShuttingDown:1;
    XBOOL							streamPaused:1;
    XBOOL							streamFirstTime:1;			// first time active
    XBOOL							streamUnderflow:1;

    OPErr							streamErr;

    void							*platformContext;

    struct GM_CaptureAudioStream	*pNext;
};
typedef struct GM_CaptureAudioStream GM_CaptureAudioStream;

static GM_CaptureAudioStream	*theCaptureStreams = NULL;

// verify reference is a valid audio stream structure
static GM_CaptureAudioStream * PV_CaptureAudioStreamGetFromReference(void *reference)
{
    GM_CaptureAudioStream *pStream;
    GM_CaptureAudioStream *next;
	
    pStream = (GM_CaptureAudioStream *)reference;
    next = theCaptureStreams;
    while ( next != NULL )
	{
	    if (next == pStream)
		{
		    if (pStream->streamID == CAPTURE_STREAM_ID)
			{
			    return pStream;
			}
		}
	    next = next->pNext;
	}
    return NULL;
}

static void PV_AddCaptureAudioStream(GM_CaptureAudioStream *next)
{
    GM_CaptureAudioStream *last;

    if (next)
	{
	    next->streamID = CAPTURE_STREAM_ID;
	    if (theCaptureStreams == NULL)
		{
		    theCaptureStreams = next;
		}
	    else
		{
		    last = theCaptureStreams;
		    while (last->pNext)
			{
			    last = last->pNext;
			}
		    last->pNext = next;
		}
	    next->pNext = NULL;
	}
}


static void PV_FreeCaptureAudioStream(GM_CaptureAudioStream *found)
{
    GM_CaptureAudioStream *next, *last;

    found = PV_CaptureAudioStreamGetFromReference(found);		// verify as valid
    if (found)
	{
	    if (found->streamID == CAPTURE_STREAM_ID)
		{
		    last = next = theCaptureStreams;
		    while (next)
			{
			    if (next == found)								// found object in list?
				{
				    if (next == theCaptureStreams)				// is object the top object
					{
					    theCaptureStreams = next->pNext;		// yes, change to next object
					}
				    else
					{
					    if (last)						// no, change last to point beyond next
						{
						    last->pNext = next->pNext;
						}
					}
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

// Get an empty GM_CaptureAudioStream. Will return 0 if can't allocate stream
static void *PV_GetEmptyCaptureAudioStream(void)
{
    GM_CaptureAudioStream	*pStream;
    void *					ref;

    ref = 0;
    pStream = (GM_CaptureAudioStream *)XNewPtr((INT32)sizeof(GM_CaptureAudioStream));
    if (pStream)
	{
	    pStream->reference = (void *)-1;
	    ref = (void *)pStream;
	}
    return ref;
}

static UINT32 PV_GetSampleSizeInBytes(GM_StreamData * pAS)
{
    return (UINT32)pAS->channelSize * ((UINT32)pAS->dataBitSize / 8);
}

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
void		*GM_AudioCaptureStreamSetup(	void *platformContext,		// platform context
						void *userReference, 			// user reference
						GM_StreamObjectProc pProc, 		// control callback
						UINT32 bufferSize, 		// buffer size in bytes
						XFIXED sampleRate,				// Fixed 16.16
						char dataBitSize,				// 8 or 16 bit data
						char channelSize,				// 1 or 2 channels of date
						OPErr *pErr)
{
    GM_CaptureAudioStream	*pStream;
    void *					returnedReference;
    OPErr					haeErr;

    haeErr = NO_ERR;
    returnedReference = 0;
    if ( (pErr) && (pProc) && ( (channelSize >= 1) || (channelSize <= 2) ) && ( (dataBitSize == 8) || (dataBitSize == 16) ) )
	{
	    returnedReference = PV_GetEmptyCaptureAudioStream();
	    if (returnedReference)
		{
		    pStream = (GM_CaptureAudioStream *)returnedReference;

		    pStream->streamCallback = pProc;
		    pStream->streamUnderflow = FALSE;
		    pStream->reference = userReference;
		    pStream->samplesCaptured = 0; 
		    pStream->streamShuttingDown = FALSE;

		    pStream->streamData.pData = NULL;
		    pStream->streamData.userReference = pStream->reference;
		    pStream->streamData.streamReference = pStream;
		    pStream->streamData.sampleRate = sampleRate;
		    pStream->streamData.dataBitSize = dataBitSize;
		    pStream->streamData.channelSize = channelSize;
		    pStream->platformContext = platformContext;

		    // $$kk: 10.13.98: right now we are not allowing the user to set the size; 
		    // we force use of the device value....
		    pStream->streamData.dataLength = bufferSize / PV_GetSampleSizeInBytes(&pStream->streamData);

		    // $$kk: 10.09.98: here we request allocation of enough memory for both buffers
		    haeErr = (*pProc)(platformContext, STREAM_CREATE, &pStream->streamData);
		    if (haeErr == NO_ERR)
			{
			    pStream->pStreamBuffer1 = pStream->streamData.pData;
				// convert from frames to bytes
				// $$kk: 10.09.98: we divide the allocated data into two buffers, each half the total length
			    pStream->streamBufferLength = pStream->streamData.dataLength * PV_GetSampleSizeInBytes(&pStream->streamData) / 2;
			    pStream->pStreamBuffer2 = (char *)pStream->streamData.pData + pStream->streamBufferLength;

			    PV_AddCaptureAudioStream(pStream);		// ok add stream
			}
		    else
			{
			    pStream->streamCallback = NULL;
			    (*pProc)(platformContext, STREAM_DESTROY, &pStream->streamData);
			    haeErr = DEVICE_UNAVAILABLE;
			}
		}
	    else
		{
		    haeErr = MEMORY_ERR;
		}
	}
    else
	{
	    haeErr = PARAM_ERR;
	}
    if (haeErr)
	{
	    XDisposePtr((XPTR)returnedReference);
	    returnedReference = 0;
	}
    if (pErr)
	{
	    *pErr = haeErr;
	}
    return returnedReference;
}

// This will stop a streaming audio object and free any memory.
//
// INPUT:
//	reference	This is the reference number returned from AudioStreamStart.
//
OPErr GM_AudioCaptureStreamCleanup(void *context, void *reference)
{
    GM_CaptureAudioStream	*pStream;
    GM_StreamObjectProc		pProc;
    OPErr					theErr;

    theErr = GM_AudioCaptureStreamStop(context, reference);
    if (theErr == NO_ERR)
	{
	    pStream = PV_CaptureAudioStreamGetFromReference(reference);

	    if (pStream)
		{
		    pProc = pStream->streamCallback;
		    if (pProc)
			{
			    pStream->streamCallback = NULL;

				// $$kk: 04.13.99: i am removing the STREAM_DESTROY callback for now.
				// this is an issue we should come back to.  right now, GM_AudioCaptureStreamStop
				// stops the frame proc thread.  since GM_AudioCaptureStreamStop has already
				// been called, this callback causes a crash.  at some point, we need to
				// get our model of device / thread / stream resource management a bit clearer.
				// theErr = (*pProc)(context, STREAM_DESTROY, &pStream->streamData);
			}			

		    PV_FreeCaptureAudioStream(pStream);
		}
	    else
		{
		    theErr = DEVICE_UNAVAILABLE;
		}
	}
    return theErr;
}

/* PV_AudioCaptureCallback - Low-level callback function to handle audio input.
 *      Installed by waveInOpen().  The input handler takes incoming
 *      captured audio events and places them in the input buffer.   
 *
 * Param:   hwi - Handle for the associated input device.
 *          message - One of the WIM_***** messages.
 *          dwInstance - Points to XCaptureDevice structure.
 *
 * Return:  void
 */     

static void PV_AudioCaptureCallback(void *callbackContext, HAECaptureMessage message, 
				    void *parmeter1, void *parmeter2)
{
    GM_CaptureAudioStream	*pCapture, *pNext;
    OPErr					theErr;
    char					**buffer;
    UINT32			*bufferSize;

    pCapture = theCaptureStreams;
    while (pCapture)
	{
	    pNext = pCapture->pNext;
	    switch(message)
		{
		case OPEN_CAPTURE:
		    break;
		case CLOSE_CAPTURE:
		    break;
		case DATA_READY_CAPTURE:
				
		    if (pCapture->streamCallback && parmeter1 && parmeter2)
			{
			    buffer = ((char **)parmeter1);
			    bufferSize = ((UINT32 *)parmeter2);
			    pCapture->streamData.streamReference = pCapture;
			    pCapture->streamData.userReference = pCapture->reference;
			    pCapture->streamData.pData = (void *)*buffer;

			    // $$kk: 10.06.98: PV_AudioCaptureCallback gets bufferSize in bytes, but streamData.dataLength should be in frames 
			    //					pCapture->streamData.dataLength = *bufferSize;
					
			    pCapture->streamData.dataLength = *bufferSize / PV_GetSampleSizeInBytes(&pCapture->streamData);
			    pCapture->samplesCaptured += pCapture->streamData.dataLength;

			    theErr = (*pCapture->streamCallback)(callbackContext, 
								 STREAM_HAVE_DATA, &pCapture->streamData);
			}
		    break;
		}

	    pCapture = pNext;
	}
}


// once a stream has been setup, call this function
OPErr GM_AudioCaptureStreamStart(void *threadContext, void *reference)
{
    GM_CaptureAudioStream	*pStream;
    OPErr					theErr;
	
    theErr = NO_ERR;
    pStream = PV_CaptureAudioStreamGetFromReference(reference);
    if (pStream)
	{
	    // start capture
	    if (HAE_StartAudioCapture(PV_AudioCaptureCallback, threadContext))
		{
		    theErr = DEVICE_UNAVAILABLE;
		}
	}
    return theErr;
}

// once a stream has been setup, call this function to stop the capture
OPErr GM_AudioCaptureStreamStop(void* threadContext, void *reference)
{
    GM_CaptureAudioStream	*pStream;
    OPErr					theErr;

    theErr = NO_ERR;
    pStream = PV_CaptureAudioStreamGetFromReference(reference);
    if (pStream)
	{
	    // stop buffer capture
	    if (HAE_StopAudioCapture(threadContext))
		{
		    theErr = DEVICE_UNAVAILABLE;
		}
	}
    else
	{
	    theErr = INVALID_REFERENCE;
	}

    return theErr;
}

void *GM_AudioCaptureStreamGetReference(void *reference)
{
    GM_CaptureAudioStream	*pStream;
    void					*userReference;

    userReference = 0;
    pStream = PV_CaptureAudioStreamGetFromReference(reference);
    if (pStream)
	{
	    userReference = pStream->reference;
	}
    return userReference;
}

/*
  // Pause or resume this particular audio stream
  OPErr GM_AudioCaptureStreamResume(long reference)
  {
  GM_CaptureAudioStream	*pStream;
  OPErr					theErr;

  theErr = NO_ERR;
  pStream = PV_CaptureAudioStreamGetFromReference(reference);
  if (pStream)
  {
  if (HAE_AquireAudioCapture(pStream->platformContext, (unsigned long)XFIXED_TO_LONG(pStream->streamData.sampleRate), 
  pStream->streamData.channelSize, pStream->streamData.dataBitSize, NULL) == 0)
  {
  theErr = GM_AudioCaptureStreamStart(reference);
  }
  else
  {
  theErr = DEVICE_UNAVAILABLE;
  }
  }
  else
  {
  theErr = NOT_SETUP;
  }
  return theErr;
  }

  OPErr GM_AudioCaptureStreamPause(long reference)
  {
  GM_CaptureAudioStream	*pStream;
  OPErr					theErr;

  theErr = NO_ERR;
  pStream = PV_CaptureAudioStreamGetFromReference(reference);
  if (pStream)
  {
  theErr = GM_AudioCaptureStreamStop(reference);

  if (theErr == NO_ERR)
  {
  if (HAE_ReleaseAudioCapture(pStream->platformContext))
  {
  theErr = DEVICE_UNAVAILABLE;
  }
  }
  }
  return theErr;
  }
*/

// Returns TRUE if a given AudioStream is valid
XBOOL GM_IsAudioCaptureStreamValid(void *reference)
{
    return (PV_CaptureAudioStreamGetFromReference(reference) != NULL) ? (XBOOL)TRUE : (XBOOL)FALSE;
}


// Get the count of samples captured to this stream.
// $$kk: 10.14.98: added this
UINT32 GM_AudioCaptureStreamGetSamplesCaptured(void *reference)
{
    GM_CaptureAudioStream	*pCapture;
    UINT32 samplesCaptured = 0;


    pCapture = PV_CaptureAudioStreamGetFromReference(reference);
    if (pCapture)
	{
	    samplesCaptured = pCapture->samplesCaptured;
	}

    return samplesCaptured;
}



#endif	// USE_CAPTURE_API

// EOF of GenAudioCaptureStreams.c

