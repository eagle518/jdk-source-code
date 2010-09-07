/*
 * @(#)speval.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the composition and evaluation functions.

				Created by lsh, September 20, 1993

	SCCS Revision:
		@(#)speval.c	1.47	11/24/98
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-1998                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"

#include "spcback.h"
#include "spevalcb.h"

#include "kcmptdef.h"
#include "kcmptlib.h"


#if defined(KPMAC)
#if (!defined KPMACPPC) & (defined KPMW) 
	#include <A4Stuff.h>
#endif

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do progress callback. - MAC version
 *
 * AUTHOR
 * 	mec
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t SpDoProgress (
				SpProgress_t	ProgressFunc,
				SpIterState_t	State,
				KpInt32_t		Percent,
				void			FAR *Data)
{
	#if defined(KPMAC68K)
	volatile long			hostA4, hostA5;
	volatile long 			thisA4, thisA5;
	#endif
	SpStatus_t  status;
	
	if (NULL == ProgressFunc)
		return SpStatSuccess;

	#if defined(KPMAC68K)
	/* restore host's global world - we don't know if its an A4 or A5*/
	SPretrieveCallbackA4A5(&hostA4, &hostA5);
	if (hostA5 != 0)
		thisA5 = SetA5(hostA5);			
	if (hostA4 != 0)
		thisA4 =  SetA4(hostA4);
	#endif	

	status = CallSPCallBackFunc((spCallBackUPP)ProgressFunc,State, Percent, Data);

	#if defined (KPMAC68K)
	/* reset our global world */
	if (hostA5 != 0)
		SetA5(thisA5);								
	if (hostA4 != 0)
		SetA4(thisA4);								
	#endif

	return status;
}

#else
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do progress callback. - non MAC version
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 23, 1993
 *------------------------------------------------------------------*/
SpStatus_t SpDoProgress (
				SpProgress_t	ProgressFunc,
				SpIterState_t	State,
				KpInt32_t		Percent,
				void			FAR *Data)
{
	if (NULL != ProgressFunc)
		return ProgressFunc (State, Percent, Data);

	return SpStatSuccess;
}
#endif

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert Profile combine type to Color Processor combine type.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	June 17, 1994
 *------------------------------------------------------------------*/
KpInt32_t SpConnectTypeToPTCombineType (
				SpConnectType_t	ConnectType)
{
KpInt32_t		ptCombineType;

	switch (ConnectType & SpConnect_Type_Mask) {
	case SpConnect_Std:
		ptCombineType = PT_COMBINE_STD;
		break;

	case SpConnect_pf_8:
		ptCombineType = PT_COMBINE_PF_8;
		break;

	case SpConnect_pf_16:
		ptCombineType = PT_COMBINE_PF_16;
		break;

	case SpConnect_Serial:
		ptCombineType = PT_COMBINE_SERIAL;
		break;

	case SpConnect_pf:
	default:
		ptCombineType = PT_COMBINE_PF;
		break;
	}

	switch (ConnectType & SpConnect_Mode_Mask) {
	case SpConnect_Largest:
		ptCombineType |= PT_COMBINE_LARGEST;
		break;
	}

	return (ptCombineType);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get platform specific combine type.  Limit size for ICM.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	June 17, 1994
 *------------------------------------------------------------------*/
SpConnectType_t SpGetConnectType (
				void)
{
#if defined (ICM)			/* KPWIN32 ICM */
	return SpConnect_pf | SpConnect_Largest;
#elif defined (SOLARIS_CMM) || defined (JAVACMM)
	return SpConnect_pf_16 | SpConnect_Largest;
#else
	return SpConnect_pf | SpConnect_Largest;
#endif
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Chain a set of transforms together into a single transform.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	July 5, 1994
 *------------------------------------------------------------------*/
static SpStatus_t SpChainSequenceImp (
				KpInt32_t		PtCombineType,
				KpInt32_t		RefNumCnt,
				PTRefNum_t		FAR *RefNumSequence,
				PTRefNum_t		FAR *Result,
				KpInt32_t		FAR *FailingRefNum,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
	int				Index;
	PTErr_t			PTStat;

	*FailingRefNum = -1;

/* initialize for chainning */
	PTStat = PTChainInitM (RefNumCnt, RefNumSequence, PtCombineType, KPTRUE);
	if (KCP_SYSERR_1 == PTStat)
		return SpStatUnsupported;

/* now do the chainning */
	for (Index = 0;
			(KCP_SUCCESS == PTStat) && (Index < RefNumCnt);
					Index++) {

		SpDoProgress (ProgressFunc, SpIterProcessing,
						(100 * Index) / RefNumCnt, Data);

		PTStat = PTChain (*RefNumSequence++);
		*FailingRefNum = Index;
	}

/* terminate the chain */
	if (KCP_SUCCESS == PTStat)
		PTStat = PTChainEnd (Result);

	return (SpStatusFromPTErr(PTStat));
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Connect a set of transforms together into a single transform
 *	that will yield the 'same' results as evaluating data through
 *	each of the transforms serially.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t SpConnectSequenceImp (
				SpConnectType_t	ConnectType,
				KpInt32_t		RefNumCnt,
				PTRefNum_t		FAR *RefNumSequence,
				PTRefNum_t		FAR *Result,
				KpInt32_t		FAR *FailingRefNum,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
	SpStatus_t		Status;
	int				Index;
	KpInt32_t		PtCombineType;
	PTErr_t			PTStat;
	PTRefNum_t		RefNum1;
	PTRefNum_t		RefNum2;
	PTRefNum_t		ResultRefNum;

/* initialize callers return values */
	*FailingRefNum = -1;

/* convert to ColorProcessor combine type */
	PtCombineType = SpConnectTypeToPTCombineType (ConnectType);

/*---------------------*/
/* try to do chainning */
/*---------------------*/

	Status = SpChainSequenceImp (PtCombineType, RefNumCnt, RefNumSequence,
									Result, FailingRefNum, ProgressFunc, Data);

/* done if chainning worked */
	if (SpStatSuccess == Status)
		return Status;

/*----------------------------*/
/* try to do combines instead */
/*----------------------------*/

/* prime the pump by combining the first two transforms */
	RefNum1 = RefNumSequence [0];
	RefNum2 = RefNumSequence [1];
	PTStat = PTCombine (PtCombineType, RefNum1, RefNum2, &ResultRefNum);
	if (KCP_SUCCESS != PTStat) {
		*FailingRefNum = 1;
		return (SpStatusFromPTErr(PTStat));
	}

/* do the rest */
	for (Index = 2; Index < RefNumCnt; Index++) {
		SpDoProgress (ProgressFunc, SpIterProcessing,
						(100 * (Index-1)) / (RefNumCnt-1), Data);
		RefNum1 = ResultRefNum;
		RefNum2 = RefNumSequence [Index];
		PTStat = PTCombine (PtCombineType, RefNum1, RefNum2, &ResultRefNum);
		PTCheckOut (RefNum1);

		if (KCP_SUCCESS != PTStat) {
			*FailingRefNum = Index;
			return (SpStatusFromPTErr(PTStat));
		}
	}

	*Result = ResultRefNum;
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Connect a set of transforms together into a single transform
 *	USE PTCOMBINE ONLY! (This is called from pt2pf via 
 *	SpXformFromPTRefNumSpecial()
 *
 * AUTHOR
 * 	lcc
 *
 * DATE CREATED
 *	February 9, 1995
 *------------------------------------------------------------------*/
SpStatus_t SpConnectSequenceCombine (
				SpConnectType_t	ConnectType,
				KpInt32_t		RefNumCnt,
				PTRefNum_t		FAR *RefNumSequence,
				PTRefNum_t		FAR *Result,
				KpInt32_t		FAR *FailingRefNum,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
	int				Index;
	KpInt32_t		PtCombineType;
	PTErr_t			PTStat;
	PTRefNum_t		RefNum1;
	PTRefNum_t		RefNum2;
	PTRefNum_t		ResultRefNum;

/* initialize callers return values */
	*FailingRefNum = -1;

/* convert to ColorProcessor combine type */
	PtCombineType = SpConnectTypeToPTCombineType (ConnectType);

/*-------------------*/
/*  do combines only */
/*-------------------*/

/* prime the pump by combining the first two transforms */
	RefNum1 = RefNumSequence [0];
	RefNum2 = RefNumSequence [1];
	PTStat = PTCombine (PtCombineType, RefNum1, RefNum2, &ResultRefNum);
	if (KCP_SUCCESS != PTStat) {
		*FailingRefNum = 1;
		return (SpStatusFromPTErr(PTStat));
	}

/* do the rest */
	for (Index = 2; Index < RefNumCnt; Index++) {
		SpDoProgress (ProgressFunc, SpIterProcessing,
						(100 * (Index-1)) / (RefNumCnt-1), Data);
		RefNum1 = ResultRefNum;
		RefNum2 = RefNumSequence [Index];
		PTStat = PTCombine (PtCombineType, RefNum1, RefNum2, &ResultRefNum);
		PTCheckOut (RefNum1);

		if (KCP_SUCCESS != PTStat) {
			*FailingRefNum = Index;
			return (SpStatusFromPTErr(PTStat));
		}
	}

	*Result = ResultRefNum;
	return SpStatSuccess;
}


static void CopyXYZ(KpF15d16XYZ_t *Src,
		    FixedXYZColor_t *Dst)
{
	*Dst = *(FixedXYZColor_t *)Src;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Connect a set of transforms together into a single transform
 *	that will yield the 'same' results as evaluating data through
 *	each of the transforms serially.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpConnectSequenceEx (
				SpConnectType_t	ConnectType,
				KpInt32_t		XformCnt,
				SpXform_t		FAR *XformSequence,
				SpXform_t		FAR *Result,
				KpInt32_t		FAR *FailingXform,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
	int				Index;
	SpStatus_t		Status;
	KpInt32_t		ptConnectType;
	PTRefNum_t		ResultRefNum;
	PTRefNum_t		RefNumArray [10];
	PTRefNum_t		FAR *RefNumList;
	PTRefNum_t		FAR *RefNumPtr;

	PTErr_t			R2AStatus;
	KpBool_t		FAR *NeedR2A;
	PTRefNum_t		FAR *R2APTs;
	KpInt32_t		R2ACnt = 0;
	KpBool_t		HaveWP = KPFALSE;
	SpXformData_t		*ThisXform;
	KpUInt32_t		R2AMode = 0;
	PTRelToAbs_t		R2AParam;
	PTRefNum_t		ThisR2APT;
	KpInt32_t		SpaceIn;

/* initialize callers return values */
	*FailingXform = -1;
	*Result = NULL;

/* validate that we have atleast two transforms */
	if (XformCnt < 2)
		return SpStatOutOfRange;

	/* Set up an array for needed R2A PTs */
	NeedR2A = SpMalloc ((XformCnt) * 
                           (KpInt32_t)sizeof (KpBool_t));
	if (NeedR2A == NULL)
		return SpStatMemory;

	/* Set Up an Array to hold the PTs */
	R2APTs  = SpMalloc ((XformCnt) * 
                           (KpInt32_t)sizeof (*RefNumList));
	if (R2APTs == NULL)
	{
		SpFree(NeedR2A);
		return SpStatMemory;
	}

	/* Clear the Flag Array */
	for (Index = 0; Index < XformCnt; Index++)
		NeedR2A[Index] = KPFALSE;

	/* preset the size of the parameter structure  
	   and the Lut size */
	R2AParam.RelToAbsSize = sizeof(PTRelToAbs_t);
	R2AParam.gridSize = 8;

	/* Loop thru the Xforms in reverse to make getting
	   White Points easier */
	for (Index = XformCnt - 1; Index >= 0; Index--)
	{
		ThisXform = SpXformLock(XformSequence[Index]);
		if (ThisXform == NULL)
		{
			SpFree(NeedR2A);
			SpFree(R2APTs);
			return SpStatBadXform;
		}
		/* Last Xform was Absolute so gave up White Point */
		if (HaveWP == KPTRUE)
		{
			HaveWP    = KPFALSE;
			/* This Xform has a valid Media WP,  That's
			   the only one currently used */
			if (ThisXform->MedWPValid)
			{
				CopyXYZ(&ThisXform->MedWtPoint,
					&R2AParam.srcMediaWhitePoint);
				if (ThisXform->HdrWPValid)
					CopyXYZ(&ThisXform->HdrWtPoint,
						&R2AParam.srcProfileWhitePoint);

				if ( ( (SpaceIn == SpSpaceLAB) || 
					(SpaceIn == SpSpaceXYZ) ) &&
					( (ThisXform->SpaceOut == SpSpaceLAB) ||
					(ThisXform->SpaceOut == SpSpaceXYZ) ) ) {
				/* Only put white point conversion xform 
				   between PCS connection points */
				    R2AStatus = PTGetRelToAbsPT(R2AMode,
					&R2AParam, &ThisR2APT);
				    /* Bad status could mean not
				       implemented yet */
				    if (R2AStatus == KCP_SUCCESS)
				    {
				        NeedR2A[Index] = KPTRUE;
				        R2APTs[Index] = ThisR2APT;
				        R2ACnt++;
				    }
				}
			}
		}

		/* Do not need to test the first for Absolute */
		if (Index == 0)
		{
			SpXformUnlock(XformSequence[Index]);
			break;
		}

		if (ThisXform->WhichRender == SpTransRenderAbsColormetric)
		{
			if (ThisXform->MedWPValid)
			{
				HaveWP = KPTRUE;
				CopyXYZ(&ThisXform->MedWtPoint,
					&R2AParam.dstMediaWhitePoint);
				if (ThisXform->HdrWPValid)
					CopyXYZ(&ThisXform->HdrWtPoint,
						&R2AParam.dstProfileWhitePoint);
				SpaceIn = ThisXform->SpaceIn;
			}
		}
		SpXformUnlock(XformSequence[Index]);
	} /* End reverse loop */

/* build a list of the RefNums */
	if ((XformCnt + R2ACnt) < 10)
		RefNumList = RefNumArray;
	else {
		RefNumList = SpMalloc ((XformCnt + R2ACnt) * 
                                       (KpInt32_t)sizeof (*RefNumList));
		if (NULL == RefNumList)
		{
			for (Index = 0; Index < XformCnt - 1; Index++)
			{
				if (NeedR2A[Index] == KPTRUE)
					PTCheckOut(R2APTs[Index]);
			}
			SpFree(NeedR2A);
			SpFree(R2APTs);
			return SpStatMemory;
		}
	}

	Status = SpStatSuccess;
	for (Index = 0, RefNumPtr = RefNumList;
			Index < XformCnt;
					Index++, RefNumPtr++, XformSequence++) {
		Status = SpXformGetRefNum (*XformSequence, RefNumPtr);
		if (SpStatSuccess != Status)
			break;

		/* If R2A needed, Throw in after the current one */
		if (NeedR2A[Index] == KPTRUE) 
		{
			RefNumPtr++;
			*RefNumPtr = R2APTs[Index];
		}
	}

/* clean up if errors while building list of reference numbers */
	if (SpStatSuccess != Status) {
		if (RefNumList != RefNumArray)
			SpFree (RefNumList);
		for (Index = 0; Index < XformCnt - 1; Index++)
		{
			if (NeedR2A[Index] == KPTRUE)
				PTCheckOut(R2APTs[Index]);
		}
		SpFree(NeedR2A);
		SpFree(R2APTs);
		return Status;
	}

/* connect the xforms using the appropriate connection method	*/
	SpDoProgress (ProgressFunc, SpIterInit, 0, Data);

	switch (ConnectType & SpConnect_Method_Mask) {

		case SpConnect_Default:
			Status = SpConnectSequenceImp (ConnectType, XformCnt + R2ACnt, RefNumList,
										   &ResultRefNum, FailingXform, 
										   ProgressFunc, Data);
			break;

		case SpConnect_Chain:
			ptConnectType = SpConnectTypeToPTCombineType (ConnectType);
			Status = SpChainSequenceImp (ptConnectType, XformCnt + R2ACnt, RefNumList,
										 &ResultRefNum, FailingXform, 
										 ProgressFunc, Data);
			break;

		case SpConnect_Combine:
			Status = SpConnectSequenceCombine (ConnectType, XformCnt + R2ACnt, RefNumList,
										   	   &ResultRefNum, FailingXform, 
										   	   ProgressFunc, Data);
			break;

		default:
			if (RefNumList != RefNumArray) {
				SpFree (RefNumList);
			}
			for (Index = 0; Index < XformCnt - 1; Index++)
			{
				if (NeedR2A[Index] == KPTRUE)
					PTCheckOut(R2APTs[Index]);
			}
			SpFree(NeedR2A);
			SpFree(R2APTs);
			return SpStatOutOfRange;

	}

	SpDoProgress (ProgressFunc, SpIterTerm, 100, Data);

/* clean up reference number list, if needed */
	if (RefNumList != RefNumArray)
		SpFree (RefNumList);

	for (Index = 0; Index < XformCnt - 1; Index++)
	{
		if (NeedR2A[Index] == KPTRUE)
			PTCheckOut(R2APTs[Index]);
	}
	SpFree(NeedR2A);
	SpFree(R2APTs);

/* make an SpXform from the color processor PTRefNum */
	if (SpStatSuccess == Status)
		Status = SpXformFromPTRefNumImp (ResultRefNum, Result);

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION (Private)
 *	Connect transforms using default grid size.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpConnectSequence (
				KpInt32_t			XformCnt,
				SpXform_t			FAR *XformSequence,
				SpXform_t			FAR *Result,
				KpInt32_t			FAR *FailingXform,
				SpProgress_t		ProgressFunc,
				void				FAR *Data)
{
	return SpConnectSequenceEx (SpGetConnectType (), XformCnt, XformSequence,
								Result, FailingXform, ProgressFunc, Data);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Validate layout structure.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 19, 1993
 *------------------------------------------------------------------*/
static SpStatus_t SpValidateLayout (
				SpPixelLayout_t	FAR *Layout,
				int				FAR *DataType)
{
	if (SpMaxComponents < Layout->NumChannels)
		return SpStatOutOfRange;

	switch (Layout->SampleType) {
	case SpSampleType_UByte:
		*DataType = KCM_UBYTE;
		break;

	case SpSampleType_x555Word:
		*DataType = KCM_USHORT_555;
		if (1 != Layout->NumChannels)
			return SpStatOutOfRange;

		break;

	case SpSampleType_565Word:
		*DataType = KCM_USHORT_565;
		if (1 != Layout->NumChannels)
			return SpStatOutOfRange;

		break;

	case SpSampleType_UShort12:
		*DataType = KCM_USHORT_12;
		break;

	case SpSampleType_UShort:
		*DataType = KCM_USHORT;
		break;

	case SpSampleType_RGB10:
		*DataType = KCM_R10G10B10;
		break;

	default:
		return SpStatOutOfRange;
	}

	return SpStatSuccess;
}

/*--------------------------------------------------------------------*/
typedef struct {
	SpProgress_t	ProgressFunc;
	void			FAR *Data;
	SpStatus_t	UserReturn;
} SpProgThunk_t;

static KpThreadMemHdl_t Me;


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Progress call back mapper.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	August 5, 1994
 *------------------------------------------------------------------*/

#if defined (KPWIN16)
static PTErr_t FAR PASCAL _loadds ProgCallBack (
			KpInt32_t	Percent)
#else
static PTErr_t FAR PASCAL ProgCallBack (
			KpInt32_t	Percent)
#endif
{
	SpStatus_t		Status;
	SpProgThunk_t	FAR *Thunk;


	Thunk = KpThreadMemFind (&Me, KPTHREADMEM);
	if (NULL == Thunk)
		return KCP_SUCCESS;

	Status = SpDoProgress (Thunk->ProgressFunc, SpIterProcessing,
								 Percent, Thunk->Data);

	Thunk->UserReturn = Status;
	KpThreadMemUnlock (&Me, KPTHREADMEM);
	return (SpStatSuccess == Status) ? KCP_SUCCESS : KCP_FAILURE;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Process data through a transform.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpEvaluate (
				SpXform_t		Xform,
				SpPixelLayout_t	FAR *SrcLayout,
				SpPixelLayout_t	FAR *DestLayout,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
	SpStatus_t		Status;
	int				i;
	PTRefNum_t		RefNum;
	PTEvalDTPB_t	dtpb;
	PTCompDef_t		incomps [SpMaxComponents];
	PTCompDef_t		outcomps [SpMaxComponents];
	opRefNum_t		opRefNum;
	int				SrcDataType;
	int				DestDataType;
	PTProgress_t	ProgFunc;
	PTErr_t		PTStat;
	SpProgThunk_t	FAR *Thunk;

/* validate layout structures */
	Status = SpValidateLayout (SrcLayout, &SrcDataType);
	if (SpStatSuccess == Status)
		Status = SpValidateLayout (DestLayout, &DestDataType);

	if (SpStatSuccess != Status)
		return Status;

/* source and destination consistency */
	if (SrcLayout->NumRows != DestLayout->NumRows)
		return SpStatIncompatibleArguments;

	if (SrcLayout->NumCols != DestLayout->NumCols)
		return SpStatIncompatibleArguments;

/* build color processor input component definitions */
	for (i = 0; i < SrcLayout->NumChannels; i++) {
		incomps [i].pelStride = SrcLayout->OffsetColumn;
		incomps [i].lineStride = SrcLayout->OffsetRow;
		incomps [i].addr = SrcLayout->BaseAddrs [i];
	}

/* build color processor output component definitions */
	for (i = 0; i < DestLayout->NumChannels; i++) {
		outcomps [i].pelStride = DestLayout->OffsetColumn;
		outcomps [i].lineStride = DestLayout->OffsetRow;
		outcomps [i].addr = DestLayout->BaseAddrs [i];
	}

/* build color processor image structure */
	dtpb.nPels = SrcLayout->NumCols;
	dtpb.nLines = SrcLayout->NumRows;
	dtpb.nInputs = SrcLayout->NumChannels;
	dtpb.input = incomps;
	dtpb.dataTypeI = SrcDataType;
	dtpb.nOutputs = DestLayout->NumChannels;
	dtpb.output = outcomps;
	dtpb.dataTypeO = DestDataType;

/* get color processor reference number */
	Status = SpXformGetRefNum (Xform, &RefNum);
	if (SpStatSuccess != Status)
		return Status;

	SpDoProgress (ProgressFunc, SpIterInit, 0, Data);

	ProgFunc = NULL;
	if (NULL != ProgressFunc) {
		Thunk = KpThreadMemCreate (&Me, KPTHREADMEM, sizeof (SpProgThunk_t));
		if (NULL != Thunk) {
			ProgFunc = (PTProgress_t)(NewSPEvalCBFunc(ProgCallBack));
			Thunk->ProgressFunc = ProgressFunc;
			Thunk->Data = Data;
			Thunk->UserReturn = SpStatSuccess;
			KpThreadMemUnlock (&Me, KPTHREADMEM);
		}
	}

    if (KCP_SUCCESS != (PTStat = PTEvalDT (RefNum, &dtpb, KCP_EVAL_DEFAULT, 0, 1, &opRefNum, ProgFunc)))
	{
		Thunk = KpThreadMemFind (&Me, KPTHREADMEM);
		if (Thunk == NULL) {
			Status = SpStatusFromPTErr(PTStat);
		}
		else {
			if (Thunk->UserReturn == SpStatSuccess)
				/* User did not cancel - This is CP error */
				Status = SpStatusFromPTErr(PTStat);
			else
				/* User canceled - give them back their error */
				Status = Thunk->UserReturn;
		}
	}
	else
		Status = SpStatSuccess;
		
	SpDisposeRoutineDescriptor(ProgFunc);

	SpDoProgress (ProgressFunc, SpIterTerm, 100, Data);

	if (NULL != ProgressFunc)
		KpThreadMemDestroy (&Me, KPTHREADMEM);

	return Status;
}
     
