/*
 * @(#)fut.h	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)fut.h	1.8 99/02/04

	Contains:	Fut (function table) interface file.
				It is the only one needed for most applications.
 
	Author:	Kit Enscoe, George Pawle

	COPYRIGHT (c) 1989-1999 Eastman Kodak Company.
	As  an  unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#ifndef FUT_HEADER
#define FUT_HEADER

#include "kcms_sys.h"

#if defined (JAVACMM)
#define KCP_ICC_ONLY
#endif

#if defined(KPMSBFIRST)
#define FUT_MSBF 0xf
#endif

#if defined(KPLSBFIRST)
#define FUT_MSBF 0x0
#endif

#define MF_ID_MAKER(c1, c2, c3, c4) ((c1<<24)|(c2<<16)|(c3<<8)|(c4<<0))

#define FUT_IDSTR_MAX (16384-500)			/* max size of an ID string */

/* the current year is concatenated to FUT_COPYRIGHT_PREFIX and then 
	FUT_COPYRIGHT_SUFFIX is concatenated to that resultant string			*/
#define FUT_COPYRIGHT_PREFIX	"Copyright (c) Eastman Kodak Company, 1991-"
#define FUT_COPYRIGHT_SUFFIX	", all rights reserved."

/* Remember that FUT_COPYRIGHT_PREFIX + FUT_COPYRIGHT_SUFFIX +4 for current year
	cannot be greater than or equal to FUT_COPYRIGHT_MAX_LEN (256) */
#define FUT_COPYRIGHT_MAX_LEN 256
 
#define FUT_NCHAN		(8)				/* absolute maximum number of channels */
#define FUT_NICHAN		(8)				/* number of FUT input channels (X,Y,Z,T,U,V,W,S) */
#define FUT_NOCHAN		(8)				/* number of FUT output channels (X,Y,Z,T,U,V,W,S) */
#define FUT_NMCHAN		(3)				/* number of FUT matrix channels (X,Y,Z) */

#define FUT_INPTBL_ENT	(256)			/* # of input table entries */
#define FUT_INPTBL_ENT2 (4096)			/* # of input table entries */
#define FUT_INPTBL_ENT3 (65536)			/* # of input table entries -- 16 bit  */
#define FUT_OUTTBL_ENT	(4096)			/* # of output table entries */

#define FUT_GRD_BITS		(12)		/* # of bits in grid table entry */
#define FUT_GRD_MAXVAL		(4095)		/* max value of a grid table entry */
#define FUT_GRD_MAX_Z_DIM	(64)		/* max size of Z dimension */
#define FUT_GRD_MAX_Y_DIM	(64)		/* max size of Y dimension */
#define FUT_GRD_MAX_X_DIM	(64)		/* max size of X dimension */
#define FUT_GRD_MAX_T_DIM	(64)		/* max size of T dimension */
#define FUT_GRD_MAX_ENT		(FUT_GRD_MAX_X_DIM * FUT_GRD_MAX_Y_DIM * FUT_GRD_MAX_Z_DIM * FUT_GRD_MAX_T_DIM)	/* max entries in a grid table */
#define FUT_GRD_MAXDIM		(64)		/* max size of any given dimension */
#define FUT_INP_FRACBITS	(16)		/* # of bits in input table fraction */
#define FUT_INP_DECIMAL_PT	(16)		/* bit position of decimal pt in input table scaled integers */
#define FUT_OUT_INTBITS		(12)		/* # of bits in output table integer */
#define FUT_OUT_FRACBITS	(4)			/* # of bits in output table fraction */
#define FUT_OUT_DECIMAL_PT	(4)			/* bit position of decimal pt in output table scaled integers */
#define FUT_OUT_MAXVAL		(4095)		/* max value of an output table entry */
#define FUT_MAX_PEL8		(255)		/* maximum 8-bit pel value */
#define FUT_MAX_PEL12		(4080)		/* maximum 12-bit pel value */

/* extract integer and fractional parts of an input table entry */
#define FUT_ITBL_INTEG(i)		((i) >> FUT_INP_FRACBITS)
#define FUT_ITBL_FRAC(i)		((i) & ((1<<FUT_INP_FRACBITS)-1))

/* extract integer and fractional parts of an output table entry.	Also define
 * rounding constant and macro to compute "nearest integer" (instead of truncation)
 */
#define FUT_OTBL_INTEG(o)	((o) >> FUT_OUT_FRACBITS)
#define FUT_OTBL_FRAC(o)	((o) & ((1<<FUT_OUT_FRACBITS)-1))
#define FUT_OTBL_ROUNDUP	((KpInt32_t)1<<(FUT_OUT_FRACBITS-1))
#define FUT_OTBL_NINT(o)	(((o) > FUT_MAX_PEL12) ? (KpUInt8_t)FUT_MAX_PEL8 : \
								(KpUInt8_t)FUT_OTBL_INTEG ((o) + FUT_OTBL_ROUNDUP))

				/* matrix fut definitions */
typedef KpUInt8_t mf1_tbldat_t, FAR* mf1_tbldat_p;
typedef KpUInt16_t mf2_tbldat_t, FAR* mf2_tbldat_p;

				/* Mab/Mba matrix fut definitions */
typedef KpUInt16_t mab_tbldat_t, FAR* mab_tbldat_p;

typedef KpUInt8_t clut1_tbldat_t, FAR* clut1_tbldat_p;
typedef KpUInt16_t clut2_tbldat_t, FAR* clut2_tbldat_p;
typedef KpUInt16_t curve_tbldat_t, FAR* curve_tbldat_p;

#define MF_MATRIX_DIM		(3)			/* size of matrix dimension */
#define MF1_TBL_BITS		(sizeof (mf1_tbldat_t) * 8)	/* # of bits in 'mft1' input, grid, or output table entry */
#define MF2_TBL_BITS		(sizeof (mf2_tbldat_t) * 8)	/* # of bits in 'mft2' input, grid, or output table entry */
#define MF1_TBL_MAXVAL		((1 << MF1_TBL_BITS) -1)	/* max value of a 'mft1' grid table entry */
#define MF2_TBL_MAXVAL		((1 << MF2_TBL_BITS) -1)	/* max value of a 'mft2' grid table entry */
#define MF1_TBL_ENT			(1 << MF1_TBL_BITS)			/* # of table entries for 'mft1' input or output tables */
#define MF2_MIN_TBL_ENT		(2)			/* minimum # of table entries for 'mft2' input or output tables */
#define MF2_MAX_TBL_ENT		(4096)		/* maximum # of table entries for 'mft2' input or output tables */
#define MF_GRD_MAXDIM		(255)		/* max size of any given dimension */
#define MF2_STD_ITBL_SIZE	(515)		/* standard size for MF2 input and output tables */
#define MFV_STD_ITBL_SIZE	(256)		/* standard size for Mab & Mba input tables */
#define MFV_CURVE_TBL_ENT	(4096)		/* maximum # of table entries for parametric curve tables */

/* the following defines are to suppport the mab & mba lut types */
#define CURVETYPE_HEADER (12)
#define HAS_B_CURVE_DATA (0x1)
#define HAS_MATRIX_DATA (0x2)
#define HAS_M_CURVE_DATA (0x4)

#define CLUT_HEADER (20)
#define HAS_CLUT_DATA (0x8)
#define HAS_A_CURVE_DATA (0x10)

#define LUT_TYPE_UNKNOWN (0x0)
#define MAB_LUT_TYPE (0x10000)
#define MBA_LUT_TYPE (0x20000)

#define MBA_B_CURVE_ONLY			(MBA_LUT_TYPE | HAS_B_CURVE_DATA)
#define MBA_B_CLUT_A_COMBO			(MBA_LUT_TYPE | HAS_B_CURVE_DATA | HAS_CLUT_DATA | HAS_A_CURVE_DATA)
#define MBA_B_MATRIX_M_COMBO		(MBA_LUT_TYPE | HAS_B_CURVE_DATA | HAS_MATRIX_DATA | HAS_M_CURVE_DATA)
#define MBA_B_MATRIX_M_CLUT_A_COMBO	(MBA_LUT_TYPE | HAS_B_CURVE_DATA | HAS_MATRIX_DATA | HAS_M_CURVE_DATA | HAS_CLUT_DATA | HAS_A_CURVE_DATA)

#define MAB_B_CURVE_ONLY			(MAB_LUT_TYPE | HAS_B_CURVE_DATA)
#define MAB_M_MATRIX_B_COMBO		(MAB_LUT_TYPE | HAS_M_CURVE_DATA | HAS_MATRIX_DATA | HAS_B_CURVE_DATA)
#define MAB_A_CLUT_B_COMBO			(MAB_LUT_TYPE | HAS_A_CURVE_DATA | HAS_CLUT_DATA | HAS_B_CURVE_DATA)
#define MAB_A_CLUT_M_MATRIX_B_COMBO	(MAB_LUT_TYPE | HAS_A_CURVE_DATA | HAS_CLUT_DATA | HAS_M_CURVE_DATA | HAS_MATRIX_DATA | HAS_B_CURVE_DATA)

				/* channel definitions */
#define FUT_XCHAN		(0)				/* X channel # */
#define FUT_YCHAN		(1)				/* Y channel # */
#define FUT_ZCHAN		(2)				/* Z channel # */
#define FUT_TCHAN		(3)				/* T channel # */
#define FUT_UCHAN		(4)				/* U channel # */
#define FUT_VCHAN		(5)				/* V channel # */
#define FUT_WCHAN		(6)				/* W channel # */
#define FUT_SCHAN		(7)				/* S channel # */
#define FUT_BIT(x)		((KpUInt32_t)1<<x)
#define FUT_CHAN(x)	(fut_first_chan(x))
#define FUT_X			FUT_BIT(FUT_XCHAN)
#define FUT_Y			FUT_BIT(FUT_YCHAN)
#define FUT_Z			FUT_BIT(FUT_ZCHAN)
#define FUT_T			FUT_BIT(FUT_TCHAN)
#define FUT_U			FUT_BIT(FUT_UCHAN)
#define FUT_V			FUT_BIT(FUT_VCHAN)
#define FUT_W			FUT_BIT(FUT_WCHAN)
#define FUT_S			FUT_BIT(FUT_SCHAN)
#define FUT_XY			(FUT_X|FUT_Y)
#define FUT_XYZ			(FUT_X|FUT_Y|FUT_Z)
#define FUT_XYZT		(FUT_X|FUT_Y|FUT_Z|FUT_T)
#define FUT_XYZTU		(FUT_X|FUT_Y|FUT_Z|FUT_T|FUT_U)
#define FUT_XYZTUV		(FUT_X|FUT_Y|FUT_Z|FUT_T|FUT_U|FUT_V)
#define FUT_XYZTUVW		(FUT_X|FUT_Y|FUT_Z|FUT_T|FUT_U|FUT_V|FUT_W)
#define FUT_XYZTUVWS	(FUT_X|FUT_Y|FUT_Z|FUT_T|FUT_U|FUT_V|FUT_W|FUT_S)
#define FUT_ALL			((KpUInt32_t)(1<<FUT_NCHAN)-1)
#define FUT_ALLIN		((KpUInt32_t)(1<<FUT_NICHAN)-1)
#define FUT_ALLOUT		((KpUInt32_t)(1<<FUT_NOCHAN)-1)

				/* iomask bit-field extraction macros */
#define FUT_IMASK(x)	((KpUInt32_t)((x)>>0) & 0xff)		/* input channel mask */
#define FUT_OMASK(x)	((KpUInt32_t)((x)>>8) & 0xff)		/* output channel mask */
#define FUT_PMASK(x)	((KpUInt32_t)((x)>>16) & 0xff)		/* pass-thru channel mask */
#define FUT_ORDMASK(x)	((KpUInt32_t)((x)>>24) & 0xf)		/* interp. order mask */
#define FUT_IPMASK(x)	((KpUInt32_t)((x)>>28) & 0x1)		/* "inplace" bit */
#define FUT_12BMASK(x)	((KpUInt32_t)((x)>>30) & 0x1)		/* 12 bit datum */

				/* iomask bit-field insertion macros */
#define FUT_IN(x)		((KpUInt32_t)((x) & 0xff)<<0)
#define FUT_OUT(x)		((KpUInt32_t)((x) & 0xff)<<8)
#define FUT_PASS(x)		((KpUInt32_t)((x) & 0xff)<<16)
#define FUT_ORDER(x)	((KpUInt32_t)((x) & 0xf)<<24)
#define FUT_INPLACE		((KpUInt32_t)1<<28)
#define FUT_12BITS		((KpUInt32_t)1<<30)

				/* interpolation orders */
#define FUT_DEFAULT	(0)			/* linear or order of fut */
#define FUT_NEAREST	(1)			/* nearest-neighbor */
#define FUT_LINEAR	(2)			/* linear (bilinear, trilinear, etc) */
#define FUT_CUBIC	(3)			/* 4x4x4, etc cubic convolution */

#define FUT_EVAL_BUFFER_SIZE 256	/* bytes per component buffer for format general */

/* data scaling classs */
typedef enum PTDataCLass_e {
	KCP_UNKNOWN			= 0,
	KCP_FIXED_RANGE		= 1,
	KCP_VARIABLE_RANGE	= 2,
	KCP_XYZ_PCS			= 3
} PTDataClass_t, FAR* PTDataCLass_p;

/* these factors are used to convert between 8-bit and 16-bit encodings of uvL, Lab, etc. */
/* #define KCP_MAP_ENDPOINT_REF	*/			/* Fixed Range (image Lab or Lab8) for internal Lab PCS */
#define KCP_MAP_BASE_MAX_REF				/* Variable Range (Legacy Lab) for internal Lab PCS */

/* color space data classes */
#if defined KCP_MAP_ENDPOINT_REF
#define KCP_LAB_PCS		KCP_FIXED_RANGE
#define KCP_LUV_DATA	KCP_FIXED_RANGE
#define KCP_RCS_DATA	KCP_FIXED_RANGE
#elif defined KCP_MAP_BASE_MAX_REF
#define KCP_LAB_PCS		KCP_VARIABLE_RANGE
#define KCP_LUV_DATA	KCP_VARIABLE_RANGE
#define KCP_RCS_DATA	KCP_VARIABLE_RANGE
#else
#error Undefined Lab PCS space
#endif

/* data mapping modes */
typedef enum PTDataMap_e {
	KCP_MAP_END_POINTS		= 1,
	KCP_MAP_BASE_MAX		= 2,
	KCP_REF16_TO_BASE_MAX	= 3,
	KCP_BASE_MAX_TO_REF16	= 4,
	KCP_V4LAB_TO_REF16		= 5,
	KCP_REF16_TO_V4LAB		= 6
} PTDataMap_t, FAR* PTDataMap_p;

/* table data types */
typedef enum PTTableType_e {
	KCP_PT_TABLES	= 1,
	KCP_REF_TABLES	= 2
} PTTableType_t, FAR* PTTableType_p;

typedef struct fut_iomask_s {
#if (FUT_MSBF == 0xF)
	unsigned int	funcmod : 4;	/* function modifiers (see above) */
	unsigned int	order	: 4;	/* interpolation order */
	unsigned int	pass	: 8;	/* pass channels */
	unsigned int	out		: 8;	/* output channels */
	unsigned int	in		: 8;	/* input channels */
#else
#if (FUT_MSBF == 0x0)
	unsigned int	in		: 8;	/* input channels */
	unsigned int	out		: 8;	/* output channels */
	unsigned int	pass	: 8;	/* pass channels */
	unsigned int	order	: 4;	/* interpolation order */
	unsigned int	funcmod : 4;	/* function modifiers (see above) */

#else
===	/* Unsupported byte ordering - cause compiler error */
#endif /* (FUT_MSBF == 0x0) */
#endif /* (FUT_MSBF == 0xF) */
} fut_iomask_t;

#define	NUM_PARA_PARAMS		(7)
/* Parametric equations for Mab Lut data types */
typedef struct PTParaCurve_s {
	KpInt32_t	nSig;						/* Type signature */
	KpUInt16_t	nFunction;					/* Function value */
	Fixed_t		nParams[NUM_PARA_PARAMS];	/* Parameter list */
} PTParaCurve_t, FAR* PTParaCurve_p;


/* Structure defining an input table.	The table data (array) is allocated separately.
 *
 * NOTE: the last (257th) table entry is used to perform automatic clipping
 *		when evaluating 12-bit values.	It should always be identical to the
 *		previous (256th) entry.
 */
typedef KpInt32_t fut_itbldat_t, FAR* fut_itbldat_p, FAR* FAR* fut_itbldat_h;

typedef struct fut_itbl_s {
	KpInt32_t		magic;			/* magic number for runtime checking */
	KpInt32_t		ref;			/* reference count (<0 => external) */
	KpInt32_t		id;				/* unique id number */
	KpInt32_t		size;			/* size of grid in this dimension */
	fut_itbldat_p	tbl;			/* pointer to FUT_INPTBL_ENT+1 entries */
	KpHandle_t		tblHandle;		/* and its memory handle */
	KpHandle_t		handle;			/* memory handle of this struct */
	PTDataClass_t	dataClass;		/* class of data input to this table */
	KpInt32_t		refTblEntries;	/* # entries in one reference itbl */
	mf2_tbldat_p	refTbl;			/* pointer to source itbl data */
	KpHandle_t		refTblHandle;	/* and its memory handle */
	PTParaCurve_t	ParaCurve;		/* Parametric curve */
} fut_itbl_t, FAR* fut_itbl_p;


/* Structure defining a grid table.	The table data (array) is allocated separately.
 */
typedef KpUInt16_t fut_gtbldat_t, FAR* fut_gtbldat_p, FAR* FAR* fut_gtbldat_h;

typedef struct fut_gtbl_s {
	KpInt32_t		magic;				/* magic number for runtime checking */
	KpInt32_t		ref;				/* reference count (<0 => external) */
	KpInt32_t		id;					/* unique id number */
	fut_gtbldat_p	tbl;				/* pointer to grid table data */
	KpHandle_t		tblHandle;			/* and its handle */
	KpInt32_t		tbl_size;			/* size of tbl(in bytes) */
	KpInt16_t		size[FUT_NCHAN];	/* grid table dimensions */
	KpHandle_t		handle;				/* memory handle of this struct */
	mf2_tbldat_p	refTbl;				/* pointer to source itbl data */
	KpHandle_t		refTblHandle;		/* and its memory handle */
} fut_gtbl_t, FAR* fut_gtbl_p;


/* Structure defining an output table.	The table data (array) is allocated separately.
 */
typedef KpUInt16_t fut_otbldat_t, FAR* fut_otbldat_p, FAR* FAR* fut_otbldat_h;

typedef struct fut_otbl_s {
	KpInt32_t		magic;			/* magic number for runtime checking */
	KpInt32_t		ref;			/* reference count (<0 => external) */
	KpInt32_t		id;				/* unique id number */
	fut_otbldat_p	tbl;			/* pointer to FUT_OUTTBL_ENT entries */
	KpHandle_t		tblHandle;		/* and its memory handle */
	KpHandle_t		handle;			/* memory handle of this struct */
	PTDataClass_t	dataClass;		/* class of data output from this table */
	KpInt32_t		refTblEntries;	/* # entries in one reference otbl */
	mf2_tbldat_p	refTbl;			/* pointer to source otbl data */
	KpHandle_t		refTblHandle;	/* and its memory handle */
	PTParaCurve_t	ParaCurve;		/* Parametric curve */
} fut_otbl_t, FAR* fut_otbl_p;


/* Structure defining a single output channel of a fut.
 * Each fut may have up to FUT_NOCHAN of these.
 */
typedef struct fut_chan_s {
	KpInt32_t		magic;					/* magic number for runtime checking */
	KpInt32_t		imask;					/* input mask for this channel */
	fut_gtbl_p		gtbl;					/* grid table */
	KpHandle_t		gtblHandle;				/* and its memory handle */
	fut_otbl_p		otbl;					/* optional output table */
	KpHandle_t		otblHandle;				/* and its memory handle */
	fut_itbl_p		itbl[FUT_NICHAN];		/* input tables */
	KpHandle_t		itblHandle[FUT_NICHAN];	/* and their memory handles */
	KpHandle_t		handle;					/* memory handle of this struct */
} fut_chan_t, FAR* fut_chan_p;


/* Structure defining a fut. */
typedef struct fut_s {
	KpInt32_t		magic;					/* magic number for runtime checking */
	KpChar_p		idstr;					/* optional id string */
	fut_iomask_t	iomask;					/* input/output mask for fut */
	fut_itbl_p		itbl[FUT_NICHAN];		/* input tables common to all chans */
	KpHandle_t		itblHandle[FUT_NICHAN]; /* and their memory handles */
	fut_chan_p		chan[FUT_NOCHAN];		/* output channels */
	KpHandle_t		chanHandle[FUT_NOCHAN]; /* and their memory handles */
	KpHandle_t		handle;					/* memory handle of this struct */
	KpInt32_t		refNum;					/* fut caching tag */
	KpInt32_t		modNum;					/* modification number of this fut */
	KpUInt32_t		lutConfig;				/* type of lut used to create this fut */
	Fixed_t			matrix[MF_MATRIX_DIM * MF_MATRIX_DIM + MF_MATRIX_DIM];
	KpInt32_t		mabInTblEntries[FUT_NMCHAN];		/* # entries in one reference mtbl */
	mf2_tbldat_p	mabInRefTbl[FUT_NMCHAN];			/* pointer to source mtbl data */
	KpHandle_t		mabInRefTblHandles[FUT_NMCHAN];		/* and its memory handle */
	PTParaCurve_t	mabInParaCurve[FUT_NMCHAN];			/* Parametric curve */
	KpInt32_t		mabOutTblEntries[FUT_NMCHAN];		/* # entries in one reference mtbl */
	mf2_tbldat_p	mabOutRefTbl[FUT_NMCHAN];			/* pointer to source mtbl data */
	KpHandle_t		mabOutRefTblHandles[FUT_NMCHAN];	/* and its memory handle */
	PTParaCurve_t	mabOutParaCurve[FUT_NMCHAN];		/* Parametric curve */
} fut_t, FAR* fut_p;


/* I/O codes used in fut_hdr_t (below) for encoding tables.
 * There are two fields in an io code, the "code" and the "data".
 * The data field is used only for FUTIO_SHARED and FUTIO_RAMP in
 * which case it gives the channel number of the table to be shared
 * or the grid size for a ramp input table respectively.
 */
 
#define FUTIO_DATA		(0x0ffff)	/* mask for code data */
#define FUTIO_CODE		(0xf0000)	/* mask for code number */
#define FUTIO_NULL		(0x00000)	/* table is null */
#define FUTIO_SHARED	(0x10000)	/* table is shared (data=chan#) */
#define FUTIO_RAMP		(0x20000)	/* table is a ramp (data=size) */
#define FUTIO_UNIQUE	(0x30000)	/* table is unique */

/* Fixed header for fut files.	For backwards compatibility, this
 * structure should never increase in size.
 */
typedef struct chan_hdr_s {			/* codes for each channel: */
	KpInt16_t	size[FUT_NCHAN];	/* grid table dimensions */
	KpInt32_t	icode[FUT_NCHAN];	/* input table codes */
	KpInt32_t	ocode;				/* output table code */
	KpInt32_t	gcode;				/* grid table code */
} chan_hdr_t, FAR* chan_hdr_p;

#define KCP_PT_HEADER_SIZE (500)	/* PT duplicates first 500 bytes of a fut */

typedef struct fut_hdr_s {
	KpInt32_t		magic;				/* = FUT_MAGIC for postive file id */
	KpInt32_t		version;			/* allow for future modifications */
	KpInt32_t		idstr_len;			/* strlen(fut->idstr)+1 or 0 */
	KpInt32_t		order;				/* interpolation order */
	KpInt32_t		icode[FUT_NCHAN];	/* codes for common input tables */
	chan_hdr_t		chan[FUT_NCHAN];	/* codes for each channel: */
	KpInt32_t		more;				/* more data follows if TRUE */
	KpInt32_t		srcFormat;			/* format of source data */
	PTDataClass_t	iDataClass;
	PTDataClass_t	oDataClass;
	KpUInt32_t		profileType;		/* profile type */
	KpUInt32_t		spaceIn;			/* input color space */
	KpUInt32_t		spaceOut;			/* output color space */
} fut_hdr_t, FAR* fut_hdr_p;


/* table data free modes */
typedef enum fut_freeMode_e {
	freeTable = 0,		/* free table structure always frees data */
	freeData = 1		/* free data frees only if another table data exists */
} fut_freeMode_t, FAR* fut_freeMode_p;

/* table generation function data */
typedef struct fut_calcData_s {
	KpInt32_t	chan;
} fut_calcData_t, FAR* fut_calcData_p;


#if !defined KCP_NO_ORIG_FUT_TYPEDEFS
typedef fut_itbl_p fut_itbl_ptr_t;
typedef fut_itbldat_p fut_itbldat_ptr_t;
typedef fut_gtbl_p fut_gtbl_ptr_t;
typedef fut_gtbldat_p fut_gtbldat_ptr_t;
typedef fut_otbl_p fut_otbl_ptr_t;
typedef fut_otbldat_p fut_otbldat_ptr_t;
typedef fut_chan_p fut_chan_ptr_t;
typedef fut_p fut_ptr_t;
typedef KpGenericPtr_t fut_generic_ptr_t;
#define fut_far FAR
#endif

typedef double (*fut_ifunc_t)(double, fut_calcData_p);
typedef fut_ifunc_t FAR* fut_ifunc_p;
typedef double (*fut_gfunc_t)(double FAR*, fut_calcData_p);
typedef fut_gfunc_t FAR* fut_gfunc_p;
typedef double (*fut_ofunc_t)(double, fut_calcData_p);
typedef fut_ofunc_t FAR* fut_ofunc_p;

/* handle dereferencing */
#if defined KPMAC
#define FFUTP(handle) (*(fut_p FAR*)handle)
#define FCHANP(handle) (*(fut_chan_p FAR*)handle)
#define FITBLP(handle) (*(fut_itbl_p FAR*)handle)
#define FGTBLP(handle) (*(fut_gtbl_p FAR*)handle)
#define FOTBLP(handle) (*(fut_otbl_p FAR*)handle)
#else
#define FFUTP(handle) ((fut_p)handle)
#define FCHANP(handle) ((fut_chan_p)handle)
#define FITBLP(handle) ((fut_itbl_p)handle)
#define FGTBLP(handle) ((fut_gtbl_p)handle)
#define FOTBLP(handle) ((fut_otbl_p)handle)
#endif


#if defined (__cplusplus) || defined(SABR)
extern "C" {
#endif

#if !defined(KPNONANSIC)	/* these function prototypes are defined for ansi C compilers only */
fut_p	fut_free			(fut_p);
fut_p	fut_free_futH 		(KpHandle_t);
void	fut_free_chan		(fut_chan_p);
void	fut_free_itbl		(fut_itbl_p);
void	fut_free_otbl		(fut_otbl_p);
void	fut_free_gtbl		(fut_gtbl_p);
void	fut_free_tbldat		(fut_p);
void	fut_free_itbldat	(fut_itbl_p, fut_freeMode_t);
void	fut_free_gtbldat	(fut_gtbl_p, fut_freeMode_t);
void	fut_free_otbldat	(fut_otbl_p, fut_freeMode_t);
void	fut_free_mftdat		(fut_p);
void	fut_free_imftdat	(fut_itbl_p, fut_freeMode_t);
void	fut_free_gmftdat	(fut_gtbl_p, fut_freeMode_t);
void	fut_free_omftdat	(fut_otbl_p, fut_freeMode_t);
void	fut_free_tbl		(KpGenericPtr_t);
void	fut_free_tbls		(KpInt32_t, KpGenericPtr_t FAR*);

KpInt32_t	fut_to_mft (fut_p);		/* convert fut to mft format */
KpInt32_t	mft_to_fut (fut_p);		/* convert mft to fut format */
KpInt32_t	makeMftTblDat (fut_p);	/* make mft table data from fut table data */
KpInt32_t	makeMftiTblDat (fut_itbl_p);
KpInt32_t	makeFutTblDat (fut_p);	/* make fut table data from mft table data */
KpInt32_t	makeFutiTblDat (fut_itbl_p);

fut_p		fut_new			(KpInt32_t, fut_itbl_p FAR*, fut_gtbl_p FAR*, fut_otbl_p FAR*);
fut_itbl_p	fut_new_itblEx	(PTTableType_t, PTDataClass_t, KpInt32_t, fut_ifunc_t, fut_calcData_p);
fut_gtbl_p	fut_new_gtblEx	(PTTableType_t, KpInt32_t, fut_gfunc_t, fut_calcData_p, KpInt32_p);
fut_otbl_p	fut_new_otblEx	(PTTableType_t, PTDataClass_t, fut_ofunc_t, fut_calcData_p);
fut_chan_p	fut_new_chan	(KpInt32_t, fut_itbl_p FAR*, fut_gtbl_p, fut_otbl_p);
KpInt32_t	fut_defchan		(fut_p,KpInt32_t, fut_itbl_p FAR*, fut_gtbl_p, fut_otbl_p);
KpInt32_t	fut_add_chan	(fut_p,KpInt32_t,fut_chan_p);

fut_p	fut_new_empty	(KpInt32_t, KpInt32_p, KpInt32_t, PTDataClass_t, PTDataClass_t);
fut_p	constructfut	(KpInt32_t, KpInt32_p, fut_calcData_p,
						fut_ifunc_p, fut_gfunc_p, fut_ofunc_p, PTDataClass_t, PTDataClass_t);
fut_p	fut_resize		(fut_p, KpInt32_p);

fut_p		fut_share		(fut_p);
fut_chan_p	fut_share_chan	(fut_chan_p);
fut_itbl_p	fut_share_itbl	(fut_itbl_p);
fut_gtbl_p	fut_share_gtbl	(fut_gtbl_p);
fut_otbl_p	fut_share_otbl	(fut_otbl_p);

KpInt32_t	fut_is_separable	(fut_p);

KpInt32_t	fut_get_itbl (fut_p, KpInt32_t, KpInt32_t, fut_itbldat_p*);
KpInt32_t	fut_get_gtbl (fut_p, KpInt32_t, fut_gtbldat_p*);
KpInt32_t	fut_get_otbl (fut_p, KpInt32_t, fut_otbldat_p*);

KpInt32_t	is_23_splitable	(fut_p);
KpInt32_t	fut_split23		(fut_p, fut_p FAR*, fut_p FAR*);

KpInt32_t	convert1DTable (KpGenericPtr_t, KpInt32_t, KpInt32_t, KpUInt32_t, KpGenericPtr_t,
							KpInt32_t, KpInt32_t, KpUInt32_t, PTDataMap_t, PTDataMap_t);

KpInt32_t	fut_calc_itblEx	(fut_itbl_p, fut_ifunc_t, fut_calcData_p);
KpInt32_t	fut_calc_otblEx	(fut_otbl_p, fut_ofunc_t, fut_calcData_p);
KpInt32_t	fut_calc_gtblEx	(fut_gtbl_p, fut_gfunc_t, fut_calcData_p);

KpInt32_t	evaluateFut (fut_p, KpUInt32_t, KpUInt32_t, KpInt32_t, KpGenericPtr_t FAR*, KpGenericPtr_t FAR*);

fut_p fut_comp			(fut_p, fut_p, KpInt32_t);
fut_p fut_comp_itbl		(fut_p, fut_p, KpInt32_t);
fut_p fut_comp_ilut		(fut_p,KpInt32_t,KpGenericPtr_t FAR*);
fut_p fut_comp_otbl		(fut_p, fut_p, KpInt32_t);

KpInt32_t	fut_cmp			(fut_p,fut_p,double);
KpInt32_t	fut_cmp_chan	(fut_chan_p,fut_chan_p,double);
KpInt32_t	fut_cmp_itbl	(fut_itbl_p,fut_itbl_p,double);
KpInt32_t	fut_cmp_otbl	(fut_otbl_p,fut_otbl_p,double);
KpInt32_t	fut_cmp_gtbl	(fut_gtbl_p,fut_gtbl_p,double);

fut_p		fut_copy		(fut_p);
fut_chan_p	fut_copy_chan	(fut_chan_p);
fut_itbl_p	fut_copy_itbl	(fut_itbl_p);
fut_otbl_p	fut_copy_otbl	(fut_otbl_p);
fut_gtbl_p	fut_copy_gtbl	(fut_gtbl_p);

KpInt32_t	fut_iomask	(KpChar_t *);

fut_p		fut_load_fp		(KpChar_p, KpFileProps_t);
KpInt32_t	fut_store		(fut_p, KpChar_p);
KpInt32_t	fut_store_fp	(fut_p, KpChar_p, KpFileProps_t);
fut_p		fut_read		(KpInt32_t);
fut_p		fut_read_Kp 	(KpFd_p fd);
KpInt32_t	fut_write		(KpInt32_t, fut_p);
KpInt32_t	fut_write_Kp 	(KpFd_p fd, fut_p fut);

KpInt32_t	mf_store_fp			(fut_p, KpChar_p, KpFileProps_t, KpInt32_t);
KpInt32_t	fut_writeMFut_Kp	(KpFd_p, fut_p, Fixed_p, KpInt32_t);

KpInt32_t	fut_io_encode (fut_p, fut_hdr_p);
KpInt32_t	fut_io_decode (fut_p, fut_hdr_p);

KpInt32_t	fut_write_hdr	(KpFd_p, fut_hdr_p);
KpInt32_t	fut_read_hdr	(KpFd_p, fut_hdr_p);
KpInt32_t	fut_read_futhdr (KpFd_p, fut_hdr_p);

KpInt32_t	fut_write_tbls	(KpFd_p, fut_p, fut_hdr_p);
KpInt32_t	fut_read_tbls	(KpFd_p, fut_p, fut_hdr_p);

KpInt32_t	isIdentityMatrix	(Fixed_p, KpInt32_t);

KpInt32_t	fut_readMFutHdr		(KpFd_p, fut_hdr_p);
fut_p		fut_readMFutTbls	(KpFd_p, fut_hdr_p, Fixed_p);

KpInt32_t	fut_readMabFutHdr		(KpFd_p, fut_hdr_p);
fut_p		fut_readMabFutTbls	(KpFd_p, fut_hdr_p, Fixed_p);
KpInt32_t	fut_writeMabFut_Kp	(KpFd_p, fut_p, fut_hdr_p, KpInt32_t);

void 		fut_swab_hdr	(fut_hdr_p);
void 		fut_swab_itbl	(fut_itbl_p);
void 		fut_swab_otbl	(fut_otbl_p);
void 		fut_swab_gtbl	(fut_gtbl_p);

#if !defined KCMS_NO_CRC
KpInt32_t	fut_cal_crc		(fut_p, KpInt32_p);
#endif

/* initialization functions - may be passed to fut_calc_tbl routines */
double		fut_irampEx		(double, fut_calcData_p);
double		fut_grampEx		(double_p, fut_calcData_p);
double		fut_orampEx		(double, fut_calcData_p);

/* idstring stuff */
KpInt32_t	fut_new_idstr	(fut_p, KpChar_p);
KpInt32_t	fut_set_idstr	(fut_p, KpChar_p);
KpChar_p	fut_idstr		(fut_p);
KpInt32_t	fut_make_copyright (KpChar_p);

/* fut handle stuff */
fut_p		fut_lock_fut	(KpHandle_t);
KpHandle_t	fut_unlock_fut	(fut_p);

#endif	/* end of ansi C prototypes */

#if defined (__cplusplus)
}
#endif

#define FUT_NULL_IFUNEX	((fut_ifunc_t) NULL)
#define FUT_NULL_CHAN	((fut_chan_p)NULL)
#define FUT_NULL		((fut_p)NULL)
#define FUT_NULL_ITBL	((fut_itbl_p)NULL)
#define FUT_NULL_ITBLDAT ((fut_itbldat_p)NULL)
#define FUT_NULL_OTBL	((fut_otbl_p)NULL)
#define FUT_NULL_OTBLDAT ((fut_otbldat_p)NULL)
#define FUT_NULL_GTBL	((fut_gtbl_p)NULL)
#define FUT_NULL_GTBLDAT ((fut_gtbldat_p)NULL)
#define FUT_NULL_HANDLE ((KpHandle_t)NULL)

#endif /*	FUT_HEADER */
