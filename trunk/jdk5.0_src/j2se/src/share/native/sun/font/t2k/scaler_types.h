/*
 * @(#)scaler_types.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* $Revision: 1.1 $ */
/*
**	scaler types.h
**
**	Public font scaler constants and data structure definitions which implement the Apple Font Interface.
**	These are the data types relevent to the API of a scaler.  Included are the types for managing memory blocks, 
**	and for communicating with scalers in the input and output directions.
**
**	Copyright © 1992-1994 by Apple Computer, Inc.  All rights reserved.
*/

/*#pragma once*/

#ifndef scalerTypeIncludes
	#define scalerTypeIncludes

	#ifndef mathTypesIncludes
		#include "math_types.h"		/* Fixed, mapping */
	#endif
	
	#ifndef sfntTypesIncludes
		#include "sfnt_types.h"		/* gxFontFormatTag */
	#endif
	
	#define truetypeFontFormatTag		0x74727565	/* 'true' */
	#define type1FontFormatTag		0x74797031	/* 'typ1' */
	#define nfntFontFormatTag		0x6e666e74	/* 'nfnt' */
	
	#define scaler_first_error		1
	#define scaler_first_warning		1024

	#define kOFAVersion1Dot0		0x10000
	#define kOFAVersion1Dot1		0x10100		/* added scalerVariationInfo */

	enum scalerErrors {
		scaler_no_problem = 0,				/* Everything went OK */
		
		scaler_null_context = scaler_first_error,/* Client passed a null context pointer */
		scaler_null_input,					/* Client passed a null input pointer */
		scaler_invalid_context,				/* There was a problem with the context */
		scaler_invalid_input,				/* There was a problem with an input */
		scaler_invalid_font_data,				/* A portion of the font was corrupt */
		scaler_new_block_failed,				/* A call to NewBlock() failed */
		scaler_get_font_table_failed,			/* The table was present (length > 0) but couldn't be read */
		scaler_bitmap_alloc_failed,			/* Call to allocate bitmap permanent block failed */
		scaler_outline_alloc_failed,			/* Call to allocate outline permanent block failed */
		scaler_required_table_missing,		/* A needed font table was not found */
		scaler_unsupported_outline_format,		/* Couldn't create an outline of the desired format */
		scaler_unsupported_stream_format,		/* ScalerStreamFont() call can't supply any requested format */
		scaler_unsupported_font_format,		/* No scaler supports the font format */
		scaler_hinting_error,				/* An error occurred during hinting */
		scaler_scan_error,					/* An error occurred in scan conversion */
		scaler_internal_error,				/* Scaler has a bug */
		scaler_invalid_matrix,				/* The transform matrix was unusable */
		scaler_fixed_overflow,				/* An overflow ocurred during matrix operations */
		scaler_API_version_mismatch,			/* Scaler requires a newer/older version of the scaler API */
		scaler_streaming_aborted,			/* StreamFunction callback indicated that streaming should cease */
		scaler_last_error = scaler_streaming_aborted,
	
		scaler_no_output = scaler_first_warning,	/* Couldn't fulfill any glyph request. */
		scaler_fake_metrics,				/* Returned metrics aren't based on information in the font */
		scaler_fake_linespacing,				/* Linespacing metrics not based on information in the font */
		scaler_glyph_substitution,			/* Requested glyph out of range, a substitute was used */
		scaler_last_warning = scaler_glyph_substitution
	};
	typedef tt_int32 scalerError;
	
	/* ScalerOpen output type */
	
	struct scalerInfo {
		gxFontFormatTag	format;		/* Font format supported by this scaler */
		Fixed			scalerVersion;	/* Version number of the scaler */
		Fixed			APIVersion;	/* Version of API implemented (compare with version in scalerContext) */
	};
	
	/* ScalerNewFont output type */
	
	enum scalerFontFlags {
		requiresLayoutFont		= 1,
		hasNormalLayoutFont	= 2,
		canReorderFont			= 4,
		canRearrangeFont		= 8,
		hasOutlinesFont			= 16
	};
	typedef tt_int32 scalerFontFlag;
	
	struct scalerFontInfo {
		tt_uint32			unitsPerEm;
		scalerFontFlag			flags;
		tt_uint32			numGlyphs;
	};
	
	/* ScalerNewVariation1Dot1 output type */
	
	struct scalerFixedRectangle {
		Fixed	left;
		Fixed	top;
		Fixed	right;
		Fixed	bottom;
	};
	#ifndef __cplusplus
		typedef struct scalerFixedRectangle scalerFixedRectangle;
	#endif
	
	struct scalerVariationInfo {
		scalerFixedRectangle		bounds;
	};
	
	/* ScalerNewTransform input types */
	
	enum scalerTransformFlags {
		applyHintsTransform		= 1,		/* Execute hinting instructions (grid fit) */
		exactBitmapTransform	= 2,		/* Use embedded gxBitmap iff exact size */
		useThresholdTransform	= 4,		/* Use scaled gxBitmap (if any) if below outline threshold */
		verticalTransform		= 8,		/* Glyphs will be in vertical orientation */
		deviceMetricsTransform	= 16,	/* All metrics should be device (vs. fractional) */
		allScalerTransformFlags	= applyHintsTransform | exactBitmapTransform | useThresholdTransform |
								verticalTransform | deviceMetricsTransform
	};
	typedef tt_int32 scalerTransformFlag;
	
	struct scalerTransform {
		scalerTransformFlag	flags;		/* Hint, embedded gxBitmap control, etc. */
		Fixed			pointSize;		/* The desired pointsize */
		const gxMapping	*fontMatrix;	/* The 3x3 matrix to apply to glyphs */
		gxPoint			resolution;	/* 2D device resolution */
		gxPoint			spotSize;		/* 2D pixel size */
	} ;
	
	/* ScalerNewTransform output type */
	
	struct scalerTransformInfo {
		gxPoint		before;			/* Spacing of the line before */
		gxPoint		after;			/* Spacing of the line after */
		gxPoint		caretAngle;		/* Rise (y) and run (x) of the insertion caret */
		gxPoint		caretOffset;		/* Adjustment to caret for variants like italic */
	};
	
	/* ScalerNewGlyph input types */
	
	enum scalerGlyphFlags {
		noImageGlyph = 1				/* Don't return the bitmap image for this glyph */
	};
	typedef tt_int32 scalerGlyphFlag;
	
	#define pathOutlineFormat		0x70617468	/* 'path' QuickDraw GX outline */
	
	typedef tt_int32 scalerOutlineFormat;
	
	struct scalerGlyph {
		tt_int32				glyphIndex;		/* Index of the glyph to be considered */
		tt_int32				bandingTop;		/* Banding controls (scanline numbers) top=bottom=0 means no banding */
		tt_int32				bandingBottom;
		scalerOutlineFormat	format;			/* Format of outline to return, ignored if no outline desired */
		scalerGlyphFlag		flags;			/* Control generation of image representation */
	};
	
	/* ScalerNewGlyph output types */
	
	struct scalerMetrics {
		gxPoint			advance;			
		gxPoint			sideBearing;
		gxPoint			otherSideBearing;
	};
	
	struct scalerRectangle {
		tt_int32				xMin;
		tt_int32				yMin;
		tt_int32				xMax;
		tt_int32				yMax;
	};

/*#pragma push*/
/*#pragma skipping on*/

	struct scalerBitmap {
		char					*image;			/* Pointer to pixels */
		gxPoint				topLeft;			/* Bitmap positioning relative to client's origin */
		struct scalerRectangle	bounds;			/* Bounding box of bitmap */
		tt_int32       				rowBytes;		/* Width in bytes */
	#ifdef cpuPrinter
		tt_int32       				actualBitmapSize;
		tt_int32       				scanConvertScratchSize;
		tt_int32       				dropoutScratchSize;
	#endif
	};
/*#pragma pop*/

	/* ScalerKernGlyphs input/output types */
	
	enum scalerKerningFlags {
		lineStartKerning		= 1,				/* Array of glyphs starts a line */
		lineEndKerning			= 2,				/* Array of glyphs ends a line */
		noCrossKerning			= 4,				/* Prohibit cross kerning */
		allScalerKerningFlags	= lineStartKerning | lineEndKerning | noCrossKerning
	};
	typedef tt_int32 scalerKerningFlag;
	
	enum scalerKerningNotes {
		noStakeKerningNote			= 1,		/* Indicates a glyph was involver in a kerning pair/group */
		crossStreamResetKerningNote	= 2		/* Indicates a return-to-baseline in cross-stream kerning */
	};
	typedef unsigned short scalerKerningNote;
	
	enum scalerKerningOutputs {
		noKerningAppliedOutput	= 0x0001		/* All kerning values were zero, kerning call had no effect */
	};
	typedef tt_int32 scalerKerningOutput;			/* These are bit-fields */
	
	struct scalerKerning {
		tt_int32					numGlyphs;	/* Number of glyphs in the glyphs array */
		fract					scaleFactor;	/* Amount of kerning to apply (0 == none, fract1 == all) */
		scalerKerningFlag		flags;		/* Various control flags */
		const unsigned short		*glyphs;		/* Pointer to the array of glyphs to be kerned */
		scalerKerningOutput		info;			/* Qualitative results of kerning */
	} ;
	
	/* ScalerStream input/output types */
	
	enum scalerStreamTypeFlags {
		cexec68K					= 0x0001,
		truetypeStreamType			= 0x0001,
		type1StreamType			= 0x0002,
		type3StreamType			= 0x0004,
		type42StreamType			= 0x0008,
		type42GXStreamType		= 0x0010,
		portableStreamType			= 0x0020,
		flattenedStreamType			= 0x0040,
		evenOddModifierStreamType	= 0x8000
	};
	typedef tt_uint32 scalerStreamTypeFlag;	/* Possible streamed font formats */
	
	enum scalerStreamActions {
		downloadStreamAction,				/* Transmit the (possibly sparse) font data */
		asciiDownloadStreamAction,			/* Transmit font data to a 7-bit ASCII destination */
		fontSizeQueryStreamAction,			/* Estimate in-printer memory used if the font were downloaded */
		encodingOnlyStreamAction,			/* Transmit only the encoding for the font */
		prerequisiteQueryStreamAction,		/* Return a list of prerequisite items needed for the font */
		prerequisiteItemStreamAction,			/* Transmit a specified prerequisite item */
		variationQueryStreamAction,			/* Return information regarding support for variation streaming */
		variationPSOperatorStreamAction		/* Transmit Postscript code necessary to effect variation of a font */
	};
	typedef tt_int32 scalerStreamAction;
	
	#define selectAllVariations	-1			/* Special variationCount value meaning include all variation data */

	struct scalerPrerequisiteItem {
		tt_int32			enumeration;			/* Shorthand tag identifying the item */
		tt_int32			size;					/* Worst case vm in printer item requires */
		unsigned char	name[1];				/* Name to be used by the client when emitting the item (Pascal string) */
	};
	
	struct scalerStream {
		const void				*streamRefCon;/* <-	private reference for client */
		const char				*targetVersion;/* <-	e.g. Postscript printer name (C string) */
		scalerStreamTypeFlag	types;		/* <->	Data stream formats desired/supplied */
		scalerStreamAction		action;		/* <- 	What action to take */
		tt_uint32			memorySize;	/* ->	Worst case memory use (vm) in printer or as sfnt */
		tt_int32					variationCount;	/* <-	The number of variations, or selectAllVariations */
		const struct gxFontVariation *variations;	/* <-	A pointer to an array of the variations */
		union {
			struct {
				const unsigned short	*encoding;/* <-	Intention is * unsigned short[256] */
				tt_int32			*glyphBits;	/* <->	Bitvector: a bit for each glyph, 1 = desired/supplied */
				char			*name;		/* <->	The printer font name to use/used (C string) */
			} font;						/* Normal font streaming information */
			
			struct {		
				tt_int32	       	size;			/* -> 	Size of the prereq. list in bytes (0 indicates no prerequisites)*/
				void			*list;		/* <-	Pointer to client block to hold list (nil = list size query only) */
			} prerequisiteQuery;				/* Used to obtain a list of prerequisites from the scaler */
			
			tt_int32		prerequisiteItem;		/* <- 	Enumeration value for the prerequisite item to be streamed.*/
			
			tt_int32		variationQueryResult;	/* ->	Output from the variationQueryStreamAction */
		} info;
	};
	
	struct scalerStreamData {
		tt_int32			hexFlag;				/* Indicates that the data is to be interpreted as hex, versus binary */
		tt_int32			byteCount;			/* Number of bytes in the data being streamed */
		const void		*data;				/* Pointer to the data being streamed */
	};
	
	enum scalerBlockTypes {
		scalerScratchBlock = -1,				/* Scaler alloced/freed temporary memory */
	
		scalerOpenBlock,					/* Five permanent input/state block types */
		scalerFontBlock,
		scalerVariationBlock,
		scalerTransformBlock,
		scalerGlyphBlock,
		scalerBlockCount,					/* Number of permanent block types */
		
		scalerOutlineBlock = scalerBlockCount,	/* Two output block types */
		scalerBitmapBlock 
	};
	typedef tt_int32 scalerBlockType;
	
	#define sfntDirectoryTag	0x64697220		/* 'dir ', special tag used only by scalers to access an sfnt's directory */
	
	#ifdef __cplusplus
		struct scalerContext;
	#else
		typedef struct scalerInfo scalerInfo;
		typedef struct scalerFontInfo scalerFontInfo;
		typedef struct scalerVariationInfo scalerVariationInfo;
		typedef struct scalerTransform scalerTransform;
		typedef struct scalerTransformInfo scalerTransformInfo;
		typedef struct scalerGlyph scalerGlyph;
		typedef struct scalerMetrics scalerMetrics;
		typedef struct scalerRectangle scalerRectangle;
		typedef struct scalerBitmap scalerBitmap;
		typedef struct scalerKerning scalerKerning;
		typedef struct scalerPrerequisiteItem scalerPrerequisiteItem;
		typedef struct scalerStream scalerStream;
		typedef struct scalerStreamData scalerStreamData;
		typedef struct scalerContext scalerContext;
	#endif
	
	#ifdef __cplusplus
	extern "C" {
	#endif

/* Type definitions for function pointers used in the scalerContext structure */

    /*#pragma push*/
    /*#pragma skipping on*/

    /*#pragma procname GetFontTable*/
typedef tt_int32 (*GetFontTableProcPtr)(scalerContext *context, gxFontTableTag tableTag, tt_int32 offset, tt_int32 length, void *data);
    /*#pragma procname ReleaseFontTable*/
typedef void (*ReleaseFontTableProcPtr)(scalerContext *context, void* fontData);
    /*#pragma procname NewBlock*/
typedef void* (*NewBlockProcPtr)(scalerContext *context, tt_int32 size, scalerBlockType theType, void* oldBlock);
    /*#pragma procname DisposeBlock*/
typedef void (*DisposeBlockProcPtr)(scalerContext *context, void* scratchData, scalerBlockType theType);
    /*#pragma procname StreamFunction*/
typedef tt_int32 (*StreamFunctionProcPtr)(scalerContext *context, struct scalerStream* streamInfo, const struct scalerStreamData *dataInfo);
    /*#pragma procname ScanLineFunction*/
typedef void (*ScanLineFunctionProcPtr)(scalerContext *context, const struct scalerBitmap* scanLine);
    /*#pragma procname PostErrorFunction*/
typedef void (*PostErrorFunctionProcPtr)(scalerContext *context, scalerError theProblem);

#ifdef cpuPrinter
	typedef void (*FindExtremaProcPtr)(const struct fnt_ElementType *glyphPtr, struct sc_BitMapData *bbox, tt_int32 doc, struct fsg_SplineKey *key );

	typedef tt_int32 (*ScanChar2ProcPtr)(const struct fnt_ElementType *glyphPtr, struct sc_GlobalData *scPtr, 
				struct sc_BitMapData *bbox, short lowBand, short highBand, tt_int32 scanKind, scalerContext *theContext);
#endif

    /*#pragma procname ScalerFunction*/
typedef void (*ScalerFunctionProcPtr)(scalerContext *ontext, void* data);

/* scalerContext: the vehicle with which the caller and scaler communicate */

struct scalerContext {
	Fixed				version;			/* Version of the scaler API implemented by the caller */
	void					*theFont;			/* Caller's private reference to the font being processed */
	gxFontFormatTag		format;			/* Format of the sfnt font data, corresponds to the scaler */

	GetFontTableProcPtr		GetFontTable;		/* Callback for accessing sfnt tables or portions thereof */ 
	ReleaseFontTableProcPtr	ReleaseFontTable;	/* Callback for releasing sfnt tables */
	NewBlockProcPtr		NewBlock;			/* Callback for allocating and/or growing permanent and scratch blocks */
	DisposeBlockProcPtr		DisposeBlock;		/* Callback for freeing permanent and scratch blocks */
	StreamFunctionProcPtr	StreamFunction;	/* Callback for transmitting blocks of data during streaming */
	ScanLineFunctionProcPtr	ScanLineFunction;	/* Callback for emitting individual bitmap scanlines during scan conversion */
	PostErrorFunctionProcPtr	PostErrorFunction;	/* Callback for posting errors and warnings */
	
	#ifdef cpuPrinter
		FindExtremaProcPtr	FindExtrema;		/* should point at sc_FindExtrema4  as of 2/7/94 */
		ScanChar2ProcPtr	ScanChar2;
	#endif
	
	void					*scalerBlocks[scalerBlockCount];	/* Array of permanent scaler blocks */
		
	ScalerFunctionProcPtr	ScalerFunction;		/* Callback for scaler-specific tracing, debugging, etc. */
};
    /*#pragma pop*/
	
	#ifdef __cplusplus
	}
	#endif
#endif
