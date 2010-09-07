/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)GenSynthResample.c	1.7 03/12/19
 */

/*****************************************************************************/
/*
**	Mixes a voice using a resample algorithm.
**
** Modification History:
**
**	01/05/02		Created by FB
*/
/*****************************************************************************/

#include "GenSnd.h"
#include "GenPriv.h"

/* For mono code:
 * do standard linear interpolation here. 
 * This code is only used in case of an audio device that is
 * not capable of 16 bit output. Pretty rare these days.
 */

//#define XDEBUG
//#define TRACE_DEBUG
//#define DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
#include <stdio.h>
#include "X_API.h"
#include "HAE_API.h"
#endif

// MONO output

void PV_ServeResampleFullBuffer(GM_Voice *this_voice, void *threadContext) {
    PV_ServeInterp2FullBuffer(this_voice, threadContext);
}

void PV_ServeResamplePartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext) {
    PV_ServeInterp2PartialBuffer(this_voice, looping, threadContext);
}

void PV_ServeResampleFullBuffer16 (GM_Voice *this_voice, void *threadContext) {
    PV_ServeInterp2FullBuffer16(this_voice, threadContext);
}

void PV_ServeResamplePartialBuffer16 (GM_Voice *this_voice, XBOOL looping, void *threadContext) {
    PV_ServeInterp2PartialBuffer16(this_voice, looping, threadContext);
}

// STEREO OUTPUT CODE

void PV_ServeStereoResampleFullBuffer (GM_Voice *this_voice, void *threadContext) {
    PV_ServeStereoResampleFullBuffer16(this_voice, threadContext);
}

void PV_ServeStereoResamplePartialBuffer (GM_Voice *this_voice, XBOOL looping, void *threadContext) {
    PV_ServeStereoResamplePartialBuffer16(this_voice, looping, threadContext);
}

#ifdef DEBUG_OUTPUT
static INT32 outfh = 0;
static INT32 infh = 0;
UINT16 debugOutputBuffer[1024];

void writeOutput() {
    if (!outfh) {
	outfh = HAE_FileOpenForWrite("resample_output.pcm");
	if (outfh != 0) {
	    printf("Resampler: Opened debug output file\n");
	} else {
	    printf("Resampler: Opened NOT debug output file\n");
	}
    }
    if (outfh) {
	int i; UINT32* inp=(UINT32*) (&MusicGlobals->songBufferDry[0]);
	for (i=0; i<512 * 2; i++) { // 2 channels
	    debugOutputBuffer[i]=(UINT16) ((*inp++) >> OUTPUT_SCALAR);
	}
	HAE_WriteFile(outfh, &debugOutputBuffer[0], MusicGlobals->Four_Loop * 4 * 2 * 2); // 2 bytes/sample + 2 channels
    }
}
void writeInput(void* input, int length) {
    if (!infh) {
	infh = HAE_FileOpenForWrite("resample_input.pcm");
	if (infh != 0) {
	    printf("Resampler: Opened debug input file\n");
	} else {
	    printf("Resampler: Opened NOT debug input file\n");
	}
    }
    if (infh) {
	HAE_WriteFile(infh, input, length);
    }
}
#endif


void PV_ServeStereoResampleFullBuffer16 (GM_Voice *this_voice, void *threadContext) {
    INT32 		*destL;   // pointer to output sample
    XFIXED 		cur_wave; // in frames
    INT32		ampValueL, ampValueR;
    INT32		amplitudeL, amplitudeR, ampIncL, ampIncR;
    INT32		in_length, out_length, inFrameSize;
    UINT32		newInSampleRate;
    UBYTE		*src, *source;

#ifdef TRACE_DEBUG
    printf("> PV_ServeStereoResampleFullBuffer16\n");
    //PV_ServeStereoInterp2FullBuffer16(this_voice, threadContext);
    //return;
#endif
	
#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel) { 
	PV_ServeStereoInterp2FullBuffer16NewReverb (this_voice, threadContext);
	return;
    }
#endif
    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    ampIncL = ((ampValueL - amplitudeL) / (MusicGlobals->Four_Loop)) >> 4;
    ampIncR = ((ampValueR - amplitudeR) / (MusicGlobals->Four_Loop)) >> 4;

    amplitudeL = amplitudeL >> 4;
    amplitudeR = amplitudeR >> 4;

    destL = &MusicGlobals->songBufferDry[0];

    cur_wave = this_voice->NoteWave;
    source = (this_voice->NotePtr);
    inFrameSize = this_voice->bitSize * this_voice->channels / 8;
	
    /* in_length in frames */
    /* somehow, NotePtrEnd is not really pointing to the notes last sample ! */
    //in_length = (((UINT_PTR) this_voice->NotePtrEnd) - ((UINT_PTR) source)) / inFrameSize - (cur_wave >> STEP_BIT_RANGE);
    in_length = (INT32) ((((UINT_PTR) this_voice->NotePtrEnd) - ((UINT_PTR) source)) - (cur_wave >> STEP_BIT_RANGE));
    out_length = MusicGlobals->Four_Loop * 4;

#ifdef XDEBUG
    printf("BEFORE: in_length=%d frames. out_length=%d frames. channels=%d, bitSize=%d, cur_wave=%d == %d.%d, end_wave=%d\n",
	   in_length, out_length, this_voice->channels, this_voice->bitSize, cur_wave, cur_wave>>STEP_BIT_RANGE, cur_wave & 0xFFF, (this_voice->NotePtrEnd - this_voice->NotePtr - 1));
#endif	

    /* has the sample rate changed ? */
    newInSampleRate = (UINT32) (XFIXED_TO_UNSIGNED_LONG((this_voice->NotePitch * 22050L) + XFIXED_1 / 2));
    if (newInSampleRate != this_voice->resampleParams->input_samplerate) {
#ifdef XDEBUG
	printf("New sample rate of %d\n", newInSampleRate);
#endif
	SR_change_samplerate(this_voice->resampleParams, newInSampleRate, this_voice->resampleParams->output_samplerate);
    }
    /* SR_resample32_add takes length arguments in frames. cur_wave, etc. are in frames, too. */
    src = (UBYTE*) (((UINT_PTR) source) + ((cur_wave>>STEP_BIT_RANGE) * inFrameSize));
    SR_resample32_add(this_voice->resampleParams, this_voice->channels, this_voice->bitSize,
		      amplitudeL, amplitudeR,
		      ampIncL, ampIncR,
		      src, &in_length, 
		      destL, &out_length);
#ifdef DEBUG_OUTPUT
    printf("Writing from offset %d: %d bytes\n", 
	   (INT32) (src - ((UBYTE*) source)), in_length*inFrameSize);
    writeInput(src, in_length*inFrameSize);
#endif
    cur_wave += (in_length << STEP_BIT_RANGE);


#ifdef XDEBUG
    printf("AFTER: in_length=%d frames. out_length=%d frames. \n",
	   in_length, out_length);
    printf("Short position: before: %d.%d\n", oldcur_wave>>STEP_BIT_RANGE, oldcur_wave & 0xFFF);
#endif		
    /* update with new input position */
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = ampValueL;
    this_voice->lastAmplitudeR = ampValueR;

#ifdef XDEBUG
    printf("PV_ServeStereoResampleFullBuffer16. Sample position: before: %d.%d    after: %d.%d\n", 
	   oldcur_wave>>STEP_BIT_RANGE, oldcur_wave & 0xFFF,
	   cur_wave>>STEP_BIT_RANGE, cur_wave & 0xFFF); //fflush(stdout);
#endif
#ifdef DEBUG_OUTPUT
    writeOutput();
#endif
#ifdef TRACE_DEBUG
    printf("< PV_ServeStereoResampleFullBuffer16\n");
#endif
}

void PV_ServeStereoResamplePartialBuffer16(GM_Voice *this_voice, XBOOL looping, void *threadContext) {
    INT32 		*destL;   // pointer to output sample
    INT16 		*source;  // pointer to input sample
    XFIXED 		cur_wave;     // current input sample number
    XFIXED 		wave_increment;
    XFIXED 		end_wave;     // end sample number
    XFIXED		wave_adjust;  // length of loop portion
    INT32		ampValueL, ampValueR;
    INT32		amplitudeL, amplitudeR;
    INT32		ampIncL, ampIncR;
    INT32		in_length, out_length, remaining_out, inFrameSize;
    UINT32		newInSampleRate;
    UBYTE		*src;

#if (USE_SMALL_MEMORY_REVERB == FALSE) && (USE_VARIABLE_REVERB == TRUE)
    if (this_voice->reverbLevel || this_voice->chorusLevel) {
	PV_ServeStereoInterp2PartialBuffer16NewReverb (this_voice, looping, threadContext);
	return;
    }
#endif
#ifdef TRACE_DEBUG
    printf("> PV_ServeStereoResamplePartialBuffer16\n");
#endif

    PV_CalculateStereoVolume(this_voice, &ampValueL, &ampValueR);
    amplitudeL = this_voice->lastAmplitudeL;
    amplitudeR = this_voice->lastAmplitudeR;
    ampIncL = ((ampValueL - amplitudeL) / (MusicGlobals->Four_Loop)) >> 4;
    ampIncR = ((ampValueR - amplitudeR) / (MusicGlobals->Four_Loop)) >> 4;

    amplitudeL = amplitudeL >> 4;
    amplitudeR = amplitudeR >> 4;

    destL = &MusicGlobals->songBufferDry[0];
    cur_wave = this_voice->NoteWave;
    source = (short *) this_voice->NotePtr;
    inFrameSize = this_voice->bitSize * this_voice->channels / 8;

    wave_increment = PV_GetWavePitch(this_voice->NotePitch);

    if (looping) {
	end_wave = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	// $$fb 2002-02-10: fix for 4780440: REGRESSION: loud clicks during playback
	wave_adjust = (XFIXED) (this_voice->NoteLoopEnd - this_voice->NoteLoopPtr) << STEP_BIT_RANGE;
    } else {
	end_wave = (XFIXED) (this_voice->NotePtrEnd - this_voice->NotePtr) << STEP_BIT_RANGE;
	wave_adjust = 0;
    }

    //* has the sample rate changed ? * /
    newInSampleRate = (UINT32) (XFIXED_TO_UNSIGNED_LONG((this_voice->NotePitch * 22050L) + XFIXED_1 / 2));
    if (newInSampleRate != this_voice->resampleParams->input_samplerate) {
#ifdef XDEBUG
	printf("  New sample rate of %d\n", newInSampleRate);
#endif
	SR_change_samplerate(this_voice->resampleParams, newInSampleRate, this_voice->resampleParams->output_samplerate);
    }

    remaining_out = MusicGlobals->Four_Loop * 4;
    while (remaining_out>0) {
	out_length = remaining_out;
	// $$fb 2002-02-10: fix for 4780440: REGRESSION: loud clicks during playback
	in_length = ((end_wave - cur_wave) >> STEP_BIT_RANGE);
		
#ifdef TRACE_DEBUG
	printf("  before: in_length=%d, out_length=%d, cur_wave=%d == %d.%d,     end_wave=%d == %d.%d,   wave_adjust=%d == %d.%d\n", in_length, out_length, cur_wave, cur_wave>>STEP_BIT_RANGE, cur_wave & 0xFFF, end_wave, end_wave>>STEP_BIT_RANGE, end_wave & 0xFFF, wave_adjust, wave_adjust>>STEP_BIT_RANGE, wave_adjust & 0xFFF);
#endif
	src = (UBYTE*) (((UINT_PTR) source) + ((cur_wave>>STEP_BIT_RANGE) * inFrameSize));
	SR_resample32_add(this_voice->resampleParams, this_voice->channels, this_voice->bitSize,
			  amplitudeL, amplitudeR,
			  ampIncL, ampIncR,
			  src, &in_length, 
			  destL, &out_length);
#ifdef DEBUG_OUTPUT
	printf("Reading input data from offset %d: %d bytes\n", 
	       (INT32) (src - ((UBYTE*) source)), in_length*inFrameSize);
	writeInput(src, in_length*inFrameSize);
#endif
	destL += out_length*2; // output_channels=2
	remaining_out -= out_length;
	cur_wave += (in_length << STEP_BIT_RANGE);
#ifdef TRACE_DEBUG
	printf("  after: in_length=%d, out_length=%d, cur_wave=%d == %d.%d,     end_wave=%d == %d.%d,   wave_adjust=%d == %d.%d\n", in_length, out_length, cur_wave, cur_wave>>STEP_BIT_RANGE, cur_wave & 0xFFF, end_wave, end_wave>>STEP_BIT_RANGE, end_wave & 0xFFF, wave_adjust, wave_adjust>>STEP_BIT_RANGE, wave_adjust & 0xFFF);
#endif
	THE_CHECK(INT16 *);

	if (((INT32) cur_wave) < 0) {
	    cur_wave = 0;
	}
    }
    this_voice->NoteWave = cur_wave;
    this_voice->lastAmplitudeL = ampValueL;
    this_voice->lastAmplitudeR = ampValueR;
FINISH:
#ifdef DEBUG_OUTPUT
    writeOutput();
#endif
#ifdef TRACE_DEBUG
    printf("< PV_ServeStereoResamplePartialBuffer16\n");
#endif
    return;
}
	
// EOF of GenSynthResample.c
