/*
 * @(#)sfnt_types.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* $Revision: 1.1 $ */
/* graphics:
	font file structures
	by Cary Clark, Georgiann Delaney, Herb Derby, Michael Fairman, Pablo Fernicola, Dave Good, Josh Horwich, Barton House, Robert Johnson, Keith McGreggor, Mike Reed, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1994 Apple Computer, Inc.  All rights reserved.	*/

/*#pragma once*/

#ifndef sfntTypesIncludes
	#define sfntTypesIncludes

	#ifndef HeadSpinHDefined
		#include <HeadSpin.h>
	#endif
	
	#ifndef mathTypesIncludes
		#include "math_types.h"
	#endif
	
	#ifndef gxAnyNumber
		#define gxAnyNumber 1
	#endif
	
	#ifndef gxSharedFontTypeDefines
		
		typedef UInt32 gxFontTableTag;
		typedef UInt32 gxFontVariationTag;
		typedef UInt32 gxFontFormatTag;
		typedef UInt32 gxFontStorageTag;
		typedef gxFontVariationTag gxFontDescriptorTag;
	
		struct gxFontVariation {
			gxFontVariationTag		name;
			Fixed				value;
		};
		typedef struct gxFontVariation gxFontDescriptor;
	
		struct gxFontFeatureSetting {
			UInt16			setting;
			UInt16			nameID;
		};
	#endif
	
	struct sfntDirectoryEntry {
		gxFontTableTag				tableTag;
		UInt32 					checkSum;
		UInt32 					offset;
		UInt32 					length;
	};
	
	/* The search fields limits numOffsets to 4096. */
	
	struct sfntDirectory {
		gxFontFormatTag			format;
		UInt16				numOffsets;			/* number of tables */
		UInt16				searchRange;			/* (max2 <= numOffsets)*16 */
		UInt16				entrySelector;			/* log2(max2 <= numOffsets) */
		UInt16				rangeShift;			/* numOffsets*16-searchRange*/
		struct sfntDirectoryEntry		table[gxAnyNumber];	/* table[numOffsets] */
	};
	
	#define sizeof_sfntDirectory		12
	
	/* Cmap - character id to glyph id gxMapping */
	
	#define cmapFontTableTag			0x636d6170			/* 'cmap' */
	
	struct sfntCMapSubHeader {
		UInt16				format;
		UInt16				length;
		UInt16				languageID;			/* base-1 */
	};
	
	#define sizeof_sfntCMapSubHeader	6
	
	struct sfntCMapEncoding {
		UInt16				platformID;			/* base-0 */
		UInt16				scriptID;				/* base-0 */
	        UInt32					offset;
	};
	
	#define sizeof_sfntCMapEncoding 8
	
	struct sfntCMapHeader {
		UInt16				version;
		UInt16				numTables;
		struct sfntCMapEncoding		encoding[gxAnyNumber];
	};
	
	#define sizeof_sfntCMapHeader		4
	
	/* Name table */
	
	#define nameFontTableTag			0x6e616d65			/* 'name' */
	
	struct sfntNameRecord {
		UInt16				platformID;			/* base-0 */
		UInt16				scriptID;				/* base-0 */
		UInt16				languageID;			/* base-0 */
		UInt16				nameID;				/* base-0 */
		UInt16				length;
		UInt16				offset;
	};
	
	#define sizeof_sfntNameRecord		12
	
	struct sfntNameHeader {
		UInt16				format;
		UInt16				count;
		UInt16				stringOffset;
		struct sfntNameRecord		record[gxAnyNumber];
	};
	
	#define sizeof_sfntNameHeader	6
	
	
	/* Fvar table - gxFont variations */
	
	#define variationFontTableTag		0x66766172			/* 'fvar' */
	
	/* These define each gxFont variation */
	
	struct sfntVariationAxis {
		gxFontVariationTag			axisTag;
		Fixed					minValue;
		Fixed					defaultValue;
		Fixed					maxValue;
		Int16					flags;
		Int16					nameID;
	};
	
	#define sizeof_sfntVariationAxis	20
	
	/* These are named locations in gxStyle-space for the user */
	
	struct sfntInstance {
		Int16					nameID;
		Int16					flags;
		Fixed						coord[gxAnyNumber];	/* [axisCount] */
		/* room to grow since the header carries a tupleSize field */
	};
	
	#define sizeof_sfntInstance		4
	
	struct sfntVariationHeader {
		Fixed					version;				/* 1.0 Fixed */
		UInt16				offsetToData;			/* to first axis = 16*/
		UInt16				countSizePairs;			/* axis+inst = 2 */
		UInt16				axisCount, axisSize;
		UInt16				instanceCount;
		UInt16				instanceSize;
		/* Éother <count,size> pairs */
		struct sfntVariationAxis		axis[gxAnyNumber];		/* [axisCount] */
		struct sfntInstance			instance[gxAnyNumber];	/* [instanceCount] */
		/* Éother arrays of data */
	};
	
	#define sizeof_sfntVariationHeader	16
	
	/* Fdsc table - gxFont descriptor */
	
	#define descriptorFontTableTag		0x66647363			/* 'fdsc' */
	
	struct sfntDescriptorHeader {
		Fixed					version;				/* 1.0 in Fixed */
		UInt32				        descriptorCount;
		gxFontDescriptor			descriptor[gxAnyNumber];
	};
	
	#define sizeof_sfntDescriptorHeader	8
	
	/* Feat Table - layout feature table */
	
	#define featureFontTableTag		0x66656174				/* 'feat' */
	
	struct sfntFeatureName {
		UInt16				featureType;
		UInt16				settingCount;
		UInt32					offsetToSettings;
		UInt16				featureFlags;
		UInt16				nameID;
	};
	
	struct sfntFontRunFeature {
		UInt16			featureType;
		UInt16			setting;
	};

	struct sfntFeatureHeader {
		UInt32						version;				/* 1.0 */
		UInt16				featureNameCount;
		UInt16				featureSetCount;
		UInt32						reserved;				/* set to 0 */
		struct sfntFeatureName		names[gxAnyNumber];
		struct gxFontFeatureSetting	settings[gxAnyNumber];
		struct sfntFontRunFeature		runs[gxAnyNumber];
	};
	
	/* OS/2 Table */
	
	#define os2FontTableTag			0x4f532f32			/* 'OS/2' */
	
	/*  Special invalid glyph ID value, useful as a sentinel value, for example */
	#define nonGlyphID				(65535)
	
	/*** need to put os/2 structure definition here */

	#ifndef __cplusplus
		#ifndef gxSharedFontTypeDefines
			typedef struct gxFontVariation gxFontVariation;
			typedef struct gxFontFeatureSetting gxFontFeatureSetting;
		#endif
		typedef struct sfntDirectoryEntry sfntDirectoryEntry;
		typedef struct sfntDirectory sfntDirectory;
		typedef struct sfntCMapSubHeader sfntCMapSubHeader;
		typedef struct sfntCMapEncoding sfntCMapEncoding;
		typedef struct sfntCMapHeader sfntCMapHeader;
		typedef struct sfntNameRecord sfntNameRecord;
		typedef struct sfntNameHeader sfntNameHeader;
		typedef struct sfntVariationAxis sfntVariationAxis;
		typedef struct sfntInstance sfntInstance;
		typedef struct sfntVariationHeader sfntVariationHeader;
		typedef struct sfntDescriptorHeader sfntDescriptorHeader;
		typedef struct sfntFeatureName sfntFeatureName;
		typedef struct sfntFeatureHeader sfntFeatureHeader;
	#endif
	
	#ifndef gxSharedFontTypeDefines
		#define gxSharedFontTypeDefines
	#endif
#endif
