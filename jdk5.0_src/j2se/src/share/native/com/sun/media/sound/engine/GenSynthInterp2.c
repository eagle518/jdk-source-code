/*
 * @(#)GenSynthInterp2.c	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenSynthInterp2.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**	Confidential-- Internal use only
**
** Modification History:
**
**	1/18/96		Spruced up for C++ extra error checking
**				Changed the macro 'THE_CHECK' to accept a type for typecasting the source pointer
**	3/1/96		Removed extra PV_DoCallBack, and PV_GetWavePitch
**	4/25/96		Fixed bug with PV_ServeInterp2PartialBuffer16 that preserved the amp differently 
**				than PV_ServeInterpFulllBuffer16. Caused a click when playing mono with mono output
**	5/2/96		Changed 'int's to INT32 and to BOOL_FLAG
**	7/8/96		Improved enveloping and wave shaping code
**	7/10/96		Fixed stereo filter bug
**	12/30/96	Changed copyright
**	6/4/97		Added USE_SMALL_MEMORY_REVERB tests around code to disable when this
**				flag is used
**	2/3/98		Renamed songBufferLeftMono to songBufferDry
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/20/98		now support variable send chorus as well as reverb
**	11/10/98	Removed CLIP macro
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

#if USE_TERP2 == TRUE
// $$kk: 04.19.99
// void PV_ServeInterp2FullBuffer (GM_Voice *this_voice)
void PV_ServeInterp2FullBuffer (GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*dest;
    register LOOPCOUNT		a, inner;
    register UBYTE 			*source, *calculated_source;
    register INT32			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register INT32			amplitude, amplitudeAdjust;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{
	    // $$kk: 04.19.99
	    // PV_ServeInterp2FullBufferNewReverb (this_voice); 
	    PV_ServeInterp2FullBufferNewReverb (this_voice, threadContext); 
	    return;
	}
#endif
    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop;
    dest = &MusicGlobals->songBufferDry[0];
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
	    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    *dest += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    dest[1] += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    dest[2] += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    dest[3] += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;
			    dest += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 16; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];	// average left & right channels
				    c = calculated_source[2] + calculated_source[3];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) * amplitude) >> 1;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    *dest += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    dest[1] += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    dest[2] += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    dest[3] += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
			    cur_wave += wave_increment;
			    dest += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 16; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];	// average left & right channels
				    c = calculated_source[2] + calculated_source[3];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) * amplitude) >> 1;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude;
}
#endif	// USE_TERP2

#if (USE_TERP2 == TRUE) || (USE_DROP_SAMPLE == TRUE)
// $$kk: 04.19.99
// void PV_ServeInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping)
void PV_ServeInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*dest;
    register LOOPCOUNT		a, inner;
    register UBYTE 			*source, *calculated_source;
    register INT32			b, c;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitude, amplitudeAdjust;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{
	    // $$kk: 04.19.99
	    // PV_ServeInterp2PartialBufferNewReverb (this_voice, looping);
	    PV_ServeInterp2PartialBufferNewReverb (this_voice, looping, threadContext);
	    return;
	}
#endif
    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop;
    dest = &MusicGlobals->songBufferDry[0];
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
	    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    *dest += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    calculated_source = source + ( (cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) * amplitude) >> 1;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    *dest += ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitude;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    calculated_source = source + ( (cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) * amplitude) >> 1;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}

    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude;
FINISH:
    return;
}
#endif	// USE_TERP2

#if USE_TERP2 == TRUE
// $$kk: 04.19.99
// void PV_ServeStereoInterp2FullBuffer(GM_Voice *this_voice)
void PV_ServeStereoInterp2FullBuffer(GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*destL;
    register LOOPCOUNT		a, inner;
    register UBYTE 			*source, *calculated_source;
    register INT32			b, c;
    register XFIXED 		cur_wave;
    register INT32			sample;
    register XFIXED 		wave_increment;
    INT32					ampValueL, ampValueR;
    register INT32			amplitudeL;
    register INT32			amplitudeR;
    register INT32			amplitudeLincrement;
    register INT32			amplitudeRincrement;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2FullBufferNewReverb (this_voice);
	    PV_ServeStereoInterp2FullBufferNewReverb (this_voice, threadContext);
	    return;
	}
#endif
    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Four_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Four_Loop);

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

	    if (this_voice->channels == 1)
		{	// mono instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[0] += sample * amplitudeL;
			    destL[1] += sample * amplitudeR;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[2] += sample * amplitudeL;
			    destL[3] += sample * amplitudeR;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[4] += sample * amplitudeL;
			    destL[5] += sample * amplitudeR;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[6] += sample * amplitudeL;
			    destL[7] += sample * amplitudeR;
			    destL += 8;
			    cur_wave += wave_increment;

			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    *destL += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeL);
				    b = calculated_source[1];
				    c = calculated_source[3];
				    destL[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeR);
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
	    if (this_voice->channels == 1)
		{	// mono instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[0] += sample * amplitudeL;
			    destL[1] += sample * amplitudeR;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[2] += sample * amplitudeL;
			    destL[3] += sample * amplitudeR;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[4] += sample * amplitudeL;
			    destL[5] += sample * amplitudeR;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[6] += sample * amplitudeL;
			    destL[7] += sample * amplitudeR;
			    destL += 8;
			    cur_wave += wave_increment;

			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    *destL += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeL);
				    b = calculated_source[1];
				    c = calculated_source[3];
				    destL[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeR);
				    destL += 2;
			
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    this_voice->lastAmplitudeL = amplitudeL;
    this_voice->lastAmplitudeR = amplitudeR;
    this_voice->NoteWave = cur_wave;
}
#endif	// USE_TERP2

#if (USE_TERP2 == TRUE) || (USE_DROP_SAMPLE == TRUE)
// $$kk: 04.19.99
// void PV_ServeStereoInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping)
void PV_ServeStereoInterp2PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*destL;
    register LOOPCOUNT		a, inner;
    register UBYTE 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    INT32					ampValueL, ampValueR;
    register INT32			amplitudeL;
    register INT32			amplitudeR;
    register INT32			amplitudeLincrement, amplitudeRincrement;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2PartialBufferNewReverb (this_voice, looping); 
	    PV_ServeStereoInterp2PartialBufferNewReverb (this_voice, looping, threadContext); 
	    return;
	}
#endif
    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Four_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Four_Loop);

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

	    if (this_voice->channels == 1)
		{	// mono instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    if (cur_wave + (wave_increment << 2) >= end_wave)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[0] += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    cur_wave += wave_increment;
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[2] += sample * amplitudeL;
				    destL[3] += sample * amplitudeR;
				    cur_wave += wave_increment;
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[4] += sample * amplitudeL;
				    destL[5] += sample * amplitudeR;
				    cur_wave += wave_increment;
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[6] += sample * amplitudeL;
				    destL[7] += sample * amplitudeR;
				    cur_wave += wave_increment;
				}
			    else
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[0] += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    cur_wave += wave_increment;

				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[2] += sample * amplitudeL;
				    destL[3] += sample * amplitudeR;
				    cur_wave += wave_increment;

				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[4] += sample * amplitudeL;
				    destL[5] += sample * amplitudeR;
				    cur_wave += wave_increment;

				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[6] += sample * amplitudeL;
				    destL[7] += sample * amplitudeR;
				    cur_wave += wave_increment;
				}
			    destL += 8;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// Stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    *destL += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeL);
				    b = calculated_source[1];
				    c = calculated_source[3];
				    destL[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeR);
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
	    if (this_voice->channels == 1)
		{	// mono instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    if (cur_wave + (wave_increment << 2) >= end_wave)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[0] += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    cur_wave += wave_increment;
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[2] += sample * amplitudeL;
				    destL[3] += sample * amplitudeR;
				    cur_wave += wave_increment;
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[4] += sample * amplitudeL;
				    destL[5] += sample * amplitudeR;
				    cur_wave += wave_increment;
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[6] += sample * amplitudeL;
				    destL[7] += sample * amplitudeR;
				    cur_wave += wave_increment;
				}
			    else
				{
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[0] += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				    cur_wave += wave_increment;

				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[2] += sample * amplitudeL;
				    destL[3] += sample * amplitudeR;
				    cur_wave += wave_increment;

				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[4] += sample * amplitudeL;
				    destL[5] += sample * amplitudeR;
				    cur_wave += wave_increment;

				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
				    destL[6] += sample * amplitudeL;
				    destL[7] += sample * amplitudeR;
				    cur_wave += wave_increment;
				}
			    destL += 8;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// Stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    *destL += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeL);
				    b = calculated_source[1];
				    c = calculated_source[3];
				    destL[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80) * amplitudeR);
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL;
    this_voice->lastAmplitudeR = amplitudeR;
FINISH:
    return;
}
#endif	// USE_TERP2

// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥

// 16 bit cases
#if USE_TERP2 == TRUE
// $$kk: 04.19.99
// void PV_ServeInterp2FullBuffer16 (GM_Voice *this_voice)
void PV_ServeInterp2FullBuffer16 (GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*dest;
    register LOOPCOUNT		a, inner;
    register INT16 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register INT32			amplitude, amplitudeAdjust;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{
	    // $$kk: 04.19.99
	    // PV_ServeInterp2FullBuffer16NewReverb (this_voice); 
	    PV_ServeInterp2FullBuffer16NewReverb (this_voice, threadContext); 
	    return;
	}
#endif
    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop >> 4;
    amplitude = amplitude >> 4;

    dest = &MusicGlobals->songBufferDry[0];
    source = (short *) this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);

#ifdef KATMAI
    if (MusicGlobals->useKatmaiCPU)
	{
	    XFIXED					cur_wave_next_frame;

	    cur_wave_next_frame = wave_increment * MusicGlobals->Four_Loop;
	    _mm_prefetch((char *)&source[cur_wave>>STEP_BIT_RANGE], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
	    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;

			    cur_wave += wave_increment;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    dest[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;

			    cur_wave += wave_increment;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    dest[2] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;

			    cur_wave += wave_increment;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    dest[3] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
			    dest += 4;
			    cur_wave += wave_increment;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *dest += (sample  * amplitude) >> 5;	// divide extra for summed stereo channels
				    dest++;

				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;

			    cur_wave += wave_increment;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    dest[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;

			    cur_wave += wave_increment;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    dest[2] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;

			    cur_wave += wave_increment;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    dest[3] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
			    dest += 4;
			    cur_wave += wave_increment;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *dest += (sample  * amplitude) >> 5;	// divide extra for summed stereo channels
				    dest++;

				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude << 4;
}
#endif	// USE_TERP2

#if (USE_TERP2 == TRUE) || (USE_DROP_SAMPLE == TRUE)
// $$kk: 04.19.99
// void PV_ServeInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping)
void PV_ServeInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*dest;
    register LOOPCOUNT		a, inner;
    register INT16 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitude, amplitudeAdjust;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{
	    // $$kk: 04.19.99
	    // PV_ServeInterp2PartialBuffer16NewReverb (this_voice, looping); 
	    PV_ServeInterp2PartialBuffer16NewReverb (this_voice, looping, threadContext); 
	    return;
	}
#endif
    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop >> 4;
    amplitude = amplitude >> 4;

    dest = &MusicGlobals->songBufferDry[0];
    cur_wave = this_voice->NoteWave;
    source = (short *) this_voice->NotePtr;

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
	    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    if (cur_wave + (wave_increment << 2) >= end_wave)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[2] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[3] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				}
			    else
				{
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[2] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[3] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				}
			    dest += 4;
			    cur_wave += wave_increment;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *dest += ((sample >> 1) * amplitude) >> 5;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    else
#endif
	{
	    if (this_voice->channels == 1)
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    if (cur_wave + (wave_increment << 2) >= end_wave)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[2] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[3] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				}
			    else
				{
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    *dest += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[1] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[2] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    dest[3] += (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b) * amplitude) >> 4;
				}
			    dest += 4;
			    cur_wave += wave_increment;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *dest += ((sample >> 1) * amplitude) >> 5;
				    dest++;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude << 4;
FINISH:
    return;
}
#endif	// USE_TERP2

#if (USE_TERP2 == TRUE) || (USE_DROP_SAMPLE == TRUE)
// $$kk: 04.19.99
// void PV_ServeStereoInterp2FullBuffer16 (GM_Voice *this_voice)
void PV_ServeStereoInterp2FullBuffer16 (GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*destL;
    register LOOPCOUNT		a, inner;
    register INT16 			*source, *calculated_source;
    register INT32			b, c;
    register XFIXED 		cur_wave;
    register INT32			sample;
    register XFIXED 		wave_increment;
    INT32					ampValueL, ampValueR;
    register INT32			amplitudeL;
    register INT32			amplitudeR;
    register INT32			amplitudeLincrement;
    register INT32			amplitudeRincrement;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{ 
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2FullBuffer16NewReverb (this_voice);
	    PV_ServeStereoInterp2FullBuffer16NewReverb (this_voice, threadContext);
	    return;
	}
#endif
    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Four_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Four_Loop);

    amplitudeL = amplitudeL >> 4;
    amplitudeR = amplitudeR >> 4;
    amplitudeLincrement = amplitudeLincrement >> 4;
    amplitudeRincrement = amplitudeRincrement >> 4;

    destL = &MusicGlobals->songBufferDry[0];
    cur_wave = this_voice->NoteWave;

    source = (short *) this_voice->NotePtr;

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

	    if (this_voice->channels == 1)
		{	// mono instrument

		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[0] += (sample * amplitudeL) >> 4;
			    destL[1] += (sample * amplitudeR) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[2] += (sample * amplitudeL) >> 4;
			    destL[3] += (sample * amplitudeR) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[4] += (sample * amplitudeL) >> 4;
			    destL[5] += (sample * amplitudeR) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[6] += (sample * amplitudeL) >> 4;
			    destL[7] += (sample * amplitudeR) >> 4;
			    destL += 8;
			    cur_wave += wave_increment;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
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
	    if (this_voice->channels == 1)
		{	// mono instrument

		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[0] += (sample * amplitudeL) >> 4;
			    destL[1] += (sample * amplitudeR) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[2] += (sample * amplitudeL) >> 4;
			    destL[3] += (sample * amplitudeR) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[4] += (sample * amplitudeL) >> 4;
			    destL[5] += (sample * amplitudeR) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[6] += (sample * amplitudeL) >> 4;
			    destL[7] += (sample * amplitudeR) >> 4;
			    destL += 8;
			    cur_wave += wave_increment;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
				    destL += 2;
			
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    this_voice->lastAmplitudeL = amplitudeL << 4;
    this_voice->lastAmplitudeR = amplitudeR << 4;
    this_voice->NoteWave = cur_wave;
}
#endif	// USE_TERP2

INLINE static XFIXED PV_GetWhole(XFIXED value)
{
    return value>>STEP_BIT_RANGE;
}

INLINE static XFIXED PV_GetFrac(XFIXED value)
{
    return value & STEP_FULL_RANGE;
}


INLINE static INT32 PV_GetFractionalStep(XFIXED value, INT32 step1, INT32 step2)
{
    return (((INT32) (value & STEP_FULL_RANGE) * (step2-step1))>>STEP_BIT_RANGE) + step1;
}

#if (USE_TERP2 == TRUE) || (USE_DROP_SAMPLE == TRUE)
// $$kk: 04.19.99
// void PV_ServeStereoInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping)
void PV_ServeStereoInterp2PartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*destL;
    register LOOPCOUNT		a, inner;
    register INT16 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    INT32					ampValueL, ampValueR;
    register INT32			amplitudeL;
    register INT32			amplitudeR;
    register INT32			amplitudeLincrement, amplitudeRincrement;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel)
	{
	    // $$kk: 04.19.99
	    // PV_ServeStereoInterp2PartialBuffer16NewReverb (this_voice, looping);
	    PV_ServeStereoInterp2PartialBuffer16NewReverb (this_voice, looping, threadContext);
	    return;
	}
#endif
    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Four_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Four_Loop);

    amplitudeL = amplitudeL >> 4;
    amplitudeR = amplitudeR >> 4;
    amplitudeLincrement = amplitudeLincrement >> 4;
    amplitudeRincrement = amplitudeRincrement >> 4;

    destL = &MusicGlobals->songBufferDry[0];
    cur_wave = this_voice->NoteWave;
    source = (short *) this_voice->NotePtr;

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

	    if (this_voice->channels == 1)
		{	// mono instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    if (cur_wave + (wave_increment << 2) >= end_wave)
				{
				    for (inner = 0; inner < 4; inner++)
					{
					    THE_CHECK(INT16 *);
					    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
					    // calculated_source = source + (UINT32)cur_wave;
					    b = *calculated_source;
					    c = calculated_source[1];
					    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
					    //sample = trunc(cur_wave) * (c-b) + b;
					    *destL += (sample * amplitudeL) >> 4;
					    destL[1] += (sample * amplitudeR) >> 4;
					    destL += 2;
					    cur_wave += wave_increment;
					}
				}
			    else
				{
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    destL[1] += (sample * amplitudeR) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[2] += (sample * amplitudeL) >> 4;
				    destL[3] += (sample * amplitudeR) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[4] += (sample * amplitudeL) >> 4;
				    destL[5] += (sample * amplitudeR) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[6] += (sample * amplitudeL) >> 4;
				    destL[7] += (sample * amplitudeR) >> 4;
				    destL += 8;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// Stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
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
	    if (this_voice->channels == 1)
		{	// mono instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    if (cur_wave + (wave_increment << 2) >= end_wave)
				{
				    for (inner = 0; inner < 4; inner++)
					{
					    THE_CHECK(INT16 *);
					    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
					    b = *calculated_source;
					    c = calculated_source[1];
					    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
					    *destL += (sample * amplitudeL) >> 4;
					    destL[1] += (sample * amplitudeR) >> 4;
					    destL += 2;
					    cur_wave += wave_increment;
					}
				}
			    else
				{
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    //calculated_source = source + PV_GetWhole(cur_wave);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    //sample = PV_GetFractionalStep(cur_wave, b, c);
				    *destL += (sample * amplitudeL) >> 4;
				    destL[1] += (sample * amplitudeR) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[2] += (sample * amplitudeL) >> 4;
				    destL[3] += (sample * amplitudeR) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[4] += (sample * amplitudeL) >> 4;
				    destL[5] += (sample * amplitudeR) >> 4;
				    cur_wave += wave_increment;

				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[6] += (sample * amplitudeL) >> 4;
				    destL[7] += (sample * amplitudeR) >> 4;
				    destL += 8;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// Stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
				    destL += 2;
				    cur_wave += wave_increment;
				}
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitudeL << 4;
    this_voice->lastAmplitudeR = amplitudeR << 4;
FINISH:
    return;
}
#endif	// USE_TERP2

// EOF of GenSynthInterp2.c

