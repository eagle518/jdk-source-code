/*
 * @(#)ptiomem.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)ptiomem.c	2.60 99/03/05

	Contains:	functions to read and write binary PT "memory" files.

	The external database is responsible for loading and storing a block of memory
	which contains all of the PT information.  It needs to know only the location
	and size of this block.
	
	COPYRIGHT (c) 1992-1999 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <string.h>
#include <stdio.h>
#include <memory.h>

#include "fut_util.h"
#include "kcms_sys.h"
#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "attrib.h"
#include "kcptmgr.h"
#if defined (KPMAC)
#include "memloger.h"
#endif

/* prototypes */
static PTErr_t gridDimValid (PTType_t, PTRefNum_t, PTRefNum_p);

/* PTCheckIn reads the private data header and attributes of a PT in an
 * external memory block, then enters it into the checked in PT list.
 */

PTErr_t
	PTCheckIn (	PTRefNum_p	PTRefNumP,
				PTAddr_t	PTAddr)
{
PTErr_t		errnum;
KpFd_t		fd;
KpHandle_t	PTHdr = NULL, PTAttr = NULL;
#if !defined KCP_ICC_ONLY
fut_hdr_t	FAR *futp;
#endif
PTType_t	format;

/* Check for valid PTRefNumP */
	if (PTRefNumP == NULL) {
		errnum = KCP_BAD_PTR;
		goto ErrOut4;
	}

/* initialize memory file manager to read the PT */
	if (KpOpen (NULL, "m", &fd, NULL, (KpGenericPtr_t)PTAddr, PTCHECKINSIZE) != KCMS_IO_SUCCESS) {
		errnum = KCP_SYSERR_1;
		goto ErrOut4;
	}

/* read the header info */
	errnum = TpReadHdr (&fd, &PTHdr, &format);
	if (errnum != KCP_SUCCESS) {
		goto ErrOut1;
	}

#if !defined KCP_ICC_ONLY
	switch (format) {
	case PTTYPE_FUTF:				/* discard the attribute info */
		futp = lockBuffer (PTHdr);	/* get the attributes */
		errnum = readAttributes (&fd, futp->idstr_len, &PTAttr);
		unlockBuffer (PTHdr);
		if (errnum != KCP_SUCCESS) {
			goto ErrOut2;
		}

		if (PTMemTest () == 0) {		/* not enough memory to continue */
			errnum = KCP_NO_CHECKIN_MEM;
			goto ErrOut3;
		}

		break;

	default:
		break;
	}
#endif
	   	
	errnum = registerPT (PTHdr, PTAttr, PTRefNumP);	/* enter PT into list */
	   	
	if (errnum != KCP_SUCCESS) {
#if !defined KCP_ICC_ONLY
ErrOut3:
#endif
		(void) freeAttributes(PTAttr);	/* free the attributes */
		freeBuffer (PTAttr);

#if !defined KCP_ICC_ONLY
ErrOut2:
#endif
		(void) TpFreeHdr (PTHdr);		/* free the header */
	}

ErrOut1:
	(void) Kp_close (&fd);

ErrOut4:
	return (errnum);
}


/* PTActivate reads the PT data from an external memory block and
 * sets up the technology specific PT memory structures.  Before loading
 * the PT, check to make sure that it matches the "checked in" info.
 */
PTErr_t
	PTActivate(	PTRefNum_t	PTRefNum,
				KpInt32_t	mBlkSize,
				PTAddr_t	PTAddr)
{
PTErr_t		errnum;
KpFd_t		fd;
KpHandle_t	PTHdr, PTData, PTHdr2;
fut_hdr_p	futp;
#if !defined KCMS_NO_CRC
KpChar_t 	strCRCmade[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
KpChar_t 	strCRCfound[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
KpInt32_t	crcAttrSize, crc32;
#endif
KpInt32_t	attrSize;
PTType_t	format;

	errnum = getPTStatus (PTRefNum);
	if (errnum != KCP_PT_INACTIVE) {
		goto ErrOut1;
	}

	/* initialize memory file manager */
	if (KpOpen (NULL, "m", &fd, NULL, (KpGenericPtr_t)PTAddr, mBlkSize) != KCMS_IO_SUCCESS) {
		errnum = KCP_SYSERR_1;
		goto ErrOut1;
	}

/* read in the encoded header, verify that its the same as the original, then discard */
	errnum = TpReadHdr (&fd, &PTHdr2, &format);
	if (errnum != KCP_SUCCESS ) {
		goto ErrOut2;
	}

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nPTActivate\n PTRefNum %x, mBlkSize %d, PTAddr %x, format %x\n",
						PTRefNum, mBlkSize, PTAddr, format);
	kcpDiagLog (string);}
	#endif

/* get and save size of attributes */
    futp = lockBuffer (PTHdr2);
	attrSize = futp->idstr_len;
	unlockBuffer (PTHdr2);

	PTHdr = getPTHdr (PTRefNum);	/* get the original PT header */

/* make sure the PT header and checkin info match */
	errnum = TpCompareHdr (PTHdr, PTHdr2);

	(void) TpFreeHdr (PTHdr2);		/* free the header */

	if (errnum != KCP_SUCCESS) {	/* then check for an error in hdrVerify */
		goto ErrOut2;
	}

#if !defined KCP_ICC_ONLY
	switch (format) {
	case PTTYPE_FUTF:						/* discard the attribute info */
		if (Kp_skip (&fd, attrSize) != KCMS_IO_SUCCESS){ /* may have been setAttribute after checkin */
			errnum = KCP_PTERR_3;
			goto ErrOut2;
		}

		break;

	default:
		break;
	}
#endif

	errnum = TpReadData (&fd, format, PTRefNum, PTHdr, &PTData);		/* get the PT data */
	if (errnum == KCP_SUCCESS) {
		if (PTMemTest () == 0) {			/* enough memory to continue operations? */
			errnum = KCP_NO_ACTIVATE_MEM;
			goto ErrOut3;
		}

	#if !defined KCMS_NO_CRC
		errnum = TpCalCrc (PTHdr, PTData, &crc32);	/* calculate the CRC */
		if (errnum == KCP_SUCCESS) {
			KpItoa(crc32, strCRCmade);
			crcAttrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;
			errnum = PTGetAttribute(PTRefNum, KCM_CRC, &crcAttrSize, strCRCfound);

			if (errnum == KCP_INVAL_PTA_TAG) {		/* if not present, just set it */
				PTSetAttribute(PTRefNum, KCM_CRC, strCRCmade);
				errnum = KCP_SUCCESS;
			}
			else {
/*				if ((errnum == KCP_SUCCESS)*/			/* if present, must match */ 
/*					&& (strcmp (strCRCmade, strCRCfound) != 0)) { */
/*					errnum = KCP_INCON_PT; */
/*					goto ErrOut3; */
/*				} */
			}
		}
	#endif
	}

	if (errnum == KCP_SUCCESS) {	/* Everything's OK, now activate */
		makeActive (PTRefNum, PTData);
	}

ErrOut2:
	(void) Kp_close (&fd);
ErrOut1:
	return (errnum);


ErrOut3:
	(void) TpFreeData (PTData);	/* Release the PT memory */
	goto ErrOut2;
}


/* deactivate a checked in PT */
PTErr_t PTDeActivate (PTRefNum_t PTRefNum)
{
PTErr_t errnum;

	errnum = getPTStatus (PTRefNum);		/* must currently be active */

	if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_SERIAL_PT)) {
		errnum = makeInActive (PTRefNum, KPTRUE);	/* deactivate the PT */
		if (KCP_PT_TABLE_DELETED == errnum)
		{
			errnum = KCP_SUCCESS;
		}
	}

	return errnum;
}


/* check out a checked in PT. */
PTErr_t PTCheckOut (PTRefNum_t PTRefNum)
{
PTErr_t 	errnum;
PTRefNum_t	matrixPTRefNum;
KpUInt32_t	lutConfig;

	errnum = PTDeActivate (PTRefNum);				/* free the PT data */
	if ((errnum == KCP_SUCCESS) || (errnum == KCP_PT_INACTIVE)) {
		errnum = getMatrixPTRefNum (PTRefNum, &matrixPTRefNum, &lutConfig);
		if (KCP_SUCCESS == errnum)
		{
			errnum = makeCheckedOut (matrixPTRefNum);		/* release matrix PT table */
		}
		errnum = makeCheckedOut (PTRefNum);		/* release PT table */
	}

	return (errnum);
}


/* convert a fut into a checked in and activated PT */
/* frees source fut on error */
PTErr_t
	fut2PT (fut_p		*futSrc,
			KpInt32_t	inSpace,
			KpInt32_t	outSpace,
			KpInt32_t	srcFormat,
			PTRefNum_p	PTRefNumNew)
{
PTErr_t		PTErr;
fut_hdr_p	PTHdr = NULL;
KpHandle_t	PTHdrH = NULL, PTDataH = NULL;
KpChar_t	colorSpaceAttr[20];

	*PTRefNumNew = 0;

	if ( ! IS_FUT(*futSrc)) goto ErrOut1;

	PTHdr = allocBufferPtr (sizeof(fut_hdr_t));	/* get buffer for resultant info header */
	if (PTHdr == NULL) {
		goto ErrOut4;
	}

	if (!fut_io_encode (*futSrc, PTHdr)) {	/* make the info header */
		goto ErrOut3;
	}

	PTHdr->srcFormat = srcFormat;

	PTDataH = fut_unlock_fut (*futSrc);
	if (PTDataH == NULL) {
		goto ErrOut2;
	}
	*futSrc = NULL;

	PTHdrH = unlockBufferPtr (PTHdr);		/* unlock the header buffer */
	if (PTHdrH == NULL) {
		goto ErrOut2;
	}
	PTHdr = NULL;

	PTErr = registerPT (PTHdrH, NULL, PTRefNumNew);	/* enter PT into list */
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}

	makeActive (*PTRefNumNew, PTDataH);		/* activate the new PT */

	if (inSpace != -1) {	/* set the input color space attribute */
		KpItoa (inSpace, colorSpaceAttr); 
		PTErr = PTSetAttribute (*PTRefNumNew, KCM_IN_SPACE, colorSpaceAttr);
	}

	if (outSpace != -1) {	/* set the output color space attribute */
		KpItoa (outSpace, colorSpaceAttr); 
		PTErr = PTSetAttribute (*PTRefNumNew, KCM_OUT_SPACE, colorSpaceAttr);
	}

	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}

getOut:
	return PTErr;


ErrOut4:
	PTErr = KCP_NO_CHECKIN_MEM;	
	goto ErrOut0;

ErrOut3:
	PTErr = KCP_ENCODE_PTHDR_ERR;
	goto ErrOut0;

ErrOut2:
	PTErr = KCP_MEM_UNLOCK_ERR;
	goto ErrOut0;

ErrOut1:
	PTErr = KCP_BAD_ARG;

ErrOut0:
	if (PTDataH != NULL) {
		*futSrc = fut_lock_fut (PTDataH);
	}
	if (*futSrc != FUT_NULL) fut_free (*futSrc);

	if (PTHdr != NULL) freeBufferPtr (PTHdr);
	if (PTHdrH != NULL) freeBuffer (PTHdrH);
	if (*PTRefNumNew != 0) PTCheckOut (*PTRefNumNew);

	goto getOut;
}


/* PTGetSize calculates the size of a PT in FuT format.  */
PTErr_t
	PTGetSize (	PTRefNum_t	PTRefNum,
				KpInt32_p	mBlkSize)
{
PTErr_t		errnum;

	errnum = PTGetSizeF (PTRefNum, PTTYPE_FUTF, mBlkSize);

	return (errnum);
}


/* PTGetSizeF calculates the size of a PT in any format.  */
PTErr_t
	PTGetSizeF (PTRefNum_t	PTRefNum,
				PTType_t	format,
				KpInt32_p	mBlkSize)
{
PTErr_t		errnum, errnum1;
KpInt32_t	extSize, intSize;
KpHandle_t	PTHdr, PTData;
PTRefNum_t	matrixPTRefNum;
KpUInt32_t	lutConfig;
#if !defined KCP_ICC_ONLY
KpHandle_t	PTAttr;
#endif
#if !defined KCMS_NO_CRC
KpChar_t 	strCRCmade[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
KpInt32_t	crc32;
#endif

	errnum = getPTStatus (PTRefNum);

	if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_PT_INACTIVE) || (errnum == KCP_SERIAL_PT)) {

		if (mBlkSize == NULL)
			return (KCP_BAD_PTR);

		switch (format) {
#if !defined KCP_ICC_ONLY
			case PTTYPE_FUTF:
				extSize = KCP_PT_HEADER_SIZE;	/* size of external header */
				break;
#endif

			case PTTYPE_MFT1:
			case PTTYPE_MFT2:
			case PTTYPE_MFT2_VER_0:
				extSize = (2 * sizeof (KpInt32_t)) + (4 * sizeof (KpUInt8_t))
					+ (MF_MATRIX_DIM * MF_MATRIX_DIM * sizeof (KpInt32_t));
				break;

			case PTTYPE_MAB1:
			case PTTYPE_MAB2:
			case PTTYPE_MBA1:
			case PTTYPE_MBA2:
				extSize = (7 * sizeof (KpInt32_t)) + (2 * sizeof (KpUInt8_t)) + sizeof (KpUInt16_t);
				errnum1 = getMatrixPTRefNum (PTRefNum, &matrixPTRefNum, &lutConfig);
				if (KCP_SUCCESS == errnum1)
				{
					extSize += ((MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM) * sizeof (KpInt32_t));
				}
				break;

			default:
				return (KCP_INVAL_PTTYPE);
		}
	   	
		if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_SERIAL_PT)) {

		/* when active, add size of external PT data block */
			PTHdr = getPTHdr (PTRefNum);
			PTData = getPTData (PTRefNum);

			intSize = TpGetDataSize (PTHdr, PTData, format);
			if (intSize == 0) {
				PTRefNum_t	resizePTRefNum;

				/* 	TpGetDataSize will return 0 if the grid table 
					dimensions are not valid for the format specified.
					PTGetPTF will attempt to resize the grid table of
					the PT.  Check if that resizing is possible */
				errnum = gridDimValid (format, PTRefNum, &resizePTRefNum);
  				if (errnum != KCP_SUCCESS) {
					return errnum;
				}

				/*	Determine the size of the resized PT */
				PTHdr = getPTHdr (resizePTRefNum);
				PTData = getPTData (resizePTRefNum);
				intSize = TpGetDataSize (PTHdr, PTData, format);
				PTCheckOut (resizePTRefNum);
				if (intSize == 0) {
					return KCP_INCON_PT;
				}
			}
			extSize += intSize;	/* add size of data */

		#if !defined KCMS_NO_CRC
			switch (format) {
			case PTTYPE_FUTF:
				errnum = TpCalCrc (PTHdr, PTData, &crc32);
				if (errnum == KCP_SUCCESS) {
					KpItoa(crc32, strCRCmade);
					PTSetAttribute(PTRefNum, KCM_CRC, strCRCmade);
				}
				break;

			default:
				break;
			}
		#endif
		}
	   	
/* add size of attributes. Must be done after CRC calculation */
#if !defined KCP_ICC_ONLY
		switch (format) {
		case PTTYPE_FUTF:
			PTAttr = getPTAttr (PTRefNum);
			extSize += getAttrSize(PTAttr);		/* plus size of attributes */

			break;

		default:
			break;
		}
#endif
	   	
		*mBlkSize = extSize;	/* return external size of PT */
		errnum = KCP_SUCCESS;
	}

	return (errnum);
}


/* PTGetPT writes a PT to external memory in FuT format.  */
PTErr_t
	PTGetPT (	PTRefNum_t	PTRefNum,
				KpInt32_t	mBlkSize,
				PTAddr_t	PTAddr)
{
PTErr_t		errnum;

	errnum = PTGetPTF (PTRefNum, PTTYPE_FUTF, mBlkSize, PTAddr);

	return (errnum);
}


/* PTGetPTF writes a PT to external memory in a variety of formats.  */
PTErr_t
	PTGetPTF (	PTRefNum_t	PTRefNum,
				PTType_t	format,
				KpInt32_t	mBlkSize,
				PTAddr_t	PTAddr)
{
PTErr_t		errnum, PTstatus;
KpHandle_t	PTHdr, PTAttr, PTData;
KpFd_t		fd;
KpInt32_t	attrSize, resultSize, nBytes;
PTRefNum_t	resizePTRefNum = 0, thePTRefNum;
KpChar_p	memData;


	errnum = getPTStatus (PTRefNum);

	if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_PT_INACTIVE) || (errnum == KCP_SERIAL_PT)) {
		#if defined KCP_DIAG_LOG
		{KpChar_t	string[256];
		sprintf (string, "\nPTGetPTF\n PTRefNum %x, format %x, mBlkSize %d, PTAddr %x\n",
							PTRefNum, format, mBlkSize, PTAddr);
		kcpDiagLog (string);}
		#endif

		PTstatus = errnum;

		/* 	verify the dimensions of the grid table are valid for the specified format. */
		errnum = gridDimValid (format, PTRefNum, &resizePTRefNum);
		if (errnum != KCP_SUCCESS) {
			goto ErrOut1;
		}

		if (resizePTRefNum != 0) {
			thePTRefNum = resizePTRefNum;	/* resized PT made */
		}
		else {
			thePTRefNum = PTRefNum;			/* use original PT */
		}
		
		/*	determine the size the resized PT */
		errnum = PTGetSizeF (thePTRefNum, format, &resultSize);
		if (errnum != KCP_SUCCESS) {
			goto ErrOut1;
		}

		if (resultSize > mBlkSize) {	/* PT may not be larger than the buffer size */
			errnum = KCP_PT_BLOCK_TOO_SMALL;
			goto ErrOut1;
		}

		PTAttr = getPTAttr (thePTRefNum);
		PTHdr = getPTHdr (thePTRefNum);
		PTData = getPTData (thePTRefNum);

		/* initialize memory file manager to write the PT */
		if (KpOpen (NULL, "m", &fd, NULL, (KpGenericPtr_t)PTAddr, mBlkSize) != KCMS_IO_SUCCESS) {
			errnum = KCP_SYSERR_1;
			goto ErrOut1;
		}

		attrSize = getAttrSize (PTAttr);					/* get size of attributes */
		errnum = TpWriteHdr (&fd, format, PTHdr, attrSize);	/* write the header info */

		if (errnum != KCP_SUCCESS) {
			Kp_close (&fd);
			goto ErrOut1;
		} 	

#if !defined KCP_ICC_ONLY
		switch (format) {
		case PTTYPE_FUTF:
			errnum = writeAttributes (&fd, PTAttr);	/* write the attributes */
			if (errnum != KCP_SUCCESS) {
				break;
			}

		default:
			break;
		}
#endif

		/* if PT active, write data to external format */
		if (((PTstatus == KCP_PT_ACTIVE) || (PTstatus == KCP_SERIAL_PT)) && (errnum == KCP_SUCCESS)) {
			errnum = TpWriteData (&fd, format, PTHdr, PTData);
		}

		(void) Kp_close (&fd);

		/*	if the result PT size is smaller than the memory block size
			fill the end of the memory block with zeros						*/
		nBytes = mBlkSize - resultSize;
		if (nBytes > 0) {
			memData = (KpChar_p)PTAddr + resultSize;
			while (nBytes--) {
				*memData++ = 0;
			}
		}			

	}

ErrOut1:
	if (resizePTRefNum != 0) {
		PTCheckOut (resizePTRefNum);
	}

	return (errnum);
}


/* PTMemTest checks whether there is a reasonable amount of memory still available */
KpInt32_t PTMemTest(void)
{
KpHandle_t theBuffer;

#if defined (KPMAC)
	theBuffer = NewHandle (MINPTMEMSIZE);	/* do not want temp mem in this case */
	MemLogAlloc (theBuffer, MINPTMEMSIZE);
#else
	theBuffer = allocBufferHandle (MINPTMEMSIZE);
#endif

	if (theBuffer == NULL) {
		return (0);
	}
	else {
		freeBuffer (theBuffer);
		return (1);
	}
}


/* gridDimValid determines whether the grid table dimensions are valid
 * for the format specified.  If the dimensions are not valid then the
 * function attempts to create a
 * PT with the correct size grid tables.  If it is successful the
 * PTRefNum of the resized PT is returned in the location pointed to by
 * resizePTRefNumP.  If resizing is not required the value returned in 
 * the location pointed to by resizePTRefNumP is 0.
 *
 * NOTE:  	If this function creates a resized PT, that PT is checked in.  
 *			it is the responsibility of the calling function to check out
 *			that PT.
 */
static PTErr_t
	gridDimValid (	PTType_t	format,
					PTRefNum_t	PTRefNum,
					PTRefNum_p	resizePTRefNumP)
{
KpHandle_t	PTData;
KpInt32_t	inputChans, outputChans, LUTDimensions, dummy = 0;
fut_p	 	fut;
PTErr_t		retVal, error = KCP_SUCCESS;

	/*	Assume no resizing */
	if (NULL != resizePTRefNumP) {
		*resizePTRefNumP = 0;
	}

	/*	Convert the PTRefNum to a fut */
	PTData = getPTData (PTRefNum);
	fut = fut_lock_fut (PTData);
	if (fut == FUT_NULL) {
		return KCP_PTERR_2;
	}

	if ( ! IS_FUT (fut) ) {	/* check for valid fut */
		retVal = KCP_NOT_FUT;
		goto GetOut;
	}

	switch (format ) {

#if !defined KCP_ICC_ONLY
	case PTTYPE_FUTF:
		/* 	may want to check if any of the grid dimensions exceed the max 
			grid dimension, but for now accept any size						*/
		break;
#endif
	case PTTYPE_MAB1:
	case PTTYPE_MAB2:
	case PTTYPE_MBA1:
	case PTTYPE_MBA2:
		/* 	may want to check if any of the grid dimensions exceed the max 
			grid dimension, but for now accept any size						*/
		break;

	case PTTYPE_MFT1:
	case PTTYPE_MFT2:
	case PTTYPE_MFT2_VER_0:
		/*	The grid dimensions must all be the same.  If they are not then
			attempt to build a grid table where all the dimensions are the
			same.															*/
		retVal = (PTErr_t) fut_mfutInfo (fut, &LUTDimensions, &inputChans, &outputChans, format,
										&dummy, &dummy, &dummy);
		if (1 != retVal) {
			if (-2 != retVal) {
				retVal = KCP_INVAL_GRID_DIM;
				goto GetOut;
			}
			else {
				KpInt32_t	i1, newGridDims[FUT_NICHAN];
				fut_p		futresized;

				for (i1 = 0; i1 < FUT_NICHAN; i1++) {		/* define new grid sizes */
					newGridDims[i1] = LUTDimensions;
				}

				futresized = fut_resize (fut, newGridDims);	/* resize the fut */
				if (futresized == NULL) {
					retVal = KCP_NO_MEMORY;
					goto GetOut;
				}

				if (futresized == fut) {					/* should not happen, probably fut_mfutInfo() error */
					retVal = KCP_SYSERR_3;
					goto GetOut;
				}

				if (fut_to_mft (futresized) != 1) {			/* convert to reference tables */
					retVal = KCP_INCON_PT;
					goto GetOut;
				}

				retVal = fut2PT (&futresized, -1, -1, PTTYPE_CALCULATED, resizePTRefNumP);	/* make into PT */
				if (retVal == KCP_SUCCESS) {
					retVal = copyAllAttr (PTRefNum, *resizePTRefNumP);	/* Copy all attributes to new PT */
					if (retVal != KCP_SUCCESS) {
						PTCheckOut (*resizePTRefNumP);
						goto GetOut;
					}
				}
 			}
		}
		break;

	default:
		retVal = KCP_INVAL_PTTYPE;
	}

	retVal = KCP_SUCCESS;

GetOut:
	fut_unlock_fut (fut);
	return retVal;
}
