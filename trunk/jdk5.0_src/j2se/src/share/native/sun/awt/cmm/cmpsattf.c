/*
 * @(#)cmpsattf.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)cmpsattf.c	2.13 99/06/04

	Contains:	FuT specific attribute propagation when composing PTs

	Copyright:	(c) 1991-1999 by Eastman Kodak Company, all rights
			reserved.
 */

#include "kcmptlib.h"
#include "attrib.h"
#include "attrcipg.h"
#include "kcptmgr.h"


#define ATTR_LIST_END 0


/* attributes which are propagated from the first("Input") PT */
static KpInt32_t propIAttrF[] = {
	KCM_SPACE_IN,
	KCM_MEDIUM_SENSE_IN,
	KCM_SENSE_INVERTIBLE_IN,
	KCM_IN_CHAIN_CLASS,
	KCM_IN_CHAIN_CLASS_2,
	ATTR_LIST_END};

/* attributes which are propagated from the second("Output") PT */
static KpInt32_t propOAttrF[] = {
	KCM_SPACE_OUT,
	KCM_MEDIUM_SENSE_OUT,
	KCM_SENSE_INVERTIBLE_OUT,
	KCM_OUT_CHAIN_CLASS,
	KCM_OUT_CHAIN_CLASS_2,
	ATTR_LIST_END};


/* use PTRefNum1 and PTRefNum2 to propagate attributes to PTRefNumR */
PTErr_t ComposeAttrFut(	PTRefNum_t PTRefNum1,
						PTRefNum_t PTRefNum2,
						PTRefNum_t PTRefNumR)
{
PTErr_t	errnum = KCP_SUCCESS;
KpInt8_t	strInSpace[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
KpInt8_t	strOutSpace[KCM_MAX_ATTRIB_VALUE_LENGTH+1];
KpInt32_t	inspace, outspace, attrSize;

	/* get 1st PT */
	attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;
	errnum = PTGetAttribute(PTRefNum1, KCM_SPACE_OUT, &attrSize, strOutSpace);
	if (errnum == KCP_SUCCESS) {
		outspace = KpAtoi(strOutSpace);

		/* get 2nd PT */
		attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;
		errnum = PTGetAttribute(PTRefNum2, KCM_SPACE_IN, &attrSize, strInSpace);

		if (errnum == KCP_SUCCESS) {
			inspace = KpAtoi(strInSpace);

			if ((outspace == KCM_UNKNOWN) && (inspace != KCM_UNKNOWN)) {
				errnum = moveAttrList (PTRefNum2, 0, propIAttrF, 0, PTRefNumR);
				if (errnum == KCP_SUCCESS) {		
					errnum = moveAttrList (PTRefNum2, 0, propOAttrF, 0, PTRefNumR);
				}
				return (errnum);
			}
			else {
				if ((outspace != KCM_UNKNOWN) && (inspace == KCM_UNKNOWN)) {
					errnum = moveAttrList (PTRefNum1, 0, propIAttrF, 0, PTRefNumR);
					if (errnum == KCP_SUCCESS) {		
						errnum = moveAttrList (PTRefNum1, 0, propOAttrF, 0, PTRefNumR);
					}
					return (errnum);
				}
			}
		}
	}

	errnum = moveAttrList (PTRefNum1, 0, propIAttrF, 0, PTRefNumR);	/* propagate "input" attributes */
	
	if (errnum == KCP_SUCCESS) {
		errnum = moveAttrList (PTRefNum2, 0, propOAttrF, 0, PTRefNumR);	/* propagate "output" attributes */
	}

	return (errnum);
}


/* propagate a list of attributes */
PTErr_t moveAttrList (	PTRefNum_t PTRefNum1, PTRefNum_t PTRefNum2,
						KpInt32_p attrList, KpInt32_t hasSimAttr, PTRefNum_t PTRefNumR)
{
PTErr_t 	errnum = KCP_SUCCESS;
KpHandle_t	startR1AttrList = NULL, startR2AttrList = NULL;
KpInt32_t	i1;
KpInt32_t	attrSize;
KpChar_t	attribute[KCM_MAX_ATTRIB_VALUE_LENGTH+1];

	errnum = getPTStatus (PTRefNum1);
	if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_PT_INACTIVE) || (errnum == KCP_SERIAL_PT)) {
		startR1AttrList = getPTAttr (PTRefNum1);
	}
	
	errnum = getPTStatus (PTRefNum2);
	if ((errnum == KCP_PT_ACTIVE) || (errnum == KCP_PT_INACTIVE) || (errnum == KCP_SERIAL_PT)) {
		startR2AttrList = getPTAttr (PTRefNum2);
	}
	
	for (i1 = 0; attrList[i1] != ATTR_LIST_END; i1++) {
		errnum = KCP_FAILURE;

		if (startR1AttrList != NULL) {
			attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;
			errnum = GetAttribute (startR1AttrList, attrList[i1], &attrSize, attribute);
		}

		if ((errnum != KCP_SUCCESS) && (startR2AttrList != NULL)) {	/* try for default attribute */
			attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;
			errnum = GetAttribute (startR2AttrList, attrList[i1], &attrSize, attribute);
		}

		if (errnum == KCP_SUCCESS) {
			errnum = PTSetAttribute(PTRefNumR, attrList[i1], attribute);	/* write to destination PT */
		}

		if (hasSimAttr == 1) {
			if (errnum == KCP_INVAL_PTA_TAG) {	/* try for backup attribute */
				attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH;	/* read attribute */
				errnum = GetAttribute (startR2AttrList, attrList[i1+1], &attrSize, attribute);
				if (errnum == KCP_SUCCESS) {	/* write to destination PT */
					errnum = PTSetAttribute (PTRefNumR, attrList[i1], attribute);
				}
			}
			
			i1++;		/* skip past backup attribute */
		}

		if ((errnum != KCP_INVAL_PTA_TAG) && (errnum != KCP_SUCCESS)) {
			return (errnum);
		}
	}

	return KCP_SUCCESS;
}

