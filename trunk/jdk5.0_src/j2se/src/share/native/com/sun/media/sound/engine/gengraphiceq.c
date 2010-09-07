/*
 * @(#)gengraphiceq.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <math.h>
#include <string.h>	// for malloc
#include "GenPriv.h"
#include "HAE_API.h"


GraphicEqParams		gGraphicEqParams;

#define	LOG8	0.903


#define COEFF_SHIFT				26
#define HISTORY_SHIFT			10
#define COEFF_HISTORY_SHIFT		16



static float	coef[kNumberOfBands * 3] =	{
    0.0025579741,-1.9948111773,0.9948840737,
    0.0063700872,-1.9868060350,0.9872598052,
    0.0168007612,-1.9632060528,0.9663984776,
    0.0408578217,-1.8988473415,0.9182843566,
    0.0914007276,-1.7119922638,0.8171985149,
    0.1845672876,-1.0703823566,0.6308654547,
    0.3760778010, 0.6695288420,0.2478443682
};
	
INT32	intCoefficients[kNumberOfBands * 3];

//++------------------------------------------------------------------------------
//	GetGraphicEqParams()
//
//++------------------------------------------------------------------------------
GraphicEqParams* GetGraphicEqParams()
{
    return &gGraphicEqParams;
}




//++------------------------------------------------------------------------------
//	InitGraphicEq()
//
//++------------------------------------------------------------------------------
void InitGraphicEq()
{
    /*
      60 hz
      150 hz
      400 hz
      1 khz
      2.4 khz
      6 khz
      15 khz
    */
 

    GraphicEqParams* params = GetGraphicEqParams();
    int i;
	
	
    for(i = 0 ; i < kNumberOfBands; i++ )
	{
	    params->mHistory1L[i] = 0.0;
	    params->mHistory2L[i] = 0.0;
	    params->mHistory1R[i] = 0.0;
	    params->mHistory2R[i] = 0.0;

	    params->mControlList[i] = 0.5;

	    intCoefficients[i*3] = coef[i*3] * (float)(1L << COEFF_SHIFT);
	    intCoefficients[i*3+1] = coef[i*3+1] * (float)(1L << COEFF_SHIFT);
	    intCoefficients[i*3+2] = coef[i*3+2] * (float)(1L << COEFF_SHIFT);
	}
	
	

    //!!@ just a hack for testing
    params->mControlList[0] = 0.9;
    params->mControlList[1] = 0.9;
    params->mControlList[2] = 0.0;
    params->mControlList[3] = 0.0;
    params->mControlList[4] = 0.0;
    params->mControlList[5] = 0.0;
    params->mControlList[6] = 0.0;
	
    CalculateGraphicEqGains();
}


//++------------------------------------------------------------------------------
//	CalculateGraphicEqGains()
//
//++------------------------------------------------------------------------------
void	CalculateGraphicEqGains()
{
    GraphicEqParams* params = GetGraphicEqParams();
    int i;

    // maps gain from 1/8 to 8 (-18dB to +18dB)
    for(i = 0 ; i < kNumberOfBands ; i++ )
	{
	    double fgain = params->mControlList[i];
	    fgain = (fgain * 2. * LOG8) - LOG8;
	    params->mGain[i] = pow(10.0, fgain);
	}
}
						
//++------------------------------------------------------------------------------
//	RunGraphicEq()
//
//++------------------------------------------------------------------------------
void RunGraphicEq(INT32 *buffer, int nSampleFrames)
{
    //CalculateGraphicEqGains();
    GraphicEqParams* params = GetGraphicEqParams();
	
    float	*gain = params->mGain;	
    int		i;
    INT32	intGain[kNumberOfBands];

#define GAINSHIFT	8

#define INPUTSHIFT	16
		
    for(i = 0; i < kNumberOfBands; i++)
	{
	    intGain[i] = gain[i] * (1L << GAINSHIFT);
	}
	
    while(nSampleFrames-- > 0)
	{
	    INT32 inputL = buffer[0] >> INPUTSHIFT;
	    INT32 inputR = buffer[1] >> INPUTSHIFT;

#if 0
	    INT32 fresultL = 0;
	    INT32 fresultR = 0;
		
	    INT32 *hist1_ptrL = params->mHistory1L;
	    INT32 *hist2_ptrL = params->mHistory2L;
		
	    INT32 *hist1_ptrR = params->mHistory1R;
	    INT32 *hist2_ptrR = params->mHistory2R;
		
	    INT32 *coef_ptr = intCoefficients;
		
	    int j;
	    for(j = 0 ; j < kNumberOfBands ; j++ )
		{		
		    INT32 coeff1 = *coef_ptr++;
		    INT32 coeff2 = *coef_ptr++;
		    INT32 coeff3 = *coef_ptr++;

		    INT32 history1L = *hist1_ptrL;
		    INT32 history2L = *hist2_ptrL;
		    INT32 history1R = *hist1_ptrR;
		    INT32 history2R = *hist2_ptrR;


		    INT32 scaledInputL = (inputL >> HISTORY_SHIFT) * coeff1;
		    INT32 scaledInputR = (inputR >> HISTORY_SHIFT) * coeff1;	
			
		    INT32 scaledHist1L = (history1L >> HISTORY_SHIFT) * coeff2;
		    INT32 scaledHist1R = (history1R >> HISTORY_SHIFT) * coeff2;
			
		    INT32 scaledHist2L = (history2L >> HISTORY_SHIFT) * coeff3;
		    INT32 scaledHist2R = (history2R >> HISTORY_SHIFT) * coeff3;
			
		    INT32 new_histL = (scaledInputL - scaledHist1L - scaledHist2L) >> COEFF_HISTORY_SHIFT;
		    INT32 new_histR = (scaledInputR - scaledHist1R - scaledHist2R) >> COEFF_HISTORY_SHIFT;
						
		    INT32 outputL = new_histL - history2L;
		    INT32 outputR = new_histR - history2R;
			
			
			
		    *hist2_ptrL++ = *hist1_ptrL;
		    *hist1_ptrL++ = new_histL;
			
		    *hist2_ptrR++ = *hist1_ptrR;
		    *hist1_ptrR++ = new_histR;
#if 0			
		    fresultL += ( (outputL >> GAINSHIFT) * intGain[j]);
		    fresultR += ( (outputR >> GAINSHIFT) * intGain[j]);
			
#else
		    fresultL += outputL;
		    fresultR += outputR;

#endif
		}
		
	    *buffer++ = fresultL << (INPUTSHIFT - 4);		
	    *buffer++ = fresultR << (INPUTSHIFT - 4);		
#else

	    INT32	L = inputL >> 20;
	    INT32	R = inputR >> 20;
		
	    *buffer++ = L << 20;
	    *buffer++ = R << 20;	

#endif



	}
}

