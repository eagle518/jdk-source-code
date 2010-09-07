/*
 * @(#)genresonantfilter.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*********************************** resonant filter *************************************/
#include "GenPriv.h"
#include <math.h>

ResonantFilterParams		gResonantFilterParams;
void RectToCoeff(float x, float y); /* prototype */


//++------------------------------------------------------------------------------
//	GetResonantFilterParams()
//
//++------------------------------------------------------------------------------
ResonantFilterParams* GetResonantFilterParams()
{
    return &gResonantFilterParams;
}

//++------------------------------------------------------------------------------
//	InitResonantFilter()
//
//++------------------------------------------------------------------------------
void InitResonantFilter()
{
    ResonantFilterParams* params = GetResonantFilterParams();

    // init filter memory
    params->y1 = 0.0;
    params->y2 = 0.0;

    params->pi = 4.0 * atan(1.0);
    params->sweep = 0.003;

    params->mControlList[0] = 0.2;
    params->mControlList[1] = 0.85;
		
    params->mFrequency = 0;
    params->mResonance = 0;	// force recalc
    CalculateResonantParams(0.2, 0.95);
}


//++------------------------------------------------------------------------------
//	CalculateResonantParams()
//
//++------------------------------------------------------------------------------
void CalculateResonantParams(float inFrequency, float inResonance)
{
    ResonantFilterParams* params = GetResonantFilterParams();
	
    float srate = 44100;	//!!@
    float omega, x, y;
	

    // both frequency and resonance params between 0.0 and 1.0

    if(params->mFrequency == inFrequency && params->mResonance == inResonance)
	{
	    return;	// params haven't changed
	}
	
	
    params->mFrequency = inFrequency*inFrequency*inFrequency;
	
	
    if(params->mFrequency < 0 ) params->mFrequency = 0;
    if(params->mFrequency >= 1.0 ) params->mFrequency = 0.99999;

    params->mResonance = pow(inResonance, 0.1); //!!@ using floating point...
	
    if(params->mResonance < 0) params->mResonance = 0.0;
    if(params->mResonance >= 1.0) params->mResonance = 0.99999;


    omega = params->mFrequency * params->pi;

    x = cos(omega) * params->mResonance;
    y = sin(omega) * params->mResonance;
	
    RectToCoeff(x, y);
}


//++------------------------------------------------------------------------------
//	RectToCoeff()
//
//++------------------------------------------------------------------------------
void RectToCoeff(float x, float y)
{
    ResonantFilterParams* params = GetResonantFilterParams();


    // scale so that 0Hz == 0dB
    params->c0 = sqrt((1-x)*(1-x) + y*y);
	
    params->c1 = 2 * x;
    params->c2 = -x*x -y*y;
}


//++------------------------------------------------------------------------------
//	RunResonantFilter()
//
//++------------------------------------------------------------------------------
void RunResonantFilter(INT32 *buffer, int nSampleFrames)
{
    ResonantFilterParams* params = GetResonantFilterParams();



    float frequency = params->mControlList[0];	// get frequency control
    float resonance = params->mControlList[1];	// get resonance control
	
    INT32 c0, c1, c2;
    INT32 y1, y2;


#if 1
    frequency += params->sweep;
    if(frequency > 0.8)
	{
	    frequency = 0.8;
	    if(params->sweep > 0) params->sweep = -params->sweep;
	}
    if(frequency < 0.1)
	{
	    frequency = 0.1;
	    if(params->sweep < 0) params->sweep = -params->sweep;
	}
	
	
#else
    frequency = (gController100 / 128.0);

#endif
    params->mControlList[0] = frequency;


	
    CalculateResonantParams(frequency, resonance);

    y2 = params->y2;
    y1 = params->y1;
	

#define COEFF_SHIFT			16
#define	COEFF_MULTIPLY		(1L << 	COEFF_SHIFT)

    c0 = COEFF_MULTIPLY * params->c0;
    c1 = COEFF_MULTIPLY * params->c1;
    c2 = COEFF_MULTIPLY * params->c2;

#define INPUTSHIFT	11
	

    while(nSampleFrames-- > 0)
	{
	    INT32 input = ((buffer[0] + buffer[1]) ) >> (INPUTSHIFT + 2);
		
	    INT32 y = (c0 * input  +  c1*y1  +  c2*y2) >> COEFF_SHIFT;
	    y2 = y1;
	    y1 = y;

	    *buffer++ = y << (INPUTSHIFT - 1);
	    *buffer++ = y << (INPUTSHIFT - 1);
	}
	
    params->y2 = y2;
    params->y1 = y1;
}

