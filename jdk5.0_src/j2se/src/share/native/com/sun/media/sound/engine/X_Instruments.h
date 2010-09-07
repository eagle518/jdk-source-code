/*
 * @(#)X_Instruments.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
**	X_Instruments.h
**
**	Tools for creating instruments. The structure enclosed here are used
**	for create expanded editable structures.
**
**	History	-
**	2/16/98		Created. Pulled from MacOS specific editor codebase
**	11/10/98	Added XNewInstrumentWithBasicEnvelopeResource
*/
/*****************************************************************************/
#ifndef X_INSTRUMENTS
#define X_INSTRUMENTS

#ifndef __X_API__
#include "X_API.h"
#endif

#ifndef X_FORMATS
#include "X_Formats.h"
#endif

#ifndef G_SOUND
#include "GenSnd.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define FULL_RANGE				(VOLUME_RANGE * 2)
#define SUSTAIN_DEFAULT_TIME	45000


    // Basic envelope structure used for LFO's and ADSR's
    typedef struct
    {
	// Used for display
	INT32		startScaleH;
	INT32		endScaleH;

	INT32		startScaleV;
	INT32		endScaleV;

	INT32		startRangeV;
	INT32		endRangeV;

	INT32		stageCount;
	INT32		level[ADSR_STAGES+1];
	INT32		time[ADSR_STAGES+1];
	INT32		flags[ADSR_STAGES+1];
    } XEnvelopeData;

    typedef CurveRecord XTieToData;

    typedef struct
    {
	XEnvelopeData	envelopeLFO;

	INT32			period;
	INT32			waveShape;
	INT32			DC_feed;			// amount to use as ADSR
	INT32			depth;				// amount to use as LFO
    } XLFOData;

    typedef struct
    {
	INT32			LPF_frequency;
	INT32			LPF_resonance;
	INT32			LPF_lowpassAmount;
    } XLowPassFilterData;

    // Use one of these INST_XXXX for unitType in the structure XUnitData below. When there
    // are multiple LFO types use a different unitID for tracking.
    /*
      INST_ADSR_ENVELOPE
      INST_EXPONENTIAL_CURVE
      INST_LOW_PASS_FILTER
      INST_DEFAULT_MOD
      INST_PITCH_LFO
      INST_VOLUME_LFO
      INST_STEREO_PAN_LFO
      INST_STEREO_PAN_NAME2
      INST_LOW_PASS_AMOUNT
      INST_LPF_DEPTH
      INST_LPF_FREQUENCY
    */
    typedef UINT32	XUnitType;

    typedef struct
    {
	XUnitType		unitType;
	UINT32	unitID;

	union	
	    {
		XEnvelopeData		envelopeADSR;
		XLFOData			lfo;
		XLowPassFilterData	lpf;
		XTieToData			curve;
		XBOOL				useDefaultModwheelAction;
	    } u;
    } XUnitData;

    // The XInstrumentData structure is a deconstruction of the InstrumentResource structure
    typedef struct
    {
	// how many units in instrument, only used for reconstruction
	INT32			unitCount;
	// seperate unit information:
	XUnitData		units[256];
    }  XInstrumentData;

    typedef enum XEnvelopeType
    {
	NONE_E			=	0,
	FOUR_POINT_E,
	TWO_POINT_E,
	FLAT_FULL_E
    } XEnvelopeType;

    InstrumentResource*	XNewInstrumentResource(XShortResourceID leadSndID);
    InstrumentResource* XNewInstrumentWithBasicEnvelopeResource(XShortResourceID leadSndID, XEnvelopeType type);
    void				XDisposeInstrumentResource(InstrumentResource* theX);

    XInstrumentData*	XCreateXInstrument(InstrumentResource* theX,
					   UINT32 theXSize);

    // pass in original instrument resource (theX) its size (theXSize) and the structure XInstrumentData (pXInstrument)
    // to build a new instrument resource. The variable theX is not touched, and you can deallocate it after this
    // function is used.
    InstrumentResource*	XReconstructInstrument(InstrumentResource* theX,
					       UINT32 theXSize,
					       XInstrumentData* pXInstrument);

    INT32				XGetTotalEnvelopeTime(XEnvelopeData* pXEnvelope);

    void				XEnvelopeAdjustSustainTime(XEnvelopeData* pXEnvelope);

    INT32				XFindType(XInstrumentData* pXInstrument, XUnitType type);
    INT32				XAddType(XInstrumentData* pXInstrument, XUnitType type);
    void				XRemoveType(XInstrumentData* pXInstrument,
						    XUnitType unitType,
						    UINT32 unitID);

    XBOOL				XDeleteEnvelopePoint(XEnvelopeData* pEnvelope, short int whichPoint);

    // Given an envelope, rebuild the ADSR type to one of XEnvelopeType type
    void				XFillDefaultADSREnvelope(XEnvelopeData* pEnvelope, XEnvelopeType type);

    // Given an envelope, shift volume plus or minus 
    void				XShiftEnvelopeVolume(XEnvelopeData* pEnvelope, INT32 shift);

    // add extra 0 time point for editing
    void				XAddZeroEnvelope(XEnvelopeData* pEnvelope);

    // remove extra 0 time point from editing
    void				XRemoveZeroEnvelope(XEnvelopeData* pEnvelope);

    void				XAddDefaultADSREnvelope(XInstrumentData* pXInstrument, XEnvelopeType type);


#ifdef __cplusplus
}
#endif


#endif	// X_INSTRUMENTS
// EOF of X_Instruments.h

