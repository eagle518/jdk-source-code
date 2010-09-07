/*
 * @(#)genparametriceq.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
//++------------------------------------------------------------------------------
//	File:		EParametricEQ.cpp
//
//	Purpose:	parametric eqs
//++------------------------------------------------------------------------------

#if WIN
#include "BPrecompile.h"
#endif

// shelving code
// Q cannot go to zero

#include "GenPriv.h"
#include <math.h>


#define	LOG8	0.903
#define	LOG10	1.0
#define	QMAXSCALE	1.9590			// log(10.*(10-1)+1)/log(10)
#define	QMINSCALE	3.9069e-4		// log(0.0001*(10-1)+1)/log(10)
#define	QRANGE		(QMAXSCALE-QMINSCALE)

// parametric equalization according to Massie/Mitra
//

ParametricEqParams		gParametricEqParams;


//++------------------------------------------------------------------------------
//	GetParametricEqParams()
//
//++------------------------------------------------------------------------------
ParametricEqParams* GetParametricEqParams()
{
    return &gParametricEqParams;
}


//++------------------------------------------------------------------------------
//	InitParametricEq()
//
//++------------------------------------------------------------------------------
void InitParametricEq()
{
    ParametricEqParams* params = GetParametricEqParams();


    params->mFreqValue = 0;
    params->mQValue = 0;
    params->mGainValue = 0;	// force parameter recalc

    //!!@ just a hack for testing
    params->mControlList[0] = 0.1;		// frequency
    params->mControlList[1] = 0.8;		// resonance
    params->mControlList[2] = 0.99;		// gain
	

    params->pi = 4.0 * atan(1.0);


    params->x1 = 0;
    params->x2 = 0;
    params->y1 = 0;
    params->y2 = 0;
	
    params->sweep = 0.003;
	
    CalculateParametricParams();
}

//++------------------------------------------------------------------------------
//	CalculateParametricParams()
//
//++------------------------------------------------------------------------------
void CalculateParametricParams(void)
{
    ParametricEqParams* params = GetParametricEqParams();


    double frequency = params->mControlList[0];
    double gain = params->mControlList[2];
    double q = 1.0 - params->mControlList[1];
    double bw;
    double k1, k2, temp;
	
	
    if(frequency == params->mFreqValue && q == params->mQValue && gain == params->mGainValue )
	{
	    return;	// no parameter recalc is necessary
	}

    params->mFreqValue = frequency;
    params->mQValue = q;
    params->mGainValue = gain;


    // get frequency control
    frequency = frequency * frequency * frequency;
		
    // maps gain from 1/8 to 8 (-18dB to +18dB)
    gain = (gain * 2. * LOG8) - LOG8;
    gain = pow(10., gain);
	
    // Q goes from 1/10 to 10
    q *= QRANGE;
    q += QMINSCALE;
    q = (pow(10, q) - 1.0) / (10.0 - 1.0);
	
    // according to Massie/Mitra	
    // compute width from q and center frequency
    bw = q * frequency;

    if( bw > 0.45 )				// limit input to tan below.
	{
	    bw = 0.45;
	}
    else if( bw < 0.001 )
	{
	    bw = 0.001;
	}

    k1 = -cos(frequency * params->pi);
    temp = tan(bw * params->pi);
    k2 = (1.0 - temp) / (1.0 + temp);
    temp = k1 * (1.0 + k2);
	
    params->b0 = (k2 + 1.0 - gain*k2 + gain) / 2.0;
    params->b1 = temp;
    params->b2 = (k2 + 1.0 + gain*k2 - gain) / 2.0;
    params->a1 = temp;
    params->a2 = k2;	
}


//++------------------------------------------------------------------------------
//	RunParametricEq()
//
//++------------------------------------------------------------------------------
void 	RunParametricEq(INT32 *buffer, int nSampleFrames)
{
    INT32 tx1,tx2,ty1,ty2;
    INT32 tb0,tb1,tb2,ta1,ta2;
    float kInvScale = 1000.0;
    float kScale = 1.0 / kInvScale;
	
    ParametricEqParams* params = GetParametricEqParams();

#if 1
    float freq = params->mControlList[0];
    freq += params->sweep;
    if(freq > 0.8)
	{
	    freq = 0.8;
	    if(params->sweep > 0) params->sweep = -params->sweep;
	}
    if(freq < 0.15)
	{
	    freq = 0.15;
	    if(params->sweep < 0) params->sweep = -params->sweep;
	}
	
	
    params->mControlList[0] = freq;
#endif

    CalculateParametricParams();
	
	
    tx1 = params->x1;
    tx2 = params->x2;
    ty1 = params->y1;
    ty2 = params->y2;
	
#define COEFF_SHIFT			16
#define	COEFF_MULTIPLY		(1L << 	COEFF_SHIFT)

    tb0 = COEFF_MULTIPLY * params->b0;
    tb1 = COEFF_MULTIPLY * params->b1;
    tb2 = COEFF_MULTIPLY * params->b2;
    ta1 = COEFF_MULTIPLY * params->a1;
    ta2 = COEFF_MULTIPLY * params->a2;
	
#define INPUTSHIFT	9

	
    while(nSampleFrames-- > 0)
	{
	    INT32 x0 = ((buffer[0] + buffer[1]) ) >> (INPUTSHIFT + 2);

	    INT32 result = (tb0*x0 + tb1*tx1 + tb2*tx2 - ta1*ty1 - ta2*ty2) >> COEFF_SHIFT;

	    // stereo output
	    *buffer++ =  result << INPUTSHIFT;
	    *buffer++ =  result << INPUTSHIFT;
		
	    tx2 = tx1;
	    tx1 = x0;
	    ty2 = ty1;
	    ty1 = result;
	}
	
    params->x1 = tx1;
    params->x2 = tx2;
    params->y1 = ty1;
    params->y2 = ty2;
}
