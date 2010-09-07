/*
 * @(#)GenSong.c	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenSong.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
** Overview
**	This contains code to load an maintain songs and midi data. This code library is specific
**	to the platform, but the API is independent in nature.
**
**	NOTE: relies on structures from SoundMusicSys API
**
**
** Modification History:
**
**	1/24/96		Created
**	1/25/96		Moved GM_GetUsedPatchlist from GenSeq.c
**	1/28/96		Moved GM_FreeSong from GenSeq.c
**	2/3/96		Removed extra includes
**	2/5/96		Removed unused variables. Working towards multiple songs
**				Added GM_GetSongTickLength
**	2/12/96		Added GM_SetSongTickPosition
**	3/1/96		Fixed bug with GM_SetSongTickPosition that would blow the songLoop flag away
**	3/5/96		Eliminated the global songVolume
**	4/15/96		Added support to interpret SONG resource via GM_MergeExternalSong
**	4/21/96		Removed CPU edian issues by use XGetShort & XGetLong
**				Removed register usage in parameters
**	5/18/96		More error correction in GM_GetSongTickLength & GM_SetSongTickPosition
**	5/30/96		Fixed possible bad code in PV_CreateSongFromMidi
**				Added ignore Bad patches code
**	6/7/96		Added some error correction changes to GM_LoadSong
**	7/4/96		Changed font and re tabbed
**	9/17/96		Added GM_LoadSongInstrument & GM_UnloadSongInstrument
**	9/23/96		Added channel muting feature
**	9/25/96		Changed GM_SetSongTickPosition to end just the song notes its working on
**	11/9/96		Fixed a bug with GM_FreeSong that would free instruments before
**				instruments were done playing
**	12/30/96	Changed copyright
**	1/12/97		Added support for more song types
**				Changed maxNormalizedVoices to mixLevel
**	1/28/97		Eliminated terminateDecay flag. Not used anymore
**	1/29/97		Reworked GM_LoadSong to support encrypted midi files
**	2/1/97		Added support for pitch offset control on a per channel basis
**	3/20/97		Added GM_SetSongMicrosecondPosition & GM_GetSongMicrosecondLength
**	4/20/97		Changed PV_MusicIRQ to PV_ProcessMidiSequencerSlice
**	5/3/97		Fixed a few potential problems that the MOT compiler found. Specificly
**				changed a theSong->songMidiTickLength = -1 to theSong->songMidiTickLength = 0
**				to signal calculate length.
**	7/29/97		(ddz) Added call to PV_FreePgmEntries in GM_FreeSong, and GM_GetSongProgramChanges
**	8/7/97		Changed GM_SongTicks & GM_SetSongTickPosition & GM_GetSongTickLength
**				to support UFLOAT
**	8/13/97		Renamed GM_GetSongProgramChanges to GM_GetSongInstrumentChanges and changed
**				Byte reference to XBYTE
**	8/15/97		Fixed a bug in which a data block was being freed in GM_FreeSong after
**				the main pointer was trashed.
**				Added disposal in GM_FreeSong of the new controller callbacks
**	9/5/97		(ddz) Made all pgm change stuff in one structure PatchInfo, changed PV_FreePgmEntries
**              to PV_FreePatchInfo
**	9/19/97		Fixed bug with GM_GetSongTickLength that allowed callbacks to be
**				called and freed during a length calculation
**	9/25/97		Added ddz changes, and wrapped a couple of function around USE_CREATION_API == TRUE
**	10/15/97	Modified GM_UnloadSongInstrument to handle the case in which a instrument
**				is still busy
**				Modified GM_FreeSong to handle the case in which instruments, samples, or
**				midi data is still busy.
**	10/16/97	Changed GM_LoadSong parmeters to include an option to ignore bad instruments
**				when loading.
**				Renamed ignoreBadPatches to ignoreBadInstruments
**	10/18/97	Fixed some compiler warnings and modified GM_FreeSong to kill midi data
**				pointer now rather than let the decoder thread get it later.
**	10/27/97	Removed reference to MusicGlobals->theSongPlaying
**	2/2/98		Added GM_SetVelocityCurveType
**	2/8/98		Changed BOOL_FLAG to XBOOL
** JAVASOFT
**	02.10.98:	$$kk: GM_SetSongMicrosecondPosition: added check for whether
**				song was paused so that we don't resume if this method was called on a paused song.
**	6/30/98		Removed INT16 casting in GM_MergeExternalSong
**				Changed GM_LoadSong/GM_CreateLiveSong to accept a long rather than a short for 
**				the songID
**	7/6/98		Added GM_IsSongInstrumentLoaded
**				Fixed type problems with GM_LoadSong & PV_CreateSongFromMidi
** 2/18/99		Changed GM_LoadSong & GM_CreateLiveSong to pass in a context
**				Added GM_GetSongContext & GM_SetSongContext
**	3/5/99		Added threadContext to GM_LoadSong & GM_FreeSong
**
**	JAVASOFT
**	05.04.99	Changed calls to synth to work through a pSynth pointer so that we can direct the
**	06.21.99	calls to an external synth or the Beatnik synth.
**	2002-03-14	$$fb removed compiler warnings
*/
/*****************************************************************************/

#include "X_API.h"
#include "X_Formats.h"
#include "GenSnd.h"
#include "GenPriv.h"

// Functions


static GM_Song * PV_CreateSongFromMidi(XLongResourceID theID, XPTR useThisMidiData, INT32 midiSize)
{
    XPTR		theMidiData;
    GM_Song		*theSong;
    INT32		count;

    theSong = NULL;
    if (useThisMidiData)
	{
	    theMidiData = useThisMidiData;
	}
    else
	{
	    midiSize = 0;
	    theMidiData = XGetMidiData(theID, &midiSize, NULL);
	}
    if (theMidiData)
	{
	    theSong = (GM_Song *)XNewPtr((INT32)sizeof(GM_Song));
	    if (theSong)
		{
		    theSong->midiData = theMidiData;
		    theSong->midiSize = midiSize;
		    theSong->disposeSongDataWhenDone = (useThisMidiData == NULL) ? TRUE : FALSE;
		    // Fill in remap first
		    for (count = 0; count < (MAX_INSTRUMENTS*MAX_BANKS); count++)
			{
			    theSong->instrumentRemap[count] = (XLongResourceID)-1;		// no remap
			}
		}
	}
    return theSong;
}

static void PV_SetTempo(GM_Song *pSong, INT32 masterTempo)
{
    if (pSong)
	{
	    if (masterTempo == 0L)
		{
		    masterTempo = 16667;
		}
	    masterTempo = (100L * masterTempo) / 16667;
	    if (masterTempo < 25) masterTempo = 25;
	    if (masterTempo > 300) masterTempo = 300;
	    GM_SetMasterSongTempo(pSong, (masterTempo << 16L) / 100L);
	}
}

void GM_MergeExternalSong(void *theExternalSong, XShortResourceID theSongID, GM_Song *theSong)
{
    short int			maps;
    short int			count;
    short int			number;
    SongResource_SMS	*songSMS;
    SongResource_RMF	*songRMF;
    Remap				*pMap;

    if (theExternalSong && theSong)
	{
	    switch (((SongResource_SMS *)theExternalSong)->songType)
		{
		case SONG_TYPE_SMS:
		    songSMS = (SongResource_SMS *)theExternalSong;
		    theSong->songID = theSongID;
		    theSong->songPitchShift = songSMS->songPitchShift;
		    theSong->allowProgramChanges = (songSMS->flags1 & XBF_enableMIDIProgram) ? TRUE : FALSE;
		    theSong->defaultPercusionProgram = songSMS->defaultPercusionProgram;
		    theSong->defaultReverbType = songSMS->reverbType;
		    theSong->maxSongVoices = songSMS->maxNotes;
		    theSong->mixLevel = XGetShort(&songSMS->mixLevel);
		    theSong->maxEffectVoices = songSMS->maxEffects;
		    theSong->ignoreBadInstruments = (songSMS->flags2 & XBF_ignoreBadPatches) ? TRUE : FALSE;
		    maps = XGetShort(&songSMS->remapCount);
		    PV_SetTempo(theSong, XGetShort(&songSMS->songTempo));
		    theSong->songVolume = XGetSongVolume((SongResource *)theExternalSong);

		    // Load instruments
		    if ((songSMS->flags1 & XBF_enableMIDIProgram) == FALSE)
			{
			    number = (songSMS->flags1 & XBF_fileTrackFlag) ? MAX_TRACKS : MAX_CHANNELS;
			    for (count = 0; count < number; count++)
				{
				    theSong->instrumentRemap[count] = count;
				}
			}

				// Fill in remap first
		    if (maps)
			{
			    pMap = (Remap *)&songSMS->remaps;
			    for (count = 0; count < maps; count++)
				{
				    number = XGetShort(&pMap[count].instrumentNumber) & ((MAX_INSTRUMENTS*MAX_BANKS)-1);
				    theSong->instrumentRemap[number] = XGetShort(&pMap[count].ResourceINSTID);
				}
			}
		    break;
		
		case SONG_TYPE_RMF:
		    songRMF = (SongResource_RMF *)theExternalSong;
		    theSong->songID = theSongID;
		    theSong->songPitchShift = songRMF->songPitchShift;
		    theSong->allowProgramChanges = TRUE;			// aloways allow program changes
		    theSong->defaultPercusionProgram = -1;			// GM percussion only
		    theSong->defaultReverbType = songRMF->reverbType;
		    theSong->maxSongVoices = XGetShort(&songRMF->maxNotes);
		    theSong->mixLevel = XGetShort(&songRMF->mixLevel);
		    theSong->maxEffectVoices = XGetShort(&songRMF->maxEffects);
		    theSong->ignoreBadInstruments = TRUE;
		    PV_SetTempo(theSong, XGetShort(&songRMF->songTempo));
		    theSong->songVolume = XGetSongVolume((SongResource *)theExternalSong);
		    break;
		}
	}
}


static void PV_ClearSongInstruments(GM_Song *pSong)
{
    INT32	count;

    if (pSong)
	{
	    for (count = 0; count < (MAX_INSTRUMENTS*MAX_BANKS); count++)
		{
		    pSong->instrumentData[count] = NULL;
		}
	}
}

// return valid context for song that was passed in when calling GM_LoadSong or GM_CreateLiveSong
void * GM_GetSongContext(GM_Song *pSong)
{
    void	*context;

    context = NULL;
    if (pSong)
	{
	    context = pSong->context;
	}
    return context;
}

// set valid context for song
void GM_SetSongContext(GM_Song *pSong, void *context)
{
    if (pSong)
	{
	    pSong->context = context;
	}
}

void GM_AddSongSynth(GM_Song *pSong, GM_Synth *pSynth)
{
    GM_Synth *currentSynth = pSong->pSynths;

    // $$kk: 07.12.99: i am going to interpret
    // "pSynth == NULL" as meaning "use the software synth."
    // we should revisit this later.
    if (pSynth == NULL)
	{
	    pSynth = (GM_Synth *)XNewPtr((INT32)sizeof(GM_Synth));

	    if (!pSynth)
		{
		    // $$kk: 07.12.99: this is a memory err.  should report it!
		    // but for now this is a void method....
		    return;
		}

	    pSynth->deviceHandle = NULL;
	    pSynth->pProgramChangeProcPtr = PV_ProcessProgramChange;
	    pSynth->pNoteOffProcPtr = PV_ProcessNoteOff;
	    pSynth->pNoteOnProcPtr = PV_ProcessNoteOn;
	    pSynth->pPitchBendProcPtr = PV_ProcessPitchBend;
	    pSynth->pProcessControllerProcPtr = PV_ProcessController;
	    pSynth->pProcessSongSoundOffProcPtr = GM_EndSongNotes;
	    pSynth->pNext = NULL;
	}


    if (!currentSynth)
	{
	    pSong->pSynths = pSynth;
	    return;
	}

    while (currentSynth->pNext != NULL)
	{
	    currentSynth = currentSynth->pNext;
	}

    currentSynth->pNext = pSynth;
    return;
}

// remove a synth from responding to this song's events
void GM_RemoveSongSynth(GM_Song *pSong, GM_Synth *pSynth)
{
    // $$kk: 07.10.99: should check failure case!
    GM_Synth *currentSynth = pSong->pSynths;

    if (!currentSynth)
	{
	    // no synths set!
	    return;
	}

    if (currentSynth == pSynth)
	{
	    // found it!
	    pSong->pSynths = currentSynth->pNext;
	    return;
	}

    while (currentSynth->pNext != NULL)
	{
	    if (currentSynth->pNext == pSynth)
		{
		    // found it!
		    currentSynth->pNext = currentSynth->pNext->pNext;
		    return;
		}
	}

    // didn't find it....
    return;
}

// given a synth, return the next one for the song.  
// i.e. pass NULL to get the first one (if any), and 
// pass in each one returned to get the next.  when you
// pass in the last one, NULL is returned.
GM_Synth * GM_GetSongSynth(GM_Song *pSong, GM_Synth *pSynth)
{
    GM_Synth *currentSynth = pSong->pSynths;
	
    if ( (pSynth == NULL) || (currentSynth == NULL) )
	{
	    return currentSynth;
	}

    while (currentSynth != NULL)
	{
	    if (currentSynth == pSynth)
		{
		    return currentSynth->pNext;
		}

	    currentSynth = currentSynth->pNext;
	}

    return NULL;
}


/*
  // return synth for song
  GM_Synth *GM_GetSongSynth(GM_Song *pSong)
  {
  GM_Synth	*pSynth;

  pSynth = NULL;
  if (pSong)
  {
  pSynth = pSong->pSynth;
  }
  return pSynth;
  }

  // set synth for song
  void GM_SetSongSynth(GM_Song *pSong, GM_Synth *pSynth)
  {
  if (pSong)
  {
  pSong->pSynth = pSynth;
  }
  }
*/

// GM_CreateLiveSong is used to create an active midi object that can be
// controled directly. ie via midi commands without loading midi data
//
//	context				context of song creation. C++ 'this' pointer, thread, etc.
//						Its just stored in the GM_Song->context variable
//	songID				unique ID for song structure
GM_Song * GM_CreateLiveSong(void *context, XShortResourceID songID)
{
    GM_Song			*pSong;
    short int		count;

    pSong = NULL;

    pSong = (GM_Song *)XNewPtr((INT32)sizeof(GM_Song));
    if (pSong)
	{

	    /*
	      // $$kk: 05.04.99
	      GM_Synth *pSynth = NULL;

	      // $$kk: 06.21.99
	      #if USE_EXTERNAL_SYNTH == TRUE

	      pSynth = PV_OpenExternalSynth(pSong);

	      #endif // USE_EXTERNAL_SYNTH == TRUE

	      if (pSynth == NULL)
	      {
	      pSynth = (GM_Synth *)XNewPtr((long)sizeof(GM_Synth));

	      pSynth->pProgramChangeProcPtr = PV_ProcessProgramChange;
	      pSynth->pNoteOffProcPtr = PV_ProcessNoteOff;
	      pSynth->pNoteOnProcPtr = PV_ProcessNoteOn;
	      pSynth->pPitchBendProcPtr = PV_ProcessPitchBend;
	      pSynth->pProcessControllerProcPtr = PV_ProcessController;
	      }

	      pSong->pSynth = pSynth;
	    */
	    pSong->pSynths = NULL;

	    // TEMP
	    /*
	      {
	      GM_Synth *pSynth1 = (GM_Synth *)XNewPtr((long)sizeof(GM_Synth));
	      if (!pSynth1)
	      {
	      // *pErr = MEMORY_ERR;
	      }
	      else
	      {
	      pSynth1->pProgramChangeProcPtr = PV_ProcessProgramChange;
	      pSynth1->pNoteOffProcPtr = PV_ProcessNoteOff;
	      pSynth1->pNoteOnProcPtr = PV_ProcessNoteOn;
	      pSynth1->pPitchBendProcPtr = PV_ProcessPitchBend;
	      pSynth1->pProcessControllerProcPtr = PV_ProcessController;
			
	      GM_AddSongSynth(pSong, pSynth1);
	      }
	      #if USE_EXTERNAL_SYNTH == TRUE
	      {
	      GM_Synth *pSynth2 = PV_OpenExternalSynth(pSong);
	      if (pSynth2)
	      {
	      GM_AddSongSynth(pSong, pSynth2);
	      }
	      }
	      #endif // USE_EXTERNAL_SYNTH == TRUE
	      }
	    */
	    // END TEMP

	    pSong->context = context;

	    // Fill in remap first
	    for (count = 0; count < (MAX_INSTRUMENTS*MAX_BANKS); count++)
		{
		    pSong->instrumentRemap[count] = (XLongResourceID)-1;		// no remap
		}

	    for (count = 0; count < MAX_CHANNELS; count++)
		{
		    pSong->firstChannelBank[count] = 0;
		    pSong->firstChannelProgram[count] = -1;
		}
	    PV_ConfigureInstruments(pSong);

	    pSong->defaultReverbType = GM_GetReverbType();
	    pSong->songID = songID;
	    pSong->songPitchShift = 0;
	    pSong->allowProgramChanges = TRUE;
	    pSong->defaultPercusionProgram = -1;

	    pSong->maxSongVoices = MusicGlobals->MaxNotes;
	    pSong->mixLevel = MusicGlobals->mixLevel;
	    pSong->maxEffectVoices = MusicGlobals->MaxEffects;

	    PV_SetTempo(pSong, 0L);
	    pSong->songVolume = MAX_SONG_VOLUME;
	}
    return pSong;
}

OPErr GM_StartLiveSong(GM_Song *pSong, XBOOL loadPatches)
{
    OPErr		theErr;
    short int	songSlot, count;

    theErr = NO_ERR;
    if (pSong)
	{
	    // first find a slot in the song queue
	    songSlot = -1;
	    for (count = 0; count < MAX_SONGS; count++)
		{
		    if (MusicGlobals->pSongsToPlay[count] == NULL)
			{
			    songSlot = count;
			    break;
			}
		}
	    if (songSlot != -1)
		{
		    if (loadPatches)
			{
			    for (count = 0; count < (MAX_INSTRUMENTS*MAX_BANKS); count++)
				{
				    GM_LoadSongInstrument(pSong, (XLongResourceID)count);
				}
			}

		    pSong->SomeTrackIsAlive = FALSE;
		    pSong->songFinished = FALSE;
		    pSong->AnalyzeMode = SCAN_NORMAL;

		    theErr = GM_ChangeSystemVoices(pSong->maxSongVoices,
						   pSong->mixLevel,
						   pSong->maxEffectVoices);

		    // Set reverb type now.
		    GM_SetReverbType(pSong->defaultReverbType);

		    // first time looping, and set mute tracks to off
		    pSong->songLoopCount = 0;
		    pSong->songMaxLoopCount = 0;
		    for (count = 0; count < MAX_TRACKS; count++)
			{
			    XClearBit(&pSong->trackMuted, count);
			    XSetBit(&pSong->soloTrackMuted, count);
			    pSong->pTrackPositionSave[count] = NULL;
			    pSong->trackTicksSave[count] = 0;
			}
		    pSong->loopbackSaved = FALSE;
		    pSong->loopbackCount = -1;
		    for (count = 0; count < MAX_CHANNELS; count++)
			{
			    XClearBit(&pSong->channelMuted, count);
			    XClearBit(&pSong->soloChannelMuted, count);
			    XSetBit(&pSong->allowPitchShift, count);
			}
		    XClearBit(&pSong->allowPitchShift, PERCUSSION_CHANNEL);		// don't allow pitch changes on percussion

		    pSong->velocityCurveType = DEFAULT_VELOCITY_CURVE;

		    // Start song playing now.
		    MusicGlobals->pSongsToPlay[songSlot] = pSong;
		}
	}
    return theErr;
}

OPErr GM_LoadSongInstrument(GM_Song *pSong, XLongResourceID instrument)
{
    register OPErr			theErr;

    theErr = BAD_INSTRUMENT;
    if ( pSong && (instrument >= 0) && (instrument < (MAX_INSTRUMENTS*MAX_BANKS)) )
	{
	    theErr = GM_LoadInstrument(pSong, instrument);
	    pSong->remapArray[instrument] = instrument;
	    pSong->instrumentRemap[instrument] = (XLongResourceID)-1;
	}
    return theErr;
}

OPErr GM_UnloadSongInstrument(GM_Song *pSong, XLongResourceID instrument)
{
    register OPErr				theErr;

    theErr = BAD_INSTRUMENT;
    if ( pSong && (instrument >= 0) && (instrument < (MAX_INSTRUMENTS*MAX_BANKS)) )
	{
	    theErr = GM_UnloadInstrument(pSong, instrument);
	    if (theErr == NO_ERR)
		{
		    pSong->remapArray[instrument] = instrument;
		    pSong->instrumentRemap[instrument] = (XLongResourceID)-1;
		}
	}
    return theErr;
}

// Returns TRUE if instrument is loaded, otherwise FALSE
XBOOL GM_IsSongInstrumentLoaded(GM_Song *pSong, XLongResourceID instrument)
{
    XBOOL	loaded;

    loaded = FALSE;
    if (pSong && (instrument >= 0) && (instrument < (MAX_INSTRUMENTS*MAX_BANKS)))
	{
	    //		if (MusicGlobals->InstrumentData[MusicGlobals->remapArray[instrument]])
	    if (pSong->instrumentData[instrument])
		{
		    loaded = TRUE;
		}
	}
    return loaded;
}


// Load the SongID from an external SONG resource and or a extneral midi resource.
//
//	threadContext		context of thread. passed all the way down to callbacks
//	context				context of song creation. C++ 'this' pointer, etc. this is a user variable
//						Its just stored in the GM_Song->context variable
//	songID				will be the ID used during playback
//	theExternalSong		standard SONG resource structure

//	theExternalMidiData	if not NULL, then will use this midi data rather than what is found in external SONG resource
//	midiSize			size of midi data if theExternalMidiData is not NULL
//						theExternalMidiData and midiSize is not used if the songType is RMF

//	pInstrumentArray	array, if not NULL will be filled with the instruments that need to be loaded.
//	loadInstruments		if not zero, then instruments and samples will be loaded
//	pErr				pointer to an OPErr

GM_Song * GM_LoadSong(void *threadContext, void *context, 
		      XShortResourceID songID, void *theExternalSong, 
		      void *theExternalMidiData, INT32 midiSize, 
		      XShortResourceID *pInstrumentArray, 
		      XBOOL loadInstruments, XBOOL ignoreBadInstruments,
		      OPErr *pErr) {
    GM_Song		*pSong;
    XLongResourceID	songObjectID;

    *pErr = MEMORY_ERR;
    pSong = NULL;
    if (theExternalSong)
	{
	    songObjectID = (XLongResourceID)XGetSongResourceObjectID(theExternalSong);
	    switch (XGetSongResourceObjectType(theExternalSong))
		{
		case SONG_TYPE_SMS:

		    pSong = PV_CreateSongFromMidi(songObjectID, theExternalMidiData, midiSize);

		    break;
		case SONG_TYPE_RMF:
		    if (theExternalMidiData == NULL)
			{	// we only support midi data via the resource manager
			    pSong = PV_CreateSongFromMidi(songObjectID, NULL, 0);
			}
		    else
			{
			    *pErr = PARAM_ERR;
			}
		    break;
		case SONG_TYPE_BAD:
		case SONG_TYPE_RMF_LINEAR:
		    /* satisfy compiler */
		    break;
		}
	}

    // load instruments
    if (pSong)
	{
	    pSong->pSynths = NULL;
	    pSong->context = context;

	    GM_MergeExternalSong(theExternalSong, songID, pSong);
	    pSong->ignoreBadInstruments = ignoreBadInstruments;
	    *pErr = GM_LoadSongInstruments(pSong, pInstrumentArray, loadInstruments);

	    if (*pErr)
		{
		    GM_FreeSong(threadContext, pSong);	// we ignore the error codes, because it should be ok to dispose
		    pSong = NULL;
		}
	    else
		{
		    // song length not calculated
		    pSong->songMidiTickLength = 0;
		    pSong->songMicrosecondLength = 0;
		    *pErr = NO_ERR;
		}
	}
    return pSong;
}

// Given a song pointer, this will attempt to free all memory related to the song: midi
// data, instruments, samples, etc. It can fail and will return STILL_PLAYING if
// midi data is still being accessed, or samples, or instruments.
//
// If you pass NULL, then this function will be called recursively will all songs
// currently playing.
OPErr GM_FreeSong(void *threadContext, GM_Song *pSong) {
    OPErr	err;
    XPTR	midiData;

    err = NO_ERR;
    GM_EndSong(threadContext, pSong);
    if (pSong) {
	// we must kill the notes because we are about to free
	// instrument memory
	GM_KillSongNotes(pSong);		
	// also remove the song's events from the interactive queue
	QGM_ClearSongFromQueue(pSong);
	    
	if (pSong->processingSlice == FALSE) {
	    GM_PauseSong(pSong);
	    midiData = (XPTR)pSong->midiData;		// save midi pointer now
	    pSong->midiData = NULL;					// and disable midi decoder now, just
	    // in case the decoder thread comes to life
	    GM_SetCacheSamples(pSong, FALSE);
	    err = GM_UnloadSongInstruments(pSong);
	    if (err == NO_ERR) {
		if (pSong->disposeSongDataWhenDone) {
		    XDisposePtr(midiData);
		}
		XDisposePtr((XPTR)pSong->controllerCallback);

#if 0 && USE_CREATION_API == TRUE
		if (pSong->pPatchInfo) {
		    PV_FreePatchInfo(pSong);	// must free before killing pSong pointer
		}
#endif

		XDisposePtr((XPTR)pSong);
	    } else {
		DEBUG_STR("GM_FreeSong::GM_UnloadSongInstruments::STILL_PLAYING\n");
	    }
	} else {
	    DEBUG_STR("GM_FreeSong::STILL_PLAYING\n");
	    err = STILL_PLAYING;
	}
    }
    return err;
}

// Return the length in MIDI ticks of the song passed

//	pSong	GM_Song structure. Data will be cloned for this function.
//	pErr		OPErr error type
UINT32 GM_GetSongTickLength(GM_Song *pSong, OPErr *pErr) {
    GM_Song		*theSong;
    UFLOAT		tickLength;

    *pErr = NO_ERR;
    tickLength = 0;
    if (pSong->songMidiTickLength == 0)
	{
	    theSong = (GM_Song *)XNewPtr((INT32)sizeof(GM_Song));
	    if (theSong)
		{
		    *theSong = *pSong;
		    theSong->controllerCallback = NULL;		// ignore callbacks
		    theSong->songEndCallbackPtr = NULL;
		    theSong->songTimeCallbackPtr = NULL;
		    theSong->metaEventCallbackPtr = NULL;
		    theSong->disposeSongDataWhenDone = FALSE;
		    PV_ClearSongInstruments(theSong);		// don't free the instruments

		    if (PV_ConfigureMusic(theSong) == NO_ERR)
			{
			    theSong->AnalyzeMode = SCAN_DETERMINE_LENGTH;
			    theSong->SomeTrackIsAlive = TRUE;
	
			    theSong->loopSong = FALSE;
			    theSong->songLoopCount = 0;
			    theSong->songMaxLoopCount = 0;
			    while (theSong->SomeTrackIsAlive)
				{
				    // don't need a thread context here because we don't callback
				    *pErr = PV_ProcessMidiSequencerSlice(NULL, theSong);
				    if (*pErr)
					{
					    break;
					}
				}
			    theSong->AnalyzeMode = SCAN_NORMAL;
			    pSong->songMidiTickLength = (UINT32)theSong->CurrentMidiClock;
			    tickLength = theSong->CurrentMidiClock;
			    pSong->songMicrosecondLength = (UINT32)theSong->songMicroseconds;
			    theSong->midiData = NULL;
			    theSong->songEndCallbackPtr = NULL;
			    theSong->disposeSongDataWhenDone = FALSE;

			    if (*pErr)
				{
				    tickLength = 0;
				}
			}
		    // don't need a thread context here because we don't callback
		    GM_FreeSong(NULL, theSong);	// we ignore the error codes, because it should be ok to dispose
		    // since this song was never engaged
		}
	}
    else
	{
	    tickLength = (UFLOAT)pSong->songMidiTickLength;
	}
    return (UINT32)tickLength;
}

// meta event callback that collects track names
#if USE_CREATION_API == TRUE
static void PV_TrackNameCallback(void *threadContext, GM_Song *pSong, char markerType, void *pMetaText, INT32 metaTextLength, short currentTrack)
{
    XBYTE **tnArray,*str;

    threadContext;
    if (markerType == 0x03) 
	{	// track name
	    if (currentTrack != -1)
		{
		    str = (XBYTE *)XNewPtr(metaTextLength+1);
		    if (str)
			{
			    XBlockMove(pMetaText,str+1,metaTextLength);
			    str[0] = (XBYTE)metaTextLength;
			    tnArray = (XBYTE **)pSong->metaEventCallbackReference;
			    tnArray[currentTrack] = str;
			}
		}
	}
}
#endif

// Go through the song and find all the program changes on each track
// theSongResource -> Standard Song Resource
//	pErr		OPErr error type
//  outSong      modified, loaded song
//  trackNames   array of track names extracted
#if USE_CREATION_API == TRUE
OPErr GM_GetSongInstrumentChanges(void *theSongResource, GM_Song **outSong, XBYTE **outTrackNames)
{
    OPErr		err;
    ScanMode 	saveScan;
    XBOOL		saveLoop,saveAlive;
    GM_Song		*theSong;
    void		*newMidiData;
	
    err = NO_ERR;
    // don't need a thread context here because we don't callback
    theSong = GM_LoadSong(NULL, NULL, 0, theSongResource, NULL, 0L, NULL, FALSE, TRUE, &err);
    if (!theSong)
	{
	    return err;
	}	
    err = PV_ConfigureMusic(theSong);
    // first pass: count the program changes, so we know how big to make our
    if (err == NO_ERR)
	{
	    theSong->pPatchInfo = (PatchInfo *)XNewPtr((INT32)sizeof(PatchInfo));
	    if (theSong->pPatchInfo)
		{
		    saveScan = theSong->AnalyzeMode;
		    theSong->AnalyzeMode = SCAN_COUNT_PATCHES;
		    saveLoop = theSong->loopSong;
		    theSong->loopSong = FALSE;
		    saveAlive = theSong->SomeTrackIsAlive;
		    theSong->SomeTrackIsAlive = TRUE;
		    while (theSong->SomeTrackIsAlive)
			{
			    // don't need a thread context here because we don't callback
			    err = PV_ProcessMidiSequencerSlice(NULL, theSong);
			    if (err)
				{
				    break;
				}
			}
		}
	    else
		{
		    err = MEMORY_ERR;
		}
	}
    // now theSong->pPatchInfo->pgmCount points to the number of program
    // changes without immediately preceding bank changes, and rsCount
    // is the number of program changes that were discovered to have been
    // written into the file in running status mode (implement this later)
    // done in two passes because we can't keep changing the midi data
    // pointer, so we have figure out how big it needs to be first and
    // move it in place

    if (err == NO_ERR)
	{
	    // expand the ptr enough to accomodate bank messages (4 bytes each)
	    newMidiData = XNewPtr(XGetPtrSize(theSong->midiData) + theSong->pPatchInfo->instrCount * 4);
	    XBlockMove(theSong->midiData,newMidiData,XGetPtrSize(theSong->midiData));
	    XDisposePtr(theSong->midiData);
	    theSong->midiData = newMidiData;

	    // start over
	    err = PV_ConfigureMusic(theSong);

	    // second pass: get the program changes, add bank events before each one
	    if (err == NO_ERR)
		{
		    GM_SetSongMetaEventCallback(theSong, PV_TrackNameCallback, (INT32)outTrackNames);
		    saveScan = theSong->AnalyzeMode;
		    theSong->AnalyzeMode = SCAN_FIND_PATCHES;
		    saveLoop = theSong->loopSong;
		    theSong->loopSong = FALSE;
		    saveAlive = theSong->SomeTrackIsAlive;
		    theSong->SomeTrackIsAlive = TRUE;
		    while (theSong->SomeTrackIsAlive) {
				// don't need a thread context here because we don't callback
			err = PV_ProcessMidiSequencerSlice(NULL, theSong);
			if (err) {
			    break;
			}
		    }
		}
	}
    // since there was an error, caller won't be using the song
    if (err)
	{
	    if (theSong->pPatchInfo)
		{
		    XDisposePtr(theSong->pPatchInfo);
		}
	    // don't need a thread context here because we don't callback
	    GM_FreeSong(NULL, theSong);	// we ignore the error codes, because it should be ok to dispose
	    // since this song was never engaged
	}
    else
	{
	    *outSong = theSong;
	}
    return err;
}
#endif	// USE_CREATION_API == TRUE

// Set the song position in midi ticks
OPErr GM_SetSongTickPosition(GM_Song *pSong, UINT32 songTickPosition)
{
    GM_Song		*theSong;
    OPErr		theErr;
    XBOOL	foundPosition;
    INT32		count;

    theErr = NO_ERR;
    theSong = (GM_Song *)XNewPtr((INT32)sizeof(GM_Song));
    if (theSong)
	{
	    *theSong = *pSong;
	    PV_ClearSongInstruments(theSong);		// don't free the instruments

	    if (PV_ConfigureMusic(theSong) == NO_ERR)
		{
		    theSong->AnalyzeMode = SCAN_DETERMINE_LENGTH;
		    theSong->SomeTrackIsAlive = TRUE;

		    theSong->loopSong = FALSE;
		    foundPosition = FALSE;
		    GM_PauseSong(pSong);
		    while (theSong->SomeTrackIsAlive)
			{
				// don't need a thread context here because we don't callback
			    theErr = PV_ProcessMidiSequencerSlice(NULL, theSong);
			    if (theErr)
				{
				    break;
				}
			    if (theSong->CurrentMidiClock > (UFLOAT)songTickPosition)
				{
				    foundPosition = TRUE;
				    break;
				}
			}
		    theSong->AnalyzeMode = SCAN_NORMAL;
		    theSong->loopSong = pSong->loopSong;
		    if (foundPosition)
			{
			    for (count = 0; count < (MAX_INSTRUMENTS*MAX_BANKS); count++)
				{
				    theSong->instrumentData[count] = pSong->instrumentData[count];
				}

				// $$kk: 07.20.99: this should be called through the synth function pointer.
				// but i don't think it needs to be called at all 'cause the song gets paused...?
				// GM_EndSongNotes(pSong);

			    *pSong = *theSong;		// copy over all song information at the new position
			    PV_ClearSongInstruments(theSong);		// don't free the instruments

			    GM_ResumeSong(pSong);
			}
		    // free duplicate song
		    theSong->midiData = NULL;
		    theSong->disposeSongDataWhenDone = FALSE;
		    theSong->songEndCallbackPtr = NULL;
		    theSong->songTimeCallbackPtr = NULL;
		    theSong->metaEventCallbackPtr = NULL;
		    theSong->controllerCallback = NULL;
		}
	    // don't need a thread context here because we don't callback
	    GM_FreeSong(NULL, theSong);	// we ignore the error codes, because it should be ok to dispose
	    // since this song was never engaged
	}
    return theErr;
}

UINT32 GM_SongTicks(GM_Song *pSong)
{
    if (pSong)
	{
	    if (GM_IsSongDone(pSong) == FALSE)
		{
		    return (UINT32)pSong->CurrentMidiClock;
		}
	}
    return 0L;
}

UINT32 GM_SongMicroseconds(GM_Song *pSong)
{
    if (pSong)
	{
	    if (GM_IsSongDone(pSong) == FALSE)
		{
		    return (UINT32)pSong->songMicroseconds;
		}
	}
    return 0L;
}

UINT32 GM_GetSongMicrosecondLength(GM_Song *pSong, OPErr *pErr)
{
    UFLOAT	ms;

    ms = 0;
    if (pErr && pSong)
	{
	    GM_GetSongTickLength(pSong, pErr);
	    ms = (UFLOAT)pSong->songMicrosecondLength;
	}
    return (UINT32)ms;
}

// Set the song position in microseconds
// $$kk: 08.12.98 merge: changed this method
OPErr GM_SetSongMicrosecondPosition(GM_Song *pSong, UINT32 songMicrosecondPosition)
{
    GM_Song		*theSong;
    OPErr		theErr;
    XBOOL	foundPosition;
    INT32		count;
    XBOOL	songPaused = FALSE;

    // $$kk: 02.10.98
    // the way this was, it paused the song, changed the position, and resumed.
    // if this is applied to a paused song, it suddenly starts playing again!
    // i am adding a mechanism to record whether the song was paused and only
    // resume it in that case.

    theErr = NO_ERR;
    theSong = (GM_Song *)XNewPtr((INT32)sizeof(GM_Song));
    if (theSong)
	{
	    *theSong = *pSong;
	    PV_ClearSongInstruments(theSong);		// don't free the instruments

	    if (PV_ConfigureMusic(theSong) == NO_ERR)
		{
		    theSong->AnalyzeMode = SCAN_DETERMINE_LENGTH;
		    theSong->SomeTrackIsAlive = TRUE;

		    theSong->loopSong = FALSE;
		    foundPosition = FALSE;

		    // $$kk: 02.10.98: keep track of whether song is paused
		    if (pSong->songPaused)
			songPaused = TRUE;

		    GM_PauseSong(pSong);
		    while (theSong->SomeTrackIsAlive)
			{
				// don't need a thread context here because we don't callback
			    theErr = PV_ProcessMidiSequencerSlice(NULL, theSong);
			    if (theErr)
				{
				    break;
				}
			    if (theSong->songMicroseconds > (UFLOAT)songMicrosecondPosition)
				{
				    foundPosition = TRUE;
				    break;
				}
			}
		    theSong->AnalyzeMode = SCAN_NORMAL;
		    theSong->loopSong = pSong->loopSong;
		    if (foundPosition)
			{
			    for (count = 0; count < (MAX_INSTRUMENTS*MAX_BANKS); count++)
				{
				    theSong->instrumentData[count] = pSong->instrumentData[count];
				}

				// $$kk: 07.20.99: this should be called through the synth function pointer.
				// but i don't think it needs to be called at all 'cause the song gets paused...?
				// GM_EndSongNotes(pSong);

			    *pSong = *theSong;		// copy over all song information at the new position
			    PV_ClearSongInstruments(theSong);		// don't free the instruments

				// $$kk: 02.10.98: do not resume if song was paused before
			    if (!songPaused)
				{
				    GM_ResumeSong(pSong);
				}
			}
		    // free duplicate song
		    theSong->midiData = NULL;
		    theSong->songEndCallbackPtr = NULL;
		    theSong->disposeSongDataWhenDone = FALSE;
		}
	    // don't need a thread context here because we don't callback
	    GM_FreeSong(NULL, theSong);	// we ignore the error codes, because it should be ok to dispose
	    // since this song was never engaged
	}
    return theErr;
}

// Return the used patch array of instruments used in the song passed.

//	theExternalSong		standard SONG resource structure
//	theExternalMidiData	if not NULL, then will use this midi data rather than what is found in external SONG resource
//	midiSize			size of midi data if theExternalMidiData is not NULL
//	pInstrumentArray	array, if not NULL will be filled with the instruments that need to be loaded.
//	pErr				pointer to an OPErr
#if USE_CREATION_API == TRUE
INT32 GM_GetUsedPatchlist(void *theExternalSong, void *theExternalMidiData, INT32 midiSize, 
			  XShortResourceID *pInstrumentArray, OPErr *pErr)
{
    GM_Song		*theSong;
    INT32		count;

    *pErr = NO_ERR;

    // don't need a thread context here because we don't callback
    theSong = GM_LoadSong(NULL, NULL, 0, theExternalSong, theExternalMidiData, midiSize,
			  pInstrumentArray, FALSE, TRUE, pErr);
    if (theSong)
	{
	    GM_FreeSong(NULL, theSong);	// we ignore the error codes, because it should be ok to dispose
	    // since this song was never engaged
	}

    count = 0;
    if (*pErr == NO_ERR)
	{
	    for (; count < MAX_INSTRUMENTS*MAX_BANKS; count++)
		{
		    if (pInstrumentArray[count] == (XShortResourceID)-1)
			{
			    break;
			}
		}
	}
    return count;
}
#endif	// USE_CREATION_API

void GM_SetVelocityCurveType(GM_Song *pSong, VelocityCurveType velocityCurveType)
{
    if (pSong)
	{
	    pSong->velocityCurveType = velocityCurveType;
	}
}


// EOF of GenSong.c
