/*
 * @(#)X_EditorTools.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	X_EditorTools.c
**
**	Tools for editors create and manipulating RMF data
**
**	© Copyright 1998-1999 Headspace, Inc, All Rights Reserved.
**	Written by Steve Hales
**
**	Headspace products contain certain trade secrets and confidential and
**	proprietary information of Headspace.  Use, reproduction, disclosure
**	and distribution by any means are prohibited, except pursuant to
**	a written license from Headspace. Use of copyright notice is
**	precautionary and does not imply publication or disclosure.
**
**	Restricted Rights Legend:
**	Use, duplication, or disclosure by the Government is subject to
**	restrictions as set forth in subparagraph (c)(1)(ii) of The
**	Rights in Technical Data and Computer Software clause in DFARS
**	252.227-7013 or subparagraphs (c)(1) and (2) of the Commercial
**	Computer Software--Restricted Rights at 48 CFR 52.227-19, as
**	applicable.
**
**	Confidential-- Internal use only
**
**	History	-
**	12/16/98	Created. Pulled from MacOS specific editor codebase
**				Moved editor specific cross platform code from DriverTools.c
**	1/13/99		Changed XGatherAllSoundsFromAllInstruments & XGetSamplesFromInstruments
**				to allocate more memory
**	2/5/98		Added XCopySongMidiResources & XCopyInstrumentResources & XCopySndResources
*/
/*****************************************************************************/

#include "X_EditorTools.h"


#if USE_CREATION_API == TRUE

// Set a keysplit entry. The result will be ordered for the 68k CPU
void XSetKeySplitFromPtr(InstrumentResource *theX, short int entry, KeySplit *keysplit)
{
    KeySplit	*pSplits;
    short int	count;

    count = (short)XGetShort(&theX->keySplitCount);
    if ( (count) && (entry < count) )
	{
	    pSplits = (KeySplit *) ( ((unsigned char *)&theX->keySplitCount) + sizeof(short int));
	    pSplits[entry] = *keysplit;

	    XPutShort(&pSplits[entry].sndResourceID, (unsigned short)keysplit->sndResourceID);
	    XPutShort(&pSplits[entry].miscParameter1, (unsigned short)keysplit->miscParameter1);
	    XPutShort(&pSplits[entry].miscParameter2, (unsigned short)keysplit->miscParameter2);
	}
    else
	{
	    XSetMemory(keysplit, (long)sizeof(KeySplit), 0);
	}
}

// Increase the number of key splits. This will not clean the entry's that are added.
// This will add from the end. This will freeup the passed InstrumentResource
InstrumentResource * XAddKeySplit(InstrumentResource *theX, short int howMany)
{
    long				size, moveSize, size2;
    short int			total;
    KeySplit 			*pSplits;
    char				*pSrc, *pDest;
    InstrumentResource	*theNewX;

    theNewX = NULL;
    if (theX)
	{
	    total = (short)XGetShort(&theX->keySplitCount);
	    size = XGetPtrSize(theX);
	    size2 =  (long)(sizeof(KeySplit) * howMany);
	    theNewX = (InstrumentResource *)XNewPtr(size2 + size);
	    if (theNewX)
		{
		    XBlockMove(theX, theNewX, size);	// make copy
		    XPutShort(&theNewX->keySplitCount, (unsigned short)(total + howMany));
		    // must move all zones down
		    pSplits = (KeySplit *) ( ((unsigned char *)&theX->keySplitCount) + sizeof(unsigned short int));
		    pSrc = (char *)&pSplits[total];

		    pSplits = (KeySplit *) ( ((unsigned char *)&theNewX->keySplitCount) + sizeof(unsigned short int));
		    pDest = (char *)&pSplits[total + howMany];
		    moveSize =  size - (pSrc - (char *)theX);
		    // this move must handle overlapping data
		    XBlockMove(pSrc, pDest, moveSize);
		}
	}
    return theNewX;
}

// Decrease the number of key splits. This will delete from the end
InstrumentResource * XRemoveKeySplit(InstrumentResource *theX, short int howMany)
{
    long				size, moveSize, size2;
    short int			total;
    KeySplit 			*pSplits;
    char				*pSrc, *pDest;
    InstrumentResource	*theNewX;

    theNewX = NULL;
    if (theX)
	{
	    total = (short)XGetShort(&theX->keySplitCount);
	    if (howMany > total)
		{
		    howMany = total;
		}

	    size = XGetPtrSize(theX);
	    size2 =  (long)(sizeof(KeySplit) * howMany);
	    theNewX = (InstrumentResource *)XNewPtr(size - size2);
	    if (theNewX)
		{
		    XBlockMove(theX, theNewX, size - size2);	// make copy
		    XPutShort(&theNewX->keySplitCount, (unsigned short)(total - howMany));
		    // must move all zones down
		    pSplits = (KeySplit *) ( ((unsigned char *)&theX->keySplitCount) + sizeof(unsigned short int));
		    pSrc = (char *)&pSplits[total];		// go to byte at the end

		    pSplits = (KeySplit *) ( ((unsigned char *)&theNewX->keySplitCount) + sizeof(unsigned short int));
		    pDest = (char *)&pSplits[total - howMany];

		    moveSize =  size - (pSrc - (char *)theX);
		    // this move must handle overlapping data
		    XBlockMove(pSrc, pDest, moveSize);
		}
	}
    return theNewX;
}

/*
** Return the number of INST's and an array of resource IDs
*/
short int XGetInstrumentArray(XShortResourceID *instArray, short maxArraySize)
{
    short int	icount, totalCount;
    long		size;
    XPTR		theRes;

    /* Collect INST resources
     */
    totalCount = 0;
    if (instArray)
	{
	    for (icount = 0; icount < maxArraySize; icount++)
		{
		    theRes = XGetAndDetachResource(ID_INST, icount, &size);
		    if (theRes)
			{
			    XDisposePtr(theRes);
			    instArray[totalCount++] = (XShortResourceID)icount;
			    if (totalCount > maxArraySize)
				{
				    totalCount = maxArraySize;
				    break;
				}
			}
		}
	    XBubbleSortArray((short *)instArray, (short)totalCount);
	}
    return totalCount;
}

XBOOL XIsSoundUsedInInstrument(InstrumentResource *theX, XShortResourceID sampleSoundID)
{
    XShortResourceID	sndArray[MAX_SAMPLES];		/* Max samples per instrument */
    short int			count, total;
    XBOOL				used;

    used = FALSE;
    total = XCollectSoundsFromInstrument(theX, sndArray, MAX_SAMPLES);
    for (count = 0; count < total; count++)
	{
	    if (sndArray[count] == sampleSoundID)
		{
		    used = TRUE;
		    break;
		}
	}
    return used;
}

// Renumber sample ID that is used in and instrument
void XRenumberSampleInsideInstrument(InstrumentResource *theX, XShortResourceID originalSampleID, 
				     XShortResourceID newSampleID)
{
    short int	count, total;
    KeySplit	split;

    total = theX->keySplitCount;
    if (total == 0)
	{
	    XPutShort(&theX->sndResourceID, (unsigned short)newSampleID);
	}
    else
	{
	    for (count = 0; count < total; count++)
		{
		    XGetKeySplitFromPtr(theX, count, &split);
		    if (split.sndResourceID == originalSampleID)
			{
			    split.sndResourceID = newSampleID;
			    XSetKeySplitFromPtr(theX, count, &split);
			}
		}
	}
}

/*
**	Given an INST resource, this will return an array of snd resources
**	that are used for this instrument
*/
short int XCollectSoundsFromInstrument(InstrumentResource *theX, XShortResourceID *sndArray, short maxArraySize)
{
    short int			sndOutCount, sndCount, splitCount, count, count2;
    KeySplit			theSplit;
    XShortResourceID	countSndArray[MAX_SAMPLES];		/* Max samples per instrument */
    XBOOL				goodSound;
    XShortResourceID	soundID;

    sndCount = 0;
    sndOutCount = 0;
    if (theX)
	{
	    splitCount = (short)XGetShort(&theX->keySplitCount);
	    if ( splitCount == 0)
		{
		    if (maxArraySize > 1)
			{
			    sndCount = 1;
			    countSndArray[0] = (XShortResourceID)XGetShort(&theX->sndResourceID);
			    if (countSndArray[0] == (XShortResourceID)-1)
				{
				    sndCount = 0;
				}
			}
		}
	    else
		{
		    if (maxArraySize > splitCount)
			{
			    sndCount = 1;
			    countSndArray[0] = (XShortResourceID)XGetShort(&theX->sndResourceID);
			    if (countSndArray[0] == (XShortResourceID)-1)
				{
				    sndCount = 0;
				}
			    for (count = 0; count < splitCount; count++)
				{
				    XGetKeySplitFromPtr(theX, count, &theSplit);
				    countSndArray[sndCount] = theSplit.sndResourceID;
				    if (countSndArray[sndCount] != (XShortResourceID)-1)
					{
					    sndCount++;
					}
				}
			}
		}
	    XBubbleSortArray((short *)countSndArray, (short)sndCount);

	    // Remove duplicates
	    sndOutCount = 0;
	    for (count = 0; count < sndCount; count++)
		{
		    goodSound = TRUE;
		    soundID = countSndArray[count];
		    for (count2 = 0; count2 < sndOutCount; count2++)
			{
			    if (soundID == sndArray[count2])
				{
				    goodSound = FALSE;
				    break;
				}
			}
		    if (goodSound)
			{
			    sndArray[sndOutCount++] = soundID;
			}
		}
	}
    return sndOutCount;
}

short int XCollectSoundsFromInstrumentID(XShortResourceID theID, XShortResourceID *sndArray, short maxArraySize)
{
    InstrumentResource	*theX;
    short int			totalSnds;
    long				size;

    totalSnds = 0;
    theX = (InstrumentResource *)XGetAndDetachResource(ID_INST, theID, &size);
    if (theX)
	{
	    totalSnds = XCollectSoundsFromInstrument(theX, sndArray, maxArraySize);
	    XDisposePtr((XPTR)theX);
	}
    return totalSnds;
}

XShortResourceID XCheckValidInstrument(XShortResourceID theID)
{
    InstrumentResource	*theX;
    short int			totalSnds;
    XShortResourceID	sndArray[MAX_SAMPLES];
    short int			count;
    XShortResourceID	badLoad;
    XPTR				theSnd;
    long				size;

    badLoad = 0;
    theX = (InstrumentResource *)XGetAndDetachResource(ID_INST, theID, &size);
    if (theX)
	{
	    totalSnds = XCollectSoundsFromInstrument(theX, sndArray, MAX_SAMPLES);
	    XDisposePtr((XPTR)theX);
	    for (count = 0; count < totalSnds; count++)
		{
		    theSnd = XGetAndDetachResource(ID_CSND, sndArray[count], &size);		// look for compressed version first
		    if (theSnd == NULL)
			{
			    theSnd = XGetAndDetachResource(ID_SND, sndArray[count], &size);
			}
		    if (theSnd == NULL)
			{
			    theSnd = XGetAndDetachResource(ID_ESND, sndArray[count], &size);
			}
		    if (theSnd == NULL)
			{
			    badLoad = sndArray[count];
			}
		    XDisposePtr(theSnd);
		    if (badLoad)
			{
			    break;
			}
		}
	}
    return badLoad;
}

XBOOL XCheckAllInstruments(XShortResourceID *badInstrument, XShortResourceID *badSnd)
{
    short int			count;
    XShortResourceID	bad;
    XBOOL				badLoad;
    XShortResourceID	instArray[MAX_INSTRUMENTS * MAX_BANKS];
    short int			totalInstruments;

    badLoad = FALSE;
    if (badInstrument && badSnd)
	{
	    totalInstruments = XGetInstrumentArray(instArray, MAX_INSTRUMENTS * MAX_BANKS);
	    if (totalInstruments)
		{
		    for (count = 0; count < totalInstruments; count++)
			{
			    bad = XCheckValidInstrument(instArray[count]);
			    if (bad)
				{
				    *badInstrument = instArray[count];
				    *badSnd = bad;
				    badLoad = TRUE;
				    break;
				}
			}
		}
	}
    return badLoad;
}

// Scan an array and return index
static short int PV_ConvertResourceID2Index(XShortResourceID *sndArray, short totalSnds, XShortResourceID theID)
{
    register short int count, indexValue;
	
    indexValue = -1;
    /* convert resourceID into instrument index */
    for (count = 0; count < totalSnds; count++)
	{
	    if (theID == sndArray[count])
		{
		    indexValue = count;
		    break;
		}
	}
    return indexValue;
}

// Given an array of instruments, this will return an array of SND resources that are required to load
// these instruments
short int XGetTotalKeysplits(XShortResourceID *instArray, short int totalInstruments, 
			     XShortResourceID *sndArray, short int totalSnds)
{
    register short int	count, count2, count3, count4, keyCount;
    KeySplit			theSplit;
    InstrumentResource	*theInstrument;
    char				*pLoaded;
    long				size;

    keyCount = 0;
    if (instArray && totalInstruments && sndArray && totalSnds)
	{
	    pLoaded = (char *)XNewPtr((long)sizeof(char) * totalSnds);
	    if (pLoaded)
		{
		    for (count = 0; count < totalInstruments; count++)
			{
			    theInstrument = (InstrumentResource *)XGetAndDetachResource(ID_INST, instArray[count], &size);
			    if (theInstrument)
				{
				    count3 = (short)XGetShort(&theInstrument->keySplitCount);
				    for (count2 = 0; count2 < count3; count2++)
					{
					    XGetKeySplitFromPtr(theInstrument, count2, &theSplit);
					    count4 = PV_ConvertResourceID2Index(sndArray, totalSnds, theSplit.sndResourceID);
					    if (count4)
						{
						    if (pLoaded[count4] == 0)
							{
							    pLoaded[count4] = 1;
							    keyCount++;
							}
						}
					}
				}
			}
		    XDisposePtr((XPTR)pLoaded);
		}
	}
    return keyCount;
}

/*
**	This will walk through all Instrument resources and collect all snd resources
**	that are used.
*/
short int XGatherAllSoundsFromAllInstruments(XShortResourceID *pSndArray, short int maxArraySize)
{
    short int			count, icount, sndCount, jcount;
    XShortResourceID	soundID;
    XBOOL				goodSound;
    XShortResourceID	instArray[MAX_INSTRUMENTS * MAX_BANKS];
    short int			totalInstruments;
    XShortResourceID	sndArray[MAX_SAMPLES];
    short int			totalSnds;
    XShortResourceID	*completeSndArray;
    short int			completeSndCount;
    InstrumentResource	*theX;
    long				size;

    completeSndCount = 0;
    sndCount = 0;
    completeSndArray = (XShortResourceID *)XNewPtr(MAX_INSTRUMENTS * MAX_BANKS * 128L * sizeof(XShortResourceID));
    if (completeSndArray)
	{
	    sndCount = 0;
	    totalInstruments = XGetInstrumentArray(instArray, MAX_INSTRUMENTS * MAX_BANKS);
	    if (totalInstruments)
		{
		    for (count = 0; count < totalInstruments; count++)
			{
			    theX = (InstrumentResource *)XGetAndDetachResource(ID_INST, instArray[count], &size);
			    if (theX)
				{
				    totalSnds = XCollectSoundsFromInstrument(theX, sndArray, MAX_SAMPLES);
				    XDisposePtr((XPTR)theX);

				    for (icount = 0; icount < totalSnds; icount++)
					{
					    completeSndArray[completeSndCount++] = sndArray[icount];
					}
				}
			}
		    XBubbleSortArray((short *)completeSndArray, (short)completeSndCount);

		    // Remove duplicates
		    for (jcount = 0; jcount < completeSndCount; jcount++)
			{
			    goodSound = TRUE;
			    soundID = completeSndArray[jcount];
			    for (count = 0; count < sndCount; count++)
				{
				    if (soundID == pSndArray[count])
					{
					    goodSound = FALSE;
					    break;
					}
				}
			    if (goodSound)
				{
				    if (sndCount < maxArraySize)
					{
					    pSndArray[sndCount++] = soundID;
					}
				}
			}
		}
	    XDisposePtr((XPTR)completeSndArray);
	}
    return sndCount;
}

// Given a list of instruments, this will return the sample ID's that are required to load
// all of these instruments
short int XGetSamplesFromInstruments(XShortResourceID *pInstArray, short int maxInstArraySize, 
				     XShortResourceID *pSndArray, short int maxSndArraySize)
{
    register long		count, instCount, sndCount, newCount, completeSndCount;
    XShortResourceID	soundID;
    XShortResourceID	*completeSndArray;
    XBOOL				goodSound;
    InstrumentResource	*theX;
    long				size;

    sndCount = 0;
    completeSndArray = (XShortResourceID *)XNewPtr(sizeof(XShortResourceID) * MAX_INSTRUMENTS * 128L);
    if (completeSndArray)
	{
	    if ( (pInstArray) && (pSndArray) )
		{
		    instCount = 0;
		    for (count = 0; count < maxInstArraySize; count++)
			{
			    if (pInstArray[count] != (XShortResourceID)-1)
				{
				    instCount++;
				}
			    else
				{
				    break;
				}
			}
		    for (count = 0; count < maxSndArraySize; count++)
			{
			    pSndArray[count] = (XShortResourceID)-1;
			    completeSndArray[count] = (XShortResourceID)-1;
			}


		    completeSndCount = 0;
		    for (count = 0; count < instCount; count++)
			{
			    theX = (InstrumentResource *)XGetAndDetachResource(ID_INST, pInstArray[count], &size);
			    if (theX)
				{
				    newCount = XCollectSoundsFromInstrument(theX, &completeSndArray[completeSndCount], 128);
				    XDisposePtr((XPTR)theX);
				    completeSndCount += newCount;
				}
			}
			
		    // remove duplicates in snds
		    for (newCount = 0; newCount < completeSndCount; newCount++)
			{
			    goodSound = TRUE;
			    soundID = completeSndArray[newCount];
			    for (count = 0; count < sndCount; count++)
				{
				    if (soundID == pSndArray[count])
					{
					    goodSound = FALSE;
					    break;
					}
				}
			    if (goodSound)
				{
				    pSndArray[sndCount++] = (XShortResourceID)soundID;
				}
			}
		    XBubbleSortArray((short *)pSndArray, (short)sndCount);
		}
	    XDisposePtr(completeSndArray);
	}
    return (short)sndCount;
}

// Given a song ID and two arrays, this will return the INST resources ID and the 'snd ' resource ID
// that are needed to load the song terminated with a -1.
// Will return 0 for success or 1 for failure
OPErr XGetSongInstrumentList(XShortResourceID theSongID, XShortResourceID *pInstArray, short int maxInstArraySize, 
			     XShortResourceID *pSndArray, short int maxSndArraySize)
{
    long				count, instCount, sndCount, newCount, completeSndCount;
    XShortResourceID	soundID;
    SongResource		*theSong;
    XShortResourceID	completeSndArray[MAX_SAMPLES];
    XShortResourceID	completeInstArray[MAX_INSTRUMENTS * MAX_BANKS];
    XBOOL				goodSound;
    OPErr				theErr;
    long				size;

    theErr = NO_ERR;
    if ( (pInstArray) && (pSndArray) )
	{
	    for (count = 0; count < maxInstArraySize; count++)
		{
		    pInstArray[count] = (XShortResourceID)-1;
		    completeInstArray[count] = (XShortResourceID)-1;
		}
	    for (count = 0; count < maxSndArraySize; count++)
		{
		    pSndArray[count] = (XShortResourceID)-1;
		    completeSndArray[count] = (XShortResourceID)-1;
		}
	    theSong = (SongResource *)XGetAndDetachResource(ID_SONG, theSongID, &size);
	    if (theSong)
		{
		    instCount = GM_GetUsedPatchlist(theSong, NULL, 0L, completeInstArray, &theErr);
		    if (instCount && (theErr == 0) )
			{
				// remove duplicates in inst
			    sndCount = 0;
			    for (newCount = 0; newCount < instCount; newCount++)
				{
				    goodSound = TRUE;
				    soundID = completeInstArray[newCount];
				    for (count = 0; count < sndCount; count++)
					{
					    if (soundID == pInstArray[count])
						{
						    goodSound = FALSE;
						    break;
						}
					}
				    if (goodSound)
					{
					    pInstArray[sndCount++] = (XShortResourceID)soundID;
					}
				}
			    instCount = sndCount;
			    XBubbleSortArray((short *)pInstArray, (short)instCount);

			    completeSndCount = 0;
			    for (count = 0; count < instCount; count++)
				{
				    newCount = XCollectSoundsFromInstrumentID(pInstArray[count], 
									      &completeSndArray[completeSndCount], 128);
				    completeSndCount += newCount;
				}
				
				// remove duplicates in snds
			    sndCount = 0;
			    for (newCount = 0; newCount < completeSndCount; newCount++)
				{
				    goodSound = TRUE;
				    soundID = completeSndArray[newCount];
				    for (count = 0; count < sndCount; count++)
					{
					    if (soundID == pSndArray[count])
						{
						    goodSound = FALSE;
						    break;
						}
					}
				    if (goodSound)
					{
					    pSndArray[sndCount++] = (XShortResourceID)soundID;
					}
				}
			    XBubbleSortArray((short *)pSndArray, (short)sndCount);
			}
		    else
			{
			    theErr = BAD_MIDI_DATA;
			}
		}
	    XDisposePtr((XPTR)theSong);
	}
    return theErr;
}


XPTR XCompressAndEncrypt(XPTR pData, unsigned long size, unsigned long *pNewSize)
{
    XPTR	newData;

    newData = NULL;
    if (pData && pNewSize)
	{
	    newData = XCompressPtr(pData, size, (unsigned long *)pNewSize, X_RAW);
	    if (newData)
		{
		    XEncryptData(newData, *pNewSize);
		}
	}
    return newData;
}

long XGetSongTempoFactor(SongResource *pSong)
{
    long			tempo;

    tempo = 16667;	// 1.0
    if (pSong)
	{
	    switch (((SongResource_SMS *)pSong)->songType)
		{
		case SONG_TYPE_SMS:
		    tempo = XGetShort(&((SongResource_SMS *)pSong)->songTempo);
		    break;
		case SONG_TYPE_RMF:
		    tempo = XGetShort(&((SongResource_RMF *)pSong)->songTempo);
		    break;
		}
	    if (tempo == 0)
		{
		    tempo = 16667;
		}
	}
    return tempo;
}

void XSetSongTempoFactor(SongResource *pSong, long newTempo)
{
    if (pSong)
	{
	    if (newTempo == 16667L)
		{
		    newTempo = 0;
		}
	    switch (((SongResource_SMS *)pSong)->songType)
		{
		case SONG_TYPE_SMS:
		    XPutShort(&((SongResource_SMS *)pSong)->songTempo, (unsigned short)newTempo);
		    break;
		case SONG_TYPE_RMF:
		    XPutShort(&((SongResource_RMF *)pSong)->songTempo, (unsigned short)newTempo);
		    break;
		}
	}
}

// allocate and return an list of ID's collected from ID_SND, ID_CSND, ID_ESND. pCount will
// be the number of ID's, and the long array will be the list. use XDisposePtr on the return
// pointer
XLongResourceID * XGetAllSoundID(long *pCount)
{
    long			count, size, totalResourceCount, resourceIndex, sampleCount;
    XResourceType	resType;
    XLongResourceID	theID;
    char			name[256];
    XPTR			data;
    XLongResourceID	*pArray;

    pArray = NULL;
    sampleCount = 0;
    totalResourceCount = XCountTypes(NULL);		// get total number of resource types
    if (totalResourceCount && pCount)
	{
	    pArray = (XLongResourceID *)XNewPtr((long)sizeof(XLongResourceID) * MAX_SAMPLES);
	    if (pArray)
		{
		    for (resourceIndex = 0; resourceIndex < totalResourceCount; resourceIndex++)
			{
			    resType = XGetIndexedType(NULL, resourceIndex);
			    if ( (resType == ID_SND) || (resType == ID_CSND) || (resType == ID_ESND) )
				{
				    for (count = 0; ; count++)
					{
					    data = XGetIndexedResource(resType, &theID, count, name, &size);
					    if (data)
						{
						    XDisposePtr(data);			// free pointer resource
						    pArray[sampleCount++] = theID;
						    if (sampleCount == (MAX_SAMPLES-1))
							{
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
		    *pCount = sampleCount;
		}
	}
    return pArray;
}

// This will return a MIDI/CMID/EMID/ECMI object from an open resource file
//
// INPUT:
//	theXSong		is the SongResource structure
//
// OUTPUT:
//	pMusicName		is a pascal string
//	pMusicType		is the resource type
//	pMusicID		is the resource ID
//	pReturnedSize			is the resource size
XPTR XGetMusicObjectFromSong(SongResource *theXSong, char *pMusicName, 
			     XResourceType *pMusicType, XLongResourceID *pMusicID, long *pReturnedSize)
{
    long			musicID;
    SongType		songType;
    XPTR			data;
    XResourceType	midiTypes[] = {ID_MIDI, ID_MIDI_OLD, ID_CMID, ID_EMID, ID_ECMI};
    short int		count;

    data = NULL;
    if (pReturnedSize)
	{
	    *pReturnedSize = 0;
	}
    if (pMusicName)
	{
	    pMusicName[0] = 0;
	}
    if (theXSong)
	{
	    *pMusicType = 0;
	    *pMusicID = 0;
	    musicID = XGetSongResourceObjectID(theXSong);
	    songType = XGetSongResourceObjectType(theXSong);
		
	    for (count = 0; count < (sizeof(midiTypes) / sizeof(long)); count++)
		{
		    data = XGetAndDetachResource(midiTypes[count], musicID, pReturnedSize);
		    if (data)
			{
			    if (pMusicName)
				{
				    XGetResourceName(midiTypes[count], musicID, pMusicName);
				    XCtoPstr(pMusicName);
				}
			    *pMusicType = midiTypes[count];
			    *pMusicID = musicID;
			    break;
			}
		}
	}
    return data;
}

XBOOL XIsSampleUsedInAllInstruments(XShortResourceID soundSampleID, XShortResourceID *pWhichInstrument)
{
    InstrumentResource	*theX;
    short int			count, totalInstruments;
    XBOOL				used;
    long				size;
    XShortResourceID	instArray[MAX_INSTRUMENTS * MAX_BANKS];

    used = FALSE;
    totalInstruments = XGetInstrumentArray(instArray, MAX_INSTRUMENTS * MAX_BANKS);
    if (totalInstruments && pWhichInstrument)
	{
	    for (count = 0; count < totalInstruments; count++)
		{
		    theX = (InstrumentResource *)XGetAndDetachResource(ID_INST, instArray[count], &size);
		    if (theX)
			{
			    used = XIsSoundUsedInInstrument(theX, soundSampleID);
			    XDisposePtr((XPTR)theX);
			    if (used)
				{
				    *pWhichInstrument = instArray[count];
				    break;
				}
			}
		}
	}
    return used;
}


XERR XCopySongMidiResources(XLongResourceID theSongID, XFILE readFileRef, 
			    XFILE writeFileRef, XBOOL protect, XBOOL copyNames)
{
    XPTR			pData;
    SongResource	*theSong;
    long			songSize;
    XResourceType	theDataType;
    short int		theID;
    char			theName[256], theSongName[256];
    unsigned long	dataSize, newSize;
    XPTR			newMidiData;

    XFileUseThisResourceFile(readFileRef);		// from resource file
    theSong = (SongResource *)XGetFileResource(readFileRef, ID_SONG, theSongID, theSongName, &songSize);
    if (theSong)
	{
	    theID = XGetSongResourceObjectID(theSong);

	    if (protect)
		{
		    pData = XGetMidiData(theID, (long *)&dataSize, &theDataType);
		    if (pData)
			{
			    newMidiData = XCompressAndEncrypt(pData, dataSize, &newSize);
			    if (newMidiData)
				{
				    theDataType = ID_ECMI;
				    XDisposePtr(pData);
				    pData = newMidiData;
				}
			    else
				{
				    theDataType = ID_MIDI;
				}
			}
		}
	    else
		{
		    theDataType = ID_CMID;
		    pData = XGetFileResource(readFileRef, ID_CMID, theID, theName, (long *)&dataSize);
		    if (pData == NULL)
			{
			    theDataType = ID_ECMI;
			    pData = XGetFileResource(readFileRef, ID_ECMI, theID, theName, (long *)&dataSize);
			}
		    if (pData == NULL)
			{
			    theDataType = ID_EMID;
			    pData = XGetFileResource(readFileRef, ID_EMID, theID, theName, (long *)&dataSize);
			}
		    if (pData == NULL)
			{
			    theDataType = ID_MIDI;
			    pData = XGetFileResource(readFileRef, ID_MIDI, theID, theName, (long *)&dataSize);
			}
		    if (pData == NULL)
			{
			    theDataType = ID_MIDI;		// convert it to the new type
			    pData = XGetFileResource(readFileRef, ID_MIDI_OLD, theID, theName, (long *)&dataSize);
			}
		}
	    if (pData)
		{
		    // write midi resource
		    XFileUseThisResourceFile(writeFileRef);
		    if (copyNames == FALSE)
			{
			    theName[0] = 0;
			}
		    XAddResource(theDataType, theID, theName, pData, (long)dataSize);
		    XDisposePtr(pData);

		    // write song resource
		    XFileUseThisResourceFile(readFileRef);

		    if (protect)
			{
			    XSetSongLocked(theSong, TRUE);
			}

		    XFileUseThisResourceFile(writeFileRef);
		    XAddResource(ID_SONG, theID, theName, theSong, songSize);
		    XDisposePtr(theSong);
		}
	}
    return 0;
}

XERR XCopySndResources(XShortResourceID *pSndCopy, short int sndCount, XFILE readFileRef, 
		       XFILE writeFileRef, XBOOL protect, XBOOL copyNames)
{
    short int		count, resCount;
    XPTR			pData;
    short int		theID;
    char			theName[256];
    long			size;
    long			soundTypes[] = {ID_SND, ID_ESND, ID_CSND};

    if (sndCount && pSndCopy)
	{
	    XFileUseThisResourceFile(readFileRef);		/* from resource file */
	    for (count = 0; count < sndCount; count++)
		{
		    theID = pSndCopy[count];
			
		    // determine if the resource is already in the written file
		    XFileUseThisResourceFile(writeFileRef);
		    pData = XGetAndDetachResource(ID_SND, theID, &size);
		    if (pData == NULL)
			{
			    pData = XGetAndDetachResource(ID_ESND, theID, &size);
			}
		    if (pData == NULL)
			{
			    pData = XGetAndDetachResource(ID_CSND, theID, &size);
			}
		    XDisposePtr(pData);
		    if (pData == NULL)			// check to see that its not there already!!
			{
			    for (resCount = 0; resCount < 3; resCount++)
				{
				    XFileUseThisResourceFile(readFileRef);		/* from resource file */
				    pData = XGetFileResource(readFileRef, soundTypes[resCount], theID, theName, &size);
				    if (pData)
					{
					    if (copyNames == FALSE)
						{
						    theName[0] = 0;
						}

					    XFileUseThisResourceFile(writeFileRef);
					    if (protect && (soundTypes[resCount] == ID_SND))
						{
						    XEncryptData(pData, size);
						    XAddResource(ID_ESND, theID, theName, pData, size);
						}
					    else
						{
						    XAddResource(soundTypes[resCount], theID, theName, pData, size);
						}
					    XDisposePtr(pData);
					}
				}
			}
		    else
			{
			    XDisposePtr(pData);
			}
		}
	}
    return 0;
}

XERR XCopyInstrumentResources(XShortResourceID *pInstCopy, short int instCount, 
			      XFILE readFileRef, XFILE writeFileRef, XBOOL copyNames)
{
    short int		count;
    XPTR			pData;
    short int		theID;
    long			size;
    char			theName[256];

    if (instCount && pInstCopy)
	{
	    XFileUseThisResourceFile(readFileRef);		/* from resource file */
	    for (count = 0; count < instCount; count++)
		{
		    theID = pInstCopy[count];
		    XFileUseThisResourceFile(writeFileRef);
		    pData = XGetFileResource(writeFileRef, ID_INST, theID, theName, &size);
		    if (pData == NULL)		// check to see that its not there already!!
			{
			    XFileUseThisResourceFile(readFileRef);		/* from resource file */
			    pData = XGetFileResource(readFileRef, ID_INST, theID, theName, &size);
			    if (pData)
				{
				    XFileUseThisResourceFile(writeFileRef);
				    if (copyNames == FALSE)
					{
					    theName[0] = 0;
					}
				    XAddResource(ID_INST, theID, theName, pData, size);
				}
			}
		    XDisposePtr(pData);
		}
	}
    return 0;
}

#if 0
void XTestCompression(XPTR compressedAndEncryptedData, long size, XPTR originalData, long originalSize)
{
    XPTR	pData, pData2;
    short int	safe;

    if (compressedAndEncryptedData && originalData && size && originalSize)
	{
	    // since this is encrypted, make a new copy and decrypt
	    pData = XNewPtr(size);
	    if (pData)
		{
		    XBlockMove(compressedAndEncryptedData, pData, size);
		    XDecryptData(pData, (unsigned long)size);				// decrypt first
		    pData2 = pData;
		    pData = XDecompressPtr(pData2, (unsigned long)size, TRUE);		// uncompress second
		    if (pData)
			{
			    size = XGetPtrSize(pData);	// get new size
			    if (size == originalSize)
				{
				    safe = XMemCmp(pData, originalData, size);
				    if (safe)
					{
					    DebugStr("\pmemcmp failed");
					}
				    else
					{
					    DebugStr("\pSAFE!");
					}
				}
			    else
				{
				    DebugStr("\psize failed");
				}
			}
		    else
			{
			    DebugStr("\pdecrypt failed");
			}
		    XDisposePtr(pData);
		    XDisposePtr(pData2);
		}
	}
}
#endif

#endif	// USE_CREATION_API == TRUE

// EOF of X_EditorTools.c



