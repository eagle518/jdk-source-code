/*
 * @(#)GenSetup.c	1.31 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenSetup.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Overview
**	General purpose Music Synthesis software, C-only implementation
**	(no assembly language optimizations made)
**
** Modification History:
**
**	1/10/96		Split from GenSynth.c
**	1/12/96		Changed GM_ChangeSystemVoices to support 32 voices instead of 31
**	1/18/96		Spruced up for C++ extra error checking
**				Fixed bug with GM_FinisGeneralSound referencing GM_Mixer before its allocated
**	1/19/96		Changed GM_BeginSample to support bitsize and channels
**	2/5/96		Working towards multiple songs
**	2/12/96		Moved cleaning of external midi queue to Init code
**	3/6/96		Eliminated the global songVolume
**	4/10/96		Changed min loop size from 100 to use define 'MIN_LOOP_SIZE'
**				Eliminated the 'can't use 0 as loop start' in GM_BeginSample
**	4/21/96		Removed register usage in parameters
**				Put test around ReverbBuffer being NULL
**	4/25/96		Removed SwapLong & SwapShort. Use XGetLong & XGetShort instead
**	7/4/96		Changed font and re tabbed
**	7/31/96		Fixed PV_ChangeSustainedNotes to do pedal changes as 127=on,
**				anything else as off
**	10/11/96	Added GM_BeginSampleFromInfo
**	10/23/96	Removed reference to BYTE and changed them all to UBYTE or SBYTE
**	11/3/96		Fixed a bug in GM_BeginDoubleBuffer. Changed currentPosition to
**				currentLevel
**				Changed the way the expression controller calculates extra volume,
**				now done in PV_ScaleVolumeFromChannelAndSong
**	11/8/96		Added GM_GetSampleVolume & GM_GetSamplePitch & GM_GetSampleStereoPosition
**	11/21/96	Reconfigured GM_InitGeneralSound to check parmeters in a clearer way.
**				Also prevents possible compiler problems
**	12/17/96	Fixed bug in GM_InitGeneralSound that would return an error for 32 voices
**	12/30/96	Changed copyright
**	1/12/97		Changed maxNormalizedVoices to mixLevel
**	1/22/97		Added GM_SetSampleDoneCallback
**	1/23/97		Fixed a bug with PV_ScaleVolumeFromChannelAndSong that prevent direct midi
**				control from changing the master volume
**				Changed GM_GetSampleVolume to return zero if sample reference is bad
**	1/27/97		Added fade sample engine via PV_ServeEffectsFades
**	1/28/97		Added the ability to end a sample when fade is reached
**	2/12/97		Added support for X_WIN_HARDWARE
**	2/13/97		Added GM_GetSystemVoices
**	2/20/97		Added support for TYPE 7 and 8 reverb in GM_SetReverbType
**				Fixed a bug with GM_SetMasterVolume that doesn't kill audio when
**				volume is 0
**	3/3/97		Reversed stereo placement in PV_CalculateStereoVolume
**	3/24/97		Put code in to prevent changing the voices unless they are
**				different. See GM_ChangeSystemVoices
**	4/1/97		Fixed a memory leak. Forgot to free reverb memory in GM_FinisGeneralSound
**	4/9/97		Added sampleExpansion factor
**	4/15/97		Moved sample API functions to GenSample.c
**	5/3/97		Fixed a few potential problems that the MOT compiler found
**	5/5/97		Changed PV_CalculateStereoVolume to handle the fractional volume case
**				at 1.
**	6/4/97		Renamed InitSoundManager to GM_StartHardwareSoundManager, and
**				renamed FinsSoundManager to GM_StopHardwareSoundManager, and
**				now pass in a thread context
**				Changed GM_InitGeneralSound & GM_FinisGeneralSound & GM_ChangeAudioModes &
**				GM_PauseGeneralSound & GM_ResumeGeneralSound to pass in a thread context
**				that is passed to the low level hardware API
**	6/17/97		Changed GM_SetMasterVolume to reset all songs and samples to rescale
**				current volumes
**	7/22/97		Moved GM_SetAudioTask & GM_SetAudioOutput & GM_GetAudioTask &
**				GM_GetAudioOutput from various platform specific files to GenSetup.c
**				Changed GM_SetMasterVolume to include streams calls
**	7/28/97		In GM_SetMasterVolume, wrapped a define around the stream call
**	8/15/97		Removed reference of pMixer->channelCallbackProc. Now using new
**				memory efficant controler callback
**	8/18/97		Changed X_WIN_HAE to USE_HAE_EXTERNAL_API
**	8/19/97		Seperated GM_StartHardwareSoundManager from being called inside of
**				GM_InitGeneralSound. Now it has to be called by the caller. Likewise
**				GM_StopHardwareSoundManager is no longer being called within GM_FinisGeneralSound.
**	10/12/97	Fixed SetChannelVolume & PV_ChangeSustainedNotes & SetChannelStereoPosition &
**				SetChannelModWheel & SetChannelPitchBend to check for a particular song before
**				changing playing voices. This fixed a bug in which a pitch bend from one song
**				would affect another
**	10/16/97	Removed lame instrument cache support from GM_InitGeneralSound
**	11/12/97	Added GM_MaxDevices & GM_SetDeviceID & GM_GetDeviceID & GM_GetDeviceName
**	12/4/97		Added GM_GetDeviceTimeStamp & GM_UpdateSamplesPlayed
**				Added calculation of sampleFrameSize to GM_InitGeneralSound
**				Renamed GM_Mixer to GM_Mixer
**	12/16/97	Modified GM_GetDeviceID and GM_SetDeviceID to pass a device parameter pointer
**				that is specific for that device.
**	12/16/97	Moe: removed compiler warnings
**	1/20/98		Modified SetChannelVolume to allow for zero volume levels without killing
**				note
**	2/3/98		Added GM_SetupReverb & GM_CleanupReverb
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/10/98		kcr		initialize new effect types (new reverb, chorus, etc.)
**	2/11/98		Added support for Q_48K & Q_24K & Q_8K & Q_22K_TERP_44K & Q_11K_TERP_22K
**				Fixed bug in GM_ChangeAudioModes that changed the verb mode
**				Added GM_ConvertFromOutputQualityToRate
**	2/20/98		kcr		properly dispose of reverb memory
**	2/23/98		Removed last of old variable reverb code
**	3/12/98		Renamed myMusicGlobals to pMixer. Started implementing verb config code. 
**				Redid and moved some reverb setup code into GenReverb.c
**	3/18/98		Changed GM_InitGeneralSound & GM_ChangeAudioModes to fail on an 8 bit
**				audio card if there's no support.
**	7/28/98		Modified PV_CalculateStereoVolume to handle master song pan placement
**	7/30/98		Removed some duplicated code that cleared the active note list
**	8/5/98		Put wrappers around GM_ChangeSystemVoices & GM_ChangeAudioModes to 
**				protect against MusicGlobals not being setup
**
**
**	JAVASOFT
**	03.17.98	$$kk: GM_InitGeneralSound and GM_ChangeAudioModes: left my checks for 8 and 16 bit support
**				For some sound cards, we support 16 bit but *not* 8 bit....  I also left
**				my UNSUPPORTED_HARDWARE error because I want to differentiate between this (failure
**				to acquire the device because we don't support it at all) and failure to acquire
**				the device for some random reason that may not be true later, like someone else is using it.
**	01.26.98:	$$kk: changed GM_ChangeAudioModes to only start and stop the device 
**				if it is opened
** ??			$$kk: changes to count in samples, not bytes, in GM_UpdateSamplesPlayed
**	10/6/98		Fixed bug with PV_ChangeSustainedNotes that changed a pedal down note's state
**				to the wrong state if a another pedal down event happens. Result is a stuck note.
**	11/9/98		Renamed NoteDur to voiceMode
**	12/22/98	Removed old USE_SEQUENCER flag
**	1/12/99		Added some Katmai code and dynamic flags
**	3/8/99		Renamed GM_EndSoundEffects to GM_EndAllSamples
*/
/*****************************************************************************/
#include "X_API.h"
#include "GenSnd.h"
#include "GenPriv.h"

#if USE_HAE_EXTERNAL_API
#include "HAE_API.h"
#endif


#if (X_PLATFORM == X_WINDOWS) && (USE_KAT)
#include <xmmintrin.h>
#include <excpt.h>

static XBOOL PV_IntelKatActive(void)
{
    __m128 xmm0, xmm1;
    static int gKatmaiEnabled = -1;

    if (gKatmaiEnabled == -1)
	{
	    gKatmaiEnabled = 1;

	    __try 
		{
		    // puts("Executing or_ps");
		    xmm1 = _mm_or_ps(xmm0, xmm0);
		}
	    __except (EXCEPTION_EXECUTE_HANDLER) 
		{
		    // puts("Not a Katmai System");
		    gKatmaiEnabled = 0;
		}
	}
    return (XBOOL)gKatmaiEnabled;
}
#endif

// convert GenSynth Quality to actual sample rate used
UINT32 GM_ConvertFromOutputQualityToRate(Quality quality)
{
    UINT32 sampleRate;

    sampleRate = 0;
    switch (quality)
	{
	case Q_48K:
	    sampleRate = 48000;
	    break;
	case Q_44K:
	case Q_22K_TERP_44K:
	    sampleRate = 44100;
	    break;
	case Q_24K:
	    sampleRate = 24000;
	    break;
	case Q_22K:
	case Q_11K_TERP_22K:
	    sampleRate = 22050;
	    break;
	case Q_11K:
	    sampleRate = 11025;
	    break;
	case Q_8K:
	    sampleRate = 8000;
	    break;
	}
    return sampleRate;
}

OPErr GM_PauseGeneralSound(void *threadContext)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (MusicGlobals)
	{
	    if (MusicGlobals->systemPaused == FALSE)
		{
		    GM_PauseSequencer();
			
		    //$$fb 2002-04-20: do not use GM_EndAllSamples, thread issues!
		    GM_ReleaseAllSamples(threadContext);
		    MusicGlobals->systemPaused = TRUE;
		    GM_StopHardwareSoundManager(threadContext);		// disengage from hardware
		    //$$fb 2002-04-20: now call GM_EndAllSamples to properly end all "pending" notes
		    GM_EndAllSamples(threadContext);
		}
	    else
		{
		    theErr = ALREADY_PAUSED;
		}
	}
    return theErr;
}

OPErr GM_ResumeGeneralSound(void *threadContext)
{
    OPErr	theErr;

    theErr = NO_ERR;
    if (MusicGlobals)
	{
	    if (MusicGlobals->systemPaused)
		{
		    if (GM_StartHardwareSoundManager(threadContext))			// reconnect to hardware
			{
			    MusicGlobals->systemPaused = FALSE;
			    GM_ResumeSequencer();
			}
		    else
			{
			    theErr = DEVICE_UNAVAILABLE;
			}
		}
	    else
		{
		    theErr = ALREADY_RESUMED;
		}
	}
    return theErr;
}


void GM_GetSystemVoices(INT16 *pMaxSongVoices, INT16 *pMixLevel, INT16 *pMaxEffectVoices)
{
    if (MusicGlobals && pMaxSongVoices && pMixLevel && pMaxEffectVoices)
	{
	    *pMaxSongVoices = MusicGlobals->MaxNotes;
	    *pMixLevel = MusicGlobals->mixLevel;
	    *pMaxEffectVoices = MusicGlobals->MaxEffects;
	}
}

OPErr GM_ChangeSystemVoices(INT16 maxSongVoices, INT16 mixLevel, INT16 maxEffectVoices)
{
    OPErr	theErr;
    XBOOL	change;

    theErr = NO_ERR;
    if (MusicGlobals)
	{
	    if ( (maxSongVoices >= 0) &&
		 (mixLevel > 0) &&
		 (maxEffectVoices >= 0) &&
		 ((maxEffectVoices+maxSongVoices) > 0) &&
		 ((maxEffectVoices+maxSongVoices) <= MAX_VOICES) )
		{
		    change = FALSE;
		    if (MusicGlobals->MaxNotes != maxSongVoices)
			{
			    change = TRUE;
			}
		    if (MusicGlobals->mixLevel != mixLevel)
			{
			    change = TRUE;
			}
		    if (MusicGlobals->MaxEffects != maxEffectVoices)
			{
			    change = TRUE;
			}
		    if (change)
			{
			    MusicGlobals->MaxNotes = maxSongVoices;
			    MusicGlobals->mixLevel = mixLevel;
			    MusicGlobals->MaxEffects = maxEffectVoices;

			    PV_CalcScaleBack();
			}
		}
	    else
		{
		    theErr = PARAM_ERR;
		}
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return theErr;
}

// Set the master volume, and recalculate all volumes. Scale is 0 to MAX_MASTER_VOLUME
void GM_SetMasterVolume(INT32 theVolume)
{
    if (MusicGlobals)
	{
	    MusicGlobals->MasterVolume = (INT16)theVolume;
	    PV_CalcScaleBack();

#if USE_STREAM_API
	    GM_AudioStreamSetVolumeAll(-1);	// recalculate stream volumes
#endif
	    {
		short int	count;
		GM_Song		*pSong;

		// reset volumes for sound effects
		GM_SetEffectsVolume(GM_GetEffectsVolume());

		// walk through songs and reset volumes
		for (count = 0; count < MAX_SONGS; count++)
		    {
			pSong = MusicGlobals->pSongsToPlay[count];
			if (pSong)
			    {
				GM_SetSongVolume(pSong, GM_GetSongVolume(pSong));
			    }
		    }
	    }
	}
}

INT32 GM_GetMasterVolume(void)
{
    if (MusicGlobals)
	{
	    return MusicGlobals->MasterVolume;
	}
    else
	{
	    return MAX_MASTER_VOLUME;
	}
}


OPErr GM_InitGeneralSound(void *threadContext, Quality theQuality, TerpMode theTerp, AudioModifiers theMods,
			  INT16 maxVoices, INT16 normVoices, INT16 maxEffects)
{
    register GM_Mixer	*pMixer;
    register INT32		count;
    OPErr				theErr;

    threadContext = threadContext;
    theErr = NO_ERR;
    count = maxVoices + maxEffects;
    if (count <= MAX_VOICES)
	{
	    if (normVoices > MAX_VOICES)
		{
		    if ((normVoices/100) > count)
			{
			    theErr = PARAM_ERR;
			}
		}
	    else if (normVoices > count)
		{
		    theErr = PARAM_ERR;
		}
	}
    else
	{
	    theErr = PARAM_ERR;
	}
    // Check terp mode
    switch (theTerp)
	{
	case E_AMP_SCALED_DROP_SAMPLE:
	case E_2_POINT_INTERPOLATION:
	case E_LINEAR_INTERPOLATION:
	    break;
	default:
	    theErr = PARAM_ERR;
	    break;
	}

#if USE_HAE_EXTERNAL_API
    if (theErr == NO_ERR)
	{
	    // call setup before any memory allocation happens
	    if (HAE_Setup())
		{
		    theErr = MEMORY_ERR;
		}
	}
#else
#if X_PLATFORM == X_WIN_HARDWARE
    if (theErr == NO_ERR)
	{
	    // Link to VxD. We must do this before using the memory manager
	    if (OpenVxD())
		{
		    // Pagelock all static code and data belonging to this executable
		    // Dynamically allocated data used at interrupt time must also be pagelocked
		    // Interrupt callback should be in a DLL if possible to avoid pagelocking
		    //		tons of unnecessary stuff
		    LockMe();
		}
	    else
		{
		    theErr = MEMORY_ERR;
		}
	}
#endif
#endif


    if (theErr == NO_ERR)
	{
	    // Allocate MusicGlobals
	    MusicGlobals = (GM_Mixer *)XNewPtr( (INT32)sizeof(GM_Mixer) );
	    pMixer = MusicGlobals;
	    if (MusicGlobals)
		{
#if USE_KAT
		    pMixer->useKatmaiCPU = PV_IntelKatActive();
			
		    if (pMixer->useKatmaiCPU)
			{
				// if we're on big iron run at 44 if not at 48
			    if (theQuality != Q_48K)
				{
				    theQuality = Q_44K;
				}
			}
#endif
		    // Turn off all notes!
		    for (count = 0; count < MAX_VOICES; count++)
			{
			    pMixer->NoteEntry[count].voiceMode = VOICE_UNUSED;
			}
		    pMixer->interpolationMode = theTerp;
		
		    pMixer->MasterVolume = MAX_MASTER_VOLUME;
		    pMixer->effectsVolume = MAX_MASTER_VOLUME * 3;	// samples 3 times normal volume
		    pMixer->maxChunkSize = MAX_CHUNK_SIZE;
		    pMixer->One_Slice = MAX_CHUNK_SIZE;
		    pMixer->outputQuality = theQuality;
		    switch (theQuality)
			{
#if X_PLATFORM != X_WEBTV
			case Q_48K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE+64;	// 576 in current version
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_24K:
			    pMixer->maxChunkSize = (MAX_CHUNK_SIZE+64)/2;	// 288 in current version
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
#endif
			case Q_44K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE;
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_22K_TERP_44K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE;	// 22k interpolated to 44k
			    pMixer->One_Slice = MAX_CHUNK_SIZE/2;
			    break;
#if X_PLATFORM != X_WEBTV
			case Q_22K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE/2;
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_11K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE/4;
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_11K_TERP_22K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE/2;	// 11k interpolated to 22k
			    pMixer->One_Slice = MAX_CHUNK_SIZE/4;
			    break;
			case Q_8K:
			    pMixer->maxChunkSize = (MAX_CHUNK_SIZE-416);	// (MAX_CHUNK_SIZE/4) * 8000 / 11025
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
#endif
			}
		    // set control loops
		    pMixer->One_Loop = pMixer->One_Slice;
		    pMixer->Two_Loop = pMixer->One_Slice/2;
		    pMixer->Four_Loop = pMixer->One_Slice/4;
		    pMixer->Sixteen_Loop = pMixer->One_Slice/16;
		
		    pMixer->sampleExpansion = 1;
		    // Don't generate 16 bit output, unless hardware can do it. 
		    if ( (theMods & M_USE_16) == M_USE_16)
			{
			    pMixer->generate16output = XIs16BitSupported();
			}
		    else
			{
				// $$kk: 08.12.98 merge: left my code here.
				// $$kk: 03.17.98: some solaris audio drivers do not support linear 8 bit output
				// so we need to check for mono support
			    pMixer->generate16output = !XIs8BitSupported();
			}

		    // $$kk: 03.17.98: now if whatever is set is not supported, we are hosed
		    if (pMixer->generate16output)
			{
			    if (!XIs16BitSupported())
				{
				    theErr = UNSUPPORTED_HARDWARE;
				}
			}
		    else
			{
			    if (!XIs8BitSupported())
				{
				    theErr = UNSUPPORTED_HARDWARE;
				}
			}

		    // double check users request for Stereo output. Make sure the hardware can play it
		    if ( (theMods & M_USE_STEREO) == M_USE_STEREO)
			{
			    pMixer->generateStereoOutput = XIsStereoSupported();
			}
		    else
			{
			    pMixer->generateStereoOutput = FALSE;
			}

		    pMixer->stereoFilter = ( (pMixer->generateStereoOutput) &&
					     ((theMods & M_STEREO_FILTER) == M_STEREO_FILTER) ) ? TRUE : FALSE;
		    pMixer->MaxNotes = maxVoices;
		    pMixer->mixLevel = normVoices;
		    pMixer->MaxEffects = maxEffects;
		    pMixer->reverbPtr = 0;
		    pMixer->reverbBuffer = NULL;
		    pMixer->reverbUnitType = REVERB_NO_CHANGE;
		    pMixer->reverbTypeAllocated = REVERB_NO_CHANGE;
		    pMixer->reverbBufferSize = 0;

		    if ( (theMods & M_DISABLE_REVERB) != M_DISABLE_REVERB)
			{
			    GM_SetupReverb();
			}
		    {
			ReverbMode	defaultVerb;

			defaultVerb = DEFAULT_REVERB_TYPE;		
#if USE_KAT
			if (pMixer->useKatmaiCPU)
			    {
				defaultVerb = REVERB_TYPE_10;		// default reverb for big iron
			    }
#endif
			GM_SetReverbType(defaultVerb);		// default reverb
		    }
		    GM_EndAllNotes();

		    // Compute volume multiplier for mix-level
		    PV_CalcScaleBack();

		    for (count = 0; count < MAX_SAMPLES; count++)
			{
			    pMixer->sampleCaches[count] = NULL;
			}
		    pMixer->cacheSamples = FALSE;		// don't cache samples
		    pMixer->cacheInstruments = FALSE;	// not used
		}
	    else
		{
		    theErr = MEMORY_ERR;
		}
	}
    if (theErr == NO_ERR && MusicGlobals)
	{
	    MusicGlobals->insideAudioInterrupt = 0;
	    MusicGlobals->enableDriftFixer = FALSE;
	    MusicGlobals->syncCount = XMicroseconds();
	    MusicGlobals->samplesPlayed = 0;
	    MusicGlobals->samplesWritten = 0;
	    MusicGlobals->lastSamplePosition = 0;
	    MusicGlobals->sequencerPaused = TRUE;
	    MusicGlobals->systemPaused = TRUE;
	    PV_CleanExternalQueue();

	    // calculate sample size for conversion of bytes to sample frames
	    MusicGlobals->sampleFrameSize = 1;

	    if (MusicGlobals->generate16output)
		{
		    MusicGlobals->sampleFrameSize *= 2;
		}
	    if (MusicGlobals->generateStereoOutput)
		{
		    MusicGlobals->sampleFrameSize *= 2;
		}

	    // since we don't call GM_StartHardwareSoundManager, we start the engine
	    // up paused. You'll need to call GM_ResumeGeneralSound to start the
	    // engine
	    //		MusicGlobals->sequencerPaused = FALSE;
	    //		MusicGlobals->systemPaused = FALSE;
	    //		if (GM_StartHardwareSoundManager(threadContext) == FALSE)
	    //		{
	    //			theErr = DEVICE_UNAVAILABLE;
	    //		}
	}
    return theErr;
}

// $$kk: 08.12.98 merge: changed this method
// $$kk: 01.26.98: changed this to only start and stop the device if it is opened,
#if X_PLATFORM != X_WEBTV
OPErr GM_ChangeAudioModes(void *threadContext, 
			  Quality theQuality, TerpMode theTerp, AudioModifiers theMods)
{
    register GM_Mixer	*pMixer;
    OPErr				theErr;
    ReverbMode			verb;

    // $$kk: 01.26.98: added this var reacquireDevice 
    XBOOL			reacquireDevice = FALSE;

    theErr = NO_ERR;
    pMixer = MusicGlobals;
    if (pMixer)
	{
	    // Check terp mode
	    switch (theTerp)
		{
		case E_AMP_SCALED_DROP_SAMPLE:
		case E_2_POINT_INTERPOLATION:
		case E_LINEAR_INTERPOLATION:
		    break;
		default:
		    theErr = PARAM_ERR;
		    break;
		}
	    switch (theQuality)
		{
		case Q_8K:
		case Q_11K:
		case Q_11K_TERP_22K:
		case Q_22K:
		case Q_22K_TERP_44K:
		case Q_24K:
		case Q_44K:
		case Q_48K:
		    break;
		default:
		    theErr = PARAM_ERR;
		    break;
		}
	    if (theErr == NO_ERR)
		{
		    // $$kk: 01.26.98: add this check
		    if (pMixer->systemPaused == FALSE) 
			{		
			    GM_StopHardwareSoundManager(threadContext);
			    reacquireDevice = TRUE; // reopen the device when we're done configuring
			}

		    if ( (theMods & M_USE_16) == M_USE_16)
			{
			    pMixer->generate16output = XIs16BitSupported();
			}
		    else
			{
				// $$kk: 03.17.98: some solaris audio drivers do not support linear 8 bit output
				// so we need to check for mono support
			    pMixer->generate16output = !XIs8BitSupported();
			}

		    // $$kk: 03.17.98: now if whatever is set is not supported, we are hosed
		    if (pMixer->generate16output)
			{
			    if (!XIs16BitSupported())
				{
				    theErr = UNSUPPORTED_HARDWARE;
				}
			}
		    else
			{
			    if (!XIs8BitSupported())
				{
				    theErr = UNSUPPORTED_HARDWARE;
				}
			}

		    // double check users request for Stereo output. Make sure the hardware can play it
		    if ( (theMods & M_USE_STEREO) == M_USE_STEREO)
			{
			    pMixer->generateStereoOutput = XIsStereoSupported();
			}
		    else
			{
			    pMixer->generateStereoOutput = FALSE;
			}
		    pMixer->stereoFilter = ( (pMixer->generateStereoOutput) &&
					     ((theMods & M_STEREO_FILTER) == M_STEREO_FILTER) ) ? TRUE : FALSE;
		    verb = GM_GetReverbType();	// preserve current
		    if ( (theMods & M_DISABLE_REVERB) == M_DISABLE_REVERB)
			{
				// cleanup the verb buffers
			    GM_CleanupReverb();
			}
		    else
			{
			    GM_SetupReverb();
			}
		    GM_SetReverbType(verb);		// restore verb
		    pMixer->maxChunkSize = MAX_CHUNK_SIZE;
		    pMixer->outputQuality = theQuality;
		    switch (theQuality)
			{
			case Q_48K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE+64;	// 576 in current version
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_24K:
			    pMixer->maxChunkSize = (MAX_CHUNK_SIZE+64)/2;	// 288 in current version
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_44K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE;
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_22K_TERP_44K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE;	// 22k interpolated to 44k
			    pMixer->One_Slice = MAX_CHUNK_SIZE/2;
			    break;
			case Q_22K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE/2;
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_11K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE/4;
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			case Q_11K_TERP_22K:
			    pMixer->maxChunkSize = MAX_CHUNK_SIZE/2;	// 11k interpolated to 22k
			    pMixer->One_Slice = MAX_CHUNK_SIZE/4;
			    break;
			case Q_8K:
			    pMixer->maxChunkSize = (MAX_CHUNK_SIZE-416);	// (MAX_CHUNK_SIZE/4) * 8000 / 11025
			    pMixer->One_Slice = pMixer->maxChunkSize;
			    break;
			}
		    // set control loops
		    pMixer->One_Loop = pMixer->One_Slice;
		    pMixer->Two_Loop = pMixer->One_Slice/2;
		    pMixer->Four_Loop = pMixer->One_Slice/4;
		    pMixer->Sixteen_Loop = pMixer->One_Slice/16;

		    pMixer->interpolationMode = theTerp;
		    // Recompute mix level
		    PV_CalcScaleBack();

		    // $$kk: 01.26.98: add this check
		    if (reacquireDevice == TRUE) 
			{
			    if (GM_StartHardwareSoundManager(threadContext) == FALSE)
				{
				    theErr = MEMORY_ERR;
				}
			}
		}
	}
    else
	{
	    theErr = NOT_SETUP;
	}
    return theErr;
}
#endif	// X_PLATFORM != X_WEBTV


void GM_FinisGeneralSound(void *threadContext)
{
    threadContext = threadContext;
    if (MusicGlobals)
	{
	    MusicGlobals->systemPaused = TRUE;
	    GM_FreeSong(threadContext, NULL);		// free all songs

	    // Close up sound manager BEFORE releasing memory!
	    //		GM_StopHardwareSoundManager(threadContext);

	    // clean up the verb buffers
	    GM_CleanupReverb();

	    XDisposePtr((XPTR)MusicGlobals);
	    MusicGlobals = NULL;
	}

#if USE_HAE_EXTERNAL_API == TRUE
    HAE_Cleanup();
#endif

#if X_PLATFORM == X_WIN_HARDWARE
    // Un-pagelock all static code and data of this executable
    UnlockMe();

    // Unlink from VxD
    CloseVxD();
#endif
}

UINT32 PV_ScaleVolumeFromChannelAndSong(GM_Song *pSong, INT16 channel, UINT32 volume)
{
    register UINT32		newVolume;
    TRACE_STR("> PV_ScaleVolumeFromChannelAndSong \n");
    // scale song volume based upon master song volume, only if a song channel
    if (channel != SOUND_EFFECT_CHANNEL)
	{
	    if (pSong)
		{
		    if (pSong->channelExpression[channel])
			{
				// for now, let's just scale up the volume level of the channel
				// Say 127 is 25.5% higher
#if USE_DLS
			    volume += (UINT32)pSong->channelExpression[channel];
#else
			    volume += (UINT32)pSong->channelExpression[channel] / 5;
#endif
			}

		    // scale note velocity via current channel volume
		    newVolume = (volume * (UINT32)pSong->channelVolume[channel]) / MAX_NOTE_VOLUME;

		    // scale note velocity via current song volume
		    newVolume = (newVolume * (UINT32)pSong->songVolume) / MAX_NOTE_VOLUME;
		}
	    else
		{
		    newVolume = volume;
		}
	}
    else
	{
	    // scale note velocity via current master effects volume
	    newVolume = (volume * (UINT32)MusicGlobals->effectsVolume) / MAX_MASTER_VOLUME;
	}
    TRACE_STR("< PV_ScaleVolumeFromChannelAndSong\n");
    return newVolume;
}


// ------------------------------------------------------------------------------------------------------//

/*
static const UBYTE stereoPanRamp[] =
{
0, 1, 1, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13,
14, 14, 15, 15, 16, 16, 17, 17, 18, 19, 19, 20, 20, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27,
28, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51, 52, 53, 54, 55, 56, 58, 59, 60, 62, 63, 64, 66, 67, 69, 70, 72, 73, 75, 77, 79, 81,
83, 85, 87, 89, 91, 94, 96, 99, 102, 105, 108, 111, 115, 119, 123, 127, 132, 138, 144, 151, 159,
169, 180, 195, 216, 252, 253
};
*/
#define USE_GS_RAMP		0

#if USE_GS_RAMP
// new GS ramp * 32. Divide by 32 after using this factor
static const short newStereoPanRamp[] = 
{
    0, 0, 32, 63, 95, 135, 167, 198, 230, 270, 302, 341, 373, 413, 445, 484, 516, 
    556, 595, 635, 667, 706, 746, 786, 825, 857, 897, 937, 976, 1016, 1056, 1095, 
    1135, 1175, 1214, 1254, 1294, 1333, 1373, 1421, 1461, 1500, 1540, 1580, 1619, 
    1659, 1699, 1738, 1778, 1826, 1865, 1905, 1945, 1984, 2024, 2064, 2104, 2143, 
    2183, 2223, 2262, 2302, 2342, 2373, 2413, 2453, 2492, 2532, 2564, 2603, 2643, 
    2675, 2715, 2746, 2786, 2818, 2857, 2889, 2929, 2961, 2992, 3024, 3064, 3096, 
    3127, 3159, 3191, 3223, 3254, 3278, 3310, 3342, 3365, 3397, 3429, 3453, 3477, 
    3508, 3532, 3556, 3588, 3612, 3635, 3659, 3683, 3699, 3723, 3747, 3770, 3786, 
    3810, 3826, 3842, 3866, 3882, 3897, 3913, 3929, 3945, 3961, 3977, 3992, 4008, 
    4016, 4032, 4040, 4056, 4064, 
}; 
static void PV_RemapMidiPan(INT32 stereoPosition, UINT32 *pLeft, UINT32 *pRight)
{
    UINT32	left, right;

    stereoPosition >>= 1;
    left = newStereoPanRamp[63 - stereoPosition] / 16;
    right = newStereoPanRamp[stereoPosition + 63] / 16;
    *pLeft = left;
    *pRight = right;
}
#endif

#if USE_DLS
static const UBYTE stereoPanRamp[] = 
{
    0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6,
    6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 11, 12, 12,
    12, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 18, 18, 18, 19, 19,
    20, 20, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 28, 29,
    30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
    46, 47, 49, 50, 51, 53, 54, 56, 58, 60, 62, 64, 67, 70, 73, 76, 80, 85, 91,
    98, 109, 127
};
static void PV_RemapMidiPan(INT32 stereoPosition, UINT32 *pLeft, UINT32 *pRight)
{
    UINT32	left, right;

    stereoPosition >>= 1;
    left = stereoPanRamp[63 - stereoPosition] * 5;
    right = stereoPanRamp[stereoPosition + 63] * 5;
    *pLeft = left;
    *pRight = right;
}
#endif
#if USE_DLS == 0 && USE_GS_RAMP == 0
static void PV_RemapMidiPan(INT32 stereoPosition, UINT32 *pLeft, UINT32 *pRight)
{
    UINT32	left, right;

    if (stereoPosition)
	{
	    if (stereoPosition < 0)	// left changes
		{
		    right = (MAX_NOTE_VOLUME-1) + stereoPosition;
		    //			left = (MAX_NOTE_VOLUME-1) - stereoPosition;		// new
		    left = (MAX_NOTE_VOLUME-1);
		}
	    else
		{					// right changes
		    right = (MAX_NOTE_VOLUME-1);
		    //			right = (MAX_NOTE_VOLUME-1) + stereoPosition;		// new
		    left = (MAX_NOTE_VOLUME-1) - stereoPosition;
		}
	}
    else
	{
	    left = MAX_NOTE_VOLUME;
	    right = MAX_NOTE_VOLUME;
	}
    *pLeft = left;
    *pRight = right;
}
#endif

/*
#if 0
	{
		register long *pMouse;

		pMouse = (long *)0x830;
		stereoPosition = ((*pMouse & 0xFFFFL) / 6) - 63;
	}
#endif
*/

// Given a stereo position from -63 to 63, return a volume level from 0 to 127
void PV_CalculateStereoVolume(GM_Voice *pVoice, INT32 *pLeft, INT32 *pRight)
{
    INT32	stereoPosition;
    UINT32	left, right;
    UINT32	noteVolume;

    TRACE_STR("> PV_CalculateStereoVolume\n");

    stereoPosition = pVoice->stereoPosition + pVoice->stereoPanBend;

    // multplex the master song pan
    if (pVoice->pSong)
	{
	    stereoPosition += (pVoice->pSong->songMasterStereoPlacement * 2);
	}
    stereoPosition *= -2;	// reverse left and right, and increase range

    // clip to absolute ranges
    if (stereoPosition >= (MAX_PAN_RIGHT * 2))
	{
	    stereoPosition = MAX_PAN_RIGHT * 2;
	}
    if (stereoPosition <= (MAX_PAN_LEFT * 2))
	{
	    stereoPosition = MAX_PAN_LEFT * 2;
	}

    if (pVoice->NoteChannel != SOUND_EFFECT_CHANNEL)
	{
	    // map pan for music channels
	    PV_RemapMidiPan(stereoPosition, &left, &right);
	}
    else
	{
	    if (stereoPosition)
		{
		    // map pan for effects channels
		    if (stereoPosition < 0)	// left changes
			{
			    right = (MAX_NOTE_VOLUME-1) + stereoPosition;
			    left = (MAX_NOTE_VOLUME-1) - stereoPosition;		// new
			}
		    else
			{					// right changes
			    right = (MAX_NOTE_VOLUME-1) + stereoPosition;		// new
			    left = (MAX_NOTE_VOLUME-1) - stereoPosition;
			} 
		}
	    else
		{
		    left = MAX_NOTE_VOLUME;
		    right = MAX_NOTE_VOLUME;
		}
	}

    // scale new volume based up channel volume, song volume, and current note volume
    noteVolume = PV_ScaleVolumeFromChannelAndSong(pVoice->pSong, pVoice->NoteChannel, pVoice->NoteVolume);
    noteVolume = (noteVolume * (UINT32)pVoice->NoteVolumeEnvelope) >> VOLUME_PRECISION_SCALAR;

    *pLeft = (left * noteVolume) / MAX_NOTE_VOLUME;
    *pRight = (right * noteVolume) / MAX_NOTE_VOLUME;

    // min out values to handle the fractional case
    if (*pLeft <= 1)
	{
	    *pLeft = 0;
	}
    if (*pRight <= 1)
	{
	    *pRight = 0;
	}
    TRACE_STR("< PV_CalculateStereoVolume\n");
}


void SetChannelVolume(GM_Song *pSong, INT16 the_channel, INT16 newVolume) {
    register GM_Mixer		*pMixer;
    register LOOPCOUNT		count;
    register GM_Voice		*theNote;

    TRACE_STR("> SetChannelVolume\n");

    pMixer = MusicGlobals;
    // update the current notes playing to the new volume
    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    theNote = &pMixer->NoteEntry[count];
	    if ( (theNote->voiceMode != VOICE_UNUSED) && (theNote->pSong == pSong) )
		{
		    if (theNote->NoteChannel == the_channel)
			{
			    // performance tweak. Kills note if volume level reaches zero, but this is bad
			    // for content that uses volume level to shape output
#if 0
			    if (newVolume == 0)
				{
				    theNote->voiceMode = VOICE_RELEASING;
				    theNote->NoteDecay = 0;
				    theNote->volumeADSRRecord.ADSRTime[0] = 1;
				    theNote->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
				    theNote->volumeADSRRecord.ADSRLevel[0] = 0;	// just in case
				}
#endif
				// now calculate the new volume based upon the current channel volume and
				// the unscaled note volume
			    newVolume = (INT16)PV_ScaleVolumeFromChannelAndSong(theNote->pSong, the_channel, theNote->NoteMIDIVolume);
				//CLS:  Do we not want to use a 32-bit intermediate value here?
			    newVolume = (INT16)((newVolume * MusicGlobals->scaleBackAmount) >> 8);
			    theNote->NoteVolume = newVolume;
			}
		}
	}
    TRACE_STR("< SetChannelVolume\n");
}


// Put all notes that have been in 'SUS_ON_NOTE' mode into their normal decay release mode
void PV_ChangeSustainedNotes(GM_Song *pSong, INT16 the_channel, INT16 data)
{
    register GM_Mixer		*pMixer;
    register LOOPCOUNT		count;
    register GM_Voice		*theNote;

    pMixer = MusicGlobals;
    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    theNote = &pMixer->NoteEntry[count];
	    if ( (theNote->voiceMode != VOICE_UNUSED) && (theNote->pSong == pSong) )
		{
		    if (theNote->NoteChannel == the_channel)
			{
			    if (data < 64)	// release. ( 0-63 off, 64-127 on)
				{
				    // the note has been released by the fingers, so release note
				    if (theNote->sustainMode == SUS_ON_NOTE_OFF)
					{
					    theNote->voiceMode = VOICE_RELEASING;	// decay note out to prevent clicks
					}
				    theNote->sustainMode = SUS_NORMAL;
				}
			    else
				{	// change status
				    // only do this if we're not sustaining a note already
				    if (theNote->sustainMode != SUS_ON_NOTE_OFF)
					{
					    theNote->sustainMode = SUS_ON_NOTE_ON;
					}
				}
			}
		}
	}
}

// Set stereo position from control values of 0-127. This will translate into values of 63 to -63
INT16 SetChannelStereoPosition(GM_Song *pSong, INT16 the_channel, UINT16 newPosition)
{
    register GM_Mixer		*pMixer;
    register LOOPCOUNT		count;
    register GM_Voice *	theNote;
    register INT16			newLogPosition;
    static char stereoScale[] =
    {
	63, 58, 55, 52, 50, 47, 45, 43,		41, 39, 37, 35, 33, 32, 30, 29,
	27, 26, 25, 23, 22, 21, 20, 19, 	18, 17, 17, 16, 15, 14, 14, 13, 
	12, 12, 11, 11, 10, 10,  9,  9,		 8,  8,  7,  7,  7,  6,  6,  6, 
	6,  5,  5,  5,  5,  4,  4,  4,		 4,  4,  3,  3,  3,  2,  1,  0,
	0,
	-1, -2, -3, -3, -3, -4, -4, -4, 	-4, -4, -5, -5, -5, -5, -6, -6, 
	-6, -6, -7, -7, -7, -8, -8, -9, 	-9,-10,-10,-11,-11,-12,-12,-13, 
	-14,-14,-15,-16,-17,-17,-18,-19,    -20,-21,-22,-23,-25,-26,-27,-29, 
	-30,-32,-33,-35,-37,-39,-41,-43,    -45,-47,-50,-52,-55,-58,-63
    };

    pMixer = MusicGlobals;
    // make sure and set the channel stereo position
    newLogPosition = stereoScale[newPosition];
    // update the current notes playing to the new stereo position. It will get incorporated into the mix at the
    // next audio frame
    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    theNote = &pMixer->NoteEntry[count];
	    if ( (theNote->voiceMode != VOICE_UNUSED) && (theNote->pSong == pSong) )
		{
		    if (theNote->NoteChannel == the_channel)
			{
			    theNote->stereoPosition = newLogPosition;
			}
		}
	}
    return newLogPosition;
}

// Set mod wheel position from control values of 0-127.
void SetChannelModWheel(GM_Song *pSong, INT16 the_channel, UINT16 value)
{
    register GM_Mixer		*pMixer;
    register LOOPCOUNT		count;
    register GM_Voice		*theNote;

    pMixer = MusicGlobals;

    // update the current notes playing to the new MOD wheel setting
    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    theNote = &pMixer->NoteEntry[count];
	    if ( (theNote->voiceMode != VOICE_UNUSED) && (theNote->pSong == pSong) )
		{
		    if (theNote->NoteChannel == the_channel)
			theNote->ModWheelValue = value;
		}
	}
}


// Change pitch all notes playing on this channel, and for new notes on this channel
INT16 SetChannelPitchBend(GM_Song *pSong, INT16 the_channel, UBYTE bendRange, UBYTE bendMSB, UBYTE bendLSB)
{
    register LOOPCOUNT		count;
    register GM_Mixer		*pMixer;
    register INT32			bendAmount, the_pitch_bend;
    register GM_Voice		*pNote;

    pMixer = MusicGlobals;
    // Convert LSB & MSB into values from -8192 to 8191
    the_pitch_bend = (((bendMSB * 128) + bendLSB) - 8192);

    // Scale values from -8192 to 8192 to -bend to bend in 8.8 fixed point
    bendAmount = bendRange * 256;
    the_pitch_bend = (the_pitch_bend * bendAmount) / 8192;

    // update the current note playing to the new bend value
    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    pNote = &pMixer->NoteEntry[count];
	    if ( (pNote->voiceMode != VOICE_UNUSED) && (pNote->pSong == pSong) )
		{
		    if (pNote->NoteChannel == the_channel)
			{
			    pNote->NotePitchBend = (INT16)the_pitch_bend;
			}
		}
	}
    return (INT16)the_pitch_bend;
}




UINT32 GM_GetSamplesPlayed(void) 
{
    return MusicGlobals->samplesPlayed;
}

#if X_PLATFORM != X_WEBTV
void GM_SetAudioTask(GM_AudioTaskCallbackPtr pTaskProc)
{
    if (MusicGlobals)
	{
	    MusicGlobals->pTaskProc = pTaskProc;
	}
}

void GM_SetAudioOutput(GM_AudioOutputCallbackPtr pOutputProc)
{
    if (MusicGlobals)
	{
	    MusicGlobals->pOutputProc = pOutputProc;
	}
}

GM_AudioTaskCallbackPtr GM_GetAudioTask(void)
{
    if (MusicGlobals)
	{
	    return MusicGlobals->pTaskProc;
	}
    return NULL;
}

GM_AudioOutputCallbackPtr GM_GetAudioOutput(void)
{
    if (MusicGlobals)
	{
	    return MusicGlobals->pOutputProc;
	}
    return NULL;
}
#endif	// X_PLATFORM != X_WEBTV

#if USE_HAE_EXTERNAL_API
// Connect to hardware.
//
// Global variables must be set before this can be called.
//
//	MusicGlobals->generate16output
//	MusicGlobals->generateStereoOutput
//	MusicGlobals->outputQuality
//
// Return FALSE if failure, otherwise TRUE
XBOOL GM_StartHardwareSoundManager(void *threadContext)
{
    INT32	sampleRate;
    int		ok;

    if (MusicGlobals)
	{
	    sampleRate = (INT32)GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality);

	    ok = HAE_AquireAudioCard(threadContext, sampleRate,
				     (MusicGlobals->generateStereoOutput) ? 2 : 1,
				     (MusicGlobals->generate16output) ? 16 : 8);
	    return (ok == 0) ? TRUE : FALSE;
	}
    return FALSE;
}

// Stop generating samples and shutdown
void GM_StopHardwareSoundManager(void *threadContext)
{
    // need this so that we can properly update samplesPlayed to the total written
    // to the *device* when we close the audio device.  UpdateSamplesPlayed() needs
    // to take the current device playback position as its argument.  when we shut
    // down the device, we want to move it to represent everything submitted.  if the
    // device has been opened and closed before, the device-end-position is different
    // than MusicGlobals->samplesWritten.
    static UINT32 lastSamplesWritten = 0;

    // everything that's going to play has been played; samples played by mixer
    // should equal samples submitted.  if we don't synch these here, our count
    // of samples played and samples submitted to device diverge after closing
    // and reopening the device.

    HAE_ReleaseAudioCard(threadContext);
    if (MusicGlobals)
	{
	    GM_UpdateSamplesPlayed((MusicGlobals->samplesWritten - lastSamplesWritten));
	    lastSamplesWritten = MusicGlobals->samplesWritten;
	}
}

// Get current audio time stamp in microseconds; this is the
// microseconds' worth of samples that have passed through the
// audio device.  it never decreases.
// $$kk: this and all the time stamp methods should move into a common file
// CLS:  copied this function in from Kara's
UINT32 GM_GetDeviceTimeStamp(void)
{
    UINT16	sampleRate;

    if (MusicGlobals)
	{
	    // convert from samples into microseconds
	    sampleRate = (UINT16)GM_ConvertFromOutputQualityToRate(MusicGlobals->outputQuality);
#if USE_FLOAT == FALSE
	    return (MusicGlobals->samplesPlayed * 1000000) / sampleRate;
#else
	    return (UINT32)(((float) MusicGlobals->samplesPlayed / sampleRate) * 1000000);
#endif
	}
    return 0L;
}

// Update count of samples played.  This function caluculates from number of bytes,
// given the sample frame size from the mixer variables
// $$kk: 08.12.98 merge: changed this function
// $$kk: no, we're getting the currentPos in SAMPLES, not bytes, from HAE_GetDeviceSamplesPlayedPosition().
void GM_UpdateSamplesPlayed(UINT32 currentPos)
{
    UINT32 delta;

    if (currentPos >= MusicGlobals->lastSamplePosition)
	{
	    //		delta = ((currentPos - MusicGlobals->lastSamplePosition) / MusicGlobals->sampleFrameSize);
	    delta = (currentPos - MusicGlobals->lastSamplePosition);
	}
    else
	{
	    //		delta = (currentPos / MusicGlobals->sampleFrameSize);
	    delta = currentPos;
	}

    MusicGlobals->lastSamplePosition = currentPos;

    // update mixer samples played
    MusicGlobals->samplesPlayed += delta;
#if USE_STREAM_API == TRUE
    // update samples played for each stream
    GM_AudioStreamUpdateSamplesPlayed(delta);
#endif
}

// number of devices. ie different versions of the HAE connection. DirectSound and waveOut
// return number of devices. ie 1 is one device, 2 is two devices.
// NOTE: This function needs to function before any other calls may have happened.
INT32 GM_MaxDevices(void)
{
    return HAE_MaxDevices();
}

// set the current device. device is from 0 to GM_MaxDevices()
// NOTE:	This function needs to function before any other calls may have happened.
//			Also you will need to call HAE_ReleaseAudioCard then HAE_AquireAudioCard
//			in order for the change to take place.
void GM_SetDeviceID(INT32 deviceID, void *deviceParameter)
{
    HAE_SetDeviceID(deviceID, deviceParameter);
}

// return current device ID
// NOTE: This function needs to function before any other calls may have happened.
INT32 GM_GetDeviceID(void *deviceParameter)
{
    return HAE_GetDeviceID(deviceParameter);
}

// get deviceID name
// NOTE:	This function needs to function before any other calls may have happened.
//			Format of string is a zero terminated comma delinated C string.
//			"platform,method,misc"
//	example	"MacOS,Sound Manager 3.0,SndPlayDoubleBuffer"
//			"WinOS,DirectSound,multi threaded"
//			"WinOS,waveOut,multi threaded"
//			"WinOS,VxD,low level hardware"
//			"WinOS,plugin,Director"
void GM_GetDeviceName(INT32 deviceID, char *cName, UINT32 cNameLength)
{
    HAE_GetDeviceName(deviceID, cName, cNameLength);
}

// Get current audio time stamp based upon the audio built interrupt
UINT32 GM_GetSyncTimeStamp(void)
{
    if (MusicGlobals)
	{
	    return MusicGlobals->syncCount;
	}
    return 0L;
}

INT32 GM_GetAudioBufferOutputSize(void)
{
    return HAE_GetAudioByteBufferSize();
}

// Get current audio time stamp based upon the audio built interrupt, but ahead in time and quantized for
// the particular OS
UINT32 GM_GetSyncTimeStampQuantizedAhead(void)
{
    return GM_GetSyncTimeStamp() + (HAE_GetSliceTimeInMicroseconds() * HAE_GetAudioBufferCount());
}



#endif

/* EOF of GenSetup.c
 */

