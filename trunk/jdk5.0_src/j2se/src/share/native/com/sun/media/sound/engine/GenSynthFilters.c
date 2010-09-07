/*
 * @(#)GenSynthFilters.c	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**
** "GenSynthFilters.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Modification History:
**
**	1/18/96		Spruced up for C++ extra error checking
**				Changed the macro 'THE_CHECK' to accept a type for typecasting 
**				the source pointer
**	3/1/96		Removed extra PV_DoCallBack, and PV_GetWavePitch
**	5/2/96		Changed int to BOOL_FLAG
**	6/30/96		Changed false to FALSE
**				Changed font and re tabbed
**	7/6/96		Fixed compiler warnings
**				Reduced headspace for 16 bit samples for low pass filters
**	7/8/96		Improved enveloping and wave shaping code
**	12/30/96	Changed copyright
**	6/4/97		Added USE_SMALL_MEMORY_REVERB tests around code to disable when this
**				flag is used
**	12/16/97	Moe: removed compiler warnings
**	2/3/98		Renamed songBufferLeftMono to songBufferDry
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/20/98		now support variable send chorus as well as reverb
**	11/23/98	Added support for Intel Katmai CPU
**	1/4/99		Removed FAR macro. Re ordered Katmai code and duplicated inner loops
**	2002-03-14	removed compiler warning
*/
/*****************************************************************************/

#include "GenSnd.h"
#include "GenPriv.h"

#undef KATMAI
#if (X_PLATFORM == X_WINDOWS) && USE_KAT == 1

// KATMAI support
#include "xmmintrin.h"
#define KATMAI			1
//#define USE_KATMAI_WRITE_CACHE
#endif

#define CLIP(LIMIT_VAR, LIMIT_LOWER, LIMIT_UPPER) if (LIMIT_VAR < LIMIT_LOWER) LIMIT_VAR = LIMIT_LOWER; if (LIMIT_VAR > LIMIT_UPPER) LIMIT_VAR = LIMIT_UPPER;
#define GET_FILTER_PARAMS \
	CLIP (this_voice->LPF_frequency, 0x200, MAXRESONANCE*256);	\
	if (this_voice->previous_zFrequency == 0)\
		this_voice->previous_zFrequency = this_voice->LPF_frequency;\
	CLIP (this_voice->LPF_resonance, 0, 0x100);\
	CLIP (this_voice->LPF_lowpassAmount, -0xFF, 0xFF);\
	Z1 = this_voice->LPF_lowpassAmount << 8;\
	if (Z1 < 0)\
		Xn = 65536 + Z1;\
	else\
		Xn = 65536 - Z1;\
	if (Z1 >= 0)\
	{\
		Zn = ((0x10000 - Z1) * this_voice->LPF_resonance) >> 8;\
		Zn = -Zn;\
	}\
	else\
		Zn = 0;



// $$kk: 04.19.99
// void PV_ServeInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping)
void PV_ServeInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*destL;
    register UBYTE 			*source;
    register UBYTE			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitudeL;
    register INT32			inner;

    INT32					amplitudeLincrement;
    INT32					ampValueL;
    INT32					a;

    register INT16			*z;
    register INT32			Z1value, zIndex1, zIndex2, Xn, Z1, Zn, sample;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel > 1 || this_voice->chorusLevel > 1)
	{		
	    // $$kk: 04.19.99
	    // PV_ServeInterp2FilterPartialBufferNewReverb (this_voice, looping); 
	    PV_ServeInterp2FilterPartialBufferNewReverb (this_voice, looping, threadContext); 
	    return;
	}
#endif
    z = this_voice->z;
    Z1value = this_voice->Z1value;
    zIndex2 = this_voice->zIndex;

    GET_FILTER_PARAMS

	amplitudeL = this_voice->lastAmplitudeL;
    ampValueL = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeLincrement = (ampValueL - amplitudeL) / MusicGlobals->Four_Loop;
	
    amplitudeL = amplitudeL >> 2;
    amplitudeLincrement = amplitudeLincrement >> 2;

    destL = &MusicGlobals->songBufferDry[0];
    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);
    wave_adjust = 0;

    if (looping)
	{
	    end_wave = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	    wave_adjust = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;
	}
    else
	{
	    end_wave = (XFIXED) (this_voice->NotePtrEnd - this_voice->NotePtr - 1) << STEP_BIT_RANGE;
	}

#ifdef KATMAI
    if (MusicGlobals->useKatmaiCPU)
	{
	    XFIXED					cur_wave_next_frame;

	    cur_wave_next_frame = wave_increment * MusicGlobals->Four_Loop;
	    _mm_prefetch((char *)&source[cur_wave>>STEP_BIT_RANGE], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
	    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
			
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);	// remove DC bias
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 5;
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);	// remove DC bias
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 5;
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	}
    this_voice->Z1value = Z1value;
    this_voice->zIndex = zIndex2;
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL << 2;
FINISH:
    return;
}

// $$kk: 04.19.99
// void PV_ServeStereoInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping)
void PV_ServeStereoInterp2FilterPartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*destL;
    register UBYTE 			*source;
    register UBYTE			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitudeL;
    register INT32			amplitudeR;
    register INT32			inner;
    INT32					amplitudeLincrement, amplitudeRincrement;
    INT32					ampValueL, ampValueR;
    INT32					a;
    register INT16			*z;
    register INT32			Z1value, zIndex1, zIndex2, Xn, Z1, Zn, sample;

    if (this_voice->channels > 1) 
	{
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2PartialBuffer (this_voice, looping); 
	    PV_ServeStereoInterp2PartialBuffer (this_voice, looping, threadContext); 
	    return; 
	}

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel > 1 || this_voice->chorusLevel > 1)
	{ 
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2FilterPartialBufferNewReverb (this_voice, looping); 
	    PV_ServeStereoInterp2FilterPartialBufferNewReverb (this_voice, looping, threadContext); 
	    return; 
	}
#endif
    z = this_voice->z;
    Z1value = this_voice->Z1value;
    zIndex2 = this_voice->zIndex;

    GET_FILTER_PARAMS

	PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);

    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = ((ampValueL - amplitudeL) / MusicGlobals->Four_Loop) >> 2;
    amplitudeRincrement = ((ampValueR - amplitudeR) / MusicGlobals->Four_Loop) >> 2;

    amplitudeL = amplitudeL >> 2;
    amplitudeR = amplitudeR >> 2;

    destL = &MusicGlobals->songBufferDry[0];
    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);
    wave_adjust = 0;

    if (looping)
	{
	    end_wave = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	    wave_adjust = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;
	}
    else
	{
	    end_wave = (XFIXED) (this_voice->NotePtrEnd - this_voice->NotePtr - 1) << STEP_BIT_RANGE;
	}

#ifdef KATMAI
    if (MusicGlobals->useKatmaiCPU)
	{
	    XFIXED					cur_wave_next_frame;

	    cur_wave_next_frame = wave_increment * MusicGlobals->Four_Loop;
	    _mm_prefetch((char *)&source[cur_wave>>STEP_BIT_RANGE], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
	    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
		
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 3;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 3;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    this_voice->Z1value = Z1value;
    this_voice->zIndex = zIndex2;
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL << 2;
    this_voice->lastAmplitudeR = amplitudeR << 2;
FINISH:
    return;
}


// $$kk: 04.19.99
// void PV_ServeInterp2FilterFullBuffer(GM_Voice *this_voice)
void PV_ServeInterp2FilterFullBuffer(GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*destL;
    register UBYTE 			*source;
    register UBYTE			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register INT32			amplitudeL;
    register INT32			inner;

    INT32					amplitudeLincrement;
    INT32					ampValueL;
    INT32					a;

    register INT16			*z;
    register INT32			Z1value, zIndex1, zIndex2, Xn, Z1, Zn, sample;

    // We can't filter stereo samples, so bail on this.
    if (this_voice->channels > 1) 
	{
	    // $$kk: 04.19.99
	    // PV_ServeInterp2PartialBuffer (this_voice, FALSE); 
	    PV_ServeInterp2PartialBuffer (this_voice, FALSE, threadContext); 
	    return; 
	}
#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel > 1 || this_voice->chorusLevel > 1)
	{
	    // $$kk: 04.19.99
	    // PV_ServeInterp2FilterFullBufferNewReverb (this_voice); 
	    PV_ServeInterp2FilterFullBufferNewReverb (this_voice, threadContext); 
	    return;
	}
#endif
    z = this_voice->z;
    Z1value = this_voice->Z1value;
    zIndex2 = this_voice->zIndex;

    GET_FILTER_PARAMS

	amplitudeL = this_voice->lastAmplitudeL;
    ampValueL = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeLincrement = (ampValueL - amplitudeL) / MusicGlobals->Four_Loop;
	
    amplitudeL = amplitudeL >> 2;
    amplitudeLincrement = amplitudeLincrement >> 2;

    destL = &MusicGlobals->songBufferDry[0];
    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);

#ifdef KATMAI
    if (MusicGlobals->useKatmaiCPU)
	{
	    XFIXED					cur_wave_next_frame;

	    cur_wave_next_frame = wave_increment * MusicGlobals->Four_Loop;
	    _mm_prefetch((char *)&source[cur_wave>>STEP_BIT_RANGE], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
	    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
		
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);	// remove DC bias
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 5;
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);	// remove DC bias
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 5;
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);

			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	}
    this_voice->Z1value = Z1value;
    this_voice->zIndex = zIndex2;
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL << 2;
}



// $$kk: 04.19.99
// void PV_ServeStereoInterp2FilterFullBuffer (GM_Voice *this_voice)
void PV_ServeStereoInterp2FilterFullBuffer (GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*destL;
    register UBYTE 			*source;
    register UBYTE			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register INT32			amplitudeL;
    register INT32			amplitudeR;
    register INT32			inner;
    INT32					amplitudeLincrement, amplitudeRincrement;
    INT32					ampValueL, ampValueR;
    INT32					a;

    register INT16			*z;
    register INT32			Z1value, zIndex1, zIndex2, Xn, Z1, Zn, sample;

    if (this_voice->channels > 1) 
	{
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2PartialBuffer (this_voice, FALSE); 
	    PV_ServeStereoInterp2PartialBuffer (this_voice, FALSE, threadContext); 
	    return; 
	}
#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel > 1 || this_voice->chorusLevel > 1)
	{
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2FilterFullBufferNewReverb (this_voice); 
	    PV_ServeStereoInterp2FilterFullBufferNewReverb (this_voice, threadContext); 
	    return;
	}
#endif
    z = this_voice->z;
    Z1value = this_voice->Z1value;
    zIndex2 = this_voice->zIndex;

    GET_FILTER_PARAMS

	PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);

    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = ((ampValueL - amplitudeL) / MusicGlobals->Four_Loop) >> 2;
    amplitudeRincrement = ((ampValueR - amplitudeR) / MusicGlobals->Four_Loop) >> 2;

    amplitudeL = amplitudeL >> 2;
    amplitudeR = amplitudeR >> 2;

    destL = &MusicGlobals->songBufferDry[0];
    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);

#ifdef KATMAI
    if (MusicGlobals->useKatmaiCPU)
	{
	    XFIXED					cur_wave_next_frame;

	    cur_wave_next_frame = wave_increment * MusicGlobals->Four_Loop;
	    _mm_prefetch((char *)&source[cur_wave>>STEP_BIT_RANGE], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
	    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
		
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 3;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 3;

			    for (inner = 0; inner < 4; inner++)
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) << 2;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    this_voice->Z1value = Z1value;
    this_voice->zIndex = zIndex2;
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL << 2;
    this_voice->lastAmplitudeR = amplitudeR << 2;
}

// еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее ееееееееее
// еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее ееееееееее
// еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее ееееееееее
// еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее ееееееееее
// еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее еееееееееееее ееееееееее
// 16 bit cases

// $$kk: 04.19.99
// void PV_ServeInterp2FilterFullBuffer16 (GM_Voice *this_voice)
void PV_ServeInterp2FilterFullBuffer16 (GM_Voice *this_voice, void *threadContext)
{
    // $$kk: 04.19.99
    // PV_ServeInterp2FilterPartialBuffer16 (this_voice, FALSE);
    PV_ServeInterp2FilterPartialBuffer16 (this_voice, FALSE, threadContext);
}

// $$kk: 04.19.99
// void PV_ServeInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping)
void PV_ServeInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{																			    
    register INT32 			*destL;
    register INT16 			*source;
    register INT16			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitudeL;
    register INT32			inner;

    INT32					amplitudeLincrement;
    INT32					ampValueL;
    INT32					a;

    register INT16			*z;
    register INT32			Z1value, zIndex1, zIndex2, Xn, Z1, Zn, sample;

    if (this_voice->channels > 1) 
	{ 
	    // $$kk: 04.19.99
	    // PV_ServeInterp2PartialBuffer16 (this_voice, looping); 
	    PV_ServeInterp2PartialBuffer16 (this_voice, looping, threadContext); 
	    return; 
	}
#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel > 1 || this_voice->chorusLevel > 1)
	{
	    // $$kk: 04.19.99
	    // PV_ServeInterp2FilterPartialBufferNewReverb16 (this_voice, looping); 
	    PV_ServeInterp2FilterPartialBufferNewReverb16 (this_voice, looping, threadContext); 
	    return;															  
	}
#endif
    z = this_voice->z;
    Z1value = this_voice->Z1value;
    zIndex2 = this_voice->zIndex;

    GET_FILTER_PARAMS

	amplitudeL = this_voice->lastAmplitudeL;
    ampValueL = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeLincrement = (ampValueL - amplitudeL) / MusicGlobals->Four_Loop;

    destL = &MusicGlobals->songBufferDry[0];
    source = (short *) this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);
    wave_adjust = 0;

    if (looping)
	{
	    end_wave = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	    wave_adjust = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;
	}
    else
	{
	    end_wave = (XFIXED) (this_voice->NotePtrEnd - this_voice->NotePtr - 1) << STEP_BIT_RANGE;
	}

#ifdef KATMAI
    if (MusicGlobals->useKatmaiCPU)
	{
	    XFIXED					cur_wave_next_frame;

	    cur_wave_next_frame = wave_increment * MusicGlobals->Four_Loop;
	    _mm_prefetch((char *)&source[cur_wave>>STEP_BIT_RANGE], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
	    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
		
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);		// is in the mail
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);	// remove DC bias
				    *destL += (sample * amplitudeL) >> 2;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 5;
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif
				
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);		// is in the mail
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += (sample * amplitudeL) >> 2;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);		// is in the mail
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);	// remove DC bias
				    *destL += (sample * amplitudeL) >> 2;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 5;
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);		// is in the mail
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += (sample * amplitudeL) >> 2;
				    destL++;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			}
		}
	}
    this_voice->Z1value = Z1value;
    this_voice->zIndex = zIndex2;
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL;
FINISH:
    return;
}


// $$kk: 04.19.99
// void PV_ServeStereoInterp2FilterFullBuffer16 (GM_Voice *this_voice)
void PV_ServeStereoInterp2FilterFullBuffer16 (GM_Voice *this_voice, void *threadContext)
{
    // $$kk: 04.19.99
    // PV_ServeStereoInterp2FilterPartialBuffer16 (this_voice, FALSE);
    PV_ServeStereoInterp2FilterPartialBuffer16 (this_voice, FALSE, threadContext);
}

// $$kk: 04.19.99
// void PV_ServeStereoInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping)
void PV_ServeStereoInterp2FilterPartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*destL;
    register INT16 			*source;
    register INT16			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitudeL;
    register INT32			amplitudeR;
    register INT32			inner;

    INT32					amplitudeLincrement, amplitudeRincrement;
    INT32					ampValueL, ampValueR;
    INT32					a;

    register INT16			*z;
    register INT32			Z1value, zIndex1, zIndex2, Xn, Z1, Zn, sample;

    if (this_voice->channels > 1) 
	{ 
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2PartialBuffer16 (this_voice, looping); 
	    PV_ServeStereoInterp2PartialBuffer16 (this_voice, looping, threadContext); 
	    return; 
	}
#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel > 1 || this_voice->chorusLevel > 1)
	{
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2FilterPartialBufferNewReverb16 (this_voice, looping); 
	    PV_ServeStereoInterp2FilterPartialBufferNewReverb16 (this_voice, looping, threadContext); 
	    return;
	}
#endif
    z = this_voice->z;
    Z1value = this_voice->Z1value;
    zIndex2 = this_voice->zIndex;

    GET_FILTER_PARAMS

	PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);

    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / MusicGlobals->Four_Loop;
    amplitudeRincrement = (ampValueR - amplitudeR) / MusicGlobals->Four_Loop;

    destL = &MusicGlobals->songBufferDry[0];
    source = (short *) this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);
    wave_adjust = 0;

    if (looping)
	{
	    end_wave = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	    wave_adjust = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;
	}
    else
	{
	    end_wave = (XFIXED) (this_voice->NotePtrEnd - this_voice->NotePtr - 1) << STEP_BIT_RANGE;
	}

#ifdef KATMAI
    if (MusicGlobals->useKatmaiCPU)
	{
	    XFIXED					cur_wave_next_frame;

	    cur_wave_next_frame = wave_increment * MusicGlobals->Four_Loop;
	    _mm_prefetch((char *)&source[cur_wave>>STEP_BIT_RANGE], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
	    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);		// is in the mail
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);
				    *destL += (sample * amplitudeL) >> 2;
				    destL[1] += (sample * amplitudeR) >> 2;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 3;
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += (sample * amplitudeL) >> 2;
				    destL[1] += (sample * amplitudeR) >> 2;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->LPF_resonance == 0)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);		// is in the mail
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1) >> 16;
				    Z1value = sample - (sample >> 9);
				    *destL += (sample * amplitudeL) >> 2;
				    destL[1] += (sample * amplitudeR) >> 2;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    zIndex1 = zIndex2 - (this_voice->previous_zFrequency >> 8);
			    this_voice->previous_zFrequency += (this_voice->LPF_frequency - this_voice->previous_zFrequency) >> 3;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) >> 6;
				    sample = (sample * Xn + Z1value * Z1 + z[zIndex1 & MAXRESONANCE] * Zn) >> 16;
				    zIndex1++;
				    z[zIndex2 & MAXRESONANCE] = (INT16)sample;
				    zIndex2++;
				    Z1value = sample - (sample >> 9);
				    *destL += (sample * amplitudeL) >> 2;
				    destL[1] += (sample * amplitudeR) >> 2;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    this_voice->Z1value = Z1value;
    this_voice->zIndex = zIndex2;
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL;
    this_voice->lastAmplitudeR = amplitudeR;
FINISH:
    return;
}

// EOF GenSynthFilters.c
