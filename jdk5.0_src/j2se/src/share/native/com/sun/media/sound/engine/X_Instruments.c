/*
 * @(#)X_Instruments.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	X_Instruments.c
**
**	Tools for creating instruments
**
**	History	-
**	2/16/98		Created. Pulled from MacOS specific editor codebase
**				Moved XNewInstrument & XDisposeInstrument to DriverTools.c
**				Renamed XNewInstrument to XNewInstrumentResource, and XDisposeInstrument
**				to XDisposeInstrumentResource
**	11/10/98	Added XNewInstrumentWithBasicEnvelopeResource. Fixed memory bug
**				with XReconstructInstrument
*/
/*****************************************************************************/

#include "GenSnd.h"
#include "GenPriv.h"
#include "X_Formats.h"
#include "X_Instruments.h"


#if USE_CREATION_API == TRUE

// Create a new instrument. Free with XDisposeInstrument. This creates a basic
// instrument with no evelope or extra data.
InstrumentResource * XNewInstrumentResource(XShortResourceID leadSndID)
{
    InstrumentResource *theX;

    theX = (InstrumentResource *)XNewPtr((long)sizeof(InstrumentResource));
    if (theX)
	{
	    XPutShort(&theX->sndResourceID, leadSndID);
	    XPutShort(&theX->tremoloEnd, 0x8000);

	    theX->flags1 |= ZBF_enableInterpolate | ZBF_enableAmpScale | ZBF_useSampleRate;
	}
    return theX;
}

InstrumentResource * XNewInstrumentWithBasicEnvelopeResource(XShortResourceID leadSndID, XEnvelopeType type)
{
    InstrumentResource	*tempX, *finalX;
    XInstrumentData		*theI;
    long				size;

    finalX = NULL;
    tempX = XNewInstrumentResource(leadSndID);
    if (tempX)
	{
	    size = XGetPtrSize(tempX);
	    theI = XCreateXInstrument(tempX, size);
	    if (theI)
		{
		    XAddDefaultADSREnvelope(theI, type);

		    finalX = XReconstructInstrument(tempX, size, theI);
		}
	    XDisposeInstrumentResource(tempX);
	}

    return finalX;
}

void XDisposeInstrumentResource(InstrumentResource *theX)
{
    XDisposePtr((XPTR)theX);
}


long XGetTotalEnvelopeTime(XEnvelopeData *pXEnvelope)
{
    long	totalTime, count;

    totalTime = 0;
    if (pXEnvelope)
	{
	    for (count = 0; count < pXEnvelope->stageCount; count++)
		{
		    totalTime += pXEnvelope->time[count];
		}
	}
    return totalTime;
}

void XEnvelopeAdjustSustainTime(XEnvelopeData *pXEnvelope)
{
    long	count, scale;

    if (pXEnvelope)
	{
	    scale = pXEnvelope->endScaleH - pXEnvelope->startScaleH;
	    for (count = 0; count < pXEnvelope->stageCount; count++)
		{
		    if (pXEnvelope->flags[count] == ADSR_SUSTAIN)
			{
			    pXEnvelope->time[count] = scale / 5;
			    break;
			}
		}
	}
}

long XFindType(XInstrumentData *pXInstrument, XUnitType type)
{
    long		count;

    for (count = 0; count < pXInstrument->unitCount; count++)
	{
	    if (pXInstrument->units[count].unitType == type)
		{
		    return count;
		}
	}
    return -1;
}

long XAddType(XInstrumentData *pXInstrument, XUnitType type)
{
    long		count;

    count = pXInstrument->unitCount++;
    pXInstrument->units[count].unitType = type;
    pXInstrument->units[count].unitID = count;
    return count;
}

void XRemoveType(XInstrumentData *pXInstrument, XUnitType unitType, unsigned long unitID)
{
    long		count, count2;

    for (count = 0; count < pXInstrument->unitCount; count++)
	{
	    if ( (pXInstrument->units[count].unitType == unitType) && (pXInstrument->units[count].unitID == unitID) )
		{
		    for (count2 = count; count2 < (pXInstrument->unitCount-1); count2++)
			{
			    pXInstrument->units[count2] = pXInstrument->units[count2+1];
			}

		    pXInstrument->unitCount--;
		    break;
		}
	}
}

XBOOL XDeleteEnvelopePoint(XEnvelopeData *pEnvelope, short int whichPoint)
{
    long		count;
    XBOOL		deletePoint;
    long		prevTime;

    if (pEnvelope)
	{
	    deletePoint = FALSE;
	    // delete select point
	    if (whichPoint)		// can't delete point zero
		{
		    // can't delete last point
		    if (pEnvelope->flags[whichPoint] != ADSR_TERMINATE)
			{
			    deletePoint = TRUE;
			}
		}
	    // can't delete sustain point if it will become point zero
	    if ( (pEnvelope->flags[whichPoint] == ADSR_SUSTAIN) && (whichPoint == 1) )
		{
		    deletePoint = FALSE;
		}
			
	    if (deletePoint)
		{
		    prevTime = pEnvelope->time[whichPoint];
		    for (count = whichPoint; count < ADSR_STAGES; count++)
			{
			    pEnvelope->time[count] = pEnvelope->time[count+1];
			    pEnvelope->level[count] = pEnvelope->level[count+1];
			    pEnvelope->flags[count] = pEnvelope->flags[count+1];
			}
		    pEnvelope->time[whichPoint] += prevTime;
		    pEnvelope->stageCount--;
		}
	}
    return deletePoint;
}

void XFillDefaultADSREnvelope(XEnvelopeData *pEnvelope, XEnvelopeType type)
{
    switch (type)
	{
	case NONE_E:	// none
	    pEnvelope->stageCount = 0;
	    break;
	case FOUR_POINT_E:
	    pEnvelope->level[0] = 0;
	    pEnvelope->flags[0] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[0] = 0;
		
	    pEnvelope->level[1] = VOLUME_RANGE;
	    pEnvelope->flags[1] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[1] = 15000 * 2;
		
	    pEnvelope->level[2] = 3200;
	    pEnvelope->flags[2] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[2] = 30000 * 2;
		
	    pEnvelope->level[3] = 3200;
	    pEnvelope->flags[3] = ADSR_SUSTAIN;
	    pEnvelope->time[3] = SUSTAIN_DEFAULT_TIME;
		
	    pEnvelope->level[4] = 0;
	    pEnvelope->flags[4] = ADSR_TERMINATE;
	    pEnvelope->time[4] = 30000 * 2;
	    pEnvelope->stageCount = 5;
	    break;
	case FLAT_FULL_E:
	    pEnvelope->level[0] = VOLUME_RANGE;
	    pEnvelope->flags[0] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[0] = 0;
		
	    pEnvelope->level[1] = VOLUME_RANGE;
	    pEnvelope->flags[1] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[1] = 30000 * 2;
		
	    pEnvelope->level[2] = VOLUME_RANGE;
	    pEnvelope->flags[2] = ADSR_SUSTAIN;
	    pEnvelope->time[2] = SUSTAIN_DEFAULT_TIME;
		
	    pEnvelope->level[3] = 0;
	    pEnvelope->flags[3] = ADSR_TERMINATE;
	    pEnvelope->time[3] = 30000 * 2;
	    pEnvelope->stageCount = 4;
	    break;		
	case TWO_POINT_E:
	    pEnvelope->level[0] = 0;
	    pEnvelope->flags[0] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[0] = 0;
		
	    pEnvelope->level[1] = VOLUME_RANGE;
	    pEnvelope->flags[1] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[1] = 15000 * 2;
		
	    pEnvelope->level[2] = 0;
	    pEnvelope->flags[2] = ADSR_TERMINATE;
	    pEnvelope->time[2] = 30000 * 2;
	    pEnvelope->stageCount = 3;
	    break;
	}
}

void XShiftEnvelopeVolume(XEnvelopeData *pEnvelope, long shift)
{
    long		count, stage;

    stage = pEnvelope->stageCount;
    for (count = 0; count < stage; count++)
	{
	    pEnvelope->level[count] += shift;
	}
}


void XAddZeroEnvelope(XEnvelopeData *pEnvelope)
{
    long		count, stage;

    stage = pEnvelope->stageCount;
    for (count = 0; count < stage; count++)
	{
	    if (pEnvelope->flags[count] == ADSR_SUSTAIN)
		{
		    pEnvelope->time[count] = SUSTAIN_DEFAULT_TIME;
		}
	}

    if (pEnvelope->time[0])
	{
	    pEnvelope->stageCount++;		// add extra
	    for (count = ADSR_STAGES; count > 0; count--)
		{
		    pEnvelope->level[count] = pEnvelope->level[count-1];
		    pEnvelope->flags[count] = pEnvelope->flags[count-1];
		    pEnvelope->time[count] = pEnvelope->time[count-1];
		}
	    pEnvelope->level[0] = 0;
	    pEnvelope->flags[0] = ADSR_LINEAR_RAMP;
	    pEnvelope->time[0] = 0;
	}
}

void XRemoveZeroEnvelope(XEnvelopeData *pEnvelope)
{
    long		count, stage;

    stage = pEnvelope->stageCount;
    for (count = 0; count < stage; count++)
	{
	    if (pEnvelope->flags[count] == ADSR_SUSTAIN)
		{
		    pEnvelope->time[count] = 0;
		}
	}

    if ((pEnvelope->level[0] == 0) &&
	(pEnvelope->flags[0] == ADSR_LINEAR_RAMP) && 
	(pEnvelope->time[0] == 0) )
	{
	    pEnvelope->stageCount--;
	    for (count = 0; count < ADSR_STAGES; count++)
		{
		    pEnvelope->level[count] = pEnvelope->level[count+1];
		    pEnvelope->flags[count] = pEnvelope->flags[count+1];
		    pEnvelope->time[count] = pEnvelope->time[count+1];
		}
	}
}

void XAddDefaultADSREnvelope(XInstrumentData *pXInstrument, XEnvelopeType type)
{
    long		count;

    count = XFindType(pXInstrument, INST_ADSR_ENVELOPE);
    if (count == -1)
	{
	    // not there, so use first slot
	    count = pXInstrument->unitCount;
	    pXInstrument->unitCount++;
	}
    pXInstrument->units[count].unitType = INST_ADSR_ENVELOPE;
    XFillDefaultADSREnvelope(&pXInstrument->units[count].u.envelopeADSR, type);
}

XInstrumentData *XCreateXInstrument(InstrumentResource *theX, unsigned long theXSize)
{
    long				count, count2;
    long				size, unitCount, unitSubCount;
    XUnitType			unitType;
    unsigned short int	data;
    char 				*pData, *pData2;
    char 				*pUnit;
    KeySplit 			*pSplits;
    XLFOData			*pLFO;
    XEnvelopeData		*pENV;
    XInstrumentData		*pXInstrument;
    XTieToData			*pCurve;

    pXInstrument = (XInstrumentData *)XNewPtr((long)sizeof(XInstrumentData));
    if (pXInstrument)
	{
	    pXInstrument->unitCount = 0;

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
			    size -= (pData - pData2);
			    for (count = 0; count < size; count++)
				{
				    data = XGetShort(&pData[count]);
				    if (data == 0x8000)
					{
					    count += 4;								// skip past end token and extra word
					    data = (unsigned short)pData[count] + 1;			// get first string length;
					    count2 = (long)pData[count+data] + 1;			// get second string length
					    pUnit = (char *) (&pData[count + data + count2]);
					    // NOTE: src will be non aligned, possibly on a byte boundry.
					    break;
					}
				}
			    if (pUnit)
				{
				    pUnit += 12;		// reserved global space

				    unitCount = *pUnit;		// how many unit records?
				    pUnit++;					// byte
				    pXInstrument->unitCount = unitCount;
				    if (unitCount)
					{
					    for (count = 0; count < unitCount; count++)
						{
						    unitType = (XUnitType)XGetLong(pUnit) & 0x5F5F5F5F;
						    pUnit += 4;	// long
						    pXInstrument->units[count].unitType = unitType;
						    pXInstrument->units[count].unitID = count;
						    switch (unitType)
							{
							case INST_ADSR_ENVELOPE:
							    unitSubCount = *pUnit;		// how many unit records?
							    pUnit++;					// byte
							    if (unitSubCount > ADSR_STAGES)
								{	// can't have more than ADSR_STAGES stages
								    XDisposePtr(pXInstrument);
								    pXInstrument = NULL;
								    goto bailoninstrument;
								}
							    pENV = &pXInstrument->units[count].u.envelopeADSR;
							    pENV->stageCount = unitSubCount;
							    for (count2 = 0; count2 < unitSubCount; count2++)
								{
								    pENV->level[count2] = XGetLong(pUnit);
								    pUnit += 4;

								    pENV->time[count2] = XGetLong(pUnit);
								    pUnit += 4;

								    pENV->flags[count2] = XGetLong(pUnit) & 0x5F5F5F5F;
								    pUnit += 4;
								}

							    XAddZeroEnvelope(pENV);		// add extra 0 time point for editing
							    break;

							case INST_DEFAULT_MOD:
							    pXInstrument->units[count].u.useDefaultModwheelAction = TRUE;
							    break;

							case INST_EXPONENTIAL_CURVE:					// curve entry
							    pCurve = &pXInstrument->units[count].u.curve;
							    pCurve->tieFrom = XGetLong(pUnit);
							    pUnit += 4;
							    pCurve->tieTo = XGetLong(pUnit);
							    pUnit += 4;
							    pCurve->curveCount = *pUnit++;
							    unitSubCount = pCurve->curveCount;
							    for (count2 = 0; count2 < unitSubCount; count2++)
								{
								    pCurve->from_Value[count2] = *pUnit++;
								    pCurve->to_Scalar[count2] = XGetShort(pUnit);
								    pUnit += 2;
								}
							    break;

							case INST_LOW_PASS_FILTER:		// low pass global filter parameters
							    pXInstrument->units[count].u.lpf.LPF_frequency = XGetLong(pUnit);
							    pUnit += 4;
							    pXInstrument->units[count].u.lpf.LPF_resonance = XGetLong(pUnit);
							    pUnit += 4;
							    pXInstrument->units[count].u.lpf.LPF_lowpassAmount = XGetLong(pUnit);
							    pUnit += 4;
							    break;

							    // LFO types
							case INST_VOLUME_LFO:
							case INST_PITCH_LFO:
							case INST_STEREO_PAN_LFO:
							case INST_STEREO_PAN_NAME2:
							case INST_LOW_PASS_AMOUNT:
							case INST_LPF_DEPTH:
							case INST_LPF_FREQUENCY:
							    unitSubCount = *pUnit;		// how many unit records?
							    pUnit++;					// byte
							    if (unitSubCount > ADSR_STAGES)
								{	// can't have more than ADSR_STAGES stages
								    XDisposePtr(pXInstrument);
								    pXInstrument = NULL;
								    goto bailoninstrument;
								}
							    pLFO = &pXInstrument->units[count].u.lfo;
							    pLFO->envelopeLFO.stageCount = unitSubCount;
							    for (count2 = 0; count2 < unitSubCount; count2++)
								{
								    pLFO->envelopeLFO.level[count2] = XGetLong(pUnit);
								    pUnit += 4;
								    pLFO->envelopeLFO.time[count2] = XGetLong(pUnit);
								    pUnit += 4;
								    pLFO->envelopeLFO.flags[count2] = XGetLong(pUnit) & 0x5F5F5F5F;
								    pUnit += 4;
								}
							    XAddZeroEnvelope(&pLFO->envelopeLFO);		// add extra 0 time point for editing
							    pLFO->period = XGetLong(pUnit);
							    pUnit += 4;
							    pLFO->waveShape = XGetLong(pUnit);
							    pUnit += 4;
							    pLFO->DC_feed = XGetLong(pUnit);
							    pUnit += 4;
							    pLFO->depth = XGetLong(pUnit);
							    pUnit += 4;
							    break;
							}
						}
					}
				}
			}
		}
	}
bailoninstrument:
    return pXInstrument;
}

#define TEMP_INSTRUMENT_BUILD_SPACE		4096	// temp space during build process

// pass in original instrument resource (theX) its size (theXSize) and the structure XInstrumentData (pXInstrument)
// to build a new instrument resource. The variable theX is not touched, and you can deallocate it after this
// function is used.
InstrumentResource * XReconstructInstrument(InstrumentResource *theX, unsigned long theXSize,
					    XInstrumentData *pXInstrument)
{
    InstrumentResource	*newInstrument;
    long				count, count2;
    long				size, unitCount, unitSubCount, offset;
    XUnitType			unitType;
    unsigned short int	data;
    char 				*pData, *pData2;
    char 				*pUnit;
    KeySplit 			*pSplits;
    XLFOData			*pLFO;
    XEnvelopeData		*pENV;
    XTieToData			*pCurve;
    char				*tempSpace;		// used to build the X instrument stuff before copying
    long				tempSpaceSize;

    newInstrument = NULL;
    tempSpaceSize = TEMP_INSTRUMENT_BUILD_SPACE;
    tempSpace = (char *)XNewPtr(TEMP_INSTRUMENT_BUILD_SPACE);
    if (tempSpace)
	{
	    pUnit = tempSpace;
	    XPutLong(pUnit, 0L);
	    pUnit += 4;		// reserved global space
	    XPutLong(pUnit, 0L);
	    pUnit += 4;		// reserved global space
	    XPutLong(pUnit, 0L);
	    pUnit += 4;		// reserved global space
	    unitCount = pXInstrument->unitCount;
	    *pUnit = (char)unitCount;
	    pUnit++;					// byte
	    if (unitCount)
		{
		    for (count = 0; count < unitCount; count++)
			{
			    unitType = pXInstrument->units[count].unitType;
			    XPutLong(pUnit, unitType);
			    pUnit += 4;	// long
			    switch (unitType)
				{
				case INST_ADSR_ENVELOPE:
				    pENV = &pXInstrument->units[count].u.envelopeADSR;

				    XRemoveZeroEnvelope(pENV);			// remove extra 0 time point from editing

				    unitSubCount = pENV->stageCount;		// how many unit records?
				    *pUnit = (char)unitSubCount;
				    pUnit++;					// byte
				    for (count2 = 0; count2 < unitSubCount; count2++)
					{
					    XPutLong(pUnit, pENV->level[count2]);
					    pUnit += 4;

					    XPutLong(pUnit, pENV->time[count2]);
					    pUnit += 4;

					    XPutLong(pUnit, pENV->flags[count2]);
					    pUnit += 4;
					}
				    break;

				case INST_DEFAULT_MOD:
				    // ignore
				    break;

				case INST_EXPONENTIAL_CURVE:					// curve entry
				    pCurve = &pXInstrument->units[count].u.curve;
				    XPutLong(pUnit, pCurve->tieFrom);
				    pUnit += 4;
				    XPutLong(pUnit, pCurve->tieTo);
				    pUnit += 4;
				    unitSubCount = pCurve->curveCount;
				    *pUnit++ = (char)unitSubCount;

				    for (count2 = 0; count2 < unitSubCount; count2++)
					{
					    *pUnit++ = pCurve->from_Value[count2];
					    XPutShort(pUnit, pCurve->to_Scalar[count2]);
					    pUnit += 2;
					}
				    break;

				case INST_LOW_PASS_FILTER:		// low pass global filter parameters
				    XPutLong(pUnit, pXInstrument->units[count].u.lpf.LPF_frequency);
				    pUnit += 4;
				    XPutLong(pUnit,pXInstrument->units[count].u.lpf.LPF_resonance);
				    pUnit += 4;
				    XPutLong(pUnit, pXInstrument->units[count].u.lpf.LPF_lowpassAmount);
				    pUnit += 4;
				    break;

				    // LFO types
				case INST_VOLUME_LFO:
				case INST_PITCH_LFO:
				case INST_STEREO_PAN_LFO:
				case INST_STEREO_PAN_NAME2:
				case INST_LOW_PASS_AMOUNT:
				case INST_LPF_DEPTH:
				case INST_LPF_FREQUENCY:
				    pLFO = &pXInstrument->units[count].u.lfo;
				    XRemoveZeroEnvelope(&pLFO->envelopeLFO);		// remove extra 0 time point from editing
				    unitSubCount = pLFO->envelopeLFO.stageCount;
				    *pUnit = (char)unitSubCount;
				    pUnit++;					// byte
				    for (count2 = 0; count2 < unitSubCount; count2++)
					{
					    XPutLong(pUnit, pLFO->envelopeLFO.level[count2]);
					    pUnit += 4;
					    XPutLong(pUnit, pLFO->envelopeLFO.time[count2]);
					    pUnit += 4;
					    XPutLong(pUnit, pLFO->envelopeLFO.flags[count2]);
					    pUnit += 4;
					}

				    XPutLong(pUnit, pLFO->period);
				    pUnit += 4;
				    XPutLong(pUnit, pLFO->waveShape);
				    pUnit += 4;
				    XPutLong(pUnit, pLFO->DC_feed);
				    pUnit += 4;
				    XPutLong(pUnit, pLFO->depth);
				    pUnit += 4;
				    break;
				}
			}
		}

	    if (theX)
		{
		    tempSpaceSize = pUnit - tempSpace;		// determine size of new block of units

		    // search for end of tremlo data $8000. If not there, don't walk past end of instrument
		    pSplits = (KeySplit *) ( ((char *)&theX->keySplitCount) + sizeof(short));
		    count = XGetShort(&theX->keySplitCount);
		    pData = (char *)&pSplits[count];
		    pData2 = (char *)theX;
		    size = pData - pData2;
		    offset = 0;
		    for (count = 0; count < size; count++)
			{
			    data = XGetShort(&pData[count]);
			    if (data == 0x8000)
				{
				    count += 4;								// skip past end token and extra word
				    data = (unsigned short)pData[count] + 1;			// get first string length;
				    count2 = (long)pData[count+data] + 1;			// get second string length
				    pUnit = (char *) (&pData[count + data + count2]);
				    offset = pUnit - (char *)theX;
				    // NOTE: src will be non aligned, possibly on a byte boundry.
				    break;
				}
			}
		    if (offset == 0L)
			{
				// somethings bad so bail
			    XDisposePtr((XPTR)newInstrument);
			    newInstrument = NULL;
			}
		    else
			{
			    if (offset < theXSize)
				{
				    newInstrument = (InstrumentResource *)XNewPtr(offset + tempSpaceSize);
				    if (newInstrument)
					{
					    XBlockMove((XPTR)theX, (XPTR)newInstrument, offset);
					    // tack onto end the new block of information
					    XBlockMove(tempSpace, (char *)newInstrument + offset, tempSpaceSize);

					    newInstrument->flags1 |= ZBF_extendedFormat;
					}
				}
			    else
				{
				    // too big!?
				    XDisposePtr((XPTR)newInstrument);
				    newInstrument = NULL;
				}
			}
		}
	}
    XDisposePtr(tempSpace);
    return newInstrument;
}

#endif	// USE_CREATION_API == TRUE

// EOF of X_Instruments.c

