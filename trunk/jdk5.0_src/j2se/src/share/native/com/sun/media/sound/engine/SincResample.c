/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)SincResample.c	1.8 04/04/02
 */

/*
 * Implementation of bandlimited interpolation algorithm
 * for resampling.
 *
 * Modification History:
 *	2002-01-06	Created by FB. Transformed to a
 *			real-time capable implementation with history.
 *			Original implementation by Darragh O'Brien.
 */

// TODO:
// why is SR_init called with a negative sample rate initially?
// happens with test020 in the test suite

#include "SincResample.h"

#define	NZCS		(5)	/* Number of zero crossings */
#define	CPZC		(128)	/* Coefficients per zero crossing */
#define	CPZC_SHIFT	(7)	/* 1 << CPZC_SHIFT = CPZC */
#define	COFF_SHIFT	(15)	/* precision of samples/coefficients */

#define SR_INPUT_INDEX_SHIFT  14
#define SR_INPUT_INDEX_FACTOR 16384 /* 2^SR_INPUT_INDEX_SHIFT  */
#define SR_INPUT_INDEX_MASK   16383 /* SR_INPUT_INDEX_FACTOR-1 */

#define COFF_COUNT      (2 * NZCS)

/* generic resample algorithm that reads and writes 16bit signed samples in native endianness */

void SR_resample16(SR_ResampleParams* params,
		   SR_INT16* input, unsigned int* in_count,
		   SR_INT16* output, unsigned int* out_count) {

    unsigned int	risamples;	/* number of remaining input samples */
    unsigned int	c, k;		/* Counters */
    unsigned int	h_index;	/* Inner loop history index */
    unsigned int	rosamples;	/* number of remaining output samples */
    int			c_index;	/* Starting coefficient index */
    int			sum;		/* Output sample value */
    unsigned int	channels;	/* local channels */
    unsigned int	history_load;	/* local history_load */
    unsigned int 	history_size;	/* local size of history */
    int*		history;	/* local history */
    unsigned int	history_ptr;	/* local position in history */

    /* copy some vars in local variables */
    channels = params->channels;
    history_load = params->history_load;
    history_size = params->history_size;
    history = params->history;
    history_ptr = params->history_ptr;

    risamples = (*in_count) * channels;
    rosamples = (*out_count) * channels;

    while ((risamples>0 || history_load==0) && rosamples>0) {
	/* load history with new samples */
	for (; history_load>0; history_load--) {
	    if (risamples==0) {
		goto end_of_loop;
	    }
	    history[history_ptr++] = *input;
	    input++; risamples--;
	    if (history_ptr >= history_size) {
		history_ptr = 0;
	    }
	}

	/* Generate output sample */
	for (c = 0; c < channels; c++) {
	    /* history index in inner loop */
	    h_index=( history_ptr - ((COFF_COUNT + 1) * channels)
		      + history_size + c) % history_size;

	    /* coefficient start index */
	    /* coefficient start index
	     * -- need to bring delta_c down to CPZC level */
	    c_index = CPZC - (params->delta_c >>
                      (SR_INPUT_INDEX_SHIFT - CPZC_SHIFT));

	    sum = 0;
	    for (k = 0; k <= COFF_COUNT; k++) {
#ifdef DEBUG
		if (h_index >= history_size) {
		    printf("\nh_index>=history_size  %d>%d\n", h_index, history_size);
		} /*else
		    if (c_index >= sizeof(SincFuncTable)/sizeof(SR_INT16)) {
		    printf("\nc_index>=table_size  %d>%d\n", c_index, sizeof(SincFuncTable)/sizeof(SR_INT16)); fflush(stdout);
		    } */ else
#endif
		sum += (params->table[c_index] * history[h_index]);
		c_index += CPZC;
		h_index += channels;
		if (h_index >= history_size) {
		    h_index -= history_size;
		}
	    }

	    /* Write out */
	    sum=sum>>COFF_SHIFT;
	    if (sum>32767) {
		sum=32767;
	    }
	    else if (sum<-32768) {
		sum=-32768;
	    }
	    *output = (SR_INT16) sum;
	    output++;
	    rosamples--;
	}

	/* Increment counters */
	params->delta_c += params->csteps;
	history_load = (params->delta_c >> SR_INPUT_INDEX_SHIFT) * channels;
	params->delta_c &= SR_INPUT_INDEX_MASK;
    }
end_of_loop:
    /* save local copies for next time */
    params->history_load = history_load;
    params->history_ptr = history_ptr;

    /* set OUT variables */
    *in_count -= (risamples / channels);
    *out_count -= (rosamples / channels);
}


/* input data in 8-bit unsigned or 16 bit signed, output data in 16bit samples
 * stored in 32bit integers.
 * The output samples are multiplied with left_factor respectively right_factor,
 * and scaled down by 4 bits (output samples right shifted by 4)
 *
 * To the factors are added left_factor_inc and right_factor_inc for each iteration.
 * The output samples are added to the values in the output array.
 *
 */
void SR_resample32_add(SR_ResampleParams* params, INT32 input_channels, INT32 input_sample_size,
		       INT32 left_factor, INT32 right_factor, /* volume factors */
		       INT32 left_factor_inc, INT32 right_factor_inc, /* volume factor increase */
		       void* input,
		       INT32* in_count, /* IN: number of input FRAMES, OUT: number of actually read FRAMES */
		       SR_INT32* output,
		       INT32* out_count) { /* IN: number of output FRAMES, OUT: number of actually written FRAMES */

    SR_INT16* input16; /* local copy of input as INT16 */
    SR_UINT8* input8;   /* local copy of input as INT8 */

    unsigned int	risamples;	/* number of remaining input samples */
    int			c;		/* Counter */
    unsigned int	k;		/* Counter */
    unsigned int	h_index;	/* Inner loop history index */
    unsigned int	rosamples;	/* number of remaining output samples */
    int			c_index;	/* Starting coefficient index */
    int			sum;		/* Output sample value */
    int			channels;	/* local channels */
    unsigned int	history_load;	/* local history_load */
    unsigned int 	history_size;	/* local size of history */
    int*		history;	/* local history */
    unsigned int	history_ptr;	/* local position in history */

    if ((input_channels != params->channels && input_channels*2 != params->channels)
	|| ((input_sample_size != 8) && (input_sample_size != 16))) {
#ifdef DEBUG
	printf("channels: input=%d, output=%d  |  sample size in bits: output=%d !\n",
	       (int) input_channels, (int) params->channels, (int) input_sample_size);
#endif
	return;
    }

    input16 = input;
    input8 = input;

    /* copy some vars in local variables */
    channels = params->channels;
    history_load = params->history_load;
    history_size = params->history_size;
    history = params->history;
    history_ptr = params->history_ptr;

    risamples = (*in_count) * input_channels;
    rosamples = (*out_count) * channels;

    while ((risamples>0 || history_load==0) && rosamples>0) {
	//printf("--risamples=%d   rosamples=%d  history_load=%d frames\n", risamples, rosamples, history_load);
	/* load history with new samples */
	if (input_channels*2==channels) {
	    /* Input mono, output stereo
	     * one input sample must yield 2 output samples.
	     * so double each input sample */
	    if (input_sample_size == 16) {
		/* 16 bit input samples */
		for (; history_load>0; history_load--) {
		    if (risamples==0) {
			/* revival of GOTO! */
			goto end_of_loop;
		    }
		    history[history_ptr++] = *input16;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    history[history_ptr++] = *input16;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    input16++; risamples--;
		}
	    } else {
		/* 8 bit input samples */
		for (; history_load>0; history_load--) {
		    if (risamples==0) {
			goto end_of_loop;
		    }
		    history[history_ptr++] = ((*input8)-0x80) << 8; //((SR_UINT8) (*input8-0x80)) << 8;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    history[history_ptr++] = ((*input8)-0x80) << 8; //((SR_UINT8) (*input8-0x80)) << 8;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    input8++; risamples--;
		}
	    }
	} else {
	    /* number input channels equals number of output channels */
	    if (input_sample_size == 16) {
		/* 16 bit input samples */
		for (; history_load>0; history_load--) {
		    if (risamples==0) {
			goto end_of_loop;
		    }
		    history[history_ptr++] = *input16;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    input16++; risamples--;
		    history[history_ptr++] = *input16;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    input16++; risamples--;
		}
	    } else {
		/* 8 bit input samples */
		for (; history_load>0; history_load--) {
		    if (risamples==0) {
			goto end_of_loop;
		    }
		    history[history_ptr++] = ((*input8)-0x80) << 8; //((SR_UINT8) (*input8-0x80)) << 8;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    input8++; risamples--;
		    history[history_ptr++] = ((*input8)-0x80) << 8; //((SR_UINT8) (*input8-0x80)) << 8;
		    if (history_ptr >= history_size) {
			history_ptr = 0;
		    }
		    input8++; risamples--;
		}
	    }
	}
	/* Generate output samples */
	for (c = 0; c < channels; c++) {
	    /* history index in inner loop */
	    h_index = (history_ptr - ((COFF_COUNT + 1) * channels)
		       + history_size + c) % history_size;

	    /* coefficient start index */
	    c_index = CPZC - (params->delta_c >>
	              (SR_INPUT_INDEX_SHIFT - CPZC_SHIFT));

	    sum = 0;
	    for (k = 0; k <= COFF_COUNT; k++) {
#ifdef DEBUG
		if (h_index >= history_size) {
		    printf("\nh_index>=history_size  %d>%d\n", (int) h_index, (int) history_size);
		} /*else
		    if (c_index >= sizeof(SincFuncTable)/sizeof(SR_INT16)) {
		    printf("\nc_index>=table_size  %d>%d\n", c_index, sizeof(SincFuncTable)/sizeof(SR_INT16)); fflush(stdout);
		    } */ else
#endif
			sum += (params->table[c_index] * history[h_index]);
		c_index += CPZC;
		h_index += channels;
		if (h_index >= history_size) {
		    h_index -= history_size;
		}
	    }

	    /* Write out */
	    sum=sum>>COFF_SHIFT;
	    if (c==0) {
		*output += (SR_INT32) ((sum*left_factor) >> 4);
	    } else {
		*output += (SR_INT32) ((sum*right_factor) >> 4);
	    }
	    output++;
	    rosamples--;
	}

	/* Increment counters */
	params->delta_c += params->csteps;
	history_load = (params->delta_c >> SR_INPUT_INDEX_SHIFT); /* FRAMES */
	params->delta_c &= SR_INPUT_INDEX_MASK;

	/* adjust volume factors */
	left_factor += left_factor_inc;
	right_factor += right_factor_inc;
    }
end_of_loop:
    /* save local copies for next time */
    params->history_load = history_load;
    params->history_ptr = history_ptr;

    /* set OUT variables */
    *in_count -= (risamples / input_channels);
    *out_count -= (rosamples / channels);
}


/* generation of the table: see end of this file */
static const SR_INT16 SincFuncTable[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3,
    3, 4, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20,
    22, 23, 24, 26, 27, 29, 30, 32, 34, 35, 37, 39, 40, 42, 44, 46, 47, 49, 51,
    53, 55, 57, 59, 60, 62, 64, 66, 68, 69, 71, 73, 75, 76, 78, 79, 81, 82, 84,
    85, 86, 88, 89, 90, 91, 92, 92, 93, 94, 94, 94, 95, 95, 95, 95, 94, 94, 93,
    93, 92, 91, 90, 88, 87, 85, 83, 81, 79, 77, 74, 71, 68, 65, 62, 58, 55, 51,
    47, 42, 38, 33, 28, 23, 17, 12, 6, 0, -6, -13, -19, -26, -33, -40, -48, -55,
    -63, -71, -80, -88, -97, -105, - 114, -123, -133, -142, -152, -161, -171, -
    181, -192, -202, -212, -223, -233, - 244, -255, -266, -277, - 288, -299, -
    310, -321, -333, -344, -355, -366, -378, - 389, -400, -411, -422, - 433, -
    444, -455, -466, -477, -487, -497, -508, -518, - 528, -537, -547, -556, -
    565, -574, -583, -591, -599, -607, -614, -621, -628, - 635, -641, -646, -
    652, - 657, -661, -665, -669, -672, -675, -678, -679, -681, - 682, -682, -
    682, -681, - 680, -678, -676, -673, -669, -665, -661, -655, -649, - 643, -
    636, -628, -620, - 610, -601, -590, -579, -568, -555, -542, -528, -514, -
    499, -483, -467, -450, - 432, -413, -394, -374, -354, -333, -311, -289, -
    265, - 242, -217, -192, -167, - 140, -113, -86, -58, -29, 0, 30, 60, 91,
    122, 154, 186, 219, 252, 286, 320, 355, 389, 425, 460, 496, 532, 569, 605,
    642, 679, 716, 754, 792, 829, 867, 905, 943, 980, 1018, 1056, 1094, 1131,
    1169, 1206, 1243, 1280, 1316, 1352, 1388, 1424, 1459, 1494, 1528, 1562,
    1595, 1628, 1660, 1691, 1722, 1753, 1782, 1811, 1839, 1866, 1892, 1918,
    1942, 1966, 1988, 2010, 2031, 2050, 2069, 2086, 2102, 2117, 2131, 2144,
    2155, 2165, 2173, 2181, 2187, 2191, 2194, 2196, 2196, 2195, 2192, 2188,
    2182, 2174, 2165, 2155, 2142, 2128, 2113, 2095, 2076, 2056, 2033, 2009,
    1983, 1956, 1926, 1895, 1862, 1828, 1791, 1753, 1713, 1672, 1628, 1583,
    1536, 1487, 1437, 1384, 1330, 1275, 1217, 1158, 1098, 1035, 971, 906, 838,
    769, 699, 627, 554, 479, 402, 325, 246, 165, 83, 0, -84, -170, - 257, -345,
    -434, -524, - 615, -707, -800, -894, -988, -1084, -1180, -1276, -1374, -
    1472, -1570, -1669, - 1768, -1868, -1967, -2067, -2168, -2268, -2368, -2468,
    - 2568, -2668, -2767, - 2867, -2965, -3064, -3162, -3259, -3355, -3451, -
    3546, - 3640, -3733, -3825, - 3916, -4006, -4094, -4181, -4267, -4351, -
    4433, -4514, - 4593, -4671, -4746, - 4819, -4891, -4960, -5027, -5092, -
    5155, -5215, -5272, - 5327, -5380, -5429, - 5476, -5520, -5561, -5599, -
    5634, -5666, -5695, -5720, - 5742, -5761, -5776, - 5788, -5796, -5800, -
    5801, -5798, -5791, -5780, -5765, - 5746, -5723, -5696, - 5665, -5630, -
    5590, -5546, -5498, -5446, -5389, -5327, - 5261, -5191, -5116, - 5036, -
    4952, -4863, -4770, -4671, -4568, -4461, -4349, - 4231, -4110, -3983, -
    3852, -3715, -3574, -3429, -3278, -3123, -2963, -2798, - 2628, -2454, -2275,
    - 2091, -1903, -1710, -1512, -1309, -1102, -891, -675, -454, -229, 0, 234,
    472, 714, 961, 1211, 1466, 1725, 1988, 2254, 2525, 2799, 3077, 3359, 3644,
    3933, 4226, 4521, 4820, 5122, 5427, 5735, 6047, 6360, 6677, 6997, 7318,
    7643, 7969, 8298, 8629, 8963, 9298, 9635, 9973, 10313, 10655, 10998, 11343,
    11688, 12035, 12382, 12731, 13080, 13429, 13779, 14130, 14480, 14831, 15182,
    15532, 15882, 16232, 16581, 16930, 17277, 17624, 17970, 18314, 18658, 18999,
    19340, 19678, 20015, 20350, 20682, 21013, 21341, 21667, 21990, 22311, 22628,
    22943, 23255, 23563, 23869, 24170, 24469, 24763, 25054, 25341, 25624, 25903,
    26178, 26448, 26714, 26976, 27233, 27485, 27732, 27974, 28211, 28444, 28671,
    28892, 29108, 29319, 29524, 29724, 29917, 30105, 30287, 30464, 30634, 30798,
    30955, 31107, 31252, 31391, 31524, 31650, 31769, 31882, 31988, 32088, 32181,
    32267, 32347, 32419, 32485, 32544, 32596, 32642, 32680, 32711, 32736, 32753,
    32764, 32767, 32764, 32753, 32736, 32711, 32680, 32642, 32596, 32544, 32485,
    32419, 32347, 32267, 32181, 32088, 31988, 31882, 31769, 31650, 31524, 31391,
    31252, 31107, 30955, 30798, 30634, 30464, 30287, 30105, 29917, 29724, 29524,
    29319, 29108, 28892, 28671, 28444, 28211, 27974, 27732, 27485, 27233, 26976,
    26714, 26448, 26178, 25903, 25624, 25341, 25054, 24763, 24469, 24170, 23869,
    23563, 23255, 22943, 22628, 22311, 21990, 21667, 21341, 21013, 20682, 20350,
    20015, 19678, 19340, 18999, 18658, 18314, 17970, 17624, 17277, 16930, 16581,
    16232, 15882, 15532, 15182, 14831, 14480, 14130, 13779, 13429, 13080, 12731,
    12382, 12035, 11688, 11343, 10998, 10655, 10313, 9973, 9635, 9298, 8963,
    8629, 8298, 7969, 7643, 7318, 6997, 6677, 6360, 6047, 5735, 5427, 5122,
    4820, 4521, 4226, 3933, 3644, 3359, 3077, 2799, 2525, 2254, 1988, 1725,
    1466, 1211, 961, 714, 472, 234, 0, -229, -454, -675, -891, -1102, -1309, -
    1512, -1710, - 1903, -2091, -2275, - 2454, -2628, -2798, -2963, -3123, -
    3278, -3429, -3574, - 3715, -3852, -3983, - 4110, -4231, -4349, -4461, -
    4568, -4671, -4770, -4863, - 4952, -5036, -5116, - 5191, -5261, -5327, -
    5389, -5446, -5498, -5546, -5590, - 5630, -5665, -5696, - 5723, -5746, -
    5765, -5780, -5791, -5798, -5801, -5800, - 5796, -5788, -5776, - 5761, -
    5742, -5720, -5695, -5666, -5634, -5599, -5561, - 5520, -5476, -5429, -
    5380, -5327, -5272, -5215, -5155, -5092, -5027, -4960, - 4891, -4819, -4746,
    - 4671, -4593, -4514, -4433, -4351, -4267, -4181, -4094, - 4006, -3916, -
    3825, - 3733, -3640, -3546, -3451, -3355, -3259, -3162, -3064, - 2965, -
    2867, -2767, - 2668, -2568, -2468, -2368, -2268, -2168, -2067, -1967, -
    1868, -1768, -1669, - 1570, -1472, -1374, -1276, -1180, -1084, -988, -894, -
    800, -707, -615, -524, - 434, -345, -257, -170, -84, 0, 83, 165, 246, 325,
    402, 479, 554, 627, 699, 769, 838, 906, 971, 1035, 1098, 1158, 1217, 1275,
    1330, 1384, 1437, 1487, 1536, 1583, 1628, 1672, 1713, 1753, 1791, 1828,
    1862, 1895, 1926, 1956, 1983, 2009, 2033, 2056, 2076, 2095, 2113, 2128,
    2142, 2155, 2165, 2174, 2182, 2188, 2192, 2195, 2196, 2196, 2194, 2191,
    2187, 2181, 2173, 2165, 2155, 2144, 2131, 2117, 2102, 2086, 2069, 2050,
    2031, 2010, 1988, 1966, 1942, 1918, 1892, 1866, 1839, 1811, 1782, 1753,
    1722, 1691, 1660, 1628, 1595, 1562, 1528, 1494, 1459, 1424, 1388, 1352,
    1316, 1280, 1243, 1206, 1169, 1131, 1094, 1056, 1018, 980, 943, 905, 867,
    829, 792, 754, 716, 679, 642, 605, 569, 532, 496, 460, 425, 389, 355, 320,
    286, 252, 219, 186, 154, 122, 91, 60, 30, 0, -29, -58, -86, -113, -140, -
    167, -192, - 217, -242, -265, -289, -311, -333, -354, -374, -394, - 413, -
    432, -450, -467, - 483, -499, -514, -528, -542, -555, -568, -579, -590, -
    601, -610, -620, -628, - 636, -643, -649, -655, -661, -665, -669, -673, -
    676, - 678, -680, -681, -682, - 682, -682, -681, -679, -678, -675, -672, -
    669, -665, - 661, -657, -652, -646, - 641, -635, -628, -621, -614, -607, -
    599, -591, -583, - 574, -565, -556, -547, - 537, -528, -518, -508, -497, -
    487, -477, -466, -455, - 444, -433, -422, -411, - 400, -389, -378, -366, -
    355, -344, -333, -321, -310, - 299, -288, -277, -266, - 255, -244, -233, -
    223, -212, -202, -192, -181, -171, - 161, -152, -142, -133, - 123, -114, -
    105, -97, -88, -80, -71, -63, -55, -48, -40, -33, -26, -19, -13, -6, 0, 6,
    12, 17, 23, 28, 33, 38, 42, 47, 51, 55, 58, 62, 65, 68, 71, 74, 77, 79, 81,
    83, 85, 87, 88, 90, 91, 92, 93, 93, 94, 94, 95, 95, 95, 95, 94, 94, 94, 93,
    92, 92, 91, 90, 89, 88, 86, 85, 84, 82, 81, 79, 78, 76, 75, 73, 71, 69, 68,
    66, 64, 62, 60, 59, 57, 55, 53, 51, 49, 47, 46, 44, 42, 40, 39, 37, 35, 34,
    32, 30, 29, 27, 26, 24, 23, 22, 20, 19, 18, 17, 15, 14, 13, 12, 11, 10, 10,
    9, 8, 7, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0 };

int SR_change_samplerate(SR_ResampleParams* params,
			 unsigned int new_input_samplerate,
			 unsigned int new_output_samplerate) {
    params->input_samplerate = new_input_samplerate;
    params->output_samplerate = new_output_samplerate;

    params->csteps = (unsigned int) ((((UINT64) new_input_samplerate) << SR_INPUT_INDEX_SHIFT)
                     / new_output_samplerate);
    if (params->csteps<=0) {
#ifdef DEBUG
	//fprintf(stderr, "output sample rate of %d is too large!\n", output_samplerate);
#endif
	return 0;
    }
    return 1;
}


/* returns 0 on error */
int SR_init(SR_ResampleParams* params,
	    unsigned int input_samplerate,
	    unsigned int output_samplerate,
	    unsigned int channels,
	    unsigned int sample_size_in_bits) {
    /* Initialise */
    params->input_samplerate = input_samplerate;
    params->output_samplerate = output_samplerate;
    params->channels = channels;
    params->sample_size = sample_size_in_bits;

    params->table = (SR_INT16*) SincFuncTable;

    params->channels = channels;

#ifdef DEBUG
    printf("SincResample.init(input samplerate = %d, output samplerate = %d) \n", input_samplerate, output_samplerate);
#endif
    if (!SR_change_samplerate(params, input_samplerate, output_samplerate)) {
	return 0;
    }

    /* set up history */
    params->history_size = ((params->csteps >> SR_INPUT_INDEX_SHIFT)+1+COFF_COUNT)*channels; /* in samples */
#ifdef DEBUG
    printf("     csteps=%f  history_size=%d\n",
           ((float) params->csteps)/SR_INPUT_INDEX_FACTOR,
           (int) params->history_size);
#endif
    params->history = (int*) malloc(params->history_size*sizeof(int));
    if (params->history==NULL) {
	return 0;
    }
    memset(params->history, 0, params->history_size*sizeof(int));

    /* initialize runtime parameters */
    params->delta_c = 0;
    params->history_ptr=0;

    /* preload history with 1 frame */
    params->history_load=channels;

    /* everything fine */
    return 1;
}


void SR_resample(SR_ResampleParams* params,
		 void* input,
		 unsigned int* in_count, /* IN: number of input FRAMES, OUT: number of actually read FRAMES */
		 void* output,
		 unsigned int* out_count) { /* IN: number of output FRAMES, OUT: number of actually written FRAMES */
    if (params->sample_size==16) {
	SR_resample16(params, (SR_INT16*) input, in_count, (SR_INT16*) output, out_count);
    }
}


void SR_exit(SR_ResampleParams* params) {
#ifdef DEBUG
    printf("SincResample.exit(input samplerate = %d, output samplerate = %d) \n", params->input_samplerate, params->output_samplerate);
#endif
    if (params->history) {
	free(params->history);
	params->history=NULL;
    }
}

#if 0
/* ***************************** table generation ***************************** */
/* Return sinc(x) */
double
    sinc(double x) {
    return (sin(M_PI * x) / (M_PI * x));
}

/* Return hann(x) */
double
    hann(double x, unsigned int l) {
    return (0.5 + (1 - 0.5) * cos(M_PI * (x / l)));
}

/* Compute table of filter coefficients */
void gen_table(unsigned int nzcs, unsigned int cpzc, SR_INT16 **table) {
    double		dc;	/* Step between coefficients */
    double		coff;	/* Coefficient value */
    short		*ctre;	/* Pointer to central coefficient */
    short		*start;	/* Pointer to first coefficient */
    short		*cp;	/* Pointer to current coefficient */
    unsigned int	ncoffs; /* Number of coefficients */
    unsigned int	i;	/* Counter */

    /* Number of coefficients */
    ncoffs = (2 * nzcs * cpzc) + 1;

    /* Distance between coefficients */
    dc = 1.0 / cpzc;

    /*
     * Allocate room for coefficients. Note we allocate
     * more room than is necessary to increase efficiency
     * when filtering the input signal.
     */
    *table = (short *)malloc(sizeof (short) * (ncoffs + cpzc));

    /* Initialise all table entries to zero */
    (void) memset(*table, 0, sizeof (short) * (ncoffs + cpzc));

    /* Set up pointers */
    start = *table + cpzc;
    ctre = start + ((ncoffs - 1) / 2);
    cp = ctre;

    /* Fill in table */
    for (i = 0; i <= ((ncoffs - 1) / 2); i++) {
	coff = sinc(i * dc) * hann(i * dc, nzcs);
	*cp = (short)rint(coff * ((1 << COFF_SHIFT) - 1));
	*(cp - 2 * i) = *cp;
	cp++;
    }

    /* Set central coefficient to max */
    *ctre = ((1 << COFF_SHIFT) - 1);

    /* print table */
    for (i=0; i<ncoffs+cpzc; i++) {
	printf("%d, ", (*table)[i]);
    }
}
#endif
