/*
 * @(#)kcptmgr.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)kcptmgr.h	2.165 99/03/04

	Contains:	header file for KCMS system processor

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1992-2003
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#ifndef _KCPTMGR_H_
#define _KCPTMGR_H_ 1

/* !!!! change 'vers' in KPCPversion.r !!!! */
/* THIS MUST NOT GO TO 6 (count 'em 6) chars or the precision api will barf !!! */
#define KPCP_VERSION "5.1.1"		/* color processor version number */

#include "kcms_sys.h"
#include "kcmptlib.h"
#include "fut.h"

#if defined (KPMAC)
#include <Components.h>
#endif

#if !defined (PTGLOBAL)
#define PTGLOBAL	extern
#endif

#ifdef  __cplusplus
extern "C" {
#endif
PTGLOBAL KpCriticalFlag_t	PTCacheCritFlag;
#ifdef  __cplusplus
}
#endif

#define KCP_NO_MAX_INTATTR      (-1)    /* switch to disable max test */

/* the maximum number of processors active at one time (including main processor) */ 
#define KCP_MAX_PROCESSORS	(4)

/* PT status constants */
#define NOT_CHECKED_IN (0)		/* show this pt is not checked in */
#define IS_CHECKED_IN (1)		/* show this pt is checked in */
#define NOT_SERIAL_PT (2)		/* show this is not a serial pt */
#define IS_SERIAL_PT (3)		/* show this is a serial pt */
#define MAX_PT_CHAIN_SIZE (20)	/* maximum # of PTs in a serial pt */

/* grid table dimensions */ 
#define	KCP_GRID_DIM_TWO		(2)
#define KCP_GRID_DIM_EIGHT		(8)
#define KCP_GRID_DIM_SIXTEEN	(16)

/* number of input variables */
#define	KCP_THREE_COMP	(3)		/* 3 input components */
#define	KCP_FOUR_COMP	(4)		/* 4 input components */
#define	KCP_FIVE_COMP	(5)		/* 5 input components */

#define TH1_NUM_OFFSETS (2*2*2*2)	/* number of offsets in the interpolation volume */
#define TH1_4D_PENTAHEDROA	(24)	/* number of pentahedra in a 4D volume */

#define ET_NLUT (12)			/* number of optimized evaluation tables */

typedef struct etMem_s {
	KpInt32_t		bytes;
	KpInt32_t		lockCount;
	KpGenericPtr_t	P;
	KpHandle_t		H;
} etMem_t, FAR* etMem_p;

typedef struct th1_4dControl_s {
	KpUInt32_t	tvert1, tvert2, tvert3, tvert4;	/* offsets from the base grid point to the pentahedral corners */
	KpUInt32_t	dx, dy, dz, dt;
} th1_4dControl_t, FAR* th1_4dControl_p, FAR* FAR* th1_4dControl_h;

/* PT table definition */
typedef struct PTTable_s {
	KpUInt32_t	uid;				/* structure identifier */
	PTRefNum_t	refNum;				/* the reference number for the PT */
	KpUInt32_t	lutConfig;			/* type of mab lut used to create this fut */
	KpHandle_t	hdr;				/* address of the PT header */
	KpHandle_t	attrBase;			/* address of the PT attributes */
	KpHandle_t	data;				/* address of the PT data */
	PTRefNum_t	matrixRefNum;		/* the reference number for the PT made from a Matrix */
	KpUInt32_t	checkInFlag;		/* is this PT checked in? */
	KpInt32_t	inUseCount;			/* number of links to this PT */
	KpUInt32_t	serialPTflag;		/* is this a serial PT? */
	KpInt32_t	serialCount;		/* # of PTs in the serial chain */
	PTRefNum_t	serialDef [MAX_PT_CHAIN_SIZE];	/* list of PTs for serial eval */

	fut_p		dataP;				/* pointer to PT data */
	KpInt32_t	numInputs;			/* # of input channels */
	KpInt32_t	optGridP;			/* current address of optimized grid tables; so that grid address can be combined into iLut index */
	etMem_t		etLuts [ET_NLUT];	/* optimized evaluation lookup tables */	
	KpInt32_t	etGOffsets [TH1_NUM_OFFSETS];	/* Offset in grid from base of a cell to next entry in each dimension */
	th1_4dControl_t	etFinder [TH1_4D_PENTAHEDROA];	/* finder tables for pentahedron data */

} PTTable_t, FAR* PTTable_p;

#define PTT_UID (0x70747462) /* PTTable_s identifier, is 'pttb' */

									/* be careful!!  the following must not conflict with ICC definitions! */
#define PTTYPE_CALCULATED (0x1L)	/* PT was generated internally by calculation */
#define PTTYPE_COMPOSITION (0x2L)	/* PT was generated internally by composition */

#if defined (KCP_MACPPC_MP)
#include <MP.h>

#define	kEvaluate (1)
#define	kTerminate (2)
#define	kComplete (3)
#define	kErrMP (4)

typedef struct taskControl_s {
	MPTaskID	ID;				/* ID of this task */
	MPQueueID	fromMain;		/* messages from main task to others */
	MPQueueID	toMain;			/* messages from others to main task */
	MPQueueID	termination;	/* termination queue for this task */
} taskControl_t, FAR* taskControl_p;

#endif

#if defined (KPMAC)
typedef PTErr_t (FAR PASCAL *PTRelay_t) (long savedA5, long savedA4, PTProgress_t progressFunc, KpInt32_t perCent);
#endif

/* globals which are common to all components and which are set up at system startup time */
typedef struct initializedGlobals_s {
	KpBool_t	isInitialized;		/* THIS MUST BE FIRST to be initialized to KPFALSE! */
	KpChar_t	KCPDataDir [256];	/* fully qualified path to CP data directory */
	KpInt32_t	maxGridDim;			/* size of ICC profiles (pts) */
	KpUInt32_t	numProcessors;		/* number of used processors */
	KpUInt32_t	numProcessorsAvailable;	/* number of available processors */
	
#if defined (KCP_ACCEL)
	KpUInt32_t	haveCTE;			/* 1 if CTE present */
#endif

#if defined (KPMAC)
	PTRelay_t	callBackRelay;		/* progress call back relay for PPC->68K */
#endif

#if defined (KPWIN)
	KpModuleId	moduleId;
	KpInstance	instance;
	KpModuleId	appModuleId;
#endif

} initializedGlobals_t, FAR* initializedGlobals_p;


#if (defined KCP_SINGLE_EVAL_CACHE) || (defined KCP_MACPPC_MP)
typedef struct processGlobals_s {
#if defined KCP_SINGLE_EVAL_CACHE
	PTRefNum_t			evalStatePT;			/* the PT which has evaluation tables */ 
#endif

#if defined (KCP_MACPPC_MP)
	KpUInt32_t			isMPinitialized;		/* Only want to initialize the MP once */
	taskControl_p		taskListP;
#endif

} processGlobals_t, FAR* processGlobals_p;
#endif


typedef struct callBack_s {
	KpInt32_t 			loopStart;
	KpInt32_t			loopCount;
	PTProgress_t		progressFunc;
	KpInt32_t			currPasses;
	KpInt32_t			totalPasses;
	KpInt32_t			lastProg100;
#if defined (KPMAC)
	long 				gHostA4;
	long 				gHostA5;
#endif
} callBack_t, FAR* callBack_p;


/* function prototypes */
PTErr_t		registerPT (KpHandle_t, KpHandle_t, PTRefNum_p);
PTErr_t		getPTStatus (PTRefNum_t);
KpHandle_t	getPTHdr (PTRefNum_t);
PTErr_t		setPTHdr (PTRefNum_t, KpHandle_t);
KpHandle_t	getPTAttr (PTRefNum_t);
void		setPTAttr (PTRefNum_t, KpHandle_t);
KpHandle_t	getPTData (PTRefNum_t);
PTErr_t		setMatrixPTRefNum (PTRefNum_t, PTRefNum_t, KpUInt32_t);
PTErr_t		getMatrixPTRefNum (PTRefNum_t, PTRefNum_p, KpUInt32_p);
PTErr_t		hasPTData (PTRefNum_t);
PTErr_t 	resolvePTData (PTRefNum_t, KpInt32_p, PTRefNum_p);
void		makeActive (PTRefNum_t, KpHandle_t);
void		makeSerial (PTRefNum_t);
PTErr_t		makeInActive (PTRefNum_t, KpBool_t);
PTErr_t	 	makeCheckedOut (PTRefNum_t);
KpInt32_t	PTGetSrcFormat (PTRefNum_t);
PTErr_t 	addSerialData (PTRefNum_t, PTRefNum_t);
PTTable_p	lockPTTable (PTRefNum_t);
void		unlockPTTable (PTRefNum_t);
void		deletePTTable (PTRefNum_t);

KpInt32_t getAttrSize (KpHandle_t);
PTErr_t GetAttribute (KpHandle_t, KpInt32_t, KpInt32_p, KpChar_p);
PTErr_t readAttributes (KpFd_p, KpInt32_t, KpHandle_t FAR*);
PTErr_t writeAttributes (KpFd_p, KpHandle_t);
PTErr_t freeAttributes (KpHandle_t);
PTErr_t copyAllAttr (PTRefNum_t, PTRefNum_t);
void checkDataClass (PTRefNum_t);

PTErr_t GetEval (PTEvalTypes_t, PTEvalTypes_p);

PTErr_t	TpReadHdr (KpFd_p, KpHandle_t FAR*, PTType_p);
PTErr_t	TpWriteHdr (KpFd_p, PTType_t, KpHandle_t, KpInt32_t);
PTErr_t	TpCompareHdr (KpHandle_t, KpHandle_t);
PTErr_t	TpFreeHdr (KpHandle_t);
PTErr_t	TpReadData (KpFd_p, PTType_t, PTRefNum_t, KpHandle_t, KpHandle_t FAR*);
PTErr_t	TpWriteData (KpFd_p, PTType_t, KpHandle_t, KpHandle_t);
PTErr_t	TpFreeData (KpHandle_t);
KpInt32_t TpGetDataSize (KpHandle_t, KpHandle_t, PTType_t);
PTErr_t	initAttrib (PTRefNum_t);
PTErr_t TpSetImplicitAttr (PTRefNum_t);
#if !defined KCMS_NO_CRC
PTErr_t TpCalCrc (KpHandle_t, KpHandle_t, KpInt32_p);
#endif

PTErr_t PTEvalP (PTRefNum_t, PTEvalPB_p, PTEvalTypes_t, KpInt32_t, KpInt32_t, opRefNum_p, callBack_p);
PTErr_t PTEvaluate (PTRefNum_t, PTEvalDTPB_p, PTEvalTypes_t, KpInt32_t, KpInt32_t, opRefNum_p, callBack_p);
PTErr_t PTEvalSeq (KpInt32_t, PTTable_p*, KpUInt32_p, PTEvalDTPB_p, callBack_p);
void	nullEvalTables (PTTable_p);
void	freeEvalTables (PTRefNum_t);

PTErr_t	doProgress (callBack_p, KpInt32_t);
void	initProgressPasses (KpInt32_t, callBack_p);
void	initProgress (KpInt32_t, callBack_p);

KpInt32_t	PTMemTest (void);
PTErr_t		initExport (KpHandle_t, KpHandle_t, PTType_t, fut_hdr_p FAR*, fut_p FAR*);
PTErr_t		unlockPT (KpHandle_t, fut_p);
PTErr_t		checkPT (PTRefNum_t);
PTErr_t		getMaxGridDim (KpInt32_p);
KpInt32_t	fut_get_size (fut_p, fut_hdr_p);

PTErr_t PTNewEmpty (KpInt32_t, KpInt32_p, KpInt32_t, PTRefNum_p);
PTErr_t PTNewEmptySep (KpInt32_t, KpInt32_p, PTRefNum_p);
PTErr_t fut2PT (fut_p FAR*, KpInt32_t, KpInt32_t, KpInt32_t, PTRefNum_p);
PTErr_t PTGetItbl (PTRefNum_t, KpInt32_t, KpInt32_t, KpHandle_t FAR*);
PTErr_t PTGetGtbl (PTRefNum_t, KpInt32_t, KpInt32_p, KpInt32_p, KpHandle_t FAR*);
PTErr_t PTGetOtbl (PTRefNum_t, KpInt32_t, KpHandle_t FAR*);
PTErr_t getRefTbl (KpInt32_t, PTRefNum_t, KpInt32_t, KpInt32_t, KpHandle_t FAR*, KpInt32_p);

fut_p get_lab2xyz (KpInt32_t);

void addCompType (KpInt32_p);
void KCPChainSetup (initializedGlobals_p);
PTErr_t ComposeAttr (PTRefNum_t, PTRefNum_t, KpInt32_t, PTRefNum_t);
PTErr_t ComposeAttrFut (PTRefNum_t, PTRefNum_t, PTRefNum_t);
PTErr_t moveAttrList (PTRefNum_t, PTRefNum_t, KpInt32_p, KpInt32_t, PTRefNum_t);

KpInt32_t	getIntAttrDef (PTRefNum_t, KpInt32_t);
PTErr_t		getIntAttr (PTRefNum_t, KpInt32_t, KpInt32_t, KpInt32_p);

initializedGlobals_p	getInitializedGlobals (void);

PTErr_t		KCMDTerminate (void);
KpInt32_t	KCMDsetup (KpGenericPtr_t FAR*);

#if (defined KCP_SINGLE_EVAL_CACHE) || (defined KCP_MACPPC_MP)
KpInt32_t			KCPappSetup (KpGenericPtr_t);
PTErr_t				KpTermProcess (void);
processGlobals_p	loadProcessGlobals (void);
void				unloadProcessGlobals (void);

	#if defined (KCP_MACPPC_MP)
	OSStatus	evalTaskMac (void*);
	PTErr_t		KCPInitializeMP (void);
	#endif
#endif

void		KCPInitIGblP (KpGenericPtr_t FAR*, initializedGlobals_p IGblP);
PTErr_t		PTTerminatePlatform (void);

PTErr_t		SetKCPDataDirProps (KpFileProps_p);

#if defined KCP_DIAG_LOG
void kcpDiagLog (KpChar_p);
void saveFut (fut_p, KpChar_p);
#endif

#if defined (KPMAC)
KpInt32_t	KCMDtakeDown (void);
KpInt32_t	KCPappTakeDown (void);
PTErr_t		KCMDcommand (KpInt16_t, KpGenericPtr_t);
KpInt32_t	CanDoSelector (KpInt16_t);
KpInt32_t	GetVersion (void);
#endif

#endif
