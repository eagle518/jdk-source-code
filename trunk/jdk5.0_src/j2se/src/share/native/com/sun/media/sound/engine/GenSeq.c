/*
* @(#)GenSeq.c	1.37 03/12/19
*
* Copyright 2004 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
*/
/*****************************************************************************/
/*
** "GenSeq.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Overview
**	General purpose Music Synthesis software, C-only implementation (no assembly
**	language optimizations made)
**
**	Contains only MIDI Sequencer
**
**	Modification History:
**
**	8/19/93		Split from Synthesier.c
**	11/7/95		Major changes, revised just about everything.
**	11/11/95	Started implementing exteral midi controls
**				Created Q_MIDIEvent queue system.
**				Created external GM_ interface to midi commands that can be added to queue
**				Created midi queue processor
**				Added semaphore locking of queue processing
**	11/15/95	Fixed bug with GM_GetSongVolume. Wrong values
**	12/4/95		Added DISABLE_QUEUE as a debugging feature
**	12/5/95 	Modified to support reverb; 32 bit in GM_GetAudioSampleFrame
**	12/6/95 	Moved function GM_GetAudioSampleFrame into GenSynth.c
**				Put GM_SetReverbType into PV_ConfigureMusic
**	12/7/95		Added channelReverb to PV_ResetControlers
**				Added PV_SetChannelReverb
**	1/7/96		Added GM_SetEffectsVolume & GM_GetEffectsVolume
**	1/12/96		Fixed bug with GM_BeginSong that would not set up the voices at song start up
**	1/18/96		Implemented GM patch bank loading for percussion and other banks
**	1/23/96		Added GM_FreeSong
**	1/25/96		Moved GM_GetUsedPatchlist to GenSong.c
**				Fixed bug with default percussion forcing a load of instrument 0
**	1/28/96		Moved GM_FreeSong to GenSong.c
**	2/5/96		Removed unused variables. Working towards multiple songs
**	2/12/96		Moved cleaning of external midi queue to Init code
**				Removed extra system setup for processing songs
**				GM_BeginSong now does even more, since its suppose to start a song
**	2/13/96		GM_BeginSong will now try to play more than one song. It puts it into the queue
**				Lots of multi-song support.
**				Moved sequencer slice control code from GenSynth.c
**	2/18/96		Fixed bug that was ignoring controller events and stuff during a position scan
**	2/29/96		Added support for looping controls
**	3/1/96		Changed static variables to static const
**	3/2/96		Fixed bug with reading sysex data wrong
**	3/5/96		Changed GM_GetSongVolume & GM_SetSongVolume to only use songVolume
**				Eliminated the global songVolume
**	3/21/96		Started implementing Igor meta events
**	3/25/96		Removed private PV_Get4Type to XGetLong
**	3/28/96		Fixed lock up bug that I created when reading meta events!
**	4/8/96		Fixed key split problem during meta instrument load
**	4/11/96		Fixed a memory leak caused by instruments not being set into the cache correctly
**	4/15/96		Added resource name into eMidi data structure
**				Added support to interpret SONG resource via eMidi
**	4/21/96		Added GM_GetRealtimeAudioInformation
**				Removed register usage in parameters
**	5/12/96		Added support for csnd in meta events
**				Added some failsafe support in PV_ProcessIgorMeta
**	5/18/96		Added error condition to PV_MusicIRQ
**	6/12/96		Changed behavior of all notes off control
**	6/30/96		Added fix to all notes off control that fixed a buzz effect
**				Changed font and re tabbed
**				Removed some compiler warnings
**	7/3/96		Added some support for loopstart & loopend Headspace markers
**	7/5/96		Now using GM_EndAllNotes because it sets them to release mode
**				Added to GM_EndSong a clearing of the song's track pointers
**	7/6/96		Added GM_GetSyncTimeStamp
**	7/7/96		Changed PV_ProcessMetaEvents to not use an extra 256 bytes of stack space
**	7/10/96		Fixed reverb unit to be able to be turned off.
**	7/23/96		Changed QGM_ functions to use an unsigned long for time stamp
**	7/24/96		Changed external midi queue to use a head/tail
**	7/25/96		Moved GM_GetSyncTimeStamp to Gen(XXX)Tools.c
**	8/12/96		Changed PV_ResetControlers to support semi-complete reset
**	9/10/96		Added GM_NoteOn & GM_NoteOff & GM_ProgramChange & GM_PitchBend &
**				GM_Controller & GM_AllNotesOff for direct control to bypass the queue
**	9/18/96		Changed PV_CallSongCallback and GM_SongCallbackProcPtr to
**				pass a GM_Song structure rather than an ID
**	9/19/96		Added GM_GetSongTempo
**	9/20/96		Added GM_SetSongTempo & GM_ResumeSong & GM_PauseSong
**	9/23/96		Added GM_MuteTrack & GM_UnmuteTrack & GM_MuteChannel & GM_UnmuteChannel
**				Fixed GM_SetMasterSongTempo & GM_SetSongTempo to change the tempo at execution
**	9/24/96		Added GM_SetSongTempInBeatsPerMinute & GM_GetSongTempoInBeatsPerMinute
**				Added GM_SoloTrack & GM_SoloChannel
**				Added GM_GetSongPitchOffset & GM_SetSongPitchOffset
**	9/25/96		Added GM_GetChannelMuteStatus & GM_GetTrackMuteStatus
**				Added controller 90 to change the global reverb type
**	9/28/96		Added GS workaround that attempts to smartly ignore GS bank selects
**	10/2/96		Changed GM_SetSongPitchOffset to end all notes before changing offset
**	10/13/96	Changed QGM_AllNotesOff to work with the queue and post an event
**	10/23/96	Removed reference to BYTE and changed them all to UBYTE or SBYTE
**	10/30/96	Modified QGM_NoteOn & QGM_NoteOff & QGM_ProgramChange &
**				QGM_PitchBend & QGM_Controller & QGM_AllNotesOff to accept a GM_Song *
**	10/31/96	Changed trackMuted and channelMuted to be bit flag based
**				Added soloTrackMuted bit array for solo control
**	11/3/96		Added midi pitch to GM_GetRealtimeAudioInformation
**				Changed the way the expression controller calculates extra volume
**	11/6/96		Added GM_UnsoloTrack, GM_GetTrackSoloStatus
**	11/7/96		Added GM_SongTimeCallbackProcPtr & GM_SetSongTimeCallback
**	11/9/96		Fixed bug with GM_EndSong
**				Fixed bug with GM_BeginSong that would double speed play a song
**				if already playing
**	11/14/96	Added pSong reference in GM_GetRealtimeAudioInformation
**				Added NRPN for disabling GM percussion behavior
**	11/19/96	Added channelBankMode stuff for controlling bank selects
**				Added a GM_Song structure to GM_ConvertPatchBank
**	11/21/96	Fixed bug with sequencerPaused and not pausing the sequencer
**				Changed GM_ConvertPatchBank to PV_ConvertPatchBank
**	11/26/96	Added GM_GetControllerValue
**	12/3/96		Removed default stereo pan placements
**	12/11/96	Fixed a bank select bug in PV_ProcessController
**	12/15/96	Added controls for DEFAULT_VELOCITY_CURVE
**	12/30/96	Changed copyright
**	1/12/97		Changed maxNormalizedVoices to mixLevel
**	1/21/97		Changed loopstart and loopend tags to ignore case
**	1/23/97		Added GM_SetSongFadeRate
**	1/24/97		Changed GM_GetSongVolume to return 0 if song is dead
**	1/30/97		Changed PV_ProcessIgorResource to handle embedded encrypted samples
**				Removed all '' constants and defined them
**	2/1/97		Added support for pitch offset control on a per channel basis
**				Added GM_DoesChannelAllowPitchOffset & GM_AllowChannelPitchOffset
**				Changed GM_SetSongPitchOffset to no longer kill all notes currently
**				playing when a new offset is set
**	2/2/97		Added the ability to create an inner loop count. ie loopstart=5
**	3/24/97		Fixed a problem loading percusion instruments when not use the normal
**				GM percussion channel
**	4/20/97		Added a meta event callback system
**				Changed PV_MusicIRQ to PV_ProcessMidiSequencerSlice
**	5/6/97		Fixed a time stamp rollerover bug with PV_GetNextReadOnlyQueueEvent
**				thanks marc@be.com!
**	6/20/97		Changed PV_ProcessExternalMIDIQueue to ignore the processExternalMidiQueue
**				flag. Its safe anyways because of the way events are written into queue.
**				Seems more stable.
**	6/25/97		Fixed a bug when looping songs in PV_ProcessMidiSequencerSlice. Thanks Kurt
**	7/8/97		Added or changed GM_GetSongLoopFlag & GM_SetSongLoopMax &
**				GM_SetSongLoopFlag & GM_GetSongLoopMax
**				Fixed and reworked the looping logic again. It failed to take into account
**				maxLoopCount and controller 85 (loop control)
**	7/12/97		Added a drift fixer patch to PV_ProcessSequencerEvents that tries
**				to compensate for real time midi time stamping
**	7/21/97		Put mins and maxs on GM_SetSongTempInBeatsPerMinute. 0 and 500
**				Changed GM_SetSongTempInBeatsPerMinute & GM_GetSongTempoInBeatsPerMinute
**				GM_SetSongTempo & GM_GetSongTempo & GM_SetMasterSongTempo to all be unsigned longs
**				Changed key calculations to floating point to test for persicion loss.
**	7/22/97		Changed SYNC_BUFFER_TIME to BUFFER_SLICE_TIME
**				Removed some & 0xFFFF from various calculations of song tempo
**	7/28/97		Removed extra goto in sequencer code. Ugly, ugly. When I have time, I'm going
**				to rewrite!!
**	8/8/97		(dbz) Added PV_FreePgmEntries
**	8/15/97		Renamed PV_ProcessMetaMarketEvents to PV_ProcessMetaMarkerEvents. Also combined
**				sequencer processing of marker events and callback code
**	8/25/97		Changed GM_SetControllerCallback & PV_CallControlCallbacks to solve problems with
**				compiler generating bad code
**	9/15/97		Added GM_GetMasterSongTempo
**	9/16/97		Fixed bug in PV_ProcessController with sustain pedal. Was checking the wrong
**				mid point for off.
**	9/18/97		Modified GM_SetSongPitchOffset & GM_GetSongPitchOffset to use positive values
**				to increase pitch, and negitive values will decrease pitch
**	9/19/97		ddz Added PV_FreePatchInfo & PV_InsertBankSelect
**	9/25/97		Added TrackStatus to represent track status rather than 'R' and 'F'. Symbolic use,
**				and changed trackon to use these new symbols.
**	9/30/97		Changed PV_IsMuted to allow an invalid track so that direct midi control can
**				be muted. Also modified all functions that called PV_IsMuted to handle this condition
**	10/4/97		Flushed out GM_GetControllerValue to return more controller values
**	10/12/97	Fixed SetChannelVolume & PV_ChangeSustainedNotes & SetChannelStereoPosition &
**				SetChannelModWheel & SetChannelPitchBend & PV_SetChannelReverb to check for a
**				particular song before changing playing voices. This fixed a bug in which a
**				pitch bend from one song would affect another
**	10/15/97	Added processingSlice tags in PV_ProcessMidiSequencerSlice
**	10/16/97	Added flag to allow optional reconfigure of the mixer when starting a song in
**				GM_BeginSong
**				Removed lame instrument cache support from PV_ProcessIgorMeta
**	10/26/97	Removed last reference to global variables from PV_ProcessMidiSequencerSlice
**	10/27/97	Removed reference to MusicGlobals->theSongPlaying
**	12/16/97	Moe: removed compiler warnings
**	1/21/98		Added GM_SetChannelVolume & GM_GetChannelVolume
**	1/29/98		Added some new midi control types in PV_ProcessController
**	2/5/98		Fixed PV_ProcessIgorResource to handle encrypted eMidi samples and to enable
**				caching
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	2/9/98		Fixed a bug with PV_ConfigureMusic that walked past the memory
**				allocated.
**	2/11/98		Put code wrappers around functions not used for WebTV
**	2/15/98		Changed references to BUFFER_SLICE_TIME to HAE_GetSliceTimeInMicroseconds()
**	2/20/98		kcr		added support for chorus and more variable reverb
**	2/20/98		kcr		fixed bug in enabling chorus/reverb
**	3/12/98		Added vibrato and filter NRPN's cases to PV_ProcessRegisteredParameters. They
**				are not wired yet. Pulled the values from GM/GS spec sheets
**				Changed the behavior of GM_GetSongVolume. It now returns the last volume
**				even if the song is not playing
**	4/27/98		MOE: Changed calls to XDecompressPtr()
**	5/7/98		Renamed myMusicGlobals to pMixer. Added GM_SetChannelReverb & GM_GetChannelReverb.
**	5/14/98		Modified PV_ProcessMetaMarkerEvents and PV_ProcessMidiSequencerSlice to handle
**				loopstart and loopend better. Now it walks through all the tracks before reseting
**				to loop point and kills all notes that are playing and the loopend mark to eliminate
**				any hanging notes
**	5/15/98		Added trackStatusSave to the GM_Song structure will helps when doing loopstart/loopend
**	6/18/98		Modified PV_ProcessController to store the pan (10) value unmodified, but then
**				scale the GM_Voice pan value correctly
**	6/30/98		Removed the label RestartTrackProcess in PV_ProcessMidiSequencerSlice
**	7/6/98		Removed a compiler warning in GM_SetChannelReverb
**				Fixed a bug with PV_ConfigureInstruments that caused stereo instruments to pan right and
**				cleaned up stereoPan related extras, and removed extra control overlaps.
**	7/27/98		Reenabled processExternalMidiQueue support in PV_ProcessExternalMIDIQueue
**	7/28/98		Added GM_SetSongStereoPosition & GM_GetSetStereoPosition
**				Changed meaning of processExternalMidiQueue in the GM_Mixer structure. Now its a count
**				rather than a boolean
**	8/5/98		Put wrappers around GM_GetRealtimeAudioInformation to protect against
**				MusicGlobals not being allocated
**	8/13/98		Added voiceType to GM_GetRealtimeAudioInformation
**	10/6/98		Fixed bug GM_GetControllerValue to return correct on/off value for channelSustain
**	11/9/98		Added GM_PrerollSong and change GM_BeginSong to use it.
**				Renamed NoteDur to voiceMode
**	11/30/98	Added support for omni mode in PV_ConfigureInstruments
**				Modified controller 123 to look at omni mode, and made controller 120 be the same
**				as 123
**	12/9/98		Added GM_GetPitchBend
**	12/18/98	Added auto remix of song start and reconfigure on mixer
**	1/13/99		Added some auto mix level code and voice calculation stuff
**				Changed system of auto level
**	2/24/99		Removed extra reference from GM_SetSongMetaEventCallback
**	3/5/99		Added threadContext to PV_ServeSongFade & PV_CallSongCallback &
**				PV_CallSongMetaEventCallback & GM_EndSong & PV_CallControlCallbacks &
**				PV_ProcessMidiSequencerSlice & PV_ProcessSequencerEvents
**
**	JAVASOFT
**	05.04.99	Changed calls to synth to work through a pSynth pointer so that we can direct the
**	06.21.99	calls to an external synth or the Beatnik synth.
*/
/*****************************************************************************/

//#define USE_DEBUG	1

#include "GenSnd.h"
#include "GenPriv.h"
#include "X_Formats.h"

#if USE_HAE_EXTERNAL_API == TRUE
#include "HAE_API.h"
#endif

#define IGOR_SYSEX_ID	0x0000010D

/*
March 20, 1996

Igor's Software Laboratories
Steve Hales
882 Hagemann Drive
Livermore CA 94550
Tel: (925) 449-1947
Fax: (925) 449-1342
steve@igorlabs.com

Dear Member:

This letter confirms our receipt of your application for a System Exclusive ID
number and your payment of the processing fee. You have been assigned the MIDI
System Exclusive ID number:

ID # :  00H 01H 0DH
*/

#define DISABLE_QUEUE		0		// if 1 then QGM_ API is not queued. This is used for debugging only

// Scale the division amount by the current tempo:
static void PV_ScaleDivision(GM_Song *pSong, UFLOAT div)
{
    register UFLOAT	midiDivsion;

    if (div)
	{
	    if (pSong->MIDITempo)
		{
		    midiDivsion = (div * (UFLOAT)64) / pSong->MIDITempo;
		    midiDivsion = (midiDivsion * pSong->MasterTempo) / 0x10000L;
		}
	    else
		{
		    midiDivsion = 0;
		}
	    if (pSong->AnalyzeMode == SCAN_SAVE_PATCHES)
		{
		    midiDivsion = (UFLOAT)0x7FFF;	// speed things up
		}
	    pSong->MIDIDivision = midiDivsion;
	}
}


// Resets song controlers. Pass -1 to reset all channels, otherwise just the channel passed
// will be reset.
// If completeReset is TRUE, then a complete controler reset will be done, otherwise just a
// semi complete to support the new GM reset controller protocol
void PV_ResetControlers(GM_Song *pSong, INT16 channel2Reset, XBOOL completeReset)
{
    register LOOPCOUNT		count, max, start;
    register XBOOL		generateStereoOutput;

    generateStereoOutput = MusicGlobals->generateStereoOutput;	// cache this
    if (channel2Reset == -1)
	{
	    // do all channels
	    max = MAX_CHANNELS;
	    start = 0;
	}
    else
	{
	    // do just this one
	    max = channel2Reset + 1;
	    start = channel2Reset;
	}
    for (count = start; count < max; count++)
	{
	    if (completeReset)
		{
		    if (channel2Reset == -1)
			{	// if all channels, then reset default programs
			    pSong->channelProgram[count] = (INT16)count;
			    if (count == PERCUSSION_CHANNEL)
				{
				    pSong->channelProgram[count] = 0;
				}
			}

		    pSong->channelVolume[count] = MAX_NOTE_VOLUME;			// controler 7
		    pSong->channelExpression[count] = 0;					// no extra expression
#if USE_DLS
		    pSong->channelExpression[count] = MAX_NOTE_VOLUME;
#else
		    pSong->channelChorus[count] = 0;						// no chorus
#endif
		    pSong->channelReverb[count] = DEFAULT_REVERB_LEVEL;	// enable reverb
		    pSong->channelChorus[count] = DEFAULT_CHORUS_LEVEL;	// (default is zero)

		    pSong->channelStereoPosition[count] = 64;			// set to middle

		    pSong->channelLowPassAmount[count] = 0;
		    pSong->channelResonanceFilterAmount[count] = 0;
		    pSong->channelFrequencyFilterAmount[count] = 0;
		}
	    pSong->channelWhichParameter[count] = USE_NO_RP;
	    pSong->channelRegisteredParameterLSB[count] = -1;
	    pSong->channelRegisteredParameterMSB[count] = -1;
	    pSong->channelNonRegisteredParameterLSB[count] = -1;
	    pSong->channelNonRegisteredParameterMSB[count] = -1;
	    pSong->channelSustain[count] = FALSE;
	    pSong->channelBankMode[count] = USE_GM_DEFAULT;
	    pSong->channelBank[0] = 0;
	    pSong->channelPitchBendRange[count] = DEFAULT_PITCH_RANGE;			// pitch bend controler
	    pSong->channelBend[count] = 0;
	    pSong->channelModWheel[count] = 0;
	}
}

void PV_ConfigureInstruments(GM_Song *theSong)
{
    register short int	count;

    theSong->omniModeOn = TRUE;
    PV_ResetControlers(theSong, -1, TRUE);

    for (count = 0; count < MAX_CHANNELS; count++)
	{
	    if (theSong->firstChannelProgram[count] != -1)
		{
		    theSong->channelProgram[count] = theSong->firstChannelProgram[count];
		    theSong->channelBank[count] = theSong->firstChannelBank[count];
		}
	}

    // Any stereo instruments loaded, if so then don't use default pan positions just center
    if (GM_AnyStereoInstrumentsLoaded(theSong))
	{
	    for (count = 0; count < MAX_CHANNELS; count++)
		{
		    theSong->channelStereoPosition[count] = 64;		// set to middle
		}
	}
    if (theSong->defaultPercusionProgram == -1)
	{
	    theSong->channelProgram[PERCUSSION_CHANNEL] = 0;
	    theSong->channelBank[PERCUSSION_CHANNEL] = 0;
	    theSong->firstChannelProgram[PERCUSSION_CHANNEL] = 0;
	    theSong->firstChannelBank[PERCUSSION_CHANNEL] = 0;
	}
    else
	{
	    if (theSong->defaultPercusionProgram)
		{
		    theSong->channelProgram[PERCUSSION_CHANNEL] = theSong->defaultPercusionProgram;
		}
	}

    // Set up the default division for the MIDI file:
    if (theSong->MasterTempo == 0L)
	{
	    theSong->MasterTempo = XFIXED_1;	// 1.0
	}

    // Sample output at 22.050 khz

#if USE_FLOAT == FALSE
    theSong->UnscaledMIDITempo = 500000;
#else
    theSong->UnscaledMIDITempo = 495417;	// (22050/22254) * 500000
#endif
    theSong->songMicroseconds = 0;
    theSong->CurrentMidiClock = 0;
#if USE_HAE_EXTERNAL_API == TRUE
    theSong->MIDITempo = (theSong->UnscaledMIDITempo / HAE_GetSliceTimeInMicroseconds());
#else
    theSong->MIDITempo = (theSong->UnscaledMIDITempo / BUFFER_SLICE_TIME);
#endif
    theSong->UnscaledMIDIDivision = 60;
    PV_ScaleDivision(theSong, theSong->UnscaledMIDIDivision);
}

// Configure the global synth variables from the passed song
OPErr PV_ConfigureMusic(GM_Song *theSong)
{
    register UINT32		count;
    register UBYTE		*scanptr;
    register INT16		numtracks;
    register UINT32		trackLength;
    INT16				realtracks;
    OPErr				theErr;
    XBOOL				safe;
    UINT32				size;
    UINT32 dataOffset;

    theErr = BAD_MIDI_DATA;				// assume the worst
    PV_ConfigureInstruments(theSong);
    scanptr = (UBYTE *)theSong->midiData;
    if (scanptr)
	{
	    safe = FALSE;
	    // don't walk past the midi file length
	    size = (theSong->midiSize < 3000 - (INT32)sizeof(INT32)) ? theSong->midiSize :
		3000 - (INT32)sizeof(INT32);
	    for (count = 0; count < size; count++)
		{
		    if (XGetLong(scanptr) == ID_MTHD)
			{
			    safe = TRUE;
			    break;
			}
		    scanptr++;
		}
	    if (safe)
		{
		    if (XGetShort(&scanptr[8]) < 2)		// only support format 0 and 1 midi files
			{
			    realtracks = XGetShort(&scanptr[10]);	// get real tracks and compare with tracks found to determine if corrupt
			    count = XGetShort(&scanptr[12]);		// get ticks per quarter note
			    theSong->UnscaledMIDIDivision = (UFLOAT)count;
			    PV_ScaleDivision(theSong, theSong->UnscaledMIDIDivision);
			    theSong->TSNumerator = 4;
			    theSong->TSDenominator = 2;
			    // search for first midi track
			    safe = FALSE;
			    // don't walk past the midi file length
			    size = (theSong->midiSize < 3000 - (INT32)sizeof(INT32)) ? theSong->midiSize :
				3000 - (INT32)sizeof(INT32);
			    for (count = 0; count < size; count++)
				{
				    if (XGetLong(scanptr) == ID_MTRK)
					{
					    safe = TRUE;
					    break;
					}
				    scanptr++;
				}
			    if (safe)
				{
				    numtracks = 0;
				    dataOffset = 0;
				    while ( (numtracks < MAX_TRACKS) && (XGetLong(scanptr) == ID_MTRK) )
					{
					    // calculate length of this track.
					    scanptr += 4;
					    trackLength = *scanptr++;
					    trackLength = (trackLength << 8) + *scanptr++;
					    trackLength = (trackLength << 8) + *scanptr++;
					    trackLength = (trackLength << 8) + *scanptr++;
			    
					    //$$fb sanity check for the track len: don't read past the end of the file
					    dataOffset += 4;
					    if ((dataOffset + trackLength) > theSong->midiSize) {
						//printf("track %d: trackLength is %d, but can be max. %d!\n", 
						//       numtracks, trackLength, theSong->midiSize - dataOffset);
						trackLength = theSong->midiSize - dataOffset;
					    }

					    theSong->ptrack[numtracks] = scanptr;
					    theSong->trackstart[numtracks] = scanptr;
					    theSong->trackticks[numtracks] = 0;
					    theSong->trackcumuticks[numtracks] = 0;
					    theSong->trackon[numtracks] = TRACK_FREE;
					    theSong->tracklen[numtracks] = trackLength;
					    scanptr += trackLength;
					    dataOffset += trackLength;
					    numtracks++;
					    if (dataOffset >= theSong->midiSize) {
						// no business in reading any further
						break;
					    }
					}
				    if (numtracks == realtracks)	// do real tracks match number of found tracks?
					{
					    theErr = NO_ERR;		// all is well in midi land
					}
				}
			}
		}
	}
    return theErr;
}

// Process any fading song voices
static void PV_ServeSongFade(void *threadContext, GM_Song *pSong)
{
    INT32	value;

    if (pSong)
	{
	    if (pSong->songFadeRate)
		{
		    pSong->songFixedVolume -= pSong->songFadeRate;
		    value = XFIXED_TO_LONG(pSong->songFixedVolume);
		    if (value > pSong->songFadeMaxVolume)
			{
			    value = pSong->songFadeMaxVolume;
			    pSong->songFadeRate = 0;
			}
		    if (value < pSong->songFadeMinVolume)
			{
			    value = pSong->songFadeMinVolume;
			    pSong->songFadeRate = 0;
			}
		    GM_SetSongVolume(pSong, (INT16)value);

		    if ( (pSong->songFadeRate == 0) && (pSong->songEndAtFade) )
			{	// we're done!
			    GM_KillSongNotes(pSong);
			    GM_EndSong(threadContext, pSong);
			}
		}
	}
}

// Set song fade rate. Its a 16.16 fixed value
// Input:	pSong		song to affect
//			fadeRate	amount to change every 11 ms
//						example:	FLOAT_TO_XFIXED(2.2) will decrease volume
//									FLOAT_TO_XFIXED(2.2) * -1 will increase volume
//			minVolume	lowest volume level fade will go
//			maxVolume	highest volume level fade will go
//			endSong		when fade has reached its end, if endSong is TRUE the song
//						will be stopped
void GM_SetSongFadeRate(GM_Song *pSong, XFIXED fadeRate,
			INT16 minVolume, INT16 maxVolume, XBOOL endSong)
{
    if (pSong)
	{
	    pSong->songFadeMaxVolume = maxVolume;
	    pSong->songFadeMinVolume = minVolume;
	    pSong->songFixedVolume = LONG_TO_XFIXED(pSong->songVolume);
	    pSong->songEndAtFade = endSong;
	    pSong->songFadeRate = fadeRate;
	}
}

// preroll song but don't start
static OPErr GM_PrerollSong(GM_Song *theSong, GM_SongCallbackProcPtr theCallbackProc)
{
    OPErr		theErr;
    short int	count;

    theErr = NO_ERR;
    theSong->AnalyzeMode = SCAN_NORMAL;
    theSong->songEndCallbackPtr = theCallbackProc;

    theErr = PV_ConfigureMusic(theSong);
    if (theErr == NO_ERR)
	{
	    theSong->SomeTrackIsAlive = TRUE;
	    theSong->songFinished = FALSE;

	    // first time looping, and set mute tracks to off
	    theSong->songLoopCount = 0;
	    theSong->songMaxLoopCount = 0;
	    for (count = 0; count < MAX_TRACKS; count++)
		{
		    XClearBit(&theSong->trackMuted, count);
		    XClearBit(&theSong->soloTrackMuted, count);
		    theSong->pTrackPositionSave[count] = NULL;
		    theSong->trackTicksSave[count] = 0;
		    theSong->trackStatusSave[count] = TRACK_OFF;
		}
	    theSong->loopbackSaved = FALSE;
	    theSong->loopbackCount = -1;
	    for (count = 0; count < MAX_CHANNELS; count++)
		{
		    XClearBit(&theSong->channelMuted, count);
		    XClearBit(&theSong->soloChannelMuted, count);
		    XSetBit(&theSong->allowPitchShift, count);
		}
	    XClearBit(&theSong->allowPitchShift, PERCUSSION_CHANNEL);		// don't allow pitch changes on percussion

	    theSong->velocityCurveType = DEFAULT_VELOCITY_CURVE;
	}
    return theErr;
}

// This will walk through all active songs and return a max number of voices used for midi
static short int PV_GetMaxVoicesPlayingFromAllSongs(void)
{
    register GM_Mixer		*pMixer;
    register GM_Song		*pSong;
    register short int		count, max;

    max = 0;
    pMixer = MusicGlobals;
    if (pMixer)
	{
	    for (count = 0; count < MAX_SONGS; count++)
		{
		    pSong = MusicGlobals->pSongsToPlay[count];
		    if (pSong)
			{
			    max += pSong->maxSongVoices;
			}
		}
	}
    return max;
}

// This will walk through all active songs and return a max mix level used for midi
static short int PV_GetMixLevelPlayingFromAllSongs(void)
{
    register GM_Mixer		*pMixer;
    register GM_Song		*pSong;
    register short int		count, mix;

    mix = 0;
    pMixer = MusicGlobals;
    if (pMixer)
	{
	    for (count = 0; count < MAX_SONGS; count++)
		{
		    pSong = MusicGlobals->pSongsToPlay[count];
		    if (pSong)
			{
			    mix += pSong->mixLevel;
			}
		}
	}
    return mix;
}

// Set up the system to start playing a song
OPErr GM_BeginSong(GM_Song *theSong, GM_SongCallbackProcPtr theCallbackProc,
		   XBOOL useEmbeddedMixerSettings, XBOOL autoLevel)
{
    OPErr		theErr;
    short int	songSlot, count;

    theErr = NO_ERR;
    if (theSong)
	{
	    theSong->songPaused = FALSE;
	    // first find a slot in the song queue
	    songSlot = -1;
	    for (count = 0; count < MAX_SONGS; count++)
		{
		    // Reuse slot, if song already playing
		    if (MusicGlobals->pSongsToPlay[count] == theSong)
			{
			    MusicGlobals->pSongsToPlay[count] = NULL;
			    GM_KillSongNotes(theSong);
			    songSlot = count;
			    break;
			}
		    // first an empty slot
		    if (MusicGlobals->pSongsToPlay[count] == NULL)
			{
			    songSlot = count;
			    break;
			}
		}
	    if (songSlot != -1)
		{
		    // preroll song prior to start
		    theErr = GM_PrerollSong(theSong, theCallbackProc);
		    if (theErr == NO_ERR)
			{
			    // reconfigure reverb settings if desired
			    if (useEmbeddedMixerSettings)
				{
				    // Set reverb type now.
				    GM_SetReverbType(theSong->defaultReverbType);
				}
#if 0
			    theSong->mixLevel = theSong->averageVoiceUsage;
			    count = theSong->maxVoiceUsage;
			    autoLevel = TRUE;
#endif
			    // rebalance them mixer based upon load
			    if (autoLevel)
				{
				    count = PV_GetMaxVoicesPlayingFromAllSongs();
				    count += theSong->maxSongVoices;
				    count += theSong->maxEffectVoices;
				    if (count > MAX_VOICES)
					{
					    theSong->maxSongVoices = MAX_VOICES - theSong->maxEffectVoices;
					}
				    useEmbeddedMixerSettings = TRUE;	// force a reconfigure
				}
			    // reconfigure global mixer settings if desired
			    if (useEmbeddedMixerSettings)
				{
				    theErr = GM_ChangeSystemVoices(theSong->maxSongVoices,
								   theSong->mixLevel,
								   theSong->maxEffectVoices);
				}
			    // Start song playing now.
			    MusicGlobals->pSongsToPlay[songSlot] = theSong;
			}
		}
	    else
		{
		    theErr = TOO_MANY_SONGS_PLAYING;
		}
	}
    return theErr;
}


// Pause all songs
void GM_PauseSequencer(void)
{
    if ( (MusicGlobals->systemPaused == FALSE) && (MusicGlobals->sequencerPaused == FALSE) )
	{
	    MusicGlobals->sequencerPaused = TRUE;
	    // stop all MIDI notes
	    GM_EndAllNotes();
	}
}

// resume all songs
void GM_ResumeSequencer(void)
{
    if ( (MusicGlobals->systemPaused == FALSE) && (MusicGlobals->sequencerPaused) )
	{
	    MusicGlobals->sequencerPaused = FALSE;
	}
}

// Pause just this song
void GM_PauseSong(GM_Song *pSong)
{
    if (pSong)
	{
	    if (pSong->songPaused == FALSE)
		{
		    GM_Synth *theSynth;
		    pSong->songPaused = TRUE;

		    //GM_EndSongNotes(pSong);		// end just notes from this song

		    theSynth = pSong->pSynths;
		    while (theSynth)
			{
			    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
			    // the userReference.  however, someone should be able to!!
			    pSong->userReference = (void *)theSynth;
			    (theSynth->pProcessSongSoundOffProcPtr)(pSong);
			    theSynth = theSynth->pNext;
			}

		}
	}
}

// Resume just this song
void GM_ResumeSong(GM_Song *pSong)
{
    if (pSong)
	{
	    if (pSong->songPaused)
		{
		    pSong->songPaused = FALSE;
		}
	}
}

XBOOL GM_IsSongDone(GM_Song *pSong)
{
    register INT32	count;
    XBOOL		songDone;

    songDone = FALSE;
    if (pSong)
	{
	    songDone = TRUE;
	    for (count = 0; count < MAX_TRACKS; count++)
		{
		    if (pSong->trackon[count])
			{
			    songDone = FALSE;
			    break;
			}
		}
	}
    return songDone;
}

// process end song callback
static void PV_CallSongCallback(void *threadContext, GM_Song *theSong, XBOOL clearCallback)
{
    GM_SongCallbackProcPtr theCallback;

    if (theSong)
	{
	    theCallback = theSong->songEndCallbackPtr;
	    if (theCallback)
		{
		    if (clearCallback)
			{
			    theSong->songEndCallbackPtr = NULL;
			}
		    (*theCallback)(threadContext, theSong);
		}
	}
}


void GM_SetSongCallback(GM_Song *theSong, GM_SongCallbackProcPtr theCallback)
{
    if (MusicGlobals && theSong)
	{
	    theSong->songEndCallbackPtr = theCallback;
	}
}

void GM_SetSongTimeCallback(GM_Song *theSong, GM_SongTimeCallbackProcPtr theCallback, INT32 reference)
{
    if (MusicGlobals && theSong)
	{
	    theSong->songTimeCallbackReference = reference;
	    theSong->songTimeCallbackPtr = theCallback;
	}
}


// Call meta event callback if there
static void PV_CallSongMetaEventCallback(void *threadContext, GM_Song *pSong, char markerType, void *pText, INT32 textLength, short currentTrack)
{
    GM_SongMetaCallbackProcPtr theCallback;

    if (pSong)
	{
	    theCallback = pSong->metaEventCallbackPtr;
	    if (theCallback)
		{
		    (*theCallback)(threadContext, pSong, markerType, pText, textLength, currentTrack);
		}
	}
}

void GM_SetSongMetaEventCallback(GM_Song *theSong, GM_SongMetaCallbackProcPtr theCallback, INT32 reference)
{
    if (MusicGlobals && theSong)
	{
	    theSong->metaEventCallbackPtr = theCallback;
	    theSong->metaEventCallbackReference = reference;
	}
}

// removes this song from the list of playing songs
// neither sends callbacks, nor frees the song
// must be followed by GM_EndSong!
void GM_RemoveFromSongsToPlay(GM_Song *pSong) {
    register LOOPCOUNT count;

    if (pSong) {
	for (count = 0; count < MAX_SONGS; count++) {
	    if (MusicGlobals->pSongsToPlay[count] == pSong) {
		MusicGlobals->pSongsToPlay[count] = NULL;
		break;
	    }
	}
	QGM_ClearSongFromQueue(pSong);
    }
}


// Stop this song playing, or if NULL stop all songs playing
void GM_EndSong(void *threadContext, GM_Song *pSong)
{
    register LOOPCOUNT count;

    if (pSong)
	{
	    GM_Synth *theSynth;

	    //GM_EndSongNotes(pSong);		// end just notes associated with this song
	    theSynth = pSong->pSynths;
	    while (theSynth)
		{
		    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
		    // the userReference.  however, someone should be able to!!
		    pSong->userReference = (void *)theSynth;
		    (theSynth->pProcessSongSoundOffProcPtr)(pSong);
		    theSynth = theSynth->pNext;
		}

	    for (count = 0; count < MAX_SONGS; count++)
		{
		    if (MusicGlobals->pSongsToPlay[count] == pSong)
			{
			    MusicGlobals->pSongsToPlay[count] = NULL;
			    break;
			}
		}
	    for (count = 0; count < MAX_TRACKS; count++)
		{
		    pSong->ptrack[count] = NULL;
		    pSong->trackon[count] = TRACK_OFF;
		}
	    PV_CallSongCallback(threadContext, pSong, TRUE);
	    pSong->songFinished = TRUE;
	}
    else
	{
	    for (count = 0; count < MAX_SONGS; count++)
		{
		    if (MusicGlobals->pSongsToPlay[count])
			{
			    GM_EndSong(threadContext, MusicGlobals->pSongsToPlay[count]);
			}
		}
	    MusicGlobals->systemPaused = FALSE;
	    MusicGlobals->sequencerPaused = FALSE;
	}
}


#if X_PLATFORM != X_WEBTV
// This will return realtime information about the current set of notes being playing right now.
void GM_GetRealtimeAudioInformation(GM_AudioInfo *pInfo)
{
    register GM_Mixer	*pMixer;
    register GM_Voice	*pVoice;
    register LOOPCOUNT	count, active;

    pMixer = MusicGlobals;
    if (pMixer)
	{
	    active = 0;
	    for (count = 0; count < (pMixer->MaxNotes+pMixer->MaxEffects); count++)
		{
		    pVoice = &pMixer->NoteEntry[count];
		    if (pVoice->voiceMode != VOICE_UNUSED)
			{
			    pInfo->voice[active] = (INT16)count;
			    if (count > pMixer->MaxNotes)	// in the range of MaxNotes and MaxEffects?
				{
				    pInfo->voiceType[active] = SOUND_PCM_VOICE;
				}
			    else
				{
				    pInfo->voiceType[active] = MIDI_PCM_VOICE;
				}
			    pInfo->patch[active] = pVoice->NoteProgram;
			    pInfo->scaledVolume[active] = (INT16)pVoice->NoteVolume;
			    pInfo->volume[active] = pVoice->NoteMIDIVolume;
			    pInfo->channel[active] = pVoice->NoteChannel;
			    pInfo->midiNote[active] = pVoice->NoteMIDIPitch;
			    pInfo->pSong[active] = pVoice->pSong;
			    active++;
			}
		}
	    pInfo->voicesActive = (INT16)active;
	    pInfo->maxNotesAllocated = pMixer->MaxNotes;
	    pInfo->maxEffectsAllocated = pMixer->MaxEffects;
	    pInfo->mixLevelAllocated = pMixer->mixLevel;
	}
    else
	{
	    XSetMemory((void *)pInfo, (INT32)sizeof(GM_AudioInfo), 0);
	}
}
#endif	// X_PLATFORM != X_WEBTV


static void PV_CallControlCallbacks(void *threadContext, GM_Song *pSong, short int channel, short int track, short int controler, unsigned short value)
{
    GM_ControlCallbackPtr	pControlerCallBack;
    GM_ControlerCallbackPtr	callback;
    void					*reference;

    pControlerCallBack = pSong->controllerCallback;
    if (pControlerCallBack)
	{
	    callback = pControlerCallBack->callbackProc[controler];
	    reference = pControlerCallBack->callbackReference[controler];

	    if (callback)
		{	// call user function
		    (*callback)(threadContext, pSong, reference, channel, track, controler, value);
		}
	}
}

void GM_SetControllerCallback(GM_Song *theSong, void * reference, GM_ControlerCallbackPtr controllerCallback,
			      short int controller)
{
    GM_ControlCallbackPtr	pControlerCallBack;

    if ( (theSong) && (controller < MAX_CONTROLLERS) )
	{
	    pControlerCallBack = theSong->controllerCallback;
	    if (pControlerCallBack == NULL)
		{	// not allocated yet?
		    pControlerCallBack = (GM_ControlCallbackPtr)XNewPtr((INT32)sizeof(GM_ControlCallback));
		    theSong->controllerCallback = pControlerCallBack;
		}
	    if (pControlerCallBack)
		{
		    pControlerCallBack->callbackProc[controller] = controllerCallback;
		    pControlerCallBack->callbackReference[controller] = (void *)reference;
		}
	}
}

void GM_SetSongLoopFlag(GM_Song *theSong, XBOOL loopSong)
{
    if (MusicGlobals && theSong)
	{
	    theSong->loopSong = loopSong;
	}
}

void GM_SetSongLoopMax(GM_Song *theSong, short int maxLoopCount)
{
    if (MusicGlobals && theSong)
	{
	    theSong->songMaxLoopCount = maxLoopCount;
	}
}

short int GM_GetSongLoopMax(GM_Song *theSong)
{
    short int	maxLoopCount;

    maxLoopCount = 0;
    if (MusicGlobals && theSong)
	{
	    maxLoopCount = theSong->songMaxLoopCount;
	}
    return maxLoopCount;
}

XBOOL GM_GetSongLoopFlag(GM_Song *theSong)
{
    if (MusicGlobals && theSong)
	{
	    return (XBOOL)theSong->loopSong;
	}
    return FALSE;
}

// set volume of a channel of a current song. If updateNow is active and the song is playing
// the voice will up updated
void GM_SetChannelVolume(GM_Song *theSong, short int channel, short int volume, XBOOL updateNow)
{
    if ((volume >= 0) && (volume < MAX_NOTE_VOLUME))
	{
	    if ((channel >= 0) && (channel < MAX_CHANNELS))
		{
		    theSong->channelVolume[channel] = (unsigned char)volume;
		    if (updateNow)
			{
			    if (theSong->AnalyzeMode == SCAN_NORMAL)
				{
				    SetChannelVolume(theSong, channel, volume);
				}
			}
		}
	}
}

// Given a song and a channel, this will return the current volume level
short int GM_GetChannelVolume(GM_Song *theSong, short int channel)
{
    if ((channel >= 0) && (channel < MAX_CHANNELS))
	{
	    return theSong->channelVolume[channel];
	}
    return 0;
}

// Set song volume. Range is 0 to 127. You can overdrive
void GM_SetSongVolume(GM_Song *theSong, INT16 newVolume)
{
    register GM_Mixer		*pMixer;
    register LOOPCOUNT		count;
    register GM_Voice		*theNote;

    TRACE_STR("> GM_SetSongVolume\n");
    pMixer = MusicGlobals;
    if (theSong && pMixer)
	{
	    if (newVolume < 0)
		{
		    newVolume = 0;
		}
	    if (newVolume > MAX_NOTE_VOLUME * 5)
		{
		    newVolume = MAX_NOTE_VOLUME * 5;
		}
	    theSong->songVolume = newVolume;

	    // update the current notes playing to the new volume
	    for (count = 0; count < pMixer->MaxNotes; count++)
		{
		    theNote = &pMixer->NoteEntry[count];
		    if (theNote->voiceMode != VOICE_UNUSED)
			{
			    if (theNote->pSong == theSong)
				{
				    // make sure and set the channel volume not scaled, because its scaled later
				    if (newVolume == 0)
					{
					    theNote->voiceMode = VOICE_RELEASING;
					    theNote->NoteDecay = 0;
					    theNote->volumeADSRRecord.ADSRTime[0] = 1;
					    theNote->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
					    theNote->volumeADSRRecord.ADSRLevel[0] = 0;	// just in case
					}
				    // now calculate the new volume based upon the current channel volume and
				    // the unscaled note volume
				    newVolume = (INT16)PV_ScaleVolumeFromChannelAndSong(theNote->pSong, theNote->NoteChannel, theNote->NoteMIDIVolume);
				    //CLS: Do we not want a 32-bit intermediate value here?
				    newVolume = (INT16)((newVolume * MusicGlobals->scaleBackAmount) >> 8);
				    theNote->NoteVolume = newVolume;
				}
			}
		}
	}
    TRACE_STR("< GM_SetSongVolume\n");
}

// Get song volume
INT16 GM_GetSongVolume(GM_Song *theSong)
{
    INT16	volume;

    volume = 0;
    if (theSong)
	{
	    volume = theSong->songVolume;
	}
    return volume;
}

// set song position. Range is MAX_PAN_LEFT to MAX_PAN_RIGHT
void GM_SetSongStereoPosition(GM_Song *theSong, INT16 newStereoPosition)
{
    if (theSong)
	{
	    if ( (newStereoPosition >= MAX_PAN_LEFT) && (newStereoPosition <= MAX_PAN_RIGHT) )
		{
		    theSong->songMasterStereoPlacement = newStereoPosition;
		}
	}
}

// get song position. Range is MAX_PAN_LEFT to MAX_PAN_RIGHT
INT16 GM_GetSetStereoPosition(GM_Song *theSong)
{
    INT16	songMasterStereoPlacement;

    songMasterStereoPlacement = 0;
    if (theSong)
	{
	    songMasterStereoPlacement = theSong->songMasterStereoPlacement;
	}
    return songMasterStereoPlacement;
}

OPErr GM_SetMasterSongTempo(GM_Song *pSong, XFIXED newTempo)
{
    if (pSong)
	{
	    pSong->MasterTempo = newTempo;
	    PV_ScaleDivision(pSong, pSong->UnscaledMIDIDivision);	// reflect the change now
	}
    return NO_ERR;
}

XFIXED GM_GetMasterSongTempo(GM_Song *pSong)
{
    XFIXED	tempo;

    tempo = 0;
    if (pSong)
	{
	    tempo = pSong->MasterTempo;
	}
    return tempo;
}


/*
beats | 1 min | 1 S         | 1 mili S           beats
------|-------|-------------|-------------- =  ---------
min   | 60 S  | 1000 mili S | 1000 micro S      micro S

Take the inverse and you've basically got (micro S / beat)
or (60000000 / tempo)

To get beats per minute do 60000000 / tempo
*/

// returns tempo in microsecond per quarter note
UINT32 GM_GetSongTempo(GM_Song *pSong)
{
    UINT32	result;

    result = 0;
    if (pSong)
	{
	    result = (UINT32)pSong->UnscaledMIDITempo;
	}
    return result;
}

// Sets tempo in microsecond per quarter note
void GM_SetSongTempo(GM_Song *pSong, UINT32 newTempo)
{
    if (pSong && newTempo)
	{
	    pSong->UnscaledMIDITempo = (UFLOAT)newTempo;
#if USE_HAE_EXTERNAL_API == TRUE
	    pSong->MIDITempo = (pSong->UnscaledMIDITempo / (UFLOAT)HAE_GetSliceTimeInMicroseconds());
#else
	    pSong->MIDITempo = (pSong->UnscaledMIDITempo / (UFLOAT)BUFFER_SLICE_TIME);
#endif
	    PV_ScaleDivision(pSong, pSong->UnscaledMIDIDivision);
	}
}

// returns tempo in beats per minute
UINT32 GM_GetSongTempoInBeatsPerMinute(GM_Song *pSong)
{
    UINT32	result;

    result = GM_GetSongTempo(pSong);
    if (result)
	{
	    result = 60000000L / result;
	}
    return result;
}

// sets tempo in beats per minute
void GM_SetSongTempInBeatsPerMinute(GM_Song *pSong, UINT32 newTempoBPM)
{
    if ( (newTempoBPM) && (newTempoBPM < 500) )
	{
	    GM_SetSongTempo(pSong, 60000000L / newTempoBPM);
	}
}

// Set channel reverb amount
static void PV_SetChannelReverb(GM_Song *pSong, short int the_channel, UBYTE reverbAmount)
{
    register GM_Mixer		*pMixer;
    register GM_Voice		*pVoice;
    register LOOPCOUNT		count;

    pMixer = MusicGlobals;
    // update the current notes playing to the new reverb
    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    pVoice = &pMixer->NoteEntry[count];
	    if ((pVoice->voiceMode != VOICE_UNUSED) && (pVoice->pSong == pSong))
		{
		    if (pVoice->NoteChannel == the_channel)
			{
			    // Only enable if past verb threshold
			    if (reverbAmount > GM_GetReverbEnableThreshold())
				{
				    pVoice->avoidReverb = FALSE;		// enable full on
				}
			    else
				{
				    pVoice->avoidReverb = TRUE;
				}
			    pVoice->reverbLevel = reverbAmount;
			}
		}
	}
}

// set reverb of a channel of a current song. If updateNow is active and the song is playing
// the voice will up updated
void GM_SetChannelReverb(GM_Song *theSong, short int channel, UBYTE reverbAmount, XBOOL updateNow)
{
    if (reverbAmount < MAX_NOTE_VOLUME)
	{
	    if ((channel >= 0) && (channel < MAX_CHANNELS))
		{
		    theSong->channelReverb[channel] = reverbAmount;
		    if (updateNow)
			{
			    if (theSong->AnalyzeMode == SCAN_NORMAL)
				{
				    PV_SetChannelReverb(theSong, channel, reverbAmount);
				}
			}
		}
	}
}

// Given a song and a channel, this will return the current reverb level
short int GM_GetChannelReverb(GM_Song *theSong, short int channel)
{
    if ((channel >= 0) && (channel < MAX_CHANNELS))
	{
	    return theSong->channelReverb[channel];
	}
    return 0;
}

static INLINE XBOOL PV_IsSoloTrackActive(GM_Song *pSong)
{
    short int	count, size;
    XBOOL		active;

    active = FALSE;
    size = (short int)(sizeof(pSong->soloTrackMuted) / sizeof(UINT32));
    for (count = 0; count < size; count++)
	{
	    if (pSong->soloTrackMuted[count])
		{
		    active = TRUE;
		    break;
		}
	}
    return active;
}

static INLINE XBOOL PV_IsSoloChannelActive(GM_Song *pSong)
{
    short int	count, size;
    XBOOL		active;

    active = FALSE;
    size = (short int)(sizeof(pSong->soloChannelMuted) / sizeof(UINT16));
    for (count = 0; count < size; count++)
	{
	    if (pSong->soloChannelMuted[count])
		{
		    active = TRUE;
		    break;
		}
	}
    return active;
}

// test to see if a channel or track is muted
/*static*/ XBOOL PV_IsMuted(GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack)
{
    char	channel, track;

    // test channel
    channel = (XTestBit(&pSong->channelMuted, MIDIChannel) == FALSE);
    if (PV_IsSoloChannelActive(pSong))
	{
	    channel &= (XTestBit(&pSong->soloChannelMuted, MIDIChannel));
	}

    // test track, if valid
    if ((currentTrack >= 0) && (currentTrack < MAX_TRACKS))	// valid track?
	{
	    track = (XTestBit(&pSong->trackMuted, currentTrack) == FALSE);
	    if (PV_IsSoloTrackActive(pSong))
		{
		    track &= (XTestBit(&pSong->soloTrackMuted, currentTrack));
		}
	}
    else
	{
	    track = TRUE;
	}

    if (channel && track)	// track muted?
	{
	    return FALSE;		// no
	}
    return TRUE;			// yes
}

#if 0 && USE_CREATION_API == TRUE
static void PV_AddInstrumentEntry(GM_Song *pSong, INT16 currentTrack, INT16 MIDIChannel, INT16 program, INT16 bank)
{
    InstrumentEntry	*e;
    InstrumentEntry *entry, *prev;
    UINT32			ticks;

    if (pSong->pPatchInfo)
	{
	    entry = pSong->pPatchInfo->instrChangeInfo;
	    prev = NULL;
	    ticks = pSong->trackcumuticks[currentTrack];
	    e = (InstrumentEntry *)XNewPtr((INT32)sizeof(InstrumentEntry));
	    if (e)
		{
		    while (entry)
			{
			    if (entry->track > currentTrack)
				{
				    break;
				}
			    else if (entry->track == currentTrack && ticks < entry->ticks)
				{
				    break;
				}
			    prev = entry;
			    entry = entry->next;
			}
		    if (!prev)
			{
			    e->next = pSong->pPatchInfo->instrChangeInfo;
			    pSong->pPatchInfo->instrChangeInfo = e;
			}
		    else
			{
			    e->next = prev->next;
			    prev->next = e;
			}
		    e->track = currentTrack;
		    e->channel = MIDIChannel;
		    e->program = program;
		    e->bank = bank;
		    e->ptr = pSong->pPatchInfo->instrPtrLoc;
		    e->ticks = ticks;
		    e->dirty = 0;
		    e->bankMode = pSong->channelBankMode[MIDIChannel];
		    e->bsPtr = pSong->pPatchInfo->bankPtrLoc;
		    pSong->pPatchInfo->bankPtrLoc = 0; // so comparisons will work
		}
	}
}
#endif	//	USE_CREATION_API

#if 0 && USE_CREATION_API == TRUE
void PV_InsertBankSelect(GM_Song *pSong, short channel, short currentTrack)
{
    UBYTE	*pp;
    INT32	size, sizeToMove, trackLength;
    short	track;

    if (pSong->pPatchInfo)
	{
	    pp = pSong->pPatchInfo->instrPtrLoc;
	    if (pp)
		{
		    size = XGetPtrSize(pSong->midiData);

		    sizeToMove = size - ((UBYTE *)(pp - 1) - ((UBYTE *)pSong->midiData));
		    sizeToMove -= 4;
		    XBlockMove(pp-1,pp + 3,sizeToMove);
		    pp[-1] = 0xB0 + channel; // controller
		    pp[0] = 0; // bank select
		    pp[1] = pSong->channelBank[channel]; // the bank
		    pp[2] = 0; // delta time between events
		    pSong->midiSize += 4;
		    pSong->pPatchInfo->bankPtrLoc = pp + 1;
		    pSong->pPatchInfo->instrPtrLoc += 4;
		    // now go through and increment any track start pointers whose start is > instrPtrLoc
		    for (track = 0; track < MAX_TRACKS; track++)
			{
			    if (pSong->ptrack[track] > pp)
				{
				    pSong->ptrack[track] += 4;
				}
			    if (pSong->trackstart[track] > pp)
				{
				    pSong->trackstart[track] += 4;
				}
			}
		    // finally go and update the length of this track in the header
		    pSong->tracklen[currentTrack] += 4;
		    pp = pSong->trackstart[currentTrack];
		    // read the entire value, increment, and then write it back
		    pp -= 4; // go back to the length part of the track header
		    trackLength = *pp;
		    trackLength = (trackLength << 8) + *(pp+1);
		    trackLength = (trackLength << 8) + *(pp+2);
		    trackLength = (trackLength << 8) + *(pp+3);
		    trackLength += 4;
		    *pp = (trackLength >> 24) & 0xFF;
		    *(pp+1) = (trackLength >> 16) & 0xFF;
		    *(pp+2) = (trackLength >> 8) & 0xFF;
		    *(pp+3) = trackLength & 0xFF;
		}
	}
}
#endif	// USE_CREATION_API

#if 0 && USE_CREATION_API == TRUE
void PV_FreePatchInfo(GM_Song *pSong)
{
    InstrumentEntry *entry,*prev;

    if (pSong->pPatchInfo)
	{
	    entry = pSong->pPatchInfo->instrChangeInfo;
	    while (entry)
		{
		    prev = entry;
		    entry = entry->next;
		    XDisposePtr(prev);
		}
	    pSong->pPatchInfo->instrChangeInfo = 0;
	    XDisposePtr(pSong->pPatchInfo);
	    pSong->pPatchInfo = NULL;
	}
}
#endif	// USE_CREATION_API

// Process midi program change
/*static*/ void PV_ProcessProgramChange(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 program)
{
    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    if (pSong->allowProgramChanges)
		{
		    if (MIDIChannel == PERCUSSION_CHANNEL)
			{
			    // only change the percussion program if we're not in perc mode
			    if (pSong->defaultPercusionProgram > 0)
				{
				    program = pSong->defaultPercusionProgram;
				}
			}
		    pSong->channelProgram[MIDIChannel] = program;
		}

	    if (pSong->AnalyzeMode)
		{
		    // if analyzing, note the program or the channel
		    if (pSong->allowProgramChanges == FALSE)
			{
			    program = MIDIChannel;
			}

		    if (pSong->firstChannelProgram[MIDIChannel] == -1)
			{	// first time only
			    pSong->firstChannelProgram[MIDIChannel] = program;
			    pSong->firstChannelBank[MIDIChannel] = pSong->channelBank[MIDIChannel];
			}

		    if (MIDIChannel == PERCUSSION_CHANNEL)
			{
			    // only change the percussion program if we're not in perc mode
			    if (pSong->defaultPercusionProgram > 0)
				{
				    program = pSong->defaultPercusionProgram;
				}
			}
		    pSong->channelProgram[MIDIChannel] = program;
#if 0 && USE_CREATION_API == TRUE
		    if (pSong->AnalyzeMode == SCAN_FIND_PATCHES)
			{
			    // move data over. instrPtrLoc - 1 + 4
			    // not caring how big the file is, we move total size - 4 up 4
			    // this will always work
			    // plus it moves the
			    if ((INT32)(pSong->pPatchInfo->instrPtrLoc - pSong->pPatchInfo->bankPtrLoc) > 3)
				{
				    PV_InsertBankSelect(pSong,MIDIChannel,currentTrack);
				    pSong->pPatchInfo->streamIncrement = 4;
				}
			    else
				{
				    pSong->pPatchInfo->streamIncrement = 0;
				}
			    PV_AddInstrumentEntry(pSong,currentTrack,MIDIChannel,program,pSong->channelBank[MIDIChannel]);
			}
		    if (pSong->AnalyzeMode == SCAN_COUNT_PATCHES)
			{
			    // this is a total hack. but what else to do? figure out if the most recent
			    // event was a bank change event by comparing pointers! 3 is the distance
			    // between the pointers if the delta time was very small. Since we write dt 0
			    // between controller and pgm when modifying the file, this will at least
			    // find our handiwork
			    if ((INT32)(pSong->pPatchInfo->instrPtrLoc - pSong->pPatchInfo->bankPtrLoc) > 3)
				{
				    pSong->pPatchInfo->instrCount++;
				}
			    pSong->pPatchInfo->bankPtrLoc = 0;
			    if ((*(pSong->pPatchInfo->instrPtrLoc - 1) & 0xF0) != 0xC0)
				{
				    pSong->pPatchInfo->rsCount++;
				}
			}
#endif	// USE_CREATION_API
		}
	}
}

static INT16 PV_ConvertPatchBank(GM_Song *pSong, INT16 thePatch, INT16 theChannel)
{
    INT16 theBank;

    theBank = pSong->channelBank[theChannel];
    switch (pSong->channelBankMode[theChannel])
	{
	default:
	    // this is default behavior
	    // normal bank for channels 1-9 and 11-16
	    // percussion for channel 10
	    // perc: 1 3 5 7, etc
	    // inst: 0 2 4 6, etc
	case USE_GM_DEFAULT:
	    if (theChannel == PERCUSSION_CHANNEL)
		{
		    theBank = (theBank * 2) + 1;		// odd banks are percussion
		}
	    else
		{
		    theBank = theBank * 2 + 0;		// even banks are for instruments
		}

	    if (theBank < MAX_BANKS)
		{
		    thePatch = (theBank * 128) + thePatch;
		}
	    break;
	case USE_NON_GM_PERC_BANK:
	    // will force the use of the percussion
	    // bank reguardless of channel
	case USE_GM_PERC_BANK:
	    theBank = (theBank * 2) + 1;		// odd banks are percussion
	    if (theBank < MAX_BANKS)
		{
		    thePatch = (theBank * 128) + thePatch;
		}
	    break;
	    // will force the use of the normal
	    // bank reguardless of channel
	case USE_NORM_BANK:
	    theBank = theBank * 2 + 0;		// even banks are for instruments
	    if (theBank < MAX_BANKS)
		{
		    thePatch = (theBank * 128) + thePatch;
		}
	    break;
	}
    return thePatch;
}

// Given a song and a midi note, this will determine the instrument to use based upon the percussion mode,
// bank selectable mode, and other factors
static INT16 PV_DetermineInstrumentToUse(GM_Song *pSong, INT16 midiNote, INT16 MIDIChannel)
{
    INT16	thePatch;

    thePatch = 0;
    if (pSong->defaultPercusionProgram < 0)		// in GM mode?
	{
	    switch (pSong->channelBankMode[MIDIChannel])
		{
		case USE_GM_DEFAULT:
		    if (MIDIChannel == PERCUSSION_CHANNEL)
			{	// normal GM percusion behavior
			    thePatch = PV_ConvertPatchBank(pSong, midiNote, MIDIChannel);
			}
		    else
			{	// process as normal
			    thePatch = PV_ConvertPatchBank(pSong, pSong->channelProgram[MIDIChannel], MIDIChannel);
			}
		    break;
		case USE_NON_GM_PERC_BANK:
		case USE_NORM_BANK:
		    thePatch = PV_ConvertPatchBank(pSong, pSong->channelProgram[MIDIChannel], MIDIChannel);
		    break;
		case USE_GM_PERC_BANK:
		    thePatch = PV_ConvertPatchBank(pSong, midiNote, MIDIChannel);
		    break;
		}
	}
    else
	{
	    thePatch = pSong->channelProgram[MIDIChannel];
	}
    return thePatch;
}

// Process note off
/*static*/ void PV_ProcessNoteOff(void *threadContext, GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume)
{
    register INT16		thePatch;

    volume = volume;	// not used
    currentTrack = currentTrack;

    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    if (pSong->AnalyzeMode == SCAN_NORMAL)
		{
		    if (XTestBit(&pSong->allowPitchShift, MIDIChannel))
			{
			    note += pSong->songPitchShift;
			}
		    thePatch = PV_DetermineInstrumentToUse(pSong, note, MIDIChannel);
		    StopMIDINote(pSong, thePatch, MIDIChannel, currentTrack, note);
		}
	    else
		{
		    // if pedal is on, put note into SUS_ON_NOTE_OFF, and don't kill note
		    if (pSong->channelSustain[MIDIChannel])
			{
			    pSong->voiceSustain++;
			}
		    else
			{
			    pSong->voiceCount--;
			}
		    if (pSong->firstChannelProgram[MIDIChannel] != -1)
			{
			    thePatch = PV_DetermineInstrumentToUse(pSong, note, MIDIChannel);
			    GM_SetUsedInstrument(pSong, thePatch, note, TRUE);		// mark note in instrument
			}
		}
	}
}

// Process note on
/*static*/  void PV_ProcessNoteOn(void *threadContext, GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume)
{
    register INT16		thePatch;

    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    if (volume)
		{
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    if (XTestBit(&pSong->allowPitchShift, MIDIChannel))
				{
				    note += pSong->songPitchShift;
				}
			    thePatch = PV_DetermineInstrumentToUse(pSong, note, MIDIChannel);
			    ServeMIDINote(pSong, thePatch, MIDIChannel, currentTrack, note, volume);
			}
		    else
			{
			    pSong->voiceCount++;

			    if (pSong->allowProgramChanges == FALSE)
				{
				    // if analyzing, note the channel. This is required in case program changes have been turned off
				    // and there are no program changes before the first note.
				    if (pSong->firstChannelProgram[MIDIChannel] == -1)
					{	// first time only
					    pSong->firstChannelProgram[MIDIChannel] = MIDIChannel;
					}
				    GM_SetUsedInstrument(pSong, MIDIChannel, note, TRUE);		// mark instrument
				}
			    else
				{
				    if (pSong->firstChannelProgram[MIDIChannel] != -1)
					{
					    thePatch = PV_DetermineInstrumentToUse(pSong, note, MIDIChannel);
					    GM_SetUsedInstrument(pSong, thePatch, note, TRUE);		// mark note in instrument
					}
				}
			}
		}
	    else
		{
		    PV_ProcessNoteOff(threadContext, pSong, MIDIChannel, currentTrack, note, volume);
		}
	}
}

// Process pitch bend
/*static*/ void PV_ProcessPitchBend(void *threadContext, GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, UBYTE valueMSB, UBYTE valueLSB)
{
    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    if ( (pSong->AnalyzeMode == SCAN_NORMAL) || (pSong->AnalyzeMode == SCAN_DETERMINE_LENGTH) )
		{
		    // This solves (!) a bug with pitch bend. I don't know what it is right now. You can't pitch percussion at all
		    // with the GM percussion set
		    if (pSong->defaultPercusionProgram < 0)		// in GM mode?
			{
			    if (MIDIChannel != PERCUSSION_CHANNEL)
				{
				    // change the current channel bends for new notes
				    pSong->channelBend[MIDIChannel] = SetChannelPitchBend(pSong, MIDIChannel,
											  pSong->channelPitchBendRange[MIDIChannel], valueMSB, valueLSB);
				}
			}
		    else
			{
			    // change the current channel bends for new notes
			    pSong->channelBend[MIDIChannel] = SetChannelPitchBend(pSong, MIDIChannel,
										  pSong->channelPitchBendRange[MIDIChannel], valueMSB, valueLSB);
			}
		}
	}
}


// process Registered parameters and Non Registered parameters based upon the current
// channels status
static void PV_ProcessRegisteredParameters(GM_Song *pSong, INT16 MIDIChannel, UINT16 value)
{
    UBYTE	valueLSB, valueMSB;
    XBOOL	changePercProgram;

    changePercProgram = FALSE;
    switch (pSong->channelWhichParameter[MIDIChannel])
	{
	    // GM Registered parameters
	case USE_RPN:
	    valueLSB = pSong->channelRegisteredParameterLSB[MIDIChannel];
	    valueMSB = pSong->channelRegisteredParameterMSB[MIDIChannel];
	    switch ((valueMSB * 128) + valueLSB)
		{
		case 0:		// set pitch bend range in half steps
		    pSong->channelPitchBendRange[MIDIChannel] = (UBYTE)value;
		    break;
		case 1:		// Fine Tuning
		case 2:		// Coarse Tuning
		    break;
		}
	    // reset
	    pSong->channelRegisteredParameterLSB[MIDIChannel] = -1;
	    pSong->channelRegisteredParameterMSB[MIDIChannel] = -1;
	    break;
	    // Non registered parameters
	case USE_NRPN:
	    valueLSB = pSong->channelNonRegisteredParameterLSB[MIDIChannel];
	    valueMSB = pSong->channelNonRegisteredParameterMSB[MIDIChannel];
	    switch ((valueMSB * 128) + valueLSB)
		{
		case (1 * 128) + 8:		// Vibrato rate
		    break;
		case (1 * 128) + 9:		// Vibrato depth
		    break;
		case (1 * 128) + 10:	// Vibrato delay
		    break;
		case (1 * 128) + 33:	// Resonance Freq
		    break;
		case (1 * 128) + 32:	// Cutoff Freq
		    break;
		case (4 * 128) + 0:		// curve control
		    pSong->velocityCurveType = (UBYTE)value;
		    break;
		case (5 * 128) + 0:		// patch/bank control
		    switch (value)
			{
			case 0:
			    // enable GM normal behavior for channels
			    pSong->channelBankMode[MIDIChannel] = USE_GM_DEFAULT;
			    changePercProgram = TRUE;
			    break;
			case 1:
			    // disable GM percussion and allow normal program changes
			    pSong->channelBankMode[MIDIChannel] = USE_NON_GM_PERC_BANK;
			    changePercProgram = TRUE;
			    break;
			case 2:
			    pSong->channelBankMode[MIDIChannel] = USE_GM_PERC_BANK;
			    changePercProgram = TRUE;
			    break;
			case 3:
			    pSong->channelBankMode[MIDIChannel] = USE_NORM_BANK;
			    changePercProgram = TRUE;
			    break;
			}
		    break;
		}

	    if (changePercProgram)
		{
		    if (pSong->AnalyzeMode != SCAN_NORMAL)
			{
			    if (pSong->firstChannelProgram[MIDIChannel] == -1)
				{
				    pSong->firstChannelProgram[MIDIChannel] = 0;
				    pSong->firstChannelBank[MIDIChannel] = 0;
				}
			}
		}

	    // reset
	    pSong->channelNonRegisteredParameterLSB[MIDIChannel] = -1;
	    pSong->channelNonRegisteredParameterMSB[MIDIChannel] = -1;
	    break;
	}
    pSong->channelWhichParameter[MIDIChannel] = USE_NO_RP;
}

// Process midi controlers
/*static*/ void PV_ProcessController(void *threadContext, GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 controler, UINT16 value)
{
    UBYTE	valueLSB, valueMSB;
    INT16	valueB;

    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    switch (controler)
		{
		    //			case 32:	// bank select MSB. This is GS.
		    //				break;
		case 0:		// bank select LSB.
		    if (value > (MAX_BANKS/2))
			{	// if we're selecting outside of our range, default to 0
			    value = 0;
			}
		    pSong->channelBank[MIDIChannel] = (SBYTE)value;
		    break;

		case 98:		// non registered parameter numbers LSB
		    pSong->channelNonRegisteredParameterLSB[MIDIChannel] = (SBYTE)value;
		    pSong->channelWhichParameter[MIDIChannel] = USE_NRPN;
		    break;
		case 99:		// non registered parameter numbers MSB
		    pSong->channelNonRegisteredParameterMSB[MIDIChannel] = (SBYTE)value;
		    pSong->channelWhichParameter[MIDIChannel] = USE_NRPN;
		    break;

		case 100:		// registered parameter numbers LSB
		    pSong->channelRegisteredParameterLSB[MIDIChannel] = (SBYTE)value;
		    pSong->channelWhichParameter[MIDIChannel] = USE_RPN;
		    break;
		case 101:		// registered parameter numbers MSB
		    pSong->channelRegisteredParameterMSB[MIDIChannel] = (SBYTE)value;
		    pSong->channelWhichParameter[MIDIChannel] = USE_RPN;
		    break;
		case 96:	// incecrement data entry
		    switch (pSong->channelWhichParameter[MIDIChannel])
			{
			case USE_RPN:
			    valueLSB = pSong->channelRegisteredParameterLSB[MIDIChannel];
			    valueMSB = pSong->channelRegisteredParameterMSB[MIDIChannel];
			    valueB = (valueMSB * 128) + valueLSB;
			    valueB += value;
			    pSong->channelRegisteredParameterLSB[MIDIChannel] = valueB % 128;
			    pSong->channelRegisteredParameterMSB[MIDIChannel] = valueB / 128;
			    break;
			case USE_NRPN:
			    valueLSB = pSong->channelNonRegisteredParameterLSB[MIDIChannel];
			    valueMSB = pSong->channelNonRegisteredParameterMSB[MIDIChannel];
			    valueB = (valueMSB * 128) + valueLSB;
			    valueB += value;
			    pSong->channelNonRegisteredParameterLSB[MIDIChannel] = valueB % 128;
			    pSong->channelNonRegisteredParameterMSB[MIDIChannel] = valueB / 128;
			    break;
			}
		    PV_ProcessRegisteredParameters(pSong, MIDIChannel, value);
		    break;
		case 97:	// decrement data entry
		    switch (pSong->channelWhichParameter[MIDIChannel])
			{
			case USE_RPN:
			    valueLSB = pSong->channelRegisteredParameterLSB[MIDIChannel];
			    valueMSB = pSong->channelRegisteredParameterMSB[MIDIChannel];
			    valueB = (valueMSB * 128) + valueLSB;
			    valueB -= value;
			    pSong->channelRegisteredParameterLSB[MIDIChannel] = valueB % 128;
			    pSong->channelRegisteredParameterMSB[MIDIChannel] = valueB / 128;
			    break;
			case USE_NRPN:
			    valueLSB = pSong->channelNonRegisteredParameterLSB[MIDIChannel];
			    valueMSB = pSong->channelNonRegisteredParameterMSB[MIDIChannel];
			    valueB = (valueMSB * 128) + valueLSB;
			    valueB -= value;
			    pSong->channelNonRegisteredParameterLSB[MIDIChannel] = valueB % 128;
			    pSong->channelNonRegisteredParameterMSB[MIDIChannel] = valueB / 128;
			    break;
			}
		    PV_ProcessRegisteredParameters(pSong, MIDIChannel, value);
		    break;
		    //			case 38:		// LSB data entry
		    //				break;
		case 6:			// MSB data entry for RPN controlers
		    PV_ProcessRegisteredParameters(pSong, MIDIChannel, value);
		    break;

		    //			case 33:		// Modulation MSB
		    //				break;
		case 1:			// Modulation LSB
		    pSong->channelModWheel[MIDIChannel] = (UBYTE)value;
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    SetChannelModWheel(pSong, MIDIChannel, value);
			}
		    break;
		    //			case 39:		// Volume change MSB
		    //				break;
		case 7:			// Volume change LSB
		    // make sure and set the channel volume not scaled, because its scaled later
		    pSong->channelVolume[MIDIChannel] = (UBYTE)value;
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    SetChannelVolume(pSong, MIDIChannel, value);
			}
		    break;
		    //			case 40:		// balance MSB
		    //				break;
		    //			case 8:			// balance LSB
		    //				break;

		    //			case 42:		// stereo pan MSB
		    //				break;
		case 10:		// stereo pan LSB
		    // store the original unmodified pan, but set the GM_Voice based upon pan curve
		    pSong->channelStereoPosition[MIDIChannel] = value;		// store original
		    SetChannelStereoPosition(pSong, MIDIChannel, value);
		    break;

		    //			case 43:		// expression MSB
		    //				break;
		case 11:		// expression LSB
		    pSong->channelExpression[MIDIChannel] = (UBYTE)value;
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    // calculate volume, which will include the new expression value
			    SetChannelVolume(pSong, MIDIChannel, pSong->channelVolume[MIDIChannel]);
			}
		    break;
		case 64:		// sustain
		    //  0-63 off, 64-127 on)
		    pSong->channelSustain[MIDIChannel] = (value > 63) ? TRUE : FALSE;
		    // stop sustaining note, set current notes to sustain
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    PV_ChangeSustainedNotes(pSong, MIDIChannel, value);
			}
		    else
			{
			    if (pSong->channelSustain[MIDIChannel] == FALSE)
				{
				    pSong->voiceCount -= pSong->voiceSustain;
				    pSong->voiceSustain = 0;
				}
			}
		    break;
		    //			case 67:		// soft
		    //				break;
		case 90:		// reverb type
		    GM_SetReverbType((ReverbMode)((value % MAX_REVERB_TYPES) + 1));
		    break;
		    //			case 71:		// XG harmonic		(gm2) When the value is smaller than 64, the effect becomes weaker. When the value is bigger than 64, the effect becomes stronger.
		    //			case 72:		// XG release time	(gm2) When the value is smaller than 64, the time becomes shorter. When the value is bigger than 64, the time becomes longer.
		    //			case 73:		// XG attack time	(gm2) When the value is smaller than 64, the time becomes shorter. When the value is bigger than 64, the time becomes longer.
		    //			case 74:		// XG brightness	(gm2) When the value is smaller than 64, the frequency becomes lower. When the value is bigger than 64, the frequency becomes higher.
		    //			case 75:		// Decay Time		(gm2) When the value is smaller than 64, the time becomes shorter. When the value is bigger than 64, the time becomes longer.
		    //			case 76:		// Vibrato Rate		(gm2) When the value is smaller than 64, the cycle becomes longer. When the value is bigger than 64, the cycle becomes shorter.
		    //			case 77:		// Vibrato Depth 	(gm2) When the value is smaller than 64, the depth becomes shallower. When the value is bigger than 64, the depth becomes deeper.
		    //			case 78:		// Vibrato Delay 	(gm2) When the value is smaller than 64, the delay time becomes shorter. When the value is bigger than 64, the delay time becomes longer.
		    //			case 110:		// filter attack time
		    //			case 111:		// filter decay time
		    //			case 112:		// filter sustain level
		    //			case 113:		// filter release time
		    //			case 114:		// filter env amount
		    //			case 115:		// osc 1 wave (0=saw, 1-127=square)
		    //			case 116:		// osc 2 wave (0=saw, 1-127=square)
		    //			case 117:		// detune course
		    //			case 118:		// detune fine (centered on 64)
		    //				break;
		case 91:		// amount of reverb
		case 94:		// amount of celest ( treat like reverb )
		    // set the channel reverb
		    pSong->channelReverb[MIDIChannel] = (UBYTE)value;
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    PV_SetChannelReverb(pSong, MIDIChannel, (UBYTE)value);
			}
		    break;
		case 93:		// chorus
		    pSong->channelChorus[MIDIChannel] = (UBYTE)value;
		    break;

		case 120:		// all sound off for a particular channel
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    // only a specific channel
			    GM_EndSongChannelNotes(pSong, MIDIChannel);
			}
		    break;
		case 121:		// Reset
		    // Don't support the in sequence feature to reset channel controlers.
		    // It seems that most sequences set the channels, then reset them! Ouch!
		    PV_ResetControlers(pSong, MIDIChannel, FALSE);
		    break;
		case 123:		// all notes off
		case 125:		// gm2
		    if (pSong->AnalyzeMode == SCAN_NORMAL)
			{
			    if (pSong->omniModeOn)
				{
				    // global
				    GM_EndSongNotes(pSong);	// put all notes into decay mode
				}
			    else
				{	// only a specific channel
				    GM_EndSongChannelNotes(pSong, MIDIChannel);
				}
			}
		    break;
		    //			case 126:	// mono mode on
		    //			case 127:	// poly mode on
		    //				break;
		}
	}


    if (pSong->AnalyzeMode == SCAN_NORMAL)
	{
	    // process special mute track controlers
	    switch (controler)
		{
		case 85:		// looping off (value = 0) otherwise its a max loop count
		    GM_SetSongLoopMax(pSong, value);
		    GM_SetSongLoopFlag(pSong, (XBOOL)(value ? TRUE : FALSE));
		    break;
		case 86:		// mute on loop count x
		    if (currentTrack != -1)
			{
			    if (pSong->songLoopCount == value)
				{
				    XSetBit(&pSong->trackMuted, currentTrack);
				}
			}
		    break;
		case 87:		// unmute on loop count x
		    if (currentTrack != -1)
			{
			    if (pSong->songLoopCount == value)
				{
				    XClearBit(&pSong->trackMuted, currentTrack);
				}
			}
		    break;
		}
	}
}

// get the current controller values from a GM_Song
char GM_GetControllerValue(GM_Song *pSong, INT16 channel, INT16 controller)
{
    char	value;

    value = 0;
    // Get channel based controler value
    switch (controller)
	{
	case 0:
	    // bank
	    value = pSong->channelBank[channel];
	    break;
	case 1:
	    // Mod wheel (primarily affects pitch bend)
	    value = pSong->channelModWheel[channel];
	    break;

	case 7:
	    // current channel volume
	    value = (char)GM_GetChannelVolume(pSong, channel);
	    break;

	case 10:
	    // current channel stereo position
	    value = (char)pSong->channelStereoPosition[channel];
	    break;

	case 11:
	    // current channel expression
	    value = pSong->channelExpression[channel];
	    break;

	case 64:
	    // sustain pedal on/off
	    //  0-63 off, 64-127 on)
	    value = (pSong->channelSustain[channel]) ? 0 : 127;
	    break;

	case 90:		// reverb type
	    value = (char)GM_GetReverbType() - 1;
	    break;
	case 91:
	    // current channel reverb
	    value = pSong->channelReverb[channel];
	    break;
	case 93:		// chorus
	    value = pSong->channelChorus[channel];
	    break;

	case 98:		// non registered parameter numbers LSB
	    value = pSong->channelNonRegisteredParameterLSB[channel];
	    break;
	case 99:		// non registered parameter numbers MSB
	    value = pSong->channelNonRegisteredParameterMSB[channel];
	    break;

	case 100:
	    // Registered Parameter most signifcant byte
	    value = pSong->channelRegisteredParameterMSB[channel];
	    break;

	case 101:
	    // Registered Parameter least signifcant byte
	    value = pSong->channelRegisteredParameterLSB[channel];
	    break;
	}

    // return the controller value
    return value;
}


// removes all events from queue that belong to the given song
void QGM_ClearSongFromQueue(GM_Song* pSong) {

    register Q_MIDIEvent* pEvent;

    pEvent = MusicGlobals->pTail;   // get current read event pointer
    while (MusicGlobals->pHead != pEvent) {
	if (pEvent->pSong == pSong) {
	    /* disable this event */
	    pEvent->pSong = NULL;
	    pEvent->command = 0;
	}
	// increment to the next one
	pEvent++;  
	// if we've overflowed then reset back to the begining
	if (pEvent > &MusicGlobals->theExternalMidiQueue[MAX_QUEUE_EVENTS-1]) {
	    pEvent = &MusicGlobals->theExternalMidiQueue[0];
	}
    }
}

// Get next event read only from the queue. If there's none left, return NULL
static Q_MIDIEvent* PV_GetNextReadOnlyQueueEvent(UINT32 ticks) {
    register Q_MIDIEvent	*pEvent;

    // any events?
    pEvent = NULL;
    if (MusicGlobals->pHead != MusicGlobals->pTail) {
	pEvent = MusicGlobals->pTail;   // get current read event pointer

	// do this comparison because timeStamp may roll over
	if ((INT32) (ticks - pEvent->timeStamp) > 0) {
	    MusicGlobals->pTail++;  // increment to the next one

	    // if we've overflowed then reset back to the begining
	    if (MusicGlobals->pTail > &MusicGlobals->theExternalMidiQueue[MAX_QUEUE_EVENTS-1]) {
		MusicGlobals->pTail = &MusicGlobals->theExternalMidiQueue[0];
	    }
	} else {
	    pEvent = NULL;
	}
    }
    return pEvent;
}

// Find an empty slot in the queue, timestamp it, and return a pointer
static Q_MIDIEvent * PV_GetNextStorableQueueEvent(UINT32 externalTimeStamp)
{
    register Q_MIDIEvent	*pEvent;

    if (externalTimeStamp == Q_GET_TICK)
	{
	    externalTimeStamp = GM_GetSyncTimeStamp();
	}
    pEvent = MusicGlobals->pHead;	// get current write event pointer
    MusicGlobals->pHead++;			// increment to the next one

    pEvent->timeStamp = externalTimeStamp;
    // if we've overflowed then reset back to the begining
    if (MusicGlobals->pHead > &MusicGlobals->theExternalMidiQueue[MAX_QUEUE_EVENTS-1])
	{
	    // NOTE: 	This might be a problem. If we've actaully gotten more events than what
	    //			is avaiable in the queue, currently the code will wrap and overwrite
	    //			and lose events. We need to deal with this in some way.
	    //			perhaps just doing pEvent = NULL will work, but it needs to be studied.
	    MusicGlobals->pHead = &MusicGlobals->theExternalMidiQueue[0];
	}
    return pEvent;
}

// Process a note on command. This will post a midi event into the midi event queue.
void QGM_NoteOn(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 note, INT16 velocity)
{
#if DISABLE_QUEUE
    timeStamp = timeStamp;
    GM_NoteOn(threadContext, pSong, channel, note, velocity);
#else
    register Q_MIDIEvent	* pEvent;

    pEvent = PV_GetNextStorableQueueEvent(timeStamp);
    if (pEvent) {
	pEvent->pSong = pSong;
	pEvent->midiChannel = (UBYTE)channel;
	pEvent->command = 0x90;
	pEvent->byte1 = (UBYTE)note;
	pEvent->byte2 = (UBYTE)velocity;
    }
#endif
}

// Process a note off command. This will post a midi event into the midi event queue.
void QGM_NoteOff(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 note, INT16 velocity)
{
#if DISABLE_QUEUE
    timeStamp = timeStamp;
    GM_NoteOff(threadContext, pSong, channel, note, velocity);
#else
    register Q_MIDIEvent	* pEvent;

    pEvent = PV_GetNextStorableQueueEvent(timeStamp);
    if (pEvent)
	{
	    pEvent->pSong = pSong;
	    pEvent->midiChannel = (UBYTE)channel;
	    pEvent->command = 0x80;
	    pEvent->byte1 = (UBYTE)note;
	    pEvent->byte2 = (UBYTE)velocity;
	}
#endif
}

// Process a program change command. This will post a midi event into the midi event queue.
void QGM_ProgramChange(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 program)
{
#if DISABLE_QUEUE
    timeStamp = timeStamp;
    GM_ProgramChange(threadContext, pSong, channel, program);
#else
    register Q_MIDIEvent	* pEvent;

    pEvent = PV_GetNextStorableQueueEvent(timeStamp);
    if (pEvent)
	{
	    pEvent->pSong = pSong;
	    pEvent->midiChannel = (UBYTE)channel;
	    pEvent->command = 0xC0;
	    pEvent->byte1 = (UBYTE)program;
	}
#endif
}

// Process a pitch bend command. This will post a midi event into the midi event queue.
void QGM_PitchBend(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, UBYTE valueMSB, UBYTE valueLSB)
{
#if DISABLE_QUEUE
    timeStamp = timeStamp;
    GM_PitchBend(threadContext, pSong, channel, valueMSB, valueLSB);
#else
    register Q_MIDIEvent	* pEvent;

    pEvent = PV_GetNextStorableQueueEvent(timeStamp);
    if (pEvent)
	{
	    pEvent->pSong = pSong;
	    pEvent->midiChannel = (UBYTE)channel;
	    pEvent->command = 0xE0;
	    pEvent->byte1 = valueMSB;
	    pEvent->byte2 = valueLSB;
	}
#endif
}

// Process a controller change command. This will post a midi event into the midi event queue.
void QGM_Controller(void *threadContext, GM_Song *pSong, UINT32 timeStamp, INT16 channel, INT16 controller, INT16 value)
{
#if DISABLE_QUEUE
    timeStamp = timeStamp;
    GM_Controller(threadContext, pSong, channel, controller, value);
#else
    register Q_MIDIEvent	* pEvent;

    pEvent = PV_GetNextStorableQueueEvent(timeStamp);
    if (pEvent)
	{
	    pEvent->pSong = pSong;
	    pEvent->midiChannel = (UBYTE)channel;
	    pEvent->command = 0xB0;
	    pEvent->byte1 = (UBYTE)controller;
	    pEvent->byte2 = (UBYTE)value;
	}
#endif
}

// $$kk: this -1 for the channel value has the potential to suck way hard.
// as far as i can tell, the -1 channel value actually does not get carried
// through and used; eventually GM_EndSongNotes gets called, and it is hard-
// coded to pass -1 to GM_EndNotes, which uses that value to mean "all channels"
// the -1 here doesn't make a difference that i can see at all....  in fact,
// QGM_Controller either casts it to a UBYTE (255) if the queue is being used
// or keeps the -1 value if not.  i am going to change it to 0.
void QGM_AllNotesOff(void *threadContext, GM_Song *pSong, UINT32 timeStamp)
{
    //QGM_Controller(threadContext, pSong, timeStamp, -1, 123, 0);		// issue a all notes off the queue
    QGM_Controller(threadContext, pSong, timeStamp, 0, 123, 0);		// issue a all notes off the queue
}

void QGM_LockExternalMidiQueue(void)
{
    MusicGlobals->processExternalMidiQueue++;
}

void QGM_UnlockExternalMidiQueue(void)
{
    MusicGlobals->processExternalMidiQueue--;
}

// Process a note on command.
void GM_NoteOn(void *threadContext, GM_Song *pSong, INT16 channel, INT16 note, INT16 velocity)
{
    if (pSong)
	{
	    // $$kk: 05.04.99
	    //PV_ProcessNoteOn(pSong, channel, -1, note, velocity);

	    GM_Synth *theSynth = pSong->pSynths;

	    while (theSynth)
		{
		    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
		    // the userReference.  however, someone should be able to!!
		    pSong->userReference = (void *)theSynth;
		    (theSynth->pNoteOnProcPtr)(threadContext, pSong, channel, -1, note, velocity);
		    theSynth = theSynth->pNext;
		}
	}
}

// Process a note off command.
void GM_NoteOff(void *threadContext, GM_Song *pSong, INT16 channel, INT16 note, INT16 velocity)
{
    if (pSong)
	{
	    // $$kk: 05.04.99
	    //PV_ProcessNoteOff(pSong, channel, -1, note, velocity);

	    GM_Synth *theSynth = pSong->pSynths;

	    while (theSynth)
		{
		    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
		    // the userReference.  however, someone should be able to!!
		    pSong->userReference = (void *)theSynth;
		    (theSynth->pNoteOffProcPtr)(threadContext, pSong, channel, -1, note, velocity);
		    theSynth = theSynth->pNext;
		}
	}
}

// Process a program change command.
void GM_ProgramChange(void *threadContext, GM_Song *pSong, INT16 channel, INT16 program)
{
    if (pSong)
	{
	    // $$kk: 05.04.99
	    //PV_ProcessProgramChange(pSong, channel, -1, program);

	    GM_Synth *theSynth = pSong->pSynths;

	    while (theSynth)
		{
		    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
		    // the userReference.  however, someone should be able to!!
		    pSong->userReference = (void *)theSynth;
		    (theSynth->pProgramChangeProcPtr)(threadContext, pSong, channel, -1, program);
		    theSynth = theSynth->pNext;
		}
	}
}

// Process a pitch bend command.
void GM_PitchBend(void *threadContext, GM_Song *pSong, INT16 channel, UBYTE valueMSB, UBYTE valueLSB)
{
    if (pSong)
	{
	    // $$kk: 05.04.99
	    //PV_ProcessPitchBend(pSong, channel, -1, valueMSB, valueLSB);

	    GM_Synth *theSynth = pSong->pSynths;

	    while (theSynth)
		{
		    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
		    // the userReference.  however, someone should be able to!!
		    pSong->userReference = (void *)theSynth;
		    (theSynth->pPitchBendProcPtr)(threadContext, pSong, channel, -1, valueMSB, valueLSB);
		    theSynth = theSynth->pNext;
		}
	}
}

// return pitch bend for channel
void GM_GetPitchBend(GM_Song *pSong, INT16 channel, unsigned char *pLSB, unsigned char *pMSB)
{
    register INT32			the_pitch_bend;

    if (pSong && pLSB && pMSB)
	{
	    *pLSB = 0;
	    *pMSB = 0;
	    if ((pSong->defaultPercusionProgram < 0) && (channel != PERCUSSION_CHANNEL))
		{
		    // Convert values from -8192 to 8192 to LSB & MSB values
		    the_pitch_bend = pSong->channelBend[channel] + 8192;
		    *pMSB = the_pitch_bend / 128;
		    *pLSB = the_pitch_bend & 0x7F;
		}
	}
}

// Process a controller change command.
void GM_Controller(void *threadContext, GM_Song *pSong, INT16 channel, INT16 controller, INT16 value)
{
    if (pSong)
	{
	    // $$kk: 05.04.99
	    //PV_ProcessController(pSong, channel, -1, controller, value);

	    GM_Synth *theSynth = pSong->pSynths;

	    while (theSynth)
		{
		    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
		    // the userReference.  however, someone should be able to!!
		    pSong->userReference = (void *)theSynth;
		    (theSynth->pProcessControllerProcPtr)(threadContext, pSong, channel, -1, controller, value);
		    theSynth = theSynth->pNext;
		}
	}
}

// $$kk: 07.20.99: this is not really like the MIDI AllNotesOff message 'cause
// it doesn't even take a channel value as a parameter.  i'm treating it more
// like a "cut off sound for this song" command
void GM_AllNotesOff(void *threadContext, GM_Song *pSong)
{
    if (pSong)
	{
	    // $$kk: 07.10.99
	    //GM_EndSongNotes(pSong);

	    GM_Synth *theSynth = pSong->pSynths;

	    while (theSynth)
		{
		    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
		    // the userReference.  however, someone should be able to!!
		    pSong->userReference = (void *)theSynth;
		    (theSynth->pProcessSongSoundOffProcPtr)(pSong);
		    theSynth = theSynth->pNext;
		}
	}
}

void PV_CleanExternalQueue(void)
{
    INT32		count;

    for (count = 0; count < MAX_QUEUE_EVENTS; count++)
	{
	    MusicGlobals->theExternalMidiQueue[count].timeStamp = 0;
	}
    // Setup a in pointer and a out pointer. Set them to the same thing at start
    MusicGlobals->pHead = &MusicGlobals->theExternalMidiQueue[0];
    MusicGlobals->pTail = &MusicGlobals->theExternalMidiQueue[0];
    MusicGlobals->processExternalMidiQueue = 0;
}

/* Process any events that have been place into the midi 
* event queue outside the normal process.
*
* This method is called even if the corresponding
* song is stopped, since those interactive MIDI events
* should always be delivered.
* Since MIDI events can be queued by time stamp in the
* queue, it is possible to put a lot of events in the queue
* in advance, while the virtual device (i.e. pSong) is closed.
* When those events (and especially their pSong field),
* are still evaluated, a crash is the result.
*
* $$fb I am unsure why the pSong parameter is passed
* to this function. Originally, probably, it was meant that 
* each pSong would have its own event queue (which would be
* a better design anyway), but as it turns out, there is only
* one queue for the entire mixer for interactive events.
* We should remove the pSong parameter, and call this
* method only once in PV_ProcessSequencerEvents (instead
* of once per song).
* 
* The crash is currently prevented by removing
* all events of the to-be-closed pSong from the event queue
* with the new function QGM_ClearSongFromQueue(GM_Song). This
* is done prior to freeing the pSong object.
*/
static void PV_ProcessExternalMIDIQueue(void *threadContext, GM_Song *pSong)
{
    register Q_MIDIEvent	*pEvent;
    register Q_MIDIEvent	event;
    register UINT32		ticks;
    register GM_Mixer		*pMixer;

    pMixer = MusicGlobals;

    if ((pMixer->processExternalMidiQueue == 0) && pSong) {
	ticks = GM_GetSyncTimeStamp();
	while ((pEvent = PV_GetNextReadOnlyQueueEvent(ticks)) != NULL) { // get next event
	    event = *pEvent;    // make local copy

	    if (event.pSong) {  // if the event needs to be in a specific song
		// use it
		pSong = event.pSong;
	    }
	    switch (event.command) {
		GM_Synth *theSynth;

	    case 0x90:	// •• Note On

		if (pSong->AnalyzeMode != SCAN_NORMAL) {
		    PV_ProcessNoteOn(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
		} else {

		    theSynth = pSong->pSynths;

		    while (theSynth) {
			// $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
			// the userReference.  however, someone should be able to!!
			pSong->userReference = (void *)theSynth;
			(theSynth->pNoteOnProcPtr)(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
			theSynth = theSynth->pNext;
		    }
		}
		break;
	    case 0x80: // •• Note Off

		if (pSong->AnalyzeMode != SCAN_NORMAL) {
		    PV_ProcessNoteOff(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
		} else {
		    theSynth = pSong->pSynths;

		    while (theSynth) {
			// $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
			// the userReference.  however, someone should be able to!!
			pSong->userReference = (void *)theSynth;
			(theSynth->pNoteOffProcPtr)(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
			theSynth = theSynth->pNext;
		    }
		}
		break;
	    case 0xB0:	// •• Control Change

		if (pSong->AnalyzeMode != SCAN_NORMAL) {
		    PV_ProcessController(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
		} else {
		    theSynth = pSong->pSynths;

		    while (theSynth) {
			// $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
			// the userReference.  however, someone should be able to!!
			pSong->userReference = (void *)theSynth;
			(theSynth->pProcessControllerProcPtr)(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
			theSynth = theSynth->pNext;
		    }
		}
		break;
	    case 0xC0:	// •• ProgramChange

		if (pSong->AnalyzeMode != SCAN_NORMAL) {
		    PV_ProcessProgramChange(threadContext, pSong, event.midiChannel, -1, event.byte1);
		} else {
		    theSynth = pSong->pSynths;

		    while (theSynth) {
			// $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
			// the userReference.  however, someone should be able to!!
			pSong->userReference = (void *)theSynth;
			(theSynth->pProgramChangeProcPtr)(threadContext, pSong, event.midiChannel, -1, event.byte1);
			theSynth = theSynth->pNext;
		    }
		}
		break;
	    case 0xE0:	// •• SetPitchBend

		if (pSong->AnalyzeMode != SCAN_NORMAL) {
		    PV_ProcessPitchBend(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
		} else {
		    theSynth = pSong->pSynths;

		    while (theSynth) {
			// $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
			// the userReference.  however, someone should be able to!!
			pSong->userReference = (void *)theSynth;
			(theSynth->pPitchBendProcPtr)(threadContext, pSong, event.midiChannel, -1, event.byte1, event.byte2);
			theSynth = theSynth->pNext;
		    }
		}
		break;
	    }
	}
    }
}

/*
Igor Meta events
FF 7F len data -	This is the standard sequencer specific data block. We can use it this way. Sequencers
ignore this data, but will not let you edit it in any way. I've tried other meta id's and
the results are the same.  Most don’t even display that its present. But it does allow
us to work with blocks of 8 bit data. The only way we can set it up for a sequencer
to copy and paste our block is to make it a sysex. Which means 7 bit bytes! Ug!

FF 7F len (this is a variable len)
Byte	Data Type
0-2		<Igor ID> This will be our MMA approved ID
3-6		4 byte command count
7-10	4 byte command ID for type of event. Message type:

FLUS - command
{
11		flush current instruments (default, you don’t need to send this)
}

CACH - command
{
11		cache instruments. Issuing this will cache instruments and they will not be flushed.
}

DATA - resource
{
11-14	Number of resources included in this meta event. You can have one event per data
block, or include all the resources in 	one event.


14-		The data following will be in a block format. First there will  be a 4 byte data type ID,
followed by a 4 byte ID, a pascal string (first byte is length, then data), and then a 4 byte
resource length, then the resource data. The length is only the length of the resource data.

Types:
snd( )	Standard Macintosh snd resource type. When building ALWAYS put snd's first
ID		ID reference number that is referenced in the INST resources
NAME	pascal string containing resource name
LENGTH
DATA

csnd	Standard Macintosh snd resource type that is compressed When building ALWAYS put snd's first
ID		ID reference number that is referenced in the INST resources
NAME	pascal string containing resource name
LENGTH
DATA

SONG	Igor Standard song performance resource
ID		Doesn’t matter. Should only be one song per midi file
NAME	pascal string containing resource name
LENGTH
DATA

INST	Igor Standard external instrument resource
ID		Patch number to replace
NAME	pascal string containing resource name
LENGTH
DATA
}
*/

static void PV_ProcessIgorResource(GM_Song *pSong, INT32 command, unsigned char *pMidiStream, INT32 theID, INT32 length)
{
    GM_Instrument	*theI;
    XPTR			theNewData;

    switch (command)
	{
	case ID_CSND:
	    theNewData = XDecompressPtr((XPTR)pMidiStream, length, FALSE);
	    if (theNewData)
		{
		    PV_SetSampleIntoCache(pSong, (XLongResourceID)theID, theNewData);
		}
	    break;
	case ID_ESND:
	    // since this is encrypted, make a new copy and decrypt
	    theNewData = XNewPtr(length);
	    if (theNewData)
		{
		    XBlockMove((XPTR)pMidiStream, theNewData, length);
		    XDecryptData(theNewData, length);				// decrypt first

		    PV_SetSampleIntoCache(pSong, (XLongResourceID)theID, theNewData);
		}
	    break;
	case ID_SND:
	    PV_SetSampleIntoCache(pSong, (XLongResourceID)theID, (XPTR)pMidiStream);
	    break;
	case ID_INST:
	    // this will load the instrument into the cache. When the sequence is later scanned it will flag this
	    // instrument for load, but since we've already loaded it into the cache it will not be loaded again.
	    // We don't need to set the reference count, because that will happen when we try to load the instrument
	    // again. It will get set to one hit. This works because we have the instrument already setup in the master
	    // instrument array.
	    if ( (theID >= 0) && (theID < (MAX_INSTRUMENTS*MAX_BANKS)) )
		{
		    GM_SetUsedInstrument(pSong, (XLongResourceID)theID, -1, TRUE);	// load this instrument for key splits and such
		    theI = PV_GetInstrument(theID, (void *)pMidiStream, length);
		    if (theI)
			{
			    theI->usageReferenceCount = 0;		// no change here, it will happen later
			    pSong->instrumentData[theID] = theI;
			    pSong->remapArray[theID] = theID;
			}
		    GM_SetUsedInstrument(pSong, (XLongResourceID)theID, -1, FALSE);	// don't load this instrument, already loaded
		}
	    break;
	case ID_SONG:
	    // merge in changes from external song
	    GM_MergeExternalSong((void *)pMidiStream, (XShortResourceID)theID, pSong);
	    break;
	}
}

// Validate command types. This is used to protect us from bad memory pointers, etc
static XBOOL PV_ValidateType(INT32 command)
{
    XBOOL	valid;

    valid = FALSE;
    switch (command)
	{
	case ID_CSND:
	case ID_SND:
	case ID_ESND:
	case ID_INST:
	case ID_SONG:
	    valid = TRUE;
	    break;
	}
    return valid;
}

// Validate command types. This is used to protect us from bad memory pointers, etc
static XBOOL PV_ValidateTypeCommands(INT32 command)
{
    XBOOL	valid;

    valid = FALSE;
    switch (command)
	{
	case EM_FLUSH_ID:		// immediate command
	case EM_CACHE_ID:		// immediate command
	case EM_DATA_ID:		// block resources
	    valid = TRUE;
	    break;
	}
    return valid;
}



// Process Igor Meta events given a midi stream right after the sysex ID
static void PV_ProcessIgorMeta(GM_Song *pSong, unsigned char *pMidiStream)
{
    INT32	command, unitCount, cmdCount, count, length, theID;
    char	resourceName[256];

    // only process instrument loading during the first midi scan. We can't allocate memory otherwise
    if (pSong->AnalyzeMode == SCAN_SAVE_PATCHES)
	{
	    cmdCount = XGetLong(pMidiStream);
	    if (cmdCount < (MAX_SAMPLES * 3))
		{
		    pMidiStream += 4;
		    for (count = 0; count < cmdCount; count++)
			{
			    command = XGetLong(pMidiStream);
			    if (PV_ValidateTypeCommands(command))
				{
				    pMidiStream += 4;
				    switch (command)
					{
					case EM_FLUSH_ID:		// immediate command
					    break;

					case EM_CACHE_ID:		// immediate command
					    break;

					case EM_DATA_ID:		// block resources
					    unitCount = XGetLong(pMidiStream);
					    if (unitCount < (MAX_SAMPLES * 3))
						{
						    pMidiStream += 4;
						    for (count = 0; count < unitCount; count++)
							{
							    command = XGetLong(pMidiStream);		// type
							    if (PV_ValidateType(command))
								{
								    pMidiStream += 4;
								    theID = XGetLong(pMidiStream);		// ID
								    pMidiStream += 4;
								    length = *pMidiStream;				// NAME
								    XBlockMove(pMidiStream, resourceName, length + 1);	// copy pascal string name
								    pMidiStream += length + 1;
								    length = XGetLong(pMidiStream);		// LENGTH of resource
								    pMidiStream += 4;
								    PV_ProcessIgorResource(pSong, command, pMidiStream, theID, length);
								    pMidiStream += length;
								}
							    else
								{
								    break;
								}
							}
						}
					    break;
					}
				}
			    else
				{
				    break;
				}
			}
		}
	}
}

// Given a song pointer and a string marker command, and a string length; process that command.
// If TRUE is returned, then the current track process is restarted at track 0
static XBOOL PV_ProcessMetaMarkerEvents(GM_Song *pSong, char *markerText, INT32 markerLength)
{
    INT32	count;
    XBOOL	restartTracks;

    restartTracks = FALSE;
    // support Headspace marker events
    if ((pSong->AnalyzeMode == SCAN_NORMAL) && (markerLength >= 7))
	{
	    if (XLStrnCmp("loopstart", markerText, 9) == 0)
		{
		    count = -1;	// loop forever
		    if (pSong->loopbackSaved == FALSE)		// only allow one save
			{
			    if (XLStrnCmp("loopstart=", markerText, 10) == 0)
				{
				    // check for loop counts
				    count = XStrnToLong(&markerText[10], markerLength-10);
				}
			    pSong->loopbackCount = (SBYTE)count;

			    pSong->loopbackSaved = TRUE;
			    for (count = 0; count < MAX_TRACKS; count++)
				{
				    pSong->pTrackPositionSave[count] = pSong->ptrack[count];
				    pSong->trackTicksSave[count] = pSong->trackticks[count];
				    pSong->trackStatusSave[count] = pSong->trackon[count];
				}
			    pSong->currentMidiClockSave = pSong->CurrentMidiClock;
			    pSong->songMicrosecondsSave = pSong->songMicroseconds;
			}
		}
	    else if (XLStrnCmp("loopend", markerText, markerLength) == 0)
		{
		    if ( (pSong->loopbackCount > 0) && (pSong->loopbackCount < 100) )
			{
			    pSong->loopbackCount--;
			}
		    if (pSong->loopbackCount)
			{
			    // ok, reloop.
			    restartTracks = TRUE;
			}
		}
	}
    return restartTracks;
}

// Given a pointer to a midi stream, this will read the variable length value from and update the
// midi stream pointer
static UINT32 PV_ReadVariableLengthMidi(UBYTE **ppMidiStream)
{
    register UINT32	value;
    register UBYTE	*midi_stream;

    midi_stream = *ppMidiStream;
    value = 0;					// read length and…
    while (*midi_stream & 0x80)
	{
	    value = value << 7;
	    value |= *midi_stream++ - 0x80;
	}
    value = value << 7;
    value |= *midi_stream++;
    *ppMidiStream = midi_stream;
    return value;
}


// Walk through the midi stream and process midi events for one slice of time.
OPErr PV_ProcessMidiSequencerSlice(void *threadContext, GM_Song *pSong)
{
    register LOOPCOUNT	currentTrack;
    register UBYTE		midi_byte, volume, controler;
    register UINT32		value;
    register UBYTE 		*midi_stream;
    register INT16		MIDIChannel;
    register UBYTE		valueLSB, valueMSB;
    UBYTE 				*temp_midi_stream;
    OPErr				theErr;
    XBOOL				reloopTracks;

    theErr = NO_ERR;

    // Song not loaded, bail
    if (pSong == NULL)
	{
	    return NO_SONG_PLAYING;
	}
    pSong->SomeTrackIsAlive = FALSE;
    pSong->processingSlice = TRUE;

    pSong->CurrentMidiClock += pSong->MIDIDivision;

    // 1 000 000 / 22050 * 256
    //	1 second / sample rate * samples
#if USE_HAE_EXTERNAL_API == TRUE
    pSong->songMicroseconds += (UFLOAT)HAE_GetSliceTimeInMicroseconds();
#else
    pSong->songMicroseconds += (UFLOAT)BUFFER_SLICE_TIME;
#endif
    reloopTracks = FALSE;

    for (currentTrack = 0; currentTrack < MAX_TRACKS; currentTrack++)
	{
	    midi_stream = pSong->ptrack[currentTrack];
	    if (midi_stream == NULL)
		{
		    goto ServeNextTrack;
		}
	    if (pSong->trackon[currentTrack])
		{
		    pSong->SomeTrackIsAlive = TRUE;
		    if (pSong->trackon[currentTrack] == TRACK_FREE)			// first time
			{
			    pSong->trackon[currentTrack] = TRACK_RUNNING;		// running
			    goto UpdateDeltaTime;
			}
		    pSong->trackticks[currentTrack] -= pSong->MIDIDivision;
		Do_GetEvent:
		    if (pSong->trackticks[currentTrack] < (IFLOAT)0)
			{
			    if ((UINT32)(midi_stream - pSong->trackstart[currentTrack]) > pSong->tracklen[currentTrack])
				{
				    /* the track has ended unexpectedly.
				     */
				    pSong->trackon[currentTrack] = TRACK_OFF;
				    //$$fb continue with next track
				    continue;
				}
			    goto GetMIDIevent;
			}
		    if (pSong->trackticks[currentTrack] >= (IFLOAT)0)
			{
			    goto ServeNextTrack;
			}
		UpdateDeltaTime:
		    temp_midi_stream = midi_stream;
		    value = PV_ReadVariableLengthMidi(&temp_midi_stream);
		    midi_stream = temp_midi_stream;
		    pSong->trackticks[currentTrack] += (IFLOAT)(value << 6);
		    pSong->trackcumuticks[currentTrack] += value;
		    goto Do_GetEvent;
		}
	    goto ServeNextTrack;
	GetMIDIevent:
	    midi_byte = *midi_stream++;
	    if (midi_byte == 0xFF)
		{
		    /* Meta Event
		     */
		    midi_byte = *midi_stream++;
		    switch (midi_byte)
			{
			    // Set MIDI Tempo:
			case 0x51:
			    value = midi_stream[1] << 8;
			    value |= midi_stream[2];
			    value = value << 8;
			    value |= midi_stream[3];
			    pSong->UnscaledMIDITempo = (UFLOAT)value;
#if USE_HAE_EXTERNAL_API == TRUE
			    pSong->MIDITempo = (pSong->UnscaledMIDITempo / (UFLOAT)HAE_GetSliceTimeInMicroseconds());
#else
			    pSong->MIDITempo = (pSong->UnscaledMIDITempo / (UFLOAT)BUFFER_SLICE_TIME);
#endif
			    PV_ScaleDivision(pSong, pSong->UnscaledMIDIDivision);
			    goto SkipMeta;

			    // Set Time Signature:
			case 0x58:
			    pSong->TSNumerator = midi_stream[1];
			    pSong->TSDenominator = midi_stream[2];
			    break;

			    // generic text event
			case 0x01:
			    // copyright event
			case 0x02:
			    // track name
			case 0x03:
			    // Lyric event
			case 0x05:
			    // Cue point event
			case 0x07:
			    // Marker event
			case 0x06:
			    temp_midi_stream = midi_stream;
			    value = PV_ReadVariableLengthMidi(&temp_midi_stream);	// get length

			    // there might be a problem here. need to relook into this.
			    // maybe we should only do these callbacks during NORMAL_SCAN??

			    // $$kk: 11.01.99: no kidding.  this crashes on GM_SetSongTickPosition and GM_SetSongMicrosecondPosition!
			    if (pSong->AnalyzeMode == SCAN_NORMAL)
				{
				    PV_CallSongMetaEventCallback(threadContext, pSong, midi_byte, (void *)temp_midi_stream, value, (short)currentTrack);
				}

			    // $$kk: 11.01.99: i *think* this should be called even if pSong->AnalyzeMode is not SCAN_NORMAL...?
			    if (midi_byte == 0x06)
				{
				    if (PV_ProcessMetaMarkerEvents(pSong, (char *)temp_midi_stream, value))
					{
					    reloopTracks = TRUE;	// this will finish up these tracks, then restart
					}
				}
			    goto SkipMeta;

			    // Sequencer specific event (The heart of eMidi)
			case 0x7F:
			    temp_midi_stream = midi_stream;
			    value = PV_ReadVariableLengthMidi(&temp_midi_stream);	// get length
			    midi_stream = temp_midi_stream;
			    if (midi_stream[0] == 0x00)		// IGOR sysex ID
				{
				    if (midi_stream[1] == 0x01)
					{
					    if (midi_stream[2] == 0x0D)
						{
						    PV_ProcessIgorMeta(pSong, midi_stream + 3);
						}
					}
				}
			    midi_stream += value;			// skip meta
			    goto UpdateDeltaTime;

			    // End-of-track:
			case 0x2F:
			    pSong->trackon[currentTrack] = TRACK_OFF;
			    goto ServeNextTrack;
			}
		    goto SkipMeta;
		}
	    else
		{
		    if (midi_byte <= 0x7F)
			{
			    --midi_stream;
			    midi_byte = pSong->runningStatus[currentTrack];
			}
		    else
			{
			    pSong->runningStatus[currentTrack] = midi_byte;
			    if (midi_byte == 0)		/* 0 = null event. */
				{
				    goto GetMIDIevent;
				}
			}
		    MIDIChannel = midi_byte & 0xF;
		    switch (midi_byte & 0xF0)			// process commands
			{

			    GM_Synth *theSynth;

			case 0x90:					// •• Note On
			    value = *midi_stream++;		// MIDI note
			    volume = *midi_stream++;	// note on velocity

			    if (pSong->AnalyzeMode != SCAN_NORMAL)
				{
				    // $$kk: 05.04.99
				    PV_ProcessNoteOn(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)value, (INT16)volume);
				}
			    else
				{
				    theSynth = pSong->pSynths;

				    while (theSynth)
					{
					    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
					    // the userReference.  however, someone should be able to!!
					    pSong->userReference = (void *)theSynth;
					    (theSynth->pNoteOnProcPtr)(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)value, (INT16)volume);
					    theSynth = theSynth->pNext;
					}
				}
			    break;
			case 0x80:					// •• Note Off
			    value = *midi_stream++;		// MIDI note
			    volume = *midi_stream++;	// note off velocity, ignore for now

			    if (pSong->AnalyzeMode != SCAN_NORMAL)
				{
				    // $$kk: 05.04.99
				    PV_ProcessNoteOff(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)value, (INT16)volume);
				}
			    else
				{
				    theSynth = pSong->pSynths;

				    while (theSynth)
					{
					    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
					    // the userReference.  however, someone should be able to!!
					    pSong->userReference = (void *)theSynth;
					    (theSynth->pNoteOffProcPtr)(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)value, (INT16)volume);
					    theSynth = theSynth->pNext;
					}
				}
			    break;
			case 0xB0:					// •• Control Change
			    controler = *midi_stream++; // control #
#if 0 && USE_CREATION_API == TRUE
			    if (controler == 0 && (pSong->AnalyzeMode == SCAN_COUNT_PATCHES || pSong->AnalyzeMode == SCAN_FIND_PATCHES))
				{
				    if (pSong->pPatchInfo)
					{
					    if (pSong->pPatchInfo->bankPtrLoc)
						{
						    pSong->pPatchInfo->bankPtrLoc = midi_stream;
						}
					}
				}
#endif
			    midi_byte = *midi_stream++;	// controller value

			    if (pSong->AnalyzeMode != SCAN_NORMAL)
				{
				    // $$kk: 05.04.99
				    PV_ProcessController(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)controler, (UINT16)midi_byte);
				}
			    else
				{
				    theSynth = pSong->pSynths;

				    while (theSynth)
					{
					    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
					    // the userReference.  however, someone should be able to!!
					    pSong->userReference = (void *)theSynth;
					    (theSynth->pProcessControllerProcPtr)(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)controler, (UINT16)midi_byte);
					    theSynth = theSynth->pNext;
					}
				}

			    if (pSong->AnalyzeMode == SCAN_NORMAL)
				{
				    PV_CallControlCallbacks(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)controler, (UINT16)midi_byte);
				}
			    break;
			case 0xC0:					// •• ProgramChange
			    value = *midi_stream;
#if 0 && USE_CREATION_API == TRUE
			    if (pSong->AnalyzeMode == SCAN_FIND_PATCHES || pSong->AnalyzeMode == SCAN_COUNT_PATCHES)
				{
				    if (pSong->pPatchInfo)
					{
					    if (pSong->pPatchInfo->instrPtrLoc)
						{
						    pSong->pPatchInfo->instrPtrLoc = midi_stream;
						}
					}
				}
#endif
			    midi_stream++;

			    if (pSong->AnalyzeMode != SCAN_NORMAL)
				{
				    // $$kk: 05.04.99
				    PV_ProcessProgramChange(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)value);
				}
			    else
				{
				    theSynth = pSong->pSynths;

				    while (theSynth)
					{
					    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
					    // the userReference.  however, someone should be able to!!
					    pSong->userReference = (void *)theSynth;
					    (theSynth->pProgramChangeProcPtr)(threadContext, pSong, MIDIChannel, (INT16)currentTrack, (INT16)value);
					    theSynth = theSynth->pNext;
					}
				}
#if 0 && USE_CREATION_API == TRUE
			    if (pSong->AnalyzeMode == SCAN_FIND_PATCHES)
				{
				    if (pSong->pPatchInfo)
					{
					    midi_stream += pSong->pPatchInfo->streamIncrement;
					    pSong->pPatchInfo->streamIncrement = 0;
					}
				}
#endif
			    break;
			case 0xE0:					// •• SetPitchBend
			    valueLSB = *midi_stream++;
			    valueMSB = *midi_stream++;

			    if (pSong->AnalyzeMode != SCAN_NORMAL)
				{
				    // $$kk: 05.04.99
				    PV_ProcessPitchBend(threadContext, pSong, MIDIChannel, (INT16)currentTrack, valueMSB, valueLSB);
				}
			    else
				{
				    theSynth = pSong->pSynths;

				    while (theSynth)
					{
					    // $$kk: 07.12.99: this is a temporary solution 'cause i know we don't use
					    // the userReference.  however, someone should be able to!!
					    pSong->userReference = (void *)theSynth;
					    (theSynth->pPitchBendProcPtr)(threadContext, pSong, MIDIChannel, (INT16)currentTrack, valueMSB, valueLSB);
					    theSynth = theSynth->pNext;
					}
				}
			    break;
			case 0xA0:					// ••  Key Pressure
			    midi_stream += 2;		// note, key pressure
			    break;
			case 0xD0:					// •• ChannelPressure
			    midi_stream++;
			    break;
			case 0xF7:					// •• System Exclusive
			case 0xF0:					// •• System Exclusive
			    temp_midi_stream = midi_stream;
			    value = PV_ReadVariableLengthMidi(&temp_midi_stream);
			    midi_stream = temp_midi_stream;
			    midi_stream += value;	// skip sysex
			    break;
			} /* end switch */
		    /* Not Meta Event. */
		    goto UpdateDeltaTime;
		}
	    // THE COMPILER WARNS THAT WE CAN'T GET HERE.
	    // WHAT DO YOU THINK STEVE?
	    //		goto ServeNextTrack;
	    // I HATE THIS CODE!!! This needs to be reworked. SH
	SkipMeta:
	    temp_midi_stream = midi_stream;
	    value = PV_ReadVariableLengthMidi(&temp_midi_stream);
	    midi_stream = temp_midi_stream;
	    midi_stream += value;	// skip sysex
	    goto UpdateDeltaTime;
	ServeNextTrack:
	    pSong->ptrack[currentTrack] = midi_stream;

	    // collect some information
	    if ((currentTrack < 2) && (pSong->AnalyzeMode == SCAN_SAVE_PATCHES))
		{
		    pSong->averageTotalVoices++;
		    pSong->averageActiveVoices += pSong->voiceCount;
		    if (pSong->averageTotalVoices)
			{
			    pSong->averageVoiceUsage = pSong->averageActiveVoices / pSong->averageTotalVoices;
			}
		    if (pSong->voiceCount > pSong->maxVoiceUsage)
			{
			    pSong->maxVoiceUsage = pSong->voiceCount;
			}
		}
	}


    if (reloopTracks)
	{
	    short int	count;

	    // we've been told to restart at a new position. So restore the sequencer back to the original
	    // saved position, and the next sequencer slice will start processing at the new position
	    reloopTracks = FALSE;
	    for (count = 0; count < MAX_TRACKS; count++)
		{
		    pSong->ptrack[count] = pSong->pTrackPositionSave[count];
		    pSong->trackticks[count] = pSong->trackTicksSave[count];
		    pSong->trackon[count] = pSong->trackStatusSave[count];
		}
	    pSong->CurrentMidiClock = pSong->currentMidiClockSave;
	    pSong->songMicroseconds = pSong->songMicrosecondsSave;
	    pSong->CurrentMidiClock -= pSong->MIDIDivision;

	    // 1 000 000 / 22050 * 256
	    //	1 second / sample rate * samples
#if USE_HAE_EXTERNAL_API == TRUE
	    pSong->songMicroseconds -= (UFLOAT)HAE_GetSliceTimeInMicroseconds();
#else
	    pSong->songMicroseconds -= (UFLOAT)BUFFER_SLICE_TIME;
#endif
	    GM_KillSongNotes(pSong);	// go ahead and stop all notes currently playing. This will insure that
	    // there are no hanging notes at loop restart
	}


    if (pSong->AnalyzeMode == SCAN_NORMAL)	// song finished?
	{
	    if (pSong->songTimeCallbackPtr)
		{
		    (*pSong->songTimeCallbackPtr)(threadContext, pSong, (UINT32)pSong->songMicroseconds,
						  (UINT32)pSong->CurrentMidiClock);
		}

	    if ((reloopTracks == FALSE) && (GM_IsSongDone(pSong)))
		{
		    if (pSong->songFinished == FALSE)
			{
			    XBOOL	stopSong, loopSong;

			    // we have the ability to loop x number of times, or loop forever.
			    // If loopSong is TRUE, then we loop forever but loop x times within the loop
			    stopSong = TRUE;			// stop song
			    loopSong = pSong->loopSong;

			    if (pSong->songMaxLoopCount)
				{
				    loopSong = TRUE;
				    if (pSong->songLoopCount >= pSong->songMaxLoopCount)
					{
					    pSong->songLoopCount = -1;	// let increment set it to zero
					    if (pSong->loopSong)
						{
						    loopSong = TRUE;
						}
					    else
						{
						    loopSong = FALSE;
						    stopSong = TRUE;
						    pSong->songMaxLoopCount = 0;	// stop loop
						}
					}
				    pSong->songLoopCount++;
				}

			    if (loopSong)
				{
				    // reconfigure song and loop
				    PV_ConfigureMusic(pSong);
				    stopSong = FALSE;
				}
			    if (stopSong)
				{
				    // call callback and finish
				    PV_CallSongCallback(threadContext, pSong, TRUE);
				    pSong->songFinished = TRUE;
				}
			}
		}
	}
    pSong->processingSlice = FALSE;
    return theErr;
}

// process all songs and external events
void PV_ProcessSequencerEvents(void *threadContext)
{
    GM_Song		*pSong;
    short int	count;

    if (MusicGlobals->enableDriftFixer) {
	// if enabled, this will fix the drift of real time with our synth time. This
	// is used only when real time midi data, that is being time stamped with
	// XMicroseconds is enabled
	INT32	drift;

	drift = XMicroseconds() - MusicGlobals->syncCount;
	if (drift > 1000) {	// if drift more than 1 ms reset
	    MusicGlobals->syncCount = XMicroseconds();
	    MusicGlobals->syncBufferCount = 0;
	}
    }

    if (MusicGlobals->sequencerPaused == FALSE) {
	for (count = 0; count < MAX_SONGS; count++) {
	    pSong = MusicGlobals->pSongsToPlay[count];
	    if (pSong) {
		if (pSong->AnalyzeMode == SCAN_NORMAL) {
		    // process external midi events
		    PV_ProcessExternalMIDIQueue(threadContext, pSong);
    
		    /* better re-check that song wasn't disabled meanwhile */
		    pSong = MusicGlobals->pSongsToPlay[count];
		    if (pSong && pSong->songPaused == FALSE) {  
			// only process if not paused....
			PV_ServeSongFade(threadContext, pSong);
			// process current song playing
			PV_ProcessMidiSequencerSlice(threadContext, pSong);
		    }
		}
	    }
	}
    }
}

static void PV_EndSongChannelNotes(GM_Song *pSong, short int channel)
{
    register short int		count;
    register GM_Mixer		*pMixer;
    register GM_Voice		*pNote;

    pMixer = MusicGlobals;

    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    pNote = &pMixer->NoteEntry[count];
	    if (pNote->pSong == pSong)
		{
		    if (pNote->NoteChannel == channel)
			{
			    if (pNote->voiceMode != VOICE_UNUSED)
				{
				    pNote->voiceMode = VOICE_RELEASING;
				    pNote->NoteDecay = 2;
				    pNote->volumeADSRRecord.mode = ADSR_TERMINATE;
				    pNote->volumeADSRRecord.currentPosition = 0;
				    pNote->volumeADSRRecord.ADSRLevel[0] = 0;
				    pNote->volumeADSRRecord.ADSRTime[0] = 1;
				    pNote->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
				    pNote->NoteVolumeEnvelopeBeforeLFO = 0;		// so these notes can be reused
				}
			}
		}
	}
}

void GM_MuteChannel(GM_Song *pSong, short int channel)
{
    short int	count;

    if ( (channel < MAX_CHANNELS) && (channel >= 0) )
	{
	    if (pSong)
		{
		    XSetBit(&pSong->channelMuted, channel);
		    PV_EndSongChannelNotes(pSong, channel);
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_MuteChannel(pSong, channel);
				}
			}
		}
	}
}

void GM_UnmuteChannel(GM_Song *pSong, short int channel)
{
    short int	count;

    if ( (channel < MAX_CHANNELS) && (channel >= 0) )
	{
	    if (pSong)
		{
		    XClearBit(&pSong->channelMuted, channel);
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_UnmuteChannel(pSong, channel);
				}
			}
		}
	}
}

// will write only 16 bytes for 16 Midi channels
void GM_GetChannelMuteStatus(GM_Song *pSong, char *pStatus)
{
    short int	count, count2;

    if (pStatus)
	{
	    if (pSong)
		{
		    for (count = 0; count < 16; count++)
			{
			    pStatus[count] = XTestBit(&pSong->channelMuted, count);
			}
		}
	    else
		{
		    for (count = 0; count < 16; count++)
			{
			    pStatus[count] = FALSE;
			}
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    for (count2 = 0; count2 < 16; count2++)
					{
					    pStatus[count2] |= XTestBit(&pSong->channelMuted, count2);
					}
				}
			}
		}
	}
}

void GM_SoloChannel(GM_Song *pSong, short int channel)
{
    short int	count;

    if ( (channel < MAX_CHANNELS) && (channel >= 0) )
	{
	    if (pSong)
		{
		    XSetBit(&pSong->soloChannelMuted, channel);

		    // mute un solo'd channels
		    for (count = 0; count < MAX_CHANNELS; count++)
			{
			    if (XTestBit(&pSong->soloChannelMuted, count) == FALSE)
				{
				    PV_EndSongChannelNotes(pSong, count);
				}
			}
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_SoloChannel(pSong, channel);
				}
			}
		}
	}
}

void GM_UnsoloChannel(GM_Song *pSong, short int channel)
{
    short int	count;

    if ( (channel < MAX_CHANNELS) && (channel >= 0) )
	{
	    if (pSong)
		{
		    XClearBit(&pSong->soloChannelMuted, channel);

		    // mute un solo'd channels
		    for (count = 0; count < MAX_CHANNELS; count++)
			{
			    if (XTestBit(&pSong->soloChannelMuted, count) == FALSE)
				{
				    PV_EndSongChannelNotes(pSong, count);
				}
			}
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_UnsoloChannel(pSong, channel);
				}
			}
		}
	}
}

// will write only 16 bytes for 16 Midi channels
void GM_GetChannelSoloStatus(GM_Song *pSong, char *pStatus)
{
    short int	count, count2;

    if (pStatus)
	{
	    if (pSong)
		{
		    for (count = 0; count < 16; count++)
			{
			    pStatus[count] = XTestBit(&pSong->soloChannelMuted, count);
			}
		}
	    else
		{
		    for (count = 0; count < 16; count++)
			{
			    pStatus[count] = FALSE;
			}
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    for (count2 = 0; count2 < 16; count2++)
					{
					    pStatus[count2] |= XTestBit(&pSong->soloChannelMuted, count2);
					}
				}
			}
		}
	}
}

static void PV_EndSongTrackNotes(GM_Song *pSong, short int track)
{
    register short int		count;
    register GM_Mixer		*pMixer;
    register GM_Voice		*pNote;

    pMixer = MusicGlobals;

    for (count = 0; count < pMixer->MaxNotes; count++)
	{
	    pNote = &pMixer->NoteEntry[count];
	    if (pNote->pSong == pSong)
		{
		    if (pNote->NoteTrack == track)
			{
			    if (pNote->voiceMode != VOICE_UNUSED)
				{
				    pNote->voiceMode = VOICE_RELEASING;
				    pNote->NoteDecay = 2;
				    pNote->volumeADSRRecord.mode = ADSR_TERMINATE;
				    pNote->volumeADSRRecord.currentPosition = 0;
				    pNote->volumeADSRRecord.ADSRLevel[0] = 0;
				    pNote->volumeADSRRecord.ADSRTime[0] = 1;
				    pNote->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
				    pNote->NoteVolumeEnvelopeBeforeLFO = 0;		// so these notes can be reused
				}
			}
		}
	}
}

void GM_MuteTrack(GM_Song *pSong, short int track)
{
    short int	count;

    if ( (track < MAX_TRACKS) && (track >= 0) )
	{
	    if (pSong)
		{
		    XSetBit(&pSong->trackMuted, track);
		    PV_EndSongTrackNotes(pSong, track);
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_MuteTrack(pSong, track);
				}
			}
		}
	}
}

void GM_UnmuteTrack(GM_Song *pSong, short int track)
{
    short int	count;

    if ( (track < MAX_TRACKS) && (track >= 0) )
	{
	    if (pSong)
		{
		    XClearBit(&pSong->trackMuted, track);
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_UnmuteTrack(pSong, track);
				}
			}
		}
	}
}


void GM_GetTrackMuteStatus(GM_Song *pSong, char *pStatus)
{
    short int	count, count2;

    if (pStatus)
	{
	    if (pSong)
		{
		    for (count = 0; count < MAX_TRACKS; count++)
			{
			    pStatus[count] = XTestBit(&pSong->trackMuted, count);
			}
		}
	    else
		{
		    for (count = 0; count < MAX_TRACKS; count++)
			{
			    pStatus[count] = FALSE;
			}
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    for (count2 = 0; count2 < MAX_TRACKS; count2++)
					{
					    pStatus[count2] |= XTestBit(&pSong->trackMuted, count2);
					}
				}
			}
		}
	}
}

void GM_SoloTrack(GM_Song *pSong, short int track)
{
    short int	count;

    if ( (track < MAX_TRACKS) && (track >= 0) )
	{
	    if (pSong)
		{
		    XSetBit(&pSong->soloTrackMuted, track);

		    // mute un solo'd tracks
		    for (count = 0; count < MAX_TRACKS; count++)
			{
			    if (XTestBit(&pSong->soloTrackMuted, count) == FALSE)
				{
				    PV_EndSongTrackNotes(pSong, count);
				}
			}
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_SoloTrack(pSong, track);
				}
			}
		}
	}
}

void GM_UnsoloTrack(GM_Song *pSong, short int track)
{
    short int	count;

    if ( (track < MAX_TRACKS) && (track >= 0) )
	{
	    if (pSong)
		{
		    XClearBit(&pSong->soloTrackMuted, track);

		    // mute un solo'd tracks
		    for (count = 0; count < MAX_TRACKS; count++)
			{
			    if (XTestBit(&pSong->soloTrackMuted, count) == FALSE)
				{
				    PV_EndSongTrackNotes(pSong, count);
				}
			}
		}
	    else
		{
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    GM_UnsoloTrack(pSong, track);
				}
			}
		}
	}
}

// will write only MAX_TRACKS bytes for MAX_TRACKS Midi tracks
void GM_GetTrackSoloStatus(GM_Song *pSong, char *pStatus)
{
    short int	count, count2;

    if (pStatus)
	{
	    if (pSong)
		{
		    for (count = 0; count < MAX_TRACKS; count++)
			{
			    pStatus[count] = XTestBit(&pSong->soloTrackMuted, count);
			}
		}
	    else
		{
		    for (count = 0; count < MAX_TRACKS; count++)
			{
			    pStatus[count] = FALSE;
			}
		    for (count = 0; count < MAX_SONGS; count++)
			{
			    pSong = MusicGlobals->pSongsToPlay[count];
			    if (pSong)
				{
				    for (count2 = 0; count2 < 16; count2++)
					{
					    pStatus[count2] |= XTestBit(&pSong->soloTrackMuted, count2);
					}
				}
			}
		}
	}
}

// If allowPitch is FALSE, then "GM_SetSongPitchOffset" will have no effect on passed
// channel (0 to 15)
void GM_AllowChannelPitchOffset(GM_Song *pSong, unsigned short int channel, XBOOL allowPitch)
{
    if (pSong)
	{
	    if (allowPitch)
		{
		    XSetBit(&pSong->allowPitchShift, channel);
		}
	    else
		{
		    XClearBit(&pSong->allowPitchShift, channel);
		}
	}
}

// Return if the passed channel will allow pitch offset
XBOOL GM_DoesChannelAllowPitchOffset(GM_Song *pSong, unsigned short int channel)
{
    XBOOL	flag;

    flag = FALSE;
    if (pSong)
	{
	    flag = XTestBit(&pSong->allowPitchShift, channel);
	}
    return flag;
}

// set note offset in semi tones	(12 is down an octave, -12 is up an octave)
void GM_SetSongPitchOffset(GM_Song *pSong, INT32 offset)
{
    if (pSong)
	{
	    pSong->songPitchShift = (INT16)-offset;
	}
}

// return note offset in semi tones	(12 is down an octave, -12 is up an octave)
INT32 GM_GetSongPitchOffset(GM_Song *pSong)
{
    INT32	offset;

    offset = 0;
    if (pSong)
	{
	    offset = -pSong->songPitchShift;
	}
    return offset;
}


// EOF of GenSeq.c
