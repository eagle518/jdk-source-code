/*
 * @(#)spxfmtag.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*	ShaperMatrix.c @(#)spxfmtag.c	1.30 99/01/20

	These routines convert a pt into a set of shaper tables followed by
	a matrix.
	
	They are based on the MATLAB routine pt2shmtx.m developed by Doug Walker

	The call is:
	
		SpStatus_t ComputeShaperMatrix (
				PTRefNum_t	pt,
				double		FAR *shaper [3],
				double		ColorMatrix [3] [3]);
	
	where
	
		pt = the input pt to be converted
		
		shaper[i][j] = the shaper table for color i and index j
		ColorMatrix[i][j] = the matrix for RGB index i and XYZ index j
		
	The results should approimate
	
	RCStoXYZ(pt([R,G,B])) = Sum(k) ( ShapedRGB[k] * ColorMatrix[k,:])
		where ShapedRGB = [ shaper[0][R], shaper[1][B], shaper[2][G] ]
		
	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1993-1999
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <string.h>
#include <stdio.h>
#include "sprof-pr.h"


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Generate Colorant and Response Curve tags for the specified
 *	Xform data.  This data is assumed to be a PT.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	April 15, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformCreateMatTags (
				SpProfile_t		Profile,
				KpInt32_t		DataSize,
				KpLargeBuffer_t	Data)
{
	SpStatus_t		Status;
	PTRefNum_t		RefNum;
	SpHeader_t		Hdr;
	KpInt32_t		SpDataType;

	Status = SpProfileGetHeader (Profile, &Hdr);
	if (SpStatSuccess != Status)
		return Status;

	if ((Hdr.Originator == SpSigOrgKodak1_Ver_0) ||
	    (Hdr.Originator == SpSigOrgKodak2_Ver_0)) {
		SpDataType = KCM_ICC_TYPE_0;
	}
	else {
		SpDataType = KCM_ICC_TYPE_1;
	}

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nSpXformCreateMatTags\n Profile %x, DataSize %d, Data %x\n", Profile, DataSize, Data);
	kcpDiagLog (string); }
	#endif

	Status = SpXformLoadImp (Data, DataSize, 
			SpDataType, Hdr.DataColorSpace, Hdr.InterchangeColorSpace, &RefNum);

	if (SpStatSuccess != Status)
		return Status;

	Status = SpXformCreateMatTagsFromPT (Profile, RefNum);

	PTCheckOut (RefNum);

	return Status;
}


/*---------------------------------------------------------------------
	DESCRIPTION
	This function transforms a pel array (in place) using a pt
	The array is assumed to be in plane-sequential order
	The data type is assumed to be 12-bit (KCM_USHORT_12)

	INPUTS	
	PTRefNum_t	pt			---	reference # of the pt to be transformed
	KpUInt16_t	FAR *Pels	---	pointer to the data
	int		nPels			---	number of pixels in data stream

	OUTPUTS
	KpUInt16_t	FAR *Pels	--- pointer to data that has been xformed

	RETURNS
	SpStatus_t - SpStatSuccess if successful, otherwise an sprofile error
 
   AUTHOR
   mtobin
-------------------------------------------------------------------------*/
SpStatus_t Transform12BPels (
				PTRefNum_t	pt,
				KpUInt16_t	FAR *Pels,
				int		nPels)
{
	opRefNum_t		opRefNum;
	PTEvalDTPB_t	dtpb;
	PTErr_t			PTStat;
	KpInt32_t		i;
	PTCompDef_t		comps [3];
	SpStatus_t		Status;

	for (i = 0; i < 3; i++) {
		comps [i].pelStride = 3 * sizeof(*Pels);
		comps [i].lineStride = nPels * 3 * sizeof (*Pels);
		comps [i].addr = (void FAR *) (Pels+i);
	}

/* build color processor image structure */
	dtpb.nPels = nPels;
	dtpb.nLines = 1;
	dtpb.nInputs = 3;
	dtpb.input = comps;
	dtpb.dataTypeI = KCM_USHORT_12;
	dtpb.nOutputs = 3;
	dtpb.output = comps;  /* evaluate in place */
	dtpb.dataTypeO = KCM_USHORT_12;

/* do the transformation */
	PTStat = PTEvalDT (pt, &dtpb, KCP_EVAL_DEFAULT, 0, 1, &opRefNum, NULL);

/* translate the return value to an Sp error */
	Status = SpStatusFromPTErr(PTStat);
	
	return Status;
}

/* ---------------------------------------------------------------------- */
SpStatus_t TransformPels (
				PTRefNum_t	pt,
				KpUInt8_t	FAR *Pels,
				int			nPels)
{
/*
 * Transform the pel array using the pt
 * The array is assumes to be in plane-sequential order
 */

	PTEvalPB_t		dtpb;
	PTCompDef_t		comps [3];
	opRefNum_t		opRefNum;
	int				i;
	PTErr_t			pterr;

	for (i = 0; i < 3; i++) {
		comps [i].pelStride = 3 * sizeof(*Pels);
		comps [i].lineStride = nPels * 3 * sizeof (*Pels);
		comps [i].addr = (void FAR *) (Pels+i);
	}

	dtpb.nPels = nPels;
	dtpb.nLines = 1;
	dtpb.nOutputs =
	dtpb.nInputs = 3;
	dtpb.output =
	dtpb.input = comps;

	pterr = PTEval (pt, &dtpb, KCP_EVAL_DEFAULT, 0, 1, &opRefNum, NULL);

	return (SpStatusFromPTErr(pterr));
}
