/*
 * @(#)auxpt.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	auxpt.c

	Contains:	functions which load and create the auxiliary PTs used in chaining

	COPYRIGHT (c) 1994-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <string.h>
#include <stdio.h>
#include "kcmptlib.h"
#include "kcpfchn.h"
#include "attrib.h"
#include "makefuts.h"

/* Information required to build the various auxiliary PTs. */
typedef struct auxPTBuild_s {
	PTDataClass_t	iClass;			/* input data class */
	PTDataClass_t	oClass;			/* output data class */
	KpInt32_t		iomask;			/* i/o channels mask */
	KpInt32_t		size[4];		/* maximum size of Grid Table in each dimension */
	fut_ifunc_t		ifunArray[4];	/* input table building functions */
	fut_gfunc_t		gfunArray[4];	/* grid table building functions */
	fut_ofunc_t		ofunArray[4];	/* output table building functions */
} auxPTBuild_t, FAR* auxPTBuild_p;

static PTErr_t getAuxBuild (KpChar_p PTName, auxPTBuild_p auxBuildP);

PTErr_t
	PTGetAuxPT (	const KpChar_p	PTName,
					PTRefNum_p		PTRefNumPtr)
{
	return	loadAuxPT (PTName, KCM_POSITIVE, PTRefNumPtr);
}


/* loadAuxPT - load an.
 *			   If the PT is not on disk, create it and then store to disk.
 * return:
 *		KCP_SUCCESS if successful
 *		a PTErr code otherwise
 */

PTErr_t
	loadAuxPT (	const KpChar_p	PTName,
				KpInt32_t		invert,
				PTRefNum_p		PTRefNumPtr)
{
KpChar_t		myPTName[MAX_AUX_FILENAME];
KpInt32_t		idCharPosition;
auxPTBuild_t	auxPTBuild;
PTErr_t			PTErr;
fut_p			futp = NULL;
auxData_t		auxData;

	/* validate the name passed in */
	if (NULL == PTName) goto ErrOut1;
	if (0 == strlen (PTName)) goto ErrOut1;

	*PTRefNumPtr = 0;

	/* make a local copy of PTName because we can't modify it directly */
	strcpy (myPTName, PTName);

	/* If the PT is a negative pt and invert equals KCM_POSITIVE
		 then use the positive version of the PT */
	idCharPosition = strlen (myPTName) -1;	
	if ((myPTName[idCharPosition] == 'i') && (invert == KCM_POSITIVE)) {
		myPTName[idCharPosition] = '\0';
	}

	PTErr = getAuxBuild (myPTName, &auxPTBuild);
	if (PTErr != KCP_SUCCESS) {
		goto getOut;
	}

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "loadAuxPT\n myPTName %s\n", myPTName);
	kcpDiagLog (string);}
	#endif

	/* initialize all of the constants needed for all aux PTs */
	lensityInit (&auxData.lc);
	uvLLabInit (&auxData.uvLLabC);
	LabuvLInit (&auxData.LabuvLC);

	futp = constructfut (auxPTBuild.iomask, auxPTBuild.size, &auxData.std,
					auxPTBuild.ifunArray, auxPTBuild.gfunArray, auxPTBuild.ofunArray,
					auxPTBuild.iClass, auxPTBuild.oClass);

	if (futp == NULL) {
		goto ErrOut2;
	}

	#if defined KCP_DIAG_LOG
	saveFut (futp, myPTName);
	#endif

	/* Create a PT from the fut */
	PTErr = fut2PT (&futp, KCM_UNKNOWN, KCM_UNKNOWN, PTTYPE_CALCULATED, PTRefNumPtr);
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut2;
	}

getOut:
	return (PTErr);


ErrOut2:
	PTErr = KCP_AUX_ERROR;
	goto ErrOut0;

ErrOut1:
	PTErr = KCP_BAD_ARG;

ErrOut0:
	if (*PTRefNumPtr != NULL) PTCheckOut (*PTRefNumPtr);

	goto getOut;
}


/* getAuxBuild -- 	fill in the auxPTBuild structure pointed to by auxBuildP.  
 * return:
 *		KCP_SUCCESS 		if successful,
 *		KCP_AUX_UNKNOWN 	if gAuxPTDef structure for PTName is not found
 */

static PTErr_t
	getAuxBuild (KpChar_p PTName, auxPTBuild_p auxBuildP)
{

	/* set up default values, which is for lin16 */
	auxBuildP->iClass = KCP_FIXED_RANGE;	/* input data class */
	auxBuildP->oClass = KCP_FIXED_RANGE;	/* output data class */
	auxBuildP->iomask = FUT_IN(FUT_XYZ) | FUT_OUT(FUT_XYZ);	/* i/o channels mask */
	auxBuildP->size[0] = 16;				/* size of Grid Table in each dimension */
	auxBuildP->size[1] = 16;
	auxBuildP->size[2] = 16;
	auxBuildP->size[3] = 0;
	auxBuildP->ifunArray[0] = NULL;			/* input tables are identity functions */
	auxBuildP->ifunArray[1] = NULL;
	auxBuildP->ifunArray[2] = NULL;
	auxBuildP->ifunArray[3] = NULL;
	auxBuildP->gfunArray[0] = NULL;			/* grid tables are identity functions */
	auxBuildP->gfunArray[1] = NULL;
	auxBuildP->gfunArray[2] = NULL;
	auxBuildP->gfunArray[3] = NULL;
	auxBuildP->ofunArray[0] = NULL;			/* output tables are identity functions */
	auxBuildP->ofunArray[1] = NULL;
	auxBuildP->ofunArray[2] = NULL;
	auxBuildP->ofunArray[3] = NULL;

	/* change values which are not the same as the default */
	if (strcmp (PTName, "CP02") == 0) {		/* lin16 */
		/* default, so no changes needed */
	}
#if !defined (JAVACMM)
	else if (strcmp (PTName, "CP04") == 0) {	/* fxnull */
		auxBuildP->iClass = KCP_LUV_DATA;
		auxBuildP->oClass = KCP_LUV_DATA;
		auxBuildP->size[2] = 32;
		auxBuildP->ifunArray[0] = fxnull_iFunc_x;
		auxBuildP->ifunArray[1] = fxnull_iFunc_y;
		auxBuildP->ifunArray[2] = fxnull_iFunc_z;
		auxBuildP->ofunArray[0] = fxnull_oFunc_x;
		auxBuildP->ofunArray[1] = fxnull_oFunc_y;
		auxBuildP->ofunArray[2] = fxnull_oFunc_z;
	}
	else if (strcmp (PTName, "CP05") == 0) {	/* qlin16 */
		auxBuildP->iomask = FUT_IN(FUT_XYZT) | FUT_OUT(FUT_XYZT);
		auxBuildP->size[3] = 2;
	}
	else if (strcmp (PTName, "CP07") == 0) {	/* logrgb */
		auxBuildP->ifunArray[0] = logrgb_iFunc;
		auxBuildP->ifunArray[1] = logrgb_iFunc;
		auxBuildP->ifunArray[2] = logrgb_iFunc;
		auxBuildP->ofunArray[0] = logrgb_oFunc;
		auxBuildP->ofunArray[1] = logrgb_oFunc;
		auxBuildP->ofunArray[2] = logrgb_oFunc;
	}
	else if (strcmp (PTName, "CP08") == 0) {	/* loguvl */
		auxBuildP->iClass = KCP_LUV_DATA;
		auxBuildP->oClass = KCP_LUV_DATA;
		auxBuildP->ifunArray[0] = loguvl_iFunc_x;
		auxBuildP->ifunArray[1] = loguvl_iFunc_y;
		auxBuildP->ifunArray[2] = loguvl_iFunc_z;
		auxBuildP->ofunArray[0] = loguvl_oFunc_x;
		auxBuildP->ofunArray[1] = loguvl_oFunc_y;
		auxBuildP->ofunArray[2] = loguvl_oFunc_z;
	}
	else if (strcmp (PTName, "CP10i") == 0) {	/* cmyklin */
		auxBuildP->iomask = FUT_IN(FUT_XYZT) | FUT_OUT(FUT_XYZT);
		auxBuildP->size[3] = 2;
		auxBuildP->ifunArray[0] = cmyklin_iFunc;
		auxBuildP->ifunArray[1] = cmyklin_iFunc;
		auxBuildP->ifunArray[2] = cmyklin_iFunc;
		auxBuildP->ifunArray[3] = cmyklin_iFunc;
		auxBuildP->ofunArray[0] = cmyklin_oFunc;
		auxBuildP->ofunArray[1] = cmyklin_oFunc;
		auxBuildP->ofunArray[2] = cmyklin_oFunc;
		auxBuildP->ofunArray[3] = cmyklin_oFunc;
	}
	else if (strcmp (PTName, "CP10") == 0) {	/* cmyklin invert */
		auxBuildP->iomask = FUT_IN(FUT_XYZT) | FUT_OUT(FUT_XYZT);
		auxBuildP->size[3] = 2;
		auxBuildP->ifunArray[0] = cmyklini_iFunc;
		auxBuildP->ifunArray[1] = cmyklini_iFunc;
		auxBuildP->ifunArray[2] = cmyklini_iFunc;
		auxBuildP->ifunArray[3] = cmyklini_iFunc;
		auxBuildP->ofunArray[0] = cmyklini_oFunc;
		auxBuildP->ofunArray[1] = cmyklini_oFunc;
		auxBuildP->ofunArray[2] = cmyklini_oFunc;
		auxBuildP->ofunArray[3] = cmyklini_oFunc;
	}
	else if (strcmp (PTName, "CP22") == 0) {	/* xyzmap */
		auxBuildP->iClass = KCP_XYZ_PCS;
		auxBuildP->oClass = KCP_XYZ_PCS;
		auxBuildP->size[2] = 32;
		auxBuildP->ifunArray[0] = xyzmap_iFunc;
		auxBuildP->ifunArray[1] = xyzmap_iFunc;
		auxBuildP->ifunArray[2] = xyzmap_iFunc;
		auxBuildP->ofunArray[0] = xyzmap_oFunc;
		auxBuildP->ofunArray[1] = xyzmap_oFunc;
		auxBuildP->ofunArray[2] = xyzmap_oFunc;
	}
#endif
	else if (strcmp (PTName, "CP31") == 0) {	/* uvL->Lab */
		auxBuildP->iClass = KCP_LUV_DATA;
		auxBuildP->oClass = KCP_LAB_PCS;
		auxBuildP->size[0] = 32;
		auxBuildP->size[1] = 32;
		auxBuildP->size[2] = 32;
		auxBuildP->ifunArray[0] = uvLLab_iu;
		auxBuildP->ifunArray[1] = uvLLab_iv;
		auxBuildP->ifunArray[2] = uvLLab_iL;
		auxBuildP->gfunArray[0] = uvLLab_gFun;
		auxBuildP->gfunArray[1] = uvLLab_gFun;
		auxBuildP->gfunArray[2] = uvLLab_gFun;
		auxBuildP->ofunArray[0] = uvLLab_oFun;
		auxBuildP->ofunArray[1] = uvLLab_oFun;
		auxBuildP->ofunArray[2] = uvLLab_oFun;
	}
	else if (strcmp (PTName, "CP32") == 0) {	/* Lab->uvL */
		auxBuildP->iClass = KCP_LAB_PCS;
		auxBuildP->oClass = KCP_LUV_DATA;
		auxBuildP->ifunArray[0] = LabuvL_iL;
		auxBuildP->ifunArray[1] = LabuvL_ia;
		auxBuildP->ifunArray[2] = LabuvL_ib;
		auxBuildP->gfunArray[0] = LabuvL_gFun;
		auxBuildP->gfunArray[1] = LabuvL_gFun;
		auxBuildP->gfunArray[2] = LabuvL_gFun;
		auxBuildP->ofunArray[0] = LabuvL_ou;
		auxBuildP->ofunArray[1] = LabuvL_ov;
		auxBuildP->ofunArray[2] = LabuvL_oL;
	}
	else {
		return KCP_AUX_UNKNOWN;
	}	

	return KCP_SUCCESS;
}
