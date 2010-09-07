/*
 * @(#)GenSample.c	1.29 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenSample.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Overview
**	General purpose Music Synthesis software, C-only implementation
**	(no assembly language optimizations made)
**
** Modification History:
**
**	4/15/97		Moved sample API functions to GenSample.c
**				Added code to handle sample callbacks
**				Added GM_AddSampleOffsetCallback & GM_RemoveSampleOffsetCallback
**	5/1/97		Added startOffsetFrame to GM_BeginSampleFromInfo
**	5/7/97		Changed various functions that wanted stereo pan to be in the range
**				of -255 to 255, to now accecpt the natural range of -63 to 63
**	5/8/97		Fixed GM_ChangeSampleVolume & GM_GetSampleVolume to scale the volume
**				correctly based upon the current effectsVolume
**	5/21/97		Added GM_GetSamplePlaybackPosition
**	6/17/97		Modified GM_BeginSample & GM_BeginDoubleBuffer to save the volume
**				level unscaled in the voice allocated for later use
**				Moved GM_GetEffectsVolume & GM_SetEffectsVolume from GenSetup.c and
**				modified GM_SetEffectsVolume to scale volume levels based upon
**				masterVolume and effectsVolume using the unscaled sample volume
**	6/25/97		Added GM_SetSampleLoopPoints
**	7/22/97		Added GM_GetSampleVolumeUnscaled
**	9/15/97		Added GM_GetSampleReverb
**	10/26/97	Fixed a bug with GM_BeginDoubleBuffer that forgot to look for
**				MusicGlobals being deallocated
**	11/19/97	Removed zero volume check in GM_BeginDoubleBuffer & GM_BeginSample
**	12/16/97	Moe: removed compiler warnings
**	1/12/98		Modified GM_IsSoundDone to pass TRUE/FALSE correctly
**	1/14/98		kk: added number of loops to GM_BeginSample calls
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/10/98		Changed GM_BeginSample to use max duration count
**	2/11/98		Put code wrappers around functions not used for WebTV
**	3/5/98		Changed GM_BeginDoubleBuffer && GM_BeginSample to not play samples
**				out of MAX_SAMPLE_FRAMES range
**	3/12/98		Modified GM_BeginDoubleBuffer to include a sample done callback
**	7/6/98		Fixed a compiler warning with GM_BeginSampleFromInfo
**	7/28/98		Renamed inst_struct to pInstrument
**	7/30/98		Fixed GM_ChangeSampleReverb to set the reverbLevel for new verbs when
**				enabling verb on sound effects
**	8/5/98		Changed GM_BeginDoubleBuffer & GM_BeginSample to allocate the voice
**				before it starts filling in parameters for control.
**	8/6/98		Changed the order of voice allocate in GM_BeginDoubleBuffer & GM_BeginSample
**				from the last voice to the first voice.
**	8/11/98		Added to GM_ChangeSampleReverb an amount of 25 plus the enable threshold to
**				actaully active the verb. Also optimized GM_ChangeSampleReverb a little.
**	9/12/98		Added GM_GetSamplePlaybackPointer
**	10/17/98	Fixed almost all (!) the functions that change sample related parameters to
**				didn't allow a voice sample in voice 0 to be modified.
**	11/9/98		Renamed NoteDur to voiceMode
**	11/24/98	Added GM_GetSampleReverbAmount & GM_SetSampleReverbAmount
**	3/1/99		Added VOICE_REFERENCE and changed all sample API's to use this new voice
**				reference type.
**				Changed NoteRefNum to NoteContext
**	3/3/99		Renamed myMusicGlobals to pMixer
**				Changed GM_IsSoundDone to scan all voices rather the limited range
**				Changed GM_EndSample to use function rather than direct reference
**				Removed all extra references to MusicGlobals
**				Added GM_GetSampleStartTimeStamp
**				Renamed GM_BeginSample to GM_SetupSample, GM_BeginDoubleBuffer to GM_SetupSampleDoubleBuffer,
**				GM_BeginSampleFromInfo to GM_SetupSampleFromInfo, and added GM_StartSample
**				Added GM_GetSampleFrequencyFilter GM_SetSampleFrequencyFilter GM_GetSampleResonanceFilter
**				GM_SetSampleResonanceFilter GM_GetSampleLowPassAmountFilter GM_SetSampleLowPassAmount
**	3/5/99		Added threadContext to PV_ServeEffectCallbacks
**				Added GM_SetSyncSampleStartReference & GM_SyncStartSample
**	3/6/99		Added documentation
**	3/8/99		Renamed GM_EndSoundEffects to GM_EndAllSamples
**	2002-01-07	$$fb Added GM_ChangeSampleResample and GM_GetSampleResample
**	2002-04-20	$$fb Added GM_ReleaseSample and GM_ReleaseAllSamples
*/
/*****************************************************************************/
#include "GenSnd.h"
#include "GenPriv.h"
#include "HAE_API.h"

/*
	Description of use:

	Call GM_SetEffectsVolume and GM_GetEffectsVolume to control the overal mix level of sound effects into
	the mixer.

	In general, all of these functions create, manipulate and dispose of a VOICE_REFERENCE. The VOICE_REFERENCE
	is a direct connect to an active or about to be active voice in the mixer. There are a limited number of
	them. MAX_VOICES to be exact.

	There are 3 ways to get a VOICE_REFERENCE. Call GM_SetupSample, GM_SetupSampleDoubleBuffer, or GM_SetupSampleFromInfo
	based upon your particular needs.

	This will allocate a VOICE_REFERENCE and preserve a voice in the mixer. You then need to call GM_StartSample
	to start the voice. Once the voice finishes it will be marked VOICE_UNUSED. You can also call GM_EndSample
	or GM_EndAllSamples. GM_ReleaseSample and GM_ReleaseAllSamples are thread-safe versions of them.

	MODIFIERS:
	Modifier functions of active VOICE_REFERENCE's. All functions will effect the voice within 11 ms
	of changing values.

	sample rate			GM_ChangeSamplePitch GM_GetSamplePitch

	volume				GM_ChangeSampleVolume GM_GetSampleVolumeUnscaled GM_GetSampleVolume GM_SetSampleFadeRate

	filter controls		GM_GetSampleFrequencyFilter GM_SetSampleFrequencyFilter GM_GetSampleResonanceFilter
						GM_SetSampleResonanceFilter GM_GetSampleLowPassAmountFilter GM_SetSampleLowPassAmountFilter

	loop points			GM_SetSampleLoopPoints

	stereo placement	GM_ChangeSampleStereoPosition GM_GetSampleStereoPosition

	reverb controls		GM_GetSampleReverbAmount GM_SetSampleReverbAmount GM_ChangeSampleReverb GM_GetSampleReverb

	position			GM_GetSamplePlaybackPointer GM_GetSamplePlaybackPosition

	information			GM_GetSampleStartTimeStamp GM_IsSoundDone

	callbacks			GM_SetSampleOffsetCallbackLinks GM_AddSampleOffsetCallback GM_RemoveSampleOffsetCallback
						GM_SetSampleDoneCallback


	SYNC START
	To start samples at the same time, call one of the GM_Setup... functions then call GM_SetSyncSampleStartReference
	to set a unique refernece. The reference can be a pointer to a local structure. Its not used as anything other
	that common reference for all voices that you need to start at the moment. After they are started it is ignored.
	Then GM_SyncStartSample to actaully activate the voices. They will start at the next 11 ms slice. Be careful
	using these functions directly because they don't wait for the mixer slice to be ready, so you might actaully
	start voices between 11ms slices. The best way to insure it is to use the linked voices below. Those function
	use the ones described.

	LINKED VOICES
	Call one of the GM_SetupSample... functions in the various standard ways, to get an allocate voice
	then call GM_NewLinkedSampleList. Then add it to your maintained top list of linked voices with
	by calling GM_AddLinkedSample. Use GM_FreeLinkedSampleList to delete an entire list,
	or GM_RemoveLinkedSample to just one link.

	Then you can call GM_StartLinkedSamples to start them all at the same time, or GM_EndLinkedSamples
	to end them all, or GM_SetLinkedSampleVolume, GM_SetLinkedSampleRate, and GM_SetLinkedSamplePosition
	set parameters about them all.

	management			GM_NewLinkedSampleList GM_FreeLinkedSampleList GM_AddLinkedSample GM_RemoveLinkedSample

	info				GM_GetLinkedSamplePlaybackReference

	control				GM_StartLinkedSamples GM_EndLinkedSamples

	sync control		GM_SetLinkedSampleVolume GM_SetLinkedSampleRate GM_SetLinkedSamplePosition

*/

#if X_PLATFORM != X_WEBTV
// range is 0 to MAX_MASTER_VOLUME (256). Note volume is from 0 to MAX_NOTE_VOLUME (127)
void GM_SetEffectsVolume(INT16 newVolume)
{
    register GM_Mixer		*pMixer;
    register LOOPCOUNT		count;
    register GM_Voice		*pVoice;
    short int				minValue, maxValue;

    if (newVolume < 0)
	{
	    newVolume = 0;
	}
    if (newVolume > MAX_MASTER_VOLUME * 5)
	{
	    newVolume = MAX_MASTER_VOLUME * 5;
	}
    pMixer = MusicGlobals;
    if (pMixer)
	{
	    minValue = pMixer->MaxNotes;
	    maxValue =  minValue + pMixer->MaxEffects;

	    pMixer->effectsVolume = newVolume;
	    newVolume = (newVolume * MAX_NOTE_VOLUME) / MAX_MASTER_VOLUME; // scale

	    // update the current notes playing to the new volume
	    for (count = minValue; count < maxValue; count++)
		{
		    pVoice = &pMixer->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    if (pVoice->NoteChannel == SOUND_EFFECT_CHANNEL)
				{
				    // make sure and set the channel volume not scaled, because its scaled later
				    if (newVolume == 0)
					{
					    pVoice->voiceMode = VOICE_RELEASING;
					    pVoice->NoteDecay = 0;
					    pVoice->volumeADSRRecord.ADSRTime[0] = 1;
					    pVoice->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
					    pVoice->volumeADSRRecord.ADSRLevel[0] = 0;	// just in case
					}
				    // now calculate the new volume based upon the current channel volume and
				    // the unscaled note volume
				    newVolume = (pVoice->NoteMIDIVolume * pMixer->MasterVolume) / MAX_MASTER_VOLUME;
				    newVolume = (newVolume * pMixer->effectsVolume) / MAX_MASTER_VOLUME;
				    pVoice->NoteVolume = newVolume;
				}
			}
		}
	}
}

INT16 GM_GetEffectsVolume(void)
{
    INT16	volume;

    volume = MAX_MASTER_VOLUME;
    if (MusicGlobals)
	{
	    volume = MusicGlobals->effectsVolume;
	}
    return volume;
}
#endif


#if X_PLATFORM != X_WEBTV
// Process any fading effects voices

// $$kk: 04.19.99
//void PV_ServeEffectsFades(void)
void PV_ServeEffectsFades(void *threadContext)
{
    INT32		count, minValue, maxValue;
    GM_Voice	*pVoice;
    INT32		value;
    GM_Mixer	*pMixer;

    pMixer = MusicGlobals;
    if (pMixer)
	{
	    minValue = pMixer->MaxNotes;					// only look this voice range
	    maxValue =  minValue + (pMixer->MaxEffects-1);
	    for (count = maxValue; count >= minValue; count--)
		{
		    pVoice = &pMixer->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    if (pVoice->soundFadeRate)
				{
				    pVoice->soundFixedVolume -= pVoice->soundFadeRate;
				    value = XFIXED_TO_LONG(pVoice->soundFixedVolume);
				    if (value > pVoice->soundFadeMaxVolume)
					{
					    value = pVoice->soundFadeMaxVolume;
					    pVoice->soundFadeRate = 0;
					}
				    if (value < pVoice->soundFadeMinVolume)
					{
					    value = pVoice->soundFadeMinVolume;
					    pVoice->soundFadeRate = 0;
					}
				    pVoice->NoteVolume = (INT32)value;
				    pVoice->NoteMIDIVolume = (INT16)value;
				    if ((pVoice->soundFadeRate == 0) && pVoice->soundEndAtFade)
					{
					    //$$fb 2002-04-20: this is only called from the main audio thread,
					    // -> safe to call GM_EndSample rather than GM_ReleaseSample
					    GM_EndSample((VOICE_REFERENCE)count, threadContext);
					}
				}
			}
		}
	}
}
#endif

#if X_PLATFORM != X_WEBTV
void PV_ServeEffectCallbacks(void *threadContext)
{
    INT32					count, minValue, maxValue;
    UINT32			offsetStart, offsetEnd;
    GM_Voice				*pVoice;
    GM_SampleCallbackEntry	*pCallbackEntry;
    GM_Mixer				*pMixer;

    pMixer = MusicGlobals;
    if (pMixer)
	{
	    minValue = pMixer->MaxNotes;					// only look this voice range
	    maxValue =  minValue + (pMixer->MaxEffects-1);
	    for (count = maxValue; count >= minValue; count--)
		{
		    pVoice = &pMixer->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    pCallbackEntry = pVoice->pSampleMarkList;
			    if (pCallbackEntry)
				{
				    // get current position of sample
				    offsetStart = (pVoice->NoteWave >> STEP_BIT_RANGE);
				    // get size of 1 time slice for this sample (11ms) and compute end point
				    offsetEnd = offsetStart + pVoice->NoteNextSize;
				    while (pCallbackEntry)
					{
					    if ((pCallbackEntry->frameOffset >= offsetStart) && (pCallbackEntry->frameOffset <= offsetEnd) )
						{
						    if (pCallbackEntry->pCallback)
							{
							    (*pCallbackEntry->pCallback)(threadContext, pCallbackEntry->reference, pVoice->NoteWave >> STEP_BIT_RANGE);
							    break;
							}
						}
					    pCallbackEntry = pCallbackEntry->pNext;
					}
				}
			}
		}
	}
}
#endif

// given a VOICE_REFERENCE returned from GM_Begin... this will return TRUE, if voice is
// valid
XBOOL GM_IsSoundReferenceValid(VOICE_REFERENCE reference)
{
    XBOOL	goodVoice;

    goodVoice = FALSE;
    if (MusicGlobals)
	{
	    if ( ((INT32)reference >= 0) && ((INT32)reference < (MusicGlobals->MaxNotes+MusicGlobals->MaxEffects)) )
		{
		    goodVoice = TRUE;
		}
	}
    return goodVoice;
}

/* $$fb 2003-03-14: add this function. Related to fix for 4828556 */
// given a GM_Voice, this will return the VOICE_REFERENCE associated to the reference in question
VOICE_REFERENCE PV_GetSoundReferenceFromVoice(GM_Voice* pVoice) {
    VOICE_REFERENCE ref;

    if (!pVoice) {
	ref = DEAD_VOICE;
    } else {
	ref = (VOICE_REFERENCE) (((UINT_PTR)pVoice) - ((UINT_PTR) &MusicGlobals->NoteEntry[0])) / sizeof(GM_Voice);
	if (!GM_IsSoundReferenceValid(ref)) {
	    ref = DEAD_VOICE;
	}
    }
    //printf("PV_GetSoundReferenceFromVoice(%x)=%d\n", (int) pVoice, ref); fflush(stdout);
    return ref;
}

// given a VOICE_REFERENCE, this will return the GM_Voice associated to the reference in question
GM_Voice * PV_GetVoiceFromSoundReference(VOICE_REFERENCE reference)
{
    GM_Voice	*pVoice;

    pVoice = NULL;
    if (GM_IsSoundReferenceValid(reference))
	{
	    pVoice = &MusicGlobals->NoteEntry[(INT32)reference];
	    // is voice alive?
	    if (pVoice->voiceMode == VOICE_UNUSED)
		{
		    pVoice = NULL;
		}
	}
    return pVoice;
}

XBOOL GM_IsSoundDone(VOICE_REFERENCE reference)
{
    register INT32 count;

    if (GM_IsSoundReferenceValid(reference))
	{
	    for (count = 0; count < MusicGlobals->MaxNotes+MusicGlobals->MaxEffects; count++)
		{
		    if (MusicGlobals->NoteEntry[count].voiceMode != VOICE_UNUSED)
			{
			    if (count == (INT32)reference)
				{
				    return FALSE;
				}
			}
		}
	}
    return TRUE;
}


// setup a double buffer sound effect
// stereoPosition is in the range 63 to -63
VOICE_REFERENCE GM_SetupSampleDoubleBuffer(XPTR pBuffer1, XPTR pBuffer2, UINT32 theSize, XFIXED theRate,
					   INT16 bitSize, INT16 channels,
					   INT32 sampleVolume, INT16 stereoPosition,
					   void *context,
					   GM_DoubleBufferCallbackPtr bufferCallback,
					   GM_SoundDoneCallbackPtr doneCallbackProc)
{
    register GM_Mixer	*pMixer;
    register GM_Voice	*pVoice;
    register INT16		count, max, min;

    pMixer = MusicGlobals;
    if (pMixer && (pMixer->MaxEffects > 0) && (theSize < MAX_SAMPLE_FRAMES))
	{
	    min = pMixer->MaxNotes;				// only pick a new voice within this range
	    max =  min + pMixer->MaxEffects;
	    for (count = min; count < max; count++)
		{
		    pVoice = &pMixer->NoteEntry[count];
		    if (pVoice->voiceMode == VOICE_UNUSED)
			{
			    pVoice->voiceMode = VOICE_ALLOCATED;		// allocate voice so no one else can grab it.
			    PV_CleanNoteEntry(pVoice);					// fill with all zero's except voiceMode field.
			    pVoice->noteSamplePitchAdjust = XFIXED_1;	// 1.0
			    pVoice->doubleBufferProc = bufferCallback;
			    pVoice->NotePtr = (UBYTE *) pBuffer1;
			    pVoice->NotePtrEnd = (UBYTE *) pBuffer1 + theSize;

			    pVoice->doubleBufferPtr1 = (UBYTE *) pBuffer1;
			    pVoice->doubleBufferPtr2 = (UBYTE *) pBuffer2;

			    pVoice->NoteLoopPtr = pVoice->NotePtr;
			    pVoice->NoteLoopEnd = pVoice->NotePtrEnd;

			    pVoice->NotePitch = (XFIXED)theRate / 22050;
			    pVoice->NoteLoopProc = NULL;

			    pVoice->NoteEndCallback = doneCallbackProc;
			    pVoice->NoteProgram = -1;
			    pVoice->stereoPosition = stereoPosition;
			    pVoice->bitSize = (UBYTE)bitSize;
			    pVoice->channels = (UBYTE)channels;
			    pVoice->avoidReverb = TRUE;
			    pVoice->soundFadeRate = 0;

			    pVoice->NoteMIDIVolume = (INT16)sampleVolume;	// save unscaled
			    sampleVolume = (sampleVolume * pMixer->effectsVolume) / MAX_MASTER_VOLUME;
			    sampleVolume = (sampleVolume * pMixer->MasterVolume) / MAX_MASTER_VOLUME;

			    pVoice->NoteVolume = sampleVolume;
			    pVoice->NoteVolumeEnvelope = VOLUME_RANGE;
			    pVoice->volumeADSRRecord.ADSRLevel[0] = VOLUME_RANGE;
			    pVoice->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
			    pVoice->volumeADSRRecord.currentLevel = VOLUME_RANGE;
			    pVoice->volumeADSRRecord.currentPosition = 0;
			    pVoice->volumeADSRRecord.sustainingDecayLevel = XFIXED_1;

			    pVoice->NoteChannel = SOUND_EFFECT_CHANNEL;
			    pVoice->NoteDecay = 0x7FFF;		// never release
			    pVoice->NoteContext = context;
			    pVoice->sustainMode = SUS_NORMAL;
			    // is there an initial volume level in the ADSR record that starts at time=0?  If so, don't interpolate the
			    // note's volume up from 0 to the first target level.  Otherwise, it's a traditional ramp-up from 0.
			    if (pMixer->generateStereoOutput)
				{
				    PV_CalculateStereoVolume(pVoice, &pVoice->lastAmplitudeL, &pVoice->lastAmplitudeR);
				}
			    else
				{
				    pVoice->lastAmplitudeL = (pVoice->NoteVolume * pVoice->volumeADSRRecord.ADSRLevel[0]) >> VOLUME_PRECISION_SCALAR;
				}
				//pVoice->voiceStartTimeStamp = XMicroseconds();
				//pVoice->voiceMode = VOICE_SUSTAINING;
			    return (VOICE_REFERENCE)count;
			}
		}
	}
    return DEAD_VOICE;
}

// stereoPosition is in the range 63 to -63
VOICE_REFERENCE GM_SetupSample(XPTR theData, UINT32 frames, XFIXED theRate,
			       UINT32 theStartLoop, UINT32 theEndLoop, UINT32 theLoopTarget,
			       INT32 sampleVolume, INT32 stereoPosition,
			       void *context, INT16 bitSize, INT16 channels,
			       GM_LoopDoneCallbackPtr theLoopContinueProc,
			       GM_SoundDoneCallbackPtr theCallbackProc)
{
    register GM_Mixer	*pMixer;
    register GM_Voice	*pVoice;
    register INT16		count, max, min;

    pMixer = MusicGlobals;
    if (pMixer && (pMixer->MaxEffects > 0) && (frames < MAX_SAMPLE_FRAMES))
	{
	    min = pMixer->MaxNotes;				// only pick a new voice within this range
	    max =  min + pMixer->MaxEffects;
	    for (count = min; count < max; count++)
		{
		    pVoice = &pMixer->NoteEntry[count];
		    if (pVoice->voiceMode == VOICE_UNUSED)
			{
			    pVoice->voiceMode = VOICE_ALLOCATED;	// allocate voice so no one else can grab it.
			    PV_CleanNoteEntry(pVoice);				// zeroes ALL entries, except voiceMode
			    pVoice->noteSamplePitchAdjust = XFIXED_1;

			    pVoice->NotePtr = (UBYTE *) theData;
			    pVoice->NotePtrEnd = (UBYTE *) theData + frames;
			    pVoice->NotePitch = (XFIXED)theRate / 22050;

				// reset the NoteLoopCount
			    pVoice->NoteLoopCount = 0;

			    pVoice->NoteLoopProc = theLoopContinueProc;
			    if ( (theStartLoop < theEndLoop) && (theEndLoop - theStartLoop > MIN_LOOP_SIZE) )
				{
				    pVoice->NoteLoopPtr = (UBYTE *)theData + theStartLoop;
				    pVoice->NoteLoopEnd = (UBYTE *)theData + theEndLoop;

				    // set the target number of loops
				    pVoice->NoteLoopTarget = theLoopTarget;
				}
			    pVoice->NoteEndCallback = theCallbackProc;
			    pVoice->NoteProgram = -1;
			    pVoice->stereoPosition = (INT16)stereoPosition;
			    pVoice->bitSize = (UBYTE)bitSize;
			    pVoice->channels = (UBYTE)channels;
			    pVoice->avoidReverb = TRUE;
			    pVoice->LFORecordCount = 0;
			    pVoice->pInstrument = NULL;
			    pVoice->soundFadeRate = 0;

			    pVoice->NoteMIDIVolume = (INT16)sampleVolume;	// save unscaled
			    sampleVolume = (sampleVolume * pMixer->effectsVolume) / MAX_MASTER_VOLUME;
			    sampleVolume = (sampleVolume * pMixer->MasterVolume) / MAX_MASTER_VOLUME;

			    pVoice->NoteVolume = sampleVolume;
			    pVoice->NoteVolumeEnvelope = VOLUME_RANGE;
			    pVoice->volumeADSRRecord.ADSRLevel[0] = VOLUME_RANGE;
			    pVoice->volumeADSRRecord.currentLevel = VOLUME_RANGE;
			    pVoice->volumeADSRRecord.currentPosition = 0;
			    pVoice->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
			    pVoice->volumeADSRRecord.mode = ADSR_TERMINATE;
			    pVoice->volumeADSRRecord.sustainingDecayLevel = XFIXED_1;
			    pVoice->NoteChannel = SOUND_EFFECT_CHANNEL;
			    pVoice->NoteContext = context;
			    pVoice->sustainMode = SUS_NORMAL;
			    pVoice->sampleAndHold = 1;
			    //				pVoice->NoteDecay = 32767;		// never release
			    pVoice->NoteDecay = 8;
			    // is there an initial volume level in the ADSR record that starts at time=0?  If so, don't interpolate the
			    // note's volume up from 0 to the first target level.  Otherwise, it's a traditional ramp-up from 0.
			    if (pMixer->generateStereoOutput)
				{
				    PV_CalculateStereoVolume(pVoice, &pVoice->lastAmplitudeL, &pVoice->lastAmplitudeR);
				}
			    else
				{
				    pVoice->lastAmplitudeL = (pVoice->NoteVolume * pVoice->volumeADSRRecord.ADSRLevel[0]) >> VOLUME_PRECISION_SCALAR;
				}
				//printf("Return voice %d\n", count);
				//pVoice->voiceStartTimeStamp = XMicroseconds();
				//pVoice->voiceMode = VOICE_SUSTAINING;
			    return (VOICE_REFERENCE)count;
			}
		}
	} else {
	    //printf("\n FAILURE! pMixer=%p,  pMixer->MaxEffects=%d ,  frames=%d ,  MAX_SAMPLE_FRAMES=%d\n", pMixer,  (pMixer?pMixer->MaxEffects:-1234), (int) frames, MAX_SAMPLE_FRAMES);
	}
    //printf("\nGenSample.c: Could not find a free voice!\n");	
    return DEAD_VOICE;
}

#if X_PLATFORM != X_WEBTV
// stereoPosition is in the range 63 to -63
VOICE_REFERENCE GM_SetupSampleFromInfo(GM_Waveform *pSample, void *context,
				       INT32 sampleVolume, INT32 stereoPosition,
				       GM_LoopDoneCallbackPtr theLoopContinueProc,
				       GM_SoundDoneCallbackPtr theCallbackProc,
				       UINT32 startOffsetFrame)
{
    UINT32	bytes;

    if (pSample)
	{
	    if (startOffsetFrame > pSample->waveFrames)
		{
		    startOffsetFrame = 0;
		}
	    bytes = startOffsetFrame * pSample->channels * (pSample->bitSize / 8);
	    return GM_SetupSample((XPTR)(((UBYTE *)pSample->theWaveform) + bytes),
				  pSample->waveFrames - startOffsetFrame,
				  pSample->sampledRate,
				  pSample->startLoop,
				  pSample->endLoop,
				  pSample->numLoops,
				  sampleVolume, stereoPosition, context,
				  pSample->bitSize,
				  pSample->channels,
				  theLoopContinueProc, theCallbackProc);
	}
    //printf("GenSample.c: pSample=NULL!\n");
    return DEAD_VOICE;
}
#endif

// set all the voices you want to start at the same time the same syncReference. Then call GM_SyncStartSample
// to start the sync start. Will return an error if its an invalid reference, or syncReference is NULL.
OPErr GM_SetSyncSampleStartReference(VOICE_REFERENCE reference, void *syncReference)
{
    GM_Voice	*pVoice;
    OPErr		err;

    err = NO_ERR;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if (syncReference)
		{
		    pVoice->syncVoiceReference = syncReference;
		}
	    else
		{
		    err = PARAM_ERR;
		}
	}
    else
	{
	    err = NOT_SETUP;
	}
    return err;
}

// Once you have called GM_SetSyncSampleStartReference on all the voices, this will set them to start at the next
// mixer slice. Will return an error if its an invalid reference, or syncReference is NULL.
OPErr GM_SyncStartSample(VOICE_REFERENCE reference)
{
    GM_Voice	*pVoice;
    OPErr		err;

    err = NO_ERR;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if (pVoice->syncVoiceReference)
		{
		    pVoice->voiceMode = VOICE_ALLOCATED_READY_TO_SYNC_START;
		}
	    else
		{
		    err = PARAM_ERR;
		}
	}
    else
	{
	    err = NOT_SETUP;
	}
    return err;
}

// after voice is setup, call this to start it playing now. returns 0 if started
OPErr GM_StartSample(VOICE_REFERENCE reference)
{
    GM_Voice	*pVoice;
    OPErr		err;

    err = NOT_SETUP;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->voiceStartTimeStamp = XMicroseconds();
	    pVoice->voiceMode = VOICE_SUSTAINING;
	    err = NO_ERR;
	}
    return err;
}

void GM_EndSample(VOICE_REFERENCE reference, void *threadContext)
{
    GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    PV_DoCallBack(pVoice, threadContext);
	    pVoice->voiceMode = VOICE_UNUSED;
	}
}

// Stop just sound effects

void GM_EndAllSamples(void *threadContext)
{
    register LOOPCOUNT count;
    register GM_Voice *pVoice;

    if (MusicGlobals)
	{
	    for (count = MusicGlobals->MaxNotes; count < MusicGlobals->MaxNotes + MusicGlobals->MaxEffects; count++)
		{
		    pVoice = &MusicGlobals->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    PV_DoCallBack(pVoice, threadContext);
			    pVoice->voiceMode = VOICE_UNUSED;
			}
		}
	}
}


/*
 * $$fb 2002-04-19: add this function
 * Stop the sample by marking its state as "VOICE_RELEASING".
 * This is a thread-safe version of GM_EndSample, because the actual
 * release and callback is done in the main engine thread (i.e.
 * the callback cannot happen while BuildMixerSlice is still accessing
 * the sound data)
 */
void GM_ReleaseSample(VOICE_REFERENCE reference, void *threadContext) {
    GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice) {
	pVoice->voiceMode = VOICE_RELEASING;
	pVoice->NoteDecay = 0; // end it immediately
	pVoice->volumeADSRRecord.mode = ADSR_TERMINATE;
	pVoice->volumeADSRRecord.currentPosition = 0;
	pVoice->volumeADSRRecord.ADSRLevel[0] = 0;
	pVoice->volumeADSRRecord.ADSRTime[0] = 1;
	pVoice->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
	pVoice->NoteVolumeEnvelopeBeforeLFO = 0;		// so these notes can be reused
    }
}

/*
 * $$fb 2002-04-19: add this function
 * Stop all samples by marking their state as "VOICE_RELEASING".
 * This is a thread-safe version of GM_EndAllSamples, because the actual
 * releases and callbacks are done in the main engine thread (i.e.
 * the callback cannot happen while BuildMixerSlice is still accessing
 * the sound data)
 */
void GM_ReleaseAllSamples(void *threadContext) {
    register LOOPCOUNT count;
    register GM_Voice *pVoice;

    if (MusicGlobals) {
	for (count = MusicGlobals->MaxNotes; count < MusicGlobals->MaxNotes + MusicGlobals->MaxEffects; count++) {
	    pVoice = &MusicGlobals->NoteEntry[count];
	    if (pVoice->voiceMode != VOICE_UNUSED) {
		pVoice->voiceMode = VOICE_RELEASING;
		pVoice->NoteDecay = 0; // end immediately
		pVoice->volumeADSRRecord.mode = ADSR_TERMINATE;
		pVoice->volumeADSRRecord.currentPosition = 0;
		pVoice->volumeADSRRecord.ADSRLevel[0] = 0;
		pVoice->volumeADSRRecord.ADSRTime[0] = 1;
		pVoice->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
		pVoice->NoteVolumeEnvelopeBeforeLFO = 0;		// so these notes can be reused
	    }
	}
    }
}


#if X_PLATFORM != X_WEBTV

UINT32 GM_GetSampleStartTimeStamp(VOICE_REFERENCE reference)
{
    GM_Voice		*pVoice;
    UINT32	time;

    time = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    time = pVoice->voiceStartTimeStamp;
	}
    return time;
}

UINT32 GM_GetSamplePlaybackPosition(VOICE_REFERENCE reference)
{
    UINT32	position;
    GM_Voice		*pVoice;

    position = 0L;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    // get current position of sample
	    position = pVoice->NoteWave >> STEP_BIT_RANGE;
	}
    return position;
}

void * GM_GetSamplePlaybackPointer(VOICE_REFERENCE reference)
{
    void			*pointer;
    GM_Voice		*pVoice;

    pointer = NULL;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    // get current pointer
	    pointer = (void *)pVoice->NotePtr;
	}
    return pointer;
}
#endif

#if X_PLATFORM != X_WEBTV
void GM_SetSampleDoneCallback(VOICE_REFERENCE reference, GM_SoundDoneCallbackPtr theCallbackProc, void *context)
{
    GM_Voice		*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->NoteContext = context;
	    pVoice->NoteEndCallback = theCallbackProc;
	}
}
#endif

#if X_PLATFORM != X_WEBTV
XFIXED GM_GetSamplePitch(VOICE_REFERENCE reference)
{
    XFIXED		rate;
    GM_Voice	*pVoice;

    rate = LONG_TO_XFIXED(22050L);
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    rate = pVoice->NotePitch * 22050L;
	}
    return rate;
}

void GM_ChangeSamplePitch(VOICE_REFERENCE reference, XFIXED theNewRate)
{
    GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->NotePitch = theNewRate / 22050L;
	}
}

// amount range is -255 to 255
void GM_SetSampleLowPassAmountFilter(VOICE_REFERENCE reference, short int amount)
{
    GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->LPF_lowpassAmount = (INT32)amount;
	    pVoice->LPF_base_lowpassAmount = (INT32)amount;
	}
}

// amount range is -255 to 255
short int GM_GetSampleLowPassAmountFilter(VOICE_REFERENCE reference)
{
    GM_Voice	*pVoice;
    short int	amount;

    amount = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    amount = (short int)pVoice->LPF_lowpassAmount;
	}
    return amount;
}

// resonance range is 0 to 256
void GM_SetSampleResonanceFilter(VOICE_REFERENCE reference, short int resonance)
{
    GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->LPF_resonance = (INT32)resonance;
	    pVoice->LPF_base_resonance = (INT32)resonance;
	}
}

// resonance range is 0 to 256
short int GM_GetSampleResonanceFilter(VOICE_REFERENCE reference)
{
    GM_Voice	*pVoice;
    short int		resonance;

    resonance = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    resonance = (short int)pVoice->LPF_resonance;
	}
    return resonance;
}

// frequency range is 512 to 32512
void GM_SetSampleFrequencyFilter(VOICE_REFERENCE reference, short int frequency)
{
    GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->LPF_frequency = (INT32)frequency;
	    pVoice->LPF_base_frequency = (INT32)frequency;
	}
}

// frequency range is 512 to 32512
short int GM_GetSampleFrequencyFilter(VOICE_REFERENCE reference)
{
    GM_Voice	*pVoice;
    short int	frequency;

    frequency = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    frequency = (short int)pVoice->LPF_frequency;
	}
    return frequency;
}


#endif


#if X_PLATFORM != X_WEBTV
// return the current amount of reverb mix. 0-127 is the range.
short int GM_GetSampleReverbAmount(VOICE_REFERENCE reference)
{
    short int	amount;
    GM_Voice	*pVoice;

    amount = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    amount = pVoice->reverbLevel;
	}
    return amount;
}

// set amount of reverb to mix. 0-127 is the range.
void GM_SetSampleReverbAmount(VOICE_REFERENCE reference, short int amount)
{
    GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if (reference)
		{
		    GM_ChangeSampleReverb(reference, TRUE);		// force on
		    pVoice->reverbLevel = (UBYTE) amount;
		}
	    else
		{	// off
		    GM_ChangeSampleReverb(reference, FALSE);	// force off
		}
	}
}

// Get current status of reverb. On or off
XBOOL GM_GetSampleReverb(VOICE_REFERENCE reference)
{
    XBOOL 		enable;
    GM_Voice	*pVoice;

    enable = FALSE;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if (pVoice->avoidReverb == FALSE)
		{
		    enable = TRUE;
		}
	}
    return enable;
}

// change status of reverb. Force on, or off
void GM_ChangeSampleReverb(VOICE_REFERENCE reference, XBOOL enable)
{
    GM_Voice		*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if (enable)
		{	// verb on
		    pVoice->avoidReverb = FALSE;
		    pVoice->reverbLevel = GM_GetReverbEnableThreshold() + 25;
		}
	    else
		{	// verb off
		    pVoice->avoidReverb = TRUE;
		    pVoice->reverbLevel = 0;
		}
	}
}
#endif

INT16 GM_GetSampleVolume(VOICE_REFERENCE reference)
{
    INT16		volume;
    GM_Voice	*pVoice;

    volume = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if (pVoice->voiceMode != VOICE_UNUSED)
		{
		    volume = (pVoice->NoteVolume * MAX_MASTER_VOLUME) / MusicGlobals->effectsVolume;
		    volume = (volume * MAX_MASTER_VOLUME) / MusicGlobals->MasterVolume;
		}
	}
    return volume;
}

// Return volume from a sample that is not scaled
INT16 GM_GetSampleVolumeUnscaled(VOICE_REFERENCE reference)
{
    INT16		volume;
    GM_Voice	*pVoice;

    volume = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if (pVoice->voiceMode != VOICE_UNUSED)
		{
		    volume = pVoice->NoteMIDIVolume;
		}
	}
    return volume;
}

// Volume range is from 0 to MAX_NOTE_VOLUME
void GM_ChangeSampleVolume(VOICE_REFERENCE reference, INT16 sampleVolume)
{
    register GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->NoteMIDIVolume = sampleVolume;	// save unscaled
	    sampleVolume = (sampleVolume * MusicGlobals->effectsVolume) / MAX_MASTER_VOLUME;
	    sampleVolume = (sampleVolume * MusicGlobals->MasterVolume) / MAX_MASTER_VOLUME;
	    pVoice->NoteVolume = sampleVolume;
	}
}

#if X_PLATFORM != X_WEBTV
void GM_SetSampleLoopPoints(VOICE_REFERENCE reference, UINT32 start, UINT32 end)
{
    register GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    if ( (start < end) && (end - start > MIN_LOOP_SIZE) )
		{
		    pVoice->NoteLoopPtr = (UBYTE *)pVoice->NotePtr + start;
		    pVoice->NoteLoopEnd = (UBYTE *)pVoice->NotePtr + end;
		}
	}
}
#endif

// Set sample fade rate. Its a 16.16 fixed value
// Input:	reference		sound to affect
//			fadeRate	amount to change every 11 ms
//						example:	FLOAT_TO_XFIXED(2.2) will decrease volume
//									FLOAT_TO_XFIXED(2.2) * -1 will increase volume
//			minVolume	lowest volume level fade will go
//			maxVolume	highest volume level fade will go
#if X_PLATFORM != X_WEBTV
void GM_SetSampleFadeRate(VOICE_REFERENCE reference, XFIXED fadeRate,
			  INT16 minVolume, INT16 maxVolume, XBOOL endSample)
{
    register GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->soundFadeMaxVolume = maxVolume * 4;
	    pVoice->soundFadeMinVolume = minVolume * 4;
	    pVoice->soundFixedVolume = LONG_TO_XFIXED(pVoice->NoteVolume);
	    pVoice->soundEndAtFade = endSample;
	    pVoice->soundFadeRate = fadeRate;
	}
}
#endif

#if X_PLATFORM != X_WEBTV
// range from -63 to 63
INT16 GM_GetSampleStereoPosition(VOICE_REFERENCE reference)
{
    INT16		pos;
    GM_Voice	*pVoice;

    pos = 0;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pos = pVoice->stereoPosition;
	}
    return pos;
}

// range from -63 to 63
void GM_ChangeSampleStereoPosition(VOICE_REFERENCE reference, INT16 newStereoPosition)
{
    register GM_Voice	*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice)
	{
	    pVoice->stereoPosition = newStereoPosition;
	}
}
#endif

#if X_PLATFORM != X_WEBTV
void GM_SetSampleOffsetCallbackLinks(VOICE_REFERENCE reference, GM_SampleCallbackEntry *pTopEntry)
{
    GM_Voice				*pVoice;

    if (pTopEntry)
	{
	    pVoice = PV_GetVoiceFromSoundReference(reference);
	    if (pVoice)
		{
		    pVoice->pSampleMarkList = pTopEntry;
		}
	}
}

// Get current status of resampled interpolation. On or off
XBOOL GM_GetSampleResample(VOICE_REFERENCE reference)
{
    XBOOL 		enable;
    GM_Voice	*pVoice;

    enable = FALSE;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    if (pVoice) {
	if (pVoice->resampleParams) {
	    enable = TRUE;
	}
    }
    return enable;
}

// change activation of resampler. Force on, or off
void GM_SetSampleResampleFromVoice(void* voice, XBOOL enable) {
    int			inSampleRate;
    SR_ResampleParams	*params;
    GM_Voice* 		pVoice = voice;
	
    if (pVoice) {
	if (enable && (pVoice->resampleParams == NULL)) {
	    /* resample on */
	    /* use kara's rounding algorithm -> see MixerClip.c */
	    inSampleRate = (int)(XFIXED_TO_UNSIGNED_LONG((pVoice->NotePitch * 22050L) + XFIXED_1 / 2));
#ifdef DEBUG
	    fprintf(stderr, "GenSample:SetSampleResample Creating resampleParams\n"); fflush(stderr);
#endif
	    params = (SR_ResampleParams*) XNewPtr((INT32)sizeof(SR_ResampleParams));
	    if (SR_init(params, 
			inSampleRate,				// input_samplerate
			GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality),		// output_samplerate
			MusicGlobals->generateStereoOutput?2:1,	// (output) channels
			MusicGlobals->generate16output?16:8	// (output) sample_size_in_bits
			)) {
		pVoice->resampleParams = params;
		pVoice->disposeResampleParams = TRUE;
	    } else if (params) {
		XDisposePtr((XPTR)params);
	    }
	} else {	
	    /* resample off */
	    params = pVoice->resampleParams;
	    if (params != NULL) {
#ifdef DEBUG
		fprintf(stderr, "GenSample:SetSampleResampleFromVoice setting pVoice->resampleParams to NULL. pVoice->resampleParams=%p, pVoice=%p\n", 
			pVoice->resampleParams, pVoice); fflush(stderr);
#endif
		pVoice->resampleParams = NULL;
		if (pVoice->disposeResampleParams) {
		    SR_exit(params);
#ifdef DEBUG
		    fprintf(stderr, "GenSample:SetSampleResampleFromVoice: Disposing resampleParams\n"); fflush(stderr);
#endif
		    XDisposePtr((XPTR)params);
		    pVoice->disposeResampleParams = FALSE;
		}
	    }
	}
    }
}

void GM_SetSampleResample(VOICE_REFERENCE reference, XBOOL enable) {
    GM_Voice		*pVoice;

    pVoice = PV_GetVoiceFromSoundReference(reference);
    // hack: pVoice == NULL happens if a) reference is invalid, b) the voice is currently not used (VOICE_UNUSED)
    // we want to remove a resampler reference, even if the voice is already set to UNUSED
    if (!enable && (pVoice == NULL) && GM_IsSoundReferenceValid(reference)) {
	// copied from PV_GetVoiceFromSoundReference
	pVoice = &MusicGlobals->NoteEntry[(INT32)reference];
#ifdef DEBUG
	fprintf(stderr, "GenSample:SetSampleResample: needed hack to get pVoice=%p for voice = %d\n", pVoice, (int) reference); fflush(stderr);
#endif
    }
    GM_SetSampleResampleFromVoice(pVoice, enable);
}

void GM_SetSampleResampleExtern(VOICE_REFERENCE reference, void* resampleParams) {
    GM_Voice		*pVoice;
    SR_ResampleParams	*params;
    int			inSampleRate;
	
    // setting params to NULL is equivalent to removing it. Benefit of special handling in RemoveSampleResampleExtern
    if (!resampleParams) {
	GM_RemoveSampleResampleExtern(reference, NULL);
	return;
    }

    params = (SR_ResampleParams*) resampleParams;
    pVoice = PV_GetVoiceFromSoundReference(reference);

    // only proceed if something really changes
    if (pVoice && (pVoice->resampleParams!=params || pVoice->disposeResampleParams)) {
	// if necessary, get rid of previously set resampleParams
	GM_SetSampleResampleFromVoice(pVoice, FALSE);
	
#ifdef DEBUG
	fprintf(stderr, "GenSample:SetSampleResampleExtern: setting pVoice->resampleParams to %p. pVoice=%p\n", params, pVoice); fflush(stderr);
#endif
	pVoice->resampleParams = params;
	pVoice->disposeResampleParams = FALSE;
	if (params != NULL) {
	    inSampleRate = (int)(XFIXED_TO_UNSIGNED_LONG((pVoice->NotePitch * 22050L) + XFIXED_1 / 2));
	    SR_change_samplerate(params, inSampleRate, params->output_samplerate);
	}
    }
}

void GM_RemoveSampleResampleExtern(VOICE_REFERENCE reference, void* oldResampleParams) {
    GM_Voice		*pVoice;
    SR_ResampleParams	*params;
	
    params = (SR_ResampleParams*) oldResampleParams;
    pVoice = PV_GetVoiceFromSoundReference(reference);
    // hack: pVoice == NULL happens if a) reference is invalid, b) the voice is currently not used (VOICE_UNUSED)
    // we want to remove a resampler reference, even if the voice is already set to UNUSED
    if ((pVoice == NULL) && GM_IsSoundReferenceValid(reference)) {
	// copied from PV_GetVoiceFromSoundReference
	pVoice = &MusicGlobals->NoteEntry[(INT32)reference];
#ifdef DEBUG
	fprintf(stderr, "GenSample:RemoveSampleResampleExtern: needed hack to get pVoice=%p for voice = %d\n", pVoice, (int) reference); fflush(stderr);
#endif
    }
#ifdef DEBUG
    if (!GM_IsSoundReferenceValid(reference)) {
	fprintf(stderr, "GenSample:RemoveSampleResampleExtern: reference %d is not valid!\n", (int) reference); fflush(stderr);
    } 
    else if (pVoice!=NULL && pVoice->voiceMode == VOICE_UNUSED) {
	fprintf(stderr, "GenSample:RemoveSampleResampleExtern: Voice %d UNUSED!\n", (int) reference); fflush(stderr);
    }
#endif
    if (pVoice && (!oldResampleParams || pVoice->resampleParams == params)) {
#ifdef DEBUG
	fprintf(stderr, "GenSample:RemoveSampleResampleExtern: setting pVoice->resampleParams from %p to NULL. voice=%d  pVoice=%p\n", 
		pVoice->resampleParams, (int) reference, pVoice); fflush(stderr);
#endif
	// if necessary, get rid of previously set resampleParams
	GM_SetSampleResampleFromVoice(pVoice, FALSE);
	
	pVoice->resampleParams = NULL;
	pVoice->disposeResampleParams = FALSE;
    }
#ifdef DEBUG
    else {
	fprintf(stderr, "GenSample:RemoveSampleResampleExtern: Could not remove external reference.\n");
	fprintf(stderr, "     oldResampleParams=%p  voice=%d  pVoice=%p\n", oldResampleParams, (int) reference, pVoice); 
	if (!pVoice) {
	    fprintf(stderr, "           voice is not valid\n");
	} 
	else if (oldResampleParams) {
	    if (!pVoice->resampleParams) {
		fprintf(stderr, "           the voice's resampleParams were already set to NULL\n");
	    } 
	    else if (pVoice->resampleParams != oldResampleParams) {
		fprintf(stderr, "           the voice's resampleParams are already set to a different reference: pVoice->resampleParams=%p\n", pVoice->resampleParams);
	    }
	} else {
	    fprintf(stderr, "           oldResampleParams are NULL. pVoice->resampleParams=%p\n", pVoice->resampleParams);
	}
	fflush(stderr);
    }
#endif
}



// Frees memory associated with GM_Waveform structure and the structure itself. Don't
// call this on a GM_Waveform that has been allocated on the stack.
void GM_FreeWaveform(GM_Waveform *pWaveform)
{
    if (pWaveform)
	{
	    XDisposePtr((XPTR)pWaveform->theWaveform);
	    XDisposePtr((XPTR)pWaveform);
	}
}

void GM_AddSampleOffsetCallback(VOICE_REFERENCE reference, GM_SampleCallbackEntry *pEntry)
{
    GM_Voice				*pVoice;
    GM_SampleCallbackEntry	*pNext;

    if (pEntry)
	{
	    pVoice = PV_GetVoiceFromSoundReference(reference);
	    if (pVoice)
		{
		    pNext = pVoice->pSampleMarkList;
		    while (pNext)
			{
			    if (pNext->pNext == NULL)
				{
				    break;
				}
			    else
				{
				    pNext = pNext->pNext;
				}
			}
		    if (pNext == NULL)
			{
			    pVoice->pSampleMarkList = pEntry;
			}
		    else
			{
			    pNext->pNext = pEntry;
			}
		}
	}
}

void GM_RemoveSampleOffsetCallback(VOICE_REFERENCE reference, GM_SampleCallbackEntry *pEntry)
{
    GM_Voice				*pVoice;
    GM_SampleCallbackEntry	*pNext, *pLast;

    if (pEntry)
	{
	    pVoice = PV_GetVoiceFromSoundReference(reference);
	    if (pVoice)
		{
		    pLast = pNext = pVoice->pSampleMarkList;
		    while (pNext)
			{
			    if (pNext == pEntry)								// found object in list?
				{
				    if (pNext == pVoice->pSampleMarkList)			// is object the top object
					{
					    pVoice->pSampleMarkList = pNext->pNext;		// yes, change to next object
					}
				    else
					{
					    if (pLast)									// no, change last to point beyond next
						{
						    pLast->pNext = pNext->pNext;
						}
					}
				    break;
				}
			    pLast = pNext;
			    pNext = pNext->pNext;
			}
		}
	}
}
#endif


#if X_PLATFORM != X_WEBTV
// group functions
// new
// free
// add GM_Waveform
// add double buffer
// remove
// start
// stop
// setvolume

// Group samples
//
// USE:
//
// linked samples
// Call one of the GM_SetupSample... functions in the various standard ways, to get an allocate voice
// then call GM_NewLinkedSampleList. Then add it to your maintained top list of linked voices with
// by calling GM_AddLinkedSample. Use GM_FreeLinkedSampleList to delete an entire list,
// or GM_RemoveLinkedSample to just one link.
//
// Then you can call GM_StartLinkedSamples to start them all at the same time, or GM_EndLinkedSamples
// to end them all, or GM_SetLinkedSampleVolume, GM_SetLinkedSampleRate, and GM_SetLinkedSamplePosition
// set parameters about them all.

// private structure of linked voices
struct GM_LinkedVoice
{
    VOICE_REFERENCE			playbackReference;
    struct GM_LinkedVoice	*pNext;
};
typedef struct GM_LinkedVoice GM_LinkedVoice;


LINKED_VOICE_REFERENCE GM_NewLinkedSampleList(VOICE_REFERENCE reference)
{
    GM_LinkedVoice	*pNew;

    pNew = NULL;
    if (GM_IsSoundReferenceValid(reference))
	{
	    pNew = (GM_LinkedVoice *)XNewPtr((INT32)sizeof(GM_LinkedVoice));
	    if (pNew)
		{
		    pNew->playbackReference	= reference;
		    pNew->pNext = NULL;
		}
	}
    return (LINKED_VOICE_REFERENCE)pNew;
}

void GM_FreeLinkedSampleList(LINKED_VOICE_REFERENCE pTop)
{
    GM_LinkedVoice	*pNext, *pLast;

    pNext = (GM_LinkedVoice *)pTop;
    while (pNext)
	{
	    pLast = pNext;
	    pNext = pNext->pNext;
	    XDisposePtr((XPTR)pLast);
	}
}

// Given a top link, and a new link this will add to a linked list, and return a new top
// if required.
LINKED_VOICE_REFERENCE GM_AddLinkedSample(LINKED_VOICE_REFERENCE pTop, LINKED_VOICE_REFERENCE pEntry)
{
    GM_LinkedVoice	*pNext;

    if (pEntry)
	{
	    pNext = (GM_LinkedVoice *)pTop;
	    while (pNext)
		{
		    if (pNext->pNext == NULL)
			{
			    break;
			}
		    else
			{
			    pNext = pNext->pNext;
			}
		}
	    if (pNext == NULL)
		{
		    pTop = pEntry;
		}
	    else
		{
		    pNext->pNext = (GM_LinkedVoice *)pEntry;
		}
	}
    return pTop;
}

// Given a top link and an link to remove this will disconnect the link from the list and
// return a new top if required.
LINKED_VOICE_REFERENCE GM_RemoveLinkedSample(LINKED_VOICE_REFERENCE pTop, LINKED_VOICE_REFERENCE pEntry)
{
    GM_LinkedVoice	*pNext, *pLast;

    if (pEntry)
	{
	    pLast = pNext = (GM_LinkedVoice *)pTop;
	    while (pNext)
		{
		    if (pNext == (GM_LinkedVoice *)pEntry)				// found object in list?
			{
			    if (pNext == (GM_LinkedVoice *)pTop)			// is object the top object
				{
				    pTop = pNext->pNext;						// yes, change to next object
				}
			    else
				{
				    if (pLast)									// no, change last to point beyond next
					{
					    pLast->pNext = pNext->pNext;
					}
				}
			    break;
			}
		    pLast = pNext;
		    pNext = pNext->pNext;
		}
	}
    return pTop;
}

VOICE_REFERENCE GM_GetLinkedSamplePlaybackReference(LINKED_VOICE_REFERENCE pLink)
{
    VOICE_REFERENCE			reference;

    reference = DEAD_VOICE;
    if (pLink)
	{
	    reference = ((GM_LinkedVoice *)pLink)->playbackReference;
	    if (GM_IsSoundReferenceValid(reference) == FALSE)
		{
		    reference = DEAD_VOICE;
		}
	}
    return reference;
}


OPErr GM_StartLinkedSamples(LINKED_VOICE_REFERENCE pTop)
{
    GM_LinkedVoice		*pNext;
    OPErr				err;

    err = NO_ERR;	// ok, until proved otherwise
    if (MusicGlobals)
	{
	    // set sync reference. Use our group because its easy and will be unique
	    pNext = (GM_LinkedVoice *)pTop;
	    while (pNext)
		{
		    err = GM_SetSyncSampleStartReference(((GM_LinkedVoice *)pNext)->playbackReference, (void *)pTop);
		    pNext = pNext->pNext;
		}
	    if (err == NO_ERR)
		{
		    // ok, now wait for mixer to be free
		    while (MusicGlobals->insideAudioInterrupt)
			{
			    XWaitMicroseocnds(HAE_GetSliceTimeInMicroseconds());
			}
		    pNext = (GM_LinkedVoice *)pTop;
		    while (pNext)
			{
			    err = GM_SyncStartSample(((GM_LinkedVoice *)pNext)->playbackReference);
			    pNext = pNext->pNext;
			}
		}
	}
    else
	{
	    err = NOT_SETUP;
	}
    return err;
}


// end in unison the samples for all the linked samples

// $$kk: 04.19.99
//void GM_EndLinkedSamples(LINKED_VOICE_REFERENCE pTop)
void GM_EndLinkedSamples(LINKED_VOICE_REFERENCE pTop, void *threadContext)
{
    GM_LinkedVoice	*pNext;

    pNext = (GM_LinkedVoice *)pTop;
    while (pNext)
	{
	    //$$fb 2002-04-20: use GM_ReleaseSample and not GM_EndSample
	    GM_ReleaseSample(pNext->playbackReference, threadContext);
	    pNext = pNext->pNext;
	}
}

// Volume range is from 0 to MAX_NOTE_VOLUME
// set in unison the sample volume for all the linked samples
void GM_SetLinkedSampleVolume(LINKED_VOICE_REFERENCE pTop, INT16 sampleVolume)
{
    GM_LinkedVoice	*pNext;

    pNext = (GM_LinkedVoice *)pTop;
    while (pNext)
	{
	    GM_ChangeSampleVolume(pNext->playbackReference, sampleVolume);
	    pNext = pNext->pNext;
	}
}

// set in unison the sample rate for all the linked samples
void GM_SetLinkedSampleRate(LINKED_VOICE_REFERENCE pTop, XFIXED theNewRate)
{
    GM_LinkedVoice	*pNext;

    pNext = (GM_LinkedVoice *)pTop;
    while (pNext)
	{
	    GM_ChangeSamplePitch(pNext->playbackReference, theNewRate);
	    pNext = pNext->pNext;
	}
}


// set in unison the sample position for all the linked samples
// range from -63 to 63
void GM_SetLinkedSamplePosition(LINKED_VOICE_REFERENCE pTop, INT16 newStereoPosition)
{
    GM_LinkedVoice	*pNext;

    pNext = (GM_LinkedVoice *)pTop;
    while (pNext)
	{
	    GM_ChangeSampleStereoPosition(pNext->playbackReference, newStereoPosition);
	    pNext = pNext->pNext;
	}
}

#endif

// EOF of GenSample.c
