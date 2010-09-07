/*
 * @(#)GenInterp2Reverb.c	1.16 03/12/19
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
**	2/20/98		kcr		now sum into chorus buffer as well as reverb buffer
**	11/10/98	Removed CLIP macro
**	11/23/98	Added support for Intel Katmai CPU
**	1/4/99		Removed FAR macro. Re ordered Katmai code and duplicated inner loops
**	2/21/99		Fixed a bug with stereo 8 bit samples and verb. There was an extra
**				amplitude calculation. Thanks to <w.thomas@telead.nl> for finding it.
**	2002-03-14	$$fb removed compiler warnings
*/
/*****************************************************************************/

#include "GenSnd.h"
#include "GenPriv.h"

#if ((USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB != FALSE) && (USE_TERP2 == TRUE))

#undef KATMAI
#if (X_PLATFORM == X_WINDOWS) && USE_KAT == 1

// KATMAI support
#include "xmmintrin.h"
#define KATMAI			1
//#define USE_KATMAI_WRITE_CACHE
#endif


// $$kk: 04.19.99
// void PV_ServeInterp2FullBufferNewReverb(GM_Voice *this_voice)
void PV_ServeInterp2FullBufferNewReverb(GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*dest;
    register INT32 			*destReverb, *destChorus;
    register LOOPCOUNT		a, inner;
    register UBYTE 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED			cur_wave;
    register XFIXED			wave_increment;
    register INT32			amplitude, amplitudeAdjust;
    register INT32			amplitudeReverb, amplitudeChorus;

    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop;
    dest = &MusicGlobals->songBufferDry[0];
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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
		    amplitudeReverb = (amplitude * this_voice->reverbLevel) >> 7;
		    amplitudeChorus = (amplitude * this_voice->chorusLevel) >> 7;
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    *dest += sample * amplitude;
			    destReverb[0] += sample * amplitudeReverb;
			    destChorus[0] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    dest[1] += sample * amplitude;
			    destReverb[1] += sample * amplitudeReverb;
			    destChorus[1] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    dest[2] += sample * amplitude;
			    destReverb[2] += sample * amplitudeReverb;
			    destChorus[2] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    dest[3] += sample * amplitude;
			    destReverb[3] += sample * amplitudeReverb;
			    destChorus[3] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    dest += 4;
			    destReverb += 4;
			    destChorus += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 16; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];	// average left & right channels
				    c = calculated_source[2] + calculated_source[3];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) >> 1;
				    *dest += sample * amplitude;
				    *destReverb += sample * amplitudeReverb;
				    dest++;
				    destReverb++;
				    *destChorus++ += sample * amplitudeChorus;
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
		    amplitudeReverb = (amplitude * this_voice->reverbLevel) >> 7;
		    amplitudeChorus = (amplitude * this_voice->chorusLevel) >> 7;
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    *dest += sample * amplitude;
			    destReverb[0] += sample * amplitudeReverb;
			    destChorus[0] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    dest[1] += sample * amplitude;
			    destReverb[1] += sample * amplitudeReverb;
			    destChorus[1] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    dest[2] += sample * amplitude;
			    destReverb[2] += sample * amplitudeReverb;
			    destChorus[2] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
			    b = calculated_source[0];
			    c = calculated_source[1];
			    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
			    dest[3] += sample * amplitude;
			    destReverb[3] += sample * amplitudeReverb;
			    destChorus[3] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    dest += 4;
			    destReverb += 4;
			    destChorus += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    for (inner = 0; inner < 16; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];	// average left & right channels
				    c = calculated_source[2] + calculated_source[3];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) >> 1;
				    *dest += sample * amplitude;
				    *destReverb += sample * amplitudeReverb;
				    dest++;
				    destReverb++;
				    *destChorus++ += sample * amplitudeChorus;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude;
}


// $$kk: 04.19.99
// void PV_ServeInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping)
void PV_ServeInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*dest;
    register INT32 			*destReverb, *destChorus;
    register LOOPCOUNT		a, inner;
    register UBYTE 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitude, amplitudeAdjust;
    register INT32			amplitudeReverb, amplitudeChorus;

    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop;
    dest = &MusicGlobals->songBufferDry[0];
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
				    *dest += sample * amplitude;
				    *destReverb += sample * amplitudeReverb;
				    dest++;
				    destReverb++;
				    *destChorus++ += sample * amplitudeChorus;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

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
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) >> 1;
				    *dest += sample * amplitude;
				    *destReverb += sample * amplitudeReverb;
				    dest++;
				    destReverb++;
				    *destChorus++ += sample * amplitudeChorus;
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
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    b = source[cur_wave>>STEP_BIT_RANGE];
				    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80);
				    *dest += sample * amplitude;
				    *destReverb += sample * amplitudeReverb;
				    dest++;
				    destReverb++;
				    *destChorus++ += sample * amplitudeChorus;
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    calculated_source = source + ( (cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    sample = ((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x100) >> 1;
				    *dest += sample * amplitude;
				    *destReverb += sample * amplitudeReverb;
				    dest++;
				    destReverb++;
				    *destChorus++ += sample * amplitudeChorus;
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

// $$kk: 04.19.99
// void PV_ServeStereoInterp2FullBufferNewReverb(GM_Voice *this_voice)
void PV_ServeStereoInterp2FullBufferNewReverb(GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*destL;
    register INT32 			*destReverb, *destChorus;
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
    register INT32			amplitudeReverb, amplitudeChorus;

    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Four_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Four_Loop);

    destL = &MusicGlobals->songBufferDry[0];
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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

			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[0] += sample * amplitudeL;
			    destL[1] += sample * amplitudeR;
			    destReverb[0] += sample * amplitudeReverb;
			    destChorus[0] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[2] += sample * amplitudeL;
			    destL[3] += sample * amplitudeR;
			    destReverb[1] += sample * amplitudeReverb;
			    destChorus[1] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[4] += sample * amplitudeL;
			    destL[5] += sample * amplitudeR;
			    destReverb[2] += sample * amplitudeReverb;
			    destChorus[2] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[6] += sample * amplitudeL;
			    destL[7] += sample * amplitudeR;
			    destReverb[3] += sample * amplitudeReverb;
			    destChorus[3] += sample * amplitudeChorus;
			    destL += 8;
			    destReverb += 4;
			    destChorus += 4;
			    cur_wave += wave_increment;

			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 9) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 9) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    *destL += sample * amplitudeL;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    destL[1] += sample * amplitudeR;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99
				    destL += 2;
				    destReverb++;
				    destChorus++;
			
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[0] += sample * amplitudeL;
			    destL[1] += sample * amplitudeR;
			    destReverb[0] += sample * amplitudeReverb;
			    destChorus[0] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[2] += sample * amplitudeL;
			    destL[3] += sample * amplitudeR;
			    destReverb[1] += sample * amplitudeReverb;
			    destChorus[1] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[4] += sample * amplitudeL;
			    destL[5] += sample * amplitudeR;
			    destReverb[2] += sample * amplitudeReverb;
			    destChorus[2] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[6] += sample * amplitudeL;
			    destL[7] += sample * amplitudeR;
			    destReverb[3] += sample * amplitudeReverb;
			    destChorus[3] += sample * amplitudeChorus;
			    destL += 8;
			    destReverb += 4;
			    destChorus += 4;
			    cur_wave += wave_increment;

			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 8 bit instrument
		    // HERE
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 9) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 9) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    *destL += sample * amplitudeL;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    destL[1] += sample * amplitudeR;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99

				    destL += 2;
				    destReverb++;
				    destChorus++;
			
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

// $$kk: 04.19.99
// void PV_ServeStereoInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping)
void PV_ServeStereoInterp2PartialBufferNewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*destL;
    register INT32 			*destReverb, *destChorus;
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
    register INT32			amplitudeReverb, amplitudeChorus;

    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Four_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Four_Loop);

    destL = &MusicGlobals->songBufferDry[0];
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[0] += sample * amplitudeL;
			    destL[1] += sample * amplitudeR;
			    destReverb[0] += sample * amplitudeReverb;
			    destChorus[0] += sample * amplitudeChorus;
			    cur_wave += wave_increment;
			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[2] += sample * amplitudeL;
			    destL[3] += sample * amplitudeR;
			    destReverb[1] += sample * amplitudeReverb;
			    destChorus[1] += sample * amplitudeChorus;
			    cur_wave += wave_increment;
			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[4] += sample * amplitudeL;
			    destL[5] += sample * amplitudeR;
			    destReverb[2] += sample * amplitudeReverb;
			    destChorus[2] += sample * amplitudeChorus;
			    cur_wave += wave_increment;
			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[6] += sample * amplitudeL;
			    destL[7] += sample * amplitudeR;
			    destReverb[3] += sample * amplitudeReverb;
			    destChorus[3] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    destL += 8;
			    destReverb += 4;
			    destChorus += 4;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// Stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 9) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 9) * this_voice->chorusLevel;
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
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    *destL += sample * amplitudeL;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    destL[1] += sample * amplitudeR;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99
				    destL += 2;
				    destReverb++;
				    destChorus++;
					
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[0] += sample * amplitudeL;
			    destL[1] += sample * amplitudeR;
			    destReverb[0] += sample * amplitudeReverb;
			    destChorus[0] += sample * amplitudeChorus;
			    cur_wave += wave_increment;
			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[2] += sample * amplitudeL;
			    destL[3] += sample * amplitudeR;
			    destReverb[1] += sample * amplitudeReverb;
			    destChorus[1] += sample * amplitudeChorus;
			    cur_wave += wave_increment;
			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[4] += sample * amplitudeL;
			    destL[5] += sample * amplitudeR;
			    destReverb[2] += sample * amplitudeReverb;
			    destChorus[2] += sample * amplitudeChorus;
			    cur_wave += wave_increment;
			    THE_CHECK(UBYTE *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80;
			    destL[6] += sample * amplitudeL;
			    destL[7] += sample * amplitudeR;
			    destReverb[3] += sample * amplitudeReverb;
			    destChorus[3] += sample * amplitudeChorus;
			    cur_wave += wave_increment;

			    destL += 8;
			    destReverb += 4;
			    destChorus += 4;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// Stereo 8 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 9) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 9) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(UBYTE *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    *destL += sample * amplitudeL;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b - 0x80));
				    destL[1] += sample * amplitudeR;
				    *destReverb += sample * amplitudeReverb;	// fix 2/21/99
				    *destChorus += sample * amplitudeChorus;	// fix 2/21/99
				    destL += 2;
				    destReverb++;
				    destChorus++;
					
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

// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥
// ¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥¥

// 16 bit cases

// $$kk: 04.19.99
// void PV_ServeInterp2FullBuffer16NewReverb (GM_Voice *this_voice)
void PV_ServeInterp2FullBuffer16NewReverb (GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*dest;
    register INT32 			*destReverb, *destChorus;
    register LOOPCOUNT		a, inner;
    register INT16 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register INT32			amplitude, amplitudeAdjust;
    register INT32			amplitudeReverb, amplitudeChorus;

    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop >> 4;
    amplitude = amplitude >> 4;

    dest = &MusicGlobals->songBufferDry[0];
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    *dest += (sample * amplitude) >> 4;
			    *destReverb += (sample * amplitudeReverb) >> 4;
			    *destChorus += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
			
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[1] += (sample * amplitude) >> 4;
			    destReverb[1] += (sample * amplitudeReverb) >> 4;
			    destChorus[1] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
				
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[2] += (sample * amplitude) >> 4;
			    destReverb[2] += (sample * amplitudeReverb) >> 4;
			    destChorus[2] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
							
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[3] += (sample * amplitude) >> 4;
			    destReverb[3] += (sample * amplitudeReverb) >> 4;
			    destChorus[3] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
				
			    dest += 4;
			    destReverb += 4;
			    destChorus += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;
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
				    *destReverb += (sample  * amplitudeReverb) >> 5;	// divide extra for summed stereo channels
				    *destChorus += (sample  * amplitudeChorus) >> 5;	// divide extra for summed stereo channels
				    dest++;
				    destReverb++;
				    destChorus++;
					
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
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    *dest += (sample * amplitude) >> 4;
			    *destReverb += (sample * amplitudeReverb) >> 4;
			    *destChorus += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
			
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[1] += (sample * amplitude) >> 4;
			    destReverb[1] += (sample * amplitudeReverb) >> 4;
			    destChorus[1] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
				
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[2] += (sample * amplitude) >> 4;
			    destReverb[2] += (sample * amplitudeReverb) >> 4;
			    destChorus[2] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
							
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[3] += (sample * amplitude) >> 4;
			    destReverb[3] += (sample * amplitudeReverb) >> 4;
			    destChorus[3] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
				
			    dest += 4;
			    destReverb += 4;
			    destChorus += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *dest += (sample  * amplitude) >> 5;	// divide extra for summed stereo channels
				    *destReverb += (sample  * amplitudeReverb) >> 5;	// divide extra for summed stereo channels
				    *destChorus += (sample  * amplitudeChorus) >> 5;	// divide extra for summed stereo channels
				    dest++;
				    destReverb++;
				    destChorus++;
					
				    cur_wave += wave_increment;
				}
			    amplitude += amplitudeAdjust;
			}
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude << 4;
}

// $$kk: 04.19.99
// void PV_ServeInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping)
void PV_ServeInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*dest;
    register INT32 			*destReverb, *destChorus;
    register LOOPCOUNT		a, inner;
    register INT16 			*source, *calculated_source;
    register INT32			b, c, sample;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register INT32			amplitude, amplitudeAdjust;
    register INT32			amplitudeReverb, amplitudeChorus;

    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Four_Loop >> 4;
    amplitude = amplitude >> 4;

    dest = &MusicGlobals->songBufferDry[0];
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)dest, _MM_HINT_NTA);
#endif

			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[0] += (sample * amplitude) >> 4;
			    destReverb[0] += (sample * amplitudeReverb) >> 4;
			    destChorus[0] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
						
			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[1] += (sample * amplitude) >> 4;
			    destReverb[1] += (sample * amplitudeReverb) >> 4;
			    destChorus[1] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[2] += (sample * amplitude) >> 4;
			    destReverb[2] += (sample * amplitudeReverb) >> 4;
			    destChorus[2] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[3] += (sample * amplitude) >> 4;
			    destReverb[3] += (sample * amplitudeReverb) >> 4;
			    destChorus[3] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    dest += 4;
			    destReverb += 4;
			    destChorus += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

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
				    *destReverb += ((sample >> 1) * amplitudeReverb) >> 5;
				    *destChorus++ += ((sample >> 1) * amplitudeChorus) >> 5;
				    dest++; destReverb++;
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
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[0] += (sample * amplitude) >> 4;
			    destReverb[0] += (sample * amplitudeReverb) >> 4;
			    destChorus[0] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;
						
			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[1] += (sample * amplitude) >> 4;
			    destReverb[1] += (sample * amplitudeReverb) >> 4;
			    destChorus[1] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[2] += (sample * amplitude) >> 4;
			    destReverb[2] += (sample * amplitudeReverb) >> 4;
			    destChorus[2] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    THE_CHECK(INT16 *);
			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b));
			    dest[3] += (sample * amplitude) >> 4;
			    destReverb[3] += (sample * amplitudeReverb) >> 4;
			    destChorus[3] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    dest += 4;
			    destReverb += 4;
			    destChorus += 4;
			    amplitude += amplitudeAdjust;
			}
		}
	    else
		{
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = (amplitude >> 7) * this_voice->reverbLevel;
			    amplitudeChorus = (amplitude >> 7) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0] + calculated_source[1];
				    c = calculated_source[2] + calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *dest += ((sample >> 1) * amplitude) >> 5;
				    *destReverb += ((sample >> 1) * amplitudeReverb) >> 5;
				    *destChorus++ += ((sample >> 1) * amplitudeChorus) >> 5;
				    dest++; destReverb++;
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

// $$kk: 04.19.99
// void PV_ServeStereoInterp2FullBuffer16NewReverb (GM_Voice *this_voice)
void PV_ServeStereoInterp2FullBuffer16NewReverb (GM_Voice *this_voice, void *threadContext)
{
    register INT32 			*destL;
    register INT32 			*destReverb, *destChorus;
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
    register INT32			amplitudeReverb, amplitudeChorus;

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
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[0] += (sample * amplitudeL) >> 4;
			    destL[1] += (sample * amplitudeR) >> 4;
			    destReverb[0] += (sample * amplitudeReverb) >> 4;
			    destChorus[0] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[2] += (sample * amplitudeL) >> 4;
			    destL[3] += (sample * amplitudeR) >> 4;
			    destReverb[1] += (sample * amplitudeReverb) >> 4;
			    destChorus[1] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[4] += (sample * amplitudeL) >> 4;
			    destL[5] += (sample * amplitudeR) >> 4;
			    destReverb[2] += (sample * amplitudeReverb) >> 4;
			    destChorus[2] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[6] += (sample * amplitudeL) >> 4;
			    destL[7] += (sample * amplitudeR) >> 4;
			    destReverb[3] += (sample * amplitudeReverb) >> 4;
			    destChorus[3] += (sample * amplitudeChorus) >> 4;
			    destL += 8;
			    destReverb += 4;
			    destChorus += 4;

			    cur_wave += wave_increment;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

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
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    destL += 2;
				    destReverb++;
				    destChorus++;
			
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[0] += (sample * amplitudeL) >> 4;
			    destL[1] += (sample * amplitudeR) >> 4;
			    destReverb[0] += (sample * amplitudeReverb) >> 4;
			    destChorus[0] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[2] += (sample * amplitudeL) >> 4;
			    destL[3] += (sample * amplitudeR) >> 4;
			    destReverb[1] += (sample * amplitudeReverb) >> 4;
			    destChorus[1] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[4] += (sample * amplitudeL) >> 4;
			    destL[5] += (sample * amplitudeR) >> 4;
			    destReverb[2] += (sample * amplitudeReverb) >> 4;
			    destChorus[2] += (sample * amplitudeChorus) >> 4;
			    cur_wave += wave_increment;

			    b = source[cur_wave>>STEP_BIT_RANGE];
			    c = source[(cur_wave>>STEP_BIT_RANGE)+1];
			    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
			    destL[6] += (sample * amplitudeL) >> 4;
			    destL[7] += (sample * amplitudeR) >> 4;
			    destReverb[3] += (sample * amplitudeReverb) >> 4;
			    destChorus[3] += (sample * amplitudeChorus) >> 4;
			    destL += 8;
			    destReverb += 4;
			    destChorus += 4;

			    cur_wave += wave_increment;
			    amplitudeL += amplitudeLincrement;
			    amplitudeR += amplitudeRincrement;
			}
		}
	    else
		{	// stereo 16 bit instrument
		    for (a = MusicGlobals->Four_Loop; a > 0; --a)
			{
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    destL += 2;
				    destReverb++;
				    destChorus++;
			
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

// $$kk: 04.19.99
// void PV_ServeStereoInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping)
void PV_ServeStereoInterp2PartialBuffer16NewReverb (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 			*destL;
    register INT32 			*destReverb, *destChorus;
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
    register INT32			amplitudeReverb, amplitudeChorus;

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
    destReverb = &MusicGlobals->songBufferReverb[0];
    destChorus = &MusicGlobals->songBufferChorus[0];
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    _mm_prefetch((char *)&source[(cur_wave>>STEP_BIT_RANGE) + cur_wave_next_frame], _MM_HINT_NTA);
#ifdef USE_KATMAI_WRITE_CACHE
			    _mm_prefetch((char *)destL, _MM_HINT_NTA);
#endif

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    destL[1] += (sample * amplitudeR) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 4;
				    *destChorus += (sample * amplitudeChorus) >> 4;
				    destL += 2;
				    destReverb++;
				    destChorus++;
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

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
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    destL += 2;
				    destReverb++;
				    destChorus++;
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + (cur_wave>> STEP_BIT_RANGE);
				    b = *calculated_source;
				    c = calculated_source[1];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    destL[1] += (sample * amplitudeR) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 4;
				    *destChorus += (sample * amplitudeChorus) >> 4;
				    destL += 2;
				    destReverb++;
				    destChorus++;
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
			    amplitudeReverb = ((amplitudeL + amplitudeR) >> 8) * this_voice->reverbLevel;
			    amplitudeChorus = ((amplitudeL + amplitudeR) >> 8) * this_voice->chorusLevel;

			    for (inner = 0; inner < 4; inner++)
				{
				    THE_CHECK(INT16 *);
				    calculated_source = source + ((cur_wave>> STEP_BIT_RANGE) * 2);
				    b = calculated_source[0];
				    c = calculated_source[2];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    *destL += (sample * amplitudeL) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    b = calculated_source[1];
				    c = calculated_source[3];
				    sample = (((INT32) (cur_wave & STEP_FULL_RANGE) * (c-b))>>STEP_BIT_RANGE) + b;
				    destL[1] += (sample * amplitudeR) >> 4;
				    *destReverb += (sample * amplitudeReverb) >> 5;
				    *destChorus += (sample * amplitudeChorus) >> 5;
				    destL += 2;
				    destReverb++;
				    destChorus++;
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

#endif	// USE_SMALL_MEMORY_REVERB
