/*
 * @(#)t1.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T1.H
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
#ifndef __T2K_T1__
#define __T2K_T1__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#ifdef ENABLE_T1


#define ENABLE_DECRYPT
/* #define ENABLE_ENCRYPT */

#ifdef ENABLE_DECRYPT
#define	ENABLE_EITHER_CRYPT
#endif

#ifdef ENABLE_ENCRYPT
#undef	ENABLE_EITHER_CRYPT
#define	ENABLE_EITHER_CRYPT
#endif

#define		kMaxStackValues		32
#define 	T1_MAX_MASTERS 16

typedef struct {
    tt_uint16 unicodeIndex;
    tt_uint16 glyphIndex;
    void* next;
} unicodeToGI;

typedef struct {
	/* private */
	tsiMemObject *mem;
	tt_uint8 *dataInPtr;
	tt_uint8 *decryptedData;
	tt_int32 dataLen;
	tt_int32 eexecGO;
	tt_int32 charstringsGO;
	
	tt_int32 x, y;
	tt_int32 flexOn;
	tt_int32 flexCount;
	
	
	short	lenIV;
	
	tt_uint8 *encoding;
	short NumCharStrings;
        short notdefGlyphIndex; /* Glyph index of the ".notdef" character */
	unicodeToGI **unicodeToGITable; /* Hashtable of unicode->glyphIndex mapping */
	tt_uint8  **charData;  /* CharStrings big array of pointers to character data */
	short  numSubrs;
	tt_uint8  **subrsData; /* Array of pointers to subroutines */

	short		gNumStackValues;
	tt_int32       	gStackValues[kMaxStackValues]; /* kMaxStackValues is the max allowed */
	
	
	int numMasters;
	int numAxes;
	F16Dot16 WeightVector[T1_MAX_MASTERS]; /* 0..(numMasters-1) */
	
	/* public */
	GlyphClass *glyph;
	hmtxClass *hmtx, *noDelete_hmtx;

	tt_int32 lsbx;
	tt_int32 lsby;
	tt_int32 awx;
	tt_int32 awy;
	
	tt_int32 upem;
	tt_int32 maxPointCount;
	tt_int32 ascent;
	tt_int32 descent;
	tt_int32 lineGap;
	tt_int32 advanceWidthMax;
	F16Dot16 italicAngle;
	
	F16Dot16 m00, m01, m10, m11;
} T1Class;

#ifdef ENABLE_MAC_T1
char * ExtractPureT1FromMacPOSTResources( tsiMemObject *mem, short refNum, tt_uint32 *length );
#endif
unsigned char *ExtractPureT1FromPCType1( unsigned char *src, tt_uint32 *length );


T1Class *tsi_NewT1Class( tsiMemObject *mem, tt_uint8 *data, tt_int32 dataLen );

void ParseCharString( T1Class *t, tt_uint16 charCode );

tt_uint16 tsi_T1GetGlyphIndex( T1Class *t, tt_uint32 charCode );
GlyphClass *tsi_T1GetGlyphByIndex( T1Class *t, tt_uint16 index, tt_uint16 *aw );
GlyphClass *tsi_T1GetGlyphByCharCode( T1Class *t, tt_uint32 charCode, tt_uint16 *aw );

tt_int32 tsi_T1GetParam( T1Class *t, const tt_uint8 *param, tt_int32 defaultValue );
F16Dot16 tsi_T1GetFixedParam( T1Class *t, const tt_uint8 *param, F16Dot16 defaultValue );

void tsi_DeleteT1Class( T1Class *t );

#endif /* ENABLE_T1 */


#ifdef ENABLE_CFF

#define Card8	tt_uint8	/* 0..255 		1 byte unsigned number */
#define Card16	tt_uint16	/* 0..65535		2 byte unsigned number */
#define OffSize tt_uint8	/* 1..4 		1 byte unsigned number, specifies the size of an Offset field(s) */
#define SID		tt_uint16	/* 0 - 64999	2 byte string identifier */

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	tt_uint32 baseDataOffset;
	
	OffSize	offSize;
	
	tt_uint32 *offsetArray; /* tt_uint32 offsetArray[ count + 1 ] */ 

	/* public */
	Card16	count;
} CFFIndexClass;

#define CFF_MAX_STACK 64
#define CFF_MAX_MASTERS 16



typedef struct {
	Card16 version;
	Card16 Notice;
	Card16 FullName;
	Card16 FamilyName;
	Card16 Weight;
	
	tt_int32 UniqueId;
	tt_int32 bbox_xmin, bbox_ymin, bbox_xmax, bbox_ymax;
	F16Dot16 italicAngle;
	
	tt_int32 charset;
	tt_int32 Encoding;
	tt_int32 Charstrings;
	tt_int32 PrivateDictSize, PrivateDictOffset;
	
	int numAxes;
	int numMasters;
	int lenBuildCharArray;
	F16Dot16 *buildCharArray; /* the transient array */
	F16Dot16 defaultWeight[CFF_MAX_MASTERS];
	SID NDV; /* SID of the Normalize Design vector subroutine */
	SID CDV; /* SID of the Convert Design vector subroutine */
	/* The registry */
	F16Dot16 reg_WeightVector[CFF_MAX_MASTERS]; 			/* item 0 */
	F16Dot16 reg_NormalizedDesignVector[CFF_MAX_MASTERS];	/* item 1 */
	F16Dot16 reg_UserDesignVector[CFF_MAX_MASTERS];			/* item 2 */
	
	
	/* Font Matrix */
	F16Dot16 m00, m01, m10, m11;
} TopDictInfo;

typedef struct {
	tt_int32 Subr, SubrOffset;
	tt_int32 defaultWidthX;
	tt_int32 nominalWidthX;
} PrivateDictInfo;

#define sidLimit 512

typedef struct {
	/* private */
	tsiMemObject *mem;
	
	InputStream *in;
	
	/* Global font data */
	tt_int32 NumCharStrings;
	
	/* Encoding */
	SID charCodeToSID[256];
	/* charset */
	SID *gIndexToSID; /* [ NumCharStrings ] */
	/* our T2K built lookup to find chars. */
	tt_uint16 SIDToGIndex[sidLimit];
	
	hmtxClass *hmtx;  /* [ NumCharStrings ] */
	tt_int32 upem;
	tt_int32 maxPointCount;
	tt_int32 ascent;
	tt_int32 descent;
	tt_int32 lineGap;
	tt_int32 advanceWidthMax;
	F16Dot16 italicAngle;
	

	
	/* Begin Type2BuildChar state */
	F16Dot16 gStackValues[ CFF_MAX_STACK ];
	tt_int32 gNumStackValues;
	GlyphClass *glyph;
	tt_int32 x, y, awy, awx;
	tt_int32 lsbx, lsby;
	int numStemHints;
	int pointAdded;
	int widthDone;
	tt_uint16 seed;
	/* End Type2BuildChar state */
	
	/* Begin Header */
	Card8	major;
	Card8	minor;
	Card8	hdrSize;
	OffSize	offSize;
	/* End Header */
	
	/* Name Index */
	CFFIndexClass *name; /* NumFonts in this set == name->count !!! */

	/* Top DICT Index */
	CFFIndexClass *topDict; /* per-font */
	TopDictInfo   topDictData;

	/* String Index */
	CFFIndexClass *string; /* shared by all fonts */

	/* Global Subr Index */
	CFFIndexClass *gSubr;
	tt_int32 gSubrBias;
	
	/* Encoding */
	
	/* Charsets */
	
	/* FDSelect (CIDFonts only) */
	
	/* CharStrings, per font, found through the topDict */
	CFFIndexClass *CharStrings;
	
	/* Font DICT Index per-font (CIDFonts only) */
	
	/* Private DICT, per-font */
	PrivateDictInfo privateDictData;

	/* Private Subr INDEX, per-font or per-PRivate DICT for CIDFonts */
	/* Local Subr Index */
	CFFIndexClass *lSubr;
	tt_int32 lSubrBias;
	
	/* Copyright and trandemark notices */


	/* public */
} CFFClass;

CFFClass *tsi_NewCFFClass( tsiMemObject *mem, InputStream *in, tt_int32 fontNum );
void tsi_DeleteCFFClass( CFFClass *t );

tt_uint16 tsi_T2GetGlyphIndex( CFFClass *t, tt_uint32 charCode );
GlyphClass *tsi_T2GetGlyphByIndex( CFFClass *t, tt_uint16 index, tt_uint16 *aw );
GlyphClass *tsi_T2GetGlyphByCharCode( CFFClass *t, tt_uint32 charCode, tt_uint16 *aw );

#endif /* ENABLE_CFF */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __T2K_T1__ */
