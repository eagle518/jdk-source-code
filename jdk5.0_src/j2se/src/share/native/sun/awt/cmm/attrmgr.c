/*
 * @(#)attrmgr.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)attrmgr.c	2.53 99/02/16

	Manages reading, writing, changing and deletion of PT attributes

 *********************************************************************
 *    COPYRIGHT (c) 1991-1999 Eastman Kodak Company
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *********************************************************************
 */

#include "kcms_sys.h"

#if defined(KPMAC)
	#include <Types.h>
	#include <Memory.h>
	#include <OSUtils.h>
#endif

#include <string.h>

#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "attrib.h"
#include "attrcipg.h"
#include "kcptmgr.h"

#define MAX_DIGITS 10					/* decimal digits in a 32 bit number */
#define NUMBER_OF_ATTRIB_ENTRIES 100	/* initial number of attribute entries*/

#define ADD_TAG 1						/* Add a tag to list */
#define FIND_TAG 2						/* Find tag in list */
#define DELETE_TAG 3					/* Delete tag from list */

typedef struct PTAttribCount_s {
	KpInt32_t 	num_entries;			/* the current number of tag entries */
	KpInt32_t 	max_entries;			/* the maximum number of tag entries */
} PTAttribCount_t, FAR *PTAttribCount_p;

typedef struct PTAttribEntry_s {
	KpInt32_t 		tag;				/* the attribute tag for the PT */
	KpHandle_t	string;					/* the attribute string for the PT */
} PTAttribEntry_t, FAR *PTAttribEntry_p, FAR* FAR* PTAttribEntry_h;

/* prototypes */
static PTErr_t LinearScanList ARGS((KpHandle_t startAttrList, PTAttribEntry_p attrEntry, KpInt32_t mode));
static PTErr_t SetAttribute ARGS((KpHandle_t *startAttrH, KpInt32_t attrTag, KpChar_p attrString));
static PTErr_t AddAttribute ARGS((KpHandle_t *startAttrH, KpInt32_t attrTag, KpHandle_t attrString));
static PTErr_t attrSizeCheck ARGS((KpInt32_t bytes));

KpHandle_t copyAttrList (PTAttribEntry_p startAttrListP);


/* do the "PTGetAttribute" function */
PTErr_t
	PTGetAttribute(PTRefNum_t PTRefNum, KpInt32_t attrTag, KpInt32_p size,
			KpChar_p attrString)
{
KpInt32_t		bytes;
KpHandle_t		startAttrList;
PTErr_t			errnum = KCP_SUCCESS;

	KpChar_t KPCPversion[] = KPCP_VERSION;

/* Check for valid pointers */
	if (size == NULL) {
		return KCP_BAD_PTR;
	}

	if (attrString == NULL) {
		return KCP_BAD_PTR;
	}

	if (attrTag == KCM_KCP_VERSION) {		/* special attribute? */
 		bytes = strlen (KPCPversion);

		if (bytes >= (*size)) {
			/* if the given size was too small copy as much of the string as will fit */
			strncpy (attrString, KPCPversion, (*size) -1);

			*(attrString + (*size) -1) = '\0';	/* null-terminate the string */
			errnum =  KCP_ATT_SIZE_TOO_SMALL;
		}
		else {
			strcpy(attrString, KPCPversion);	/* copy the entire attribute */
			errnum = KCP_SUCCESS;
		}
		(*size) = strlen(KPCPversion);			/* and size */
	}
	else if (attrTag == KCM_CP_RULES_DIR) {	/* special attribute? */
		initializedGlobals_p	iGP;

		iGP = getInitializedGlobals ();
		if (iGP == NULL) {
			return KCP_NO_PROCESS_GLOBAL_MEM;
		}

		/* return the path to the rules directory */
 		bytes = strlen (iGP->KCPDataDir);

		if (bytes >= (*size)) {		/* copy as much of the string as will fit */
			strncpy (attrString, iGP->KCPDataDir, (*size) -1);

			*(attrString + (*size) -1) = '\0';	/* null-terminate the string */
			errnum =  KCP_ATT_SIZE_TOO_SMALL;
		}
		else {	/* copy the entire attribute */
			strcpy (attrString, iGP->KCPDataDir);
			errnum = KCP_SUCCESS;
		}

		(*size) = bytes;
	}
	else {
		errnum = getPTStatus (PTRefNum);

		if ((errnum != KCP_PT_ACTIVE) && (errnum != KCP_PT_INACTIVE) && (errnum != KCP_SERIAL_PT)) {
			errnum = KCP_NOT_CHECKED_IN;
		}
		else {
			startAttrList = getPTAttr(PTRefNum);
			errnum = GetAttribute(startAttrList, attrTag, size, attrString);
		}
	}

	return (errnum);
}


/* do the "PTSetAttribute" function */
PTErr_t 
	PTSetAttribute(PTRefNum_t PTRefNum, KpInt32_t attrTag, KpChar_p attrString)
{
PTErr_t 	errnum = KCP_SUCCESS;
KpHandle_t	startAttr;
KpChar_t 	nullChar = '\0';
KpChar_t 	*tempAttrString;
KpInt32_t	AttrValue;

	if (attrTag == KCM_KCP_VERSION) {		/* special attribute? */
		errnum = KCP_INVAL_PTA_TAG;			/* may not be set */
	}
	else {
		/* if the string isn't NULL, put it into the list */
 		if (attrString != NULL) {

			/* Check for valid pointer */
			if (attrString == NULL) {
				goto ErrOut1;
			}
			if ((attrTag == KCM_IN_CHAIN_CLASS_2) || (attrTag == KCM_OUT_CHAIN_CLASS_2)) {
				AttrValue = KpAtoi(attrString);
				if ((AttrValue < KCM_CHAIN_MIN) || (AttrValue > KCM_CHAIN_MAX)){
					errnum = KCP_BAD_COMP_ATTR;			/* chainning class out of range */
					goto ErrOut1;
				}
			}
			tempAttrString = attrString;
			while (*tempAttrString != nullChar) {
				if (*tempAttrString == KP_NEWLINE) {
					errnum = KCP_INVAL_PTA_TAG;
					goto ErrOut1;
				}
				tempAttrString++;
			}
		}
		errnum = getPTStatus (PTRefNum);
		if ((errnum != KCP_PT_ACTIVE) && (errnum != KCP_PT_INACTIVE) && (errnum != KCP_SERIAL_PT)) {
			errnum = KCP_NOT_CHECKED_IN;
		}
		else {
			startAttr = getPTAttr(PTRefNum);
			errnum = SetAttribute(&startAttr, attrTag, attrString);

			setPTAttr(PTRefNum, startAttr);	/* set attribute start in case this was the first */
			
			if ((attrTag == KCM_IN_SPACE) || (attrTag == KCM_OUT_SPACE)) {
				checkDataClass (PTRefNum);
			}
		}
	}

ErrOut1:
	return (errnum);
}


/* calculate the size needed to store all attributes in a list */
KpInt32_t
	getAttrSize(KpHandle_t startAttrListH)
{
	PTAttribEntry_p startAttrListP;
	PTAttribCount_p tagCount;
	KpInt32_t			count, numTagEntries, loop;
	KpGenericPtr_t	attrString;

/* a 32 bit decimal number needs MAX_DIGITS characters plus null */
	KpChar_t numstr[MAX_DIGITS+1];

/* if start attribute is NULL, there are no attributes */
	if (startAttrListH == NULL) {
		return 0;
	}
	count = 0;
	startAttrListP = lockBuffer (startAttrListH);
	tagCount = (PTAttribCount_p)startAttrListP;
	(PTAttribCount_p)startAttrListP++;
	numTagEntries = tagCount->num_entries;
	
	for (loop = 0; loop < numTagEntries; loop++) {
		KpItoa(startAttrListP->tag, numstr);

			/* attribute size is length of attribute tag in ascii,
			 * length of attribute string, plus 2 for equal sign
			 * and newline terminator */
		attrString = lockBuffer (startAttrListP->string);
		count += strlen(numstr) + strlen(attrString) + 2;
		unlockBuffer (startAttrListP->string);
		startAttrListP++;
	}
	unlockBuffer (startAttrListH);
	count++;	/* add a terminating null */

	return count;
}


/* simulate reading attributes */
PTErr_t 
	readAttributes(	KpFd_p fd,
					KpInt32_t size,
					KpHandle_t * startAttrH)
{
KpChar_p	idstr, attPtr;
PTErr_t		errnum = KCP_SUCCESS;
KpInt32_t	i, j, attrTag, err, theChar;

	*startAttrH = NULL;						/* assume no attributes to set */

/* if size is zero, there are no attributes to read */
	if (size == 0) return KCP_SUCCESS;

	if ((size + KCP_PT_HEADER_SIZE) > PTCHECKINSIZE) return KCP_INVAL_PT_BLOCK;

	idstr = (KpChar_p) allocBufferPtr (size);	/* get memory for id string */
	if (idstr == NULL) {
		return KCP_NO_CHECKIN_MEM;
	}

	attPtr = idstr;							/* save the position of idstr */

/* get all the attributes into idstr and convert all newline chars to nulls */
	err = Kp_read (fd, attPtr, size);
	if (err != KCMS_IO_SUCCESS) {
		errnum = KCP_INVAL_PT_BLOCK;
		goto GetOut;
	}

	for (i = 0; i < size; i++) {
		if (attPtr[i] == KP_NEWLINE) {
			attPtr[i] = '\0';
		}
	}
/* now parse the idstr into individual attributes and put them in linked list */
	attPtr = idstr;	/* get beginning position of idstr */
	do {

		/* first get the tag by reading everything up to the equal sign */
		attrTag = 0;
		for (j = 0; (theChar = *attPtr++) != '='; j++) {
			if ((theChar < '0') || (theChar > '9')) {
				goto SkipAttr;		/* non-numeric tag, just ignore it */
			}

			attrTag = (attrTag * 10) + (theChar - '0');	/* add 'em up */

			if (j == MAX_DIGITS) {
				errnum = KCP_INVAL_PTA_TAG;	/* tag value is too large */
				goto GetOut;
			}
		}

		if (attrTag != 0) {			/* ignore attribute with tag of 0 */
			errnum = SetAttribute (startAttrH, attrTag, attPtr);	/* insert this attribute */
		}

SkipAttr:
		while (*attPtr++ != '\0') {}	 /* skip to start of next attribute */

	/* account for terminating null */
	} while ((attPtr < (idstr + size-1)) && (errnum == KCP_SUCCESS));

GetOut:
	freeBufferPtr((KpGenericPtr_t)idstr);		/* discard id string */

	return (errnum);
}


/* write attributes to external memory */
PTErr_t
	writeAttributes (		KpFd_p fd,
							KpHandle_t startAttrListH)
{
	PTAttribEntry_p startAttrListP;
	PTAttribCount_p tagCount;
	KpGenericPtr_t	attrString;
	PTErr_t			errnum = KCP_PT_HDR_WRITE_ERR;
	KpInt32_t			numTagEntries, loop;

/* need MAX_DIGITS chars for a 32 bit decimal number, plus = sign */
	KpChar_t	numstr[MAX_DIGITS+1];
	KpChar_t 	equal = '=';
	KpChar_t 	newline = KP_NEWLINE;
	KpChar_t 	nullChar = '\0';

	if (startAttrListH != NULL) {
		if ((KCP_PT_HEADER_SIZE + getAttrSize (startAttrListH)) > PTCHECKINSIZE) {
			return KCP_NO_CHECKIN_MEM;
		}

		startAttrListP = lockBuffer (startAttrListH);
		tagCount = (PTAttribCount_p)startAttrListP;
		(PTAttribCount_p)startAttrListP++;
		numTagEntries = tagCount->num_entries;
	
		for (loop = 0; loop < numTagEntries; loop++) {
			KpItoa(startAttrListP->tag, numstr);		/* convert tag to an ascii string */

	/* write the tag, an equal sign, and the attribute string to the PT block */
			attrString = lockBuffer (startAttrListP->string);
			if ( (Kp_write (fd, numstr, strlen(numstr)) != KCMS_IO_SUCCESS)   ||
	    		   (Kp_write (fd, &equal, 1) != KCMS_IO_SUCCESS)   ||
	    		   (Kp_write (fd, attrString, strlen(attrString)) != KCMS_IO_SUCCESS)  ||
	    		   (Kp_write (fd, &newline, 1) != KCMS_IO_SUCCESS) )  {
				unlockBuffer (startAttrListP->string);
				errnum = KCP_PT_BLOCK_TOO_SMALL;
				goto GetOut;
			}
			errnum = KCP_SUCCESS;
			unlockBuffer (startAttrListP->string);
			startAttrListP++;
		}

	/* terminate with a null character */
		if (Kp_write (fd, &nullChar, 1) != KCMS_IO_SUCCESS)
			errnum = KCP_PT_BLOCK_TOO_SMALL;
		else
			errnum = KCP_SUCCESS;
	}

GetOut:
	unlockBuffer (startAttrListH);
	return  (errnum);
}

/* return all the attribute tags of a given PT */
PTErr_t 
	PTGetTags(PTRefNum_t PTRefNum, KpInt32_p nTags, KpInt32_p tagArray)
{
KpHandle_t 		startAttrListH;
PTAttribEntry_p startAttrListP;
PTAttribCount_p tagCount;
KpInt32_t		totalEntries, numTags, loop;
PTErr_t			errnum;

	errnum = getPTStatus (PTRefNum);

	if ((errnum != KCP_PT_ACTIVE) && (errnum != KCP_PT_INACTIVE) && (errnum != KCP_SERIAL_PT)) {
		errnum = KCP_NOT_CHECKED_IN;
		goto ErrOut1;
	}

/* Check for valid pointers */
	if (nTags == NULL) {
		errnum = KCP_BAD_PTR;
		goto ErrOut1;
	}

	numTags = *nTags;	/* save the given number of tags */
	*nTags = 0;

	startAttrListH = getPTAttr(PTRefNum);
	startAttrListP = lockBuffer (startAttrListH);
	tagCount = (PTAttribCount_p)startAttrListP;
	(PTAttribCount_p)startAttrListP++;
	totalEntries = tagCount->num_entries;
	
	for (loop = 0; loop < totalEntries; loop++) {

		/* add tags to the array until they are exhausted or the given
		 * number is reached.  When the number has been reached, continue
		 * to count tags but stop adding to array */
		if ((*nTags <= numTags) && (tagArray != NULL)) {
			*tagArray++ = startAttrListP->tag;
		}

		(*nTags)++;							/* return the actual number of tags */
		startAttrListP++;
	}
	unlockBuffer (startAttrListH);
	errnum = KCP_SUCCESS;
	
ErrOut1:
	return (errnum);
}


/* free the memory used for an attribute list */
PTErr_t 
	freeAttributes(KpHandle_t startAttrListH)
{
	PTAttribEntry_p startAttrListP;
	PTAttribCount_p tagCount;
	KpInt32_t			totalEntries, loop;

	if (startAttrListH != NULL) {
		startAttrListP = lockBuffer (startAttrListH);
		tagCount = (PTAttribCount_p)startAttrListP;
		(PTAttribCount_p)startAttrListP++;
		totalEntries = tagCount->num_entries;

	/* scan through the list until all the attributes are freed */
		for (loop = 0; loop < totalEntries; loop++) {
			freeBuffer (startAttrListP->string);
			startAttrListP++;
		}
		tagCount->num_entries = 0;
		unlockBuffer (startAttrListH);
	}	
	return (KCP_SUCCESS);
}

/* scan each entry in the linked list until a matching tag is found */
static PTErr_t
	LinearScanList(KpHandle_t startAttrListH, PTAttribEntry_p attrEntry, KpInt32_t mode)
{
	PTAttribEntry_p startAttrListP;
	PTAttribCount_p tagCount;
	KpInt32_t			totalEntries, loop;
	PTErr_t			errnum = KCP_SUCCESS;

	if (startAttrListH == NULL) {
		return (KCP_NO_ATTR_MEM);
	}
	startAttrListP = lockBuffer (startAttrListH);
	tagCount = (PTAttribCount_p)startAttrListP;
	(PTAttribCount_p)startAttrListP++;
	totalEntries = tagCount->num_entries;

	for (loop = 0; loop < totalEntries; loop++) {
		if (startAttrListP->tag == attrEntry->tag) {
			break;
		}
		startAttrListP++;
	}
	switch(mode) {
		case ADD_TAG:
			if (loop == totalEntries) {
				tagCount->num_entries++;			/* must be a new entry (not an over write) */
			} else {
				freeBuffer (startAttrListP->string); /* free previous tag string */
			}
			*startAttrListP = *attrEntry;
			break;

		case FIND_TAG:
			if (loop != totalEntries) {
				attrEntry->string = startAttrListP->string;
			}
			break;

		case DELETE_TAG:
			if (loop != totalEntries) {
				freeBuffer (startAttrListP->string);
				for (; loop < totalEntries-1; loop++) {
					*startAttrListP = *(startAttrListP+1);
					startAttrListP++;
				}
				startAttrListP->tag = 0;
				startAttrListP->string = NULL;
				tagCount->num_entries--;
			}
			break;

		default:
			errnum = KCP_FAILURE;
	}
	unlockBuffer (startAttrListH);

	return (errnum);
}


/* search for an attribute with a given tag */
PTErr_t
	GetAttribute(KpHandle_t startAttr, KpInt32_t attrTag,
				KpInt32_p attrStrSize, KpChar_p attrString)
{
PTAttribEntry_t attrEntry;
KpGenericPtr_t	tmpAttrString;
PTErr_t			errnum;
KpInt32_t		bytes;

	attrEntry.tag = attrTag;
	attrEntry.string = NULL;
	LinearScanList(startAttr, &attrEntry, FIND_TAG);

	if (attrEntry.string == NULL) { /* no matching tag was found in the list */
		errnum = KCP_INVAL_PTA_TAG;
	}
	else {
		tmpAttrString = lockBuffer (attrEntry.string);
 		bytes = strlen(tmpAttrString);

		if (bytes >= (*attrStrSize)) {
			/* if the given size was too small copy as much of the string as will fit, */
			strncpy (attrString, tmpAttrString, (*attrStrSize)-1);

			*(attrString + (*attrStrSize-1)) = '\0';	/* null-terminate the string */
			errnum =  KCP_ATT_SIZE_TOO_SMALL;
		}
		else {
			strcpy(attrString, tmpAttrString);	/* copy the entire attribute */
			errnum = KCP_SUCCESS;
		}
		unlockBuffer (attrEntry.string);
		*attrStrSize = bytes;	/* send back the actual size of the attribute string */
	}
	return errnum;
}


static PTErr_t
	SetAttribute(KpHandle_t * startAttrListH, KpInt32_t attrTag,
				KpChar_p attrString)
{
	PTErr_t			errnum = KCP_SUCCESS;
	PTAttribEntry_t attrEntry;
	KpInt32_t		bytes;
	KpHandle_t		ourStringH;
	KpGenericPtr_t	ourStringP;

	if (startAttrListH == NULL) {
		return (KCP_NO_ATTR_MEM);
	}
	attrEntry.tag = attrTag;
	attrEntry.string = NULL;
	
/* only delete the old attribute if we're not going to replace it with a new one! */
/*	it's faster this way! (only one search) */

	if (attrString == NULL) {
		LinearScanList(*startAttrListH, &attrEntry, DELETE_TAG);	/* get rid of old attribute */
		return (KCP_SUCCESS);
	}
	if (*attrString == 0) {
		LinearScanList(*startAttrListH, &attrEntry, DELETE_TAG);	/* get rid of old attribute */
		return (KCP_SUCCESS);
	}
	bytes = strlen(attrString);

	errnum = attrSizeCheck(bytes);	/* must have valid string size */
	if (errnum != KCP_SUCCESS) {
		return errnum;
	}
	if (attrTag == 0) {
		return KCP_INVAL_PTA_TAG;
	}
	/* get memory for the attribute */
	ourStringH = allocBufferHandle (bytes +1);
	if (ourStringH == NULL) {
		return (KCP_NO_ATTR_MEM);
	}
	ourStringP = lockBuffer (ourStringH);
	strcpy (ourStringP, attrString);
	unlockBuffer (ourStringH);
	attrEntry.string = ourStringH;

	/* insert into list */
	errnum = AddAttribute(startAttrListH, attrTag, ourStringH);
	if (errnum != KCP_SUCCESS) {
		freeBuffer(ourStringH);	/* release the attribute's memory */
	}
	return (errnum);
}

/* make sure that an attribute string is within "reasonable" limits */
static 
	PTErr_t attrSizeCheck(KpInt32_t bytes)
{
	if ((bytes <= 0) || (bytes > KCM_MAX_ATTRIB_VALUE_LENGTH+1))
		return KCP_ATTR_TOO_BIG;		/* not a valid string size */
	else
		return KCP_SUCCESS;
}


/* add an attribute to an attribute list */
static PTErr_t 
	AddAttribute(KpHandle_t *startAttrH, KpInt32_t attrTag, KpHandle_t attrString)
{
	PTAttribEntry_t attrib;
	PTAttribCount_p tagCount, oldTagCount;
	KpHandle_t 		attrTable;
	
	attrib.tag = attrTag;								/* insert tag */
	attrib.string = attrString;							/* insert string */

	if (*startAttrH == NULL) {							/* if there are no attributes at all */
		/* get memory for the attribute */
		tagCount = allocBufferPtr (sizeof(PTAttribCount_t) +
					(sizeof(PTAttribEntry_t) * NUMBER_OF_ATTRIB_ENTRIES));
		if (tagCount == NULL) {
			return (KCP_NO_ATTR_MEM);
		}
		
		tagCount->num_entries = 0;							/* no entries yet. */
		tagCount->max_entries = NUMBER_OF_ATTRIB_ENTRIES;	/* maximum number of entries. */

		attrTable = getHandleFromPtr (tagCount);
		*startAttrH = attrTable;
	}
	else {
		oldTagCount = lockBuffer (*startAttrH);
		if (oldTagCount->num_entries == oldTagCount->max_entries) {
			tagCount = reallocBufferPtr (oldTagCount, (sizeof(PTAttribCount_t) +
						(sizeof(PTAttribEntry_t) * (oldTagCount->max_entries + NUMBER_OF_ATTRIB_ENTRIES))));
			if (tagCount == NULL) {
				return (KCP_NO_ATTR_MEM);
			}

			tagCount->max_entries += NUMBER_OF_ATTRIB_ENTRIES;

			attrTable = getHandleFromPtr (tagCount);
			*startAttrH = attrTable;
		}
	}

	LinearScanList(*startAttrH, &attrib, ADD_TAG);			/* add to end of list */

	return (KCP_SUCCESS);
}


/* copyAllAttr */
PTErr_t 
	copyAllAttr (	PTRefNum_t fromPTRefNum,
					PTRefNum_t toPTRefNum)
{
KpHandle_t 		fromAttrListH, newAttrListH, oldAttrListH;
PTAttribEntry_p	fromAttrListP;
PTErr_t			errnum = KCP_SUCCESS;

	fromAttrListH = getPTAttr(fromPTRefNum);
	fromAttrListP = lockBuffer (fromAttrListH);

	newAttrListH = copyAttrList(fromAttrListP);
	unlockBuffer (fromAttrListH);

	if (newAttrListH != NULL) {
		oldAttrListH = getPTAttr(toPTRefNum);
		if (oldAttrListH != NULL) {
			freeAttributes(oldAttrListH);
			freeBuffer (oldAttrListH);
		}

		setPTAttr(toPTRefNum, newAttrListH);	/* set attribute start */
	}
	else {
		errnum = KCP_NO_ATTR_MEM;
	}

	return (errnum);
}


KpHandle_t
	copyAttrList(PTAttribEntry_p startAttrListP)
{
PTAttribCount_p tagCount, newTagCount;
PTAttribEntry_p attribP, NewAttribP;
KpHandle_t 		tempStartAttrH, newStringH;
KpGenericPtr_t	orgStringP, newStringP;
KpInt32_t		bytes;
KpInt32_t		loop;

	tagCount = (PTAttribCount_p)startAttrListP;
	tempStartAttrH = allocBufferHandle ((sizeof(PTAttribEntry_t) * 
						tagCount->max_entries) + sizeof(PTAttribCount_t));
	if (tempStartAttrH == NULL) {
		return (NULL);
	}
	newTagCount = lockBuffer (tempStartAttrH);
	newTagCount->num_entries = tagCount->num_entries;						
	newTagCount->max_entries = tagCount->max_entries;
	attribP = (PTAttribEntry_p)tagCount + 1;
	NewAttribP = (PTAttribEntry_p)newTagCount + 1;
	for (loop = 0; loop < newTagCount->num_entries; loop++) {
		NewAttribP->tag = attribP->tag;
		orgStringP = lockBuffer (attribP->string);
 		bytes = strlen(orgStringP);
	/* get memory for the attribute */
		newStringH = allocBufferHandle (bytes +1);
		if (newStringH == NULL) {
			newTagCount->num_entries = loop;						
			unlockBuffer (attribP->string);
			unlockBuffer (tempStartAttrH);
			freeAttributes(tempStartAttrH);
			freeBuffer (tempStartAttrH);
			return (NULL);
		}
		newStringP = lockBuffer (newStringH);
		strcpy (newStringP, orgStringP);
		unlockBuffer (attribP->string);
		unlockBuffer (newStringH);
		NewAttribP->string = newStringH;
		
		NewAttribP++;
		attribP++;
	}

	unlockBuffer (tempStartAttrH);
	return (tempStartAttrH);
}

/* get an integer attribute and convert to binary */
PTErr_t		getIntAttr (	PTRefNum_t	PTRefNum,
				KcmAttribute	attrTag,
				KpInt32_t	maxValue,
				KpInt32_p	valueP)
{
PTErr_t	errnum;
KpInt32_t	attrSize, i1, theNum;
KpChar_t	attribute[KCM_MAX_ATTRIB_VALUE_LENGTH+1], theChar;
 
	/* get attribute */
	attrSize = KCM_MAX_ATTRIB_VALUE_LENGTH+1;
	errnum = PTGetAttribute (PTRefNum, attrTag, &attrSize, attribute);
	if (errnum != KCP_SUCCESS) {
		return errnum;
	}
 
	/* convert to binary */
	theNum = 0;
	for (i1 = 0; i1 < attrSize; i1++) {
		theChar = attribute[i1];
		/* get each character */
		if ((theChar >= '0') && (theChar <= '9')) {	/* validate */
			theNum = (theNum * 10) + (theChar - '0');	/* add 'em up */
		} else {
			errnum = KCP_BAD_COMP_ATTR;
			break;			/* not all decimal digits */
		}
	}
 
	*valueP = theNum;       /* return binary value */
 
	if ((errnum == KCP_SUCCESS) && (maxValue != KCP_NO_MAX_INTATTR)) {
		if ((*valueP < 1) || (*valueP > maxValue)) {
			errnum = KCP_BAD_COMP_ATTR;
		}
	}
 
	return errnum;
}

/* get integer attribute, return default of KCM_UNKNOWN if error */
KpInt32_t	getIntAttrDef(	PTRefNum_t      PTRefNum,
				KpInt32_t	AttrId)
{
PTErr_t		errnum;
KpInt32_t	attrIntValue;
 
	errnum = getIntAttr (PTRefNum, AttrId, KCP_NO_MAX_INTATTR, &attrIntValue);
	if (errnum != KCP_SUCCESS) {
		attrIntValue = KCM_UNKNOWN;
	}
 
	return attrIntValue;
}

