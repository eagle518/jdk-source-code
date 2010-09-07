/*
 * @(#)spattpr.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains functions for tag management.
			These functions are not needed for KCMS nor Java
			libraries.

			Pulled from spattr.c, spattrio.c and
				spattrnm.c on 7/2/99

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2000 by Eastman Kodak Company, all rights 
			reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spattpr.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spattpr.c $Revision:   2.1  $
		$Date: 3/15/02 11:39a $
		$Author: Msm $

	SCCS Revision:
		@(#)spattpr.c	1.2	12/19/03

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993 - 2000               ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/


#include "sprof-pr.h"
#include <string.h>
#include <stdio.h>


static SpStatus_t UInt32ToHexTxt( KpUInt32_t	Value, 
			KpInt32_p 	BufSize,  
			KpChar_p 	Buffer);

/*--------------------------------------------------------------------
* DESCRIPTION
 *      Test if Tag Exists - lifted from SpRawTagDataGet
 *
 * AUTHOR
 *      doro
 *
 * DATE CREATED
 *      August 10, 1996
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTagExists (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				KpBool_t	*TagExists)
{
SpStatus_t	status = SpStatSuccess;
KpInt32_t	index;
SpProfileData_t	FAR *profileData;
SpTagDirEntry_t	FAR *tagArray;
 
const KpInt32_t SpNotFound = -1;

	/* convert profile handle to pointer to locked memory */
	profileData = SpProfileLock (Profile);
	if (NULL == profileData)
		return SpStatBadProfile;
 
	/* Verify the Tag Array if Loaded.  Search Function
	   Only loads the Header */
	if (profileData->TagArray == NULL)
	{
		status = SpProfilePopTagArray(profileData);
		if (status != SpStatSuccess)
			return status;
	}
 
	/* locate the tag data */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer(profileData->TagArray);
	index = SpTagFindById (tagArray, TagId, profileData->TotalCount);

	*TagExists = (KpBool_t)((SpNotFound == index) ? KPFALSE : KPTRUE);

	/* unlock handles */
	unlockBuffer (profileData->TagArray);
	SpProfileUnlock (Profile);
	return status;
 
} /* SpTagExists */



#if !defined (SP_NO_TEXTFUNCTIONS)
/***************************************************************************
 * FUNCTION NAME
 *      SpTagGetString
 *
 * DESCRIPTION
 *      This function converts the tag value pointed to by TagValue to a string.
 *      The resulting string is copied to the buffer pointed to by Buffer.  The
 *      size of the buffer is specified in the location pointed to by BufSize.
 *      The size of the string placed in the buffer is returned in BufSize.
 *
 ***************************************************************************/
SpStatus_t KSPAPI SpTagGetString(SpTagValue_t *TagValue, 
			  KpInt32_p	BufSize,
			  KpChar_p	Buffer)
{
SpStatus_t	spStatus;
KpInt16_t	Language = 0;
KpInt16_t	Country = 0;

	switch (TagValue->TagType) {
		case Sp_AT_Text:
			spStatus = TextToString(TagValue, BufSize, Buffer);
			break;

		case Sp_AT_TextDesc:
			spStatus = TextDescToString(TagValue, BufSize, Buffer);
			break;

		case Sp_AT_MultiLanguage:
			spStatus = MultiLangToMLString(TagValue, &Language, &Country, BufSize, Buffer);
			break;

		case Sp_AT_Enum:
		case Sp_AT_Chromaticity:
		case Sp_AT_CrdInfo:
		case Sp_AT_Curve:
		case Sp_AT_Data:
		case Sp_AT_DateTime:
		case Sp_AT_DevSettings:
		case Sp_AT_Lut:
		case Sp_AT_Measurement:
		case Sp_AT_NamedColors:
		case Sp_AT_ParametricCurve:
		case Sp_AT_ProfileSeqDesc:
		case Sp_AT_ResponseCurve:
		case Sp_AT_SF15d16:
		case Sp_AT_Screening:
		case Sp_AT_Signature:
		case Sp_AT_UF16d16:
		case Sp_AT_Ucrbg:
		case Sp_AT_UInt16:
		case Sp_AT_UInt32:
		case Sp_AT_UInt64:
		case Sp_AT_UInt8:
		case Sp_AT_Viewing:
		case Sp_AT_XYZ:
		case Sp_AT_NamedColors2:
		case Sp_AT_ColorantTable:
		case Sp_AT_ColorantOrder:
			spStatus = SpStatNotImp;
			break;

		default:
			spStatus = SpStatBadTagType;
			break;
	}
	return spStatus;
} /* SpTagGetString */

/***************************************************************************
 * FUNCTION NAME
 *      SpTagGetMLString
 *
 * DESCRIPTION
 *      This function converts the tag value pointed to by TagValue to a string.
 *      The resulting string is copied to the buffer pointed to by Buffer.  The
 *      size of the buffer is specified in the location pointed to by BufSize.
 *      The size of the string placed in the buffer is returned in BufSize.
 *	The Language starts as -1 saying no preference.  If the language value
 *	is not -1, then that is the language to return.
 *
 ***************************************************************************/
SpStatus_t KSPAPI SpTagGetMLString(SpTagValue_t *TagValue, 
			KpInt16_p	Language,
			KpInt16_p	Country,
			KpInt32_p	BufSize,
			KpChar_p	Buffer)
{
SpStatus_t	spStatus;

	switch (TagValue->TagType) {
		case Sp_AT_Text:
			if ((*Language < 1) || (*Language == SpEnglish))
			{
				spStatus = TextToString(TagValue, BufSize, Buffer);
				*Language = SpEnglish;
				*Country = SpUSA;
			}
			break;

		case Sp_AT_TextDesc:
			if ((*Language < 1) || (*Language == SpEnglish))
			{
				spStatus = TextDescToString(TagValue, BufSize, Buffer);
				*Language = SpEnglish;
				*Country = SpUSA;
			}
			break;

		case Sp_AT_MultiLanguage:
			spStatus = MultiLangToMLString(TagValue, Language, Country, BufSize, Buffer);
			break;

		case Sp_AT_Enum:
		case Sp_AT_Chromaticity:
		case Sp_AT_CrdInfo:
		case Sp_AT_Curve:
		case Sp_AT_Data:
		case Sp_AT_DateTime:
		case Sp_AT_DevSettings:
		case Sp_AT_Lut:
		case Sp_AT_Measurement:
		case Sp_AT_NamedColors:
		case Sp_AT_ParametricCurve:
		case Sp_AT_ProfileSeqDesc:
		case Sp_AT_ResponseCurve:
		case Sp_AT_SF15d16:
		case Sp_AT_Screening:
		case Sp_AT_Signature:
		case Sp_AT_UF16d16:
		case Sp_AT_Ucrbg:
		case Sp_AT_UInt16:
		case Sp_AT_UInt32:
		case Sp_AT_UInt64:
		case Sp_AT_UInt8:
		case Sp_AT_Viewing:
		case Sp_AT_XYZ:
		case Sp_AT_NamedColors2:
		case Sp_AT_ColorantTable:
		case Sp_AT_ColorantOrder:
			spStatus = SpStatNotImp;
			break;

		default:
			spStatus = SpStatBadTagType;
			break;
	}
	return spStatus;
} /* SpTagGetString */



/***************************************************************************
 * FUNCTION NAME
 *      TextToString
 *
 * DESCRIPTION
 *      This function converts tags of type Sp_AT_Text to a string
 ***************************************************************************/
SpStatus_t TextToString(SpTagValue_t	*TagValue, 
			KpInt32_p	BufSize,
			KpChar_p	Buffer)
{
KpInt32_t	length;
SpStatus_t	spStatus = SpStatSuccess;

	if (*BufSize < 1)
		return SpStatBufferTooSmall;

	length = strlen(TagValue->Data.Text);

	if (length >= *BufSize) {
		length = *BufSize -1;
		spStatus = SpStatBufferTooSmall;
	}

	KpMemSet(Buffer, 0, *BufSize);

	strncpy(Buffer, TagValue->Data.Text, length);
	Buffer += length;
	Buffer = 0;
	*BufSize = length;

	return spStatus;
} /* TextToString */

/***************************************************************************
 * FUNCTION NAME
 *      TextDescToString
 *
 * DESCRIPTION
 *      This function converts tags of type Sp_AT_TextDesc to a string
 ***************************************************************************/
SpStatus_t TextDescToString(SpTagValue_t	*TagValue,
			    KpInt32_p		BufSize,
			    KpChar_p		Buffer)
{
KpInt32_t	length;
SpTextDesc_t	*TextDesc;
SpStatus_t	spStatus = SpStatSuccess;

	if (*BufSize < 1)
		return SpStatBufferTooSmall;

	TextDesc = &TagValue->Data.TextDesc;
	length = strlen(TextDesc->IsoStr);
	if (length >= *BufSize) {
		length = *BufSize -1;
		spStatus = SpStatBufferTooSmall;
	}

	KpMemSet(Buffer, 0, *BufSize);

	strncpy(Buffer, TextDesc->IsoStr, length);
	Buffer += length;
	Buffer = 0;
	*BufSize = length;

	return spStatus;
} /* TextDescToString */

/***************************************************************************
 * FUNCTION NAME
 *      MultiLangToMLString
 *
 * DESCRIPTION
 *      This function converts tags of type Sp_AT_TextDesc to a string
 ***************************************************************************/
SpStatus_t MultiLangToMLString(SpTagValue_t	*TagValue,
				KpInt16_p	Language,
				KpInt16_p	Country,
				KpInt32_p	BufSize,
				KpChar_p	Buffer)
{
KpInt32_t	length, Index;
SpMultiLang_t	*MultiLang;
SpStatus_t	spStatus = SpStatSuccess;
char		*Codes, *StrBuf;
KpUInt16_t	FAR *UniStr;

	if (*BufSize < 1)
		return SpStatBufferTooSmall;

	MultiLang = &TagValue->Data.MultiLang;

	if (*Language < 1)
	{
		/* Use first language in tag */
		Index = 0;
		*Language =MultiLang->StringInfo[Index].LanguageCode;
	} else
	{
		/* Find index for requested language */
		for (Index = 0; 
		     Index < MultiLang->Count && MultiLang->StringInfo[Index].LanguageCode != *Language; 
		     Index++)
		{
		}
		/* Requested Language not found - leave as preset */
		if (Index == MultiLang->Count)
			return spStatus;  /* Leave it as UNKNOWN */
	}

	length = MultiLang->StringInfo[Index].StringLength/2;
	if (length >= *BufSize) {
		length = *BufSize -1;
		spStatus = SpStatBufferTooSmall;
	}

	KpMemSet(Buffer, 0, *BufSize);

	UniStr = MultiLang->StringInfo[Index].UniString;
	if (NULL == UniStr)
		return SpStatBadTagType;

	Codes = (char *)UniStr;
	/* Check for Byte Swapping */
	if ( ! *Codes)
		Codes++;

	Index = 0;
	StrBuf = Buffer;
	while ((Index < length) && (*Codes))
	{
		*StrBuf = *Codes;
		StrBuf++;
		Codes += 2;
		Index++;
	}

	Buffer += length;
	Buffer = 0;
	*BufSize = length;

	return spStatus;
} /* MultiLangToString */



/************************************************************
 * FUNCTION NAME
 *      SpProfileGetHeaderString
 *
 * DESCRIPTION
 *      This function converts a specified profile header field 
 *      value into a string.  *BufSize contains the size of the 
 *      Buffer into which the string will be written. The actual 
 *      string size is returned in BufSize.
 *
 ************************************************************/
SpStatus_t KSPAPI SpProfileGetHeaderString(SpSearchType_t	hdrItem,
				    SpHeader_t		*hdr,
				    KpInt32_p		BufSize,
				    KpChar_p		Buffer)
{
SpStatus_t	spStat;
KpInt32_t	value;
KpF15d16XYZ_t	aF15d16XYZ_Value;
KpUInt32_t	uintValue;
KpChar_p	IDPtr;
KpInt32_t	IDSize;
KpInt32_t	IDTotalSize;

switch (hdrItem) {

	case SPSEARCH_PROFILECLASS:
		value = hdr->DeviceClass;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_DEVICECOLORSPACE:
		value = hdr->DataColorSpace;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_CONNECTIONSPACE:
		value = hdr->InterchangeColorSpace;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_PREFERREDCMM:
		value = hdr->CMMType;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_PLATFORM:
		value = hdr->Platform;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_DEVICEMFG:
		value = hdr->DeviceManufacturer;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_DEVICEMODEL:
		value = hdr->DeviceModel;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_PROFILEFLAGS:
		uintValue = hdr->Flags;
		spStat = UInt32ToHexTxt(uintValue, BufSize, Buffer);
		break;
	case SPSEARCH_DEVICEATTRIBUTESHI:
		uintValue = hdr->DeviceAttributes.hi;
		spStat = UInt32ToHexTxt(uintValue, BufSize, Buffer);
		break;
	case SPSEARCH_DEVICEATTRIBUTESLO:
		uintValue = hdr->DeviceAttributes.lo;
		spStat = UInt32ToHexTxt(uintValue, BufSize, Buffer);
		break;
	case SPSEARCH_RENDERINGINTENT:
		uintValue = hdr->RenderingIntent;
		spStat = UInt32ToTxt(uintValue, BufSize, Buffer);
		break;
	case SPSEARCH_ILLUMINANT:
		aF15d16XYZ_Value = hdr->Illuminant;
		spStat = F15d16XYZToTxt(aF15d16XYZ_Value, BufSize, Buffer);
		break;
	case SPSEARCH_VERSION:
		uintValue = hdr->ProfileVersion;
		spStat = UInt32ToHexTxt(uintValue, BufSize, Buffer);
		break;
	case SPSEARCH_ORIGINATOR:
		value = hdr->Originator;
		spStat = SignatureToTxt(value, BufSize, Buffer);
		break;
	case SPSEARCH_PROFILEID:
		IDSize = *BufSize;
		IDPtr = Buffer;
		uintValue = hdr->ProfileID.a;
		spStat = UInt32ToHexTxt(uintValue, &IDSize, IDPtr);
		IDTotalSize = IDSize;
		IDPtr += IDSize; /* move past the first part just gotten */
		if (*BufSize > IDTotalSize)
		{
			*IDPtr++ = ' ';
			IDTotalSize++;
		}
		IDSize = *BufSize - IDTotalSize;
		uintValue = hdr->ProfileID.b;
		spStat = UInt32ToHexTxt(uintValue, &IDSize, IDPtr);
		IDTotalSize += IDSize;
		IDPtr += IDSize; /* move past the second part just gotten */
		if (*BufSize > IDTotalSize)
		{
			*IDPtr++ = ' ';
			IDTotalSize++;
		}
		IDSize = *BufSize - IDTotalSize;
		uintValue = hdr->ProfileID.c;
		spStat = UInt32ToHexTxt(uintValue, &IDSize, IDPtr);
		IDTotalSize += IDSize;
		IDPtr += IDSize; /* move past the third part just gotten */
		if (*BufSize > IDTotalSize)
		{
			*IDPtr++ = ' ';
			IDTotalSize++;
		}
		IDSize = *BufSize - IDTotalSize;
		uintValue = hdr->ProfileID.d;
		spStat = UInt32ToHexTxt(uintValue, &IDSize, IDPtr);
		break;

	default:
		spStat = SpStatUnsupported;
		break;
	}
return spStat;
}



/************************************************************
 * FUNCTION NAME
 *	SignatureToTxt
 *
 * DESCRIPTION
 *	This function converts a SpSig_t value into a string. 
 *      *BufSize contains the size of the Buffer into which 
 *      the string will be written. The actual string size is 
 *      returned in BufSize.
 *
 ************************************************************/
SpStatus_t SignatureToTxt(KpInt32_t	value, 
			  KpInt32_p	BufSize,
			  KpChar_p	Buffer)
{
SpStatus_t	spStat = SpStatSuccess;
char		tempStr[5];
KpInt32_t	length = 4;
	
	if (*BufSize < 1)
		return SpStatBufferTooSmall;

	tempStr[0] = (char) (value >> 24);
	tempStr[1] = (char) (value >> 16);
	tempStr[2] = (char) (value >> 8);
	tempStr[3] = (char) (value);
	tempStr[4] = '\0';

	if (length >= *BufSize)
	{
		length = *BufSize - 1;
		spStat = SpStatBufferTooSmall;
	}
			
	strncpy(Buffer, tempStr, length);
	Buffer += length;
	*Buffer = 0;
	*BufSize = length;

	return spStat;
}


/************************************************************
 * FUNCTION NAME
 *	F15d16ToTxt
 *
 * DESCRIPTION
 *	This function converts a KpF15d16_t value into a string. 
 * *BufSize contains the size of the Buffer into which the string 
 * will be written. The actual string size is returned in BufSize.
 *
 ************************************************************/
 SpStatus_t F15d16ToTxt(KpF15d16_t	value, 
			KpInt32_p	BufSize,  
			KpChar_p 	Buffer)
{
char		DefStr[] = "0.000000";
char		OneStr[] = "1.000000";
KpUInt32_t	Ivalue;
KpInt32_t	length;
SpStatus_t	spStat = SpStatSuccess;
KpTChar_t	tempString[32];
KpInt32_t	DecStart, NumChars;

	if (*BufSize < 1)
		return SpStatBufferTooSmall;

	/* value must be divided by 2**16 to become a double 
	 * then round up for trunctation to integer and multiple
	 * to get the decimal part into the integer range. */
	Ivalue = (KpUInt32_t)((KpF15d16ToDouble(value) + 0.0000005) 
                               * 1000000); 
	/* Make room for null terminator */
	*BufSize -= 1;
	/* Max characters are 8 plus null terminator */
	if (*BufSize > 8)
		*BufSize = 8;

	/* Can exceed due to round up */
	if (Ivalue >= 1000000)
	{
		strncpy(Buffer, OneStr, *BufSize);
		Buffer += *BufSize;
		*Buffer = 0;
	} else
	{
		/* Fill in leading zeros */
		strncpy(Buffer, DefStr, *BufSize);

		/* Do not want sign */
		Ultoa (Ivalue, tempString, 10);
		length = strlen(tempString);

		/* Find first non-zero char location */
		DecStart = 8 - length;
		/* Move past 0. and leading 0's */
		if (DecStart < *BufSize)
		{
			Buffer += DecStart;
			/* find number of non-zero chars 
			   that fit into buffer */
			NumChars = *BufSize - DecStart;
			strncpy(Buffer, tempString, NumChars);
			Buffer += NumChars;
		} else
			Buffer += *BufSize;

		*Buffer = 0;
	}

	return spStat;	
}


/************************************************************
 * FUNCTION NAME
 *	F15d16sToTxt
 *
 * DESCRIPTION
 *	This function converts an array of  KpF15d16_t values 
 * into a string. The values are separated by spaces. *BufSize 
 * contains the size of the Buffer into which the string will
 * be written. The actual string size is returned in BufSize.
 *
 ************************************************************/
 SpStatus_t F15d16sToTxt(KpUInt32_t	count, 
			 KpF15d16_t	FAR *Values, 
			 KpInt32_p 	BufSize,  
			 KpChar_p 	Buffer)
{
KpUInt32_t	i;
SpStatus_t	spStat = SpStatSuccess;
KpChar_p	Buf;
KpInt32_t	bufSize, remainingBufSize;
	
	/* init local variables */
	Buf = Buffer;
	remainingBufSize = bufSize = *BufSize;
	/* loop over each value */
	for (i = 0; i < count; i++, Values++) {
		if (0 != i)
		{
			if (remainingBufSize)
			{
				/* add space between values in string */
				strcpy (Buf, " ");
				Buf++;
				remainingBufSize--;
			}
			else
			{
				return SpStatBufferTooSmall;
			}
		}
		bufSize = remainingBufSize;
		if ((spStat = F15d16ToTxt (*Values, &bufSize,  Buf)) != SpStatSuccess)
			return spStat;
		Buf = Buf + bufSize;	/* advance buffer pointer */
		remainingBufSize = remainingBufSize - bufSize;
	}
		
	return spStat;	
}


/************************************************************
 * FUNCTION NAME
 *	F15d16XYZToTxt
 *
 * DESCRIPTION
 *	This function converts a KpF15d16XYZ_t value into a 
 * string. *BufSize contains the size of the Buffer into 
 * which the string will be written. The actual string size 
 * is returned in BufSize.
 *
 ************************************************************/
 SpStatus_t F15d16XYZToTxt(KpF15d16XYZ_t	value, 
			   KpInt32_p 		BufSize,  
			   KpChar_p 		Buffer)
{
SpStatus_t	SpStat = SpStatSuccess;
KpChar_p	Buf;
KpInt32_t	bufSize, remainingBufSize;
KpF15d16_t	tempValue;
	
	/* init local variables */
	Buf = Buffer;
	remainingBufSize = bufSize = *BufSize;
	/* loop over each value (XYZ)*/
	tempValue = value.X;
	
	if ((SpStat = F15d16ToTxt (tempValue, &bufSize,  Buf)) != SpStatSuccess)
		return SpStat;
	remainingBufSize = remainingBufSize - bufSize;
	Buf = Buf + bufSize;

	if (remainingBufSize)
	{
		strcpy(Buf, " ");
		Buf++;
		remainingBufSize--;
	}
	else
	{
		return SpStatBufferTooSmall;
	}

	tempValue = value.Y;
	bufSize = remainingBufSize;
	if ((SpStat = F15d16ToTxt (tempValue, &bufSize,  Buf)) != SpStatSuccess)
		return SpStat;
	remainingBufSize = remainingBufSize - bufSize;
	Buf = Buf + bufSize;

	if (remainingBufSize)
	{
		strcpy(Buf, " ");
		Buf++;
		remainingBufSize--;
	}
	else
	{
		return SpStatBufferTooSmall;
	}
		
	tempValue = value.Z;
	bufSize = remainingBufSize;
	if ((SpStat = F15d16ToTxt (tempValue, &bufSize,  Buf)) != SpStatSuccess)
		return SpStat;
	remainingBufSize = remainingBufSize - bufSize;
	*BufSize = *BufSize - remainingBufSize;
	return SpStat;

}
/************************************************************
 * FUNCTION NAME
 *	Ultoa
 *
 * DESCRIPTION
 *	This function converts a UInt32 value into a string. 
 *
 ************************************************************/
KpTChar_p Ultoa(KpUInt32_t	Value, 
		KpTChar_p	String, 
		int		Radix)
{
#if defined (KPWIN)
	return _ultoa (Value, String, Radix);
#else
char	FAR *Ptr;
int	i, j;
char	c;

	if (16 == Radix)
	{
/*		sprintf (String, "%uld", Value);  Not supported on 68*/
/* Leave until kcms_sys has a KpUItoa routine */
		Ptr = String;
		do {
			if (Value % 16 > 9)
				*Ptr++ = (char) (Value % 16 - 9 + 'A');
			else
				*Ptr++ = (char) (Value % 16 + '0');
		} while ((Value /= 16) > 0);
		*Ptr = '\0';
 
		j = (int)strlen (String) - 1;
		for (i = 0; i < j; i++, j--) {
			c = String [i];
			String [i] = String [j];
			String [j] = c;
		}

	}
	else
		KpItoa (Value, String);

	return String;
#endif
}


/************************************************************
 * FUNCTION NAME
 *	UInt32ToTxt
 *
 * DESCRIPTION
 *	This function converts a UInt32 value into a string. 
 * *BufSize contains the size of the Buffer into which the 
 * string will be written. The actual string size is returned 
 * in BufSize.
 *
 ************************************************************/
SpStatus_t UInt32ToTxt( KpUInt32_t	Value, 
			KpInt32_p 	BufSize,  
			KpChar_p 	Buffer)
{
	KpInt32_t	length;
	SpStatus_t	spStat = SpStatSuccess;
	char		tempBuf [12];

	if (*BufSize < 1)
		return SpStatBufferTooSmall;

	Ultoa (Value, tempBuf, 10);
	
	length = strlen(tempBuf);
	if (length >= *BufSize)
	{
		length = *BufSize - 1;
		spStat = SpStatBufferTooSmall;
	}
			
	strncpy(Buffer, tempBuf, length);
	Buffer += length;
	*Buffer = 0;
	*BufSize = length;

	return spStat;	
}
/************************************************************
 * FUNCTION NAME
 *	UInt32ToHexTxt
 *
 * DESCRIPTION
 *	This function converts a UInt32 value into a string. 
 *	The format oif the string is "0x012345"
 * *BufSize contains the size of the Buffer into which the 
 * string will be written. The actual string size is returned 
 * in BufSize.
 *
 ************************************************************/
static SpStatus_t UInt32ToHexTxt( KpUInt32_t	Value, 
			KpInt32_p 	BufSize,  
			KpChar_p 	Buffer)
{
	KpInt32_t	length;
	SpStatus_t	spStat = SpStatSuccess;
	char		tempBuf [12];

	if (*BufSize < 1)
		return SpStatBufferTooSmall;

	KpLtos(Value, tempBuf);

	length = strlen(tempBuf);
	if (length+3  >= *BufSize)
	{
		length = *BufSize - 3;
		spStat = SpStatBufferTooSmall;
	}
			
	if (length > 0 ) {
		strcpy (Buffer, "0x");
		Buffer += 2;
		strncpy(Buffer, tempBuf, length);
		*BufSize = length + 2;
	}
	else
	{
		length = *BufSize - 1;
		strncpy(Buffer, "0x", length);
		*BufSize = length;
	}
	Buffer += length;
	*Buffer = 0;

	return spStat;	
}


/************************************************************
/************************************************************
 * FUNCTION NAME
 *	pfUInt32sToTxt
 *
 * DESCRIPTION
 *	This function converts an array of UInt32 values into 
 * a string. The count arg specifies how many UInt32s there are. 
 * The string has a space between UInt32 values. *BufSize contains 
 * the size of the Buffer into which the string will be written. 
 * The actual string size is returned in BufSize.
 *
 ************************************************************/

SpStatus_t UInt32sToTxt(KpUInt32_t	count,
			KpUInt32_t 	FAR *Values, 
			KpInt32_p 	BufSize,  
			KpChar_p 	Buffer)
{
	KpUInt32_t	i;
	SpStatus_t	SpStat = SpStatSuccess;
	KpChar_p	Buf;
	KpInt32_t	bufSize, remainingBufSize;
	
	/* init local variables */
	Buf = Buffer;
	remainingBufSize = bufSize = *BufSize;
	/* loop over each value */
	for (i = 0; i < count; i++, Values++) {
		if (0 != i)
		{
			if (remainingBufSize)
			{
				/* add space between values in string */
				strcpy (Buf, " ");
				Buf++;
				remainingBufSize--;
			}
			else
			{
				return SpStatBufferTooSmall;
			}
		}
		bufSize = remainingBufSize;
		if ((SpStat = UInt32ToTxt (*Values, &bufSize,  Buf)) != SpStatSuccess)
			return SpStat;
		Buf = Buf + bufSize;	/* advance buffer pointer */
		remainingBufSize = remainingBufSize - bufSize;
	}
	return SpStat;
}
#endif



/*--------------------------------------------------------------------
 * DESCRIPTION
 *	 Finds tags that share the same data. The IDs of tags that share the 
 *	 the same data as the input ID are filled in caller provided space.
 *	 Also, the number of matches found is returned.
 *
 * AUTHOR
 * 	shaque
 *
 * DATE CREATED
 *	Oct 10, 1997
 *------------------------------------------------------------------*/


 SpStatus_t KSPAPI SpProfileGetSharedTags(SpProfile_t Profile,
								  SpTagId_t		TagId,
								  SpTagId_t		*Matched_TagIds,
								  KpInt32_t		*num_matched_tags)
{

	SpProfileData_t		FAR	*ProfileData;
	SpTagDirEntry_t	FAR *tagArray;
	KpInt32_t			index, i, num_of_matches = 0;
	KpGenericPtr_t	FAR tagData1, FAR tagData2;
	KpUInt32_t			*temp_tagId_buffer;


	*num_matched_tags = 0;
	/* convert profile handle to pointer to locked memory */
	ProfileData = SpProfileLock(Profile);
	if (NULL == ProfileData)
		return (SpStatBadProfile);

	/* Check if Profile found via Search function */
	if (ProfileData->TagArray == NULL)
	/* If so, it needs the Tag Array Loaded */
		SpProfilePopTagArray(ProfileData);

	tagArray = (SpTagDirEntry_t FAR *) 
				lockBuffer (ProfileData->TagArray);

	temp_tagId_buffer =(KpUInt32_t *)allocBufferPtr(ProfileData->TotalCount * sizeof(KpUInt32_t));
	if (temp_tagId_buffer ==NULL)
	{
		unlockBuffer(ProfileData->TagArray);
		SpProfileUnlock(Profile);
		return(SpStatMemory);
	}
 
	for (index = 0; index < ProfileData->TotalCount; index++)
	{
		if (tagArray[index].TagId == (TagId))
			break;
	}
	
	/* Reached the end of the tag array*/
	if (index >= ProfileData->TotalCount)
	{
		*num_matched_tags = 0;
		unlockBuffer(ProfileData->TagArray);
		SpProfileUnlock(Profile);
		freeBufferPtr(temp_tagId_buffer);
		return(SpStatSuccess);
	}

	for(i = index+1; i < ProfileData->TotalCount; i++)
	{
		if(tagArray[index].TagDataSize == tagArray[i].TagDataSize)
		{
			/* compare the size of the data */
			tagData1 = lockBuffer(tagArray[index].TagData);
			tagData2 = lockBuffer(tagArray[i].TagData);

			if (0 == KpMemCmp (tagData1, tagData2,
							tagArray[index].TagDataSize)) 
			{
				temp_tagId_buffer[num_of_matches++] = (KpUInt32_t)tagArray[i].TagId;
			}
			unlockBuffer(tagArray[index].TagData);
			unlockBuffer(tagArray[i].TagData);
		}
	}

	/* If matches were found, fill in the tagID array with the matched IDs */
	if (num_of_matches > 0)
		{
			for(i=0; i<num_of_matches; i++)
				Matched_TagIds[i] = (SpTagId_t)temp_tagId_buffer[i];
		}

	unlockBuffer(ProfileData->TagArray);
	SpProfileUnlock(Profile);
	freeBufferPtr(temp_tagId_buffer);
	*num_matched_tags = num_of_matches;
	return(SpStatSuccess);
}

 /*--------------------------------------------------------------------
 * DESCRIPTION
 *	 Gets the number of tags in a Profile.
 * AUTHOR
 * 	shaque
 *
 * DATE CREATED
 *	Oct 10, 1997
 *------------------------------------------------------------------*/

 SpStatus_t KSPAPI SpProfileGetTagCount(
				SpProfile_t		Profile,
				KpInt32_t		*tagCount)
{
	SpProfileData_t		FAR *ProfileData;
	KpInt32_t			numValidTags = 0, index;
	SpTagDirEntry_t		FAR	*tagArray;

/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData) {
		return SpStatBadProfile;
	}

/* lock the tag array */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);

/* search for valid tags */
	for (index = 0; index < ProfileData->TotalCount; index++) {
		if (tagArray[index].TagDataSize != -1) {
			numValidTags++;
		}
	}

	*tagCount = numValidTags;

/* unlock tag array handle */
	unlockBuffer (ProfileData->TagArray);

/* unlock the profile */
	SpProfileUnlock (Profile);

	return SpStatSuccess;
}


 /*--------------------------------------------------------------------
 * DESCRIPTION
 *	 Test Lut Tag ID's match Lut Type
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/

SpStatus_t SpTagTestLut(
				SpTagId_t	TagId,
				void		*TagData)
{
	SpSig_t		TagType;
	char		*Buf;

/* Check AToB TagIds are not BToA Tag Type */
	if ((TagId == SpTagAToB0) ||
	    (TagId == SpTagAToB1) ||
	    (TagId == SpTagAToB2))
	{
		Buf = TagData;
		TagType = SpGetUInt32(&Buf);
		if (TagType == SpTypeLutBA)
			return SpStatBadTagType;
	}


/* Check BToA TagIds are not AToB Tag Type */
	if ((TagId == SpTagBToA0) ||
	    (TagId == SpTagBToA1) ||
	    (TagId == SpTagBToA2))
	{
		Buf = TagData;
		TagType = SpGetUInt32(&Buf);
		if (TagType == SpTypeLutAB)
			return SpStatBadTagType;
	}


	return SpStatSuccess;
}
     

