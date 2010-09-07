/*
 * @(#)cmpsatt.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)cmpsatt.c	2.27 99/02/04

	Contains:	Attribute propagation when composing PTs

	Copyright:	(c) 1991-1999 by Eastman Kodak Company, all rights reserved.

*/

#include <string.h>

#include "attrib.h"
#include "attrcipg.h"
#include "kcptmgr.h"

#define ATTR_LIST_END 0

static PTErr_t setEFFECTstate (PTRefNum_t PTRefNum1, PTRefNum_t PTRefNum2, PTRefNum_t PTRefNumR);
static void addIntStr (KpInt32_t integer, KpChar_p str);
static PTErr_t generateAttr (PTRefNum_t PTRefNumR);
static PTErr_t setLinearized (PTRefNum_t PTRefNumR, KcmAttribute attrTag);


/* attributes propagated from the first("Input") PT */
static KpInt32_t propRule02[] = {
	KCM_SPACE_IN,
	KCM_MEDIUM_IN,
	KCM_MEDIUM_DESC_IN,
	KCM_MEDIUM_PRODUCT_IN,
	KCM_ILLUM_TYPE_IN,
	KCM_MEDIUM_SENSE_IN,
	KCM_WHITE_POINT_IN,
	KCM_KCP_INPUT_WT_UPVP,
	KCM_DEVICE_PHOSPHOR_IN,
	KCM_DEVICE_LINEARIZED_IN,
	KCM_PRIMARIES_1_IN,
	KCM_PRIMARIES_2_IN,
	KCM_PRIMARIES_3_IN,
	KCM_PRIMARIES_4_IN,
	KCM_PRIMARIES_5_IN,
	KCM_PRIMARIES_6_IN,
	KCM_PRIMARIES_7_IN,
	KCM_PRIMARIES_8_IN,
	KCM_GAMMA_RED_IN,
	KCM_GAMMA_GREEN_IN,
	KCM_GAMMA_BLUE_IN,
	KCM_SENSE_INVERTIBLE_IN,
	KCM_CHAN_NAME_1_IN,
	KCM_CHAN_NAME_2_IN,
	KCM_CHAN_NAME_3_IN,
	KCM_CHAN_NAME_4_IN,
	KCM_CHAN_NAME_5_IN,
	KCM_CHAN_NAME_6_IN,
	KCM_CHAN_NAME_7_IN,
	KCM_CHAN_NAME_8_IN,
	KCM_ICC_COLORSPACE_IN,
	ATTR_LIST_END};

/* attributes propagated from the second("Output") PT */
static KpInt32_t propRule03[] = {
	KCM_SPACE_OUT,
	KCM_MEDIUM_OUT,
	KCM_MEDIUM_DESC_OUT,
	KCM_MEDIUM_PRODUCT_OUT,
	KCM_ILLUM_TYPE_OUT,
	KCM_MEDIUM_SENSE_OUT,
	KCM_PRT_UCR,
	KCM_PRT_GCR,
	KCM_PRT_BLACK_SHAPE,
	KCM_PRT_BLACKSTART_DELAY,
	KCM_PRT_LINE_RULINGS,
	KCM_PRT_SCREEN_ANGLES,
	KCM_DMAX_OUT,
	KCM_WHITE_POINT_OUT,
	KCM_KCP_OUTPUT_WT_UPVP,
	KCM_DEVICE_PHOSPHOR_OUT,
	KCM_DEVICE_LINEARIZED_OUT,
	KCM_DEVICE_MFG_OUT,
	KCM_DEVICE_MODEL_OUT,
	KCM_DEVICE_UNIT_OUT,
	KCM_DEVICE_SETTINGS_OUT,
	KCM_PRIMARIES_1_OUT,
	KCM_PRIMARIES_2_OUT,
	KCM_PRIMARIES_3_OUT,
	KCM_PRIMARIES_4_OUT,
	KCM_PRIMARIES_5_OUT,
	KCM_PRIMARIES_6_OUT,
	KCM_PRIMARIES_7_OUT,
	KCM_PRIMARIES_8_OUT,
	KCM_GAMMA_RED_OUT,
	KCM_GAMMA_GREEN_OUT,
	KCM_GAMMA_BLUE_OUT,
	KCM_DENSITY_FILTER,
	KCM_25_DOTGAIN,
	KCM_50_DOTGAIN,
	KCM_75_DOTGAIN,
	KCM_COMPRESSION_OUT,
	KCM_SENSE_INVERTIBLE_OUT,
	KCM_CHAN_NAME_1_OUT,
	KCM_CHAN_NAME_2_OUT,
	KCM_CHAN_NAME_3_OUT,
	KCM_CHAN_NAME_4_OUT,
	KCM_CHAN_NAME_5_OUT,
	KCM_CHAN_NAME_6_OUT,
	KCM_CHAN_NAME_7_OUT,
	KCM_CHAN_NAME_8_OUT,
	KCM_ICC_COLORSPACE_OUT,
	ATTR_LIST_END};

/* attributes propagated from the 1st(Input) PT with 2nd(Output) as backup */
static KpInt32_t propRule11[] = {
	KCM_DEVICE_MFG_IN,
	KCM_DEVICE_MODEL_IN,
	KCM_DEVICE_UNIT_IN,
	KCM_DEVICE_SETTINGS_IN,
	ATTR_LIST_END};

/* attributes propagated from the 2nd(Output) PT with 1st(Input) as backup, and
	2nd attribute in list from 1st PT as backup */
static KpInt32_t propRule13[] = {
	KCM_SIM_MEDIUM_OUT,
	KCM_MEDIUM_OUT,
	KCM_SIM_MEDIUM_DESC_OUT,
	KCM_MEDIUM_DESC_OUT,
	KCM_SIM_MEDIUM_SENSE_OUT,
	KCM_MEDIUM_SENSE_OUT,
	KCM_SIM_MEDIUM_PRODUCT_OUT,
	KCM_MEDIUM_PRODUCT_OUT,
	KCM_SIM_UCR,
	KCM_PRT_UCR,
	KCM_SIM_GCR,
	KCM_PRT_GCR,
	KCM_SIM_ILLUM_TYPE_IN,
	KCM_ILLUM_TYPE_IN,
	KCM_SIM_ILLUM_TYPE_OUT,
	KCM_ILLUM_TYPE_OUT,
	KCM_SIM_WHITE_POINT_OUT,
	KCM_WHITE_POINT_OUT,
	KCM_SIM_BLACK_SHAPE,
	KCM_PRT_BLACK_SHAPE,
	KCM_SIM_BLACKSTART_DELAY,
	KCM_PRT_BLACKSTART_DELAY,
	KCM_SIM_LINE_RULINGS,
	KCM_PRT_LINE_RULINGS,
	KCM_SIM_SCREEN_ANGLE,
	KCM_PRT_SCREEN_ANGLES,
	KCM_SIM_DMAX_OUT,
	KCM_DMAX_OUT,
	KCM_SIM_DEVICE_MFG_OUT,
	KCM_DEVICE_MFG_OUT,
	KCM_SIM_DEVICE_MODEL_OUT,
	KCM_DEVICE_MODEL_OUT,
	KCM_SIM_DEVICE_UNIT_OUT,
	KCM_DEVICE_UNIT_OUT,
	KCM_SIM_DEVICE_SETTINGS_OUT,
	KCM_DEVICE_SETTINGS_OUT,
	KCM_SIM_PRIMARIES_1_OUT,
	KCM_PRIMARIES_1_OUT,
	KCM_SIM_PRIMARIES_2_OUT,
	KCM_PRIMARIES_2_OUT,
	KCM_SIM_PRIMARIES_3_OUT,
	KCM_PRIMARIES_3_OUT,
	KCM_SIM_PRIMARIES_4_OUT,
	KCM_PRIMARIES_4_OUT,
	KCM_SIM_PRIMARIES_5_OUT,
	KCM_PRIMARIES_5_OUT,
	KCM_SIM_PRIMARIES_6_OUT,
	KCM_PRIMARIES_6_OUT,
	KCM_SIM_PRIMARIES_7_OUT,
	KCM_PRIMARIES_7_OUT,
	KCM_SIM_PRIMARIES_8_OUT,
	KCM_PRIMARIES_8_OUT,
	KCM_SIM_COMPRESSION_OUT,
	KCM_COMPRESSION_OUT,
	KCM_SIM_CHAN_NAME_1_OUT,
	KCM_CHAN_NAME_1_OUT,
	KCM_SIM_CHAN_NAME_2_OUT,
	KCM_CHAN_NAME_2_OUT,
	KCM_SIM_CHAN_NAME_3_OUT,
	KCM_CHAN_NAME_3_OUT,
	KCM_SIM_CHAN_NAME_4_OUT,
	KCM_CHAN_NAME_4_OUT,
	KCM_SIM_CHAN_NAME_5_OUT,
	KCM_CHAN_NAME_5_OUT,
	KCM_SIM_CHAN_NAME_6_OUT,
	KCM_CHAN_NAME_6_OUT,
	KCM_SIM_CHAN_NAME_7_OUT,
	KCM_CHAN_NAME_7_OUT,
	KCM_SIM_CHAN_NAME_8_OUT,
	KCM_CHAN_NAME_8_OUT,
	ATTR_LIST_END};



/* use PTRefNum1 and PTRefNum2 to propagate attributes to PTRefNumR */
PTErr_t ComposeAttr(PTRefNum_t	PTRefNum1,
					PTRefNum_t	PTRefNum2,
					KpInt32_t	mode,
					PTRefNum_t	PTRefNumR)
{
PTErr_t		PTErr;
KpInt32_t	inspace, outspace;

	/* get output color space of 1st PT */
	PTErr = getIntAttr (PTRefNum1, KCM_SPACE_OUT, KCP_NO_MAX_INTATTR, &outspace);
	if (PTErr == KCP_SUCCESS) {

		/* get input color space of 2nd PT */
		PTErr = getIntAttr (PTRefNum2, KCM_SPACE_IN, KCP_NO_MAX_INTATTR, &inspace);
		if (PTErr == KCP_SUCCESS) {
			if ((outspace == KCM_UNKNOWN) && (inspace != KCM_UNKNOWN)) {
				PTErr = copyAllAttr (PTRefNum2, PTRefNumR);
				return (PTErr);
			}

			if ((outspace != KCM_UNKNOWN) && (inspace == KCM_UNKNOWN)) {
				PTErr = copyAllAttr (PTRefNum1, PTRefNumR);
				return (PTErr);
			}
		}
	}

/* propagate "input" attributes */
	PTErr = moveAttrList (PTRefNum1, 0, propRule02, 0, PTRefNumR);

/* propagate "output" attributes */
	if (PTErr == KCP_SUCCESS)
		PTErr = moveAttrList (PTRefNum2, 0, propRule03, 0, PTRefNumR);

/* generate "constant" attributes */
	if (PTErr == KCP_SUCCESS)
		PTErr = generateAttr (PTRefNumR);

/* if composition mode is input set input attribute to "is linearized" */
	if ((PTErr == KCP_SUCCESS) && (mode == PT_COMBINE_ITBL))
		PTErr = setLinearized (PTRefNumR, KCM_DEVICE_LINEARIZED_IN);

/* if composition mode is output set output attribute to "is linearized" */
	if ((PTErr == KCP_SUCCESS) && (mode == PT_COMBINE_OTBL))
		PTErr = setLinearized (PTRefNumR, KCM_DEVICE_LINEARIZED_OUT);

/* Set KCM_EFFECT_TYPE attribute */
	if (PTErr == KCP_SUCCESS)
		PTErr = setEFFECTstate (PTRefNum1, PTRefNum2, PTRefNumR);

/* propagate "input" attributes, 2nd PT is backup */
	if (PTErr == KCP_SUCCESS)
		PTErr = moveAttrList (PTRefNum1, PTRefNum2, propRule11, 0, PTRefNumR);

/* propagate "simulate" attributes */
	if (PTErr == KCP_SUCCESS)
		PTErr = moveAttrList (PTRefNum2, PTRefNum1, propRule13, 1, PTRefNumR);

	return (PTErr);
}


/* Generate attributes which must be in each PT */
static PTErr_t generateAttr (PTRefNum_t PTRefNumR)
{
PTErr_t PTErr;
KpChar_t attrStr[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
kpTm_t Date;

	KpGetLocalTime (&Date);			/* get the current date and time */

	KpItoa ((KpInt32_t) (1900 + Date.year), attrStr);	/* start with year */
	addIntStr ((KpInt32_t)Date.mon, attrStr);			/* month */
	addIntStr ((KpInt32_t)Date.mday, attrStr);			/* day */
	addIntStr ((KpInt32_t)Date.hour, attrStr);			/* hour */
	addIntStr ((KpInt32_t)Date.min, attrStr);			/* minute */
	addIntStr ((KpInt32_t)Date.sec, attrStr);			/* second */

	PTErr = PTSetAttribute (PTRefNumR, KCM_CREATE_TIME, attrStr);	/* write time to destination PT */

	if (PTErr == KCP_SUCCESS) {
		KpItoa (KCM_COMPOSED_CLASS, attrStr);
		PTErr = PTSetAttribute (PTRefNumR, KCM_CLASS, attrStr);	/* set composition state to composed */
	}

	return (PTErr);
}


/* insert delimiter ":", convert integer to ascii,
 * and append it to given string */
static void addIntStr (KpInt32_t integer, KpChar_p str)
{
KpChar_t intStr[20];		/* really large just in case */

	strcat ( str, ":" );		/* insert delimiter */
	KpItoa (integer, intStr);	/* convert integer to ascii */
	strcat ( str, intStr);		/* append to input string */
}


/* Set KCM_EFFECT_TYPE attribute, which use a default if the attribute is
 * present in both PTs but with different values */
static PTErr_t setEFFECTstate (PTRefNum_t PTRefNum1, PTRefNum_t PTRefNum2,
					 PTRefNum_t PTRefNumR)
{
KpChar_t	attrStr[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
KpChar_t	attrStr1[KCM_MAX_ATTRIB_VALUE_LENGTH+1], attrStr2[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
PTErr_t		PTErr, errnum1, errnum2;
KpInt32_t	attrSize;

	PTErr = KCP_FAILURE;					/* attribute not yet written */
	KpItoa (KCM_EFFECT_MULTIPLE, attrStr);	/* initialize to default */

	attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;	/* get attribute from 1st PT */
	errnum1 = PTGetAttribute (PTRefNum1, KCM_EFFECT_TYPE, &attrSize, attrStr1);

	attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;	/* get attribute from 2nd PT */
	errnum2 = PTGetAttribute (PTRefNum2, KCM_EFFECT_TYPE, &attrSize, attrStr2);

	if (errnum1 == KCP_SUCCESS) {
		if ((errnum2 != KCP_SUCCESS) ||				/* if attribute is not in 2nd */
			(strcmp (attrStr1, attrStr2) == 0)) {	/* or attribute values are the same */

			strcpy (attrStr, attrStr1);				/* use attribute in 1st PT */
		}
	}
	else {
		if (errnum2 == KCP_SUCCESS) {				/* if attribute is in 2nd */
			strcpy (attrStr, attrStr2);				/* use it */
		}
		else {
			PTErr = KCP_SUCCESS;					/* not in either PT, do not write attribute */
		}
	}

	if (PTErr != KCP_SUCCESS) {	/* write to destination PT */
		PTErr = PTSetAttribute (PTRefNumR, KCM_EFFECT_TYPE, attrStr);
	}

	return (PTErr);
}


/* if composition mode is input set input attribute to "is linearized" */
static PTErr_t setLinearized (PTRefNum_t PTRefNumR, KcmAttribute attrTag)
{
PTErr_t PTErr;
KpChar_t attrStr[KCM_MAX_ATTRIB_VALUE_LENGTH+1];

	KpItoa (KCM_IS_LINEARIZED, attrStr);

	PTErr = PTSetAttribute (PTRefNumR, attrTag, attrStr);

	return (PTErr);
}
