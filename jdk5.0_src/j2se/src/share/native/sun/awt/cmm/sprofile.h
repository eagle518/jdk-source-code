/*
 * @(#)sprofile.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*********************************************************************/
/* @(#)sprofile.h	1.162 99/02/24
	Contains:	This header defines the Public interface to the Profile
				Processor.

				Created by lsh, September 15, 1993

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1993-2003
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#ifndef SPROFILE_H
#define SPROFILE_H

#if defined(__cplusplus)
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


#include <stdlib.h>

#include "kcmsos.h"

#if defined (KPWIN32) && !defined (_WIN64)
#pragma pack(push, SpLevel, 4)
#endif


#if defined (KPMAC) || defined (KPMSMAC)
#include <folders.h>
#define KSPAPI
#define SPCALLBACK
#define SPNEAR
#define COLORSYNCSTR	7289

#if defined (KPMACPPC)
#pragma options align=mac68k
#endif

#elif defined (KPWIN16)
#define KSPAPI		FAR PASCAL
#define SPCALLBACK	FAR PASCAL
#define SPNEAR		_near

#elif defined (KPWIN32)
#define KSPAPI		PASCAL
#define SPCALLBACK	PASCAL
#define SPNEAR

#elif defined (KPUNIX)
#define KSPAPI
#define SPCALLBACK
#define SPNEAR
#define TRUE	1
#define FALSE	0

#else
#error Unsupported OS

#endif

#if defined (JAVACMM)
#define SP_NO_OBSOLETE
#define SP_NO_FILEIO
#define SP_NO_TEXTFUNCTIONS
#define SP_READONLY_PROFILES
#endif

#if defined (SP_READONLY_PROFILES)
#define SP_NO_TAGSET
#endif

#if (defined (KPWIN) || defined (KPMAC)) && !defined (KPICM)
#if !defined (PSFUNC)
/* #define PSFUNC */
#endif
#endif
#if defined (KPSUN) && !defined (SOLARIS_CMM) && !defined (JAVACMM)
#if !defined (PSFUNC)
/* #define PSFUNC */
#endif
#endif

/*
 *	The data type SpIOFileChar_t is used to hide file system dependencies.
 *	For most OS, this is a meaningless data type.  However, for the
 *	Macintosh, there are some special parameters which need to be passed
 *	to give extra non-portable information about a file.
 */

#if defined(KPMAC) || defined(KPMSMAC)
typedef struct {
	KpChar_t    fileType[5];		/* type of file */
	KpChar_t    creatorType[5];		/* creator of file */
	short   	vRefNum;			/* vol. ref. num. for file dir. */
} SpIOFileChar_t;
typedef struct {
        char    fileType[5];         /* type of file */
        char    creatorType[5];      /* creator of file */
        short   vRefNum;             /* vol. ref. num. for file dir.  */
        long    dirID;               /* Dir entry for file dir. */
} SpFileProps_t;



#elif defined(KPWIN32)
typedef OFSTRUCT  SpIOFileChar_t;
typedef OFSTRUCT  SpFileProps_t;


#elif defined(KPWIN16) || defined(KPDOS)
#if defined(KPDOS)   /* DOS!!! */
	typedef struct tagOFSTRUCT {  /*  to get ioFile.c to compile easily */
		int dvalue;
		int hvalue;
	} OFSTRUCT;

	typedef OFSTRUCT FAR *LPOFSTRUCT;
#endif
typedef struct tagOFSTRUCT      SpIOFileChar_t;
typedef struct tagOFSTRUCT      SpFileProps_t;


#elif defined(KPUNIX)
typedef long    SpFileProps_t;

typedef long    SpIOFileChar_t;

#endif




/***********************************************************************
 * Data type definitions
 ***********************************************************************/

#define SpDefHandle(name)  struct name##_tag { char dontuse; }; \
                                typedef const struct name##_tag FAR* name

#define SpDefNearHandle(name)  struct name##_tag { char dontuse; }; \
                                typedef const struct name##_tag SPNEAR* name

typedef KpInt32_t	SpSig_t;
typedef void KPHUGE *SpHugeBuffer_t;
SpDefHandle(SpProfile_t);
SpDefHandle(SpXform_t);
SpDefHandle(SpCallerId_t);

#define SpSigMake(a,b,c,d) \
					( ( ((KpInt32_t) (unsigned char) (a)) << 24) | \
					  ( ((KpInt32_t) (unsigned char) (b)) << 16) | \
					  ( ((KpInt32_t) (unsigned char) (c)) <<  8) | \
						  (KpInt32_t) (unsigned char) (d))

/* Return status definitions */
#define	SpStatNotImp				-1
#define	SpStatSuccess				0
#define	SpStatBadCallerId			501
#define	SpStatBadLutType			502
#define	SpStatBadProfile			503
#define	SpStatBadTagData			504
#define	SpStatBadTagType			505
#define	SpStatBadTagId				506
#define	SpStatBadXform				507
#define	SpStatXformNotActive		508
#define	SpStatBufferTooSmall		509
#define	SpStatFailure				510
#define	SpStatFileNotFound			511
#define	SpStatFileReadError			512
#define	SpStatFileWriteError		513
#define	SpStatIncompatibleArguments	514
#define	SpStatMemory				515
#define	SpStatNoFileName			516
#define	SpStatNotFound				517
#define	SpStatOutOfRange			518
#define	SpStatTagNotFound			519
#define	SpStatBadBuffer				520
#define	SpStatBadProfileDir			521
#define	SpStatReqTagMissing			522
#define	SpStatUnsupported			523
#define	SpStatNoComponent			524
#define	SpStatNoCompMemory			525
#define	SpStatBadCompRequest		526
#define	SpStatBadCompInstance		527
#define SpStatCPComp				528
#define SpStatResFileErr			529
#define SpStatKcmFailure			530
#define SpStatAbort					531
#define SpStatXformIsPerceptual		532
#define SpStatXformIsColormetric	533 /* Misspelled */
#define SpStatXformIsColorimetric	533
#define SpStatXformIsSaturation		534

#define SpLastErrorCodeUsed			534
/* if you add a new error you MUST change the following files */
/*  sperr.c, msperr.r, msperr.rsrc, and sprof32.rc            */ 
/* and define these numbers in sprof32r.h                     */
/* also update the SpLastErrorCodeUsed to match the last      */
/* error code                                                 */

typedef KpInt32_t	SpStatus_t;

/* Iteration routine states */
#define	SpIterInit			1
#define	SpIterProcessing	2
#define	SpIterTerm			3

typedef KpInt32_t	SpIterState_t;


typedef SpStatus_t (SPCALLBACK *SpProgress_t)
				(SpIterState_t State, KpInt32_t Percent, void FAR *Data);

#define SpTagMaxString	256
#define SpTagMaxInts	8
#define SpTagMaxF15d16s	8

#if defined(_LP64) || defined(_WIN64)
typedef KpUInt32_t SpTagId_t;
typedef KpUInt32_t SpTagEnum_t;
#else
SpDefHandle(SpTagId_t);
SpDefHandle(SpTagEnum_t);
#endif

typedef KpInt32_t SpTagType_t;

/**********/
/* Binary */
/**********/

typedef struct {
	KpUInt32_t	Size;
	char		KPHUGE *Values;
} SpBinary_t;

/*********/
/* Curve */
/*********/

typedef KpResponse_t	SpCurve_t;

/*************************/
/* parametric Curve Type */
/*************************/
typedef struct {
	KpUInt16_t	FuncType;
	KpF15d16_p	Parameters;
}SpParaCurveData_t;

typedef struct {
	SpTagType_t		TagType;
	SpParaCurveData_t	ParaData;
} SpParaCurve_t;

typedef struct {
	SpSig_t			CurveType;	/* SpTypeCurve or SpTypeParametricCurve */
	KpUInt32_t		Reserved;
	SpCurve_t		Curve;
	SpParaCurveData_t	ParaCurve;
} SpLutCurve_t;

/********/
/* Data */
/********/

typedef struct {
	KpUInt32_t	DataFlag;
	KpUInt32_t	Count;
	char		FAR *Bytes;
} SpData_t;

/*****************/
/* Data and time */
/*****************/

typedef struct {
	KpUInt16_t	Year;	/* including the century */
	KpUInt16_t	Month;	/* 1 - 12 */
	KpUInt16_t	Day;	/* 1 - 31 */
	KpUInt16_t	Hour;	/* 0 - 23 */
	KpUInt16_t	Minute;	/* 0 - 59 */
	KpUInt16_t	Second;	/* 0 - 59 */
} SpDateTime_t;

/**********/
/* Header */
/**********/

#define HEADER_SIZE 128

/* Search Criteria to allow recursive and full searchs */
/* Full searchs return a maximum of the requested number 
   of profiles, but the Count returned is the total number
   of profiles that meet the search requirements, even if
   that number is bigger than the count given */
#define SPSEARCHFORALL		0x0400

#define SEARCH_RECURSIVE	0
#define SEARCH_NOT_RECURSIVE	1

#define SEARCH_RESOLVE_ALIAS	0
#define SEARCH_NOT_RESOLVE_ALIAS	2

/*  Header Fields to Test Against */
#define SPSEARCH_PREFERREDCMM       4
#define SPSEARCH_VERSION            13
#define SPSEARCH_PROFILECLASS       1
#define SPSEARCH_DEVICECOLORSPACE   2
#define SPSEARCH_CONNECTIONSPACE    3
#define SPSEARCH_BEFOREDATE         14
#define SPSEARCH_ONDATE             15
#define SPSEARCH_AFTERDATE          16
#define SPSEARCH_PLATFORM           5
#define SPSEARCH_PROFILEFLAGS       6
#define SPSEARCH_DEVICEMFG          7
#define SPSEARCH_DEVICEMODEL        8
#define SPSEARCH_DEVICEATTRIBUTESHI 9
#define SPSEARCH_DEVICEATTRIBUTESLO 10
#define SPSEARCH_RENDERINGINTENT    11
#define SPSEARCH_ILLUMINANT         12
#define SPSEARCH_ORIGINATOR         17
#define SPSEARCH_TIMEORDER          18
#define SPSEARCH_PROFILEID          19

/*  Tag Fields to Test Against 
	Supported by SpProfileCheckEx()
	Not supported by SpProfileCheck()
*/
#define SPSEARCH_ENUMTAGVALUE_EQUAL  20
#define SPSEARCH_ENUMTAGVALUE_NOTEQUAL  21


#define SPSEARCH_LIST               21

#define BEFORE 0
#define SAME   1
#define AFTER  2

typedef struct {
	SpSig_t			CMMType;
	KpUInt32_t		ProfileVersion;
	SpSig_t			DeviceClass;
	SpSig_t			DataColorSpace;
	SpSig_t			InterchangeColorSpace;
	SpDateTime_t		DateTime;
	SpSig_t			Platform;
	KpUInt32_t		Flags;
	SpSig_t			DeviceManufacturer;
	SpSig_t			DeviceModel;
	struct {
		KpUInt32_t	hi;
		KpUInt32_t	lo;
	} DeviceAttributes;
	KpUInt32_t		RenderingIntent;
	KpF15d16XYZ_t		Illuminant;
	SpSig_t			Originator;
	struct {
		KpUInt32_t	a;
		KpUInt32_t	b;
		KpUInt32_t	c;
		KpUInt32_t	d;
	} ProfileID;
	char			Reserved [28];
} SpHeader_t;

/************************************/
/* Rendering Types for Header ONLY. */
/* Don't use these values if you    */
/* are trying to get a specific     */
/* transform from a profile.        */
/************************************/
#define	SpRenderPerceptual		0
#define	SpRenderColormetric		1 /* misspelled */
#define	SpRenderColorimetric		1
#define	SpRenderSaturation		2
#define	SpRenderAbsColormetric		3 /* misspelled */
#define	SpRenderAbsColorimetric		3

#define SpProfileSig	SpSigMake('a', 'c', 's', 'p')
#define SpSigMfgKodak	SpSigMake('K', 'O', 'D', 'A')
#define SpSigOrgKodak SpSigMake('K', 'O', 'D', '1')
#define SpSigNone	0
#define SpMaxTextDesc	64

/********/
/* Luts */
/********/

typedef struct {
	unsigned char	InputChannels;
	unsigned char	OutputChannels;
	unsigned char	LUTDimensions;
	KpF15d16_t		Matrix3x3 [9];
	KpUInt16_t		InputTableEntries;
	KpUInt16_t		OutputTableEntries;
	KpUInt16_t		FAR *InputTable;
	KpUInt16_t		FAR *CLUT;
	KpUInt16_t		FAR *OutputTable;
} SpLut16Bit_t;

typedef struct {
	unsigned char	InputChannels;
	unsigned char	OutputChannels;
	unsigned char	LUTDimensions;
	KpF15d16_t		Matrix3x3 [9];
	unsigned char	FAR *InputTable;	/* 256*InputChannels entries */
	unsigned char	FAR *CLUT;
	unsigned char	FAR *OutputTable;	/* 256*OutputChannels entries */
} SpLut8Bit_t;

typedef struct {
	unsigned char	NumGridPt[16];
	unsigned char	Precision;
	unsigned char	Reserve[3];
	union {
		KpUInt16_t	FAR *Lut16;
		unsigned char	FAR *Lut8;
	} L;
} SpCLUT_t;

typedef struct {
	unsigned char	InputChannels;
	unsigned char	OutputChannels;
	KpUInt16_t	Reserved;
	SpLutCurve_t	FAR *Bcurve;
	KpF15d16_t	FAR *Matrix;
	SpLutCurve_t	FAR *Mcurve;
	SpCLUT_t	FAR *CLUT;
	SpLutCurve_t	FAR *Acurve;
} SpLutAB_t;

typedef struct {
	SpSig_t		LutType;
	union {
		SpLut16Bit_t	Lut16;
		SpLut8Bit_t	Lut8;
		SpLutAB_t	LutAB;
	} L;
} SpLut_t;

/***************/
/* Measurement */
/***************/

typedef struct {
	KpUInt32_t		StdObserver;
	KpF15d16XYZ_t	Backing;
	KpUInt32_t		Geometry;
	KpF15d16_t		Flare;
	KpUInt32_t		IllumType;
} SpMeasurement_t;

/* Illuminate Types */
#define SpIllumTypeD50	1
#define SpIllumTypeD65	2
#define SpIllumTypeD93	3
#define SpIllumTypeF2	4
#define SpIllumTypeD55	5
#define SpIllumTypeA	6
#define SpIllumTypeE	7
#define SpIllumTypeF8	8

/****************/
/* Named Colors */
/****************/

#define SpMaxNamedColorNameLen	32
#define SpPCSChannels         3
#define SpMaxNamedColorChannels	8


typedef struct {
	char		Name [SpMaxNamedColorNameLen + 1];
	KpUInt16_t	Values [SpMaxNamedColorChannels];
} SpNamedColor_t;


typedef struct {
	KpUInt32_t		VendorFlags;
	KpUInt32_t		Count;
	KpInt32_t		DeviceChannels;
	char			Prefix [SpMaxNamedColorNameLen + 1];
	char			Suffix [SpMaxNamedColorNameLen + 1];
	SpNamedColor_t	FAR *Colors;
} SpNamedColors_t;

typedef struct {
	char		Name [SpMaxNamedColorNameLen];
	KpUInt16_t	Values [SpPCSChannels];
	KpUInt16_t	dValues [SpMaxNamedColorChannels];
} SpNamedColor2_t;


typedef struct {
	KpUInt32_t		VendorFlags;
	KpUInt32_t		Count;
	KpInt32_t		DeviceChannels;
	char			Prefix [SpMaxNamedColorNameLen];
	char			Suffix [SpMaxNamedColorNameLen];
	SpNamedColor2_t	FAR *Colors;
} SpNamedColors2_t;

/***********************/
/* Multi-langauge Tag  */
/***********************/
typedef struct {
	KpInt16_t	LanguageCode;
	KpInt16_t	CountryCode;
	KpInt32_t	StringLength;
	KpUInt16_t	*UniString;
} SpUnicodeInfo_t;

typedef struct {
	KpInt32_t	Count;		/* Number of records that follow */
	KpInt32_t	RecSize;	/* Size of the Name Info Record */
	SpUnicodeInfo_t	*StringInfo;
} SpMultiLang_t;


/********************/
/* Text Description */
/********************/
typedef struct {
	char		FAR *IsoStr;
	KpInt32_t	UniLangCode;
	KpUInt16_t	FAR *UniStr;
	KpInt16_t	MacScriptCode;
	char		MacCount;
	char		MacStr [67];
} SpTextDesc_t;

/********************************/
/* Profile Sequence Description */
/********************************/
typedef struct {
	SpTagType_t	TagType;
	KpUInt32_t	Reserved;
	SpMultiLang_t	FAR MLDesc;
} SpDeviceDesc_t;

typedef struct {
	SpSig_t			DeviceManufacturer;
	SpSig_t			DeviceModel;
	struct {
		KpUInt32_t		hi;
		KpUInt32_t		lo;
	} DeviceAttributes;
	SpTextDesc_t	DeviceManufacturerDesc;
	SpTextDesc_t	DeviceModelDesc;
	SpSig_t			Technology;
} SpProfileSeqDescRecord_t;

typedef struct {
	SpSig_t			DeviceManufacturer;
	SpSig_t			DeviceModel;
	struct {
		KpUInt32_t		hi;
		KpUInt32_t		lo;
	} DeviceAttributes;
	SpDeviceDesc_t	DeviceManufacturerDesc;
	SpDeviceDesc_t	DeviceModelDesc;
	SpSig_t			Technology;
} SpProfileSeqDescRecord2_t;

typedef struct {
	KpUInt32_t					Count;
	SpProfileSeqDescRecord_t	FAR *Records;
} SpProfileSeqDesc_t;

typedef struct {
	KpUInt32_t					Count;
	SpProfileSeqDescRecord2_t	FAR *Records;
} SpProfileSeqDesc2_t;

/**********************/
/* Signed Fixed 15.16 */
/**********************/
typedef struct {
	KpUInt32_t	Count;
	KpF15d16_t	FAR *Values;
} SpSF15d16s_t;

/*************/
/* Screening */
/*************/
typedef struct {
	KpF15d16_t	Frequency;
	KpF15d16_t	Angle;
	KpUInt32_t	Spot;
} SpScreen_t;

typedef struct {
	KpUInt32_t	Flag;
	KpUInt32_t	Channels;
	SpScreen_t	FAR *Screens;
} SpScreening_t;

#define SpScreenFlagPrnDefault		1
#define SpScreenFlagLinesPerInch	2

#define SpScreenSpotUnknown		0
#define SpScreenSpotPrinter		1
#define SpScreenSpotRound		2
#define SpScreenSpotDiamond		3
#define SpScreenSpotEllipse		4
#define SpScreenSpotLine		5
#define SpScreenSpotSquare		6
#define SpScreenSpotCross		7

/************************/
/* Unsigned Fixed 16.16 */
/************************/
typedef struct {
	KpUInt32_t	Count;
	KpUInt32_t	FAR *Values;
} SpUF16d16s_t;

/*********/
/* Ucrbg */
/*********/
typedef struct {
	KpUInt32_t	UcrCount;
	KpUInt16_t	FAR *Ucr;
	KpUInt32_t	bgCount;
	KpUInt16_t	FAR *bg;
	char		FAR *Desc;
} SpUcrbg_t;

/*******************/
/* Unsigned Int 16 */
/*******************/
typedef struct {
	KpUInt32_t	Count;
	KpUInt16_t	FAR *Values;
} SpUInt16s_t;

/*******************/
/* Unsigned Int 32 */
/*******************/
typedef struct {
	KpUInt32_t	Count;
	KpUInt32_t	FAR *Values;
} SpUInt32s_t;

/*******************/
/* Unsigned Int 64 */
/*******************/
typedef struct {
	KpUInt32_t	hi;
	KpUInt32_t	lo;
} SpUInt64_t;

typedef struct {
	KpUInt32_t	Count;
	SpUInt64_t	FAR *Values;
} SpUInt64s_t;

/******************/
/* Unsigned Int 8 */
/******************/
typedef struct {
	KpUInt32_t		Count;
	unsigned char	FAR *Values;
} SpUInt8s_t;

/**********************/
/* Viewing Conditions */
/**********************/
typedef struct {
	KpF15d16XYZ_t	Illuminant;
	KpF15d16XYZ_t	Surround;
	KpUInt32_t		IllumType;
} SpViewing_t;

/**********************/
/* Color rendering dictonaries Info */
/**********************/
typedef struct {
	KpUInt32_t	count;
	char		*CRD_String;
} SpCrdItem_t;

typedef struct {
	SpCrdItem_t	ProdDesc;
	SpCrdItem_t	CrdInfo[4];
} SpCrdInfo_t;

/*****************************/
/* Phosphor data structures  */
/*****************************/
typedef struct {
	KpF15d16_t	x;
	KpF15d16_t	y;
}   SpPhosphorChrom_t;

typedef struct {
	SpPhosphorChrom_t	red;
	SpPhosphorChrom_t	green;
	SpPhosphorChrom_t	blue;
} SpPhosphor_t;

/**************************************/
/* Chromaticity Data Structure        */
/**************************************/
typedef struct {
	KpF15d16_t	x;
	KpF15d16_t	y;
}   SpChromCoord_t;

typedef struct {
	KpInt32_t		NumChan;
	KpInt32_t		ColorType;
	SpChromCoord_t		*Coords;
} SpChromaticity_t;

/**************************************/
/* Device Setting Data Structures     */
/**************************************/
typedef struct {
	KpUInt16_t		Interval;
	KpF15d16_t		MeasVal;
} SpResponse16_t;

typedef struct {
	SpSig_t		MeasSig;
	KpInt32_t	*RespCount;
	KpF15d16XYZ_t	*MeasArray;
	SpResponse16_t	*RespData;
} SpResCurve_t;
 
typedef struct {
	KpInt32_t	NumChan;
	KpInt32_t	MeasCount;
	SpResCurve_t	*ResCurves;
} SpResCurveTag_t;

/**************************************/
/* Device Setting Data Structures     */
/**************************************/
typedef struct {
SpSig_t		SettingSig;
KpInt32_t	SettingSize;
KpInt32_t	numSettings;
void		*Setting;  /* KpDouble64_t or KpInt32_t */
} SpSetStruct_t, *SpSetStruct_p;
 
typedef struct {
KpInt32_t	SetCount;
KpInt32_t	CombSize;
SpSetStruct_t	*SetArray;
} SpCombStruct_t, *SpCombStruct_p;

typedef struct {
SpSig_t		PlatformId;
KpInt32_t	CombCount;
KpInt32_t	PlatformSize;
SpCombStruct_t	*CombArray;
} SpPlatStruct_t, *SpPlatStruct_p;
 
typedef struct {
KpInt32_t	NumOfPlatforms;
SpPlatStruct_t	*PlatformArray;
} SpDevStruct_t;

/****************************/
/* Colorant Table Structure */
/****************************/
#define SpMaxColorantNameLen	32

typedef struct {
char		ColorantName[SpMaxColorantNameLen];
KpUInt16_t	Coord[SpPCSChannels];
} SpColorantEntry_t;

typedef struct {
KpUInt32_t		Count;
SpColorantEntry_t	*Colorant;
} SpColorantTable_t;

/****************************/
/* Colorant Order Structure */
/****************************/
/* Maximum number of colors in a color space */
#define SpMaxColorCount		16 

typedef struct {
KpUInt32_t	Count;
KpInt8_t	Colorant[SpMaxColorCount];
} SpColorantOrder_t;

/************************/
/* Simple Langage Codes */
/************************/
#define SpEnglish	0x656e
#define SpUSA		0x7573

/*******************/
/* Type signatures */
/*******************/

#define SpTypeChromaticity		SpSigMake('c', 'h', 'r', 'm')
#define SpTypeColorantOrder		SpSigMake('c', 'l', 'r', 'o')
#define SpTypeColorantTable		SpSigMake('c', 'l', 'r', 't')
#define SpTypeCrdInfo			SpSigMake('c', 'r', 'd', 'i')
#define SpTypeCurve			SpSigMake('c', 'u', 'r', 'v')
#define SpTypeData			SpSigMake('d', 'a', 't', 'a')
#define SpTypeDateTime			SpSigMake('d', 't', 'i', 'm')
#define SpTypeDevSettings		SpSigMake('d', 'e', 'v', 's')
#define SpTypeLutBA			SpSigMake('m', 'B', 'A', ' ')
#define SpTypeLutAB			SpSigMake('m', 'A', 'B', ' ')
#define SpTypeLut16			SpSigMake('m', 'f', 't', '2')
#define SpTypeLut8			SpSigMake('m', 'f', 't', '1')
#define SpTypeMeasurement		SpSigMake('m', 'e', 'a', 's')
#define SpTypeMultiLanguage		SpSigMake('m', 'l', 'u', 'c')
#define SpTypeNamedColors		SpSigMake('n', 'c', 'o', 'l')
#define SpTypeNamedColors2		SpSigMake('n', 'c', 'l', '2')
#define SpTypeParametricCurve		SpSigMake('p', 'a', 'r', 'a')
#define SpTypeProfileSeqDesc		SpSigMake('p', 's', 'e', 'q')
#define SpTypeResponseCurve		SpSigMake('r', 'c', 's', '2')
#define SpTypeSF15d16			SpSigMake('s', 'f', '3', '2')
#define SpTypeScreening			SpSigMake('s', 'c', 'r', 'n')
#define SpTypeSignature			SpSigMake('s', 'i', 'g', ' ')
#define SpTypeText			SpSigMake('t', 'e', 'x', 't')
#define SpTypeTextDesc			SpSigMake('d', 'e', 's', 'c')
#define SpTypeUF16d16			SpSigMake('u', 'f', '3', '2')
#define SpTypeUcrbg			SpSigMake('b', 'f', 'd', ' ')
#define SpTypeUInt16			SpSigMake('u', 'i', '1', '6')
#define SpTypeUInt32			SpSigMake('u', 'i', '3', '2')
#define SpTypeUInt64			SpSigMake('u', 'i', '6', '4')
#define SpTypeUInt8			SpSigMake('u', 'i', '0', '8')
#define SpTypeViewing			SpSigMake('v', 'i', 'e', 'w')
#define SpTypeXYZ			SpSigMake('X', 'Y', 'Z', ' ')

/* private types */
#define SpTypeLutFuTF 			SpSigMake('f', 'u', 't', 'f')
#define SpTypeLutFTuF 			SpSigMake('f', 't', 'u', 'f')

/********************/
/* Type identifiers */
/********************/
/* Last type used is 38 */

#define	Sp_AT_Enum			1

#define Sp_AT_Chromaticity		34

#define Sp_AT_ColorantTable		35
#define Sp_AT_ColorantOrder		36

#define	Sp_AT_CrdInfo			31
#define	Sp_AT_Curve			10
#define	Sp_AT_Data			11
#define	Sp_AT_DateTime			12
#define Sp_AT_DevSettings		32
#define	Sp_AT_Lut			13
#define	Sp_AT_Measurement		14
#define	Sp_AT_MultiLanguage		37
#define	Sp_AT_NamedColors		15
#define	Sp_AT_NamedColors2		30
#define Sp_AT_ParametricCurve		38
#define	Sp_AT_ProfileSeqDesc		16
#define Sp_AT_ResponseCurve		33
#define	Sp_AT_SF15d16			17
#define	Sp_AT_Screening			18
#define	Sp_AT_Signature			19
#define	Sp_AT_Text			20
#define	Sp_AT_TextDesc			21
#define	Sp_AT_UF16d16			22
#define	Sp_AT_Ucrbg			23
#define	Sp_AT_UInt16			24
#define	Sp_AT_UInt32			25
#define	Sp_AT_UInt64			26
#define	Sp_AT_UInt8			27
#define	Sp_AT_Viewing			28
#define	Sp_AT_XYZ			29

#define	Sp_AT_Unknown			1000
#define	Sp_AT_ENDLIST			1001


typedef struct {
	SpTagId_t		TagId;
	SpTagType_t		TagType;
	union {
		SpTagEnum_t		TagEnum;	/* Sp_AT_Enum */
		SpBinary_t		Binary;		/* Sp_AT_Unknown */

		SpCrdInfo_t		CrdData;	/* Sp_AT_CrdInfo */
		SpCurve_t		Curve;		/* Sp_AT_Curve */
		SpData_t		Data;		/* Sp_AT_Data */
		SpDateTime_t		DateTime;	/* Sp_AT_DateTime */
		SpLut_t			Lut;		/* Sp_AT_Lut */
		SpMeasurement_t		Measurement;	/* Sp_AT_Measurement */
		SpNamedColors_t		NamedColors;	/* Sp_AT_NamedColors */
		SpNamedColors2_t	NamedColors2;	/* Sp_AT_NamedColors2 */
		SpProfileSeqDesc_t	ProfileSeqDesc;	/* Sp_AT_ProfileSeqDesc */
		SpProfileSeqDesc2_t	ProfileSeqDesc2;	/* Sp_AT_ProfileSeqDesc */
		SpSF15d16s_t		SF15d16s;	/* Sp_AT_SF15d16 */
		SpScreening_t		Screening;	/* Sp_AT_Screening */
		SpSig_t			Signature;	/* Sp_AT_Signature */
		char			FAR *Text;	/* Sp_AT_Text */
		SpTextDesc_t		TextDesc;	/* Sp_AT_TextDesc */
		SpUcrbg_t		Ucrbg;		/* Sp_AT_Ucrbg */
		SpUF16d16s_t 		UF16d16s;	/* Sp_AT_UF16d16 */
		SpUInt16s_t		UInt16s;	/* Sp_AT_UInt16 */
		SpUInt32s_t		UInt32s;	/* Sp_AT_UInt32 */
		SpUInt64s_t		UInt64s;	/* Sp_AT_UInt64 */
		SpUInt8s_t		UInt8s;		/* Sp_AT_UInt8 */
		SpViewing_t		Viewing;	/* Sp_AT_Viewing */
		KpF15d16XYZ_t		XYZ;		/* Sp_AT_XYZ */

		SpDevStruct_t		DevSet;		/* Sp_AT_DevSettings */
		SpResCurveTag_t		Response;	/* Sp_AT_ResponseCurve */
		SpChromaticity_t	Chromaticity;	/* Sp_AT_Chromaticity */

		SpColorantTable_t	ColorTable;	/* Sp_AT_ColorantTable */
		SpColorantOrder_t	ColorOrder;	/* Sp_AT_ColorantOrder */

		SpMultiLang_t		MultiLang;	/* Sp_AT_MultiLanguage */
		SpParaCurve_t		ParaCurve;	/* Sp_AT_ParametricCurve */
	} Data;
} SpTagValue_t;


/***********************************************************************
 * Tag ID definitions
 ***********************************************************************/
#define SpTagIdConst(a, b, c, d) \
				((SpTagId_t) SpSigMake ((a), (b), (c), (d)))

#define SpTagUnknown			((SpTagId_t) 0U)

/* Kodak private tags */
#define SpTagPrivate			SpTagIdConst('p', 'r', 'v', 't')
#define SpTagKDeviceBits		SpTagIdConst('K', '0', '0', '3')
#define SpTagKCompressedLUT		SpTagIdConst('K', '0', '0', '4')
#define SpTagKDeviceSerialNum		SpTagIdConst('K', '0', '0', '6')
#define SpTagKDeviceSettings		SpTagIdConst('K', '0', '0', '7')
#define SpTagKToneInfoDesc		SpTagIdConst('K','0','0','8')
#define SpTagKDeviceUnit		SpTagIdConst('K', '0', '0', '9')
#define SpTagKDMax			SpTagIdConst('K', '0', '1', '0')
#define SpTagKEffectType		SpTagIdConst('K', '0', '1', '1')
#define SpTagKIllum			SpTagIdConst('K', '0', '1', '3')
#define SpTagKInterpretation		SpTagIdConst('K', '0', '1', '5')
#define SpTagKLinearizationType		SpTagIdConst('K', '0', '1', '6')
#define SpTagKLinearized		SpTagIdConst('K', '0', '1', '7')
#define SpTagKMedium			SpTagIdConst('K', '0', '1', '8')
#define SpTagKMediumDesc		SpTagIdConst('K', '0', '1', '9')
#define SpTagKMediumProduct		SpTagIdConst('K', '0', '2', '0')
#define SpTagKMediumSense		SpTagIdConst('K', '0', '2', '1')
#define SpTagKDotGain25			SpTagIdConst('K', '0', '2', '2')
#define SpTagKDotGain50			SpTagIdConst('K', '0', '2', '3')
#define SpTagKDotGain75			SpTagIdConst('K', '0', '2', '4')
#define SpTagKPhosphor			SpTagIdConst('K', '0', '2', '5')
#define SpTagKCreationSoftware	SpTagIdConst('K', '0', '2', '6')
#define SpTagKPrtBlackShape		SpTagIdConst('K', '0', '2', '8')
#define SpTagKPrtBlackStartDelay	SpTagIdConst('K', '0', '2', '9')
#define SpTagKSenseInvertible		SpTagIdConst('K', '0', '3', '0')
#define SpTagKVersion			SpTagIdConst('K', '0', '3', '1')
#define SpTagKDensityType		SpTagIdConst('K', '0', '3', '2')
#define SpTagKProfileHistory		SpTagIdConst('K', '0', '3', '3')
#define SpTagKPCDFilmTerm		SpTagIdConst('K', '0', '3', '4')

/* Kodak private tags for device to device profiles */
#define SpTagKXchDeviceBits		SpTagIdConst('K', '0', '4', '1')
#define SpTagKXchDeviceSerialNum	SpTagIdConst('K', '0', '4', '4')
#define SpTagKXchDeviceSettings		SpTagIdConst('K', '0', '4', '5')
#define SpTagKXchDeviceUnit		SpTagIdConst('K', '0', '4', '7')
#define SpTagKXchGamma			SpTagIdConst('K', '0', '4', '8')
#define SpTagKXchIllum			SpTagIdConst('K', '0', '4', '9')
#define SpTagKXchInterpretation		SpTagIdConst('K', '0', '5', '0')
#define SpTagKXchLinearizationType	SpTagIdConst('K', '0', '5', '1')
#define SpTagKXchLinearized		SpTagIdConst('K', '0', '5', '2')
#define SpTagKXchMedium			SpTagIdConst('K', '0', '5', '3')
#define SpTagKXchMediumDesc		SpTagIdConst('K', '0', '5', '4')
#define SpTagKXchMediumProduct		SpTagIdConst('K', '0', '5', '5')
#define SpTagKXchMediumSense		SpTagIdConst('K', '0', '5', '6')
#define SpTagKXchPhosphor		SpTagIdConst('K', '0', '5', '7')
#define SpTagKXchSenseInvertible	SpTagIdConst('K', '0', '5', '8')
#define SpTagKXchDotGain25		SpTagIdConst('K', '0', '5', '9')
#define SpTagKXchDotGain50		SpTagIdConst('K', '0', '6', '0')
#define SpTagKXchDotGain75		SpTagIdConst('K', '0', '6', '1')

/* Kodak private tags for chaining rules */
#define SpTagKChainAToB0		SpTagIdConst('K', '0', '7', '0')
#define SpTagKChainBToA0		SpTagIdConst('K', '0', '7', '1')
#define SpTagKChainPreview0		SpTagIdConst('K', '0', '7', '2')

#define SpTagKChainAToB1		SpTagIdConst('K', '0', '7', '3')
#define SpTagKChainBToA1		SpTagIdConst('K', '0', '7', '4')
#define SpTagKChainPreview1		SpTagIdConst('K', '0', '7', '5')

#define SpTagKChainAToB2		SpTagIdConst('K', '0', '7', '6')
#define SpTagKChainBToA2		SpTagIdConst('K', '0', '7', '7')
#define SpTagKChainPreview2		SpTagIdConst('K', '0', '7', '8')

#define SpTagKChainGamut		SpTagIdConst('K', '0', '7', '9')

/* Kodak private tags for multi-colorant profile */
#define SpTagKInkName0			SpTagIdConst('K', '0', '8', '0')
#define SpTagKInkName1			SpTagIdConst('K', '0', '8', '1')
#define SpTagKInkName2			SpTagIdConst('K', '0', '8', '2')
#define SpTagKInkName3			SpTagIdConst('K', '0', '8', '3')
#define SpTagKInkName4			SpTagIdConst('K', '0', '8', '4')
#define SpTagKInkName5			SpTagIdConst('K', '0', '8', '5')
#define SpTagKInkName6			SpTagIdConst('K', '0', '8', '6')
#define SpTagKInkName7			SpTagIdConst('K', '0', '8', '7')
#define SpTagKInkDensities		SpTagIdConst('K', '0', '8', '8')


/***************/
/* Public tags */
/***************/
#define SpTagAToB0			SpTagIdConst('A', '2', 'B', '0')
#define SpTagAToB1			SpTagIdConst('A', '2', 'B', '1')
#define SpTagAToB2			SpTagIdConst('A', '2', 'B', '2')
#define SpTagBlueColorant		SpTagIdConst('b', 'X', 'Y', 'Z')
#define SpTagBlueTRC			SpTagIdConst('b', 'T', 'R', 'C')
#define SpTagBToA0			SpTagIdConst('B', '2', 'A', '0')
#define SpTagBToA1			SpTagIdConst('B', '2', 'A', '1')
#define SpTagBToA2			SpTagIdConst('B', '2', 'A', '2')
#define SpTagCalibrationDateTime	SpTagIdConst('c', 'a', 'l', 't')
#define SpTagCharTarget			SpTagIdConst('t', 'a', 'r', 'g')
#define SpTagChromaticAdapt		SpTagIdConst('c', 'h', 'a', 'd')
#define SpTagChromaticity		SpTagIdConst('c', 'h', 'r', 'm')
#define SpTagColorantOrder		SpTagIdConst('c', 'l', 'r', 'o')
#define SpTagColorantTable		SpTagIdConst('c', 'l', 'r', 't')
#define SpTagCopyRight			SpTagIdConst('c', 'p', 'r', 't')
#define SpTagCrdInfo			SpTagIdConst('c', 'r', 'd', 'i')
#define SpTagDeviceMfgDesc		SpTagIdConst('d', 'm', 'n', 'd')
#define SpTagDeviceModelDesc		SpTagIdConst('d', 'm', 'd', 'd')
#define SpTagDevSettings		SpTagIdConst('d', 'e', 'v', 's')
#define SpTagGamut			SpTagIdConst('g', 'a', 'm', 't')
#define SpTagGrayTRC			SpTagIdConst('k', 'T', 'R', 'C')
#define SpTagGreenColorant		SpTagIdConst('g', 'X', 'Y', 'Z')
#define SpTagGreenTRC			SpTagIdConst('g', 'T', 'R', 'C')
#define SpTagLuminance			SpTagIdConst('l', 'u', 'm', 'i')
#define SpTagMeasurement		SpTagIdConst('m', 'e', 'a', 's')
#define SpTagMediaBlackPnt		SpTagIdConst('b', 'k', 'p', 't')
#define SpTagMediaWhitePnt		SpTagIdConst('w', 't', 'p', 't')
#define SpTagNamedColor			SpTagIdConst('n', 'c', 'o', 'l')
#define SpTagNamedColor2		SpTagIdConst('n', 'c', 'l', '2')
#define SpTagPreview0			SpTagIdConst('p', 'r', 'e', '0')
#define SpTagPreview1			SpTagIdConst('p', 'r', 'e', '1')
#define SpTagPreview2			SpTagIdConst('p', 'r', 'e', '2')
#define SpTagProfileDesc		SpTagIdConst('d', 'e', 's', 'c')
#define SpTagProfileSeqDesc		SpTagIdConst('p', 's', 'e', 'q')
#define SpTagPS2CRD0			SpTagIdConst('p', 's', 'd', '0')
#define SpTagPS2CRD1			SpTagIdConst('p', 's', 'd', '1')
#define SpTagPS2CRD2			SpTagIdConst('p', 's', 'd', '2')
#define SpTagPS2CRD3			SpTagIdConst('p', 's', 'd', '3')
#define SpTagPS2CSA			SpTagIdConst('p', 's', '2', 's')
#define SpTagPS2RenderIntent		SpTagIdConst('p', 's', '2', 'i')
#define SpTagRedColorant		SpTagIdConst('r', 'X', 'Y', 'Z')
#define SpTagRedTRC			SpTagIdConst('r', 'T', 'R', 'C')
#define SpTagResponseCurve		SpTagIdConst('r', 'e', 's', 'p')
#define SpTagScreeningDesc		SpTagIdConst('s', 'c', 'r', 'd')
#define SpTagScreening			SpTagIdConst('s', 'c', 'r', 'n')
#define SpTagTechnology			SpTagIdConst('t', 'e', 'c', 'h')
#define SpTagUcrbg			SpTagIdConst('b', 'f', 'd', ' ')
#define SpTagViewingCondDesc		SpTagIdConst('v', 'u', 'e', 'd')
#define SpTagViewingConditions		SpTagIdConst('v', 'i', 'e', 'w')
#define SpTagENDLIST			SpTagIdConst('o', 'm', 'e', 'g')

/* CMM Type Identifier */
#define SpCMMType			SpSigMake('K', 'C', 'M', 'S')

/* Profile Version Identifier */
#define SPICCVER23	0x02300000U	/* Major = 02, minor.bugfix=3.0, reserved=0000 */
#define SPICCVER40	0x04000000U	/* Major = 04, minor.bugfix=0.0, reserved=0000 */
#define SpProfileVersion		0x04000000U	/* Major = 04, minor.bugfix=0.0, reserved=0000 */
#define SpProfileTagVersion		0x04000000U	/* Major = 04, minor.bugfix=0.0, reserved=0000 */

/* Color Space Identifiers */
#define SpSpaceXYZ			SpSigMake('X', 'Y', 'Z', ' ')
#define SpSpaceLAB			SpSigMake('L', 'a', 'b', ' ')
#define SpSpaceluv			SpSigMake('L', 'u', 'v', ' ')
#define SpSpaceYCbCr			SpSigMake('Y', 'C', 'b', 'r')
#define SpSpaceYxy			SpSigMake('Y', 'x', 'y', ' ')
#define SpSpaceRGB			SpSigMake('R', 'G', 'B', ' ')
#define SpSpaceGRAY			SpSigMake('G', 'R', 'A', 'Y')
#define SpSpaceHSV 			SpSigMake('H', 'S', 'V', ' ')
#define SpSpaceHLS 			SpSigMake('H', 'L', 'S', ' ')
#define SpSpaceCMYK			SpSigMake('C', 'M', 'Y', 'K')
#define SpSpaceCMY 			SpSigMake('C', 'M', 'Y', ' ')
/* The color spaces MCH5-MCH8 are not official ICC color spaces */
#define SpSpaceMCH5			SpSigMake('M', 'C', 'H', '5')
#define SpSpaceMCH6			SpSigMake('M', 'C', 'H', '6')
#define SpSpaceMCH7			SpSigMake('M', 'C', 'H', '7')
#define SpSpaceMCH8			SpSigMake('M', 'C', 'H', '8')
/* New color spaces approved at the July 1996 ICC meeting */
#define SpSpace2CLR			SpSigMake('2', 'C', 'L', 'R')
#define SpSpace3CLR			SpSigMake('3', 'C', 'L', 'R')
#define SpSpace4CLR			SpSigMake('4', 'C', 'L', 'R')
#define SpSpace5CLR			SpSigMake('5', 'C', 'L', 'R')
#define SpSpace6CLR			SpSigMake('6', 'C', 'L', 'R')
#define SpSpace7CLR			SpSigMake('7', 'C', 'L', 'R')
#define SpSpace8CLR			SpSigMake('8', 'C', 'L', 'R')
#define SpSpace9CLR			SpSigMake('9', 'C', 'L', 'R')
#define SpSpaceACLR			SpSigMake('A', 'C', 'L', 'R')
#define SpSpaceBCLR			SpSigMake('B', 'C', 'L', 'R')
#define SpSpaceCCLR			SpSigMake('C', 'C', 'L', 'R')
#define SpSpaceDCLR			SpSigMake('D', 'C', 'L', 'R')
#define SpSpaceECLR			SpSigMake('E', 'C', 'L', 'R')
#define SpSpaceFCLR			SpSigMake('F', 'C', 'L', 'R')
/* Unofficial color space to allow Sp routines to return a reasonable
   color space for gamut alarm transforms */
#define SpSpaceGAMUT			SpSigMake('G', 'A', 'M', 'T')

/* Special case color spaces for internal use only */
#define SpSpaceIXYZ			SpSigMake('e', 'k', 0, 0)	/* XYZ in an image */
#define SpSpaceALAB			SpSigMake('e', 'k', 0, 1)	/* Adobe Lab, 2^3 grid, mft1 */
#define SpSpaceILAB			SpSigMake('e', 'k', 0, 2)	/* Lab in an image */
#define	SpSpaceILCH			SpSigMake('e', 'k', 0, 3)   /* CIE LCH in an image */
#define SpSpaceIuvL			SpSigMake('e', 'k', 0, 4)   /* u'v'L* in an image */

/* Profile Class Identifiers */
#define SpProfileClassInput		SpSigMake('s', 'c', 'n', 'r')
#define SpProfileClassOutput		SpSigMake('p', 'r', 't', 'r')
#define SpProfileClassDisplay		SpSigMake('m', 'n', 't', 'r')
#define SpProfileClassAbst		SpSigMake('a', 'b', 's', 't')
#define SpProfileClassLink		SpSigMake('l', 'i', 'n', 'k')
#define SpProfileClassSpace		SpSigMake('s', 'p', 'a', 'c')
#define SpProfileClassNamedColor	SpSigMake('n', 'm', 'c', 'l')

/* Technology Identifiers */
#define SpTechFilmScanner		SpSigMake('f', 's', 'c', 'n')
#define SpTechReflectiveScanner		SpSigMake('r', 's', 'c', 'n')
#define SpTechInkJetPrinter		SpSigMake('i', 'j', 'e', 't')
#define SpTechThermalWaxPrinter		SpSigMake('t', 'w', 'a', 'x')
#define SpTechElectrophotoPrinter	SpSigMake('e', 'p', 'h', 'o')
#define SpTechElectrostaticPrinter	SpSigMake('e', 's', 't', 'a')
#define SpTechDyeSubPrinter		SpSigMake('d', 's', 'u', 'b')
#define SpTechPhotoPaperPrinter		SpSigMake('r', 'p', 'h', 'o')
#define SpTechFilmWriter		SpSigMake('f', 'p', 'r', 'n')
#define SpTechVideoMonitor		SpSigMake('v', 'i', 'd', 'm')
#define SpTechVideoCamera		SpSigMake('v', 'i', 'd', 'c')
#define SpTechProjectionTv		SpSigMake('p', 'j', 't', 'v')
#define SpTechCRTDisplay		SpSigMake('C', 'R', 'T', ' ')
#define SpTechActiveMatDisp		SpSigMake('A', 'M', 'D', ' ')
#define SpTechPassiveMatDisp		SpSigMake('P', 'M', 'D', ' ')
#define SpTechPhotoCD			SpSigMake('K', 'P', 'C', 'D')
#define SpTechPhotoImageSetter		SpSigMake('i', 'm', 'g', 's')
#define SpTechGravure			SpSigMake('g', 'r', 'a', 'v')
#define SpTechOffsetLithography		SpSigMake('o', 'f', 'f', 's')
#define SpTechSilkscreen		SpSigMake('s', 'i', 'l', 'k')
#define SpTechFlexography		SpSigMake('f', 'l', 'e', 'x')

/***********************************************************************
 * Tag Enumeration value definitions
 ***********************************************************************/

/* Unknown for all enumerated values */
#define SpEnumUnknown		((SpTagEnum_t) 0U)

/* Effect Type */
#define SpEffectInput		((SpTagEnum_t) 1U)
#define SpEffectOutput		((SpTagEnum_t) 2U)
#define SpEffectAdapt		((SpTagEnum_t) 3U)
#define SpEffectTone		((SpTagEnum_t) 4U)
#define SpEffectGreyBalance	((SpTagEnum_t) 5U)
#define SpEffectSelColor	((SpTagEnum_t) 6U)
#define SpEffectNegToPos	((SpTagEnum_t) 7U)
#define SpEffectMultiple	((SpTagEnum_t) 8U)
#define SpEffectPicture		((SpTagEnum_t) 9U)
#define SpEffectMPA			((SpTagEnum_t) 10U)
#define SpEffectLook		((SpTagEnum_t) 11U)

/* Illumination */
#define SpKIllumD50Flourescent	((SpTagEnum_t) 1U)
#define SpKIllumD65Flourescent	((SpTagEnum_t) 2U)
#define SpKIllumTungsten	((SpTagEnum_t) 3U)
#define SpKIllumD50		((SpTagEnum_t) 4U)
#define SpKIllumD65		((SpTagEnum_t) 5U)
#define SpKIllumD93		((SpTagEnum_t) 6U)
#define SpKIllumF2		((SpTagEnum_t) 7U)
#define SpKIllumIncandescent	((SpTagEnum_t) 8U)

/* Interpretation */
#define SpInterpPerceptual	((SpTagEnum_t) 0U)
#define SpInterpColormetric	((SpTagEnum_t) 1U) /* misspelled */
#define SpInterpColorimetric	((SpTagEnum_t) 1U)
#define SpInterpPhotoCD		((SpTagEnum_t) 2U)
#define SpInterpNegative	((SpTagEnum_t) 3U)

/* Medium */
#define SpMediumReflective	((SpTagEnum_t) 1U)
#define SpMediumTransmissive	((SpTagEnum_t) 2U)

/* Medium Sense */
#define SpMediumSensePositive	((SpTagEnum_t) 1U)
#define SpMediumSenseNegative	((SpTagEnum_t) 2U)

/* Sense Invertible */
#define SpSenseIsInvertible	((SpTagEnum_t) 1U)
#define SpSenseNotInvertible	((SpTagEnum_t) 2U)

/* Medium Product */
#define SpMediumKodaChrome	((SpTagEnum_t) 1U)
#define SpMediumEktaColor	((SpTagEnum_t) 2U)
#define SpMediumEktaChrome	((SpTagEnum_t) 3U)
#define SpMediumFujiChrome	((SpTagEnum_t) 4U)
#define SpMediumEktaTherm	((SpTagEnum_t) 5U)
#define SpMediumPaper		((SpTagEnum_t) 6U)
#define SpMediumPhotoPrint	((SpTagEnum_t) 7U)
#define SpMediumTransparency	((SpTagEnum_t) 8U)
#define SpMediumColorNegative	((SpTagEnum_t) 9U)

/* Linearized */
#define SpIsLinearized		((SpTagEnum_t) 1U)
#define SpNotLinearized		((SpTagEnum_t) 2U)

/* Monitor Phosphor */
#define SpPhosphorP22		((SpTagEnum_t) 1U)
#define SpPhosphorEBU		((SpTagEnum_t) 2U)
#define SpPhosphorSMPTE_C	((SpTagEnum_t) 3U)

/* Black Shape */
#define SpBlackShapeAggressive	((SpTagEnum_t) 1U)
#define SpBlackShapeNormal	((SpTagEnum_t) 2U)

/* Black Start Delay */
#define SpBlackDelayShort	((SpTagEnum_t) 1U)
#define SpBlackDelayMedium	((SpTagEnum_t) 2U)
#define SpBlackDelayLong	((SpTagEnum_t) 3U)

#define SpMeasStaA		SpSigMake('S', 't', 'a', 'A')
#define SpMeasStaE		SpSigMake('S', 't', 'a', 'E')
#define SpMeasStaI		SpSigMake('S', 't', 'a', 'I')
#define SpMeasStaT		SpSigMake('S', 't', 'a', 'T')
#define SpMeasStaM		SpSigMake('S', 't', 'a', 'M')
#define SpMeasDN		SpSigMake('D', 'N', ' ', ' ')
#define SpMeasDNP		SpSigMake('D', 'N', ' ', 'P')
#define SpMeasDNN		SpSigMake('D', 'N', 'N', ' ')
#define SpMeasDNNP		SpSigMake('D', 'N', 'N', 'P')

#define SpAppleID		SpSigMake('A', 'P', 'P', 'L')
#define SpMicrosoftID		SpSigMake('M', 'S', 'F', 'T')
#define SpSGIID			SpSigMake('S', 'G', 'I', ' ')
#define SpSunID			SpSigMake('S', 'U', 'N', 'W')
#define SpTaligentID		SpSigMake('T', 'G', 'N', 'T')

#define SpResolutID		SpSigMake('r', 's', 'l', 'n')
#define SpMediaID		SpSigMake('m', 't', 'y', 'p')
#define SpHalftoneID		SpSigMake('h', 'f', 't', 'n')

#define SpMediaStandard		((SpTagEnum_t) 1U)
#define SpMediaTransparency	((SpTagEnum_t) 2U)
#define SpMediaGlossy		((SpTagEnum_t) 3U)
#define SpMediaUSER		((SpTagEnum_t) 256U)

#define SpDitherNone		((SpTagEnum_t) 1U)
#define SpDitherCoarse		((SpTagEnum_t) 2U)
#define SpDitherFine		((SpTagEnum_t) 3U)
#define SpDitherLineArt		((SpTagEnum_t) 4U)
#define SpDitherErrorDif	((SpTagEnum_t) 5U)
#define SpDitherReserved6	((SpTagEnum_t) 6U)
#define SpDitherReserved7	((SpTagEnum_t) 7U)
#define SpDitherReserved8	((SpTagEnum_t) 8U)
#define SpDitherReserved9	((SpTagEnum_t) 9U)
#define SpDitherGrayScale	((SpTagEnum_t) 10U)
#define SpDitherUSER		((SpTagEnum_t) 256U)


#define SP_TNL_CURVE_STRUCT_VER			1
#define SP_CURVE_DEFAULT				0x00
#define SP_CURVE_LOW_CONTRAST			0x01


/********************/
/* System Functions */
/********************/

typedef struct SpDBInfo_s
{
	char			toolkitVersion[10];					/* Version of ICC API */
	char			colorProcessorVersion[10];			/* Version of CP */
	KpInt32_t		acceleratorPresent;					/* Hardware accelerators in system */
}SpDBInfo;

/* Search Engine Structures  */

typedef KpInt32_t SpSearchType_t;

typedef struct SpSearchCriterion_s
{
   SpSearchType_t    SearchElement;
   union
   {
      SpSig_t        Signature;
      KpUInt32_t     Value;
      KpF15d16XYZ_t  XYZ;
      SpDateTime_t   Date;
      struct {
         KpUInt32_t	a;
         KpUInt32_t	b;
         KpUInt32_t	c;
         KpUInt32_t	d;
      } ProfileID;
      struct {
		SpTagId_t	TagId;
        SpTagEnum_t	TagEnum;
      } Tag;
  } SearchValue;
} SpSearchCriterion_t;

typedef struct SpSearch_s
{
   KpInt32_t            critCount;
   SpSearchCriterion_t *criterion;
   KpInt32_t            critSize;
} SpSearch_t, *SpSearch_p;

typedef struct SpDataBaseEntry_s 
{
   SpFileProps_t	fileProps;
   char *		dirName;
} SpDataBaseEntry_t, *SpDataBaseEntry_p;

typedef struct SpDataBase_s
{
   KpInt32_t		numEntries;
   SpDataBaseEntry_p	Entries;
} SpDataBase_t, *SpDataBase_p;

/* control structure for SpInitializeEx */
#if defined (KPWIN)
typedef struct SpInitInfo_s {
	KpUInt32_t	structSize;
	HINSTANCE	appModuleId;
} SpInitInfo_t, FAR* SpInitInfo_p;
#endif

SpStatus_t KSPAPI SpInitCMS (
				SpCallerId_t	FAR *CallerId,
				SpProgress_t 	ProgressFunc,
				void			FAR *Data,
				KpMemoryData_t	MemoryData);

SpStatus_t KSPAPI SpInitialize (
				SpCallerId_t	FAR *CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);

SpStatus_t KSPAPI SpTerminate (
				SpCallerId_t	FAR *CallerId);

#if defined (KPWIN)
SpStatus_t KSPAPI SpInitializeEx (
				SpCallerId_t	FAR *CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data,
				SpInitInfo_t	FAR *InitInfo);
#endif

#if defined (KPMAC) || defined(KPMSMAC)
SpStatus_t KSPAPI SpInitCMSComp (
				KpInt32_t 		CPInstance,
				SpCallerId_t	FAR *CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data,
				KpMemoryData_t	MemoryData);
SpStatus_t KSPAPI SpInitializeComp (
				KpInt32_t 		CPInstance,
				SpCallerId_t	FAR *CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);
SpStatus_t KSPAPI SpTerminateComp (
				SpCallerId_t FAR *CallerId,
				KpInt32_t CPJointInstance);
#endif

SpStatus_t KSPAPI SpInitThread (
				SpCallerId_t	CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);

SpStatus_t KSPAPI SpTermThread (
				SpCallerId_t	CallerId);

SpStatus_t KSPAPI SpGetErrorText (
				SpStatus_t	StatusVal,
				size_t		BufferSize,
				char		FAR *Buffer);


SpStatus_t KSPAPI SpGetInfo(
				SpDBInfo *info,
				KpInt32_t size);

#if defined (KPMAC) || defined(KPMSMAC)
void SpSetICCInstance (KpInt32_t theNewAPI);
void SpGetICCInstance (KpInt32_t * theCurAPI);
#endif

/*****************/
/* Tag Functions */
/*****************/

typedef SpStatus_t (SPCALLBACK *SpTagIter_t) (
											SpIterState_t	State,
											SpProfile_t		Profile,
											SpTagId_t		TagId,
											void			FAR *Data);

SpStatus_t KSPAPI SpTagIter (
				SpProfile_t		Profile,
				SpTagIter_t		TagIterFunc,
				void			FAR *Data);

SpStatus_t KSPAPI SpTagFree (
				SpTagValue_t	FAR *Value);

SpStatus_t KSPAPI SpTagExists (
				SpProfile_t		Profile,
				SpTagId_t		TagId,
				KpBool_t		*TagExists);

SpStatus_t KSPAPI SpTagGetById (
				SpProfile_t		Profile,
				SpTagId_t		TagId,
				SpTagValue_t	FAR *Value);

SpStatus_t KSPAPI SpTagGetByIdEx (
				SpProfile_t		Profile,
				SpTagId_t		TagId,
				SpTagValue_t	FAR *Value);

SpStatus_t KSPAPI SpTagSet (
				SpProfile_t		Profile,
				SpTagValue_t	FAR *Value);

SpStatus_t KSPAPI SpTagDeleteById (
				SpProfile_t		Profile,
				SpTagId_t		TagId);

#if !defined (KP_NO_TEXTFUNCTIONS)
/**********************/
/* Tag Name Functions */
/**********************/

KpInt32_t KSPAPI SpTagIdListSize (
				void);

SpStatus_t KSPAPI SpTagIdListGetEntry (
				KpInt32_t			n,
				SpTagId_t	FAR *TagId);

SpStatus_t KSPAPI SpTagGetIdName (
				SpTagId_t		TagId,
				size_t			BufferSize,
				char			FAR *Buffer);

void KSPAPI SpTagGetIdType (
				SpTagId_t	TagId,
				SpTagType_t	FAR *TagType);

KpInt32_t KSPAPI SpTagTypeListSize (
				void);

SpStatus_t KSPAPI SpTagTypeListGetEntry (
				KpInt32_t	n,
				SpTagType_t	FAR *TagType);

SpStatus_t KSPAPI SpTagGetTypeName (
				SpTagType_t	TagType,
				size_t		BufferSize,
				char		FAR *Buffer);

SpStatus_t KSPAPI SpTagEnumListSize (
				SpTagId_t	TagId,
				KpInt32_t	FAR *Size);

SpStatus_t KSPAPI SpTagEnumListGetEntry (
				SpTagId_t	TagId,
				KpInt32_t	n,
				SpTagEnum_t	FAR *TagEnum);

SpStatus_t KSPAPI SpTagGetEnumName (
				SpTagId_t	TagId,
				SpTagEnum_t	TagEnum,
				size_t		BufferSize,
				char		FAR *Buffer);

#endif
/*********************/
/* Profile Functions */
/*********************/

SpStatus_t KSPAPI SpProfileCreate (
				SpCallerId_t	CallerId,
				SpProfile_t		FAR *Profile);

SpStatus_t KSPAPI SpProfileCreateDT (
				SpCallerId_t	CallerId,
				KpUInt32_t		DataType,
				SpProfile_t		FAR *Profile);

SpStatus_t KSPAPI SpProfileCreateEx (
				SpCallerId_t	CallerId,
				KpUInt32_t		DataType,
				KpUInt32_t		ProfileVersion,
				SpProfile_t		FAR *Profile);

#if !defined (SP_READONLY_PROFILES)
SpStatus_t KSPAPI SpProfileSetDT (
				KpUInt32_t		DataType,
				SpProfile_t		Profile);
#endif

SpStatus_t KSPAPI SpProfileLoadFromBuffer (
				SpCallerId_t	CallerId,
				void			FAR *Data,
				SpProfile_t		FAR *Profile);

#if !defined (SP_NO_FILEIO)
SpStatus_t KSPAPI SpProfileLoadProfile (
				SpCallerId_t	CallerId,
				KpChar_t	*FileName,
				SpFileProps_t	*Props,
				SpProfile_t	FAR *Profile);
#endif /* SP_NO_FILEIO */

SpStatus_t KSPAPI SpProfileSetHeader (
				SpProfile_t Profile,
				SpHeader_t	FAR *Header);

SpStatus_t KSPAPI SpProfileGetHeader (
				SpProfile_t Profile,
				SpHeader_t	FAR *Header);

#if !defined (SP_NO_OBSOLETE)
SpStatus_t KSPAPI SpProfileLoadFromDisk (
				SpCallerId_t	CallerId,
				KpChar_t		*FileName,
				SpIOFileChar_t	*Props,
				SpProfile_t		FAR *Profile);

SpStatus_t KSPAPI SpProfileSaveToDiskEx (
				SpProfile_t		Profile,
				KpChar_t		*Name,
				SpIOFileChar_t	*Props,
				KpBool_t		ShareTags);

SpStatus_t KSPAPI SpProfileSaveToDisk (
				SpProfile_t		Profile,
				KpChar_t		*Name,
				SpIOFileChar_t	*Props);
#endif

#if !defined (SP_NO_FILEIO) || !defined (SP_NO_SEARCH)
SpStatus_t KSPAPI SpProfileLoadHeader(
				char				*Filename,
				SpFileProps_t		*Props,
				SpHeader_t			FAR *Header);
#endif /* !SP_NO_FILEIO || !SP_NO_SEARCH */

#if !defined (SP_NO_FILEIO)
SpStatus_t KSPAPI SpProfileLoadTag(
				char				*Filename,
				SpFileProps_t		*Props,
				SpTagId_t			TagId,
				SpTagValue_t		FAR *Value);

SpStatus_t KSPAPI SpProfileLoadTagEx(
				char				*Filename,
				SpFileProps_t		*Props,
				SpTagId_t			TagId,
				SpTagValue_t		FAR *Value);
#endif /* SP_NO_FILEIO */

SpStatus_t KSPAPI SpProfileCheck(
				SpSearch_p		SearchValue,
				SpHeader_t		FAR *Header);

SpStatus_t KSPAPI SpProfileCheckEx(	
				SpSearch_p			SearchValue,
				char				*Filename,
				SpFileProps_t		*Props);

SpStatus_t KSPAPI SpProfileFree (
				SpProfile_t		FAR *Profile);

#if !defined (SP_NO_FILEIO)
SpStatus_t KSPAPI SpProfileSaveEx (
				SpProfile_t	Profile,
				KpBool_t	ShareTags);

SpStatus_t KSPAPI SpProfileSave (
				SpProfile_t	Profile);

SpStatus_t KSPAPI SpProfileSaveProfileEx (
				SpProfile_t	Profile,
				KpChar_t	*Name,
				SpFileProps_t	*Props,
				KpBool_t	ShareTags);

SpStatus_t KSPAPI SpProfileSaveProfile (
				SpProfile_t	Profile,
				KpChar_t	*Name,
				SpFileProps_t	*Props);

#endif /* !SP_NO_FILEIO */

SpStatus_t KSPAPI SpProfileDelete (
				SpProfile_t		FAR *Profile);

#if !defined (SP_NO_OBSOLETE)
SpStatus_t KSPAPI SpProfileGetDiskName (
				SpProfile_t		Profile,
				size_t			BufferSize,
				KpChar_t		*Buffer,
				SpIOFileChar_t	*Props);

SpStatus_t KSPAPI SpProfileSetDiskName (
				SpProfile_t		Profile,
				KpChar_t		*FileName,
				SpIOFileChar_t	*Props);

SpStatus_t KSPAPI SpProfileGetName (
				SpProfile_t			Profile,
				size_t				BufferSize,
				KpChar_t			*Buffer,
				SpFileProps_t		*Props);

SpStatus_t KSPAPI SpProfileSetName (
				SpProfile_t			Profile,
				KpChar_t			*FileName,
				SpFileProps_t		*Props);
#endif

SpStatus_t KSPAPI SpProfileGetDefaultDB(
				KpInt32_t		numEntries,
				KpInt32_t		FileNameSize,
				SpDataBaseEntry_p	Entries);

SpStatus_t KSPAPI SpProfileSearch(
				SpCallerId_t		CallerId,
				SpDataBase_t		*DataBaseList,
				SpSearch_t		*SearchCriterion,
				SpProfile_t		*profileList,
				KpInt32_t		listSize,
				KpInt32_t		*foundCount);

SpStatus_t KSPAPI SpProfileSearchEx(
				SpCallerId_t		CallerId,
				SpDataBase_t		*DataBaseList,
				SpSearch_t		*SearchCriterion,
				SpProfile_t		*profileList,
				KpInt32_t		listSize,
				KpInt32_t		searchMode,
				KpInt32_t		*foundCount);

SpStatus_t KSPAPI SpProfileSearchGetListSize(
				SpCallerId_t		CallerId,
				SpDataBase_t		*DataBaseList,
				SpSearch_t		*SearchCriterion,
				KpInt32_t		searchMode,
				KpInt32_t		*foundCount);

SpStatus_t KSPAPI SpProfileSearchRefine(
				SpSearch_t    *SearchCriterion,
				SpProfile_t   *profileList,
				KpInt32_t     listSize,
				KpInt32_t     *foundCount);

SpStatus_t KSPAPI SpProfileSaveToBuffer (
				SpProfile_t	Profile,
				KpChar_p	*lpBuffer,
				KpUInt32_p	bufferSize);

SpStatus_t KSPAPI SpProfileGetProfileSize (
				SpProfile_t	Profile,
				KpUInt32_p	Size);

SpStatus_t KSPAPI SpProfileGetProfileId (
				SpProfile_t	Profile,
				KpUInt32_p	ID);

/***********************/
/* Transform Functions */
/***********************/

#define SpMaxTransforms		4

/* Transform types */
#define	SpTransTypeUnknown	0
#define	SpTransTypeIn		1
#define	SpTransTypeOut		2
#define	SpTransTypeGamut	3
#define	SpTransTypeSim		4

typedef KpInt32_t SpTransType_t;

/* Rendering types */
#define	SpTransRenderAny			0
#define	SpTransRenderPerceptual			1
#define	SpTransRenderColormetric		2 /* misspelled */
#define	SpTransRenderColorimetric		2
#define	SpTransRenderSaturation			3
#define	SpTransRenderAbsColormetric		4 /* misspelled */
#define	SpTransRenderAbsColorimetric	4

typedef KpInt32_t SpTransRender_t;

/* ICC Data Types for use with the "DT" functions */
#define SP_ICC_TYPE_CURRENT 0
#define SP_ICC_TYPE_0 1
#define SP_ICC_TYPE_1 2

/* ICC Data Types for use for reading Lut tags */
#define SP_ICC_FUT 0		/* fut lut type */
#define SP_ICC_MFT1 8		/* mft1 (8 bit) lut type */
#define SP_ICC_MFT2 16		/* mft2 (16 bit) lut type */
#define SP_ICC_MAB1 1		/* mab lut type with 8 bits per clut entry */
#define SP_ICC_MAB2 2		/* mab lut type with 16 bits per clut entry */
#define SP_ICC_MBA1 3		/* mba lut type with 8 bits per clut entry */
#define SP_ICC_MBA2 4		/* mba lut type with 16 bits per clut entry */

typedef struct {
	KpInt32_t		LutType;
	SpTransRender_t	WhichRender;
	SpTransType_t	WhichTransform;
	SpSig_t			SpaceIn;
	SpSig_t			SpaceOut;
} SpXformDesc_t;

SpStatus_t KSPAPI SpXformSetData (
				SpProfile_t		Profile,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				KpInt32_t		DataSize,
				SpHugeBuffer_t	Data);

SpStatus_t KSPAPI SpXformGetDataSize (
				SpProfile_t		Profile,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				KpInt32_t		FAR *DataSize);

SpStatus_t KSPAPI SpXformGetData (
				SpProfile_t		Profile,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				KpInt32_t		DataSize,
				SpHugeBuffer_t	Data);

SpStatus_t KSPAPI SpXformGet (
				SpProfile_t		Profile,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformSet (
				SpProfile_t		Profile,
				KpInt32_t		LutType,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				SpXform_t		Xform);

SpStatus_t KSPAPI SpXformFree (
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformGetChannels (
				SpXform_t		Xform,
				KpInt32_t		FAR *In,
				KpInt32_t		FAR *Out);

SpStatus_t KSPAPI SpXformGetDesc (
				SpXform_t		Xform,
				SpXformDesc_t	FAR *Desc);

SpStatus_t KSPAPI SpXformGenerate (
				SpProfile_t		Profile,
				KpUInt32_t		GridSize,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTrans,
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformFromBuffer (
				KpInt32_t	BufferSize,
				SpHugeBuffer_t	Buffer,
				SpSig_t		SpaceIn,
				SpSig_t		SpaceOut,
				SpXform_t	FAR *Xform);

SpStatus_t KSPAPI SpXformFromBufferDT (
				KpInt32_t	DataType,
				KpInt32_t	BufferSize,
				SpHugeBuffer_t	Buffer,
				SpSig_t		SpaceIn,
				SpSig_t		SpaceOut,
				SpXform_t	FAR *Xform);

SpStatus_t KSPAPI SpXformInvert (
				SpXform_t 	Xform, 
				KpBool_t 	invertInp, 
				KpBool_t	invertOut);

SpStatus_t KSPAPI SpXformDuplicate (
				SpXform_t 	srcXform, 
				SpXform_t	FAR *dupXform);

SpStatus_t KSPAPI SpXformToPT(	SpXform_t	spXform,
				KpUInt32_t	LutType,
				KpUInt32_t	Datasize,
				SpHugeBuffer_t	pXformData);
 
SpStatus_t KSPAPI SpXformToBufferDT(
				SpXform_t	spXform,
				KpUInt32_t	LutType,
				KpInt32_t	DataType,
				KpUInt32_t	Datasize,
				SpHugeBuffer_t	pXformData);
 
SpStatus_t KSPAPI SpXformGetPTSize(
				SpXform_t	spXform,
				KpUInt32_t	LutType,
				KpUInt32_t	*Datasize);

SpStatus_t KSPAPI SpXformGetBufferSizeDT(
				SpXform_t	spXform,
				KpUInt32_t	LutType,
				KpInt32_t	DataType,
				KpUInt32_t	*Datasize);

SpStatus_t KSPAPI SpXformCreateFromDataNC (
				KpInt32_t	Size,
				KpLargeBuffer_t	Data,
				SpXform_t	FAR *Xform);

/*****************/
/* Lut Functions */
/*****************/

SpStatus_t KSPAPI SpLut8Create (
				KpUInt16_t	InChannels,
				KpUInt16_t	OutChannels,
				KpUInt16_t	LUTDimensions,
				SpLut_t		FAR *Lut);

SpStatus_t KSPAPI SpLut16Create (
				KpUInt16_t	InChannels,
				KpUInt16_t	InTableEntries,
				KpUInt16_t	OutChannels,
				KpUInt16_t	OutTableEntries,
				KpUInt16_t	LUTDimensions,
				SpLut_t		FAR *Lut);

SpStatus_t KSPAPI SpLutABCreate (
				KpUInt16_t	InChannels,
				KpUInt16_t	OutChannels,
				KpUInt32_t	BCurve,
				KpUInt32_t	Matrix,
				KpUInt32_t	MCurve,
				KpUInt32_t	Clut,
				KpUInt32_t	ACurve,
				SpLut_t		FAR *Lut);

SpStatus_t KSPAPI SpLutBACreate (
				KpUInt16_t	InChannels,
				KpUInt16_t	OutChannels,
				KpUInt32_t	BCurve,
				KpUInt32_t	Matrix,
				KpUInt32_t	MCurve,
				KpUInt32_t	Clut,
				KpUInt32_t	ACurve,
				SpLut_t		FAR *Lut);

SpStatus_t KSPAPI SpLutFree (
				SpLut_t		FAR *Lut);

/**************************************/
/*  TRC/Colorant Generation Function  */
/**************************************/
SpStatus_t KSPAPI SpTagCreateColorantRC (SpProfile_t	Profile, 
					SpPhosphor_t	FAR *PData,
					KpF15d16XYZ_t	FAR *WhitePoint,
					KpF15d16_t	Gamma);

SpStatus_t KSPAPI SpTagCreateColorantRC3Gamma (SpProfile_t	Profile, 
					SpPhosphor_t	FAR *PData,
					KpF15d16XYZ_t	FAR *WhitePoint,
					KpF15d16_t	RedGamma,
					KpF15d16_t	GreenGamma,
					KpF15d16_t	BlueGamma,
					KpInt32_t	Reserved);

/****************************************/
/* Composition and Evaluation Functions */
/****************************************/

/* Pixel types */
#define	SpSampleType_UByte		1
#define	SpSampleType_x555Word	2
#define	SpSampleType_565Word	3
#define	SpSampleType_UShort12	4	/* data is 12 lowest bits of UShort */
#define	SpSampleType_UShort  	5	/* data is 16 bits */
#define	SpSampleType_RGB10   	6	/* data is 10 bits per component */


typedef KpInt32_t SpSampleType_t;

#define SpMaxComponents	10

typedef struct {
	SpSampleType_t	SampleType;
	KpInt32_t		NumCols;		/* # of columns */
	KpInt32_t		NumRows;		/* # of rows */
	KpInt32_t		OffsetColumn;	/* bytes to next column */
	KpInt32_t		OffsetRow;		/* bytes to next row */
	KpInt32_t		NumChannels;	/* # of components (3-rgb, 4-cmyk) */
	SpHugeBuffer_t	BaseAddrs [SpMaxComponents];	/* pointers to first
													pixel of each components */
} SpPixelLayout_t;


/*      Profile List Entry structure    */
typedef struct SpProfListEntry_s {
	SpProfile_t		profile;
	SpTransRender_t		whichRender;
	SpTransType_t		whichTransform;
} SpProfListEntry_t, *SpProfListEntry_p;
 
/*      Device Link Parameter Block structure   */
typedef struct SpDevLinkPB_s {
	KpInt32_t		numProfiles;
	SpProfListEntry_p	pProfileList;
	SpXform_t		xform;
	KpInt32_t		lutSize;
} SpDevLinkPB_t, *SpDevLinkPB_p;

SpStatus_t KSPAPI SpEvaluate (
				SpXform_t		Xform,
				SpPixelLayout_t	FAR *SrcLayout,
				SpPixelLayout_t	FAR *DestLayout,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);

SpStatus_t KSPAPI SpCombineXforms (
				KpInt32_t		XformCnt,
				SpXform_t		FAR *XformsSequence,
				SpXform_t		FAR *Result,
				KpInt32_t		FAR *FailingXform,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);


				
SpStatus_t KSPAPI SpXformCreateFromData (
				KpInt32_t		Size,
				KpLargeBuffer_t	Data,
				SpXform_t		FAR *Xform);


SpStatus_t KSPAPI SpProfileMakeDeviceLink(SpCallerId_t	callerId,
				SpDevLinkPB_p		pDevLinkDesc,
				SpProfile_t		FAR *pProfile);

SpStatus_t KSPAPI SpProfileMakeDeviceLinkEx(SpCallerId_t	callerId,
				SpDevLinkPB_p		pDevLinkDesc,
				KpUInt32_t		ProfileVersion,
				SpProfile_t		FAR *pProfile);

SpStatus_t KSPAPI SpGetMPState(	KpUInt32_p	MP_Available,
				KpUInt32_p	MP_Used);

SpStatus_t KSPAPI SpSetMPState(	KpUInt32_t	MP_Used);

#if defined (KPMACPPC)
#pragma options align=reset
#endif

#if defined (KPWIN32) && !defined (_WIN64)
#pragma pack(pop, SpLevel)
#endif

#if defined(__cplusplus)
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#endif	/* SPROFILE_H */



