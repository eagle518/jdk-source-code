/*
 * @(#)GenSynthInterp1.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenSynthInterp1.c"
**
**
** Modification History:
**
**	1/18/96		Spruced up for C++ extra error checking
**				Changed the macro 'THE_CHECK' to accept a type for typecasting the 
**				source pointer
**	3/1/96		Removed extra PV_DoCallBack, and PV_GetWavePitch
**	5/2/96		Changed int to BOOL_FLAG
**	12/30/96	Changed copyright
**	2/3/98		Renamed songBufferLeftMono to songBufferDry
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	11/10/98	Removed CLIP macro
*/
/*****************************************************************************/

#include "GenSnd.h"
#include "GenPriv.h"

#if USE_TERP1 == TRUE

/* Interpolation type 1: two point linear interpolation
*/
// $$kk: 04.19.99
// void PV_ServeInterp1FullBuffer (GM_Voice *this_voice)
void PV_ServeInterp1FullBuffer (GM_Voice *this_voice, void *threadContext)
{
    register INT32 FAR		*dest;
    register LOOPCOUNT		a, inner;
    register UBYTE FAR		*source;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED			cur_wave_shift;
    register INT32			amplitude, amplitudeAdjust;
    register INT16			j;

    if (this_voice->channels > 1)
	{
	    PV_ServeInterp2FullBuffer(this_voice);
	    return;
	}

    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Sixteen_Loop;
    dest = &MusicGlobals->songBufferDry[0];

    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);

    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
	{
	    for (inner = 0; inner < 16; inner++)
		{
		    cur_wave_shift = cur_wave>>STEP_BIT_RANGE;
		    j = source[cur_wave_shift];
		    if (cur_wave & STEP_OVERFLOW_FLAG)
			*dest += ((j + source[cur_wave_shift+1] - 0x100) * amplitude) >> 1;
		    else
			*dest += (j - 0x80) * amplitude;
		    dest++;
		    cur_wave += wave_increment;
		}
	    amplitude += amplitudeAdjust;
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude;
}

// $$kk: 04.19.99
// void PV_ServeInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping)
void PV_ServeInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 FAR		*dest;
    register LOOPCOUNT		a, inner;
    register UBYTE FAR		*source;
    register XFIXED 		cur_wave;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    register XFIXED			cur_wave_shift;

    register INT32			amplitude, amplitudeAdjust;

    if (this_voice->channels > 1)
	{
	    PV_ServeInterp2PartialBuffer(this_voice, looping);
	    return;
	}
    amplitude = this_voice->lastAmplitudeL;
    amplitudeAdjust = (this_voice->NoteVolume * this_voice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;
    amplitudeAdjust = (amplitudeAdjust - amplitude) / MusicGlobals->Sixteen_Loop;
    dest = &MusicGlobals->songBufferDry[0];

    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);

    if (looping)
	{
	    end_wave = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	    wave_adjust = (this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;
	}
    else
	end_wave = (XFIXED) (this_voice->NotePtrEnd - this_voice->NotePtr - 1) << STEP_BIT_RANGE;
    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
	{
	    for (inner = 0; inner < 16; inner++)
		{
		    THE_CHECK(UBYTE *);
		    cur_wave_shift = cur_wave>>STEP_BIT_RANGE;
		    if (cur_wave & STEP_OVERFLOW_FLAG)
			*dest += ((source[cur_wave_shift]+source[cur_wave_shift+1] - 0x100) * amplitude) >> 1;
		    else
			*dest += (source[cur_wave_shift] - 0x80) * amplitude;
		    dest++;
		    cur_wave += wave_increment;
		}
	    amplitude += amplitudeAdjust;
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = amplitude;
FINISH:
    return;
}


// $$kk: 04.19.99
// void PV_ServeStereoInterp1FullBuffer (GM_Voice *this_voice)
void PV_ServeStereoInterp1FullBuffer (GM_Voice *this_voice, void *threadContext)
{
    register INT32 FAR		*destL;
    register LOOPCOUNT		a, inner;
    register UBYTE FAR		*source;
    register XFIXED			cur_wave;
    register XFIXED			cur_wave_shift;
    register XFIXED 		wave_increment;
    INT32				ampValueL, ampValueR;
    register INT32 amplitudeL;
    register INT32 amplitudeR;
    register INT32 amplitudeLincrement, amplitudeRincrement;
    register INT32 sample;

    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Sixteen_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Sixteen_Loop);

    destL = &MusicGlobals->songBufferDry[0];

    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);

    if (this_voice->channels == 1)
	{	// mono instrument
	    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
		{
		    for (inner = 0; inner < 16; inner++)
			{
			    cur_wave_shift = cur_wave>>STEP_BIT_RANGE;
			    if (cur_wave & STEP_OVERFLOW_FLAG)
				{
				    sample = source[cur_wave_shift]+source[cur_wave_shift+1] - 0x100;
				    *destL += (sample * amplitudeL) >> 1;
				    destL[1] += (sample * amplitudeR) >> 1;
				}
			    else
				{
				    sample = source[cur_wave_shift] - 0x80;
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				}
			    destL += 2;
			    cur_wave += wave_increment;
			}
		    amplitudeL += amplitudeLincrement;
		    amplitudeR += amplitudeRincrement;
		}
	}
    else
	{	// stereo 8 bit instrument
	    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
		{
		    for (inner = 0; inner < 16; inner++)
			{
			    cur_wave_shift = (cur_wave>>STEP_BIT_RANGE) * 2;
			    if (cur_wave & STEP_OVERFLOW_FLAG)
				{
				    *destL += ((source[cur_wave_shift+0]+source[cur_wave_shift+2] - 0x100) * amplitudeL) >> 1;
				    destL[1] += ((source[cur_wave_shift+1]+source[cur_wave_shift+3] - 0x100) * amplitudeR) >> 1;
				}
			    else
				{
				    *destL += ((source[cur_wave_shift+0] - 0x80) * amplitudeL);
				    destL[1] += ((source[cur_wave_shift+1] - 0x80) * amplitudeR);
				}
			    destL += 2;
			    cur_wave += wave_increment;
			}
		    amplitudeL += amplitudeLincrement;
		    amplitudeR += amplitudeRincrement;
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = ampValueL;
    this_voice->lastAmplitudeR = ampValueR;
}

// $$kk: 04.19.99
// void PV_ServeStereoInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping)
void PV_ServeStereoInterp1PartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext)
{
    register INT32 FAR		*destL;
    register LOOPCOUNT		a, inner;
    register UBYTE FAR		*source;
    register XFIXED 		cur_wave;
    register XFIXED			cur_wave_shift;
    register XFIXED 		wave_increment;
    register XFIXED 		end_wave, wave_adjust;
    INT32				ampValueL, ampValueR;
    register INT32 sample;
    register INT32 amplitudeL;
    register INT32 amplitudeR;
    register INT32 amplitudeLincrement, amplitudeRincrement;

    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    amplitudeLincrement = (ampValueL - amplitudeL) / (MusicGlobals->Sixteen_Loop);
    amplitudeRincrement = (ampValueR - amplitudeR) / (MusicGlobals->Sixteen_Loop);

    destL = &MusicGlobals->songBufferDry[0];

    source = this_voice->NotePtr;
    cur_wave = this_voice->NoteWave;
    wave_increment = PV_GetWavePitch(this_voice->NotePitch);
    if (looping)
	{
	    end_wave = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	    wave_adjust = (this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;
	}
    else
	end_wave = (XFIXED) (this_voice->NotePtrEnd - this_voice->NotePtr - 1) << STEP_BIT_RANGE;

    if (this_voice->channels == 1)
	{	// mono instrument
	    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
		{
		    for (inner = 0; inner < 16; inner++)
			{
			    THE_CHECK(UBYTE *);
			    cur_wave_shift = cur_wave>>STEP_BIT_RANGE;
			    if (cur_wave & STEP_OVERFLOW_FLAG)
				{
				    sample = source[cur_wave_shift]+source[cur_wave_shift+1] - 0x100;
				    *destL += (sample * amplitudeL) >> 1;
				    destL[1] += (sample * amplitudeR) >> 1;
				}
			    else
				{
				    sample = source[cur_wave_shift] - 0x80;
				    *destL += sample * amplitudeL;
				    destL[1] += sample * amplitudeR;
				}
			    destL += 2;
			    cur_wave += wave_increment;
			}
		    amplitudeL += amplitudeLincrement;
		    amplitudeR += amplitudeRincrement;
		}
	}
    else
	{	// stereo 8 bit instrument
	    for (a = MusicGlobals->Sixteen_Loop; a > 0; --a)
		{
		    for (inner = 0; inner < 16; inner++)
			{
			    THE_CHECK(UBYTE *);
			    cur_wave_shift = (cur_wave>>STEP_BIT_RANGE) * 2;
			    if (cur_wave & STEP_OVERFLOW_FLAG)
				{
				    *destL += ((source[cur_wave_shift+0]+source[cur_wave_shift+2] - 0x100) * amplitudeL) >> 1;
				    destL[1] += ((source[cur_wave_shift+1]+source[cur_wave_shift+3] - 0x100) * amplitudeR) >> 1;
				}
			    else
				{
				    *destL += ((source[cur_wave_shift+0] - 0x80) * amplitudeL);
				    destL[1] += ((source[cur_wave_shift+1] - 0x80) * amplitudeR);
				}
			    destL += 2;
			    cur_wave += wave_increment;
			}
		    amplitudeL += amplitudeLincrement;
		    amplitudeR += amplitudeRincrement;
		}
	}
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = ampValueL;
    this_voice->lastAmplitudeR = ampValueR;
FINISH:
    return;
}

#endif	// 	(USE_TERP1 == TRUE

// EOF of GenSynthInterp1.c

