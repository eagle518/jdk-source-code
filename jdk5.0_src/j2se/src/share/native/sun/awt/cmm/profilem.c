/*
 * @(#)profilem.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)profilem.c	1.14 99/01/24

	Contains:	forward and inverse monochrome PT via PTNewMonoPT

	Written by:	Color Proccessor group

	COPYRIGHT (c) 1995-1999 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "makefuts.h"
#include "attrib.h"
#include "kcptmgr.h"

/*------------------------------------------------------------------------------
 *  PTNewMonoPT
 *------------------------------------------------------------------------------
 */

PTErr_t
	 PTNewMonoPT (	ResponseRecord_p	grayTRC,
					KpUInt32_t			gridsize,
					KpBool_t			invert,
					PTRefNum_p			thePTRefNumP)
{
PTErr_t		PTErr;
KpInt32_t	dim[3], inSpace, outSpace;
fut_p		theFut = NULL;

	/* Check for valid ptrs */
	PTErr = KCP_BAD_ARG;
	if (thePTRefNumP == NULL) goto GetOut;
	if (grayTRC == NULL) goto GetOut;
	if (gridsize < 2) goto GetOut;

	*thePTRefNumP = 0;

	/* all dimensions are the same */
	dim[0] = dim[1] = dim[2] = (KpInt32_t) gridsize;
	
	/* pass the input arguments along to the fut maker */
	if (invert == KPFALSE) {	
		/* Create (1D -> 3D) FuT */
		theFut = fut_new_empty (1, dim, 3, KCP_FIXED_RANGE, KCP_LAB_PCS);	
		if (theFut == NULL) {
			goto ErrOut4;
		}

		PTErr = makeForwardXformMono (grayTRC, theFut);

		inSpace = KCM_MONO;			/* setup the foward color space */
		outSpace = KCM_CIE_LAB; 
	}
	else {	
		/* Create (3D -> 1D) FuT */
		theFut = fut_new_empty (3, dim, 1, KCP_LAB_PCS, KCP_FIXED_RANGE);	
		if (theFut == NULL) {
			goto ErrOut4;
		}

		PTErr = makeInverseXformMono (grayTRC, theFut);

		inSpace = KCM_CIE_LAB;		/* setup the inverse color space */
		outSpace = KCM_MONO; 
	}
	
	if (PTErr != KCP_SUCCESS) {
	   goto ErrOut1;
	}

	if (fut_to_mft (theFut) != 1) {			/* convert to reference tables */
		goto ErrOut3;
	}

	PTErr = fut2PT (&theFut, inSpace, outSpace, PTTYPE_CALCULATED, thePTRefNumP);	/* make into PT */
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}

GetOut:
	return (PTErr);


ErrOut4:
	PTErr = KCP_NO_MEMORY;
	goto ErrOut0;

ErrOut3:
	PTErr = KCP_INCON_PT;
	goto ErrOut0;

ErrOut1:
	PTErr = KCP_BAD_ARG;

ErrOut0:
	if (theFut != NULL) fut_free (theFut);
	if (*thePTRefNumP != 0) PTCheckOut (*thePTRefNumP);
	goto GetOut;
}
