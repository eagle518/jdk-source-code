/*
 * @(#)GenPatch.c	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "GenPatch.c"
**
**	Generalized Music Synthesis package. Part of SoundMusicSys.
**
**
**
** Overview
**	This contains code to load an maintain patches (instruments). This code library is specific
**	to the platform, but the API is independent in nature.
**
**
** Modification History:
**
**	7/7/95		Created
**	11/7/95		Major changes, revised just about everything.
**	11/20/95	Removed the BF_ flags, now you must walk through the union structure
**				Remove bit fields. BIT FIELDS DON'T WORK WITH MPW!!!!
**   12/6/95	Added reverb on/off bit
**	12/11/95	Enlarged PV_GetEnvelopeData to include the parsing for the external instrument
**				format
**	12/14/95	Modified PV_GetSampleData to accept an external snd handle and cache/convert
**				that into a sample
**	1/13/96		Modified PV_GetInstrument to only run SMOD on 8 bit mono data
**				Modified PV_GetEnvelopeData to support the extended format bit in the internal instrument
**	1/18/96		Spruced up for C++ extra error checking
**				Added MIN_LOOP_SIZE
**				Added GM_SetUsedInstrument & GM_IsInstrumentUsed & GM_IsInstrumentRangeUsed
**				and changed the way InstUseList is built. Now its only built with GM_LoadSongInstruments
**				and only loads the samples needed in the splits
**	1/28/96		Removed all uses of Resource Manager
**	1/29/96		Added useSampleRate factor for playback of instruments & sampleAndHold bits
**				Changed PV_CreateInstrumentFromResource to propigate features to splits
**	2/3/96		Removed extra includes
**	2/5/96		Removed unused variables. Working towards multiple songs
**	2/18/96		Added panPlacement to the GM_Instrument structure
**	2/21/96		Changed to support the change in XGetResourceAndDetach
**	3/25/96		Removed private GetLong to XGetLong
**	3/28/96		Changed PV_GetInstrument to support external instruments
**				Added PV_SetSampleIntoCache
**	4/10/96		Reworked the sample cache system to not clone the sample data
**	4/20/96		Added defines for max lfos, and max curves, MAX_LFOS & MAX_CURVES
**	4/21/96		Removed CPU edian issues by use XGetShort & XGetLong
**	5/12/96		Removed some warnings by fixing cast errors
**	5/18/96		More error correction in GM_LoadSongInstruments
**	5/19/96		Fixed an error condition in GM_UnloadInstrument
**	5/30/96		Added ignoreBadInstruments code
**	6/7/96		Added code to support the case inwhich there are no program changes
**				Added new version of GM_IsInstrumentUsed. Thanks Mark!
**	6/28/96		Changed PV_GetSoundResource to search for CSND first before SND
**	6/30/96		Fixed bug with PV_GetSoundResource (!)
**				Changed font and re tabbed
**	7/5/96		Fixed thread order problem with GM_UnloadInstrument & GM_UnloadSongInstruments
**	10/23/96	Removed reference to BYTE and changed them all to UBYTE or SBYTE
**				Added defines for instrument types
**				Changed GetKeySplitFromPtr to XGetKeySplitFromPtr
**	10/31/96	Added GM_IsInstrumentLoaded
**	12/9/96		Added GM_LoadInstrumentFromExternal
**	12/10/96	Fixed a ignoreBadInstruments bug that caused certain instruments to fail even though
**				the flag was set
**	12/30/96	Changed copyright
**	1/16/97		Changed LFORecord to LFORecords
**	1/28/97		Removed STAND_ALONE define
**	1/30/97		Changed PV_GetSoundResource to search for ESND also
**				Moved PV_GetSoundResource to DriverTools.c and renamed XGetSoundResourceByID
**	3/10/97		Added code to cache samples and not to remove them before the referenceCount
**				reaches zero
**	4/14/97		Changed KeymapSplit to GM_KeymapSplit
**	4/20/97		Changed PV_MusicIRQ to PV_ProcessMidiSequencerSlice
**	4/21/97		Added volume levels per keysplit
**	4/24/97		Fixed volume bug when value is 0. Now defaults to 100.
**	7/8/97		Discovered a bug in which a non GM instrument tries to load but fails, and the
**				fall back to GM is suppose to select a GM instrument and load it, but it fails
**				because GM_LoadInstrument knows its not used from the sequencer scan. Need to
**				set the instrument used flags. NOTE: DID NOT FIX.
**	8/26/97		Fixed various compiler warnings for Sun
**	10/15/97	Modified GM_UnloadInstrument to return an error if instrument is still in use
**				at the other thread level.
**				Eliminated the reserved_1 from PV_CreateInstrumentFromResource & PV_GetEnvelopeData &
**				PV_GetInstrument
**				Added OPErr to GM_FlushInstrumentCache so return STILL_PLAYING if instrument
**				is busy
**	10/16/97	Changed GM_LoadSongInstruments to use a XBOOL for a flag rather than an int
**				Renamed ignoreBadPatches to ignoreBadInstruments
**				Removed lame support for instrument caching from GM_UnloadInstrument
**	10/27/97	Changed GM_UnloadSongInstruments to handle errors better
**	10/28/97	Changed PV_FindCacheFromID, theID parameter from short to long
**				Reduced code size a bit in PV_GetSampleFromID
**	12/1/97		Fixed a memory leak with GM_UnloadSongInstruments. Loop broke when
**				no error happened. Needed to wrap test around break.
**	1/20/98		Fixed GM_LoadSongInstruments to return the correct error code when failing
**	2/5/98		Added a GM_Song pointer to PV_SetSampleIntoCache
**	2/8/98		Changed BOOL_FLAG to XBOOL
**	5/4/98		Eliminated neverInterpolate & enablePitchRandomness from the
**				GM_Instrument structure and all code that used it. Its not used.
**	6/18/98		Fixed problem with GM_LoadSongInstruments when using ignoreBadInstruments
**				it would still fail on certain types of percussion instruments
**	6/27/98		Fixed a sign problem when loading samples in PV_GetInstrument
**				and tweaked again.
**	7/1/98		Changed various API to use the new XResourceType and XLongResourceID
**	7/7/98		Modified PV_GetSampleData & PV_GetSampleFromID & & PV_ProcessSampleWithSMOD &
**				PV_CreateInstrumentFromResource to use new types
**	11/4/98		Changed PV_GetInstrument to handle the case in which useSoundModifierAsRootKey
**				is being used and the instrument tries to scale 8 bit samples.
**	11/6/98		Removed noteDecayPref from the GM_Waveform structure and changed various API's
**				to reflect that.
**	1/14/99		Added GM_LoadInstrumentFromExternalData
*/
/*****************************************************************************/

#define DISPLAY_INSTRUMENTS			0
#define DISPLAY_INSTRUMENTS_FILE	0

#if DISPLAY_INSTRUMENTS
#include <Types.h>
#include <OSUtils.h>
#include <Files.h>
#include <Resources.h>
#include <Retrace.h>
#include <Memory.h>
#include <Sound.h>
#include <Errors.h>
#include <Timer.h>
#include <Events.h>
#include <Gestalt.h>
#include <Sound.h>
#include <Lists.h>
//	#include <MacLibrary.h>
#include "TestLibrary.h"

static short drawDebug;
#endif

#include "X_API.h"
#include "X_Formats.h"
#include "GenSnd.h"
#include "GenPriv.h"
#include "SMOD.h"

// SMOD jump table
static void (*smod_functions[])(unsigned char *pSample, INT32 length, INT32 param1, INT32 param2) =
{
    VolumeAmpScaler,		//	Amplifier/Volume Scaler
    NULL,
    NULL,
    NULL
};

// Private Functions

// Return a pointer to a 'snd' resource from its ID.
// If a handle is passed, then its assumed to be a snd resource and is used instead. It is not disposed
// of after use, so you must dispose of it
static void * PV_GetSampleData(XLongResourceID theID, XPTR useThisSnd, CacheSampleInfo *pInfo)
{
    XPTR			theData, thePreSound;
    char			*theSound;
    CacheSampleInfo	theSoundData;
    SampleDataInfo	newSoundInfo;
    INT32			size;

    theSound = NULL;
    if (useThisSnd)
	{
	    theData = useThisSnd;
	}
    else
	{
	    theData = XGetSoundResourceByID(theID, &size);
	}
    if (theData)
	{
	    // convert snd resource into a simple pointer of data with information

	    thePreSound = XGetSamplePtrFromSnd(theData, &newSoundInfo);

	    if (newSoundInfo.pMasterPtr != theData)
		{	// this means that XGetSamplePtrFromSnd created a new sample
		    XDisposePtr(theData);
		}

	    if (thePreSound)
		{
		    // validate loop points
		    if ((newSoundInfo.loopStart > newSoundInfo.loopEnd) ||
			(newSoundInfo.loopEnd > newSoundInfo.frames) ||
			((newSoundInfo.loopEnd - newSoundInfo.loopStart) < MIN_LOOP_SIZE) )
			{
				// disable loops
			    newSoundInfo.loopStart = 0;
			    newSoundInfo.loopEnd = 0;
			}
		    theSoundData.theID = (short int)theID;
		    theSoundData.waveSize = newSoundInfo.size;
		    theSoundData.waveFrames = newSoundInfo.frames;
		    theSoundData.loopStart = newSoundInfo.loopStart;
		    theSoundData.loopEnd = newSoundInfo.loopEnd;
		    theSoundData.baseKey = newSoundInfo.baseKey;
		    theSoundData.bitSize = (char)newSoundInfo.bitSize;
		    theSoundData.channels = (char)newSoundInfo.channels;
		    theSoundData.rate = newSoundInfo.rate;

#if DISPLAY_INSTRUMENTS
		    DPrint(drawDebug, "---->Getting 'snd' ID %ld rate %lX loopstart %ld loopend %ld basekey %ld\r", (INT32)theID,
			   theSoundData.rate, theSoundData.loopStart,
			   theSoundData.loopEnd, (INT32)theSoundData.baseKey);
#endif
		    theSoundData.pSampleData = thePreSound;
		    theSoundData.pMasterPtr = newSoundInfo.pMasterPtr;
		    theSoundData.cacheBlockID = ID_INST;
		    *pInfo = theSoundData;
		    theSound = (char *)thePreSound;
		}
	}
    return theSound;
}

// Free cache entry
static void PV_FreeCacheEntry(CacheSampleInfo *pCache)
{
    if (pCache)
	{
	    if (pCache->pSampleData)
		{
		    XDisposePtr(pCache->pMasterPtr);
		}
	    XDisposePtr(pCache);
	}
}

// Given an ID this will return a pointer to the sample that matches that ID if it is loaded into the sample
// cache, otherwise NULL is returned.
static void * PV_FindSoundFromID(XLongResourceID theID)
{
    register short int	count;
    register void		*pSample;
    CacheSampleInfo		*pCache;

    pSample = NULL;
    for (count = 0; count < MAX_SAMPLES; count++)
	{
	    pCache = MusicGlobals->sampleCaches[count];
	    if (pCache)
		{
		    if (pCache->theID == theID)
			{
			    pSample = pCache->pSampleData;
			    break;
			}
		}
	}
    return pSample;
}

// Given an ID, this will return the cache entry
static CacheSampleInfo * PV_FindCacheFromID(INT32 theID)
{
    register short int	count;
    register void		*pSample;
    CacheSampleInfo		*pCache;

    pSample = NULL;
    for (count = 0; count < MAX_SAMPLES; count++)
	{
	    pCache = MusicGlobals->sampleCaches[count];
	    if (pCache)
		{
		    if (pCache->theID == theID)
			{
			    return pCache;
			}
		}
	}
    return NULL;
}

// Given a pointer this will return an index to the sample that matches that pointer if it is loaded into the sample
// cache, otherwise NULL is returned.
static short int PV_FindCacheIndexFromPtr(void *pInSample)
{
    register short int	count;
    register short int	sampleIndex;
    CacheSampleInfo		*pCache;

    sampleIndex = -1;
    for (count = 0; count < MAX_SAMPLES; count++)
	{
	    pCache = MusicGlobals->sampleCaches[count];
	    if (pCache)
		{
		    if (pCache->pSampleData == pInSample)
			{
			    sampleIndex = count;
			    break;
			}
		}
	}
    return sampleIndex;
}

/*
  // This will free a reference to the sound id that is passed.
  static void PV_FreeCacheEntryFromID(short int theID)
  {
  register short int 	count;
  CacheSampleInfo		*pCache;

  for (count = 0; count < MAX_SAMPLES; count++)
  {
  pCache = MusicGlobals->sampleCaches[count];
  if (pCache)
  {
  if (pCache->theID == theID)
  {
  pCache->referenceCount--;
  if (pCache->referenceCount == 0)
  {
  PV_FreeCacheEntry(pCache);
  MusicGlobals->sampleCaches[count] = NULL;
  }
  break;
  }
  }
  }
  }
*/

// This will free a cache reference to the sound pointer that is passed.
static void PV_FreeCacheEntryFromPtr(void *pSample)
{
    register short int 	count;
    CacheSampleInfo		*pCache;

    for (count = 0; count < MAX_SAMPLES; count++)
	{
	    pCache = MusicGlobals->sampleCaches[count];
	    if (pCache)
		{
		    if (pCache->pSampleData == pSample)
			{
			    pCache->referenceCount--;
			    if (pCache->referenceCount == 0)
				{
				    PV_FreeCacheEntry(pCache);
				    MusicGlobals->sampleCaches[count] = NULL;
				}
			    break;
			}
		}
	}
}

// Pass TRUE to cache samples, and share them. FALSE to create new copy for each sample
void GM_SetCacheSamples(GM_Song *pSong, XBOOL cacheSamples)
{
    if (pSong)
	{
	    pSong->cacheSamples = cacheSamples;
	}
    if (MusicGlobals)
	{
	    MusicGlobals->cacheSamples = cacheSamples;
	}
}

XBOOL GM_GetCacheSamples(GM_Song *pSong)
{
    XBOOL	cached;

    cached = FALSE;
    if (pSong)
	{
	    cached = pSong->cacheSamples;
	}
    return cached;
}

// This will place a sample into the sample cache. Used for eMidi files.
void PV_SetSampleIntoCache(GM_Song *pSong, XLongResourceID theID, XPTR pSndFormatData)
{
    register short int	count;
    register void		*pSample;
    CacheSampleInfo		info;

    GM_SetCacheSamples(pSong, TRUE);	// enable caching
    pSample = NULL;
    // first, find out if there is a sample already for this ID
    pSample = PV_FindSoundFromID(theID);
    if (pSample)
	{	// yes, there is so free it
	    count = PV_FindCacheIndexFromPtr(pSample);
	    MusicGlobals->sampleCaches[count]->referenceCount = 1;
	    PV_FreeCacheEntryFromPtr(pSample);
	}
    info.referenceCount = 1;
    pSample = PV_GetSampleData(theID, pSndFormatData, &info);
    if (pSample)
	{
	    for (count = 0; count < MAX_SAMPLES; count++)
		{
		    if (MusicGlobals->sampleCaches[count] == NULL)
			{
			    MusicGlobals->sampleCaches[count] = (CacheSampleInfo *)XNewPtr((INT32)sizeof(CacheSampleInfo));
			    if (MusicGlobals->sampleCaches[count])
				{
				    *MusicGlobals->sampleCaches[count] = info;
				    MusicGlobals->sampleCaches[count]->theID = theID;
				}
			    break;
			}
		}
	}
}

// This will return a pointer to a sound. If the sound is already loaded, then just the pointer is returned.
static void * PV_GetSampleFromID(XLongResourceID theID, CacheSampleInfo *pInfo)
{
    register short int	count;
    register void		*pSample;
    CacheSampleInfo		*pCache;
    CacheSampleInfo		*pNewCache;

    pSample = NULL;
    pCache = NULL;
    if (MusicGlobals->cacheSamples)
	{
	    pCache = PV_FindCacheFromID(theID);
	}
    if (pCache == NULL)
	{	// not loaded, so load it
	    pSample = PV_GetSampleData(theID, NULL, pInfo);
	    pInfo->referenceCount = 1;
	    if (pSample)
		{
		    for (count = 0; count < MAX_SAMPLES; count++)
			{
			    if (MusicGlobals->sampleCaches[count] == NULL)
				{
				    pNewCache = (CacheSampleInfo *)XNewPtr((INT32)sizeof(CacheSampleInfo));
				    if (pNewCache)
					{
					    pNewCache->theID = theID;
					    *pNewCache = *pInfo;
					    MusicGlobals->sampleCaches[count] = pNewCache;
					}
				    break;
				}
			}
		}
	}
    else
	{
	    pCache->referenceCount++;
	    *pInfo = *pCache;
	    pSample = pInfo->pSampleData;
	}
    return pSample;
}



// Process a sample pointer with a SMOD
static void PV_ProcessSampleWithSMOD(void *pSample, INT32 length, INT32 masterID,
				     XShortResourceID smodID, short param1, short param2)
{
    short int	sampleIndex;

    if ( (smodID < SMOD_COUNT) && smod_functions[smodID])
	{
	    sampleIndex = PV_FindCacheIndexFromPtr(pSample);
	    if (sampleIndex != -1)
		{
		    // Find sample, and remove its ID number so that other instrument that try to share this one
		    // will force a new copy to be loaded.
		    MusicGlobals->sampleCaches[sampleIndex]->theID = -masterID;		// SMOD's sounds are negitive of their masters

		    (*smod_functions[smodID])((unsigned char *)pSample, length, param1, param2);
		}
	    else
		{
		    //DEBUG_STR("\pSomething is wrong with sample cache. Can't find sample.");
		}
	}
}

// Given an external instrument resource and an internal resource type fill the envelope data
// and if not there, place a default envelope
static void PV_GetEnvelopeData(InstrumentResource	*theX, GM_Instrument *theI, INT32 theXSize)
{
    INT32					count, count2, lfoCount;
    INT32					size, unitCount, unitType, unitSubCount;
    unsigned short int		data;
    register char 			*pData, *pData2;
    register char 			*pUnit;
    register KeySplit 		*pSplits;
    register LFORecord		*pLFO;
    register ADSRRecord		*pADSR;
    register CurveRecord	*pCurve;
    XBOOL					disableModWheel;

    disableModWheel = FALSE;
    theI->volumeADSRRecord.currentTime = 0;
    theI->volumeADSRRecord.currentPosition = 0;
    theI->volumeADSRRecord.currentLevel = 0;
    theI->volumeADSRRecord.previousTarget = 0;
    theI->volumeADSRRecord.mode = 0;
    theI->volumeADSRRecord.sustainingDecayLevel = 65536;

    theI->LPF_frequency = 0;
    theI->LPF_resonance = 0;
    theI->LPF_lowpassAmount = 0;

    // fill default
    theI->volumeADSRRecord.ADSRLevel[0] = VOLUME_RANGE;
    theI->volumeADSRRecord.ADSRFlags[0] = ADSR_TERMINATE;
    theI->volumeADSRRecord.ADSRTime[0] = 0;
    theI->LFORecordCount = 0;
    theI->curveRecordCount = 0;
    theI->extendedFormat = FALSE;
    pUnit = NULL;
    size = theXSize;
    if (theX && size)
	{
	    if (theX->flags1 & ZBF_extendedFormat)
		{
		    // search for end of tremlo data $8000. If not there, don't walk past end of instrument
		    pSplits = (KeySplit *) ( ((char *)&theX->keySplitCount) + sizeof(short));
		    count = XGetShort(&theX->keySplitCount);
		    pData = (char *)&pSplits[count];
		    pData2 = (char *)theX;
		    size -= (INT32) (pData - pData2);
		    for (count = 0; count < size; count++)
			{
			    data = XGetShort(&pData[count]);
			    if (data == 0x8000)
				{
				    count += 4;								// skip past end token and extra word
				    data = (unsigned short)pData[count] + 1;			// get first string length;
				    count2 = (INT32)pData[count+data] + 1;			// get second string length
				    pUnit = (char *) (&pData[count + data + count2]);
				    // NOTE: src will be non aligned, possibly on a byte boundry.
				    break;
				}
			}
		    if (pUnit)
			{
			    theI->extendedFormat = TRUE;
			    pUnit += 12;		// reserved global space

			    unitCount = *pUnit;		// how many unit records?
			    pUnit++;					// byte
			    if (unitCount)
				{
				    lfoCount = 0;
				    for (count = 0; count < unitCount; count++)
					{
					    unitType = XGetLong(pUnit) & 0x5F5F5F5F;
					    pUnit += 4;	// long
					    switch (unitType)
						{
						case INST_EXPONENTIAL_CURVE:
						    if (theI->curveRecordCount >= MAX_CURVES)	// can only handle 4
							{
							    goto bailoninstrument;
							}
						    pCurve = &theI->curve[theI->curveRecordCount];
						    theI->curveRecordCount++;
						    pCurve->tieFrom = XGetLong(pUnit) & 0x5F5F5F5F; pUnit += 4;
						    pCurve->tieTo = XGetLong(pUnit) & 0x5F5F5F5F; pUnit += 4;
						    unitSubCount = *pUnit++;
						    if (unitSubCount > MAX_CURVES)
							{
							    goto bailoninstrument;
							}
						    pCurve->curveCount = (short int)unitSubCount;
						    for (count2 = 0; count2 < unitSubCount; count2++)
							{
							    pCurve->from_Value[count2] = *pUnit++;
							    pCurve->to_Scalar[count2] = XGetShort(pUnit);
							    pUnit += 2;
							}
						    // there's one extra slot in the definition to allow for this PAST the end of the 8 possible entries
						    pCurve->from_Value[count2] = 127;
						    pCurve->to_Scalar[count2] = pCurve->to_Scalar[count2];
						    break;
						case INST_ADSR_ENVELOPE:
						    unitSubCount = *pUnit;		// how many unit records?
						    pUnit++;					// byte
						    if (unitSubCount > ADSR_STAGES)
							{	// can't have more than ADSR_STAGES stages
							    goto bailoninstrument;
							}
						    pADSR = &theI->volumeADSRRecord;
						    for (count2 = 0; count2 < unitSubCount; count2++)
							{
							    pADSR->ADSRLevel[count2] = XGetLong(pUnit);
							    pUnit += 4;

							    pADSR->ADSRTime[count2] = XGetLong(pUnit);
							    pUnit += 4;

							    pADSR->ADSRFlags[count2] = XGetLong(pUnit) & 0x5F5F5F5F;
							    pUnit += 4;
							}
						    break;

						case INST_LOW_PASS_FILTER:		// low pass global filter parameters
						    theI->LPF_frequency = XGetLong(pUnit);
						    pUnit += 4;
						    theI->LPF_resonance = XGetLong(pUnit);
						    pUnit += 4;
						    theI->LPF_lowpassAmount = XGetLong(pUnit);
						    pUnit += 4;
						    break;

						case INST_DEFAULT_MOD:		// default mod wheel hookup
						    disableModWheel = TRUE;
						    break;

						    // LFO types
						case INST_PITCH_LFO:
						case INST_VOLUME_LFO:
						case INST_STEREO_PAN_LFO:
						case INST_STEREO_PAN_NAME2:
						case INST_LOW_PASS_AMOUNT:
						case INST_LPF_DEPTH:
						case INST_LPF_FREQUENCY:
						    if (lfoCount > MAX_LFOS)
							{
							    goto bailoninstrument;
							}
						    unitSubCount = *pUnit;		// how many unit records?
						    pUnit++;					// byte
						    if (unitSubCount > ADSR_STAGES)
							{	// can't have more than ADSR_STAGES stages
							    //DEBUG_STR("\p too many stages");
							    goto bailoninstrument;
							}
						    pLFO = &theI->LFORecords[lfoCount];
						    for (count2 = 0; count2 < unitSubCount; count2++)
							{
							    pLFO->a.ADSRLevel[count2] = XGetLong(pUnit);
							    pUnit += 4;
							    pLFO->a.ADSRTime[count2] = XGetLong(pUnit);
							    pUnit += 4;
							    pLFO->a.ADSRFlags[count2] = XGetLong(pUnit) & 0x5F5F5F5F;
							    pUnit += 4;
							}
						    pLFO->where_to_feed = unitType & 0x5F5F5F5F;

						    pLFO->period = XGetLong(pUnit);
						    pUnit += 4;
						    pLFO->waveShape = XGetLong(pUnit);
						    pUnit += 4;
						    pLFO->DC_feed = XGetLong(pUnit);
						    pUnit += 4;
						    pLFO->level = XGetLong(pUnit);
						    pUnit += 4;

						    pLFO->currentWaveValue = 0;
						    pLFO->currentTime = 0;
						    pLFO->LFOcurrentTime = 0;
						    pLFO->a.currentTime = 0;
						    pLFO->a.currentPosition = 0;
						    pLFO->a.currentLevel = 0;
						    pLFO->a.previousTarget = 0;
						    pLFO->a.mode = 0;
						    pLFO->a.sustainingDecayLevel = 65536;
						    lfoCount++;
						    break;
						}
					}

				    if ((lfoCount < (MAX_LFOS-1)) && (theI->curveRecordCount < MAX_CURVES) && (disableModWheel == FALSE))
					{
					    pCurve = &theI->curve[theI->curveRecordCount];
					    theI->curveRecordCount++;

					    // Create a straight-line curve function to tie mod wheel to pitch LFO
					    pCurve->tieFrom = MOD_WHEEL_CONTROL;
					    pCurve->tieTo = PITCH_LFO;
					    pCurve->curveCount = 2;
					    pCurve->from_Value[0] = 0;
					    pCurve->to_Scalar[0] = 0;
					    pCurve->from_Value[1] = 127;
					    pCurve->to_Scalar[1] = 256;
					    pCurve->from_Value[2] = 127;
					    pCurve->to_Scalar[2] = 256;

					    // Create a default pitch LFO unit to tie the MOD wheel to.
					    pLFO = &theI->LFORecords[lfoCount];
					    pLFO->where_to_feed = PITCH_LFO;
					    pLFO->period = 180000;
					    pLFO->waveShape = SINE_WAVE;
					    pLFO->DC_feed = 0;
					    pLFO->level = 64;
					    pLFO->currentWaveValue = 0;
					    pLFO->currentTime = 0;
					    pLFO->LFOcurrentTime = 0;
					    pLFO->a.ADSRLevel[0] = 65536;
					    pLFO->a.ADSRTime[0] = 0;
					    pLFO->a.ADSRFlags[0] = ADSR_TERMINATE;
					    pLFO->a.currentTime = 0;
					    pLFO->a.currentPosition = 0;
					    pLFO->a.currentLevel = 0;
					    pLFO->a.previousTarget = 0;
					    pLFO->a.mode = 0;
					    pLFO->a.sustainingDecayLevel = 65536;
					    lfoCount++;
					}

				    theI->LFORecordCount = (unsigned char)lfoCount;
				bailoninstrument:
				    ;
				}
			}
		}
	}
}

// Create instrument from 'snd' resource ID
static GM_Instrument * PV_CreateInstrumentFromResource(GM_Instrument *theMaster, XLongResourceID theID)
{
    GM_Instrument	*theI;
    UBYTE			*theSound;
    CacheSampleInfo	sndInfo;

    theI = NULL;
    theSound = (UBYTE *)PV_GetSampleFromID(theID, &sndInfo);
    if (theSound)
	{
	    theI = (GM_Instrument *)XNewPtr((INT32)sizeof(GM_Instrument));
	    if (theI)
		{
		    theI->u.w.theWaveform = (SBYTE *)theSound;

		    if (theMaster)
			{
			    theI->disableSndLooping = theMaster->disableSndLooping;
			    theI->playAtSampledFreq = theMaster->playAtSampledFreq;
			    theI->doKeymapSplit = FALSE;
			    theI->notPolyphonic = theMaster->notPolyphonic;
			    theI->avoidReverb = theMaster->avoidReverb;
			    theI->useSampleRate = theMaster->useSampleRate;
			    theI->sampleAndHold = theMaster->sampleAndHold;
			}
		    else
			{
			    theI->disableSndLooping = FALSE;
			    theI->playAtSampledFreq = FALSE;
			    theI->doKeymapSplit = FALSE;
			    theI->notPolyphonic = FALSE;
			    theI->avoidReverb = FALSE;
			    theI->useSampleRate = FALSE;
			    theI->sampleAndHold = FALSE;
			}
		    theI->u.w.bitSize = sndInfo.bitSize;
		    theI->u.w.channels = sndInfo.channels;
		    theI->u.w.waveformID = sndInfo.theID;
		    theI->u.w.waveSize = sndInfo.waveSize;
		    theI->u.w.waveFrames = sndInfo.waveFrames;
		    theI->u.w.startLoop = sndInfo.loopStart;
		    theI->u.w.endLoop = sndInfo.loopEnd;
		    theI->u.w.baseMidiPitch = (unsigned char)sndInfo.baseKey;
		    theI->u.w.sampledRate = sndInfo.rate;
		}
	}
    return theI;
}

// Instruments 0 to MAX_INSTRUMENTS*MAX_BANKS are the standard MIDI instrument placements.
// This will create an internal instrument from and external instrument. If theX is non-null then it
// will use that data to create the GM_Instrument
GM_Instrument * PV_GetInstrument(XLongResourceID theID, void *theExternalX, INT32 patchSize)
{
    GM_Instrument		*theI, *theS;
    InstrumentResource	*theX;
    INT32				size;
    short int 			count;
    UBYTE				*theSound;
    KeySplit			theXSplit;
    CacheSampleInfo		sndInfo;
    LOOPCOUNT			i;
    INT32				theSampleID;

    theI = NULL;
    theX = (InstrumentResource *)theExternalX;
    if (theExternalX == NULL)
	{
	    theX = (InstrumentResource *)XGetAndDetachResource(ID_INST, theID, &patchSize);
	}
    if (theX)
	{
	    if (XGetShort(&theX->keySplitCount) < 2)	// if its 1, then it has no splits
		{
		    // get the sample ID from a short, values can be negative
		    // then allow conversion to take place.

		    // NOTE:	I know this is awfull, but if you change it things will break. The
		    //			internal ID values are all 32 bit signed, and some of the external
		    //			file structures are 16 bit signed.
		    theSampleID = (INT32)((short)XGetShort(&theX->sndResourceID));

		    theSound = (UBYTE *)PV_GetSampleFromID((INT32)theSampleID, &sndInfo);
		    if (theSound)
			{
			    theI = (GM_Instrument *)XNewPtr((INT32)sizeof(GM_Instrument));
			    if (theI)
				{
				    theI->u.w.theWaveform = (SBYTE *)theSound;

				    theI->disableSndLooping = TEST_FLAG_VALUE(theX->flags1, ZBF_disableSndLooping);
				    theI->playAtSampledFreq = TEST_FLAG_VALUE(theX->flags2, ZBF_playAtSampledFreq);
				    theI->doKeymapSplit = FALSE;
				    theI->notPolyphonic = TEST_FLAG_VALUE(theX->flags2, ZBF_notPolyphonic);
				    theI->avoidReverb = TEST_FLAG_VALUE(theX->flags1, ZBF_avoidReverb);
				    theI->useSampleRate = TEST_FLAG_VALUE(theX->flags1, ZBF_useSampleRate);
				    theI->sampleAndHold = TEST_FLAG_VALUE(theX->flags1, ZBF_sampleAndHold);
				    theI->useSoundModifierAsRootKey = TEST_FLAG_VALUE(theX->flags2, ZBF_useSoundModifierAsRootKey);
				    PV_GetEnvelopeData(theX, theI, patchSize);		// get envelope

				    theI->u.w.bitSize = sndInfo.bitSize;
				    theI->u.w.channels = sndInfo.channels;
				    theI->u.w.waveformID = XGetShort(&theX->sndResourceID);
				    theI->u.w.waveSize = sndInfo.waveSize;
				    theI->u.w.waveFrames = sndInfo.waveFrames;
				    theI->u.w.startLoop = sndInfo.loopStart;
				    theI->u.w.endLoop = sndInfo.loopEnd;

				    theI->masterRootKey = XGetShort(&theX->midiRootKey);
				    theI->panPlacement = theX->panPlacement;
				    theI->u.w.baseMidiPitch = (unsigned char)sndInfo.baseKey;
				    theI->u.w.sampledRate = sndInfo.rate;

				    // NOTE!! If ZBF_useSoundModifierAsRootKey is TRUE, then we are using
				    // the Sound Modifier data blocks as a root key replacement for samples in
				    // the particular split
				    theI->miscParameter1 = XGetShort(&theX->miscParameter1);
				    theI->miscParameter2 = XGetShort(&theX->miscParameter2);
				    if (theI->useSoundModifierAsRootKey)
					{
					    theI->enableSoundModifier = FALSE;
					    if (theI->miscParameter2 == 0)		// Forces a default value of 0 to 100
						{
						    theI->miscParameter2 = 100;
						}
					}
				    else
					{
					    theI->enableSoundModifier = TEST_FLAG_VALUE(theX->flags2, ZBF_enableSoundModifier);
					    theI->smodResourceID = theX->smodResourceID;

					    // Process sample in place
					    if ( (theI->enableSoundModifier) && (theI->u.w.bitSize == 8) && (theI->u.w.channels == 1) )
						{
#if DISPLAY_INSTRUMENTS
						    DPrint(drawDebug, "---->Processing instrument %ld with SMOD %ld\r", (INT32)theID, (INT32)theI->smodResourceID);
#endif
						    PV_ProcessSampleWithSMOD(theI->u.w.theWaveform,
									     theI->u.w.waveSize,
									     theI->u.w.waveformID,
									     theI->smodResourceID,
									     theI->miscParameter1,
									     theI->miscParameter2);
						}
					}
				}
			}
		}
	    else
		{	// Keysplits
#if DISPLAY_INSTRUMENTS
		    DPrint(drawDebug, "----->Processing %ld keysplits\r", (INT32)XGetShort(&theX->keySplitCount));
#endif
		    size = XGetShort(&theX->keySplitCount) * (INT32)sizeof(GM_KeymapSplit);
		    size += (INT32)sizeof(GM_KeymapSplitInfo);

		    theI = (GM_Instrument *)XNewPtr(size + (INT32)sizeof(GM_Instrument));
		    if (theI)
			{
			    theI->disableSndLooping = TEST_FLAG_VALUE(theX->flags1, ZBF_disableSndLooping);
			    theI->doKeymapSplit = TRUE;
			    theI->notPolyphonic = TEST_FLAG_VALUE(theX->flags2, ZBF_notPolyphonic);
			    theI->avoidReverb = TEST_FLAG_VALUE(theX->flags1, ZBF_avoidReverb);
			    theI->useSampleRate = TEST_FLAG_VALUE(theX->flags1, ZBF_useSampleRate);
			    theI->sampleAndHold = TEST_FLAG_VALUE(theX->flags1, ZBF_sampleAndHold);
			    theI->playAtSampledFreq = TEST_FLAG_VALUE(theX->flags2, ZBF_playAtSampledFreq);
			    theI->useSoundModifierAsRootKey = TEST_FLAG_VALUE(theX->flags2, ZBF_useSoundModifierAsRootKey);
			    PV_GetEnvelopeData(theX, theI, patchSize);		// get envelope

			    theI->u.k.KeymapSplitCount = XGetShort(&theX->keySplitCount);
			    theI->u.k.defaultInstrumentID = (XShortResourceID)XGetShort(&theX->sndResourceID);

			    theI->masterRootKey = XGetShort(&theX->midiRootKey);
			    theI->panPlacement = theX->panPlacement;

				// NOTE!! If ZBF_useSoundModifierAsRootKey is TRUE, then we are using
				// the Sound Modifier data blocks as a root key replacement for samples in
				// the particular split
			    theI->miscParameter1 = XGetShort(&theX->miscParameter1);
			    theI->miscParameter2 = XGetShort(&theX->miscParameter2);
			    if (theI->useSoundModifierAsRootKey)
				{
				    theI->enableSoundModifier = FALSE;
				    if (theI->miscParameter2 == 0)		// Forces a default value of 0 to 100
					{
					    theI->miscParameter2 = 100;
					}
				}
			    else
				{
				    theI->enableSoundModifier = TEST_FLAG_VALUE(theX->flags2, ZBF_enableSoundModifier);
				    theI->smodResourceID = theX->smodResourceID;
				}

			    for (count = 0; count < theI->u.k.KeymapSplitCount; count++)
				{
				    XGetKeySplitFromPtr(theX, count, &theXSplit);
				    theI->u.k.keySplits[count].lowMidi = theXSplit.lowMidi;
				    theI->u.k.keySplits[count].highMidi = theXSplit.highMidi;
				    theI->u.k.keySplits[count].miscParameter1 = theXSplit.miscParameter1;
				    if (theI->useSoundModifierAsRootKey && (theXSplit.miscParameter2 == 0))		// Forces a default value of 0 to 100
					{
					    theXSplit.miscParameter2 = 100;
					}
				    theI->u.k.keySplits[count].miscParameter2 = theXSplit.miscParameter2;

				    //					if (GM_IsInstrumentRangeUsed(theID, (INT16)theXSplit.lowMidi, (INT16)theXSplit.highMidi))
				    {
#if DISPLAY_INSTRUMENTS
					DPrint(drawDebug, "------->Keysplit %ld low %ld high %ld\r", (INT32)count,
					       (INT32)theXSplit.lowMidi,
					       (INT32)theXSplit.highMidi);
#endif
					theS =  PV_CreateInstrumentFromResource(theI, (XLongResourceID)theXSplit.sndResourceID);
					theI->u.k.keySplits[count].pSplitInstrument = theS;
					if (theS)
					    {
						theS->useSoundModifierAsRootKey = theI->useSoundModifierAsRootKey;
						theS->miscParameter1 = theXSplit.miscParameter1;
						if (theS->useSoundModifierAsRootKey && (theXSplit.miscParameter2 == 0))		// Forces a default value of 0 to 100
						    {
							theXSplit.miscParameter2 = 100;
						    }
						theS->miscParameter2 = theXSplit.miscParameter2;

						theS->masterRootKey = theI->masterRootKey;
						theS->avoidReverb = theI->avoidReverb;
						theS->volumeADSRRecord = theI->volumeADSRRecord;
						for (i = 0; i < theI->LFORecordCount; i++)
						    {
							theS->LFORecords[i] = theI->LFORecords[i];
						    }
						theS->LFORecordCount = theI->LFORecordCount;
						for (i = 0; i < theI->curveRecordCount; i++)
						    {
							theS->curve[i] = theI->curve[i];
						    }
						theS->curveRecordCount = theI->curveRecordCount;
						theS->LPF_frequency = theI->LPF_frequency;
						theS->LPF_resonance = theI->LPF_resonance;
						theS->LPF_lowpassAmount = theI->LPF_lowpassAmount;

						if (theS->useSoundModifierAsRootKey == FALSE)
						    {
							// Process sample in place
							if ( (theS->enableSoundModifier) && (theS->u.w.bitSize == 8) && (theS->u.w.channels == 1) )
							    {
#if DISPLAY_INSTRUMENTS
								DPrint(drawDebug, "----->Processing instrument %ld with SMOD %ld\r", (INT32)theID, (INT32)theI->smodResourceID);
#endif
								PV_ProcessSampleWithSMOD(	theS->u.w.theWaveform,
												theS->u.w.waveSize,
												theXSplit.sndResourceID,
												theS->smodResourceID,
												theS->miscParameter1,
												theS->miscParameter2);
							    }
						    }
					    }
				    }
				}
			}
		}
	    if (theExternalX == NULL)
		{
		    XDisposePtr((XPTR)theX);
		}
	}
#if DISPLAY_INSTRUMENTS
    if (theI)
	{
	    DPrint(drawDebug, "-------->INST info: masterRootKey %ld\r", (INT32)theI->masterRootKey);
	}
#endif
    return theI;
}

// This will remap the 'from' instrument into the 'to' instrument.
OPErr GM_RemapInstrument(GM_Song *pSong, XLongResourceID from, XLongResourceID to)
{
    OPErr			theErr;

    theErr = BAD_INSTRUMENT;
    if (pSong && (from >= 0) && (from < MAX_INSTRUMENTS*MAX_BANKS) )
	{
	    if ( (to >= 0) && (to < MAX_INSTRUMENTS*MAX_BANKS) )
		{
		    if (to != from)
			{
			    if (pSong->instrumentData[from])
				{
				    pSong->remapArray[to] = from;
				    theErr = NO_ERR;
				}
			}
		    else
			{
			    theErr = NO_ERR;
			}
		}
	}
    return theErr;
}

XBOOL GM_AnyStereoInstrumentsLoaded(GM_Song *pSong)
{
    register GM_Instrument	*theI;
    register short int		instrument;
    XBOOL				stereoLoaded;

    stereoLoaded = FALSE;
    if (pSong)
	{
	    for (instrument = 0; instrument < (MAX_INSTRUMENTS*MAX_BANKS); instrument++)
		{
		    theI = pSong->instrumentData[instrument];
		    if (theI)
			{
			    if (theI->doKeymapSplit == FALSE)	// only look at wave data
				{
				    if (theI->u.w.channels > 1)
					{
					    stereoLoaded = TRUE;
					    break;
					}
				}
			}
		}
	}
    return stereoLoaded;
}

XBOOL GM_IsInstrumentLoaded(GM_Song *pSong, XLongResourceID instrument)
{
    if ( pSong && (instrument >= 0) && (instrument < (MAX_INSTRUMENTS*MAX_BANKS)) )
	{
	    if (pSong->instrumentData[instrument])
		{
		    return TRUE;
		}
	}
    return FALSE;
}

// Load an instrument based from a memory definition, and assign it to the instrument ID passed.
// This will unload the instrument if its already loaded.
OPErr GM_LoadInstrumentFromExternalData(GM_Song *pSong, XLongResourceID instrument,
					void *theX, UINT32 theXPatchSize)
{
    register GM_Instrument	*theI;
    register OPErr			theErr;

    theErr = MEMORY_ERR;
    if ( (instrument >= 0) && (instrument < (MAX_INSTRUMENTS*MAX_BANKS)) )
	{
	    if (pSong)
		{
		    theErr = NO_ERR;

		    theI = pSong->instrumentData[instrument];
		    if (theI)
			{
			    GM_UnloadInstrument(pSong, instrument);
			}
		    theI = PV_GetInstrument(instrument, theX, theXPatchSize);

		    if (theI)
			{
			    theI->usageReferenceCount++;		// increment reference count
			    pSong->instrumentData[instrument] = theI;
			    pSong->remapArray[instrument] = instrument;
			    pSong->instrumentRemap[instrument] = (XLongResourceID)-1;
			}
		    else
			{
			    theErr = BAD_INSTRUMENT;
			}
		}
	    else
		{
		    theErr = NOT_SETUP;
		}
	}
    else
	{
	    theErr = PARAM_ERR;
	}
    return theErr;
}


// Given an instrument number from 0 to MAX_INSTRUMENTS*MAX_BANKS, this will load that instrument into the musicvars globals, including
// splits
OPErr GM_LoadInstrument(GM_Song *pSong, XLongResourceID instrument)
{
    register GM_Instrument	*theI;
    register OPErr			theErr;

    theErr = MEMORY_ERR;
    if ( (instrument >= 0) && (instrument < (MAX_INSTRUMENTS*MAX_BANKS)) )
	{
	    if (pSong)
		{
		    theErr = NO_ERR;

		    theI = pSong->instrumentData[instrument];
		    // use cached instrument, if its not there, then load it
		    if (theI == NULL)
			{
			    theI = PV_GetInstrument(instrument, NULL, 0);
			}

		    if (theI)
			{
			    theI->usageReferenceCount++;		// increment reference count
			    pSong->instrumentData[instrument] = theI;
			    pSong->remapArray[instrument] = instrument;
			}
		    else
			{
			    theErr = BAD_INSTRUMENT;
			}
		}
	    else
		{
		    theErr = NOT_SETUP;
		}
	}
    else
	{
	    theErr = PARAM_ERR;
	}
    return theErr;
}

OPErr GM_UnloadInstrument(GM_Song *pSong, XLongResourceID instrument)
{
    register GM_Instrument		*theI;
    register GM_KeymapSplit		*k;
    register OPErr				theErr;
    register short int			splitCount;

    theErr = BAD_INSTRUMENT;
    if ( (instrument >= 0) && (instrument < (MAX_INSTRUMENTS*MAX_BANKS)) )
	{
	    if (pSong)
		{
		    theErr = NO_ERR;
		    theI = pSong->instrumentData[instrument];
		    if (theI)
			{
			    if (theI->processingSlice == FALSE)
				{
				    theI->usageReferenceCount--;		// decrement reference count
				    if (theI->usageReferenceCount == 0)
					{
					    pSong->instrumentData[instrument] = NULL;	// do this first so other threads won't walk into
					    // bad news land
					    if (theI->doKeymapSplit)
						{
						    k = theI->u.k.keySplits;
						    for (splitCount = 0; splitCount < theI->u.k.KeymapSplitCount; splitCount++)
							{
							    if (k->pSplitInstrument)
								{
								    if (k->pSplitInstrument->u.w.theWaveform)
									{
									    PV_FreeCacheEntryFromPtr(k->pSplitInstrument->u.w.theWaveform);
									}
								    XDisposePtr(k->pSplitInstrument);
								}
							    k++;
							}
						}
					    else
						{
						    if (theI->u.w.theWaveform)
							{
							    PV_FreeCacheEntryFromPtr(theI->u.w.theWaveform);
							}
						}
					    XDisposePtr((void FAR *)theI);
					}

				    else
					{ 	// duplicate reference
					    theErr = NO_ERR;
					}
				}
			    else
				{
				    theErr = STILL_PLAYING;
				}
			}
		    //			else
		    //			{
		    //				theErr = BAD_INSTRUMENT;
		    //			}
		}
	    else
		{
		    theErr = NOT_SETUP;
		}
	}
    else
	{
	    theErr = PARAM_ERR;
	}
    return theErr;
}


// Scan the midi file and determine which instrument that need to be loaded and load them.
OPErr GM_LoadSongInstruments(GM_Song *theSong, XShortResourceID *pArray, XBOOL loadInstruments)
{
    register INT32	count, loadCount, instCount, newCount;
    XBOOL			loopSongSave;
    OPErr			theErr;
    SBYTE			remapUsedSaved[MAX_INSTRUMENTS];
    SBYTE			remapUsed[MAX_INSTRUMENTS];

#if DISPLAY_INSTRUMENTS
    {
	char text[256];

	drawDebug = DNew((char *)"\pInstruments to load");
	sprintf(text, "SONG %ld debug file", (INT32)theSong->songID);
#if DISPLAY_INSTRUMENTS_FILE
	DAttachFile(drawDebug, ctop(text));
	DPrint(drawDebug, "Writing output to file: Ô%pÕ\r", text);
#endif
    }
#endif
    // Set the sequencer to mark instruments only
    theErr = NO_ERR;

    theSong->pUsedPatchList = (SBYTE *)XNewPtr((MAX_INSTRUMENTS*MAX_BANKS*128L) / 8);
    if (theSong->pUsedPatchList)
	{
	    for (count = 0; count < MAX_INSTRUMENTS*MAX_BANKS; count++)
		{
		    theSong->remapArray[count] = count;
		    if (pArray)
			{
			    pArray[count] = (XShortResourceID)-1;
			}
		}
	    for (count = 0; count < MAX_CHANNELS; count++)
		{
		    theSong->firstChannelBank[count] = 0;
		    theSong->firstChannelProgram[count] = -1;
		}
	    theErr = PV_ConfigureMusic(theSong);
	    if (theErr == NO_ERR)
		{
		    if (theSong->defaultPercusionProgram == -1)
			{
			    theSong->channelBank[PERCUSSION_CHANNEL] = 0;
			    theSong->firstChannelBank[PERCUSSION_CHANNEL] = 0;
			}
		    else
			{
			    if (theSong->defaultPercusionProgram)
				{
				    theSong->firstChannelProgram[PERCUSSION_CHANNEL] = theSong->defaultPercusionProgram;
				    GM_SetUsedInstrument(theSong, (XLongResourceID)theSong->defaultPercusionProgram, 60, TRUE);
				}
			}

		    theSong->AnalyzeMode = SCAN_SAVE_PATCHES;
		    theSong->SomeTrackIsAlive = TRUE;

		    loopSongSave = theSong->loopSong;
		    theSong->loopSong = FALSE;
		    while (theSong->SomeTrackIsAlive)
			{
			    theErr = PV_ProcessMidiSequencerSlice(NULL, theSong);
			    if (theErr)
				{
				    break;
				}
			}
		    theSong->AnalyzeMode = SCAN_NORMAL;
		    theSong->loopSong = loopSongSave;

		    if (theErr == NO_ERR)
			{
				// are we trying to load any instruments? This is for the case were there are no program changes. We must do something
			    newCount = FALSE;
			    for (count = 0; count < MAX_CHANNELS; count++)
				{
				    if (count != PERCUSSION_CHANNEL)	// only look at non percussion channels
					{
					    if (theSong->firstChannelProgram[count] != -1)
						{
						    newCount = TRUE;
						    break;
						}
					}
				}
			    if (newCount == FALSE)
				{	// there have been no program changes. So set up just the piano in all channels
				    for (count = 0; count < MAX_CHANNELS; count++)
					{
					    theSong->firstChannelProgram[count] = 0;
					    theSong->channelProgram[count] = 0;
					}
				    GM_SetUsedInstrument(theSong, 0, -1, TRUE);		// load the entire piano
				}
#if DISPLAY_INSTRUMENTS
			    DPrint(drawDebug, "Loading instruments:\r");
#endif
			    instCount = 0;
			    for (count = 0; count < MAX_INSTRUMENTS*MAX_BANKS; count++)
				{
				    // instrument needs to be loaded
				    if (GM_IsInstrumentUsed(theSong, count, -1))
					{
#if DISPLAY_INSTRUMENTS
					    DPrint(drawDebug, "Instrument %ld: ", (INT32)count);
#endif
					    loadCount = theSong->instrumentRemap[count];
					    if (loadCount == -1)
						{
						    loadCount = count;
						}
#if DISPLAY_INSTRUMENTS
					    else
						{
						    DPrint(drawDebug, "remapped to %ld ", (INT32)loadCount);
						}
#endif
#if DISPLAY_INSTRUMENTS
					    DPrint(drawDebug, "loading instrument %ld\r", loadCount);
#endif
					    if (pArray)
						{
						    pArray[instCount++] = (short)loadCount;
						}

					    if (loadInstruments)
						{
						    if (loadCount != count)
							{
							    GM_GetInstrumentUsedRange(theSong, loadCount, remapUsedSaved);		// save
							    GM_GetInstrumentUsedRange(theSong, count, remapUsed);
							    GM_SetInstrumentUsedRange(theSong, loadCount, remapUsed);
							}
						    theErr = GM_LoadInstrument(theSong, loadCount);
						    if (theErr != NO_ERR)
							{	// if the instrument is some other bank, then go back to the standard GM bank before failing
							    if (loadCount > MAX_INSTRUMENTS)
								{
#if DISPLAY_INSTRUMENTS
								    DPrint(drawDebug, "Failed loading extra bank instrument %ld, falling back to GM.\r", loadCount);
#endif
								    newCount = (loadCount % MAX_INSTRUMENTS);
								    newCount += ((loadCount / MAX_INSTRUMENTS) & 1) * MAX_INSTRUMENTS;
								    loadCount = newCount;

#if DISPLAY_INSTRUMENTS
								    DPrint(drawDebug, "Loading instrument %ld\r", loadCount);
#endif
								    theErr = GM_LoadInstrument(theSong, loadCount);
								    if (theSong->ignoreBadInstruments)
									{
									    theErr = NO_ERR;
									}
								}
							    else
								{	// we are in GM, so check our ignore flag
								    if (theSong->ignoreBadInstruments)
									{
									    theErr = NO_ERR;
									}
								}
							}
						    if (loadCount != count)
							{
							    GM_SetInstrumentUsedRange(theSong, loadCount, remapUsedSaved);		// save
							}
						    if (theErr)
							{
#if DISPLAY_INSTRUMENTS
							    DPrint(drawDebug, "Failed to load instrument %ld (%ld)\r", (INT32)loadCount, (INT32)theErr);
#endif
							    break;
							}
						    theErr = GM_RemapInstrument(theSong, loadCount, count);	// remap from: to
						    // we are in GM, so check our ignore flag
						    if (theSong->ignoreBadInstruments)
							{
							    theErr = NO_ERR;
							}
						}
					}
				}
			}
		}

	    if (theErr != NO_ERR)
		{
		    GM_UnloadSongInstruments(theSong);		// ignore error
		}
	    XDisposePtr(theSong->pUsedPatchList);
	    theSong->pUsedPatchList = NULL;
	}
    else
	{
	    theErr = MEMORY_ERR;
	}
#if DISPLAY_INSTRUMENTS
    DPrint(drawDebug, "\rClick to exit");
    while (Button() == FALSE) {};
    while (Button()) {};
    FlushEvents(mDownMask, 0);
    DCopy(drawDebug);
    DDispose(drawDebug);
#endif

    return theErr;
}

OPErr GM_UnloadSongInstruments(GM_Song *pSong)
{
    short int	count;
    OPErr		err;

    err = NO_ERR;
    if (pSong)
	{
	    for (count = 0; count < (MAX_INSTRUMENTS*MAX_BANKS); count++)
		{
		    if (pSong->instrumentData[count])
			{
			    err = GM_UnloadInstrument(pSong, count);
			    if (err == NO_ERR)
				{
				    pSong->instrumentData[count] = NULL;		// redundant, but clear
				}
			    else
				{
				    break;
				}
			}
		}
	}
    return err;
}

// Set the patch & key used bit. Pass -1 in theKey to set all the keys in that patch
void GM_SetUsedInstrument(GM_Song *pSong, XLongResourceID thePatch, INT16 theKey, XBOOL used)
{
    UINT32	bit, count;

    if (pSong && pSong->pUsedPatchList)
	{
	    if (theKey != -1)
		{
		    bit = ((INT32)thePatch * 128L) + (INT32)theKey;
		    //			if (bit < (MAX_INSTRUMENTS*MAX_BANKS*128L))
		    {
			if (used)
			    {
				XSetBit((void *)pSong->pUsedPatchList, bit);
			    }
			else
			    {
				XClearBit((void *)pSong->pUsedPatchList, bit);
			    }
		    }
		}
	    else
		{
		    for (count = 0; count < MAX_INSTRUMENTS; count++)
			{
			    bit = ((INT32)thePatch * 128L) + count;
			    //				if (bit < (MAX_INSTRUMENTS*MAX_BANKS*128L))
			    {
				if (used)
				    {
					XSetBit((void *)pSong->pUsedPatchList, bit);
				    }
				else
				    {
					XClearBit((void *)pSong->pUsedPatchList, bit);
				    }
			    }
			}
		}
	}
}


XBOOL GM_IsInstrumentUsed(GM_Song *pSong, XLongResourceID thePatch, INT16 theKey)
{
#if 1
    // faster code
    if (pSong && pSong->pUsedPatchList)
	{
	    register UINT32   bit, count,byteIndex,bitIndex;
	    register unsigned char  *patchlist = (unsigned char*)pSong->pUsedPatchList;

	    bit = (INT32)thePatch << 7;
	    if (theKey != -1)
		{
		    bit += theKey;
		    if (bit < (MAX_INSTRUMENTS*MAX_BANKS*128L))
			{
			    byteIndex = bit >> 3;
			    bitIndex = bit & 7;
			    return ( (*(patchlist + byteIndex) & (1<<bitIndex)) != 0);
			}
		}
	    else
		{
		    for (count = 0; count < MAX_INSTRUMENTS; count++, bit++ )
			{
			    if (bit < (MAX_INSTRUMENTS*MAX_BANKS*128L))
				{
				    byteIndex = bit >> 3;
				    bitIndex = bit & 7;
				    if ( *(patchlist + byteIndex) & (1<<bitIndex) )
					{
					    return TRUE;
					}
				}
			}
		}
	    return FALSE;
	}
    return TRUE;
#else
    register UINT32	bit, count;
    register XBOOL		used;

    used = FALSE;
    if (pSong && pSong->pUsedPatchList)
	{
	    if (theKey != -1)
		{
		    bit = ((INT32)thePatch * 128L) + (INT32)theKey;
		    //			if (bit < (MAX_INSTRUMENTS*MAX_BANKS*128L))
		    {
			used = XTestBit((void *)pSong->pUsedPatchList, bit);
		    }
		}
	    else
		{
		    for (count = 0; count < MAX_INSTRUMENTS; count++)
			{
			    bit = ((INT32)thePatch * 128L) + count;
			    //				if (bit < (MAX_INSTRUMENTS*MAX_BANKS*128L))
			    {
				used = XTestBit((void *)pSong->pUsedPatchList, bit);
				if (used)
				    {
					break;
				    }
			    }
			}
		}
	}
    else
	{
	    used = TRUE;
	}
    return used;
#endif
}

void GM_GetInstrumentUsedRange(GM_Song *pSong, XLongResourceID thePatch, SBYTE *pUsedArray)
{
    register UINT32 bit, count;

    if (pSong)
	{
	    for (count = 0; count < MAX_INSTRUMENTS; count++)
		{
		    bit = ((INT32)thePatch * 128L) + count;
		    pUsedArray[count] = XTestBit((void *)pSong->pUsedPatchList, bit);
		}
	}
}

void GM_SetInstrumentUsedRange(GM_Song *pSong, XLongResourceID thePatch, SBYTE *pUsedArray)
{
    register INT16		count;

    if (pSong)
	{
	    for (count = 0; count < MAX_INSTRUMENTS; count++)
		{
		    GM_SetUsedInstrument(pSong, thePatch, count, pUsedArray[count]);
		}
	}
}


XBOOL GM_IsInstrumentRangeUsed(GM_Song *pSong, XLongResourceID thePatch, INT16 theLowKey, INT16 theHighKey)
{
#if 0
    // enable this test, to always load samples from a particular instrument
    return TRUE;
#else
    register XBOOL	used;
    register INT16		count;

    used = FALSE;
    if (pSong)
	{
	    for (count = theLowKey; count <= theHighKey; count++)
		{
		    used = GM_IsInstrumentUsed(pSong, thePatch, count);
		    if (used)
			{
			    break;
			}
		}
	}
#if DISPLAY_INSTRUMENTS
    DPrint(drawDebug, "---->Testing INST %ld - key range (%ld to %ld) = %s\r", (INT32)thePatch,
	   (INT32)theLowKey,
	   (INT32)theHighKey,
	   (used) ? "VALID" : "FAILED");
#endif

    return used;
#endif
}


// EOF of GenPatch.c
