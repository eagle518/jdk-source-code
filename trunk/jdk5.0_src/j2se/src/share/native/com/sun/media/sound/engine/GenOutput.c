/*
 * @(#)GenOutput.c	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenOutput.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**	Confidential-- Internal use only
**
**
** Modification History:
**
**	4/21/96		Removed register usage in parameters
**	11/21/96	Added PHASE_OFFSET macro to allow for zero based 8 bit DACs
**	12/30/96	Changed copyright
**	1/22/97		Added support for MOD code in all output loops
**	6/10/97		Moved MOD support code to GenSynth.c prior to output stage
**	7/28/97		Merged Sun timing test code
**	11/10/97	Flipped phase of 8 bit output for Solaris
**	12/16/97	Moe: removed compiler warnings
**	2/3/98		Renamed songBufferLeftMono to songBufferDry
**	2/11/98		Added support for Q_24K & Q_48K & Q_8K
**	2/13/98		Removed sun's timing code
**	2/15/98		Added more obvious support for 11 terped to 22 and 22 terped to 44
**	11/23/98	Added support for Intel Katmai CPU
**	1/4/99		Re ordered Katmai code and duplicated inner loops
*/
/*****************************************************************************/

/* header for gethrvtime()					Liang */
#ifdef VIS_TIMING
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#define NITER 1000000
#endif

#ifdef USE_VIS
#include "VIS_JavaSound.h"
#endif

#include "GenSnd.h"
#include "GenPriv.h"

#undef KATMAI
#if (X_PLATFORM == X_WINDOWS) && USE_KAT == 1

// KATMAI support
#include "xmmintrin.h"
#define KATMAI			1
//#define USE_KATMAI_WRITE_CACHE
#endif

// This is 8 bit phase. Some hardware wants silence to be 0, some wants it
// to be 128.
#if ( (X_PLATFORM == X_BE) 				||	\
	  (X_PLATFORM == X_SOLARIS) )
#define PHASE_OFFSET	0		// silence is 0
#else
#define PHASE_OFFSET	0x80	// silence is 128
#endif

#if (USE_8_BIT_OUTPUT == TRUE) && (USE_STEREO_OUTPUT == TRUE)
void PV_Generate8outputStereo(OUTSAMPLE8 FAR * dest8)
{
    register INT32			b, c;
    register INT32			*sourceLR;
    register LOOPCOUNT		count;

#ifdef VIS_TIMING 
    {
	INT32      *sourceLsave = sourceL;
	OUTSAMPLE8 *dest8save   = dest8;
	hrtime_t start, end;
	double   vtime;
	int      k;
	start = gethrvtime();
	for (k = 0; k < NITER; k ++) {
	    sourceL = sourceLsave;
	    dest8   = dest8save;
#endif

	    /* Convert intermediate 16-bit sample format to 16 bit output samples:
	     */
	    sourceLR = &MusicGlobals->songBufferDry[0];

	    if ( (MusicGlobals->outputQuality != Q_11K_TERP_22K) && (MusicGlobals->outputQuality != Q_22K_TERP_44K) )
		{
		    // 22k & 44k
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_ShiftOffsetDuplicate_U8_S32_Sat(dest8, (int*)sourceLR, MusicGlobals->Four_Loop<<3, OUTPUT_SCALAR+8, PHASE_OFFSET);
#else
		    // native sample rates
		    for (count = MusicGlobals->Four_Loop; count > 0; --count)
			{
			    dest8[0] = (sourceLR[0] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[1] = (sourceLR[1] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[2] = (sourceLR[2] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[3] = (sourceLR[3] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[4] = (sourceLR[4] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[5] = (sourceLR[5] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[6] = (sourceLR[6] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[7] = (sourceLR[7] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8 += 8;
			    sourceLR += 8;
			}
#endif
		}
	    else
		{                   
		    // 11k
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_ShiftOffsetDuplicate_U8_S32_Sat(dest8, (int*)sourceLR, MusicGlobals->Four_Loop<<3, OUTPUT_SCALAR+8, PHASE_OFFSET);
#else
		    // 11k terped to 22k, and 22k terped to 44k
		    for (count = MusicGlobals->Four_Loop; count > 0; --count)
			{
			    b =  (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    c =  (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[0] = (OUTSAMPLE8)b;
			    dest8[1] = (OUTSAMPLE8)c;
			    dest8[2] = (OUTSAMPLE8)b;
			    dest8[3] = (OUTSAMPLE8)c;

			    b = (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    c = (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[4] = (OUTSAMPLE8)b;
			    dest8[5] = (OUTSAMPLE8)c;
			    dest8[6] = (OUTSAMPLE8)b;
			    dest8[7] = (OUTSAMPLE8)c;

			    b = (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    c = (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[8] = (OUTSAMPLE8)b;
			    dest8[9] = (OUTSAMPLE8)c;
			    dest8[10] = (OUTSAMPLE8)b;
			    dest8[11] = (OUTSAMPLE8)c;

			    b = (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    c = (*sourceLR++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[12] = (OUTSAMPLE8)b;
			    dest8[13] = (OUTSAMPLE8)c;
			    dest8[14] = (OUTSAMPLE8)b;
			    dest8[15] = (OUTSAMPLE8)c;
			    dest8 += 16;
			}
#endif
		}

#ifdef VIS_TIMING 
	}
	end = gethrvtime();
	vtime = (double) (end - start);
	printf("\n");
	printf("PV_Generate8outputStereo\n");
	printf("   Samples = %3d\n", MusicGlobals->Four_Loop<<2);
	switch (MusicGlobals->outputQuality) {
	case Q_44K: printf("   Quality =  44 KHz\n"); break;
	case Q_22K: printf("   Quality =  22 KHz\n"); break;
	case Q_11K: printf("   Quality =  11 KHz\n"); break;
	}
	printf("     Loops =  %d\n", NITER);
#ifdef USE_VIS
	printf("  VIS Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("  VIS Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#else
	printf("    C Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("    C Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#endif
	printf("\n");
	exit(2);
    }
#endif
}
#endif	// (USE_8_BIT_OUTPUT == TRUE) && (USE_STEREO_OUTPUT == TRUE)

#if (USE_8_BIT_OUTPUT == TRUE) && (USE_MONO_OUTPUT == TRUE)
void PV_Generate8outputMono(OUTSAMPLE8 FAR * dest8)
{
    register LOOPCOUNT		count;
    register INT32	b;
    register INT32 *source;

    /* Convert intermediate 16-bit sample format to 16 bit output samples:
     */
    source = &MusicGlobals->songBufferDry[0];

#ifdef VIS_TIMING 
    {
	INT32      *sourcesave = source;
	OUTSAMPLE8 *dest8save  = dest8;
	hrtime_t start, end;
	double   vtime;
	int      k;
	start = gethrvtime();
	for (k = 0; k < NITER; k ++) {
	    source = sourcesave;
	    dest8  = dest8save;
#endif
	    /* Here's how to add a buzz if we want to make sure the Sound Manager's alive.
	     */
	    //		source[0] += 0x60;
	    if ( (MusicGlobals->outputQuality != Q_11K_TERP_22K) && (MusicGlobals->outputQuality != Q_22K_TERP_44K) )
		{
		    // 22k & 44k
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_ShiftOffset_U8_S32_Sat(dest8, (int*)source, MusicGlobals->Four_Loop<<2, OUTPUT_SCALAR+8, PHASE_OFFSET);
#else
		    // native sample rates
		    for (count = MusicGlobals->Four_Loop; count > 0; --count)
			{
			    dest8[0] = (source[0] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[1] = (source[1] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[2] = (source[2] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[3] = (source[3] >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    source += 4;
			    dest8 += 4;
			}
#endif
		}
	    else
		{                   
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_ShiftOffsetDuplicate_U8_S32_Sat(dest8, (int*)source, MusicGlobals->Four_Loop<<2, OUTPUT_SCALAR+8, PHASE_OFFSET);
#else
		    // 11k terped to 22k, and 22k terped to 44k
		    for (count = MusicGlobals->Four_Loop; count > 0; --count)
			{
			    b = (*source++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[0] = (OUTSAMPLE8)b;
			    dest8[1] = (OUTSAMPLE8)b;

			    b = (*source++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[2] = (OUTSAMPLE8)b;
			    dest8[3] = (OUTSAMPLE8)b;

			    b = (*source++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[4] = (OUTSAMPLE8)b;
			    dest8[5] = (OUTSAMPLE8)b;

			    b = (*source++ >> (OUTPUT_SCALAR + 8)) + PHASE_OFFSET;
			    dest8[6] = (OUTSAMPLE8)b;
			    dest8[7] = (OUTSAMPLE8)b;
			    dest8 += 8;
			}
#endif
		}

#ifdef VIS_TIMING 
	}
	end = gethrvtime();
	vtime = (double) (end - start);
	printf("\n");
	printf("PV_Generate8outputMono\n");
	printf("   Samples = %3d\n", MusicGlobals->Four_Loop<<2);
	switch (MusicGlobals->outputQuality) {
	case Q_44K: printf("   Quality =  44 KHz\n"); break;
	case Q_22K: printf("   Quality =  22 KHz\n"); break;
	case Q_11K: printf("   Quality =  11 KHz\n"); break;
	}
	printf("     Loops =  %d\n", NITER);
#ifdef USE_VIS
	printf("  VIS Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("  VIS Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#else
	printf("    C Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("    C Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#endif
	printf("\n");
	exit(2);
    }
#endif
}
#endif	// (USE_8_BIT_OUTPUT == TRUE) && (USE_MONO_OUTPUT == TRUE)

#if (USE_16_BIT_OUTPUT == TRUE) && (USE_STEREO_OUTPUT == TRUE)
void PV_Generate16outputStereo(OUTSAMPLE16 FAR * dest16)
{
    register LOOPCOUNT	count;
#if X_PLATFORM != X_WEBTV
    register INT32		b, c;
#endif
    register INT32		*sourceLR;
    register INT32		i, overflow_test, k8000;

    /* Convert intermediate 16-bit sample format to 16 bit output samples:
     */
#ifdef VIS_TIMING 
    {
	INT32       *sourceLsave = sourceLR;
	OUTSAMPLE16 *dest16save  = dest16;
	hrtime_t start, end;
	double   vtime;
	int      k;
	start = gethrvtime();
	for (k = 0; k < NITER; k ++) {
	    sourceL = sourceLsave;
	    dest16  = dest16save;
#endif
	    sourceLR = &MusicGlobals->songBufferDry[0];

	    k8000 = 0x8000;
	    if ( (MusicGlobals->outputQuality != Q_11K_TERP_22K) && (MusicGlobals->outputQuality != Q_22K_TERP_44K) )
		{
		    // 22k & 44k
		    // 22k & 44k
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_Shift_S16_S32_Sat(dest16, (int*)sourceLR, MusicGlobals->Four_Loop<<3, OUTPUT_SCALAR);
#else

#ifdef KATMAI
		    if (MusicGlobals->useKatmaiCPU)
			{
			    XFIXED				next_frame;

			    next_frame = 8 * MusicGlobals->Four_Loop;
			    _mm_prefetch((char *)sourceLR, _MM_HINT_NTA);
#ifndef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest16, _MM_HINT_NTA);
#endif

			    // native sample rates
			    for (count = MusicGlobals->Four_Loop; count > 0; --count)
				{
				    _mm_prefetch((char *)sourceLR + next_frame, _MM_HINT_NTA);
#ifndef USE_KATMAI_WRITE_CACHE
				    _mm_prefetch((char *)dest16, _MM_HINT_NTA);
#endif

				    i = sourceLR[0] >> OUTPUT_SCALAR;
				    overflow_test = i + k8000;
				    dest16[0] = (OUTSAMPLE16)i;
				    i = sourceLR[1] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[1] = (OUTSAMPLE16)i;

				    i = sourceLR[2] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[2] = (OUTSAMPLE16)i;
				    i = sourceLR[3] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[3] = (OUTSAMPLE16)i;

				    i = sourceLR[4] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[4] = (OUTSAMPLE16)i;
				    i = sourceLR[5] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[5] = (OUTSAMPLE16)i;

				    i = sourceLR[6] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[6] = (OUTSAMPLE16)i;
				    i = sourceLR[7] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[7] = (OUTSAMPLE16)i;

				    if (overflow_test & 0xFFFF0000)
					{
					    i = sourceLR[0] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[0] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[1] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[1] = (OUTSAMPLE16)(i - k8000);
		
					    i = sourceLR[2] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[2] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[3] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[3] = (OUTSAMPLE16)(i - k8000);
		
					    i = sourceLR[4] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[4] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[5] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[5] = (OUTSAMPLE16)(i - k8000);
		
					    i = sourceLR[6] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[6] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[7] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[7] = (OUTSAMPLE16)(i - k8000);
					}
				    sourceLR += 8; dest16 += 8;
				}
			}
		    else
#endif	// KATMAI
			{
			    // native sample rates
			    for (count = MusicGlobals->Four_Loop; count > 0; --count)
				{
				    i = sourceLR[0] >> OUTPUT_SCALAR;
				    overflow_test = i + k8000;
				    dest16[0] = (OUTSAMPLE16)i;
				    i = sourceLR[1] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[1] = (OUTSAMPLE16)i;

				    i = sourceLR[2] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[2] = (OUTSAMPLE16)i;
				    i = sourceLR[3] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[3] = (OUTSAMPLE16)i;

				    i = sourceLR[4] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[4] = (OUTSAMPLE16)i;
				    i = sourceLR[5] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[5] = (OUTSAMPLE16)i;

				    i = sourceLR[6] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[6] = (OUTSAMPLE16)i;
				    i = sourceLR[7] >> OUTPUT_SCALAR;
				    overflow_test |= i + k8000;
				    dest16[7] = (OUTSAMPLE16)i;

				    if (overflow_test & 0xFFFF0000)
					{
					    i = sourceLR[0] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[0] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[1] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[1] = (OUTSAMPLE16)(i - k8000);
		
					    i = sourceLR[2] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[2] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[3] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[3] = (OUTSAMPLE16)(i - k8000);
		
					    i = sourceLR[4] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[4] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[5] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[5] = (OUTSAMPLE16)(i - k8000);
		
					    i = sourceLR[6] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[6] = (OUTSAMPLE16)(i - k8000);
					    i = sourceLR[7] >> OUTPUT_SCALAR;
					    i += k8000;
					    if (i & 0xFFFF0000)
						{ if (i > 0) i = 0xFFFF; else i = 0;}
					    dest16[7] = (OUTSAMPLE16)(i - k8000);
					}
				    sourceLR += 8; dest16 += 8;
				}
			}
#endif
		}
#if X_PLATFORM != X_WEBTV
	    else
		{                   
		    // 11k terped to 22k, and 22k terped to 44k
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_ShiftDuplicate_S16_S32_Sat(dest16, (int*)sourceLR, MusicGlobals->Four_Loop<<3, OUTPUT_SCALAR);
#else
		    for (count = MusicGlobals->Four_Loop; count > 0; --count)
			{
			    b = *sourceLR++ >> OUTPUT_SCALAR;
			    b += k8000;
			    if (b & 0xFFFF0000)
				{ if (b > 0) b = 0xFFFF; else b = 0;}
			    b -= k8000;
			    c = *sourceLR++ >> OUTPUT_SCALAR;
			    c += k8000;
			    if (c & 0xFFFF0000)
				{ if (c > 0) c = 0xFFFF; else c = 0;}
			    c -= k8000;
			    dest16[0] = (OUTSAMPLE16)b;
			    dest16[1] = (OUTSAMPLE16)c; 
			    dest16[2] = (OUTSAMPLE16)b;
			    dest16[3] = (OUTSAMPLE16)c; 

			    b = *sourceLR++ >> OUTPUT_SCALAR;
			    b += k8000;
			    if (b & 0xFFFF0000)
				{ if (b > 0) b = 0xFFFF; else b = 0;}
			    b -= k8000;
			    c = *sourceLR++ >> OUTPUT_SCALAR;
			    c += k8000;
			    if (c & 0xFFFF0000)
				{ if (c > 0) c = 0xFFFF; else c = 0;}
			    c -= k8000;
			    dest16[4] = (OUTSAMPLE16)b;
			    dest16[5] = (OUTSAMPLE16)c; 
			    dest16[6] = (OUTSAMPLE16)b;
			    dest16[7] = (OUTSAMPLE16)c; 

			    b = *sourceLR++ >> OUTPUT_SCALAR;
			    b += k8000;
			    if (b & 0xFFFF0000)
				{ if (b > 0) b = 0xFFFF; else b = 0;}
			    b -= k8000;
			    c = *sourceLR++ >> OUTPUT_SCALAR;
			    c += k8000;
			    if (c & 0xFFFF0000)
				{ if (c > 0) c = 0xFFFF; else c = 0;}
			    c -= k8000;
			    dest16[8] = (OUTSAMPLE16)b;
			    dest16[9] = (OUTSAMPLE16)c; 
			    dest16[10] = (OUTSAMPLE16)b;
			    dest16[11] = (OUTSAMPLE16)c; 

			    b = *sourceLR++ >> OUTPUT_SCALAR;
			    b += k8000;
			    if (b & 0xFFFF0000)
				{ if (b > 0) b = 0xFFFF; else b = 0;}
			    b -= k8000;

			    c = *sourceLR++ >> OUTPUT_SCALAR;
			    c += k8000;
			    if (c & 0xFFFF0000)
				{ if (c > 0) c = 0xFFFF; else c = 0;}
			    c -= k8000;
			    dest16[12] = (OUTSAMPLE16)b;
			    dest16[13] = (OUTSAMPLE16)c; 
			    dest16[14] = (OUTSAMPLE16)b;
			    dest16[15] = (OUTSAMPLE16)c; 

			    dest16 += 16;
			}
#endif
		}
#endif	// X_PLATFORM != X_WEBTV

#ifdef VIS_TIMING 
	}
	end = gethrvtime();
	vtime = (double) (end - start);
	printf("\n");
	printf("PV_Generate16outputStereo\n");
	printf("   Samples = %3d\n", MusicGlobals->Four_Loop<<2);
	switch (MusicGlobals->outputQuality) {
	case Q_44K: printf("   Quality =  44 KHz\n"); break;
	case Q_22K: printf("   Quality =  22 KHz\n"); break;
	case Q_11K: printf("   Quality =  11 KHz\n"); break;
	}
	printf("     Loops =  %d\n", NITER);
#ifdef USE_VIS
	printf("  VIS Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("  VIS Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#else
	printf("    C Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("    C Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#endif
	printf("\n");
	exit(2);
    }
#endif
}
#endif	// (USE_16_BIT_OUTPUT == TRUE) && (USE_STEREO_OUTPUT == TRUE)

#if (USE_16_BIT_OUTPUT == TRUE) && (USE_MONO_OUTPUT == TRUE)
void PV_Generate16outputMono(OUTSAMPLE16 FAR * dest16)
{
    register LOOPCOUNT	count;
    register INT32		i;
    register INT32		*source;
    register INT32		overflow_test;
    register INT32		k8000 = 0x8000;

    /* Convert intermediate 16-bit sample format to 16 bit output samples:
     */
#ifdef VIS_TIMING 
    {
	INT32       *sourcesave = source;
	OUTSAMPLE16 *dest16save = dest16;
	hrtime_t start, end;
	double   vtime;
	int      k;
	start = gethrvtime();
	for (k = 0; k < NITER; k ++) {
	    source = sourcesave;
	    dest16 = dest16save;
#endif
	    source = &MusicGlobals->songBufferDry[0];

	    /* Here's how to add a buzz if we want to make sure the Sound Manager's alive.
	     */
	    //		source[0] += 0x60;
	    if ( (MusicGlobals->outputQuality != Q_11K_TERP_22K) && (MusicGlobals->outputQuality != Q_22K_TERP_44K) )
		{
		    // native sample rates
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_Shift_S16_S32_Sat(dest16, (int*)source, MusicGlobals->Four_Loop<<2, OUTPUT_SCALAR);
#else
		    for (count = MusicGlobals->Four_Loop; count > 0; --count)
			{
			    i = source[0] >> OUTPUT_SCALAR;
			    overflow_test = i + k8000;
			    dest16[0] = (OUTSAMPLE16)i;

			    i = source[1] >> OUTPUT_SCALAR;
			    overflow_test |= i + k8000;
			    dest16[1] = (OUTSAMPLE16)i;

			    i = source[2] >> OUTPUT_SCALAR;
			    overflow_test |= i + k8000;
			    dest16[2] = (OUTSAMPLE16)i;

			    i = source[3] >> OUTPUT_SCALAR;
			    overflow_test |= i + k8000;
			    dest16[3] = (OUTSAMPLE16)i;

			    if (overflow_test & 0xFFFF0000)
				{
				    i = source[0] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[0] = (OUTSAMPLE16)(i - k8000);
	
				    i = source[1] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[1] = (OUTSAMPLE16)(i - k8000);
	
				    i = source[2] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[2] = (OUTSAMPLE16)(i - k8000);
	
				    i = source[3] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[3] = (OUTSAMPLE16)(i - k8000);
				}
			    source += 4; dest16 += 4;
			}
#endif
		}
	    else
		{                   
		    // 11k terped to 22k, and 22k terped to 44k
#ifdef USE_VIS
		    // use VIS version.				-Liang
		    VIS_ShiftDuplicate_S16_S32_Sat(dest16, (int*)source, MusicGlobals->Four_Loop<<2, OUTPUT_SCALAR);
#else
		    for (count = MusicGlobals->Four_Loop; count > 0; --count)
			{
			    i = source[0] >> OUTPUT_SCALAR;
			    overflow_test = i + k8000;
			    dest16[0] = (OUTSAMPLE16)i;
			    dest16[1] = (OUTSAMPLE16)i;

			    i = source[1] >> OUTPUT_SCALAR;
			    overflow_test |= i + k8000;
			    dest16[2] = (OUTSAMPLE16)i;
			    dest16[3] = (OUTSAMPLE16)i;

			    i = source[2] >> OUTPUT_SCALAR;
			    overflow_test |= i + k8000;
			    dest16[4] = (OUTSAMPLE16)i;
			    dest16[5] = (OUTSAMPLE16)i;

			    i = source[3] >> OUTPUT_SCALAR;
			    overflow_test |= i + k8000;
			    dest16[6] = (OUTSAMPLE16)i;
			    dest16[7] = (OUTSAMPLE16)i;

			    if (overflow_test & 0xFFFF0000)
				{
				    i = source[0] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[0] = (OUTSAMPLE16)(i - k8000);
				    dest16[1] = (OUTSAMPLE16)(i - k8000);
	
				    i = source[1] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[2] = (OUTSAMPLE16)(i - k8000);
				    dest16[3] = (OUTSAMPLE16)(i - k8000);
	
				    i = source[2] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[4] = (OUTSAMPLE16)(i - k8000);
				    dest16[5] = (OUTSAMPLE16)(i - k8000);
	
				    i = source[3] >> OUTPUT_SCALAR;
				    i += k8000;
				    if (i & 0xFFFF0000)
					{ if (i > 0) i = 0xFFFF; else i = 0;}
				    dest16[6] = (OUTSAMPLE16)(i - k8000);
				    dest16[7] = (OUTSAMPLE16)(i - k8000);
				}
			    source += 4; dest16 += 8;
			}
#endif
		}

#ifdef VIS_TIMING 
	}
	end = gethrvtime();
	vtime = (double) (end - start);
	printf("\n");
	printf("PV_Generate16outputMono\n");
	printf("   Samples = %3d\n", MusicGlobals->Four_Loop<<2);
	switch (MusicGlobals->outputQuality) {
	case Q_44K: printf("   Quality =  44 KHz\n"); break;
	case Q_22K: printf("   Quality =  22 KHz\n"); break;
	case Q_11K: printf("   Quality =  11 KHz\n"); break;
	}
	printf("     Loops =  %d\n", NITER);
#ifdef USE_VIS
	printf("  VIS Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("  VIS Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("  VIS Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#else
	printf("    C Time = %10.6f msec\n", 
	       vtime/((double)NITER*1000000.0));
	printf("    C Time = %10.6f nsec/sample\n", 
	       vtime/((double)NITER*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f cycles/sample (on Ultra 1/170)\n",
	       vtime/((double)NITER*6*(MusicGlobals->Four_Loop<<2)));
	printf("    C Time = %10.6f sec (total)\n", vtime/(1000000000.0));
#endif
	printf("\n");
	exit(2);
    }
#endif
}
#endif	// (USE_16_BIT_OUTPUT == TRUE) && (USE_MONO_OUTPUT == TRUE)

// EOF of GenOutput.c

