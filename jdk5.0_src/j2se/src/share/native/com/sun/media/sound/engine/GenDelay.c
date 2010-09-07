/*
 * @(#)GenDelay.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <math.h>
#include <string.h>	// for malloc
#include "GenPriv.h"
#include "HAE_API.h"

#define INPUTSHIFT	9

#define COEFF_SHIFT			16
#define	COEFF_MULTIPLY		(1L << 	COEFF_SHIFT)


#define _PI 	3.14159265359


DelayParams		gDelayParams;

//++------------------------------------------------------------------------------
//	GetDelayParams()
//
//++------------------------------------------------------------------------------
DelayParams* GetDelayParams()
{
    return &gDelayParams;
}


//++------------------------------------------------------------------------------
//	InitDelay()
//
//++------------------------------------------------------------------------------
void InitDelay()
{
    long sampleFramesDelay;
    DelayParams* params = GetDelayParams();
    float filterValue, freq;
	
	
    // allocate the delay line memory
    long kMaxBytes = 2 * sizeof(INT32) * kDelayBufferFrameSize;
    params->mDelayBuffer = (INT32*)HAE_Allocate(kMaxBytes );

    params->mSecondsDelay = 0.25;
	

    sampleFramesDelay = params->mSecondsDelay * 44100.0;	//!!@

    params->mWriteIndex = 0;
    params->mReadIndex = kDelayBufferFrameSize - 2*sampleFramesDelay;
	
    //params->mFeedbackValue = 0.7;		//unused
    params->mFeedbackGain = 0.4;

    filterValue = 0.2;
    freq = filterValue  *  22000.0;

    params->mLopassK = (1.0 - exp(-2*_PI * freq / 44100.0) ) * COEFF_MULTIPLY;
    params->mFilterMemoryL = 0;
    params->mFilterMemoryR = 0;
}

//++------------------------------------------------------------------------------
//	ShutdownDelay()
//
//++------------------------------------------------------------------------------
void ShutdownDelay()
{
    DelayParams* params = GetDelayParams();

    // deallocate the buffer
    HAE_Deallocate(params->mDelayBuffer );
}



//++------------------------------------------------------------------------------
//	RunDelay()
//
//++------------------------------------------------------------------------------
void RunDelay(INT32 *sourceP, int nSampleFrames)
{
    DelayParams* params = GetDelayParams();

    INT32	*buffer = params->mDelayBuffer;
    float	feedbackGain = params->mFeedbackGain;
    long	writeIndex = params->mWriteIndex;
    long	readIndex = params->mReadIndex;
	
    INT32	lopassK = params->mLopassK;
    INT32	filterMemoryL = params->mFilterMemoryL;
    INT32	filterMemoryR = params->mFilterMemoryR;
	

#define FEEDBACKSHIFT	7
	
    INT32	intFeedbackGain = feedbackGain * (1L << FEEDBACKSHIFT);
	
    while(nSampleFrames-- > 0)
	{
	    // get input
	    INT32 inputL = sourceP[0] >> INPUTSHIFT;
	    INT32 inputR = sourceP[1] >> INPUTSHIFT;

	    INT32 tapL = buffer[readIndex];
	    INT32 tapR = buffer[readIndex+1];
		
	    INT32 filterOutputL, filterOutputR;


	    // now run through its filter
	    filterOutputL = (lopassK * filterMemoryL) >> COEFF_SHIFT;
	    filterMemoryL = tapL  + filterMemoryL - filterOutputL;

	    filterOutputR = (lopassK * filterMemoryR) >> COEFF_SHIFT;
	    filterMemoryR = tapR  + filterMemoryR - filterOutputR;




	    // write input plus feedback back into the delay line
	    buffer[writeIndex] = inputL + ((filterOutputL*intFeedbackGain) >> FEEDBACKSHIFT) ;
	    buffer[writeIndex+1] = inputR + ((filterOutputR*intFeedbackGain) >> FEEDBACKSHIFT) ;


	    // write to output
	    *sourceP++ = (filterOutputL + inputL) << (INPUTSHIFT - 1);
	    *sourceP++ = (filterOutputR + inputR) << (INPUTSHIFT - 1);
	
	    // increment the read head	
	    readIndex = (readIndex + 2) % kDelayBufferFrameSize;
		
	    // wrap-around write pointer
	    writeIndex = (writeIndex + 2) % kDelayBufferFrameSize;
	}

    // remember state
    params->mWriteIndex = writeIndex;
    params->mReadIndex = readIndex;
	
	
    params->mFilterMemoryL = filterMemoryL;
    params->mFilterMemoryR = filterMemoryR;
}
