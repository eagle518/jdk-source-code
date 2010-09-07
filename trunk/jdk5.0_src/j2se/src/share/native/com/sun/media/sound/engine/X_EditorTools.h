/*
 * @(#)X_EditorTools.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	X_EditorTools.h
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
**	2/5/98		Added XCopySongMidiResources & XCopyInstrumentResources & XCopySndResources
*/
/*****************************************************************************/
#ifndef X_EDITOR_TOOLS
#define X_EDITOR_TOOLS

#ifndef __X_API__
#include "X_API.h"
#endif

#ifndef X_FORMATS
#include "X_Formats.h"
#endif

#ifndef G_SOUND
#include "GenSnd.h"
#include "GenPriv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


    // Utillities for instruments
    XBOOL XIsSoundUsedInInstrument(InstrumentResource *theX, XShortResourceID sampleSoundID);
    void XRenumberSampleInsideInstrument(InstrumentResource *theX, XShortResourceID originalSampleID, 
					 XShortResourceID newSampleID);
    short int XCollectSoundsFromInstrument(InstrumentResource *theX, XShortResourceID *sndArray, short maxArraySize);
    short int XCollectSoundsFromInstrumentID(XShortResourceID theID, XShortResourceID *sndArray, short maxArraySize);
    XBOOL XCheckAllInstruments(XShortResourceID *badInstrument, XShortResourceID *badSnd);
    XShortResourceID XCheckValidInstrument(XShortResourceID theID);
    short int XGetInstrumentArray(XShortResourceID *instArray, short maxArraySize);

    short int XGatherAllSoundsFromAllInstruments(XShortResourceID *pSndArray, short int maxArraySize);

    XBOOL XIsSampleUsedInAllInstruments(XShortResourceID soundSampleID, XShortResourceID *pWhichInstrument);

    short int XGetTotalKeysplits(XShortResourceID *instArray, short int totalInstruments, 
				 XShortResourceID *sndArray, short int totalSnds);

    // Given a song ID and two arrays, this will return the INST resources ID and the 'snd ' resource ID
    // that are needed to load the song terminated with a -1.
    // Will return 0 for success or 1 for failure
    OPErr XGetSongInstrumentList(XShortResourceID theSongID, XShortResourceID *pInstArray, short int maxInstArraySize, 
				 XShortResourceID *pSndArray, short int maxSndArraySize);

    short int XGetSamplesFromInstruments(XShortResourceID *pInstArray, short int maxInstArraySize, 
					 XShortResourceID *pSndArray, short int maxSndArraySize);

    void XSetKeySplitFromPtr(InstrumentResource *theX, short int entry, KeySplit *keysplit);

    InstrumentResource * XAddKeySplit(InstrumentResource *theX, short int howMany);
    InstrumentResource * XRemoveKeySplit(InstrumentResource *theX, short int howMany);

    XPTR XCompressAndEncrypt(XPTR pData, unsigned long size, unsigned long *pNewSize);

    long XGetSongTempoFactor(SongResource *pSong);
    void XSetSongTempoFactor(SongResource *pSong, long newTempo);

    // allocate and return an list of ID's collected from ID_SND, ID_CSND, ID_ESND. pCount will
    // be the number of ID's, and the long array will be the list. use XDisposePtr on the return
    // pointer
    XLongResourceID * XGetAllSoundID(long *pCount);

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
				 XResourceType *pMusicType, XLongResourceID *pMusicID, long *pReturnedSize);

    XERR XCopySongMidiResources(XLongResourceID theSongID, XFILE readFileRef, 
				XFILE writeFileRef, XBOOL protect, XBOOL copyNames);
    XERR XCopyInstrumentResources(XShortResourceID *pInstCopy, short int instCount, 
				  XFILE readFileRef, XFILE writeFileRef, XBOOL copyNames);
    XERR XCopySndResources(XShortResourceID *pSndCopy, short int sndCount, XFILE readFileRef, 
			   XFILE writeFileRef, XBOOL protect, XBOOL copyNames);


    // Test API's
    void XTestCompression(XPTR compressedAndEncryptedData, long size, XPTR originalData, long originalSize);

#ifdef __cplusplus
}
#endif


#endif	// X_EDITOR_TOOLS
// EOF of X_EditorTools.h


