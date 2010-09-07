/*
 * @(#)futiomem.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)futiomem.c	2.73 99/02/09

	Contains:	functions to read and write binary fut "memory" files.

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1992-1999
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.

These routines intended for use with a system which maintains its own fut
database.  The external database is responsible for loading and storing a
block of memory which contains all of the fut information.  It needs to know
only the location and size of this block.  The fut_io_mem routines are based
on the fut_io disk file routines and translate the external memory block into
the internal fut structure format needed for function evaluation.

To support architecture dependent byte ordering, byte swapping is performed as
neccessary when reading a file, by checking the byte ordering of the "magic"
numbers.  The "standard" byte ordering is Most Significant Byte First
(e.g. Sun, Macintosh) but this default can be overridden (see below).
 */


#include <string.h>
#include "fut.h"
#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "kcptmgr.h"
#include "attrib.h"
#include "attrcipg.h"
#include "makefuts.h"

#define	MATRIX_GRID_SIZE	16

/* prototypes */
static KpInt32_t fut_size_chan (fut_chan_p, chan_hdr_p);
static KpInt32_t fut_size_itbl (fut_itbl_p);
static KpInt32_t fut_size_otbl (fut_otbl_p);
static KpInt32_t fut_size_gtbl (fut_gtbl_p);
static PTDataClass_t getPTDataClass (PTRefNum_t, KpInt32_t);
static PTDataClass_t getDataClass (KpInt32_t);
static void checkInDataClass (PTDataClass_t, fut_itbl_p[]);


/* TpReadHdr reads the header information from an open binary fut file into
 * futio header structure.  This information describes shared and identity
 * (ramp) tables and indicates the content of the remaining part of the file,
 * but does not set up the input, output, or grid tables.
 * It returns a pointer to the loaded fut header.
 */

PTErr_t
	TpReadHdr (	KpFd_p			fd,
				KpHandle_t FAR*	PTHdr,
				PTType_p		formatP)
{
PTErr_t		errnum;
fut_hdr_p	futHdr;
KpInt32_t	ret, hdrSize;

	hdrSize = (KpInt32_t)sizeof (fut_hdr_t);

	/* get header memory */
	futHdr = (fut_hdr_p) allocBufferPtr (hdrSize);
	if (futHdr == NULL) {
		errnum = KCP_NO_CHECKIN_MEM;
		goto ErrOut;
	}

	ret = Kp_read (fd, (KpGenericPtr_t)&futHdr->magic, sizeof (KpInt32_t));
	if (ret != 1) {
		errnum = KCP_INVAL_PT_BLOCK;
		goto ErrOut;
	}

#if !defined KCP_ICC_ONLY
	if ((futHdr->magic == FUT_CIGAM) || (futHdr->magic == FUT_MAGIC) ) {
		ret = fut_read_futhdr (fd, futHdr);	/* read in the header */
	}
	else {
#endif
#if defined (KPLSBFIRST)						/* swap bytes if necessary */
		Kp_swab32 ((KpGenericPtr_t)&futHdr->magic, 1);
#endif

		switch (futHdr->magic) {
		case PTTYPE_MFT1:	/* 8 bit matrix fut */
		case PTTYPE_MFT2:	/* 16 bit matrix fut */
			ret = fut_readMFutHdr (fd, futHdr);	/* read the matrix fut header */
			futHdr->idstr_len = 0;
			break;
		
		case PTTYPE_MA2B:
		case PTTYPE_MB2A:
			ret = fut_readMabFutHdr (fd, futHdr);	/* read the matrix fut header */
			futHdr->idstr_len = 0;
			break;
		
		default:
			errnum = KCP_INVAL_PT_BLOCK;	/* unknown type */
			goto ErrOut;
		}
#if !defined KCP_ICC_ONLY
	}
#endif

	if (ret != 1) {
		errnum = KCP_INVAL_PT_BLOCK;
		goto ErrOut;
	}

	futHdr->srcFormat = futHdr->magic;	/* remember original format */
	*formatP = futHdr->magic;			/* return type of PT */

	*PTHdr = unlockBufferPtr((KpGenericPtr_t)futHdr);	/* return handle to header info */
	if (*PTHdr == NULL) {
		errnum = KCP_MEM_UNLOCK_ERR;
		goto ErrOut;
	}

	return KCP_SUCCESS;

ErrOut:
	if (futHdr != NULL) {
		freeBufferPtr ((KpGenericPtr_t)futHdr);
	}

	return (errnum);

}


/* verify that two header info blocks are the same
 */
PTErr_t
	TpCompareHdr (	KpHandle_t	PTHdr1,
					KpHandle_t	PTHdr2)
{
PTErr_t errnum = KCP_SUCCESS;
KpInt32_t i1, i2;
chan_hdr_p chanf, chani;
fut_hdr_p futHdr1, futHdr2;

	if (PTHdr1 == PTHdr2) {
		return (KCP_SUCCESS);
	}

	futHdr1 = (fut_hdr_p)lockBuffer (PTHdr1);	/* get header pointer 1 */
	if (futHdr1 == NULL) {
		errnum = KCP_MEM_LOCK_ERR;
		goto GetOut;
	}

	futHdr2 = (fut_hdr_p)lockBuffer (PTHdr2);	/* get header pointer 2 */
	if (futHdr2 == NULL) {
		errnum = KCP_MEM_LOCK_ERR;
		goto GetOut;
	}

	if (	(futHdr1->magic != futHdr2->magic)
		||	(futHdr1->version != futHdr2->version)
	   	||	(futHdr1->order != futHdr2->order)) {
		errnum = KCP_INCON_PT;						/* no match */
		goto GetOut;
	}

	switch (futHdr1->magic) {
	case FUT_CIGAM: /* fut with bytes reversed */
	case FUT_MAGIC: /* fut with bytes in correct order */

		for (i1 = 0; i1 < FUT_NCHAN; i1++) {
			if (futHdr1->icode[i1] != futHdr2->icode[i1]) {
				errnum = KCP_INCON_PT;					/* no match */
				goto GetOut;
			}
		}

		for (i1=0, chanf=futHdr1->chan, chani=futHdr2->chan; i1<FUT_NCHAN; ++i1, chanf++, chani++) {
			if (chanf->gcode != chani->gcode) {
				errnum = KCP_INCON_PT;					/* no match */
				goto GetOut;
			}

			if (chanf->gcode != 0) {		/* if the channel is defined */
				for (i2 = 0; i2 < FUT_NCHAN; i2++) {
					if (chanf->icode[i2] != chani->icode[i2]) {
						errnum = KCP_INCON_PT;			/* no match */
						goto GetOut;
					}

				/* if this input is defined */
					if (chanf->icode[i2] != FUTIO_NULL) {
						if (chanf->size[i2] != chani->size[i2]) {
							errnum = KCP_INCON_PT;		/* no match */
							goto GetOut;
						}
					}
				}

				if (chanf->ocode != chani->ocode) {
					errnum = KCP_INCON_PT;				/* no match */
					goto GetOut;
				}
			}
		}

		if (futHdr1->more != futHdr2->more) {
			errnum = KCP_INCON_PT;						/* no match */
			goto GetOut;
		}
		break;

	case PTTYPE_MFT1:	/* 8 bit matrix fut */
	case PTTYPE_MFT2:	/* 16 bit matrix fut */
	case PTTYPE_MA2B:
	case PTTYPE_MB2A:
		for (i1 = 0; i1 < 2; i1++) {
			if (futHdr1->icode[i1] != futHdr2->icode[i1]) {
				errnum = KCP_INCON_PT;						/* no match */
				goto GetOut;
			}
		}
		break;
		
	default:
		return (KCP_INVAL_PT_BLOCK);		/* unknown type */
	}


GetOut:
	if ( ! unlockBuffer (PTHdr1)) {
		errnum = KCP_MEM_UNLOCK_ERR;
	}
	if ( ! unlockBuffer (PTHdr2)) {
		errnum = KCP_MEM_UNLOCK_ERR;
	}

	return errnum;
}


/* initialize a PT for export
 * lock and return pointers to the header and fut
 * make the tables for the specified export format
 */
PTErr_t
	initExport (	KpHandle_t		PTHdr,
					KpHandle_t		PTData,
					PTType_t		format,
					fut_hdr_p FAR*	futHdrP,
					fut_p FAR*		futP)
{
PTErr_t		errnum = KCP_SUCCESS;
fut_p		fut;
fut_hdr_p	futHdr;
KpInt32_t	status;

/* get fut pointer */
	fut = fut_lock_fut ((KpHandle_t)PTData);
	if ( ! IS_FUT(fut)) {
		errnum = KCP_PTERR_2;
		goto GetOut;
	}

/* get header pointer of checked in PT */
	futHdr = (fut_hdr_p) lockBuffer (PTHdr);
	if (futHdr == NULL) {
		errnum = KCP_MEM_LOCK_ERR;
		goto GetOut;
	}

	if (format == PTTYPE_FUTF) {	/* make fixed tables */
		status = makeFutTblDat (fut);
		if (status != 1) {
			fut_free_tbldat	(fut);
			errnum = KCP_INCON_PT;
		}
	}
	else {
		status = makeMftTblDat (fut);
		if (status != 1) {
			fut_free_mftdat	(fut);
			errnum = KCP_INCON_PT;
		}
	}
	

GetOut:
	if (errnum == KCP_SUCCESS) {
		*futP = fut;		/* return pointers to caller */
		*futHdrP = futHdr;
	}
	else {
		(void) unlockPT (PTHdr, fut);
		*futP = NULL;
		*futHdrP = NULL;
	}

	return errnum;
}


/* unlock header and data of a PT
 */
PTErr_t
	unlockPT (	KpHandle_t	PTHdr,
				fut_p		fut)
{
PTErr_t errnum = KCP_SUCCESS;

	if (fut_unlock_fut (fut) == NULL) {
		errnum = KCP_PTERR_1;
	}
	else {
		if ( ! unlockBuffer (PTHdr)) {
			errnum = KCP_MEM_UNLOCK_ERR;
		}
	}

	return errnum;
}


/* TpFreeHdr releases the header of a checked in fut.
 */
PTErr_t
	TpFreeHdr (	KpHandle_t	PTHdr)
{
	freeBuffer (PTHdr);		/* free the header info structure */

	return (KCP_SUCCESS);
}


/* TpReadData reads a fut from a memory block and returns a handle to a newly allocated fut
 */
PTErr_t
	TpReadData(	KpFd_p			fd,
				PTType_t		format,
				PTRefNum_t		PTRefNum,
				KpHandle_t		PTHdr,
				KpHandle_t FAR*	PTData)
{
PTErr_t			errnum;
fut_p			fut = NULL, theFutFromMatrix = NULL, newFut = NULL, lab2xyzFut = NULL, finalFut = NULL;
fut_hdr_p		futHdr;
Fixed_t			matrix[MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM];
KpInt32_t		ret, iomask;
KpChar_t		ENUM_String[20];
KpInt32_t		inCS, i, i1;
ResponseRecord_t	inRedTRC, inGreenTRC, inBlueTRC;
ResponseRecord_t	outRedTRC, outGreenTRC, outBlueTRC;
PTRefNum_t		matrixPTRefNum;
PTDataClass_t 	iClass, oClass;

	futHdr = (fut_hdr_p) lockBuffer (PTHdr);	/* get buffer pointer */
	if (futHdr == NULL) {
		errnum = KCP_MEM_LOCK_ERR;
		goto GetOut;
	}

	futHdr->profileType = getIntAttrDef (PTRefNum, KCM_ICC_PROFILE_TYPE);
	futHdr->spaceIn = getIntAttrDef (PTRefNum, KCM_SPACE_IN);
	futHdr->spaceOut = getIntAttrDef (PTRefNum, KCM_SPACE_OUT);
	futHdr->iDataClass = getDataClass (futHdr->spaceIn);
	futHdr->oDataClass = getDataClass (futHdr->spaceOut);

	switch (format) {
	case FUT_CIGAM: /* fut with bytes reversed */
	case FUT_MAGIC: /* fut with bytes in correct order */
		if ((fut = fut_alloc_fut ()) == NULL) {	/* allocate a new fut structure */
			errnum = KCP_NO_ACTIVATE_MEM;
		}
		else {
			if (fut_read_tbls (fd, fut, futHdr) != 1) {	/* read fut tables */
				errnum = KCP_PT_DATA_READ_ERR;
			}
			else {
				if (fut_io_decode (fut, futHdr) == 0) {
					errnum = KCP_PTERR_0;
				}
				else {
					errnum = KCP_SUCCESS;
				}
			}
		}
		break;

	case PTTYPE_MFT1:
	case PTTYPE_MFT2:
		fut = fut_readMFutTbls (fd, futHdr, matrix);	/* read matrix fut tables */
		if (fut == NULL) {
			errnum = KCP_NO_ACTIVATE_MEM;
		}
		else {
			inCS = getIntAttrDef (PTRefNum, KCM_SPACE_IN);

			if ((inCS == KCM_CIE_XYZ) && (isIdentityMatrix (matrix, MF_MATRIX_DIM) != 1)) {
				ret = makeOutputMatrixXform ((Fixed_p)&matrix, 8, &theFutFromMatrix);
				if (ret != 1) {
					errnum = KCP_INCON_PT;
					goto GetOut;
				}
				else {
					iomask = FUT_PASS(FUT_XYZ);		/* get the Lab to XYZ fut */
					lab2xyzFut = get_lab2xyz (KCP_GRID_DIM_SIXTEEN);

					newFut = fut_comp (theFutFromMatrix, lab2xyzFut, iomask);			

					if (newFut != NULL) {
						finalFut = fut_comp (fut, newFut, iomask);
					}			

					fut_free (theFutFromMatrix);	/* free intermediate futs */
					fut_free (lab2xyzFut);
					fut_free (fut);
					fut_free (newFut);

					fut = finalFut;

					/* set the input color space attribute to Lab */
					KpItoa (KCM_CIE_LAB, ENUM_String);
					errnum = PTSetAttribute (PTRefNum, KCM_SPACE_IN, ENUM_String);
					if (errnum != KCP_SUCCESS) {
						goto GetOut;
					}

					/* set the input composition attribute to Lab */
					errnum = PTSetAttribute (PTRefNum, KCM_IN_CHAIN_CLASS_2, "6");
					if (errnum != KCP_SUCCESS) {
						goto GetOut;
					}
				}
			}

			if ((fut == NULL) || !fut_io_encode (fut, futHdr)) {	/* make the info header */
				errnum = KCP_INCON_PT;
				goto GetOut;
			}

			errnum = KCP_SUCCESS;
		}
		break;

	case PTTYPE_MA2B:
	case PTTYPE_MB2A:
	
		matrix[0] = matrix[4] = matrix[8] = KpF15d16FromDouble(1.0);
		matrix[1] = matrix[2] = matrix[3] = 
		matrix[5] = matrix[6] = matrix[7] = 
		matrix[9] = matrix[10] = matrix[11] = KpF15d16FromDouble(0.0);
		
		fut = fut_readMabFutTbls (fd, futHdr, matrix);	/* read matrix fut tables */
		if (fut == NULL) {
			errnum = KCP_NO_ACTIVATE_MEM;
		}
		else {
			if (fut->lutConfig & HAS_MATRIX_DATA) {
				i = MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM;
				for (i1 = 0; i1 < i; i1++)
				{
					fut->matrix[i1] = matrix[i1];
				}
				switch (fut->lutConfig) {
				case MAB_M_MATRIX_B_COMBO:
				case MBA_B_MATRIX_M_COMBO:
					inRedTRC.CurveCount = fut->mabInTblEntries[0];
					inGreenTRC.CurveCount = fut->mabInTblEntries[1];
					inBlueTRC.CurveCount = fut->mabInTblEntries[2];
					inRedTRC.CurveData = fut->mabInRefTbl[0];
					inGreenTRC.CurveData = fut->mabInRefTbl[1];
					inBlueTRC.CurveData = fut->mabInRefTbl[2];
					outRedTRC.CurveCount = fut->mabOutTblEntries[0];
					outGreenTRC.CurveCount = fut->mabOutTblEntries[1];
					outBlueTRC.CurveCount = fut->mabOutTblEntries[2];
					outRedTRC.CurveData = fut->mabOutRefTbl[0];
					outGreenTRC.CurveData = fut->mabOutRefTbl[1];
					outBlueTRC.CurveData = fut->mabOutRefTbl[2];
					iClass = getDataClass(futHdr->spaceIn);
					oClass = getDataClass(futHdr->spaceOut);
					ret = makeFutFromMatrix ((Fixed_p)&matrix, &inRedTRC, &inGreenTRC, &inBlueTRC, 
												&outRedTRC, &outGreenTRC, &outBlueTRC, MATRIX_GRID_SIZE, iClass, oClass, 
												(fut_p *)&theFutFromMatrix);
					break;

				case MBA_B_MATRIX_M_CLUT_A_COMBO:
					inRedTRC.CurveCount = fut->mabInTblEntries[0];
					inGreenTRC.CurveCount = fut->mabInTblEntries[1];
					inBlueTRC.CurveCount = fut->mabInTblEntries[2];
					inRedTRC.CurveData = fut->mabInRefTbl[0];
					inGreenTRC.CurveData = fut->mabInRefTbl[1];
					inBlueTRC.CurveData = fut->mabInRefTbl[2];
					iClass = getDataClass(futHdr->spaceIn);
					oClass = KCP_UNKNOWN;
					ret = makeFutFromMatrix ((Fixed_p)&matrix, &inRedTRC, &inGreenTRC, &inBlueTRC, 
												NULL, NULL, NULL, MATRIX_GRID_SIZE, iClass, oClass, (fut_p *)&theFutFromMatrix);
					break;

				case MAB_A_CLUT_M_MATRIX_B_COMBO:
					outRedTRC.CurveCount = fut->mabOutTblEntries[0];
					outGreenTRC.CurveCount = fut->mabOutTblEntries[1];
					outBlueTRC.CurveCount = fut->mabOutTblEntries[2];
					outRedTRC.CurveData = fut->mabOutRefTbl[0];
					outGreenTRC.CurveData = fut->mabOutRefTbl[1];
					outBlueTRC.CurveData = fut->mabOutRefTbl[2];
					iClass = KCP_UNKNOWN;
					oClass = getDataClass(futHdr->spaceOut);
					ret = makeFutFromMatrix ((Fixed_p)&matrix, NULL, NULL, NULL, &outRedTRC, &outGreenTRC,
												&outBlueTRC, MATRIX_GRID_SIZE, iClass, oClass, (fut_p *)&theFutFromMatrix);
					break;

				default:
					break;
				}
				if (NULL != theFutFromMatrix)
				{
					/* Create a PT from the fut */
					errnum = fut2PT (&theFutFromMatrix, KCM_UNKNOWN, KCM_UNKNOWN, PTTYPE_CALCULATED, &matrixPTRefNum);
					if (errnum != KCP_SUCCESS) {
						goto GetOut;
					}
					errnum = setMatrixPTRefNum (PTRefNum, matrixPTRefNum, fut->lutConfig);
					if (errnum != KCP_SUCCESS) {
						goto GetOut;
					}
				}
				if (ret != 1) {
					errnum = KCP_INCON_PT;
					goto GetOut;
				}
			}

			if ((fut == NULL) || !fut_io_encode (fut, futHdr)) {	/* make the info header */
				errnum = KCP_INCON_PT;
				goto GetOut;
			}

			errnum = KCP_SUCCESS;
		}
		break;

	default:
		break;
	}


GetOut:
	if ((errnum != KCP_SUCCESS) || (fut == NULL)) {
		fut_free (fut);
	}
	else {		/* return handle to fut to caller */

	/* make sure the futs are in the reference state */
		if (fut_to_mft (fut) == 1) {
			*PTData = (KpHandle_t)fut_unlock_fut (fut);
		}
	}

	if ( ! unlockBuffer (PTHdr)) {
		errnum = KCP_MEM_UNLOCK_ERR;
	}

	return errnum;
}


/* TpFreeData frees all of the memory allocated for a fut.
 */
PTErr_t
	TpFreeData (	KpHandle_t	PTData)
{
PTErr_t errnum = KCP_PTERR_2;
fut_p fut;

	fut = fut_lock_fut ((KpHandle_t)PTData);
	if (IS_FUT(fut)) {
		fut_free (fut);
		errnum = KCP_SUCCESS;
	}

	return errnum;
}


/* TpWriteHdr writes the header of a fut to an external memory block.
 */
PTErr_t
	TpWriteHdr(	KpFd_p			fd,
				PTType_t	format,
				KpHandle_t	PTHdr,
				KpInt32_t		attrSize)
{
PTErr_t errnum = KCP_SUCCESS;
fut_hdr_p futHdr;

	switch (format) {
	case PTTYPE_FUTF:	/* get buffer pointer */
		futHdr = (fut_hdr_p)lockBuffer (PTHdr);
		if (futHdr == NULL) {
			errnum = KCP_MEM_LOCK_ERR;
			return errnum;
		}

		futHdr->idstr_len = attrSize;		/* insert size of attributes */
	
		if (fut_write_hdr (fd, futHdr) == 0) {	/* and write out the header */
			errnum = KCP_PT_HDR_WRITE_ERR;
		}

		if ( ! unlockBuffer (PTHdr)) {
			errnum = KCP_MEM_UNLOCK_ERR;
		}

		break;

	case PTTYPE_MFT1:
	case PTTYPE_MFT2:
	case PTTYPE_MFT2_VER_0:
		errnum = KCP_SUCCESS;
		break;

	case PTTYPE_MAB1:
	case PTTYPE_MAB2:
	case PTTYPE_MBA1:
	case PTTYPE_MBA2:
		errnum = KCP_SUCCESS;
		break;

	default:
		errnum = KCP_INVAL_PTTYPE;
		break;
	}
	
	return errnum;
}


/* TpWriteData writes the table data of a fut to an external memory block.
 */
PTErr_t
	TpWriteData(	KpFd_p		fd,
					PTType_t	format,
					KpHandle_t	PTHdr,
					KpHandle_t	PTData)
{
PTErr_t		errnum,  errnum1;
fut_hdr_p	futHdr;
fut_p		fut;

	errnum = initExport (PTHdr, PTData, format, &futHdr, &fut);	/* set up to export the data */
	if (errnum != KCP_SUCCESS) {
		return errnum;
	}

	/* write out the tables */
	switch (format) {
	case PTTYPE_FUTF:
		if (fut_write_tbls (fd, fut, futHdr) == 0) {
			errnum = KCP_PT_DATA_WRITE_ERR;
		}
		
		fut_free_tbldat (fut);	/* free the made data tables */
		break;

	case PTTYPE_MFT1:
	case PTTYPE_MFT2:
	case PTTYPE_MFT2_VER_0:
		if (fut_writeMFut_Kp (fd, fut, NULL, format) != 1) {
			errnum = KCP_PT_DATA_WRITE_ERR;
		}
		
		fut_free_mftdat (fut);	/* free the made data tables */
		break;

	case PTTYPE_MAB1:
	case PTTYPE_MAB2:
	case PTTYPE_MBA1:
	case PTTYPE_MBA2:
		if (fut_writeMabFut_Kp (fd, fut, futHdr, format) != 1) {
			errnum = KCP_PT_DATA_WRITE_ERR;
		}
		
		fut_free_mftdat (fut);	/* free the made data tables */
		break;

	default:
		errnum = KCP_INVAL_PTTYPE;
		break;
	}

	errnum1 = unlockPT (PTHdr, fut);

	if (errnum != KCP_SUCCESS) {
		return (errnum);
	} else {
		return (errnum1);
	}
}


/* TpGetDataSize returns the size in bytes of fut
 */

KpInt32_t
	TpGetDataSize (	KpHandle_t	PTHdr,
					KpHandle_t	PTData,
					PTType_t	format)
{
KpInt32_t	size, futRet, imask, omask, LUTDimensions, inputChans, outputChans;
KpInt32_t	i, tableSize, oTableEntries, gTableEntries, iTableEntries, nParaParams;
PTErr_t		errnum = KCP_INVAL_PTTYPE;
fut_hdr_p	futHdr;
fut_p		fut;
KpUInt32_t	lutConfig;

	size = 0;

	errnum = initExport (PTHdr, PTData, format, &futHdr, &fut);		/* set up to export the data */
	if (errnum == KCP_SUCCESS) {
		switch (format) {
		case PTTYPE_FUTF:
			size = fut_get_size (fut, futHdr);
		
			fut_free_tbldat (fut);	/* free the made data tables */
			break;

		case PTTYPE_MFT1:
		case PTTYPE_MFT2:
		case PTTYPE_MFT2_VER_0:
			futRet = fut_mfutInfo (fut, &LUTDimensions, &inputChans, &outputChans, format,
									&iTableEntries, &gTableEntries, &oTableEntries);

			if (futRet == 1) {
				size = inputChans * iTableEntries;	/* total input table entries */

				size += outputChans * (gTableEntries + oTableEntries);	/* plus total grid and output table entries */

				if (format == PTTYPE_MFT1) {
					size *= sizeof (mf1_tbldat_t);	/* mult by bytes in each entry */
				}
				else {
					size += 2;						/* plus input and output table counters */

					size *= sizeof (mf2_tbldat_t);	/* mult by bytes in each entry */
				}
			}
		
			fut_free_mftdat (fut);	/* free the made data tables */
			break;

		case PTTYPE_MAB1:
		case PTTYPE_MAB2:
		case PTTYPE_MBA1:
		case PTTYPE_MBA2:
		
			lutConfig = fut->lutConfig;
			/* input tables must be common and in first n contiguous input channels */
			imask = fut->iomask.in;							/* get the fut's input mask */
			for (inputChans = 0; inputChans < FUT_NICHAN; inputChans++, imask >>= 1) {
				if ( ! IS_ITBL(fut->itbl[inputChans]) || ((imask & 1) == 0)) {
					break;
				}
			}
			if (imask != 0) {
				return (0);		/* this fut can not be made into a matrix fut */
			}
			/* output tables must be in first n contiguous output channels */
			omask = fut->iomask.out;				/* get the fut's output mask */
			for (outputChans = 0; outputChans < FUT_NOCHAN; outputChans++, omask >>= 1) {
			if ( ! IS_CHAN(fut->chan[outputChans]) || ((omask & 1) == 0)) {
					break;
				}
			}
			if (omask != 0) {
				return (0);		/* this fut can not be made into a matrix fut */
			}
			tableSize = sizeof (mab_tbldat_t);

			if ((LUT_TYPE_UNKNOWN == lutConfig) || (MAB_B_CURVE_ONLY == lutConfig) ||
			(MAB_A_CLUT_B_COMBO == lutConfig) || (MBA_B_CLUT_A_COMBO == lutConfig) ||
			(MAB_A_CLUT_M_MATRIX_B_COMBO == lutConfig) || (MBA_B_MATRIX_M_CLUT_A_COMBO == lutConfig))
			{
				for (i = 0; i < inputChans; i++)
				{
					if (PARA_TYPE_SIG == fut->itbl[i]->ParaCurve.nSig)
					{
						nParaParams = getNumParaParams(fut->itbl[i]->ParaCurve.nFunction);
						size += nParaParams * sizeof (Fixed_t) + CURVETYPE_HEADER;
					} else {
						iTableEntries = fut->itbl[i]->refTblEntries;
						size += (iTableEntries * tableSize) + CURVETYPE_HEADER;	/* total input table entries */
					}
					size = (size + 3) & ~3;
				}
			}
			
			if ((LUT_TYPE_UNKNOWN == lutConfig) || (MBA_B_CLUT_A_COMBO == lutConfig) ||
			(MBA_B_MATRIX_M_CLUT_A_COMBO == lutConfig) || (MAB_A_CLUT_B_COMBO == lutConfig) ||
			(MAB_A_CLUT_M_MATRIX_B_COMBO == lutConfig))
			{
				gTableEntries = 0;
				for (i = 0; i < outputChans; i++)
				{
					gTableEntries += (fut->chan[0]->gtbl->tbl_size / sizeof (fut_gtbldat_t)); /* assume 8 bit grid tables */
				}
				if ((PTTYPE_MAB2 == format) || (PTTYPE_MBA2 == format))
				{
					gTableEntries *= 2;		/* 16 bit grid tables */
				}
				size += gTableEntries;
				size += CLUT_HEADER;
				size = (size + 3) & ~3;
			}

			if ((LUT_TYPE_UNKNOWN == lutConfig) || (MBA_B_CURVE_ONLY == lutConfig) ||
			(MAB_A_CLUT_B_COMBO == lutConfig) || (MBA_B_CLUT_A_COMBO == lutConfig) ||
			(MAB_A_CLUT_M_MATRIX_B_COMBO == lutConfig) || (MBA_B_MATRIX_M_CLUT_A_COMBO == lutConfig))
			{
				for (i = 0; i < outputChans; i++)
				{
					if (PARA_TYPE_SIG == fut->chan[i]->otbl->ParaCurve.nSig)
					{
						nParaParams = getNumParaParams(fut->chan[i]->otbl->ParaCurve.nFunction);
						size += nParaParams * sizeof (Fixed_t) + CURVETYPE_HEADER;
					} else {
						oTableEntries = fut->chan[i]->otbl->refTblEntries;
						size += (oTableEntries * tableSize + CURVETYPE_HEADER);	/* plus total output table entries */
					}
					size = (size + 3) & ~3;
				}
			}
			if ((MBA_B_MATRIX_M_CLUT_A_COMBO == lutConfig) ||
				(MBA_B_MATRIX_M_COMBO == lutConfig) || (MAB_M_MATRIX_B_COMBO == lutConfig))
			{
				for (i = 0; i < FUT_NMCHAN; i++)
				{
					if (PARA_TYPE_SIG == fut->mabInParaCurve[i].nSig)
					{
						nParaParams = getNumParaParams(fut->mabInParaCurve[i].nFunction);
						size += nParaParams * sizeof (Fixed_t) + CURVETYPE_HEADER;
					} else {
						iTableEntries = fut->mabInTblEntries[i];
						size += (iTableEntries * tableSize) + CURVETYPE_HEADER;	/* plus total matrix inputput table entries */
					}
					size = (size + 3) & ~3;
				}
			}
			if ((MAB_A_CLUT_M_MATRIX_B_COMBO == lutConfig) ||
				(MBA_B_MATRIX_M_COMBO == lutConfig) || (MAB_M_MATRIX_B_COMBO == lutConfig))
			{
				for (i = 0; i < FUT_NMCHAN; i++)
				{
					if (PARA_TYPE_SIG == fut->mabOutParaCurve[i].nSig)
					{
						nParaParams = getNumParaParams(fut->mabOutParaCurve[i].nFunction);
						size += nParaParams * sizeof (Fixed_t) + CURVETYPE_HEADER;
					} else {
						oTableEntries = fut->mabOutTblEntries[i];
						size += (oTableEntries * tableSize) + CURVETYPE_HEADER;	/* plus total matrix outputput table entries */
					}
					size = (size + 3) & ~3;
				}
			}

			fut_free_mftdat (fut);	/* free the made data tables */
			break;

		default:
			break;
		}

	 	errnum = unlockPT (PTHdr, fut);
		if (errnum != KCP_SUCCESS) {
			size = 0;
		}
	}

	return (size);
}


#if !defined KCMS_NO_CRC
/* calculate the signed 32-bit fut cyclical redundancy check (CRC)
 */
 
PTErr_t
	TpCalCrc (	KpHandle_t		PTHdr,
				KpHandle_t		PTData,
				KpInt32_t FAR*	crc32)
{
PTErr_t		errnum;
fut_hdr_p	futHdr;
fut_p		fut;

	errnum = initExport (PTHdr, PTData, PTTYPE_FUTF, &futHdr, &fut);	/* set up to export the data */
	if (errnum != KCP_SUCCESS) {
		return errnum;
	}

	fut_cal_crc (fut, crc32);	/* calculate the crc */

		
	fut_free_tbldat (fut);	/* free the made data tables */

	errnum = unlockPT (PTHdr, fut);
	if (errnum != KCP_SUCCESS) {
		return errnum;
	}

	return errnum;
}
#endif


/* fut_get_size returns the size in bytes of a fut
 */

KpInt32_t
	fut_get_size (	fut_p		fut,
					fut_hdr_p	futHdr)
{
KpInt32_t	size = 0, i1;

	/* add up size of the input tables */
	for ( i1=0; i1<FUT_NICHAN; i1++ ) {
		if ( futHdr->icode[i1] == FUTIO_UNIQUE )
			size += fut_size_itbl (fut->itbl[i1]);
	}

	/* add up size of the output channels */
	for ( i1=0; i1<FUT_NOCHAN; i1++ ) {
		if ( fut->chan[i1] != 0 )
			size += fut_size_chan (fut->chan[i1], & futHdr->chan[i1]);
	}
	
	return size;
}


/* fut_size_chan returns the size in bytes of a channel
 */

static KpInt32_t
	fut_size_chan (	fut_chan_p	chan,
					chan_hdr_p	chanio)
{
KpInt32_t size = 0;
KpInt32_t	i1;

	if ( ! IS_CHAN (chan) )
		return (0);

/* add up size of the input tables */
	for ( i1=0; i1<FUT_NICHAN; i1++ ) {
		if ( chanio->icode[i1] == FUTIO_UNIQUE )
			size += fut_size_itbl (chan->itbl[i1]);
	}

/* add up size of the output table */
	if ( chanio->ocode == FUTIO_UNIQUE ) {
		size += fut_size_otbl (chan->otbl);
	}

/* add up size of the grid table */
	if ( chanio->gcode == FUTIO_UNIQUE ) {
		size += fut_size_gtbl (chan->gtbl);
	}

	return size;
}


/* fut_size_itbl returns the size in bytes of an input table */
static KpInt32_t
	fut_size_itbl (	fut_itbl_p	itbl)
{
	if ( ! IS_ITBL (itbl)) return (0);

	return ((sizeof (KpInt32_t)*4) + (sizeof (fut_itbldat_t) * (FUT_INPTBL_ENT+1)));
}


/* fut_size_otbl returns the size in bytes of an output table */
static KpInt32_t
	fut_size_otbl (	fut_otbl_p	otbl)	
{
	if ( ! IS_OTBL (otbl)) return (0);

	return ((sizeof (KpInt32_t)*3) + (sizeof (fut_otbldat_t)*FUT_OUTTBL_ENT));
}


/* fut_size_gtbl returns the size in bytes of a grid table */
static KpInt32_t
	fut_size_gtbl (	fut_gtbl_p	gtbl)
{
	if (( ! IS_GTBL (gtbl)) || (gtbl->tbl == NULL)) return (0);

	return (((KpInt32_t)sizeof (KpInt32_t)*5) + ((KpInt32_t)sizeof (KpInt16_t)*FUT_NCHAN) + gtbl->tbl_size);
}


PTErr_t
	TpSetImplicitAttr (	PTRefNum_t PTRefNum)
{
KpChar_t	attribute[256];
PTErr_t		errnum;
fut_hdr_p	futHdr;
chan_hdr_p	chan;
KpInt32_t	i1, i2, attributeTag, numOutVar, numInVar[FUT_NCHAN];
KpHandle_t	PTHdr;

	errnum = PTSetAttribute (PTRefNum, KCM_TECH_TYPE, KCM_FUT_S);

	PTHdr = getPTHdr (PTRefNum);			/* get the header info */
	futHdr = (fut_hdr_p)lockBuffer (PTHdr);
	if (futHdr != NULL) {

		/* set the technology version attribute */
		KpItoa (futHdr->version, attribute);
		errnum = PTSetAttribute (PTRefNum, KCM_TECH_VERSION, attribute);

		/* get the number of active output channels */
		/* and the number of active inputs for each channel */

		for (i1 = 0; i1 < FUT_NCHAN; i1++) {
			numInVar[i1] = 0;					/* zero out the number of inputs array */
		}

		numOutVar = 0;							/* and # of outputs */

		switch (futHdr->magic) {
		case PTTYPE_MFT1:
		case PTTYPE_MFT2:
		case PTTYPE_MB2A:
		case PTTYPE_MA2B:
			numOutVar = futHdr->icode[1];		/* copy from header */ 
			for (i1 = 0; i1 < numOutVar; i1++) {
				numInVar[i1] = futHdr->icode[0];
			}

			break;

		default:	/* PTTYPE_FUTF */
			for (i1 = 0, chan = futHdr->chan; i1 < FUT_NCHAN; i1++, chan++) {
				if ((chan->gcode & FUTIO_CODE) != FUTIO_NULL) {
					numOutVar++;
					for (i2 = 0; i2 < FUT_NCHAN; i2++) {
						if ((chan->icode[i2] & FUTIO_CODE) != FUTIO_NULL) {
							(numInVar[i1])++;
						}
					}
				}
			}
			break;
		}

	/* set the attribute for number of inputs for each channnel  */
		for (i1 = 0, attributeTag = KCM_NUM_IN_VAR_1; i1<FUT_NCHAN; i1++) {
			if (numInVar[i1] != 0) {
				KpItoa (numInVar[i1], attribute);
				PTSetAttribute (PTRefNum, attributeTag, attribute);
				attributeTag++;
			}
		}

	/* set the number of output channnels attribute */
		KpItoa (numOutVar, attribute);
		errnum = PTSetAttribute (PTRefNum, KCM_NUM_OUT_VAR, attribute);

		if ( ! unlockBuffer (PTHdr)) {
			errnum = KCP_MEM_UNLOCK_ERR;
		}
	}

	return errnum;
}


PTErr_t
	initAttrib (PTRefNum_t PTRefNum)
{
PTErr_t		errnum = KCP_SUCCESS;
KpInt32_t	attrSize, year;
KpChar_t	attrBuf[KCM_MAX_ATTRIB_VALUE_LENGTH+1], yearStr[10];
kpTm_t  	currentTime;
	
	/* If the copyright string isn't there, then generate it */
	attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;
	errnum = PTGetAttribute (PTRefNum, KCM_COPYRIGHT, &attrSize, attrBuf);
	if (errnum == KCP_INVAL_PTA_TAG) {
		KpGetLocalTime(&currentTime);		/* get the current time, which has the current year */
		year = currentTime.year + 1900;		/* add 1900 to get the actual year */											
		KpItoa (year, yearStr);
		
		strcpy (attrBuf, FUT_COPYRIGHT_PREFIX);	/* make the copyright string. */
		strcat (attrBuf, yearStr);
		strcat (attrBuf, FUT_COPYRIGHT_SUFFIX);

		errnum = PTSetAttribute (PTRefNum, KCM_COPYRIGHT, attrBuf);
	}

	return errnum;
}


/* get the input or output data class of a PT */
static PTDataClass_t
	getPTDataClass (PTRefNum_t	PTRefNum,
					KpInt32_t	attrTag)
{
KpInt32_t		colorSpace;
PTDataClass_t	dataClass;

	colorSpace = getIntAttrDef (PTRefNum, attrTag);
	if (colorSpace == KCM_UNKNOWN) {
		dataClass = KCP_UNKNOWN;
	}
	else {
		dataClass = getDataClass (colorSpace);
	}

	return dataClass;
}



/* convert data class attribute to enum */
static PTDataClass_t
	getDataClass (	KpInt32_t	colorSpace)
{
PTDataClass_t	dataClass;
	
	switch (colorSpace) {
	case KCM_RCS:
		dataClass = KCP_RCS_DATA;
		break;
	
	case KCM_CIE_LUV:
		dataClass = KCP_LUV_DATA;
		break;
	
	case KCM_CIE_LAB:
		dataClass = KCP_LAB_PCS;
		break;
	
	case KCM_CIE_XYZ:
		dataClass = KCP_XYZ_PCS;
		break;
	
	default:
		dataClass = KCP_FIXED_RANGE;
		break;
	}

	return dataClass;
}


/* check the input and output data class of a PT
 * if a color space is known and the data class is not known,
 * set the data class to correspond to the color space
 */
void
	checkDataClass (PTRefNum_t	PTRefNum)
{
KpInt32_t		i1;
KpHandle_t		PTData;
fut_p			fut;
fut_chan_p		chan;
fut_otbl_p		otbl;
PTDataClass_t	iDataClass, oDataClass;

	iDataClass = getPTDataClass (PTRefNum, KCM_IN_SPACE);
	oDataClass = getPTDataClass (PTRefNum, KCM_OUT_SPACE);

	PTData = getPTData (PTRefNum);
	fut = fut_lock_fut (PTData);
	if ( ! IS_FUT(fut)) return;		/* bummer */

	checkInDataClass (iDataClass, fut->itbl);	/* check the data class of each shared input table */

	for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
		chan = fut->chan[i1];

		if (IS_CHAN(chan)) {
			checkInDataClass (iDataClass, chan->itbl);	/* check the data class of each input table */

			if (oDataClass != KCP_UNKNOWN) {	/* check the data class of each output table */
				otbl = chan->otbl;
				if ((IS_OTBL(otbl)) && (otbl->dataClass == KCP_UNKNOWN)) {
					otbl->dataClass = oDataClass;
				}
			}
		}
	}
	
	fut_unlock_fut (fut);
}


static void
	checkInDataClass (	PTDataClass_t	dataClass,
						fut_itbl_p		itbls[])
{
KpInt32_t	i1;
fut_itbl_p	itbl;

	if (dataClass != KCP_UNKNOWN) {
		for (i1 = 0; i1 < FUT_NICHAN; i1++) {
			itbl = itbls[i1];
			if ((IS_ITBL(itbl)) && (itbl->dataClass == KCP_UNKNOWN)) {
				itbl->dataClass = dataClass;
			}
		}
	}
	
	return;
}
