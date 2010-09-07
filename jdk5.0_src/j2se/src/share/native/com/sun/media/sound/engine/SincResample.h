/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SincResample.h	1.6 04/03/31
 */

/*
 * A resample algorithm implemented with bandlimited
 * interpolation. The algorithm is detailed at
 * http://www-ccrma.stanford.edu/~jos/resample/
 *
 * Modification History:
 *	2002-01-06		Created by FB
 */

#ifndef _SINC_RESAMPLE_INCLUDED
#define	_SINC_RESAMPLE_INCLUDED

#define USE_X_API

#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#ifdef USE_X_API
#include "X_API.h"

typedef UINT32	SR_UINT32;
typedef UINT16  SR_UINT16;
typedef UBYTE   SR_UINT8;

typedef INT32   SR_INT32;
typedef INT16   SR_INT16;
typedef SBYTE   SR_INT8;

#else
typedef unsigned int	SR_UINT32;
typedef unsigned short  SR_UINT16;
typedef unsigned char   SR_UINT8;

typedef int		SR_INT32;
typedef short  		SR_INT16;
typedef char   		SR_INT8;
#endif

/* the struct that contains all runtime parameters of the resampler */
typedef struct tagSRResampleParams {
    unsigned int	input_samplerate;
    unsigned int	output_samplerate;
    int		channels;
    int		sample_size;	/* sample size in bits */
    unsigned int	delta_c;	/* Time-shift in coefficients */
    unsigned int	csteps;		/* Steps between coefficients */
    SR_INT16*	table;		/* sinc-function table */
    unsigned int 	history_size;	/* size of history */
    int*		history;	/* history of data */
    unsigned int	history_load;	/* number of samples to load into history in next iteration */
    unsigned int	history_ptr;	/* position in history */
} SR_ResampleParams;

/* returns 0 on error */
extern int SR_init(SR_ResampleParams* params,
		   unsigned int input_samplerate,
		   unsigned int output_samplerate,
		   unsigned int channels,
		   unsigned int sample_size_in_bits);

extern void SR_resample(SR_ResampleParams* params,
			void* input,
			unsigned int* in_count, /* IN: number of input FRAMES, OUT: number of actually read FRAMES */
			void* output,
			unsigned int* out_count); /* IN: number of output FRAMES, OUT: number of actually written FRAMES */

/* returns 0 on error */
extern int SR_change_samplerate(SR_ResampleParams* params,
				unsigned int new_input_samplerate,
				unsigned int new_output_samplerate);


/*
 * input data in 16 bit, 1 or 2 channels,
 * output data in 16bit samples stored in 32bit integers, stereo. */
extern void SR_resample32_add(SR_ResampleParams* params, INT32 input_channels, INT32 input_sample_size,
			      INT32 left_factor, INT32 right_factor, /* volume factors */
			      INT32 left_factor_inc, INT32 right_factor_inc, /* volume factor increase */
			      void* input,
			      INT32* in_count, /* IN: number of input FRAMES, OUT: number of actually read FRAMES */
			      SR_INT32* output,
			      INT32* out_count); /* IN: number of output FRAMES, OUT: number of actually written FRAMES */

extern void SR_exit(SR_ResampleParams* params);

#endif
