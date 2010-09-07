/*
 * @(#)kcmptlib.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	kcmptlib.h

	Contains:       Header file for KCMS Processor Library

	Copyright (c) 1992-2000 Eastman Kodak Company, all rights reserved.
 */

#ifndef _KCMSPTLIB_H
#define _KCMSPTLIB_H 1

#include "kcms_sys.h"
#include "kcmptdef.h"

#ifdef KPWIN
typedef PTErr_t (FAR PASCAL *PTProgress_t) (KpInt32_t);
#else
typedef PTErr_t (*PTProgress_t) (KpInt32_t);
#endif


#ifdef __cplusplus
extern "C" {
#endif
PTErr_t PTCheckIn (PTRefNum_p, PTAddr_t);
PTErr_t PTCheckOut (PTRefNum_t);
PTErr_t PTActivate (PTRefNum_t, KpInt32_t, PTAddr_t);
PTErr_t PTDeActivate (PTRefNum_t);
PTErr_t PTGetPTInfo (PTRefNum_t, PTAddr_h, PTAddr_h, PTAddr_h);
PTErr_t PTGetAttribute (PTRefNum_t, KpInt32_t, KpInt32_p, KpChar_p);
PTErr_t PTSetAttribute (PTRefNum_t, KpInt32_t, KpChar_p);
PTErr_t PTGetTags (PTRefNum_t, KpInt32_p, KpInt32_p);
PTErr_t PTGetSize (PTRefNum_t, KpInt32_p);
PTErr_t PTGetPT (PTRefNum_t, KpInt32_t, PTAddr_t);
PTErr_t PTGetSizeF (PTRefNum_t, PTType_t, KpInt32_p);
PTErr_t PTGetPTF (PTRefNum_t, PTType_t, KpInt32_t, PTAddr_t);
PTErr_t PTEval (PTRefNum_t, PTEvalPB_p, PTEvalTypes_t, KpInt32_t, KpInt32_t, opRefNum_p, PTProgress_t);
PTErr_t PTEvalDT (PTRefNum_t, PTEvalDTPB_p, PTEvalTypes_t, KpInt32_t, KpInt32_t, opRefNum_p, PTProgress_t);
PTErr_t PTEvalRdy (opRefNum_t, KpInt32_p);
PTErr_t PTEvalCancel (opRefNum_t);
PTErr_t PTChainValidate (KpInt32_t, PTRefNum_p, KpInt32_p);
PTErr_t PTChainInit (KpInt32_t, PTRefNum_p, KpInt32_t, KpInt32_p);
PTErr_t PTChainInitM (KpInt32_t, PTRefNum_p, KpInt32_t, KpInt32_t);
PTErr_t PTChain (PTRefNum_t);
PTErr_t PTChainEnd (PTRefNum_p);
PTErr_t PTCombine (KpInt32_t, PTRefNum_t, PTRefNum_t, PTRefNum_p);
PTErr_t PTEvaluators (KpInt32_p, evalList_p);
PTErr_t PTProcessorReset (void);
PTErr_t PTInitialize (void);
PTErr_t PTInitCMS (KpMemoryData_t MemoryData);
PTErr_t PTResetCMS ();
PTErr_t PTGetAuxPT (const KpChar_p, PTRefNum_p);
#if defined (KPWIN)
PTErr_t PTInitializeEx (PTInitInfo_p InitInfo);
#endif
PTErr_t PTTerminate (void);
PTErr_t PTInitGlue (void);
PTErr_t PTTermGlue (void);
PTErr_t PTNewEmpty (KpInt32_t, KpInt32_p, KpInt32_t, PTRefNum_p);
PTErr_t PTNewEmptySep (KpInt32_t, KpInt32_p, PTRefNum_p);
PTErr_t PTInvert (PTRefNum_t, KpInt32_t);
PTErr_t PTGetItbl (PTRefNum_t, KpInt32_t, KpInt32_t, KpHandle_t FAR*);
PTErr_t PTGetGtbl (PTRefNum_t, KpInt32_t, KpInt32_p, KpInt32_p, KpHandle_t FAR*);
PTErr_t PTGetOtbl (PTRefNum_t, KpInt32_t, KpHandle_t FAR*);
PTErr_t PTNewMatGamAIPT (FixedXYZColor_p, FixedXYZColor_p, 
			FixedXYZColor_p, ResponseRecord_p, ResponseRecord_p,
			ResponseRecord_p, KpUInt32_t, KpBool_t, newMGmode_p, PTRefNum_p);
PTErr_t PTNewMatGamPT (FixedXYZColor_p, FixedXYZColor_p, 
			FixedXYZColor_p, ResponseRecord_p, ResponseRecord_p,
			ResponseRecord_p, KpUInt32_t, KpBool_t, PTRefNum_p);
PTErr_t PTNewMonoPT (ResponseRecord_p, KpUInt32_t, KpBool_t, PTRefNum_p);
PTErr_t PTGetRelToAbsPT (KpUInt32_t, PTRelToAbs_p, PTRefNum_p);
PTErr_t PTApplInitialize (KpHandle_t, KpInt32_t);
PTErr_t PTGetFlavor (KpInt32_p);
#if defined(KPMAC)
void KCMPTSetComponentInstance (KpInt32_t);
void KCMPTGetComponentInstance (KpInt32_p);
#else
PTErr_t PTInitThread (void);
PTErr_t PTTermThread (void);
#endif
PTErr_t PTGetMPState (KpUInt32_p, KpUInt32_p);
PTErr_t PTSetMPState (KpUInt32_t MP_Used);

PTErr_t PTCreateTRC (KpUInt16_p, KpFloat32_t);

#ifdef __cplusplus
}
#endif

#endif
