/*
 * @(#)kcptmgrd.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	kcptmgrd.h

	Contains:		Kodak Color Management System interface file.

	Copyright:		(c) 1992-2000 Eastman Kodak Company, all rights reserved.
*/

#ifndef KCMCPR_D_H_
#define KCMCPR_D_H_ 1

#if defined (KPMACPPC)
#pragma options align=mac68k	/* make sure these structures are 68k type for backward compatibility */
#endif

/* Note: negative numbers may not be used due to Component Manager requirements */
#define	PTCHECKIN 1				/* Check in a PT */
#define	PTCHECKOUT 2			/* Check out a PT */
#define	PTACTIVATE 3			/* Activate a PT */
#define	PTDEACTIVATE 4			/* DeActivate a PT */
#define	PTGETATTRIBUTE 5		/* Get the PT's attributes */
#define	PTSETATTRIBUTE 6		/* Set the PT's attributes */
#define	PTGETSIZE 7				/* get external PT size */
#define	PTGETPT 8				/* get PT to external memory block */
#define	PTEVAL 9				/* evaluation a PT function */
#define	PTEVALRDY 10			/* is the evaluator ready for another PT ? */
#define	PTEVALCANCEL 11			/* kill an evaluation */
#define	PTCOMBINE 12			/* composition */
#define	PTCHAINVALIDATE 13		/* validate a composition list */
#define	PTCHAININIT 14			/* define a composition list */
#define	PTCHAIN 15				/* compose next PT in list */
#define	PTCHAINEND 16			/* get composed PT */
#define	PTNEWEMPTY 17			/* create a PT with non-initialized tables */
#define	PTEVALUATORS 18			/* number of evaluator boards */
#define	PTPROCESSORRESET 19		/* Reset everything */
#define	PTGETTAGS 20			/* number of attribute tags in PT */
#define	PTGETPTINFO 21			/* get PT info from checkin list */
#define	PToldPT 22				/* make a PT from properly formatted tables */
#define	PTCTEGRIDSIZE 23		/* get size of grid tables on CTE */
#define	PTPROCESSORSETUP 24		/* initialize the cte */
#define	PTINITIALIZE 25			/* initialize the color processor */
#define	PTTERMINATE 26			/* reset when application terminates */
#define	PTGETITBL 27			/* get an input table of a PT */
#define	PTGETGTBL 28			/* get a grid table of a PT */
#define	PTGETOTBL 29			/* get an output table of a PT */
#define	PTAPPLINITIALIZE 30		/* initialize the color processor called from the bridge */
#define	PTNEWEMPTYSEP 31		/* create a separable PT with non-initialized tables */
#define	PTSETHOSTA5	32			/* compatable with filter stuff */
#define	PTGETXFORMSIZE 33		/* get external XFORM size */
#define	PTGETXFORM 34			/* get XFORM to external memory block */
#define	PTEVALDT 35				/* evaluation a PT function */
#define	PTEVALPT 36				/* evaluation a PT function with a PT (not at fut) */
#define	PTGETSIZEF 37			/* get external PTF size */
#define	PTGETPTF 38				/* get PTF to external memory block */
#define	PTNEWMATGAMPT 39		/* Make a PT from a matrix and a set of 1D gamma tables */
#define	PTCHAININITM 40			/* define a composition list with a mode */
#define	PTNEWMATGAMAIPT 41		/* PTNEWMATGAMPT with adaptation and interpolation control */
#define	PTINITGLUE 42			/* initialize the color processor glue */
#define	PTTERMGLUE 43			/* terminate the color processor glue */
#define	PTNEWMONOPT 44			/* Make a gray scale PT from a matrix and a set of 1D gamma tables */
#define	PTGETFLAVOR 45			/* Get the type of image evaluations from the CP */
#define	PTGETRELTOABSPT 46		/* PTGETRELTOABSPT */
#define	PTICMGRIDSIZE 47		/* get size of grid tables on CTE for the ICM */
#define	PTCOLORSPACE 48			/* set the input color space for next pteval */
#define	PTINVERT 49				/* Invert the PT - moved form sprofile */
#define	PTGETMPSTATE 50			/* get the current MP state */
#define	PTSETMPSTATE 51			/* set the current MP state */
#define	PTCREATETRC 52			/* set TRC from single gamma */
#define	PTINITCMS 53			/* initialize the cp with Application memory functions */
#define	PTRESETCMS 54			/* reset the cp with kcms_sys memory functions */
#define	PTGETAUXPT 55			/* get an auxiliary PT */

typedef struct PTCheckInParam_s {
	PTRefNum_p	PTRefNum;		/* PT reference number */
	PTAddr_t	PTAddr;			/* PT address */
} PTCheckInParam_t, FAR* PTCheckInParam_p;

typedef struct PTActivateParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	KpInt32_t	mBlkSize;		/* size of the PT in external memory */
	PTAddr_t	PTAddr;			/* PT address */
} PTActivateParam_t, FAR* PTActivateParam_p;

typedef struct PTDeActivateParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
} PTDeActivateParam_t, FAR* PTDeActivateParam_p;

typedef struct PTCheckOutParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
} PTCheckOutParam_t, FAR* PTCheckOutParam_p;

typedef struct PTGetPTInfoParam_s {
	PTRefNum_t		PTRefNum;	/* PT reference number */
	PTAddr_p FAR*	PTHdr;		/* PT header */
	PTAddr_p FAR*	PTAttr; 	/* PT attributes */
	PTAddr_p FAR*	PTData;		/* PT data */
} PTGetPTInfoParam_t, FAR* PTGetPTInfoParam_p;

typedef struct PTGetAttributeParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	KpInt32_t	attributeTag;
	KpInt32_p	size;
	KpChar_p	attribute;
} PTGetAttributeParam_t, FAR* PTGetAttributeParam_p;

typedef struct PTSetAttributeParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	KpInt32_t	attributeTag;
	KpChar_p	attribute;
} PTSetAttributeParam_t, FAR* PTSetAttributeParam_p;

typedef struct PTGetTagsParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	KpInt32_p	nTags;
	KpInt32_p	tagArray;
} PTGetTagsParam_t, FAR* PTGetTagsParam_p;

typedef struct PTGetSizeParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	KpInt32_p	mBlkSize;		/* size of the PT in external memory */
} PTGetSizeParam_t, FAR* PTGetSizeParam_p;

typedef struct PTGetSizeFParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	PTType_t	PTType;			/* Type of format to store PT */
	KpInt32_p	mBlkSize;		/* size of the PT in external memory */
} PTGetSizeFParam_t, FAR* PTGetSizeFParam_p;

typedef struct PTGetPTParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	KpInt32_t	mBlkSize;		/* size of the PT in external memory */
	PTAddr_t	PTAddr;			/* external PT address */
} PTGetPTParam_t, FAR* PTGetPTParam_p;

typedef struct PTGetPTFParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */
	PTType_t	PTType;			/* Type of format to store PT */
	KpInt32_t	mBlkSize;		/* size of the PT in external memory */
	PTAddr_t	PTAddr;		/* external PT address */
} PTGetPTFParam_t, FAR* PTGetPTFParam_p;

typedef struct PTEvalRdyParam_s {
	opRefNum_t	opRefNum;		/* reference # for this operation */
	KpInt32_p	progress;		/* evaluation progress */
} PTEvalRdyParam_t, FAR* PTEvalRdyParam_p;

typedef struct PTEvalParam_s {
	PTRefNum_t		PTRefNum;	/* PT reference number */
	PTEvalPB_p		evalDef;	/* address of evaluation parameter block */
	PTEvalTypes_t	evalID;		/* evaluator ID */
	KpInt32_t		devNum;		/* evaluator # */
	KpInt32_t		aSync;		/* synchronous/asynchronous flag */
	opRefNum_p		opRefNum;	/* reference # for this operation */
	PTProgress_t	progress;	/* progress callback function */
#if defined(KPMAC)
	long			hostA5;
	long			hostA4;
#endif
} PTEvalParam_t, FAR* PTEvalParam_p;

typedef struct PTEvalDTParam_s {
	PTRefNum_t		PTRefNum;	/* PT reference number */
	PTEvalDTPB_p	evalDef;	/* address of evaluation parameter block */
	PTEvalTypes_t	evalID;		/* evaluator ID */
	KpInt32_t		devNum;		/* evaluator # */
	KpInt32_t		aSync;		/* synchronous/asynchronous flag */
	opRefNum_p		opRefNum;	/* reference # for this operation */
	PTProgress_t	progress;	/* progress callback function */
#if defined(KPMAC)
	long			hostA5;
	long			hostA4;
#endif
} PTEvalDTParam_t, FAR* PTEvalDTParam_p;

typedef struct PTEvalCancelParam_s {
	opRefNum_t	opRefNum;		/* reference # for this operation */
} PTEvalCancelParam_t, FAR* PTEvalCancelParam_p;

typedef struct PTCombineParam_s {
	KpInt32_t	mode;			/* compose mode */
	PTRefNum_t	PTRefNum1;		/* PT reference number, PT 1 */
	PTRefNum_t	PTRefNum2;		/* PT reference number, PT 2 */
	PTRefNum_p	PTRefNumR;		/* PT reference number, resultant PT */
} PTCombineParam_t, FAR* PTCombineParam_p;

typedef struct PTChainValidateParam_s {
	KpInt32_t	nPT;			/* number of PT's */
	PTRefNum_p	PTList;			/* list of PT's */
	KpInt32_p	index;			/* index of PT at which error occurred */
} PTChainValidateParam_t, FAR* PTChainValidateParam_p;

typedef struct PTChainInitParam_s {
	KpInt32_t	nPT;			/* number of PT's */
	PTRefNum_p	PTList;			/* list of PT's */
	KpInt32_t	validate;		/* validate/no validate flag */
	KpInt32_p	index;			/* index of PT at which error occurred */
} PTChainInitParam_t, FAR* PTChainInitParam_p;

typedef struct PTChainInitMParam_s {
	KpInt32_t	nPT;			/* number of PT's */
	PTRefNum_p	PTList;			/* list of PT's */
	KpInt32_t	compMode;		/* composition mode */
	KpInt32_t	rulesKey;		/* key to enable chainning rules */
} PTChainInitMParam_t, FAR* PTChainInitMParam_p;

typedef struct PTChainParam_s {
	PTRefNum_t	PTRefNum;		/* reference # of next PT to chain */
} PTChainParam_t, FAR* PTChainParam_p;

typedef struct PTChainEndParam_s {
	PTRefNum_p	PTRefNum;		/* reference # of resultant PT */
} PTChainEndParam_t, FAR* PTChainEndParam_p;

typedef struct PTEvaluatorsParam_s {
	KpInt32_p	nEval;			/* number of evaluator types */
	evalList_p	evalList;		/* type and number of type */
} PTEvaluatorsParam_t, FAR* PTEvaluatorsParam_p;

typedef struct PTNewEmptyParam_s {
	KpInt32_t	ndim;			/* number of dimensions for each channel */
	KpInt32_p 	dim;			/* list of dimension sizes */
	KpInt32_t	nchan;			/* number of output channels */
	PTRefNum_p	PTRefNum;		/* returned PT reference number */ 
} PTNewEmptyParam_t, FAR* PTNewEmptyParam_p;

typedef struct PTNewEmptySepParam_s {
	KpInt32_t	nchan;			/* number of output channels */
	KpInt32_p 	dim;			/* list of dimension sizes */
	PTRefNum_p	PTRefNum;		/* returned PT reference number */ 
} PTNewEmptySepParam_t, FAR* PTNewEmptySepParam_p;

typedef struct PTGetItblParam_s {
	PTRefNum_t	PTRefNum;		/* PT reference number */ 
	KpInt32_t	ochan;			/* output channel selector */
	KpInt32_t	ichan;			/* input channel selector */
	KcmHandle FAR*	itblDat;	/* returned table address */
} PTGetItblParam_t, FAR* PTGetItblParam_p;

typedef struct PTGetGtblParam_s {
	PTRefNum_t		PTRefNum;	/* PT reference number */
	KpInt32_t 		ochan;		/* output channel */
	KpInt32_p		nDim;		/* returned # dimensions in grid table */
	KpInt32_p		dimList;	/* returned list of dimension sizes */
	KcmHandle FAR*	gtblDat;	/* returned table address */
} PTGetGtblParam_t, FAR* PTGetGtblParam_p;

typedef struct PTGetOtblParam_s {
	PTRefNum_t		PTRefNum;	/* PT reference number */
	KpInt32_t 		ochan;		/* output channel */
	KcmHandle FAR*	otblDat;	/* returned table address */
} PTGetOtblParam_t, FAR* PTGetOtblParam_p;

typedef struct PTNewMatGamPTParam_s {
	FixedXYZColor_p		rXYZ;
	FixedXYZColor_p		gXYZ;
	FixedXYZColor_p		bXYZ;
	ResponseRecord_p	rTRC;
	ResponseRecord_p	gTRC;
	ResponseRecord_p	bTRC;
	KpUInt32_t			gridsize;
	KpBool_t			invert;
	PTRefNum_p			thePTRefNumP;
} PTNewMatGamPTParam_t, FAR* PTNewMatGamPTParam_p;

typedef struct PTNewMatGamAIPTParam_s {
	FixedXYZColor_p		rXYZ;
	FixedXYZColor_p		gXYZ;
	FixedXYZColor_p		bXYZ;
	ResponseRecord_p	rTRC;
	ResponseRecord_p	gTRC;
	ResponseRecord_p	bTRC;
	KpUInt32_t			gridsize;
	KpBool_t			invert;
	newMGmode_p			newMGmode;
	PTRefNum_p			thePTRefNumP;
} PTNewMatGamAIPTParam_t, FAR* PTNewMatGamAIPTParam_p;

typedef struct PTNewMonoPTParam_s {
	ResponseRecord_p	grayTRC;
	KpUInt32_t			gridsize;
	KpBool_t			invert;
	PTRefNum_p			thePTRefNumP;
} PTNewMonoPTParam_t, FAR* PTNewMonoPTParam_p;

typedef struct PTGetRelToAbsPTParam_s {
	KpInt32_t		RelToAbsMode;
	PTRelToAbs_p	PTRelToAbs;
	PTRefNum_p		PTRefNumPtr;
} PTGetRelToAbsPTParam_t, FAR* PTGetRelToAbsPTParam_p;

typedef struct PTGetAuxPTParam_s {
	KpChar_p		PTName;
	PTRefNum_p		PTRefNumPtr;
} PTGetAuxPTParam_t, FAR* PTGetAuxPTParam_p;

typedef struct PTGetFlavorParam_s {
	KpInt32_p	kcpFlavor;
} PTGetFlavorParam_t, FAR* PTGetFlavorParam_p;

typedef struct PTInvertParam_s {
	PTRefNum_t	RefNum;		/* PT reference number */
	KpInt32_t	senseAttrib;	/* Table to invert */
} PTInvertParam_t, FAR* PTInvertParam_p;

typedef struct PTGetMPStateParam_s {
	KpUInt32_p	MP_Available;	/* whether or not MP is available */
	KpUInt32_p	MP_Used;		/* MP in use? */
} PTGetMPStateParam_t, FAR* PTGetMPStateParam_p;

typedef struct PTSetMPStateParam_s {
	KpUInt32_t	MP_Used;		/* enable or disable MP */
} PTSetMPStateParam_t, FAR* PTSetMPStateParam_p;

typedef struct PTCreateTRCParam_s {
	KpUInt16_p	TRC;
	KpFloat32_t	gamma;
} PTCreateTRCParam_t, FAR* PTCreateTRCParam_p;

typedef struct PTInitCMSParam_s {
	KpMemoryData_t MemoryData;		/* Application supplied memory functions */
} PTInitCMSParam_t, FAR* PTInitCMSParam_p;


#if defined (KPMACPPC)
#pragma options align=reset	/* set alignment back to original in case of succeeding #includes */
#endif

#endif	/* KCMCPR_D_H_ */
