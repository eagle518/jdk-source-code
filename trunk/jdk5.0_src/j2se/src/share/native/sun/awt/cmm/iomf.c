/*
 * @(#)iomf.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	iomf.c

	Contains:	functions to read and write binary matrix fut files.

	Author:		George Pawle

	All opens, closes, reads and writes are performed with the functions
	KpOpen, Kp_close, Kp_read, and Kp_write, respectively
	to provide an "operating system independent" i/o interface.	These
	functions are implemented differently for each operating system, and are
	defined in the library kcms_sys.

	COPYRIGHT (c) 1993-2002 Eastman Kodak Company.
	As	an	unpublished	work pursuant to Title 17 of the United
	States Code.	All rights reserved.
 */

#include <string.h>
#include <stdio.h>
#include "kcptmgr.h"
#include "attrib.h"
#include "makefuts.h"

#define FUT_MATRIX_ZERO (0x0)
#define FUT_MATRIX_ONE (0x10000)
#define GBUFFER_SIZE (MF1_TBL_ENT*2)

/* constants for CP version 0 */
#define KCP_V0_ITABLE_SIZE	(256)
#define KCP_V0_OTABLE_SIZE	(4096)
#define KCP_9TO6_MASK	(0x3c0)
#define KCP_3TO0_MASK	(0xf)

/* constants for Mab & Mba Lut tags */
#define ICC_TOTAL_CHANNELS		(16)
#define MAB_CURVE_OFFSETS		(12)
#define MAB_FIRST_DATA_ENTRY	(32)

#define CURVE_COUNT (8)

/* data types for various flavors of mft2's */
typedef enum mft2Type_e {
	KCP_UNKNOWN_TYPE	=0,
	KCP_V0_TYPE			=1,		/* mft2 with version 0 transform */
	KCP_MFT2_TYPE_1		=3,		/* mft2 with lutType clarification */
	KCP_MFT2_FROM_MFT1	=4		/* mft2 generated from mft1 */
} mft2Type_t, FAR* mft2Type_p;

/* ICC mft2 info
 * used for reading in ICC profile matrix-fut tables
 */
typedef struct mft2_s {
	KpInt32_t		nIChan;						/* Number of input channels */
	KpInt32_t		nOChan;						/* Number of output channels */
    KpInt32_t		clutDimSize;				/* Number of grid points */
    KpInt32_t		iTblEntries;				/* Number of input table entries */
    KpInt32_t		clutEntries;				/* Number of clut table entries */
    KpInt32_t		oTblEntries;				/* Number of output table entries */
    mft2Type_t		type;						/* type of data in tables */
	mf2_tbldat_p	inputTable [FUT_NICHAN];	/* addresses of input tables */
	mf2_tbldat_p	clutTable;					/* address of clut table, which is interleaved grid tables */
	mf2_tbldat_p	outputTable [FUT_NOCHAN];	/* addresses of output tables */
} mft2_t, FAR* mft2_p;

/* ICC mft2 info
 * used for reading in ICC profile matrix-fut tables
 */
typedef struct mab_s {
	KpUInt32_t		lutConfig;					/* type of lut used to create this fut */
	KpUInt32_t		nIChan;						/* Number of input channels */
	KpUInt32_t		nOChan;						/* Number of output channels */
    KpUInt8_t		clutDimSize [FUT_NCHAN];	/* Number of grid points per channel */
    KpUInt8_t		clutPrecision;				/* precision of the data in the clut tables */
    KpUInt32_t		bTblEntries[FUT_NCHAN];		/* Number of input table entries */
	mab_tbldat_p	bTable[FUT_NCHAN];			/* addresses of input tables */
	PTParaCurve_t	bPTParaCurve[FUT_NCHAN];	/* Parametric curve data */
    KpUInt32_t		mTblEntries[FUT_NCHAN];		/* Number of M curve table entries */
	mab_tbldat_p	mTable[FUT_NCHAN];			/* addresses of M curve tables */
	PTParaCurve_t	mPTParaCurve[FUT_NCHAN];	/* Parametric curve data */
    KpUInt32_t		clutEntries;				/* Number of clut table entries */
	mab_tbldat_p	clutTable;					/* addresses of clut table, which are interleaved grid tables */
    KpUInt32_t		aTblEntries[FUT_NCHAN];		/* Number of output table entries */
	mab_tbldat_p	aTable[FUT_NCHAN];			/* addresses of output tables */
	PTParaCurve_t	aPTParaCurve[FUT_NCHAN];	/* Parametric curve data */
} mab_t, FAR* mab_p;

typedef struct moffsets_s {
	KpInt32_t		bCurveOffset;				/* offset to the B curve */
	KpInt32_t		matrixOffset;				/* offset to the matrix */
	KpInt32_t		mCurveOffset;				/* offset to the M curve */
	KpInt32_t		clutOffset;					/* offset to the CLUT */
    KpInt32_t		aCurveOffset;				/* offset to the A curve */
} moffsets_t, FAR* moffsets_p;

typedef struct {
	KpUInt32_t	nCount;
} mabCurve_t;

typedef struct {
	KpUInt16_t	nFunction;
	KpUInt16_t	nReserve;
} mabPara_t;


typedef struct mcurve_s {
	KpInt32_t		nSig;						/* Type signature */
	KpInt32_t		nReserve;					/* Reserved */
	union {
		mabCurve_t	Curve;
		mabPara_t	Para;
	} C;
} mcurve_t, FAR* mcurve_p;

typedef struct mclut_s {
	KpUInt8_t		nDim[ICC_TOTAL_CHANNELS];	/* Number of grid points in each dimension */
	KpUInt8_t		nPrecision;					/* Precision of data elements */
	KpUInt8_t		nReserve0;					/* Reserved */
	KpUInt8_t		nReserve1;					/* Reserved */
	KpUInt8_t		nReserve2;					/* Reserved */
} mclut_t, FAR* mclut_p;

static void			fut_free_mft (mft2_p);
static void			fut_free_mab (mab_p);
static KpUInt32_t	fut_read_mft_data (KpFd_p, fut_hdr_p, Fixed_p, mft2_p);
static mft2Type_t	checkCPv0Gen (mft2_p);
static mft2Type_t	checkT2DGen (mft2_p);
static fut_p		futFromMFutTbls	(mft2_p, fut_hdr_p);
static KpInt32_t	calcNextGBufSize (KpInt32_t, KpInt32_p);
static KpDouble64_t	getValueRatio (PTDataMap_t, KpInt32_t, KpInt32_t);
static KpDouble64_t	getIndexRatio (PTDataMap_t, KpInt32_t, KpInt32_t);
static KpInt32_t	fut_read_mab_data (KpFd_p, fut_hdr_p, Fixed_p, mab_p);
static fut_p		futFromMabFutTbls	(mab_p, fut_hdr_p);
static KpInt32_t	readMabCurveData(KpFd_p, KpUInt32_t, KpUInt32_p, mab_tbldat_p *, PTParaCurve_p);
static KpInt32_t	writeMabCurveData(KpFd_p, KpUInt32_t, mab_tbldat_p, 
										PTParaCurve_p, PTDataMap_t, PTDataMap_t);
static KpInt32_t	writeMatrixData(KpFd_p, fut_p);
static KpInt32_t	writeClutData(KpFd_p, fut_p, KpUInt8_t, KpInt32_t, mclut_p);



/* fut_readMFutHdr reads the header of a matrix fut from
 * an open file descriptor and stores it in a fut I/O header.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_read error
 * -2 to -4 on a table specific error
 */

KpInt32_t
	fut_readMFutHdr(	KpFd_p		fd,
						fut_hdr_p	futHdr)
{
KpInt32_t	status;
KpUInt32_t	dummy;
KpUInt8_t	inVars, outChans, gridDim;
		
	futHdr->iDataClass = KCP_UNKNOWN;
	futHdr->oDataClass = KCP_UNKNOWN;

	/* read in the common matrix fut stuff */
	status = Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt32_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&inVars, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&outChans, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&gridDim, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));

	if (status == 1) {
		futHdr->version = 1;					/* save in fut locations */
		futHdr->order = 0;

		if ((inVars < 1) || (inVars > FUT_NICHAN)) {
			return (-2);
		}
		futHdr->icode[0] = (KpInt32_t) inVars;

		if ((outChans < 1) || (outChans > FUT_NOCHAN)) {
			return (-3);
		}
		futHdr->icode[1] = (KpInt32_t) outChans;

		if (gridDim < 2) {
			return (-4);
		}
		futHdr->icode[2] = (KpInt32_t) gridDim;
	}
	else {
		status = -1;
	}

	return status;
}


/* fut_readMFutTbls reads the tables of a matrix fut
 * from an open file descriptor and puts them into the supplied fut.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_read error
 * -2 to -4 on a table specific error
 */
fut_p
	fut_readMFutTbls (	KpFd_p			fd,
						fut_hdr_p		futHdr,
						Fixed_p			matrix)
{
KpInt32_t	status;
KpUInt16_t	iTblEntries = 0, oTblEntries = 0;
mft2_t		theMft2;
fut_p		fut = NULL;
PTDataClass_t	inDataClass, outDataClass;


	theMft2.nIChan = futHdr->icode[0];				/* get the size info */
	theMft2.nOChan = futHdr->icode[1];
	theMft2.clutDimSize = futHdr->icode[2];

	status = fut_read_mft_data (fd, futHdr, matrix, &theMft2);	/* get the data for the mft */

	if (status == 1) {	/* determine the type of data in the mft */
		if ((futHdr->spaceIn == KCM_ADOBE_LAB) || (futHdr->spaceOut == KCM_ADOBE_LAB)) {
			if (futHdr->spaceIn == KCM_ADOBE_LAB) {
				inDataClass = KCP_VARIABLE_RANGE;
			} else {
				inDataClass = KCP_FIXED_RANGE;
			}
			if (futHdr->spaceOut == KCM_ADOBE_LAB) {
				outDataClass = KCP_VARIABLE_RANGE;
			} else {
				outDataClass = KCP_FIXED_RANGE;
			}
			fut = get_linlab_fut (KCP_LINLAB_GRID_SIZE, inDataClass, outDataClass);
		}
		else {
			if (futHdr->magic == PTTYPE_MFT1) {		/* if transform is mft1 */
				theMft2.type = KCP_MFT2_FROM_MFT1;	/* in differentiated PCS/device mode */
			}
			else {

				/* fix a bug in OCS */
				if ((futHdr->spaceOut == KCM_CMYK) && (theMft2.nOChan == 3)) {
					futHdr->spaceOut = KCM_CIE_LAB;
					futHdr->oDataClass = KCP_LAB_PCS;
				}

				theMft2.type = checkCPv0Gen (&theMft2);				/* mft2 made by V0 CP? */
				
				if (theMft2.type != KCP_V0_TYPE) {
					if (futHdr->profileType != KCM_ICC_TYPE_0) {	/* from non-version 0 profile? */
						theMft2.type = KCP_MFT2_TYPE_1;				/* in differentiated PCS/device mode */
					}
					else {
						theMft2.type = KCP_V0_TYPE;					/* default to version 0 type */
					}
				}
			}

			#if defined KCP_DIAG_LOG
			{KpChar_t	string[256];
			sprintf (string, "fut_readMFutTbls\n iDataClass %d, oDataClass %d, profileType %d, spaceIn %d, spaceOut %d, theMft2.type %d\n",
						futHdr->iDataClass, futHdr->oDataClass, futHdr->profileType, futHdr->spaceIn, futHdr->spaceOut, theMft2.type);
			kcpDiagLog (string);}
			#endif

			fut = futFromMFutTbls (&theMft2, futHdr);				/* convert the tables into a fut */
		}
	}

	fut_free_mft (&theMft2);	/* free the mft memory */

	return fut;
}


/* get the data for the mft */
/* read the data and convert mft1 to mft2 format if needed */
KpUInt32_t
	fut_read_mft_data (	KpFd_p			fd,
						fut_hdr_p		futHdr,
						Fixed_p			matrix,
						mft2_p			theMft2)
{
KpUInt16_t		tmpUI16;
KpInt32_t		srciTblEntries, i1, status, srcEntryBytes;
KpInt32_t		srciTblSize, srcCLutSize, srcoTblSize, mft2iTblSize, mft2CLutSize, mft2oTblSize;
KpUInt8_p		mf1dataP;
KpUInt32_t		tmpData;
PTDataMap_t		tableMap;
mf2_tbldat_t	gData, tmpTbl [MF2_MAX_TBL_ENT];
mf2_tbldat_p	gDataP;
Fixed_t			lMatrix [MF_MATRIX_DIM * MF_MATRIX_DIM];
Fixed_p			lMatrixP;

	theMft2->inputTable[0] = NULL;		/* in case of error */
	theMft2->clutTable = NULL;
	theMft2->outputTable[0] = NULL;

	/* read the matrix tables */
	if (matrix != NULL) {
		lMatrixP = matrix;				/* return matrix data */
	}
	else {
		lMatrixP = (Fixed_p)&lMatrix;	/* discard matrix data */
	}

	status = Kp_read (fd, (KpGenericPtr_t) lMatrixP, sizeof(Fixed_t) * MF_MATRIX_DIM * MF_MATRIX_DIM);
	if (status != 1) {
		goto ErrOutM1;
	}

#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)lMatrixP, MF_MATRIX_DIM * MF_MATRIX_DIM);
#endif

	/* get the number of input and output table entries */
	switch (futHdr->magic) {
	case PTTYPE_MFT1:
		srcEntryBytes = sizeof (mf1_tbldat_t);	/* size of each entry in this table */
		srciTblEntries = MF1_TBL_ENT;

		if ((futHdr->iDataClass == KCP_VARIABLE_RANGE) || (futHdr->iDataClass == KCP_XYZ_PCS)) {
			theMft2->iTblEntries = MF2_STD_ITBL_SIZE;
		}
		else {
			theMft2->iTblEntries = MF1_TBL_ENT;
		}

		theMft2->oTblEntries = MF1_TBL_ENT;
		break;

	case PTTYPE_MFT2:
		srcEntryBytes = sizeof (mf2_tbldat_t);	/* size of each entry in this table */

		status = Kp_read (fd, (KpGenericPtr_t)&tmpUI16, sizeof (KpUInt16_t));
		if (status != 1) {
			goto ErrOutM1;
		}
	#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
	#endif

		if ((tmpUI16 < MF2_MIN_TBL_ENT) || (tmpUI16 > MF2_MAX_TBL_ENT)) {
			goto ErrOut0;
		}

		srciTblEntries = tmpUI16;
		theMft2->iTblEntries = srciTblEntries;

		status = Kp_read (fd, (KpGenericPtr_t)&tmpUI16, sizeof (KpUInt16_t));
		if (status != 1) {
			goto ErrOutM1;
		}
	#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
	#endif

		if ((tmpUI16 < MF2_MIN_TBL_ENT) || (tmpUI16 > MF2_MAX_TBL_ENT)) {
			goto ErrOut0;
		}

		theMft2->oTblEntries = tmpUI16;
		break;

	default:
		goto ErrOutM2;	/* unknown type */
	}
	
	theMft2->clutEntries = theMft2->nOChan;
	for (i1 = 0; i1 < theMft2->nIChan; i1++) {	/* calc total entries in the clut */
		theMft2->clutEntries *= theMft2->clutDimSize;
	}

	srciTblSize = srciTblEntries * srcEntryBytes;				/* size in bytes of each table */
	mft2iTblSize = theMft2->iTblEntries * sizeof (mf2_tbldat_t);
	srcCLutSize = theMft2->clutEntries * srcEntryBytes;
	mft2CLutSize = theMft2->clutEntries * sizeof (mf2_tbldat_t);
	srcoTblSize = theMft2->oTblEntries * srcEntryBytes;
	mft2oTblSize = theMft2->oTblEntries * sizeof (mf2_tbldat_t);

	theMft2->inputTable[0] = allocBufferPtr (mft2iTblSize * theMft2->nIChan);	/* get the needed memory for input */
	if (theMft2->inputTable[0] == NULL) {
		goto ErrOut0;
	}

	theMft2->clutTable = allocBufferPtr (mft2CLutSize);		/* clut */
	if (theMft2->clutTable == NULL) {
		goto ErrOut0;
	}

	theMft2->outputTable[0] = allocBufferPtr (mft2oTblSize * theMft2->nOChan);	/* and output tables */
	if (theMft2->outputTable[0] == NULL) {
		goto ErrOut0;
	}

	/* get the input table data */
	for (i1 = 0; i1 < theMft2->nIChan; i1++) {

		status = Kp_read (fd, (KpGenericPtr_t)tmpTbl, srciTblSize);	/* read the input table */
		if (status != 1) {
			goto ErrOutM1;
		}

		theMft2->inputTable[i1] = theMft2->inputTable[0] + (i1 * theMft2->iTblEntries);

		if (futHdr->magic == PTTYPE_MFT1) {			/* convert mft1 to 16 bit reference */
			if ((futHdr->iDataClass == KCP_VARIABLE_RANGE) || (futHdr->iDataClass == KCP_XYZ_PCS)) {
				tableMap = KCP_BASE_MAX_TO_REF16;	/* 8 to 16 bit PCS */
			}
			else {
				tableMap = KCP_MAP_END_POINTS;		/* 8 to 16 bit device */
			}

			convert1DTable (tmpTbl, sizeof (mf1_tbldat_t), srciTblEntries, MF1_TBL_MAXVAL,
							theMft2->inputTable[i1], sizeof (mf2_tbldat_t), theMft2->iTblEntries, MF2_TBL_MAXVAL,
							tableMap, KCP_MAP_END_POINTS);
		}
		else {	/* already mft2 format, just copy */
		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, theMft2->iTblEntries);
		#endif
			KpMemCpy (theMft2->inputTable[i1], tmpTbl, mft2iTblSize);
		}
	}

	/* get the clut data */
	mf1dataP = (KpUInt8_p) theMft2->clutTable;
	if (futHdr->magic == PTTYPE_MFT1) {
		mf1dataP += (mft2CLutSize - srcCLutSize);	/* load into end of buffer */
	}

	status = Kp_read (fd, (KpGenericPtr_t) mf1dataP, srcCLutSize);	
	if (status != 1) {
		goto ErrOutM1;
	}

	if (futHdr->magic == PTTYPE_MFT1) {		/* convert to mft2 using map end points */
		gDataP = theMft2->clutTable;
		for (i1 = 0; i1 < theMft2->clutEntries; i1++) {
			tmpData = (KpUInt32_t) mf1dataP [i1];
			gData = (mf2_tbldat_t) (((tmpData * MF2_TBL_MAXVAL) + (MF1_TBL_MAXVAL >> 1)) / MF1_TBL_MAXVAL);
			gDataP[i1] = gData;				/* store each clut entry */
		}
	}
	else {
	#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)theMft2->clutTable, theMft2->clutEntries);
	#endif
	}

	/* get the output table data */
	for (i1 = 0; i1 < theMft2->nOChan; i1++) {

		status = Kp_read (fd, (KpGenericPtr_t)tmpTbl, srcoTblSize);	/* read the output table */
		if (status != 1) {
			goto ErrOutM1;
		}

		theMft2->outputTable[i1] = theMft2->outputTable[0] + (i1 * theMft2->oTblEntries);

		if (futHdr->magic == PTTYPE_MFT1) {	/* convert source to 16 bit reference */
			if ((futHdr->oDataClass == KCP_VARIABLE_RANGE) || (futHdr->oDataClass == KCP_XYZ_PCS)) {
				tableMap = KCP_BASE_MAX_TO_REF16;	/* 8 to 16 bit Lab or XYZ */
			}
			else {
				tableMap = KCP_MAP_END_POINTS;		/* 8 to 16 bit device */
			}

			convert1DTable (tmpTbl, sizeof (mf1_tbldat_t), theMft2->oTblEntries, MF1_TBL_MAXVAL,
							theMft2->outputTable[i1], sizeof (mf2_tbldat_t), theMft2->oTblEntries, MF2_TBL_MAXVAL,
							KCP_MAP_END_POINTS, tableMap);
		}
		else {	/* already 16 bits, just copy */
		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, theMft2->oTblEntries);
		#endif
			KpMemCpy (theMft2->outputTable[i1], tmpTbl, mft2oTblSize);
		}
	}

GetOut:
	return status;


ErrOutM2:
	status = -2;
	goto GetOut;

ErrOutM1:
	status = -1;
	goto GetOut;

ErrOut0:
	status = 0;
	goto GetOut;
}


void
	fut_free_mft (mft2_p	theMft2)
{
	freeBufferPtr (theMft2->inputTable[0]);	/* release all mft2 memory */
	theMft2->inputTable[0] = NULL;
	freeBufferPtr (theMft2->clutTable);
	theMft2->clutTable = NULL;
	freeBufferPtr (theMft2->outputTable[0]);
	theMft2->outputTable[0] = NULL;
}


void
	fut_free_mab (mab_p	theMab)
{
	freeBufferPtr (theMab->aTable[0]);	/* release all mab memory */
	theMab->aTable[0] = NULL;
	freeBufferPtr (theMab->mTable[0]);
	theMab->mTable[0] = NULL;
	freeBufferPtr (theMab->clutTable);
	theMab->clutTable = NULL;
	freeBufferPtr (theMab->bTable[0]);
	theMab->bTable[0] = NULL;
}


/* futFromMFutTbls the tables of an mft2 to build a fut with mft2 reference tables
 * It checks for special cases and handles them appropriately.
 * Returns: 
 * 1 on success
 * -2 on memory allocation error
 */
static fut_p
	futFromMFutTbls (	mft2_p		theMft2,
						fut_hdr_p	futHdr)
{
fut_p			fut = NULL;
KpInt32_t		i1, i2, srcOTableMax, iTblEntries, gTblEntries, oTblEntries;
KpUInt32_t		tmpData;
PTDataMap_t		inputMap, outputMap;
mf2_tbldat_t	gData, tmpMftTable [MF2_MAX_TBL_ENT];
mf2_tbldat_p	mftDataP, gDataP;
KpInt32_t		iomask, dimTbl[FUT_NICHAN] = {1, 1, 1, 1, 1, 1, 1, 1};
fut_itbl_p		itbl, itbls[FUT_NICHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_gtbl_p		gtbl, gtbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_otbl_p		otbl, otbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

	/* set up the input mapping mode */
#if defined KCP_MAP_ENDPOINT_REF
	if (theMft2->iTblEntries == MF2_STD_ITBL_SIZE) {
		iTblEntries = MFV_STD_ITBL_SIZE;		/* switch to a better size */
	} else {
		iTblEntries = theMft2->iTblEntries;		/* retain size */
	}
	if (futHdr->iDataClass == KCP_XYZ_PCS) {
		inputMap = KCP_BASE_MAX_TO_REF16;
	}
	else if (theMft2->type == KCP_V0_TYPE) {
		inputMap = KCP_MAP_END_POINTS;			/* this results in copying tables */
	}
	else if (futHdr->spaceIn == KCM_CIE_LAB) {
		inputMap = KCP_BASE_MAX_TO_REF16;
	}
	else {
		inputMap = KCP_MAP_END_POINTS;			/* this results in copying tables */
	}
#elif defined KCP_MAP_BASE_MAX_REF
	if ((theMft2->type == KCP_V0_TYPE) &&
		((futHdr->iDataClass == KCP_VARIABLE_RANGE) ||
		 (futHdr->iDataClass == KCP_XYZ_PCS))) {
		if (theMft2->iTblEntries == KCP_V0_ITABLE_SIZE) {
			iTblEntries = MF2_STD_ITBL_SIZE;		/* switch to a better size */
		}
		else {
			iTblEntries = theMft2->iTblEntries;		/* retain size */
		}
		inputMap = KCP_BASE_MAX_TO_REF16;
	}
	else {
		iTblEntries = theMft2->iTblEntries;		/* this results in copying tables */
		inputMap = KCP_MAP_END_POINTS;
	}
#endif
	if (iTblEntries < 64) {
		iTblEntries = 64;		/* use minimum size */
	}

	/* convert each input table to mft2 */
	iomask = 0;
	for (i1 = 0; i1 < theMft2->nIChan; i1++) {
		if ((itbl = fut_alloc_itbl ()) == NULL) {	/* get an input table */
			goto GetOut;
		}

		itbls[i1] = itbl;					/* build itbl list */

		if (fut_alloc_imftdat (itbl, iTblEntries) == NULL) {
			goto GetOut;
		}

		itbl->id = fut_unique_id ();		/* this table has its own data */

		iomask |= FUT_IN(FUT_BIT(i1));		/* build i/o mask */
		dimTbl[i1] = theMft2->clutDimSize;	/* and grid table dimensions */		

		itbl->size = dimTbl[i1];
		itbl->dataClass = futHdr->iDataClass;

		convert1DTable (theMft2->inputTable [i1], sizeof (mf2_tbldat_t), theMft2->iTblEntries, MF2_TBL_MAXVAL,
						itbl->refTbl, sizeof (mf2_tbldat_t), iTblEntries, MF2_TBL_MAXVAL,
						inputMap, KCP_MAP_END_POINTS);
	}

	/* calc # of grid table entries */
	gTblEntries = theMft2->clutEntries / theMft2->nOChan;

	/* set up the output mapping mode */
	oTblEntries = theMft2->oTblEntries;
#if defined KCP_MAP_ENDPOINT_REF
	if (futHdr->spaceOut == KCM_CIE_LAB) {
			outputMap = KCP_BASE_MAX_TO_REF16;
			srcOTableMax = KCP_LAB16_L_100;
			if ((theMft2->type == KCP_MFT2_TYPE_1) && (oTblEntries < KCP_V1_ITABLE_SIZE)) {
				oTblEntries = KCP_V1_ITABLE_SIZE;		/* use minimum size */
			}
	}
	else if (futHdr->oDataClass == KCP_XYZ_PCS) {
			outputMap = KCP_BASE_MAX_TO_REF16;
			srcOTableMax = KCP_LAB16_L_100;
	}
	else {
		outputMap = KCP_MAP_END_POINTS;
		if (theMft2->type == KCP_V0_TYPE) {
			if (KCP_BASE_MAX_TO_REF16 == outputMap) {
				srcOTableMax = FUT_MAX_PEL12;
			}
			else {
				srcOTableMax = FUT_OUT_MAXVAL;
			}
		}
		else {
			srcOTableMax = MF2_TBL_MAXVAL;		/* this results in copying tables */
		}
	}
#elif defined KCP_MAP_BASE_MAX_REF
	if (theMft2->type == KCP_V0_TYPE) {
		if ((futHdr->oDataClass == KCP_VARIABLE_RANGE) ||
			(futHdr->oDataClass == KCP_XYZ_PCS)) {
			outputMap = KCP_BASE_MAX_TO_REF16;
			srcOTableMax = FUT_OUT_MAXVAL;
		}
		else {
			outputMap = KCP_MAP_END_POINTS;
			srcOTableMax = FUT_MAX_PEL12;
		}
	}
	else {
		srcOTableMax = MF2_TBL_MAXVAL;		/* this results in copying tables */
		outputMap = KCP_MAP_END_POINTS;
	}
#endif
	/* convert each channel */
	for (i1 = 0; i1 < theMft2->nOChan; i1++) {
		iomask |= FUT_OUT(FUT_BIT(i1));		/* build i/o mask */

		/* convert the grid tables */
		if ((gtbl = fut_alloc_gtbl ()) == NULL) {	/* get a grid table */
			goto GetOut;
		}

		gtbls[i1] = gtbl;					/* build gtbl list */

		gtbl->id = fut_unique_id ();		/* this table has its own data */

		gtbl->tbl_size = sizeof(fut_gtbldat_t);

		for (i2 = 0; i2 < FUT_NICHAN; i2++) {
			gtbl->size[i2] = (KpInt16_t) dimTbl[i2];	/* insert size of each dimension */
			gtbl->tbl_size *= gtbl->size[i2];	/* calc total grid size */
		}

		if (fut_alloc_gmftdat (gtbl) == NULL) {
			goto GetOut;
		}

		mftDataP = theMft2->clutTable;		/* source data */
		mftDataP += i1;						/* offset for this channel */
		gDataP = gtbl->refTbl;				/* destination grid table pointer */
		
		for (i2 = 0; i2 < gTblEntries; i2++) {
			gData = *mftDataP;
			mftDataP += theMft2->nOChan;	/* next channel data */

			if (theMft2->type == KCP_V0_TYPE) {
				tmpData = (KpUInt32_t) gData >> (MF2_TBL_BITS - FUT_GRD_BITS);
				gData = (mf2_tbldat_t) (((tmpData * MF2_TBL_MAXVAL) + (FUT_GRD_MAXVAL >> 1)) / FUT_GRD_MAXVAL);
			}

			gDataP[i2] = gData;	/* store each grid table entry */
		}

		/* convert the output tables to mft2 */
		if ((otbl = fut_alloc_otbl ()) == NULL) {	/* get an output table */
			goto GetOut;
		}

		otbls[i1] = otbl;							/* build otbl list */

		if (fut_alloc_omftdat (otbl, oTblEntries) == NULL) {	/* get memory for table data */
			goto GetOut;
		}

		otbl->id = fut_unique_id ();				/* this table has its own data */
		otbl->dataClass = futHdr->oDataClass;		/* define data class */

		mftDataP = theMft2->outputTable [i1];		/* source data */

		if (theMft2->type == KCP_V0_TYPE) {
			for (i2 = 0; i2 < theMft2->oTblEntries; i2++) {
				tmpMftTable [i2] = mftDataP[i2] >> (MF2_TBL_BITS - FUT_GRD_BITS);	/* extract actual data from high 12 bits */
			}
			
			mftDataP = tmpMftTable;					/* this is now the source data */
		}

		/* convert otbl data into reference table data */
		convert1DTable (mftDataP, sizeof (mf2_tbldat_t), theMft2->oTblEntries, srcOTableMax,
						otbl->refTbl, sizeof (mf2_tbldat_t), oTblEntries, MF2_TBL_MAXVAL,
						KCP_MAP_END_POINTS, outputMap);

	}

	/* Assemble FuT:  */
	fut = fut_new (iomask, itbls, gtbls, otbls);
	if (fut != NULL) {
		KpInt32_t	isTwoCubedILabCS;

		/* test for image Lab input and PCS Lab output with 2 grid points in each dimension */
		isTwoCubedILabCS = 0;
		if (((futHdr->spaceIn == KCM_IMAGE_LAB) ||(futHdr->spaceIn == KCM_RGB)) && (futHdr->spaceOut == KCM_CIE_LAB)) {
			KpInt32_t	i1;

			isTwoCubedILabCS = 1;

			for (i1 = 0; i1 < 3; i1++) {
				fut_itbl_p	itbl;
				fut_chan_p	chan;

				chan = fut->chan[i1];
				if ((! IS_CHAN(chan)) || (chan->imask != FUT_XYZ)) {	/* must have 3 output channels, each a function of 3 inputs */
					isTwoCubedILabCS = 0;
				}

				itbl = fut->itbl[i1];						/* every grid dimension must be 2 */
				if (! IS_ITBL(itbl) || (itbl->size != 2)) {
					isTwoCubedILabCS = 0;
				}
			}
		}

		if (isTwoCubedILabCS == 1) {
			PTDataClass_t	dataClass = KCP_FIXED_RANGE;
			fut_p			resizeFut, futResized;
#if defined KCP_MAP_BASE_MAX_REF
			KpInt32_t		lab8diff, lab16diff;
			mf2_tbldat_t	L100, aNeutral, bNeutral, lab8a, lab16a;
			mf2_tbldat_p	indat[3], outdat[1];

			if (theMft2->type != KCP_MFT2_FROM_MFT1) {
				/* evaluate a* channel with lab16 neutral and lab8 neutral
				 * if the result is closest to lab16 neutral, assumr lab16 but clip to lab8
				 */
				indat[0] = &L100; indat[1] = &aNeutral; indat[2] = &bNeutral;
				outdat[0] = &lab8a;

				L100 = KCP_LAB8_L_100; aNeutral = KCP_LAB8_AB_NEUTRAL; bNeutral = KCP_LAB8_AB_NEUTRAL;
				evaluateFut (fut, FUT_Y, KCM_USHORT, 1, (KpGenericPtr_t FAR*) indat, (KpGenericPtr_t FAR*) outdat);
				lab8diff = lab8a & 0xffff;
				lab8diff -= KCP_LAB16_AB_NEUTRAL;
				if (lab8diff < 0) {
					lab8diff = -lab8diff;			/* absolute value */
				}

				outdat[0] = &lab16a;
				L100 = KCP_LAB16_L_100; aNeutral = KCP_LAB16_AB_NEUTRAL; bNeutral = KCP_LAB16_AB_NEUTRAL;
				evaluateFut (fut, FUT_Y, KCM_USHORT, 1, (KpGenericPtr_t FAR*) indat, (KpGenericPtr_t FAR*) outdat);
				lab16diff = lab16a &0xffff;
				lab16diff -= KCP_LAB16_AB_NEUTRAL;
				if (lab16diff < 0) {
					lab16diff = -lab16diff;			/* absolute value */
				}

				if (lab16diff < lab8diff) {
					dataClass = KCP_LAB_PCS;
				}
			}
#endif
			resizeFut = get_linlab_fut (KCP_LINLAB_GRID_SIZE, dataClass, dataClass);

			if (resizeFut != NULL) {
				futResized = fut_comp (fut, resizeFut, iomask);	/* resize the fut */
				fut_free (resizeFut);
				if (futResized != NULL) {
					fut_free (fut);			/* free small fut */
					fut = futResized;		/* use resized fut */
				}
			}
		}
	}

GetOut:
	fut_free_tbls (FUT_NICHAN, (void *)itbls);
	fut_free_tbls (FUT_NOCHAN, (void *)gtbls);
	fut_free_tbls (FUT_NOCHAN, (void *)otbls);

	return fut;
}


/* mf_store_fp stores fut to the file named "filename", performing the
 * open and close automatically.  Returns 1 on success, 0 or negative
 * on error.
 */
KpInt32_t
	mf_store_fp (	fut_p			fut,
					KpChar_p		filename,
					KpFileProps_t	fileProps,
					KpInt32_t		MFutType)
{
KpFd_t		fd;
KpInt32_t	ret = 0;

	/* Open with the new e mode for exclusive.  The file must be closed, or at least unlocked when done */
	if (KpOpen (filename, "e", &fd, &fileProps) ) {
		if ((ret = makeMftTblDat (fut)) == 1) {
			ret = fut_writeMFut_Kp (&fd, fut, NULL, MFutType);
		}

		(void) Kp_close (&fd);
	}

	return (ret);
}


/* fut_writeMFut_Kp writes a fut in the specified matrix fut format
 * to an open file descriptor.
 * Returns: 
 * 1 on success
 * 0 on invalid fut error
 * -1 on header, id string, or Kp_write error
 * -2 to -5 on a table specific error
 */
KpInt32_t
	fut_writeMFut_Kp (	KpFd_p		fd,
						fut_p		fut,
						Fixed_p		matrix,
						KpInt32_t	MFutType)
{
KpInt32_t	status;
KpUInt32_t	dummy = 0;
KpUInt8_t	inputChans, outputChans, LUTDimensions;
KpInt32_t	inputChansT, outputChansT, LUTDimensionsT;
KpInt32_t	i1, i2, i3, wrtEntryBytes, outCount, outBytes, gData, dstData;
KpInt32_t	wrtiTblEntries, wrtiTableMaxValue, gTblEntries, wrtgTableMaxValue;
KpInt32_t	wrtoTblEntries, wrtoTableMaxValue, totalGSize;
PTDataMap_t	inputMap, outputMap;
Fixed_t		lMatrix[MF_MATRIX_DIM * MF_MATRIX_DIM];
KpUInt16_t	tmpUI16;
fut_chan_p	chan;
KpUInt8_p	mf1dataP;
KpUInt16_p	mf2dataP;
mf2_tbldat_p	gDataP[FUT_NOCHAN];
mf2_tbldat_t	tmpTbl[MF2_MAX_TBL_ENT];
KpInt32_t	tagType;

	status = fut_mfutInfo (fut, &LUTDimensionsT, &inputChansT, &outputChansT, MFutType,
								&wrtiTblEntries, &gTblEntries, &wrtoTblEntries);
	if (status != 1) {
		goto GetOut;
	}
	
	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "fut_writeMFut_Kp\n MFutType %x\n", MFutType);
	kcpDiagLog (string);}
	#endif

	LUTDimensions = (KpUInt8_t)LUTDimensionsT; /* type conversion */
	inputChans = (KpUInt8_t)inputChansT;
	outputChans = (KpUInt8_t)outputChansT;
	
	if (MFutType == PTTYPE_MFT2_VER_0) {		/* for header of LUT tag */
		tagType = PTTYPE_MFT2;
	}
	else {
		tagType = MFutType;
	}

#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)&tagType, 1);	
#endif

	/* write out the common matrix fut stuff */
	status = Kp_write (fd, (KpGenericPtr_t)&tagType, sizeof(KpInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&inputChans, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&outputChans, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&LUTDimensions, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));

	if (status != 1) {
		goto ErrOutM1;
	}
	
	/* get the matrix to write */
	if (matrix != NULL) {	/* copy the matrix */
		for (i1 = 0; i1 < (MF_MATRIX_DIM * MF_MATRIX_DIM); i1++) {
			lMatrix[i1] = matrix[i1];
		}
	}
	else {	/* create an identity matrix when there is no matrix */
		for (i1 = 0, i3 = 0; i1 < MF_MATRIX_DIM; i1++) {
			for (i2 = 0; i2 < MF_MATRIX_DIM; i2++, i3++) {
				if (i1 == i2) {
					lMatrix[i3] = FUT_MATRIX_ONE;
				}
				else {
					lMatrix[i3] = FUT_MATRIX_ZERO;
				}
			}
		}
	}
	
#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)lMatrix, MF_MATRIX_DIM * MF_MATRIX_DIM);
#endif

	/* write out the matrix */
	status = Kp_write (fd, (KpGenericPtr_t) lMatrix, sizeof(Fixed_t) * MF_MATRIX_DIM * MF_MATRIX_DIM);
	if (status != 1) {
		goto ErrOutM1;
	}

	/* set up to write standard mft2 tables, which may be modifided by the other cases */
	wrtEntryBytes = sizeof (mf2_tbldat_t);				/* bytes in each table entry */

	wrtiTableMaxValue = MF2_TBL_MAXVAL;
	inputMap = KCP_MAP_END_POINTS;

	wrtgTableMaxValue = MF2_TBL_MAXVAL;

	wrtoTableMaxValue = MF2_TBL_MAXVAL;
	outputMap = KCP_MAP_END_POINTS;

	switch (MFutType) {
		case PTTYPE_MFT1:
			wrtEntryBytes = sizeof (mf1_tbldat_t);
			
			wrtiTableMaxValue = MF1_TBL_MAXVAL;
			if ((fut->itbl[0]->dataClass == KCP_VARIABLE_RANGE) || (fut->itbl[0]->dataClass == KCP_XYZ_PCS)) {
				inputMap = KCP_REF16_TO_BASE_MAX;
			}

			wrtgTableMaxValue = MF1_TBL_MAXVAL;

			wrtoTableMaxValue = MF1_TBL_MAXVAL;
			if ((fut->chan[0]->otbl->dataClass == KCP_VARIABLE_RANGE) || (fut->chan[0]->otbl->dataClass == KCP_XYZ_PCS)) {
				outputMap = KCP_REF16_TO_BASE_MAX;
			}

			break;

		case PTTYPE_MFT2_VER_0:
			if ((fut->itbl[0]->dataClass == KCP_VARIABLE_RANGE) || (fut->itbl[0]->dataClass == KCP_XYZ_PCS)) {
				inputMap = KCP_REF16_TO_BASE_MAX;
			}

			wrtgTableMaxValue = FUT_GRD_MAXVAL;

			if ((fut->chan[0]->otbl->dataClass == KCP_VARIABLE_RANGE) || (fut->chan[0]->otbl->dataClass == KCP_XYZ_PCS)) {
				outputMap = KCP_REF16_TO_BASE_MAX;
				wrtoTableMaxValue = FUT_GRD_MAXVAL;
			}
			else {
				wrtoTableMaxValue = FUT_MAX_PEL12;
			}

		/* no changes for standard mft2
		 * write the additional info for mft2's
		 */
		case PTTYPE_MFT2:
			tmpUI16 = (KpUInt16_t) wrtiTblEntries;

		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
		#endif
		
			status = Kp_write (fd, (KpGenericPtr_t) &tmpUI16, sizeof (KpUInt16_t));
			if (status != 1) {
				goto ErrOutM1;
			}

			tmpUI16 = (KpUInt16_t) wrtoTblEntries;
		
		#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)&tmpUI16, 1);
		#endif
		
			status = Kp_write (fd, (KpGenericPtr_t) &tmpUI16, sizeof (KpUInt16_t));
			if (status != 1) {
				goto ErrOutM1;
			}
		
			break;

		default:
			goto ErrOutM2;	/* unknown type */
	}

/* input table conversion */
	/* convert each input table to required precision */
	for (i1 = 0; (fut->itbl[i1] != NULL) && (i1 < FUT_NICHAN); i1++) {
		convert1DTable (fut->itbl[i1]->refTbl, sizeof (mf2_tbldat_t), fut->itbl[i1]->refTblEntries, MF2_TBL_MAXVAL,
						tmpTbl, wrtEntryBytes, wrtiTblEntries, wrtiTableMaxValue,
						inputMap, KCP_MAP_END_POINTS);

	#if (FUT_MSBF == 0)
		if (MFutType != PTTYPE_MFT1)
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, wrtiTblEntries);
	#endif
		status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, wrtiTblEntries * wrtEntryBytes); /* write the input table */
		if (status != 1) {
			goto ErrOutM1;
		}
	}

/* grid table conversion */
	for (outputChans = 0; outputChans < FUT_NOCHAN; outputChans++) {
		if ((chan = fut->chan[outputChans]) == NULL) {
			break;
		}

		gDataP[outputChans] = chan->gtbl->refTbl;	/* get each grid table pointer */
	}

	totalGSize = gTblEntries * outputChans * wrtEntryBytes;
	outCount = 0;												/* count bytes written to buffer */
	outBytes = calcNextGBufSize (GBUFFER_SIZE, &totalGSize);	/* set up for first write */

	mf1dataP = (KpUInt8_p) tmpTbl;
	mf2dataP = (KpUInt16_p) tmpTbl;
	
	for (i1 = 0; i1 < gTblEntries; i1++) {
		for (i2 = 0; i2 < (KpInt32_t)outputChans; i2++) {
			gData = (KpUInt32_t) *(gDataP[i2])++;	/* get each grid table entry */

			dstData = ((gData * wrtgTableMaxValue) + (MF2_TBL_MAXVAL >> 1)) / MF2_TBL_MAXVAL;	/* scale for output */

			if (MFutType == PTTYPE_MFT1) {
				*mf1dataP++ = (KpUInt8_t)dstData;
			}
			else {
				if (MFutType == PTTYPE_MFT2_VER_0) {
					KpInt32_t   noise;

					noise = (dstData >> 2) & (FUT_BIT(MF2_TBL_BITS - FUT_GRD_BITS) -1);
					gData = (dstData << (MF2_TBL_BITS - FUT_GRD_BITS)) | noise;	/* 12 bit data with psuedo-noise in low 4 bits */
				}
			#if (FUT_MSBF == 0)
				Kp_swab16 ((KpGenericPtr_t)&gData, 1);
			#endif
				*mf2dataP++ = (KpUInt16_t) gData;
			}

			outCount += wrtEntryBytes;				/* count bytes in buffer */

			if (outCount == outBytes) {
				outCount = 0;					/* reset bytes written */
				mf1dataP = (KpUInt8_p) tmpTbl;
				mf2dataP = (KpUInt16_p) tmpTbl;

				status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, outBytes);
				if (status != 1) {
					goto ErrOutM1;
				}

				outBytes = calcNextGBufSize (outBytes, &totalGSize);	/* set up for next time */
			}
		}
	}

/* output table conversion */
	for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
		if ((chan = fut->chan[i1]) == NULL) {
			break;										/* all done */
		}

		convert1DTable (fut->chan[i1]->otbl->refTbl, sizeof (mf2_tbldat_t), fut->chan[i1]->otbl->refTblEntries, MF2_TBL_MAXVAL,
						tmpTbl, wrtEntryBytes, wrtoTblEntries, wrtoTableMaxValue,
						KCP_MAP_END_POINTS, outputMap);

		if (MFutType == PTTYPE_MFT2_VER_0) {
			mf2dataP = (KpUInt16_p) tmpTbl;

			for (i2 = 0; i2 < wrtoTblEntries; i2++) {
				KpUInt16_t   noise, srcData;

				srcData = mf2dataP[i2];

				noise = (srcData >> 2) & (FUT_BIT(MF2_TBL_BITS - FUT_GRD_BITS) -1);
				tmpUI16 = (srcData << (MF2_TBL_BITS - FUT_GRD_BITS)) | noise;	/* 12 bit data with psuedo-noise in low 4 bits */
				mf2dataP[i2] = tmpUI16;
 			}
		}

		#if (FUT_MSBF == 0)
		if (MFutType != PTTYPE_MFT1) {
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, wrtoTblEntries);
		}
		#endif

		status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, wrtoTblEntries * wrtEntryBytes);
		if (status != 1) {
			goto ErrOutM1;
		}
	}

GetOut:
	return status;


ErrOutM2:
	status = -2;
	goto GetOut;

ErrOutM1:
	status = -1;
	goto GetOut;
}


static KpInt32_t
	calcNextGBufSize ( KpInt32_t curBufSize, KpInt32_p totalBytesRemaining)
{
KpInt32_t	bytesToWrite;

	*totalBytesRemaining -= curBufSize;		/* bytes to write after this buffer is written */
	if (*totalBytesRemaining <= 0) {
		bytesToWrite = *totalBytesRemaining + curBufSize;	/* last buffer to write, adjust size */
	}
	else {
		bytesToWrite = curBufSize;			/* not last buffer to write */
	}

	return bytesToWrite;
}


/* fut_mfutInfo returns matrix fut information of a fut.
 *
 * Returns: 
 *	1	on success
 *	0	invalid fut
 * -1	grid dimensions too large
 * -2	grid dimensions are not the same
 * -3	input channels not contiguous and starting at 0 or too many inputs
 * -4	output channels not contiguous and starting at 0 or too many outputs
 */
KpInt32_t
	fut_mfutInfo (	fut_p		fut,				/* get info of this fut */
					KpInt32_p	LUTDimensionsP,		/* # points in each grid dimension */
					KpInt32_p	inputChansP,		/* # of input channels */
					KpInt32_p	outputChansP,		/* # of input channels */
					KpInt32_t	MFutType,			/* type of intended table output */
					KpInt32_p	inputEntriesP,		/* # of input table entries */
					KpInt32_p	gridEntriesP,		/* # of grid table entries */
					KpInt32_p	outputEntriesP)		/* # of output table entries */
{
KpInt32_t	LUTDimensions, inputChans, outputChans, iTblEntries, gTblEntries, oTblEntries;
KpInt32_t	imask, omask, thisGridDim;
KpInt32_t	status = 1;								/* assume success */

	if ( ! IS_FUT(fut)  || ! IS_ITBL(fut->itbl[0])) {
		return (0);
	}

/* get the # of input channels, # of output channels, and the grid dimensions */

	/* input tables must be common and in first n contiguous input channels */
	imask = fut->iomask.in;							/* get the fut's input mask */
	LUTDimensions = fut->itbl[0]->size;				/* initialize the size */

	if (LUTDimensions > MF_GRD_MAXDIM) {
		status = -1;								/* this fut can not be made into a matrix fut */
	}
	
	for (inputChans = 0; inputChans < FUT_NICHAN; inputChans++, imask >>= 1) {
		if ( ! IS_ITBL(fut->itbl[inputChans]) || ((imask & 1) == 0)) {
			break;
		}
		
		thisGridDim = fut->itbl[inputChans]->size;

		if (LUTDimensions != thisGridDim) {			/* sizes must be the same */
			if (LUTDimensions < thisGridDim) {		/* always return largest size */
				LUTDimensions = thisGridDim;
			}

			if (status == 1) {
				status = -2;						/* this fut can not be made into a matrix fut */
			}
		}
	}

	if (imask != 0) {		/* input channels not contiguous */
		if (status == 1) {
			status = -3;	/* this fut can not be made into a matrix fut */
		}
	}
	
	/* output tables must be in first n contiguous output channels */
	omask = fut->iomask.out;				/* get the fut's output mask */
	outputChans = 0;

	for (outputChans = 0; outputChans < FUT_NOCHAN; outputChans++, omask >>= 1) {
		if ( ! IS_CHAN(fut->chan[outputChans]) || ((omask & 1) == 0)) {
			break;
		}
	}

	if (omask != 0) {
		if (status == 1) {
			status = -4;	/* this fut can not be made into a matrix fut */
		}
	}

	/* get # entries in input, grid, and output tables */
	iTblEntries = 0;	/* in case type not specified */
	gTblEntries = 0;
	oTblEntries = 0;

	if (( ! IS_GTBL(fut->chan[0]->gtbl)) || ( ! IS_OTBL(fut->chan[0]->otbl))) {
		status = -4;		/* this fut can not be made into a matrix fut */
	}
	else {
		gTblEntries = fut->chan[0]->gtbl->tbl_size / sizeof (fut_gtbldat_t);

		switch (MFutType) {
		case PTTYPE_MFT1:
			iTblEntries = MF1_TBL_ENT;
			oTblEntries = MF1_TBL_ENT;
			break;

		case PTTYPE_MFT2:
			iTblEntries = fut->itbl[0]->refTblEntries;
			oTblEntries = fut->chan[0]->otbl->refTblEntries;
			break;

		case PTTYPE_MFT2_VER_0:
			if (fut->itbl[0]->refTblEntries == MF2_STD_ITBL_SIZE) {
				iTblEntries = KCP_V0_ITABLE_SIZE;			/* return original size size */
			}
			else {
				iTblEntries = fut->itbl[0]->refTblEntries;	/* retain size */
			}
			oTblEntries = KCP_V0_OTABLE_SIZE;
			break;
		
		default:
			break;
		}
	}

	*LUTDimensionsP = LUTDimensions;				/* return info */
	*inputChansP = inputChans;
	*outputChansP = outputChans;
	*inputEntriesP = iTblEntries;
	*gridEntriesP = gTblEntries;
	*outputEntriesP = oTblEntries;

	return (status);
}


/* return 1 if the matrix is an identity matrix, 0 if not */
KpInt32_t
	isIdentityMatrix (	Fixed_p		matrix,
						KpInt32_t	matrixSize)
{
KpInt32_t	i1, i2, i3;

	for (i1 = 0, i3 = 0; i1 < matrixSize; i1++) {
		for (i2 = 0; i2 < matrixSize; i2++, i3++) {
			if (i1 == i2) {
				if (matrix[i3] != FUT_MATRIX_ONE) {
					return (0);
				}
			}
			else {
				if (matrix[i3] != FUT_MATRIX_ZERO) {
					return (0);
				}
			}
		}
	}

	return (1);
}


/* Convert a table to a table with differing size and/or precision
 * The modes using REF8 and REF16 assume that "END_POINTS" tables
 * are referenced to 8 and 16 bit ICC PCS definitions, respectively.
 */
KpInt32_t
	convert1DTable (KpGenericPtr_t srcTable, KpInt32_t srcBytes, KpInt32_t srcEntries, KpUInt32_t srcMaxValue,
					KpGenericPtr_t destTable, KpInt32_t destBytes, KpInt32_t destEntries, KpUInt32_t destMaxValue,
					PTDataMap_t inputMode, PTDataMap_t outputMode)
{
KpInt32_t	dIndex, sTableIndex, sTableIndexNext;
KpUInt32_t	intDestData, copyIndex, copyValue;
KpDouble64_t	sIndex, sTableFrac, srcData, srcData1, destData;
KpDouble64_t indexRatio, valueRatio;

	indexRatio = getIndexRatio (inputMode, srcEntries, destEntries);
	
	if (fabs(indexRatio - 1.0) < .00001) {
		copyIndex = 1;	/* just copy */
	}
	else {
		copyIndex = 0;
	}

	valueRatio = getValueRatio (outputMode, srcMaxValue, destMaxValue);
	
	if (fabs(valueRatio - 1.0) < .00001) {
		copyValue = 1;	/* just copy */
	}
	else {
		copyValue = 0;
	}
	
	for (dIndex = 0; dIndex < destEntries; dIndex++) {
		
		if (copyIndex == 1) {
			sTableIndex = dIndex;							/* source table position */
			sTableIndexNext = sTableIndex;
			sTableFrac = (KpDouble64_t)0;					/* Set for compiler - not used in this path */
		}
		else {
			sIndex = (KpDouble64_t) dIndex * indexRatio;			/* calculate the source table position */
			sTableIndex = (KpInt32_t) sIndex;					/* the source index */
			sTableFrac = sIndex - (KpDouble64_t) sTableIndex;	/* and the source interpolant */

			if (sTableIndex >= srcEntries) {	/* make sure we're in range for interpolation */
				sTableIndex = srcEntries -1;	/* 1st source is past end */
				sTableIndexNext = sTableIndex;
			}
			else {
				sTableIndexNext = sTableIndex +1;

				if (sTableIndexNext == srcEntries) {
					sTableIndexNext = sTableIndex;	/* 1st source is at end */
				}
			}
		}

		switch (srcBytes) {			/* get the table values to interpolate */
		case 1:
			srcData = (KpDouble64_t) ((KpUInt8_p) srcTable) [sTableIndex];
			srcData1 = (KpDouble64_t) ((KpUInt8_p) srcTable) [sTableIndexNext];
			break;
		
		case 2:
			srcData = (KpDouble64_t) ((KpUInt16_p) srcTable) [sTableIndex];
			srcData1 = (KpDouble64_t) ((KpUInt16_p) srcTable) [sTableIndexNext];
			break;
		
		case 4:
			srcData = (KpDouble64_t) ((KpUInt32_p) srcTable) [sTableIndex];
			srcData1 = (KpDouble64_t) ((KpUInt32_p) srcTable) [sTableIndexNext];
			break;
		
		default:
			srcData = (KpDouble64_t)0;
			srcData1 = (KpDouble64_t)0;
			break;
		}

		if (copyIndex != 1) {
			srcData += (sTableFrac * (srcData1 - srcData));	/* interpolate */
		}

		if (copyValue == 1) {
			intDestData = (KpUInt32_t)(srcData  + 0.5);
		}
		else {
			destData = srcData * valueRatio;					/* convert for output */

			intDestData = (KpUInt32_t)(destData + 0.5);			/* round and convert to integer */
		}

		if (intDestData > destMaxValue) {
			intDestData = destMaxValue;
		}

		switch (destBytes) {		/* store each entry */
		case 1:
			((KpUInt8_p) destTable) [dIndex] = (KpUInt8_t)intDestData;
			break;
		
		case 2:
			((KpUInt16_p) destTable) [dIndex] = (KpUInt16_t)intDestData;
			break;
		
		case 4:
			((KpUInt32_p) destTable) [dIndex] = intDestData;
			break;
		
		default:
			((KpUInt32_p) destTable) [dIndex] = 0;
			break;
		}
	}
	
	return 0;
}


static KpDouble64_t
	getIndexRatio (	PTDataMap_t	inputMode,
					KpInt32_t	srcEntries,
					KpInt32_t	destEntries)
{	
KpDouble64_t sStepFloat, dStepFloat;
KpDouble64_t indexRatio;

	switch (inputMode) {
	case KCP_MAP_END_POINTS:
		sStepFloat = (KpDouble64_t) (srcEntries -1);
		dStepFloat = (KpDouble64_t) (destEntries -1);
		break;

#if defined KCP_MAP_ENDPOINT_REF
	case KCP_REF16_TO_BASE_MAX:
		sStepFloat = (KpDouble64_t) srcEntries  * 65535;
		dStepFloat = (KpDouble64_t) (destEntries -1) * 65536;
		break;

	case KCP_BASE_MAX_TO_REF16:
		sStepFloat = (KpDouble64_t) (srcEntries -1) * 65536;
		dStepFloat = (KpDouble64_t) destEntries * 65535;
		break;

#elif defined KCP_MAP_BASE_MAX_REF
	case KCP_REF16_TO_BASE_MAX:
		sStepFloat = (KpDouble64_t) (srcEntries -1)  * 65536;
		dStepFloat = (KpDouble64_t) destEntries * 65535;
		break;

	case KCP_BASE_MAX_TO_REF16:
		sStepFloat = (KpDouble64_t) srcEntries * 65535;
		dStepFloat = (KpDouble64_t) (destEntries -1) * 65536;
		break;
#endif

	case KCP_V4LAB_TO_REF16:
		sStepFloat = (KpDouble64_t) (srcEntries -1);
		dStepFloat = (KpDouble64_t) (destEntries -1) * 256 / 257;
		break;
	
	case KCP_REF16_TO_V4LAB:
		sStepFloat = (KpDouble64_t) (srcEntries -1) * 256 / 257;
		dStepFloat = (KpDouble64_t) (destEntries -1);
		break;
	
	default:
		sStepFloat = 0.0;
		dStepFloat = 1.0;
		break;
	}

	indexRatio = sStepFloat / dStepFloat;
	
	return (indexRatio);
}	

	
static KpDouble64_t
	getValueRatio (	PTDataMap_t	outputMode,
					KpInt32_t	srcMaxValue,
					KpInt32_t	destMaxValue)
{
KpDouble64_t valueRatio, srcTempMaxValue, destTempMaxValue;

	switch (outputMode) {
	case KCP_MAP_END_POINTS:
		srcTempMaxValue = (KpDouble64_t) srcMaxValue;
		destTempMaxValue = (KpDouble64_t) destMaxValue;
		break;

#if defined KCP_MAP_ENDPOINT_REF
	case KCP_REF16_TO_BASE_MAX:
		srcTempMaxValue = (KpDouble64_t) (srcMaxValue + 1) * 65535;
		destTempMaxValue = (KpDouble64_t) destMaxValue * 65536;
		break;

	case KCP_BASE_MAX_TO_REF16:
		srcTempMaxValue = (KpDouble64_t) srcMaxValue *  65536;
		destTempMaxValue = (KpDouble64_t) (destMaxValue + 1) * 65535;
		break;

#elif defined KCP_MAP_BASE_MAX_REF
	case KCP_REF16_TO_BASE_MAX:
		srcTempMaxValue = (KpDouble64_t) srcMaxValue *  65536;
		destTempMaxValue = (KpDouble64_t) (destMaxValue + 1) * 65535;
		break;

	case KCP_BASE_MAX_TO_REF16:
		srcTempMaxValue = (KpDouble64_t) (srcMaxValue + 1) * 65535;
		destTempMaxValue = (KpDouble64_t) destMaxValue * 65536;
		break;
#endif

	case KCP_V4LAB_TO_REF16:
		srcTempMaxValue = (KpDouble64_t) srcMaxValue;
		destTempMaxValue = (KpDouble64_t) (srcMaxValue +1) * 255 / 256;
		break;
	
	case KCP_REF16_TO_V4LAB:
		srcTempMaxValue = (KpDouble64_t) (srcMaxValue +1) * 255 / 256;
		destTempMaxValue = (KpDouble64_t) srcMaxValue;
		break;
	
	default:
		srcTempMaxValue = 1.0;
		destTempMaxValue = 0.0;
		break;
	}

	valueRatio = destTempMaxValue / srcTempMaxValue;
	
	return (valueRatio);
}


/* Check the the given mft2 to see if it was generated by a version 0 CP.
 * This is the case when every output table entry has bits 9:6 equal to bits 3:0.
 */

static mft2Type_t
	checkCPv0Gen (mft2_p	theMft2)
{
mft2Type_t		status;
KpInt32_t		i1, i2, bits9to6, bits3to0;
mf2_tbldat_p	theOTbl;
mf2_tbldat_t	theOTblData;

	if ((theMft2->iTblEntries != KCP_V0_ITABLE_SIZE) || (theMft2->oTblEntries != KCP_V0_OTABLE_SIZE)) {
		goto notCPv0Gen;
	}

	for (i1 = 0; i1 < theMft2->nOChan; i1++) {	/* check each entry of each output table */
		theOTbl = theMft2->outputTable [i1];

		for (i2 = 0; i2 < KCP_V0_OTABLE_SIZE; i2++) {
			theOTblData = theOTbl[i2];
			bits9to6 = (KpInt32_t)(theOTblData & KCP_9TO6_MASK) >> 6;
			bits3to0 = theOTblData & KCP_3TO0_MASK;
			
			if (bits9to6 != bits3to0) {		/* do they match? */
				goto notCPv0Gen;			/* this was not generated by CP version 0 */
			}
		}		
	}

	status = KCP_V0_TYPE;

	return status;


notCPv0Gen:
	status = KCP_UNKNOWN_TYPE;

	return status;
}


/* fut_readMabFutHdr reads the header of a matrix fut from
 * an open file descriptor and stores it in a fut I/O header.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_read error
 * -2 to -4 on a table specific error
 */

KpInt32_t
	fut_readMabFutHdr(	KpFd_p		fd,
						fut_hdr_p	futHdr)
{
KpInt32_t	status;
KpUInt32_t	dummy;
KpUInt8_t	inVars, outChans;
		
	futHdr->iDataClass = KCP_UNKNOWN;
	futHdr->oDataClass = KCP_UNKNOWN;

	/* read in the common matrix fut stuff */
	status = Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt32_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&inVars, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&outChans, sizeof(KpUInt8_t)) &&
			Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt16_t));

	if (status == 1) {
		futHdr->version = 1;					/* save in fut locations */
		futHdr->order = 0;

		if ((inVars < 1) || (inVars > FUT_NICHAN)) {
			return (-2);
		}
		futHdr->icode[0] = (KpInt32_t) inVars;

		if ((outChans < 1) || (outChans > FUT_NOCHAN)) {
			return (-3);
		}
		futHdr->icode[1] = (KpInt32_t) outChans;
	} else {
		status = -1;
	}
	return status;
}


/* fut_readMabFutTbls reads the tables of a matrix fut
 * from an open file descriptor and puts them into the supplied fut.
 *
 * Returns: 
 * 1 on success
 * -1 on Kp_read error
 * -2 to -4 on a table specific error
 */
fut_p
	fut_readMabFutTbls (	KpFd_p	fd,
						fut_hdr_p	futHdr,
						Fixed_p		matrix)
{
KpInt32_t	status;
KpUInt16_t	iTblEntries = 0, oTblEntries = 0;
mab_t		theMab;
fut_p		fut = NULL;

	theMab.nIChan = futHdr->icode[0];		/* get the size info */
	theMab.nOChan = futHdr->icode[1];

	status = fut_read_mab_data (fd, futHdr, matrix,  &theMab);	/* get the data for the mft */

	if (status == 1) {	/* determine the type of data in the mft */

#if defined KCP_DIAG_LOG
		{KpChar_t	string[256];
		sprintf (string, "fut_readMFutTbls\n iDataClass %d, oDataClass %d, profileType %d, spaceIn %d, spaceOut %d, theMab.type %d\n",
						futHdr->iDataClass, futHdr->oDataClass, futHdr->profileType, futHdr->spaceIn, futHdr->spaceOut);
		kcpDiagLog (string);}
#endif

		fut = futFromMabFutTbls (&theMab, futHdr);				/* convert the tables into a fut */
	}

	fut_free_mab (&theMab);	/* free the mft memory */

	return fut;
}


/* get the data for the mab/mba */
KpInt32_t
	fut_read_mab_data (	KpFd_p			fd,
						fut_hdr_p		futHdr,
						Fixed_p			matrix,
						mab_p			theMab)
{
moffsets_t		MabOffsets;
KpInt32_t		status;
KpUInt32_t		i1, tmpData;
KpInt32_t		srcCLutSize, mabCLutSize;
KpUInt8_p		mf1dataP;
Fixed_t			lMatrix [MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM];
Fixed_p			lMatrixP;
mab_tbldat_p	gDataP;
KpUInt16_t		gData;


	theMab->lutConfig = LUT_TYPE_UNKNOWN;

	/* read in the all the offsets to determine which combination we have */
	Kp_set_position (fd, MAB_CURVE_OFFSETS);
	status = Kp_read (fd, (KpGenericPtr_t)&MabOffsets, sizeof(MabOffsets));
	if (status != 1) {
		goto ErrOutM1;
	}

#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)&MabOffsets, 5);
#endif

	theMab->aTable[0] = NULL;		/* in case of error */
	theMab->bTable[0] = NULL;
	theMab->clutTable = NULL;
	theMab->mTable[0] = NULL;

	if (0 != MabOffsets.matrixOffset)		/* if there is an offset for the matrix read it in */
	{
		/* read the matrix tables */
		if (matrix != NULL) {
			lMatrixP = matrix;				/* return matrix data */
		}
		else {
			lMatrixP = (Fixed_p)&lMatrix;	/* discard matrix data */
		}
		theMab->lutConfig |= HAS_MATRIX_DATA;
		Kp_set_position (fd, MabOffsets.matrixOffset);
		status = Kp_read (fd, (KpGenericPtr_t) lMatrixP, sizeof(lMatrix));
		if (status != 1) {
			goto ErrOutM1;
		}
#if (FUT_MSBF == 0)
		Kp_swab32 ((KpGenericPtr_t)lMatrixP, (MF_MATRIX_DIM * MF_MATRIX_DIM) + MF_MATRIX_DIM);
#endif
	}

	switch (futHdr->magic) {
	case PTTYPE_MB2A:
		theMab->lutConfig |= MBA_LUT_TYPE;
		if (MabOffsets.bCurveOffset) {
			Kp_set_position (fd, MabOffsets.bCurveOffset);
			status = readMabCurveData(fd, theMab->nIChan, theMab->bTblEntries, theMab->bTable, theMab->bPTParaCurve);
			if (status != 1)
				return -3;
			theMab->lutConfig |= HAS_B_CURVE_DATA;
		}
		if (MabOffsets.mCurveOffset) {
			Kp_set_position (fd, MabOffsets.mCurveOffset);
			status = readMabCurveData(fd, theMab->nIChan, theMab->mTblEntries, theMab->mTable, theMab->mPTParaCurve);
			if (status != 1)
				return -3;
			theMab->lutConfig |= HAS_M_CURVE_DATA;
		}
		if (MabOffsets.aCurveOffset) {
			Kp_set_position (fd, MabOffsets.aCurveOffset);
			status = readMabCurveData(fd, theMab->nOChan, theMab->aTblEntries, theMab->aTable, theMab->aPTParaCurve);
			if (status != 1)
				return -3;
			theMab->lutConfig |= HAS_A_CURVE_DATA;
		}
		break;

	case PTTYPE_MA2B:
		theMab->lutConfig |= MAB_LUT_TYPE;
		if (MabOffsets.bCurveOffset) {
			Kp_set_position (fd, MabOffsets.bCurveOffset);
			status = readMabCurveData(fd, theMab->nOChan, theMab->bTblEntries, theMab->bTable, theMab->bPTParaCurve);
			if (status != 1)
				return -3;
			theMab->lutConfig |= HAS_B_CURVE_DATA;
		}
		if (MabOffsets.mCurveOffset) {
			Kp_set_position (fd, MabOffsets.mCurveOffset);
			status = readMabCurveData(fd, theMab->nOChan, theMab->mTblEntries, theMab->mTable, theMab->mPTParaCurve);
			if (status != 1)
				return -3;
			theMab->lutConfig |= HAS_M_CURVE_DATA;
		}
		if (MabOffsets.aCurveOffset) {
			Kp_set_position (fd, MabOffsets.aCurveOffset);
			status = readMabCurveData(fd, theMab->nIChan, theMab->aTblEntries, theMab->aTable, theMab->aPTParaCurve);
			if (status != 1)
				return -3;
			theMab->lutConfig |= HAS_A_CURVE_DATA;
		}
		break;

	default:
		goto ErrOutM2;	/* unknown type */
	}	
	
	
	/* read in the the grid dimensions */
	if (MabOffsets.clutOffset) {
		Kp_set_position (fd, MabOffsets.clutOffset);
		status = Kp_read (fd, (KpGenericPtr_t)&theMab->clutDimSize, sizeof(KpUInt8_t) * FUT_NCHAN);	
		theMab->lutConfig |= HAS_CLUT_DATA;
		if (status != 1) {
			goto ErrOutM1;
		}
		Kp_skip (fd, 8);					/* jump to the clut precision */
		status = Kp_read (fd, (KpGenericPtr_t)&theMab->clutPrecision, sizeof(KpUInt8_t));	
		Kp_skip (fd, 3);					/* skip the 3 bytes reserved for padding */

		theMab->clutEntries = theMab->nOChan;
		for (i1 = 0; i1 < theMab->nIChan; i1++) {	/* calc total entries in the clut */
			theMab->clutEntries *= theMab->clutDimSize[i1];
		}
		srcCLutSize = theMab->clutEntries * theMab->clutPrecision;
		mabCLutSize = theMab->clutEntries * sizeof (KpUInt16_t);

		theMab->clutTable = allocBufferPtr (mabCLutSize);		/* clut */
		if (theMab->clutTable == NULL) {
			goto ErrOut0;
		}

		/* get the clut data */
		mf1dataP = (KpUInt8_p) theMab->clutTable;
		if (1 == theMab->clutPrecision) {
			mf1dataP += (mabCLutSize - srcCLutSize);	/* load into end of buffer */
		}

		status = Kp_read (fd, (KpGenericPtr_t) mf1dataP, srcCLutSize);	
		if (status != 1) {
			goto ErrOutM1;
		}

		if (1 == theMab->clutPrecision) {		/* convert to 16 bit precision */
			gDataP = theMab->clutTable;
			for (i1 = 0; i1 < theMab->clutEntries; i1++) {
				tmpData = (KpUInt32_t) mf1dataP [i1];
				gData = (mab_tbldat_t) (((tmpData * MF2_TBL_MAXVAL) + (MF1_TBL_MAXVAL >> 1)) / MF1_TBL_MAXVAL);
				gDataP[i1] = gData;				/* store each clut entry */
			}
		}
		else {
#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)theMab->clutTable, theMab->clutEntries);
#endif
		}
	}	


GetOut:
	return status;


ErrOutM2:
	status = -2;
	goto GetOut;

ErrOutM1:
	status = -1;
	goto GetOut;

ErrOut0:
	status = 0;
	goto GetOut;
}


/* futFromMabFutTbls the tables of an mft2 to build a fut with mft2 reference tables
 * It checks for special cases and handles them appropriately.
 * Returns: 
 * 1 on success
 * -2 on memory allocation error
 */
static fut_p
	futFromMabFutTbls (	mab_p		theMab,
						fut_hdr_p	futHdr)
{
fut_p			fut = NULL;
KpUInt32_t		i, i1;
KpInt32_t		i2, srcTblEntries, destTblEntries, gTblEntries, dim[FUT_NCHAN];
KpInt32_p		TblEntriesPtr;
mab_tbldat_p	*srcTable, mabDataP, gDataP;
PTParaCurve_p	PTParaCurve;
KpInt32_t		iomask, dimTbl[FUT_NICHAN] = {1, 1, 1, 1, 1, 1, 1, 1};
fut_itbl_p		itbl, itbls[FUT_NICHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_gtbl_p		gtbl, gtbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_otbl_p		otbl, otbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
PTDataMap_t 	inputMap, outputMap;

	/* setup the proper input tables */
	switch (theMab->lutConfig) {
	case MBA_B_CURVE_ONLY:
	case MBA_B_CLUT_A_COMBO:
		srcTable = theMab->bTable;
		TblEntriesPtr = (KpInt32_p)theMab->bTblEntries;
		PTParaCurve = theMab->bPTParaCurve;
		break;
		
	case MBA_B_MATRIX_M_CLUT_A_COMBO:
		srcTable = theMab->mTable;
		TblEntriesPtr = (KpInt32_p)theMab->mTblEntries;
		PTParaCurve = theMab->mPTParaCurve;
		break;
		
	case MAB_A_CLUT_B_COMBO:
	case MAB_A_CLUT_M_MATRIX_B_COMBO:
		srcTable = theMab->aTable;
		TblEntriesPtr = (KpInt32_p)theMab->aTblEntries;
		PTParaCurve = theMab->aPTParaCurve;
		break;
		
	default:
		TblEntriesPtr = 0;			/* no input tables to convert */
		break;
	}

	if (TblEntriesPtr)
	{
		/* convert each input table to mft2 */
		if ((MBA_B_MATRIX_M_CLUT_A_COMBO != theMab->lutConfig) &&
			(futHdr->spaceIn == KCM_CIE_LAB))
		{
			inputMap = KCP_V4LAB_TO_REF16;
		} else {
			inputMap = KCP_MAP_END_POINTS;
		}	

		iomask = 0;
		for (i1 = 0; i1 < theMab->nIChan; i1++) {
			srcTblEntries = destTblEntries = TblEntriesPtr[i1];
			if ((MBA_B_MATRIX_M_CLUT_A_COMBO != theMab->lutConfig) &&
				(futHdr->spaceIn == KCM_CIE_LAB))
			{
				if (destTblEntries < MF2_STD_ITBL_SIZE) {
					destTblEntries = MF2_STD_ITBL_SIZE;		/* use minimum size */
				}
			} else {
				if (destTblEntries < 64) {
					destTblEntries = 64;		/* use minimum size */
				}
			}	
			if ((itbl = fut_alloc_itbl ()) == NULL) {	/* get an input table */
				goto GetOut;
			}

			itbls[i1] = itbl;					/* build itbl list */

			if (fut_alloc_imftdat (itbl, destTblEntries) == NULL) {
				goto GetOut;
			}

			itbl->id = fut_unique_id ();		/* this table has its own data */

			iomask |= FUT_IN(FUT_BIT(i1));		/* build i/o mask */
			dimTbl[i1] = theMab->clutDimSize[i1];	/* and grid table dimensions */		

			itbl->size = dimTbl[i1];
			itbl->dataClass = futHdr->iDataClass;

			convert1DTable (srcTable[i1], sizeof (mab_tbldat_t), srcTblEntries, MF2_TBL_MAXVAL,
							itbl->refTbl, sizeof (mab_tbldat_t), destTblEntries, MF2_TBL_MAXVAL,
							inputMap, KCP_MAP_END_POINTS);
			itbl->ParaCurve = PTParaCurve[i1];
		}
	}

	switch (theMab->lutConfig) {
	case MAB_A_CLUT_B_COMBO:
	case MAB_A_CLUT_M_MATRIX_B_COMBO:
	case MBA_B_CLUT_A_COMBO:
	case MBA_B_MATRIX_M_CLUT_A_COMBO:
		gTblEntries = 1;				/* need to create some grid tables */
		break;
		
	default:
		gTblEntries = 0;				/* no grid tables to create */
		break;
	}
	if (gTblEntries)
	{
		/* calc # of grid table entries */
		gTblEntries = theMab->clutEntries / theMab->nOChan;
		/* convert each channel */
		for (i1 = 0; i1 < theMab->nOChan; i1++) {
			/* convert the grid tables */
			if ((gtbl = fut_alloc_gtbl ()) == NULL) {	/* get a grid table */
				goto GetOut;
			}
			
			gtbls[i1] = gtbl;					/* build gtbl list */

			gtbl->id = fut_unique_id ();		/* this table has its own data */

			gtbl->tbl_size = sizeof(fut_gtbldat_t);

			for (i2 = 0; i2 < FUT_NICHAN; i2++) {
				gtbl->size[i2] = (KpInt16_t) dimTbl[i2];	/* insert size of each dimension */
				gtbl->tbl_size *= gtbl->size[i2];	/* calc total grid size */
			}

			if (fut_alloc_gmftdat (gtbl) == NULL) {
				goto GetOut;
			}
			mabDataP = theMab->clutTable;		/* source data */
			mabDataP += i1;						/* offset for this channel */
			gDataP = gtbl->refTbl;				/* destination grid table pointer */

			for (i2 = 0; i2 < gTblEntries; i2++) {
				gDataP[i2] = *mabDataP;
				mabDataP += theMab->nOChan;	/* next channel data */
			}
		}
	}

	/* setup the proper output tables */
	switch (theMab->lutConfig) {
	case MAB_B_CURVE_ONLY:
	case MAB_A_CLUT_B_COMBO:
		srcTable = theMab->bTable;
		TblEntriesPtr = (KpInt32_p)theMab->bTblEntries;
		PTParaCurve = theMab->bPTParaCurve;
		break;
		
	case MAB_A_CLUT_M_MATRIX_B_COMBO:
		srcTable = theMab->mTable;
		TblEntriesPtr = (KpInt32_p)theMab->mTblEntries;
		PTParaCurve = theMab->mPTParaCurve;
		break;
		
	case MBA_B_CLUT_A_COMBO:
	case MBA_B_MATRIX_M_CLUT_A_COMBO:
		srcTable = theMab->aTable;
		TblEntriesPtr = (KpInt32_p)theMab->aTblEntries;
		PTParaCurve = theMab->aPTParaCurve;
		break;
		
	default:
		TblEntriesPtr = 0;			/* no output tables to convert */
		break;
	}

	if (TblEntriesPtr)
	{
		if ((MAB_A_CLUT_M_MATRIX_B_COMBO != theMab->lutConfig) &&
			(futHdr->spaceOut == KCM_CIE_LAB))
		{
			outputMap = KCP_V4LAB_TO_REF16;
		} else {
			outputMap = KCP_MAP_END_POINTS;
		}	

		/* convert each channel */
		for (i1 = 0; i1 < theMab->nOChan; i1++) {
			srcTblEntries = destTblEntries = TblEntriesPtr[i1];
			iomask |= FUT_OUT(FUT_BIT(i1));		/* build i/o mask */

			/* convert the output tables to mft2 */
			if ((otbl = fut_alloc_otbl ()) == NULL) {	/* get an output table */
				goto GetOut;
			}

			otbls[i1] = otbl;							/* build otbl list */

			if (fut_alloc_omftdat (otbl, destTblEntries) == NULL) {	/* get memory for table data */
				goto GetOut;
			}

			otbl->id = fut_unique_id ();				/* this table has its own data */
			otbl->dataClass = futHdr->oDataClass;		/* define data class */

			/* convert otbl data into reference table data */
			convert1DTable (srcTable[i1], sizeof (mab_tbldat_t), srcTblEntries, MF2_TBL_MAXVAL,
							otbl->refTbl, sizeof (mab_tbldat_t), destTblEntries, MF2_TBL_MAXVAL,
							KCP_MAP_END_POINTS, outputMap);
			otbl->ParaCurve = PTParaCurve[i1];
		}
	}
	
	if ((MAB_B_CURVE_ONLY != theMab->lutConfig) && (MBA_B_CURVE_ONLY != theMab->lutConfig)) {
		/* Assemble FuT:  */
		fut = fut_new (iomask, itbls, gtbls, otbls);
	} else {
		for (i1 = 0; i1 < theMab->nOChan; i1++) {
			dim[i1] = 8;
		}
		fut = fut_new_empty (theMab->nIChan, dim, theMab->nOChan, futHdr->iDataClass, futHdr->oDataClass);
		if (MBA_B_CURVE_ONLY == theMab->lutConfig)
		{	
			for (i1 = 0; i1 < theMab->nIChan; i1++) {
				fut_free_itbl (fut->itbl[i1]);			/* free the data */
				fut->itbl[i1] = fut_copy_itbl(itbls[i1]);
				fut->itblHandle[i1] = fut->itbl[i1]->handle;
				fut->itbl[i1]->size = 8;
			}
			for (i1 = 0; i1 < theMab->nOChan; i1++) {
				for (i = 0; i < theMab->nIChan; i++) {
					fut->chan[i1]->itbl[i] = fut->itbl[i];
					fut->chan[i1]->itblHandle[i] = fut->itblHandle[i];
				}
			}
		} else {
			for (i1 = 0; i1 < theMab->nOChan; i1++) {
				fut_free_otbl (fut->chan[i1]->otbl);	/* free the data */
				fut->chan[i1]->otbl = fut_copy_otbl(otbls[i1]);
				fut->chan[i1]->otblHandle = fut->chan[i1]->otbl->handle;
			}
		}
	}
	if (fut != NULL) {
		KpInt32_t	isTwoCubedILabCS;

		/* test for image Lab input and PCS Lab output with 2 grid points in each dimension */
		isTwoCubedILabCS = 0;
		if (((futHdr->spaceIn == KCM_IMAGE_LAB) ||(futHdr->spaceIn == KCM_RGB)) && (futHdr->spaceOut == KCM_CIE_LAB)) {
			KpInt32_t	i1;

			isTwoCubedILabCS = 1;

			for (i1 = 0; i1 < 3; i1++) {
				fut_itbl_p	itbl;
				fut_chan_p	chan;

				chan = fut->chan[i1];
				if ((! IS_CHAN(chan)) || (chan->imask != FUT_XYZ)) {	/* must have 3 output channels, each a function of 3 inputs */
					isTwoCubedILabCS = 0;
				}

				itbl = fut->itbl[i1];						/* every grid dimension must be 2 */
				if (! IS_ITBL(itbl) || (itbl->size != 2)) {
					isTwoCubedILabCS = 0;
				}
			}
		}

		if (isTwoCubedILabCS == 1) {
			PTDataClass_t	dataClass = KCP_FIXED_RANGE;
			fut_p			resizeFut, futResized;

			resizeFut = get_linlab_fut (KCP_LINLAB_GRID_SIZE, dataClass, dataClass);

			if (resizeFut != NULL) {
				futResized = fut_comp (fut, resizeFut, iomask);	/* resize the fut */
				fut_free (resizeFut);
				if (futResized != NULL) {
					fut_free (fut);			/* free small fut */
					fut = futResized;		/* use resized fut */
				}
			}
		}
		fut->lutConfig = theMab->lutConfig;		/* remember how we made it */

		/* save the extra tables */
		/* the first set is for the input respone curves */
		switch (theMab->lutConfig) {
		case MBA_B_MATRIX_M_COMBO:
		case MBA_B_MATRIX_M_CLUT_A_COMBO:
			srcTable = theMab->bTable;
			TblEntriesPtr = (KpInt32_p)theMab->bTblEntries;
			PTParaCurve = theMab->bPTParaCurve;
			break;

		case MAB_M_MATRIX_B_COMBO:
			srcTable = theMab->mTable;
			TblEntriesPtr = (KpInt32_p)theMab->mTblEntries;
			PTParaCurve = theMab->mPTParaCurve;
			break;

		default:
			TblEntriesPtr = 0;			/* no extra tables to convert */
			break;
		}
		if (TblEntriesPtr)
		{
			if (futHdr->spaceIn == KCM_CIE_LAB)
			{
				inputMap = KCP_V4LAB_TO_REF16;
			} else {
				inputMap = KCP_MAP_END_POINTS;
			}
			for (i1 = 0; i1 < FUT_NMCHAN; i1++) {
				srcTblEntries = destTblEntries = TblEntriesPtr[i1];
				if ((futHdr->spaceIn == KCM_CIE_LAB) ||
					(futHdr->spaceIn == KCM_CIE_XYZ))
				{
					if (destTblEntries < MF2_STD_ITBL_SIZE) {
						destTblEntries = MF2_STD_ITBL_SIZE;		/* use minimum size */
					}
				} else {
					if (destTblEntries < 64) {
						destTblEntries = 64;		/* use minimum size */
					}
				}
				fut->mabInTblEntries[i1] = 	destTblEntries;
				fut->mabInRefTbl[i1] = (mf2_tbldat_p) allocBufferPtr (destTblEntries * sizeof (mf2_tbldat_t));

				if (fut->mabInRefTbl[i1] != NULL) {
					fut->mabInRefTblHandles[i1] = getHandleFromPtr ((KpGenericPtr_t)fut->mabInRefTbl[i1]);
				}
				else {
					goto GetOut;
				}
				convert1DTable (srcTable[i1], sizeof (mab_tbldat_t), srcTblEntries, MF2_TBL_MAXVAL,
								fut->mabInRefTbl[i1], sizeof (mab_tbldat_t), destTblEntries, MF2_TBL_MAXVAL,
								inputMap, KCP_MAP_END_POINTS);
				
				fut->mabInParaCurve[i1] = PTParaCurve[i1];
			}
		}
		/* the second set is for the output respone curves */
		switch (theMab->lutConfig) {
		case MBA_B_MATRIX_M_COMBO:
			srcTable = theMab->mTable;
			TblEntriesPtr = (KpInt32_p)theMab->mTblEntries;
			PTParaCurve = theMab->mPTParaCurve;
			break;

		case MAB_M_MATRIX_B_COMBO:
		case MAB_A_CLUT_M_MATRIX_B_COMBO:
			srcTable = theMab->bTable;
			TblEntriesPtr = (KpInt32_p)theMab->bTblEntries;
			PTParaCurve = theMab->bPTParaCurve;
			break;

		default:
			TblEntriesPtr = 0;			/* no extra tables to convert */
			break;
		}
		if (TblEntriesPtr)
		{
			if (futHdr->spaceOut == KCM_CIE_LAB)
			{
				outputMap = KCP_V4LAB_TO_REF16;
			} else {
				outputMap = KCP_MAP_END_POINTS;
			}

			for (i1 = 0; i1 < FUT_NMCHAN; i1++) {
				srcTblEntries = destTblEntries = TblEntriesPtr[i1];
				if (destTblEntries < 256) {
					destTblEntries = 256;
				}
				fut->mabOutTblEntries[i1] = destTblEntries;
				fut->mabOutRefTbl[i1] = (mf2_tbldat_p) allocBufferPtr (destTblEntries * sizeof (mf2_tbldat_t));

				if (fut->mabOutRefTbl[i1] != NULL) {
					fut->mabOutRefTblHandles[i1] = getHandleFromPtr ((KpGenericPtr_t)fut->mabOutRefTbl[i1]);
				}
				else {
					goto GetOut;
				}

				convert1DTable (srcTable[i1], sizeof (mab_tbldat_t), srcTblEntries, MF2_TBL_MAXVAL,
								fut->mabOutRefTbl[i1], sizeof (mab_tbldat_t), destTblEntries, MF2_TBL_MAXVAL,
								KCP_MAP_END_POINTS, outputMap);
				
				fut->mabOutParaCurve[i1] = PTParaCurve[i1];
			}
		}
	}

GetOut:
	fut_free_tbls (FUT_NICHAN, (void *)itbls);
	fut_free_tbls (FUT_NOCHAN, (void *)gtbls);
	fut_free_tbls (FUT_NOCHAN, (void *)otbls);

	return fut;
}


static KpInt32_t
readMabCurveData(KpFd_p fd, KpUInt32_t nChan, KpUInt32_p TblEntriesPtr, mab_tbldat_p *TablePtr, PTParaCurve_p PTParaCurve)
{
	mcurve_t	curveType;
	KpInt32_t	nSig, nTblEntries, nTotalEntries, nTblSize, startOfCurves;
	KpUInt16_t	tmpTbl [MF2_MAX_TBL_ENT];
	KpInt32_t	status, cOffset;
	KpUInt32_t	i1;
	KpUInt8_t	dummy;

	Kp_get_position (fd, &startOfCurves);
	nTblEntries = 0;
	/* First go through the table headers to determine the total amount
	   of memory needed for all tables */
	for (i1 = 0; i1 < nChan; i1++)
	{
		status = Kp_read (fd, (KpGenericPtr_t) &curveType, CURVETYPE_HEADER);
		if (status != 1) {
			return (status);
		}		
#if (FUT_MSBF == 0)
		Kp_swab32 ((KpGenericPtr_t)&curveType.nSig, 1);
#endif
		if (CURVE_TYPE_SIG == curveType.nSig)
		{
#if (FUT_MSBF == 0)
			Kp_swab32 ((KpGenericPtr_t)&curveType.C.Curve.nCount, 1);
#endif
			if (curveType.C.Curve.nCount == 1) {	/* special case of gamma function */
				TblEntriesPtr[i1] = MFV_CURVE_TBL_ENT;
				nTblEntries += MFV_CURVE_TBL_ENT;
			}	else if (curveType.C.Curve.nCount == 0){	/* special case of linear w/ no data */
				TblEntriesPtr[i1] = 2;
				nTblEntries += 2;
			}	else {
				TblEntriesPtr[i1] = curveType.C.Curve.nCount;
				nTblEntries += curveType.C.Curve.nCount;
			}
			Kp_skip (fd, curveType.C.Curve.nCount * sizeof (mab_tbldat_t));	/* jump to the next curve header */
		} else if (PARA_TYPE_SIG == curveType.nSig){
			TblEntriesPtr[i1] = MFV_CURVE_TBL_ENT;
			nTblEntries += MFV_CURVE_TBL_ENT;
#if (FUT_MSBF == 0)
			Kp_swab32 ((KpGenericPtr_t)&curveType.C.Para.nFunction, 1);
#endif
			nTblSize = getNumParaParams(curveType.C.Para.nFunction);
			Kp_skip (fd, nTblSize * sizeof (Fixed_t));	/* jump to the next curve header */
		} else {
			/* illegal tag type - return error */
			return (-1);
		}
		/* move to next higher 4 byte boundary */
		Kp_get_position (fd, &cOffset);
		while ((cOffset & 3) != 0) {
			Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));
			cOffset++;
		}

	}

	/* Next, allocate enough memory to hold all tables */
	nTblSize = nTblEntries * sizeof (mab_tbldat_t);	/* size in bytes of each table */
	TablePtr[0] = allocBufferPtr (nTblSize);		/* get the needed memory for input */
	if (TablePtr[0] == NULL) {
		return (-1);
	}

	/* Now go back through the tables and read the data */
	Kp_set_position (fd, startOfCurves);
	nTotalEntries = 0;
	/* get the table data */
	for (i1 = 0; i1 < nChan; i1++)
	{
		TablePtr[i1] = TablePtr[0] + nTotalEntries;					/* cut up the buffer into the appropriate sizes */
		status = Kp_read (fd, (KpGenericPtr_t) &curveType, CURVETYPE_HEADER);
		if (status != 1) {
			return (status);
		}
		nSig = curveType.nSig;
#if (FUT_MSBF == 0)
		Kp_swab32 ((KpGenericPtr_t)&nSig, 1);
#endif
		PTParaCurve[i1].nSig = nSig;
		if (CURVE_TYPE_SIG == nSig)
		{
			nTblEntries = curveType.C.Curve.nCount;
#if (FUT_MSBF == 0)
			Kp_swab32 ((KpGenericPtr_t)&nTblEntries, 1);
#endif
			nTblSize = nTblEntries * sizeof (mab_tbldat_t);	/* size in bytes of each table */
			status = Kp_read (fd, (KpGenericPtr_t)tmpTbl, nTblSize);	/* read the input table */
			if (status != 1) {
				return (status);
			}		
#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)tmpTbl, nTblEntries);
#endif
			if (nTblEntries == 0) {	/* special case of linear data */
				*TablePtr[i1] = 0;
				*(TablePtr[i1] + 1) = 65535;
				nTblEntries = 2;
			} else if (nTblEntries == 1) {	/* special case of a gamma value */
				makeCurveFromPara (0, (Fixed_p)tmpTbl, TablePtr[i1], MFV_CURVE_TBL_ENT);
				nTblEntries = MFV_CURVE_TBL_ENT;
			}
			else {
				KpMemCpy (TablePtr[i1], tmpTbl, nTblSize);
			}
		} else {
			nTblEntries = MFV_CURVE_TBL_ENT;
#if (FUT_MSBF == 0)
			Kp_swab16 ((KpGenericPtr_t)&curveType.C.Para.nFunction, 1);
#endif
			PTParaCurve[i1].nFunction = curveType.C.Para.nFunction;
			nTblSize = getNumParaParams(PTParaCurve[i1].nFunction);
			status = Kp_read (fd, (KpGenericPtr_t)PTParaCurve[i1].nParams, nTblSize * sizeof (Fixed_t));	/* read the input table */
			if (status != 1) {
				return (status);
			}		
#if (FUT_MSBF == 0)
			Kp_swab32 ((KpGenericPtr_t)PTParaCurve[i1].nParams, nTblSize);
#endif
			makeCurveFromPara (PTParaCurve[i1].nFunction, PTParaCurve[i1].nParams, TablePtr[i1], MFV_CURVE_TBL_ENT);
		}
		nTotalEntries += nTblEntries;
		/* move to next higher 4 byte boundary */
		Kp_get_position (fd, &cOffset);
		while ((cOffset & 3) != 0) {
			Kp_read (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));
			cOffset++;
		}
	}
	return (status);
}


/* fut_writeMabFut_Kp writes a fut in the specified matrix fut format
 * to an open file descriptor.
 * Returns: 
 * 1 on success
 * 0 on invalid fut error
 * -1 on header, id string, or Kp_write error
 * -2 to -5 on a table specific error
 */
KpInt32_t
	fut_writeMabFut_Kp (KpFd_p		fd,
						fut_p		fut,
						fut_hdr_p	futHdr,
						KpInt32_t	MabFutType)
{
	KpInt32_t	status, imask, omask, tagType, gTblEntries;
	KpUInt32_t	dummy = 0;
	KpUInt32_t	bOffset = 0, matrixOffset = 0, mOffset = 0, clutOffset = 0, aOffset = 0;
	KpUInt8_t	i, inputChans, outputChans;
	KpUInt32_t	lutType, lutConfig;
	mclut_t		MabClutHeader;
	PTDataMap_t inputMap, outputMap;
	
	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "fut_writeMabFut_Kp\n MabFutType %x\n", MabFutType);
	kcpDiagLog (string);}
	#endif

	if ((PTTYPE_MAB1 == MabFutType) || (PTTYPE_MAB2 == MabFutType))
	{
		if (LUT_TYPE_UNKNOWN != fut->lutConfig)
		{
			if (MAB_LUT_TYPE && fut->lutConfig)
			{
				lutConfig = fut->lutConfig;
			} else {
				goto ErrOutM4;		/* this fut can not be made into a matrix fut */
			}
		} else {
			lutConfig = MAB_A_CLUT_B_COMBO;			/* set as the default */
		}
		lutType = PTTYPE_MA2B;
	} else if ((PTTYPE_MBA1 == MabFutType) || (PTTYPE_MBA2 == MabFutType))
	{
		if (LUT_TYPE_UNKNOWN != fut->lutConfig)
		{
			if (MBA_LUT_TYPE && fut->lutConfig)
			{
				lutConfig = fut->lutConfig;
			} else {
				goto ErrOutM4;		/* this fut can not be made into a matrix fut */
			}
		} else {
			lutConfig = MBA_B_CLUT_A_COMBO;			/* set as the default */
		}
		lutType = PTTYPE_MB2A;
	} else {
		goto ErrOutM1;
	}
	if ((PTTYPE_MAB1 == MabFutType) || (PTTYPE_MBA1 == MabFutType))
	{
		MabClutHeader.nPrecision = 1;		/* 1 byte per clut entry */
	} else {
		MabClutHeader.nPrecision = 2;		/* 2 bytes per clut entry */
	}
	MabClutHeader.nReserve0 = 0;		/* reserved bytes need to be aero */ 
	MabClutHeader.nReserve1 = 0;		/* reserved bytes need to be aero */
	MabClutHeader.nReserve2 = 0;		/* reserved bytes need to be aero */
	
	for (i = 0; i < ICC_TOTAL_CHANNELS; i++) {
		MabClutHeader.nDim[i] = 0;				/* zero out the Array */
	}

	/* input tables must be common and in first n contiguous input channels */
	imask = fut->iomask.in;							/* get the fut's input mask */
	for (inputChans = 0; inputChans < FUT_NICHAN; inputChans++, imask >>= 1) {
		if ( ! IS_ITBL(fut->itbl[inputChans]) || ((imask & 1) == 0)) {
			break;
		}
		
		MabClutHeader.nDim[inputChans] = (KpUInt8_t)fut->itbl[inputChans]->size;
	}
	if (imask != 0) {
		goto ErrOutM4;		/* this fut can not be made into a matrix fut */
	}
	gTblEntries = fut->chan[0]->gtbl->tbl_size / sizeof (fut_gtbldat_t);

	/* output tables must be in first n contiguous output channels */
	omask = fut->iomask.out;				/* get the fut's output mask */
	for (outputChans = 0; outputChans < FUT_NOCHAN; outputChans++, omask >>= 1) {
		if ( ! IS_CHAN(fut->chan[outputChans]) || ((omask & 1) == 0)) {
			break;
		}
	}
	if (omask != 0) {
		goto ErrOutM4;		/* this fut can not be made into a matrix fut */
	}

	tagType = lutType;

#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)&tagType, 1);	
#endif

	/* write out the common matrix fut stuff */
	status = Kp_write (fd, (KpGenericPtr_t)&tagType, sizeof(KpInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&inputChans, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&outputChans, sizeof(KpUInt8_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt16_t));

	if (status != 1) {
		goto GetOut;
	}

	if ((fut->itbl[0]->dataClass == KCP_VARIABLE_RANGE) ||
				(fut->itbl[0]->dataClass == KCP_XYZ_PCS) )
	{
		inputMap = KCP_REF16_TO_V4LAB;
	} else {
		inputMap = KCP_MAP_END_POINTS;
	}
	
	if ((fut->chan[0]->otbl->dataClass == KCP_VARIABLE_RANGE) ||
				(fut->chan[0]->otbl->dataClass == KCP_XYZ_PCS) )
	{
		outputMap = KCP_REF16_TO_V4LAB;
	} else {
		outputMap = KCP_MAP_END_POINTS;
	}	
	
	switch (lutConfig) {
	case MBA_B_CURVE_ONLY:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < inputChans; i++)
		{
			status = writeMabCurveData(fd, fut->itbl[i]->refTblEntries, fut->itbl[i]->refTbl,
											&fut->itbl[i]->ParaCurve, inputMap, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	case MAB_B_CURVE_ONLY:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < outputChans; i++)
		{
			status = writeMabCurveData(fd, fut->chan[i]->otbl->refTblEntries, fut->chan[i]->otbl->refTbl,
											&fut->chan[i]->otbl->ParaCurve, KCP_MAP_END_POINTS, outputMap);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	case MBA_B_MATRIX_M_COMBO:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < FUT_NMCHAN; i++)
		{
			status = writeMabCurveData(fd, fut->mabInTblEntries[i], (mab_tbldat_p)fut->mabInRefTbl[i],
											&fut->mabInParaCurve[i], inputMap, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&matrixOffset);
		writeMatrixData(fd, fut);
		Kp_get_position (fd, (KpInt32_p)&mOffset);
		for (i = 0; i < FUT_NMCHAN; i++)
		{
			status = writeMabCurveData(fd, fut->mabOutTblEntries[i], (mab_tbldat_p)fut->mabOutRefTbl[i],
											&fut->mabOutParaCurve[i], KCP_MAP_END_POINTS, outputMap);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	case MBA_B_CLUT_A_COMBO:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < inputChans; i++)
		{
			status = writeMabCurveData(fd, fut->itbl[i]->refTblEntries, fut->itbl[i]->refTbl,
											&fut->itbl[i]->ParaCurve, inputMap, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&clutOffset);
		writeClutData(fd, fut, outputChans, gTblEntries, &MabClutHeader);
		Kp_get_position (fd, (KpInt32_p)&aOffset);
		for (i = 0; i < outputChans; i++)
		{
			status = writeMabCurveData(fd, fut->chan[i]->otbl->refTblEntries, fut->chan[i]->otbl->refTbl,
											&fut->chan[i]->otbl->ParaCurve, KCP_MAP_END_POINTS, outputMap);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	case MBA_B_MATRIX_M_CLUT_A_COMBO:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < FUT_NMCHAN; i++)
		{
			status = writeMabCurveData(fd, fut->mabInTblEntries[i], (mab_tbldat_p)fut->mabInRefTbl[i],
											&fut->mabInParaCurve[i], inputMap, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&matrixOffset);
		writeMatrixData(fd, fut);
		Kp_get_position (fd, (KpInt32_p)&mOffset);
		for (i = 0; i < inputChans; i++)
		{
			status = writeMabCurveData(fd, fut->itbl[i]->refTblEntries, fut->itbl[i]->refTbl,
											&fut->itbl[i]->ParaCurve, KCP_MAP_END_POINTS, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&clutOffset);
		writeClutData(fd, fut, outputChans, gTblEntries, &MabClutHeader);
		Kp_get_position (fd, (KpInt32_p)&aOffset);
		for (i = 0; i < outputChans; i++)
		{
			status = writeMabCurveData(fd, fut->chan[i]->otbl->refTblEntries, fut->chan[i]->otbl->refTbl,
											&fut->chan[i]->otbl->ParaCurve, KCP_MAP_END_POINTS, outputMap);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	case MAB_M_MATRIX_B_COMBO:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < FUT_NMCHAN; i++)
		{
			status = writeMabCurveData(fd, fut->mabOutTblEntries[i], (mab_tbldat_p)fut->mabOutRefTbl[i],
											&fut->mabOutParaCurve[i], KCP_MAP_END_POINTS, outputMap);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&matrixOffset);
		writeMatrixData(fd, fut);
		Kp_get_position (fd, (KpInt32_p)&mOffset);
		for (i = 0; i < FUT_NMCHAN; i++)
		{
			status = writeMabCurveData(fd, fut->mabInTblEntries[i], (mab_tbldat_p)fut->mabInRefTbl[i],
											&fut->mabInParaCurve[i], inputMap, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	case MAB_A_CLUT_B_COMBO:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < outputChans; i++)
		{
			status = writeMabCurveData(fd, fut->chan[i]->otbl->refTblEntries, fut->chan[i]->otbl->refTbl,
											&fut->chan[i]->otbl->ParaCurve, KCP_MAP_END_POINTS, outputMap);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&clutOffset);
		writeClutData(fd, fut, outputChans, gTblEntries, &MabClutHeader);
		Kp_get_position (fd, (KpInt32_p)&aOffset);
		for (i = 0; i < inputChans; i++)
		{
			status = writeMabCurveData(fd, fut->itbl[i]->refTblEntries, fut->itbl[i]->refTbl,
											&fut->itbl[i]->ParaCurve, inputMap, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	case MAB_A_CLUT_M_MATRIX_B_COMBO:
		Kp_set_position (fd, MAB_FIRST_DATA_ENTRY);
		Kp_get_position (fd, (KpInt32_p)&bOffset);
		for (i = 0; i < FUT_NMCHAN; i++)
		{
			status = writeMabCurveData(fd, fut->mabOutTblEntries[i], (mab_tbldat_p)fut->mabOutRefTbl[i],
												&fut->mabOutParaCurve[i], KCP_MAP_END_POINTS, outputMap);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&matrixOffset);
		writeMatrixData(fd, fut);
		Kp_get_position (fd, (KpInt32_p)&mOffset);
		for (i = 0; i < outputChans; i++)
		{
			status = writeMabCurveData(fd, fut->chan[i]->otbl->refTblEntries, fut->chan[i]->otbl->refTbl,
										&fut->chan[i]->otbl->ParaCurve, KCP_MAP_END_POINTS, KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		Kp_get_position (fd, (KpInt32_p)&clutOffset);
		writeClutData(fd, fut, outputChans, gTblEntries, &MabClutHeader);
		Kp_get_position (fd, (KpInt32_p)&aOffset);
		for (i = 0; i < inputChans; i++)
		{
			status = writeMabCurveData(fd, fut->itbl[i]->refTblEntries, fut->itbl[i]->refTbl,
												&fut->itbl[i]->ParaCurve, inputMap  , KCP_MAP_END_POINTS);
			if (status != 1) {
				goto GetOut;
			}
		}
		break;

	default:
		status = -1;
		goto ErrOutM1;
		break;
	}

	/* write out the offsets for the matrix fut */
	Kp_set_position (fd, MAB_CURVE_OFFSETS);
#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)&bOffset, 1);
	Kp_swab32 ((KpGenericPtr_t)&matrixOffset, 1);
	Kp_swab32 ((KpGenericPtr_t)&mOffset, 1);
	Kp_swab32 ((KpGenericPtr_t)&clutOffset, 1);
	Kp_swab32 ((KpGenericPtr_t)&aOffset, 1);
#endif
	status = Kp_write (fd, (KpGenericPtr_t)&bOffset, sizeof(KpUInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&matrixOffset, sizeof(KpUInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&mOffset, sizeof(KpUInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&clutOffset, sizeof(KpUInt32_t)) &&
			Kp_write (fd, (KpGenericPtr_t)&aOffset, sizeof(KpUInt32_t));

	
GetOut:
	return status;

ErrOutM1:
	status = -1;
	goto GetOut;

ErrOutM4:
	status = -4;
	goto GetOut;

}

static KpInt32_t
writeMabCurveData(KpFd_p fd, KpUInt32_t nTblEntries, mab_tbldat_p TablePtr, 
					PTParaCurve_p PTParaCurve, PTDataMap_t inputMap, PTDataMap_t outputMap)
{
	mcurve_t	curveType;
	KpInt32_t	status, nTblSize, cOffset;
	KpUInt16_t	tmpTbl [MF2_MAX_TBL_ENT];
	Fixed_t		tmpParams[NUM_PARA_PARAMS];	/* parametric parameter list */
	KpUInt8_t	dummy = 0;

	nTblSize = nTblEntries * sizeof (mab_tbldat_t);	/* size in bytes of each table */
	/* write the table data */
	if (0 == PTParaCurve->nSig) {
		PTParaCurve->nSig = CURVE_TYPE_SIG;
	}
	curveType.nSig = PTParaCurve->nSig;
	curveType.nReserve = 0;
#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)&curveType.nSig, 1);
#endif
	if (CURVE_TYPE_SIG == PTParaCurve->nSig)
	{
		curveType.C.Curve.nCount = nTblEntries;
#if (FUT_MSBF == 0)
		Kp_swab32 ((KpGenericPtr_t)&curveType.C.Curve.nCount, 1);
#endif
		status = Kp_write (fd, (KpGenericPtr_t) &curveType, CURVETYPE_HEADER);
		if (status != 1) {
			return (status);
		}
		convert1DTable (TablePtr, sizeof (mf2_tbldat_t), nTblEntries, MF2_TBL_MAXVAL,
						tmpTbl, sizeof (mab_tbldat_t), nTblEntries, MF2_TBL_MAXVAL,
						inputMap, outputMap);

#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)tmpTbl, nTblEntries);
#endif
		status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, nTblSize);	/* write the table */
		if (status != 1) {
			return (status);
		}
	} else {
		curveType.C.Para.nFunction = PTParaCurve->nFunction;
		curveType.C.Para.nReserve = 0;
#if (FUT_MSBF == 0)
		Kp_swab16 ((KpGenericPtr_t)&curveType.C.Para.nFunction, 1);
		Kp_swab16 ((KpGenericPtr_t)&curveType.C.Para.nReserve, 1);
#endif
		status = Kp_write (fd, (KpGenericPtr_t) &curveType, CURVETYPE_HEADER);
		nTblSize = getNumParaParams(PTParaCurve->nFunction);
		KpMemCpy (PTParaCurve->nParams, tmpParams, nTblSize);
#if (FUT_MSBF == 0)
		Kp_swab32 ((KpGenericPtr_t)tmpParams, nTblSize);
#endif
		status = Kp_write (fd, (KpGenericPtr_t)tmpParams, nTblSize);	/* write the table */
		if (status != 1) {
			return (status);
		}		
	}
	Kp_get_position (fd, &cOffset);
	/* Pad with zeros until the next 4 byte boundary */
	while ((cOffset & 3) != 0) {
		Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));
		cOffset++;
	}
	return (status);
}


static KpInt32_t
writeMatrixData(KpFd_p fd, fut_p fut)
{
	KpInt32_t	status;
	KpUInt32_t	i1;
	Fixed_t		lMatrix[MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM];

		/* get the matrix to write */
	for (i1 = 0; i1 < (MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM); i1++) {
		lMatrix[i1] = fut->matrix[i1];
	}
	
#if (FUT_MSBF == 0)
	Kp_swab32 ((KpGenericPtr_t)lMatrix, MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM);
#endif

	/* write out the matrix */
	status = Kp_write (fd, (KpGenericPtr_t) lMatrix, sizeof(Fixed_t) * (MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM));
	
	return (status);
}

static KpInt32_t
writeClutData(KpFd_p fd, fut_p fut, KpUInt8_t outputChans, KpInt32_t gTblEntries, mclut_p MabClutHeader)
{
	mf2_tbldat_t	tmpTbl[MF2_MAX_TBL_ENT];
	mf2_tbldat_p	gDataP[FUT_NOCHAN];
	KpUInt8_p		mf1dataP;
	KpUInt16_p		mf2dataP;
	KpInt32_t		status, totalGSize;
	fut_chan_p		chan;
	KpInt32_t		i1, i2, outCount, outBytes, gData, cOffset;
	KpUInt8_t		nPrecision, dummy = 0;

	status = Kp_write (fd, (KpGenericPtr_t)MabClutHeader, sizeof(mclut_t));
	if (status != 1) {
		goto ErrOutM1;
	}
	nPrecision = MabClutHeader->nPrecision;
	
/* grid table conversion */
	for (outputChans = 0; outputChans < FUT_NOCHAN; outputChans++) {
		if ((chan = fut->chan[outputChans]) == NULL) {
			break;
		}
		gDataP[outputChans] = chan->gtbl->refTbl;	/* get each grid table pointer */
	}

	totalGSize = gTblEntries * outputChans * nPrecision;
	outCount = 0;												/* count bytes written to buffer */
	outBytes = calcNextGBufSize (GBUFFER_SIZE, &totalGSize);	/* set up for first write */

	mf1dataP = (KpUInt8_p) tmpTbl;
	mf2dataP = (KpUInt16_p) tmpTbl;
	
	for (i1 = 0; i1 < gTblEntries; i1++) {
		for (i2 = 0; i2 < (KpInt32_t)outputChans; i2++) {
			gData = (KpUInt32_t) *(gDataP[i2])++;	/* get each grid table entry */

			if (1 == nPrecision) {
				*mf1dataP++ = (KpUInt8_t)gData;
			}
			else {
#if (FUT_MSBF == 0)
				Kp_swab16 ((KpGenericPtr_t)&gData, 1);
#endif
				*mf2dataP++ = (KpUInt16_t) gData;
			}

			outCount += nPrecision;				/* count bytes in buffer */

			if (outCount == outBytes) {
				status = Kp_write (fd, (KpGenericPtr_t)tmpTbl, outBytes);
				if (status != 1) {
					goto ErrOutM1;
				}
				outBytes = calcNextGBufSize (outBytes, &totalGSize);	/* set up for next time */
				outCount = 0;											/* reset bytes written */
				mf1dataP = (KpUInt8_p) tmpTbl;
				mf2dataP = (KpUInt16_p) tmpTbl;
			}
		}
	}

	/* Make sure we pad with zeros to even 4 byte boundary */
	Kp_get_position (fd, &cOffset);
	while ((cOffset & 3) != 0) {
		Kp_write (fd, (KpGenericPtr_t)&dummy, sizeof(KpUInt8_t));
		cOffset++;
	}


ErrOutM1:
	return status;
}




