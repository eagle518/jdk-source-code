/*
 * @(#)sprof-pr.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This header is used to contain private (implementation)
				types, constants and common includes.

				Created by lsh, September 15, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2002 by Eastman Kodak Company, 
                            all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sprof-pr.h $
		$Logfile: /DLL/KodakCMS/sprof_lib/h/sprof-pr.h $
		$Revision: 16 $
		$Date: 9/25/02 9:52a $
		$Author: Arourke $

	SCCS Revision:
		@(#)sprof-pr.h	1.96 4/16/99

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-2002                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/
#ifndef SPROFPR_H
#define SPROFPR_H

#include "kcms_sys.h"

#if defined (KPSGI) 
#include "cms.h"
#endif /* KPSGI */

#include "attrib.h"
#include "kcmptlib.h"

#include "sprofile.h"
#include "sprofprv.h"

#if !defined (SPGLOBAL)
#define SPGLOBAL	extern
#endif

#if defined(__cplusplus)
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */


typedef struct {
	KpInt32_t	GridSize;
	PTRefNum_t	RefNum;
	KpInt32_t	Render; /* rendering intent */
	SpParadigm_t ParadigmType;
	KpBool_t		Valid;
} SpPTCache_t;

SPGLOBAL KpCriticalFlag_t	SpCacheCritFlag;
SPGLOBAL SpPTCache_t		Sp_uvL2Lab;
SPGLOBAL SpPTCache_t		Sp_Lab2uvL;

/* the following enum and defines are used for fixed conversions only! */
typedef enum
{ 
	SpDir2LAB = 0,
	SpDir2UVL
} SpDirection_t;

#define uvl2labName 	"uvl2lab.pt"
#define lab2uvlPName	"lab2uvlp.pt" /* perceptual */
#define lab2uvlCName	"lab2uvlc.pt" /* colormetric */
#define lab2uvlSName	"lab2uvls.pt" /* saturation */

KpBool_t useFixed(void);
KpInt32_t getPTFromFile(SpDirection_t direction, KpInt32_t Render, PTRefNum_t *thePT);
void displayWarning(KpChar_p message);

/* end of fixed conversions stuff */

/* the following stuff if for stdprof debugging */

#if defined KCP_DIAG_LOG
/* write a string to the diagnostic log file */
void kcpDiagLog (KpChar_p	string);
#endif

#if defined(_DEBUG)
KpBool_t writePTs(void);
#else
#define writePTs() KPFALSE
#endif

/* end of debugging stuff */

#define SPNumElem(Array) (sizeof (Array) / sizeof (*Array))
#define SPArgUsed(x)	x = x
#define Stringize2(s)	#s
#define Stringize(s)	Stringize2(s)

/* 68K floating point processor defines */
#define	SpNoFPU	0
#define	SpFoundFPU	1

/* Base number of TagArray elements to allocate space for */
#define SPNUMTAGS 20

/* Defines for Maximum Lut Sizes */
#define SPMAXLUTSIZE 	256
#define SPMAXTABLESIZE 	4096

/* Defines the default data type for the luts from the profiles */
#define NO_DT_ICC_TYPE SP_ICC_TYPE_0

/* Size of curve created by SpXformCreateMatTags fns */
#define SHAPERCURVESIZE 256		/* size of the curve */


/***********************************************************************
 * Internal Instance Structure
 ***********************************************************************/
typedef struct SpInstanceGlobals_s
{
	KpUInt32_t	currentUsers;
	KpUInt32_t	NextUsers;
} SpInstanceGlobals_t, FAR* SpInstanceGlobals_p;

SPGLOBAL KpThreadMemHdl_t		ICCRootInstanceID;
/***********************************************************************
 * Internal Caller ID data structures
 ***********************************************************************/

#define SpCallerIdDataSig	SpSigMake ('c', 'a', 'l', 'l')
typedef struct {
	SpSig_t		Signature;
	KpInt32_t	CallerId;
} SpCallerIdData_t;

/***********************************************************************
 * Internal Transform data structures
 ***********************************************************************/

#define SpXformDataSig	SpSigMake ('x', 'f', 'o', 'r')
typedef struct {
	SpSig_t			Signature;
	PTRefNum_t		PTRefNum;
	KpInt32_t		LutType;
	KpInt32_t		LutSize;
	SpTransRender_t	WhichRender;
	SpTransType_t	WhichTransform;
	KpInt32_t		SpaceIn;
	KpInt32_t		SpaceOut;
	KpUInt32_t		ChainIn;
	KpUInt32_t		ChainOut;
	KpBool_t		HdrWPValid;
	KpF15d16XYZ_t		HdrWtPoint;
	KpBool_t		MedWPValid;
	KpF15d16XYZ_t		MedWtPoint;
} SpXformData_t;

/***********************************************************************
 * Internal Profile data structures
 ***********************************************************************/

typedef struct {
	KpUInt32_t	Id;
	KpUInt32_t	Offset;
	KpUInt32_t	Size;
} SpTagRecord_t;

typedef struct SpTagDirEntry_tag {
	SpTagId_t	TagId;
	KpHandle_t	TagData;  /* Handle to type void KPHUGE * */
	KpInt32_t	TagDataSize;
} SpTagDirEntry_t;

#define SpProfileDataSig	SpSigMake ('p', 'r', 'o', 'f')

#if defined(KPSGI)
typedef struct {
	SpSig_t		signature;
	CMSContext	ctxt;
	CMSProfile	prof;
} SpProfileData_t;

#else
typedef struct {
	SpSig_t				Signature;
	SpCallerId_t		CallerId;
	SpHeader_t			Header;
	KpInt32_t			TotalCount;		/* Total number of tags					*/
	KpInt32_t			FreeIndex;		/* Index of first free element			*/
	KpHandle_t			TagArray;		/* Handle to array of SpTagDirEntry_t's	*/
	KpHandle_t			FileName;		/* File name (path NOT included)		*/
	SpFileProps_t		Props;			/* File information for the Macintosh	*/
	KpInt32_t			LockCount;		
	KpInt32_t			ProfileSize;	/* size of profile or header */
	KpBool_t			ProfChanged;	/* Tag added or deleted */
} SpProfileData_t;
#endif	/* KPSGI */

typedef struct {
    SpCallerId_t	CallerId;
    SpSearch_t		*SearchCriterion;
    SpProfile_t		*profileList;
    KpInt32_t		listSize;
    KpInt32_t		CurrentIndex;
    SpStatus_t		RetStatus;
    KpInt32_t		GetALL;
} Limits_t;


/************************************
 * Static Functions raised to separate 
 * functions to reduce library size
 ************************************/
SpStatus_t SetWtPt(SpProfile_t Profile,
		SpXformData_t *XformData);
SpStatus_t SpSetColorSpaceICC2Kp (
		PTRefNum_t RefNum,
		KpInt32_t  AttrNum,
		KpInt32_t  SpColorSpace);
KpBool_t SpTagShare (
		SpTagDirEntry_t FAR *TagArray,
		KpUInt32_t           Index,
		SpTagRecord_t   FAR *TagRecords,
		SpTagRecord_t   FAR *TagRecord);

/************************************/
/* Profile implementation functions */
/************************************/

SpProfileData_t	FAR *SpProfileLock (
				SpProfile_t	Profile);

void SpProfileUnlock (
				SpProfile_t	Profile);

SpStatus_t SpProfileAlloc (
				SpCallerId_t	CallerId,
				SpProfile_t		FAR *Profile,
				SpProfileData_t	FAR * FAR *ProfileDataPtr);

KpBool_t KSPAPI SpIsICCProfile(	char		*Filename,
				SpFileProps_t	*Props);

/**************************************/
/* Transform implementation functions */
/**************************************/

KpInt32_t SpConnectTypeToPTCombineType (
				SpConnectType_t	ConnectType);

SpConnectType_t SpGetConnectType (
				void);

SpXformData_t FAR *SpXformLock (
				SpXform_t	Xform);

void SpXformUnlock (
				SpXform_t	Xform);

SpStatus_t SpXformAllocate (
				SpXform_t	FAR *Xform);

SpStatus_t SpXformLoadImp (
				KpLargeBuffer_t	Data,
				KpInt32_t		Size,
				KpInt32_t		SpDataType,
				SpSig_t			SpaceIn,
				SpSig_t			SpaceOut,
				PTRefNum_t		FAR *RefNum);

SpStatus_t SpXformFromPTRefNumImp (
				PTRefNum_t	RefNum,
				SpXform_t	FAR *Xform);

SpStatus_t SpColorSpaceKp2Sp (
				KpInt32_t	KpColorSpace,
				KpInt32_t	FAR *SpColorSpace);

SpStatus_t SpColorSpaceSp2Kp (
				KpInt32_t	SpColorSpace,
				KpInt32_t	FAR *KpColorSpace);

KpInt32_t SpGetChannelsFromColorSpace (
				KpInt32_t	ColorSpace);

SpStatus_t SpXformSetColorSpace (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum,
				KpInt32_t	SpColorSpace);

SpStatus_t SpXformInitColorSpace (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum,
				KpInt32_t	SpColorSpace);

SpStatus_t SpXformGetColorSpace (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum,
				KpInt32_t	FAR *ColorSpace);

SpStatus_t SpConnectSequenceImp (
				SpConnectType_t	ConnectType,
				KpInt32_t		RefNumCnt,
				PTRefNum_t		FAR *RefNumSequence,
				PTRefNum_t		FAR *Result,
				KpInt32_t		FAR *FailingRefNum,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);

SpStatus_t SpConnectSequenceCombine (	/* connect the sequence with Combine only - no chaining */
				SpConnectType_t	ConnectType,
				KpInt32_t		RefNumCnt,
				PTRefNum_t		FAR *RefNumSequence,
				PTRefNum_t		FAR *Result,
				KpInt32_t		FAR *FailingRefNum,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);
SpStatus_t SpXformBuildCnvrt (
				KpBool_t		Lab2uvL,
				KpInt32_t		Render,
				SpConnectType_t	ConnectType,
				SpParadigm_t	ParadigmType,
				PTRefNum_t		FAR *RefNum);

SpStatus_t SpDTtoKcmDT (
				KpInt32_t	SpDataType,
				KpInt32_p	KcmDataType);

/**************************************/
/* Floating Point functions  for      */
/* uvL/LAB fut generation SW          */
/**************************************/
KpBool_t SpWhichParadigm(
				SpParadigm_t 	ParadigmType, 
				KpInt32_t Render);
KpInt32_t LAB_to_uvL (
				PTRefNum_t		FAR *aPT,
				KpInt32_t		Render,
				SpParadigm_t 	ParadigmType,
				KpInt32_t		GridSize);
KpInt32_t UVL_to_lab (
				PTRefNum_t		FAR *aPT,
				KpInt32_t		Render,
				SpParadigm_t 	ParadigmType,
				KpInt32_t		GridSize);

/**************************************/
/* Floating Point functions  for      */
/* ComputeShaperMatrix SW             */
/**************************************/

SpStatus_t Transform12BPels (
				PTRefNum_t	pt,
				KpUInt16_t	FAR *Pels,
				int		nPels);

SpStatus_t TransformPels (
				PTRefNum_t	pt,
				KpUInt8_t	FAR *Pels,
				int			nPels);

/********************************/
/* Tag implementation functions */
/********************************/
KpInt32_t SpTagFindById (
				SpTagDirEntry_t	FAR *TagArray,
				SpTagId_t	TagId,
				KpInt32_t	TotalCount);


SpStatus_t KSPAPI SpTagToPublic (
				SpHeader_t		FAR *Header,
				SpTagId_t		TagId,
				KpUInt32_t		TagDataSize,
				void			KPHUGE *TagData,
				SpTagValue_t	FAR *Value);

SpStatus_t KSPAPI SpHeaderToPublic (
				char		KPHUGE *Ptr,
				KpUInt32_t	BufferSize,
				SpHeader_t	FAR *Header);

SpStatus_t KSPAPI SpTagFromPublic (
				SpHeader_t		FAR *Header,
				SpTagValue_t	FAR *Value,
				KpUInt32_t		FAR *BufferSize,
				void			KPHUGE * FAR *Buffer);

SpStatus_t KSPAPI SpHeaderFromPublic (
				SpHeader_t	FAR *Header,
				KpUInt32_t	BufferSize,
				char		KPHUGE *Buffer);

SpStatus_t SpTagDirEntryAdd (
				SpProfileData_t FAR *ProfileData,
				SpTagId_t		TagId,
				KpUInt32_t		DataSize,
				void			KPHUGE *Data);
				
KpInt32_t SpTagGetCount (
				SpProfileData_t	FAR *ProfileData);

SpStatus_t SpProfileValidate (
				SpProfileData_t FAR *ProfileData);

/*****************************/
/* Data conversion functions */
/*****************************/

void SpPutBytes (
				char		KPHUGE * FAR *Ptr,
				KpUInt32_t	Size,
				void		KPHUGE *Data);

void SpPutUInt16 (
				char		KPHUGE * FAR *Ptr,
				KpUInt16_t	Value);

void SpPutUInt16s (
				char		KPHUGE * FAR *Ptr,
				KpUInt16_t	KPHUGE *Values,
				KpUInt32_t	Count);

void SpPutUInt32 (
				char		KPHUGE * FAR *Ptr,
				KpUInt32_t	Value);

void SpPutUInt32s (
				char		KPHUGE * FAR *Ptr,
				KpUInt32_t	KPHUGE *Values,
				KpUInt32_t	Count);

void SpPutF15d16 (
				char		KPHUGE * FAR *Ptr,
				KpF15d16_t	KPHUGE *Values,
				KpUInt32_t	Count);

void SpPutF15d16XYZ (
				char			KPHUGE * FAR *Ptr,
				KpF15d16XYZ_t	FAR *XYZ);

void SpPutF1d15 (
				char		KPHUGE * FAR *Ptr,
				KpF1d15_t	KPHUGE *Values,
				KpUInt32_t	Count);

void SpPutF1d15XYZ (
				char			KPHUGE * FAR *Ptr,
				KpF1d15XYZ_t	FAR *Value);

void SpGetBytes (
				char		KPHUGE * FAR *Ptr,
				void		KPHUGE *Value,
				KpUInt32_t	Size);

KpUInt16_t SpGetUInt16 (
				char		KPHUGE * FAR *Ptr);

void SpGetUInt16s (
				char		KPHUGE * FAR *Ptr,
				KpUInt16_t	KPHUGE *Values,
				KpUInt32_t	Count);

KpUInt32_t SpGetUInt32 (
				char		KPHUGE * FAR *Ptr);

void SpGetUInt32s (
				char		KPHUGE * FAR *Ptr,
				KpUInt32_t	KPHUGE *Values,
				KpUInt32_t	Count);

void SpGetF15d16 (
				char		KPHUGE * FAR *Ptr,
				KpF15d16_t	KPHUGE *Value,
				KpUInt32_t	Count);

void SpGetF15d16XYZ (
				char			KPHUGE * FAR *Ptr,
				KpF15d16XYZ_t	FAR *XYZ);

void SpGetF1d15 (
				char			KPHUGE * FAR *Ptr,
				KpF1d15_t	KPHUGE *Values,
				KpUInt32_t	Count);

void SpGetF1d15XYZ (
				char			KPHUGE * FAR *Ptr,
				KpF1d15XYZ_t	FAR *XYZ);


/********************************/
/* Lut implementation functions */
/********************************/

void KSPAPI SpTagGetType (
				KpUInt32_t	ProfVers,
				SpTagId_t	TagId,
				SpTagType_t	FAR *TagType);

SpStatus_t SpCurveToPublic (
				char		KPHUGE * FAR *Buf,
				SpCurve_t	FAR *Curve);

SpStatus_t SpParaCurveToPublic (
				char		KPHUGE * FAR *Buf,
				SpParaCurve_t	FAR *ParaCurve);

SpStatus_t SpParaCurveDataToPublic (
				char		KPHUGE * FAR *Buf,
				SpParaCurveData_t	FAR *ParaCurve);

SpStatus_t SpTagTestLut(
				SpTagId_t	TagId,
				void		*TagData);

SpStatus_t SpLutToPublic (
				char	KPHUGE *buf,
				SpLut_t FAR *Lut);

SpStatus_t SpLutFromPublic (
				SpLut_t		FAR *Lut,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer);

SpStatus_t SpRespFromPublic (	SpResCurveTag_t	FAR *ResCurve,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer);
SpStatus_t SpRespToPublic (	KpUInt32_t	BufferSize,
				char		KPHUGE *buf,
				SpResCurveTag_t	FAR *ResCurve);

SpStatus_t SpDevSetFromPublic (	SpDevStruct_t	FAR *DevSet,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer);
SpStatus_t SpDevSetToPublic (	KpUInt32_t	BufferSize,
				char		KPHUGE *buf,
				SpDevStruct_t   FAR *DevSet);

SpStatus_t SpChromFromPublic (	SpChromaticity_t	FAR *Chrom,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer);
SpStatus_t SpChromToPublic (	KpUInt32_t	BufferSize,
				char		KPHUGE *buf,
				SpChromaticity_t        FAR *Chrom);

/*********/
/* Misc. */
/*********/

SpStatus_t KSPAPI SpProfSetIOFileData (
				SpIOFileChar_t	*src,
				SpIOFileChar_t	*dest);

#if defined (KPMAC)
SpStatus_t KSPAPI SpProfSetSpFileProps(
				SpFileProps_t	*src,
				SpFileProps_t	*dest);

SpStatus_t KSPAPI SpProfSetIOFileProps(
				SpIOFileChar_t	*src,
				SpFileProps_t	*dest);

SpStatus_t KSPAPI SpProfSetSpFileData (
				SpFileProps_t	*src,
				SpIOFileChar_t	*dest);

SpStatus_t KSPAPI SpProfileClearProps (
				SpFileProps_t   *dest);
#endif

KpBool_t SpStrAppend (
				size_t	BufferSize,
				char	FAR *Buffer,
				char	FAR *Str);

SpStatus_t SpDoProgress (
				SpProgress_t	ProgressFunc,
				SpIterState_t	State,
				KpInt32_t		Percent,
				void			FAR *Data);

SpStatus_t SpCallerIdValidate (
				SpCallerId_t	CallerId);

SpStatus_t SpSetKcmAttrInt (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum,
				KpInt32_t	AttrValue);

KpInt32_t SpGetKcmAttrInt (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum);


void SpFree (void FAR *Ptr);

void FAR *SpMalloc (KpInt32_t Size);

KpBool_t TestFileCB(KpfileDirEntry_t FAR *fileSearch,
                    Limits_t             *Limits);

KpInt32_t TestHeaderDate(SpDateTime_t *ValA, SpDateTime_t *ValB);

SpStatus_t KSPAPI SpProfileSaveOutData (
				SpProfile_t	Profile,
				KpFileId	fd,
				KpBool_t	ShareTags);
 
SpStatus_t KSPAPI SpProfileOrderList(
				SpProfile_t	*profileList,
				KpInt32_t	foundCount);

SpStatus_t SpXformGetDataFromCP(
				PTRefNum_t	PTRefNum,
				KpInt32_t	LutType,
				KpInt32_t	DataType,
				KpUInt32_t	FAR *DataSize,
				SpHugeBuffer_t	FAR *Data);


SpStatus_t TextToString(SpTagValue_t	*TagValue,
			KpInt32_p	BufSize,
			KpChar_p	pBuffer);

SpStatus_t TextDescToString(SpTagValue_t	*TagValue,
			    KpInt32_p		BufSize,
			    KpChar_p		Buffer);

SpStatus_t MultiLangToMLString(SpTagValue_t	*TagValue,
			KpInt16_p	Language,
			KpInt16_p	Country,
			KpInt32_p	BufSize,
			KpChar_p	Buffer);

SpStatus_t SignatureToTxt(KpInt32_t	value,
			  KpInt32_p	BufSize,
			  KpChar_p	Buffer);

SpStatus_t F15d16ToTxt(KpF15d16_t	value,
		       KpInt32_p	BufSize,
		       KpChar_p		Buffer);

SpStatus_t F15d16sToTxt(KpUInt32_t	count,
			KpF15d16_t	FAR *Values,
			KpInt32_p	BufSize,
			KpChar_p	Buffer);

SpStatus_t F15d16XYZToTxt(KpF15d16XYZ_t	value,
			  KpInt32_p	BufSize,
			  KpChar_p	Buffer);

KpTChar_p Ultoa(KpUInt32_t	Value,
		KpTChar_p	String,
		int		Radix);

SpStatus_t UInt32ToTxt( KpUInt32_t	Value,
			KpInt32_p	BufSize,
			KpChar_p	Buffer);

SpStatus_t UInt32sToTxt(KpUInt32_t	count,
			KpUInt32_t	FAR *Values,
			KpInt32_p	BufSize,
			KpChar_p	Buffer);

SpStatus_t SpProfileLoadFromBufferImp (
			SpProfileData_t FAR *ProfileData,
			char		KPHUGE *BaseAddr);

SpStatus_t SpProfilePopTagArray(SpProfileData_t *pProfile);

SpStatus_t SpProfileSetLinkHeader(
			SpProfile_t	pProfile,
			SpDevLinkPB_p	pDevLinkDesc);
 
SpStatus_t SpProfileSetLinkDesc(
			SpProfile_t	pProfile,
			SpDevLinkPB_p	pDevLinkDesc);
 
SpStatus_t SpProfileSetLinkSeqDesc(
			SpProfile_t	pProfile,
			SpDevLinkPB_p	pDevLinkDesc);
 
SpStatus_t SpProfileCreateSeqRecord(
			SpProfile_t			pProfile,
			SpProfileSeqDescRecord_t	*pSeqRecord);
 
SpStatus_t SpProfileSetLinkMLDesc(
			SpProfile_t	pProfile,
			SpDevLinkPB_p	pDevLinkDesc);
 
SpStatus_t SpProfileSetLinkMLSeqDesc(
			SpProfile_t	pProfile,
			SpDevLinkPB_p	pDevLinkDesc);
 
SpStatus_t SpProfileCreateMLSeqRecord(
			SpProfile_t			pProfile,
			SpProfileSeqDescRecord2_t	*pSeqRecord);
 
void SpFreeTextDesc(	SpTextDesc_t	*pTextDesc);
void SpFreeMultiLang(	SpMultiLang_t	*pMultiLang);

SpStatus_t SpFreeDeviceDesc(SpDeviceDesc_t *DeviceDesc);

void SpRespFree (	SpResCurveTag_t	*pRespTag);
void SpDevSetFree (	SpDevStruct_t	*pDevSet);
 
void SpGetStringFromSig(KpInt32_t       signature,
                        KpTChar_p       pString);

SpStatus_t SpStatusFromPTErr(PTErr_t PTErr);

void SpCurveToResponseRec ( KpResponse_t *Curve, ResponseRecord_p ResRec);

/****************************/
/*  Post Script Functions   */
/****************************/
#if defined (PSFUNC)
SpStatus_t KSPAPI SpPSCRDCreate( SpProfile_t     Profile,
                                SpTransRender_t Rendering,
                                void KPHUGE    **RawTag,
                                KpUInt32_t      *RawSize);
 
SpStatus_t KSPAPI SpPSCSACreate( SpProfile_t     Profile,
                                SpTransRender_t Rendering,
                                void KPHUGE    **RawTag,
                                KpUInt32_t      *RawSize);
#endif

/********************/
/* Matrix functions */
/********************/
#define SP_MATRIX_MAX_DIM 3

/* define a matrix */
typedef struct SpMatrix_s {
	KpInt32_t	nRows;			/* number of rows in this matrix */
	KpInt32_t	nCols;			/* number of columns in this matrix */
	double		coef[SP_MATRIX_MAX_DIM][SP_MATRIX_MAX_DIM];	/* the matrix coefficients */

} SpMatrix_t, FAR* SpMatrix_p, FAR* FAR* SpMatrix_h;

/* function prototypes */
KpInt32_t	SpMatMul (SpMatrix_p, SpMatrix_p, SpMatrix_p);
KpInt32_t	SpMatDotMul (SpMatrix_p, SpMatrix_p, SpMatrix_p);
KpInt32_t	SpMatDotDiv (SpMatrix_p, SpMatrix_p, SpMatrix_p);
KpInt32_t	SpMatCopy (SpMatrix_p, SpMatrix_p);
KpInt32_t	SpMatZero (SpMatrix_p);
SpStatus_t SolveMat (double	FAR *FAR *mat, int	DimR, int DimC);
double MakeGamma (double g, double x);

#if defined(__cplusplus)
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#endif  /*define SPROFPR_H */
     
     
     
